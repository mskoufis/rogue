/**
 *-----------------------------------------------------------------------------
 * Title      : UDP Common Functions
 * ----------------------------------------------------------------------------
 * File       : Common.h
 * Created    : 2017-01-07
 * Last update: 2017-01-07
 * ----------------------------------------------------------------------------
 * Description:
 * UDP Common
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
#ifndef __ROGUE_PROTOCOLS_UDP_COMMON_H__
#define __ROGUE_PROTOCOLS_UDP_COMMON_H__
#include <rogue/Logging.h>
#include <stdint.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

namespace rogue {
   namespace protocols {
      namespace udp {

         //! UDP Core
         class Core {

               static const uint32_t MaxJumboPayload = 8900;
               static const uint32_t MaxStdPayload   = 1400;

            protected:

               rogue::LoggingPtr udpLog_;

               //! Jumbo frames enables
               bool jumbo_;

               //! Socket
               int32_t  fd_;

               //! Remote socket address
               struct sockaddr_in remAddr_;

               //! Timeout value
               uint32_t timeout_;

               boost::thread* thread_;

               //! mutex
               boost::mutex udpMtx_;

            public:

               //! Setup class in python
               static void setup_python();

               //! Creator
               Core(bool jumbo);

               //! Destructor
               ~Core();

               //! Return max payload
               uint32_t maxPayload();

               //! Set UDP RX Size
               bool setRxSize(uint32_t size);

               //! Set timeout for frame transmits in microseconds
               void setTimeout(uint32_t timeout);
         };

         // Convienence
         typedef boost::shared_ptr<rogue::protocols::udp::Core> CorePtr;
      }
   }
};

#endif
