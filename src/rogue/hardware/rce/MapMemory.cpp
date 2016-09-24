/**
 *-----------------------------------------------------------------------------
 * Title      : RCE Memory Mapped Access
 * ----------------------------------------------------------------------------
 * File       : MapMemory.h
 * Author     : Ryan Herbst, rherbst@slac.stanford.edu
 * Created    : 2017-09-17
 * Last update: 2017-09-17
 * ----------------------------------------------------------------------------
 * Description:
 * Class for interfacing to RCE mapped memory space.
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
#include <rogue/hardware/rce/MapMemory.h>
#include <rogue/interfaces/memory/Block.h>
#include <boost/make_shared.hpp>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

namespace rhr = rogue::hardware::rce;
namespace rim = rogue::interfaces::memory;
namespace bp  = boost::python;

//! Class creation
rhr::MapMemoryPtr rhr::MapMemory::create () {
   rhr::MapMemoryPtr r = boost::make_shared<rhr::MapMemory>();
   return(r);
}

//! Creator
rhr::MapMemory::MapMemory() {
   fd_      = -1;
}

//! Destructor
rhr::MapMemory::~MapMemory() {
   this->close();
}

//! Open the device. Pass destination.
bool rhr::MapMemory::open ( ) {
   bool ret;

   ret = true;

   mapMtx_.lock();

   if ( fd_ > 0 ) ret = false;
   else {
      fd_ = ::open("/dev/mem", O_RDWR | O_SYNC);
      if ( fd_ > 0 ) ret = true;
   }
   mapMtx_.unlock();
   return(ret);
}

//! Close the device
void rhr::MapMemory::close() {
   uint32_t x;

   mapMtx_.lock();
   if ( fd_ < 0 ) return;

   for ( x=0; x < maps_.size(); x++ )
      munmap(maps_[x].ptr,maps_[x].size);

   maps_.clear();
   mapMtx_.unlock();

   ::close(fd_);
}

//! Add a memory space
void rhr::MapMemory::addMap(uint32_t address, uint32_t size) {
   rhr::Map map;

   map.base = address;
   map.size = size;

   mapMtx_.lock();

   if ( fd_ >= 0 ) {

      map.ptr = (uint8_t *)mmap(NULL, map.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, map.base);

      if ( map.ptr == NULL ) return;

      maps_.push_back(map);
   }
   mapMtx_.unlock();
}

// Find matching address space, lock before use
uint8_t * rhr::MapMemory::findSpace (uint32_t base, uint32_t size) {
   uint32_t x;

   for (x=0; x < maps_.size(); x++) {
      if ( (base > maps_[x].base) && (((base - maps_[x].base) + size) < maps_[x].size) ) {
         return(maps_[x].ptr + (base-maps_[x].base));
      }
   }
   return(NULL);
}

//! Post a transaction
void rhr::MapMemory::doTransaction(bool write, bool posted, rim::BlockPtr block) { 
   uint8_t * ptr;

   if ((ptr = findSpace(block->getAddress(),block->getSize())) == NULL) {
      block->complete(1);
   }
   else if (write) {
      memcpy(ptr,block->getData(),block->getSize());
      block->complete(0);
   }
   else {
      memcpy(block->getData(),ptr,block->getSize());
      block->complete(0);
   }
}

void rhr::MapMemory::setup_python () {

   bp::class_<rhr::MapMemory, bp::bases<rim::Slave>, rhr::MapMemoryPtr, boost::noncopyable >("MapMemory",bp::init<>())
      .def("create",         &rhr::MapMemory::create)
      .staticmethod("create")
      .def("open",           &rhr::MapMemory::open)
      .def("close",          &rhr::MapMemory::close)
      .def("addMap",         &rhr::MapMemory::addMap)
   ;
}

