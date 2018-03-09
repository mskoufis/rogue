/**
 *-----------------------------------------------------------------------------
 * Title      : Stream frame iterator
 * ----------------------------------------------------------------------------
 * File       : FrameIterator.h
 * Created    : 2018-03-06
 * ----------------------------------------------------------------------------
 * Description:
 * Stream frame iterator
 * ----------------------------------------------------------------------------
 * This file is part of the rogue software platform. It is subject to 
 * the license terms in the LICENSE.txt file found in the top-level directory 
 * of this distribution and at: 
 *    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
 * No part of the rogue software platform, including this file, may be 
 * copied, modified, propagated, or distributed except according to the terms 
 * contained in the LICENSE.txt file.
 * ----------------------------------------------------------------------------
**/
#ifndef __ROGUE_INTERFACES_STREAM_FRAME_ITERATOR_H__
#define __ROGUE_INTERFACES_STREAM_FRAME_ITERATOR_H__
#include <stdint.h>
#include <vector>

#include <boost/python.hpp>
namespace rogue {
   namespace interfaces {
      namespace stream {

         //! Frame iterator
         class FrameIterator : public std::iterator<std::random_access_iterator_tag, uint8_t> {
            friend class rogue::interfaces::stream::Frame;

               //! End flag
               bool end_;

               //! Frame position
               uint32_t framePos_;

               //! Associated frame
               boost::shared_ptr<rogue::interfaces::stream::Frame> frame_;

               //! current buffer
               std::vector<boost::shared_ptr<rogue::interfaces::stream::Buffer> >::iterator curr_;

               //! Current buffer position
               uint32_t buffPos_;

               //! Creator
               FrameIterator(boost::shared_ptr<rogue::interfaces::stream::Frame> frame, 
                             uint32_t offset, bool end);

            public:

               //! Creator
               FrameIterator();

               //! Setup class in python
               static void setup_python();

               //! Copy assignment
               const rogue::interfaces::stream::FrameIterator operator =(
                     const rogue::interfaces::stream::FrameIterator &rhs);

               //! De-reference
               uint8_t operator *() const;

               //! Pointer
               uint8_t * operator ->() const;

               //! De-reference by index
               uint8_t operator [](const uint32_t &offset) const;

               //! Increment
               const rogue::interfaces::stream::FrameIterator & operator ++();

               //! Post Increment
               rogue::interfaces::stream::FrameIterator operator ++(int);

               //! Decrement
               const rogue::interfaces::stream::FrameIterator & operator --();

               //! Post Decrement
               rogue::interfaces::stream::FrameIterator operator --(int);

               //! Not Equal
               bool operator !=(const rogue::interfaces::stream::FrameIterator & other) const;

               //! Equal
               bool operator ==(const rogue::interfaces::stream::FrameIterator & other) const;

               //! Less than
               bool operator <(const rogue::interfaces::stream::FrameIterator & other) const;

               //! greater than
               bool operator >(const rogue::interfaces::stream::FrameIterator & other) const;

               //! Less than equal
               bool operator <=(const rogue::interfaces::stream::FrameIterator & other) const;

               //! greater than equal
               bool operator >=(const rogue::interfaces::stream::FrameIterator & other) const;

               //! Increment by value
               rogue::interfaces::stream::FrameIterator operator +(const int32_t &add) const;

               //! Descrment by value
               rogue::interfaces::stream::FrameIterator operator -(const int32_t &sub) const;

               //! Sub incrementers
               int32_t operator -(const rogue::interfaces::stream::FrameIterator &other) const;

               //! Increment by value
               rogue::interfaces::stream::FrameIterator & operator +=(const int32_t &add);

               //! Descrment by value
               rogue::interfaces::stream::FrameIterator & operator -=(const int32_t &sub);

         };
      }
   }
}

#endif

