/**
 *-----------------------------------------------------------------------------
 * Title         : SLAC Batcher Core, Version 1
 * ----------------------------------------------------------------------------
 * File          : CoreV1.h
 * Author        : Ryan Herbst <rherbst@slac.stanford.edu>
 * Created       : 10/26/2018
 *-----------------------------------------------------------------------------
 * Description :
 *    AXI Batcher V1
 *
 * The batcher protocol starts with a super header followed by a number of
 * frames each with a tail to define the boundaries of each frame.
 *
 * Super Header:
 *
 *    Byte 0:
 *       Bits 3:0 = Version = 1
 *       Bits 7:4 = Width = 2 * 2 ^ val Bytes
 *    Byte 1:
 *       Bits 15:8 = Sequence # for debug
 *    The reset of the width is padded with zeros
 *
 * Frame Tail: Tail size is always equal to the interfae width or 64-bis
 *             whichever is larger. Padded values are 0 (higher bytes).
 *
 *    Word 0:
 *       bits 31:0 = size
 *    Word 1:
 *       bits  7:0  = Destination
 *       bits 15:8  = First user
 *       bits 23:16 = Last user
 *       bits 31:24 = Valid bytes in last field
 *
 *-----------------------------------------------------------------------------
 * This file is part of the rogue software platform. It is subject to 
 * the license terms in the LICENSE.txt file found in the top-level directory 
 * of this distribution and at: 
    * https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
 * No part of the rogue software platform, including this file, may be 
 * copied, modified, propagated, or distributed except according to the terms 
 * contained in the LICENSE.txt file.
 *-----------------------------------------------------------------------------
**/
#include <stdint.h>
#include <thread>
#include <memory>
#include <rogue/interfaces/stream/Frame.h>
#include <rogue/interfaces/stream/FrameIterator.h>
#include <rogue/protocols/batcher/CoreV1.h>
#include <rogue/protocols/batcher/Data.h>
#include <rogue/GeneralError.h>
#include <math.h>

namespace rpb = rogue::protocols::batcher;
namespace ris = rogue::interfaces::stream;

//! Class creation
rpb::CoreV1Ptr rpb::CoreV1::create() {
   rpb::CoreV1Ptr p = std::make_shared<rpb::CoreV1>();
   return(p);
}

//! Setup class in python
void rpb::CoreV1::setup_python() { }

//! Creator with version constant
rpb::CoreV1::CoreV1() {
   log_ = rogue::Logging::create("batcher.CoreV1");

   headerSize_ = 0;
   tailSize_   = 0;
   seq_        = 0;
}

//! Deconstructor
rpb::CoreV1::~CoreV1() {}

//! Record count
uint32_t rpb::CoreV1::count() {
   return list_.size();
}

//! Get header size
uint32_t rpb::CoreV1::headerSize() {
   return headerSize_;
}

//! Get header iterator
ris::FrameIterator rpb::CoreV1::header() {
   return frame_->beginRead();
}

//! Get tail size
uint32_t rpb::CoreV1::tailSize() {
   return tailSize_;
}

//! Get tail iterator
ris::FrameIterator & rpb::CoreV1::tail(uint32_t index) {
   if ( index >= tails_.size() ) 
      throw rogue::GeneralError::boundary("batcher::CoreV1::tail", index, tails_.size());

   // Invert order on return
   return tails_[(tails_.size()-1) - index];
}

//! Get data
rpb::DataPtr & rpb::CoreV1::record(uint32_t index) {
   if ( index >= list_.size() ) 
      throw rogue::GeneralError::boundary("batcher::CoreV1::record", index, list_.size());

   // Invert order on return
   return list_[(list_.size()-1) - index];
}

//! Return sequence
uint32_t rpb::CoreV1::sequence() {
   return seq_;
}

//! Process a frame
bool rpb::CoreV1::processFrame ( ris::FramePtr frame ) {
   uint8_t  temp;
   uint32_t rem;
   uint32_t fSize;
   uint8_t  dest;
   uint8_t  fUser;
   uint8_t  lUser;
   uint32_t fJump;

   // Reset old data
   reset();

   ris::FrameIterator beg;
   ris::FrameIterator mark;
   ris::FrameIterator tail;

   // Drop errored frames
   if ( (frame->getError()) ) {
      log_->warning("Dropping frame due to error: 0x%x",frame->getError());
      return false;
   }

   // Drop small frames
   if ( (rem = frame->getPayload()) < 16)  {
      log_->warning("Dropping small frame size = %i",frame->getPayload());
      return false;
   }

   // Get version & size
   beg = frame->beginRead();
   ris::fromFrame(beg, 1, &temp);
   
   /////////////////////////////////////////////////////////////////////////
   // Super-Frame Header in firmware
   /////////////////////////////////////////////////////////////////////////
   // v.txMaster.tValid               := '1';
   // v.txMaster.tData(3 downto 0)    := x"1";  -- Version = 0x1
   // v.txMaster.tData(7 downto 4)    := toSlv(log2(AXIS_WORD_SIZE_C/2), 4);
   // v.txMaster.tData(15 downto 8)   := r.seqCnt;
   // v.txMaster.tData(511 downto 16) := (others => '0');
   // ssiSetUserSof(AXIS_CONFIG_G, v.txMaster, '1');   
   /////////////////////////////////////////////////////////////////////////

   // Check version, convert width
   if ( (temp & 0xF) != 1 ) {
      log_->warning("Version mismatch. Got %i",(temp&0xF));
      return false;
   }
   
   /////////////////////////////////////////////////////////////////////////
   // headerSize = (uint32_t)pow(2,float( ( (temp >> 4) & 0xF) + 1) );
   /////////////////////////////////////////////////////////////////////////
   // Integer pow() when powers of 2 (more efficient than floating point)
   headerSize_ = 1 << ( ((temp >> 4) & 0xF) + 1);

   // Set tail size, min 64-bits
   tailSize_ = (headerSize_ < 8)?8:headerSize_;

   // Get sequence #
   ris::fromFrame(beg, 1, &seq_);

   // Frame needs to large enough for header + 1 tail
   if ( rem < (headerSize_ + tailSize_)) {
      log_->warning("Not enough space (%i) for tail (%i) + header (%i)",rem,headerSize_,tailSize_);
      reset();
      return false;
   }

   // Skip the rest of the header, compute remaining frame size
   beg += (headerSize_-2); // Aready read 2 bytes from frame
   rem -= headerSize_;

   // Set marker to end of frame
   mark = frame->endRead();

   // Process each frame, stop when we have reached just after the header
   while (mark != beg) {

      // sanity check
      if ( rem < tailSize_ ) {
         log_->warning("Not enough space (%i) for tail (%i)",rem,tailSize_);
         reset();
         return false;
      }

      // Jump to start of the tail
      mark -= tailSize_;
      rem  -= tailSize_;

      // Add tail iterator to end of list
      tails_.push_back(mark);
      
      // Get tail data, use a new iterator
      tail = mark;
      ris::fromFrame(tail, 4, &fSize);
      ris::fromFrame(tail, 1, &dest);
      ris::fromFrame(tail, 1, &fUser);
      ris::fromFrame(tail, 1, &lUser);

      // Round up rewind amount to width
      if ( (fSize % headerSize_) == 0) fJump = fSize;
      else fJump = ((fSize / headerSize_) + 1) * headerSize_;

      // Not enough data for rewind value
      if ( fJump > rem ) {
         log_->warning("Not enough space (%i) for frame (%i)",rem,fJump);
         reset();
         return false;
      }

      // Set marker to start of data 
      mark -= fJump;
      rem  -= fJump;

      // Create data record and add it to end of list
      list_.push_back(rpb::Data::create(mark,fSize,dest,fUser,lUser));
   }
   return true;
}

//! Reset data
void rpb::CoreV1::reset() {
   frame_.reset();
   list_.clear();
   tails_.clear();

   headerSize_ = 0;
   tailSize_   = 0;
   seq_        = 0;
}

