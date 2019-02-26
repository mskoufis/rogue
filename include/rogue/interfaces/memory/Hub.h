/**
 *-----------------------------------------------------------------------------
 * Title      : Hub Hub
 * ----------------------------------------------------------------------------
 * File       : Hub.h
 * Author     : Ryan Herbst, rherbst@slac.stanford.edu
 * Created    : 2016-09-20
 * ----------------------------------------------------------------------------
 * Description:
 * A memory interface hub. Accepts requests from multiple masters and forwards
 * them to a downstream slave. Address is updated along the way. Includes support
 * for modification callbacks.
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
#ifndef __ROGUE_INTERFACES_MEMORY_HUB_H__
#define __ROGUE_INTERFACES_MEMORY_HUB_H__
#include <stdint.h>
#include <vector>

#include <rogue/interfaces/memory/Master.h>
#include <rogue/interfaces/memory/Slave.h>
#include <thread>

#ifndef NO_PYTHON
#include <boost/python.hpp>
#endif

namespace rogue {
   namespace interfaces {
      namespace memory {

         //! Memory interface Hub device
         /** The memory bus Hub serves as both a Slave and a Master for memory transactions. It
          * will accept a Transaction from an attached Master and pass it down to the next
          * level Slave or Hub device. It will apply its local offset address to the transaction
          * as it is passed down to the next level. 
          *
          * A Hub can be sub-classed in either Python or C++ is order to further manipulate the 
          * transaction data on the way down or convert the initial Transaction into multiple 
          * transactions to the next level. This can be usefull to hide complex windows memory 
          * spaces or transactions that require multipled steps be performed in hardware.
          *
          * If a non zero min and max transaction size are passed at creation this Hub will
          * behave as if it is a new root Slave memory device in the tree. This is usefull in
          * cases where this Hub will master a paged address or other virtual address space.
          *
          * A pyrogue.Device instance is the most typical Hub used in Rogue.
          */
         class Hub : public Master, public Slave {

               //! Offset address of hub
               uint64_t offset_;

               //! Flag if this is a base slave
               bool root_;

            public:

               //! Class factory which returns a pointer to a Hub (HubPtr)
               /**Not exposed to Python
                *
                * @param offset The offset of this Hub device
                * @param min The min transaction size, 0 if not a virtual memory space root
                * @param min The max transaction size, 0 if not a virtual memory space root
                */
               static std::shared_ptr<rogue::interfaces::memory::Hub> create (uint64_t offset, uint32_t min, uint32_t max);

               //! Setup class for use in python
               /* Not exposed to Python
                */
               static void setup_python();

               //! Create a Hub device with a given offset
               /** Exposed to Python as Hub()
                *
                * Do not call directly. Only called from the Master class.
                * @param offset The offset of this Hub device
                */
               Hub(uint64_t offset, uint32_t min, uint32_t max);

               //! Destroy the Hub
               ~Hub();

               //! Get offset of this Hub
               /** Return the offset address of this Hub
                *
                * Exposted as _getOffset() to Python
                * @return 64-bit address offset
                */
               uint64_t getOffset();

               //! Get full address of this Hub
               /** Return the full address of this Hub
                *
                * Exposted as _getAddress() to Python
                * @return 64-bit address
                */
               uint64_t getAddress();

               //! Interface to service the getSlaveId request from an attached master
               /** This Hub will foward this request to the next level device.
                *
                * Not exposted to Python
                * @return 32-bit slave ID
                */
               uint32_t doSlaveId();

               //! Interface to service the getMinAccess request from an attached master
               /** This Hub will foward this request to the next level device.                 *
                * Not exposted to Python
                * @return Min transaction access size
                */
               uint32_t doMinAccess();

               //! Interface to service the getMaxAccess request from an attached master
               /** This Hub will foward this request to the next level device. A Hub
                * sub-class is allowed to override this method.
                *
                * Not exposted to Python
                * @return Max transaction access size
                */
               uint32_t doMaxAccess();

               //! Interface to service the getAddress request from an attached master
               /** This Hub will foward this request to the next level device and apply
                * the local address offset. A Hub sub-class is allowed to override this method.
                *
                * Not exposted to Python
                * @return Max transaction access size
                */
               uint64_t doAddress();

               //! Interface to service the transaction request from an attached master
               /** This Hub will foward this request to the next level device and apply
                * the local address offset. 
                *
                * It is possible for this method to be overriden in either a Python or C++
                * subclass. Examples of sub-classing a Hub are included elsewhere in this
                * document.
                *
                * Exposted to Python as _doTransaction()
                * @param transaction Transaction pointer as TransactionPtr
                */
               virtual void doTransaction(std::shared_ptr<rogue::interfaces::memory::Transaction> transaction);
         };

         //! Alias for using shared pointer as HubPtr
         typedef std::shared_ptr<rogue::interfaces::memory::Hub> HubPtr;
       
#ifndef NO_PYTHON

         // Memory Hub class, wrapper to enable pyton overload of virtual methods
         class HubWrap : 
            public rogue::interfaces::memory::Hub, 
            public boost::python::wrapper<rogue::interfaces::memory::Hub> {

            public:

               // Constructor
               HubWrap(uint64_t offset, uint32_t min, uint32_t max);

               // Post a transaction. Master will call this method with the access attributes.
               void doTransaction(std::shared_ptr<rogue::interfaces::memory::Transaction> transaction);

               // Post a transaction. Master will call this method with the access attributes.
               void defDoTransaction(std::shared_ptr<rogue::interfaces::memory::Transaction> transaction);
         };
         
         // Convienence
         typedef std::shared_ptr<rogue::interfaces::memory::HubWrap> HubWrapPtr;
#endif

      }
   }
}

#endif

