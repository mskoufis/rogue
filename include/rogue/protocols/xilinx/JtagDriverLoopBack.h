//-----------------------------------------------------------------------------
// Title      : JTAG Support
//-----------------------------------------------------------------------------
// Company    : SLAC National Accelerator Laboratory
//-----------------------------------------------------------------------------
// Description: JtagDriverLoopBack.h
//-----------------------------------------------------------------------------
// This file is part of 'SLAC Firmware Standard Library'.
// It is subject to the license terms in the LICENSE.txt file found in the
// top-level directory of this distribution and at:
//    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html.
// No part of 'SLAC Firmware Standard Library', including this file,
// may be copied, modified, propagated, or distributed except according to
// the terms contained in the LICENSE.txt file.
//-----------------------------------------------------------------------------

#ifndef __ROGUE_PROTOCOLS_XILINX_JTAG_DRIVER_LOOP_BACK_H__
#define __ROGUE_PROTOCOLS_XILINX_JTAG_DRIVER_LOOP_BACK_H__

#include <rogue/protocols/xilinx/JtagDriverAxisToJtag.h>

namespace rogue
{
	namespace protocols
	{
		namespace xilinx
		{
			// The loopback driver is used for testing; it loops TDI back to TDO;
			// Optionally, it can be initialized with a file-name.
			// The file is expected to contain text describing JTAG vectors of the format
			//  file:    record {, record}
			//  record:  bitlen_line, tms_line, tdi_line, tdo_line {, tms_line, tdi_line, tdo_line }
			//
			//  bitlen_line: "LENBITS: ", number_in_ascii
			//  tms_line   : "TMS :    ", 32bit_hexnum_in_ascii
			//  tdi_line   : "TDI :    ", 32bit_hexnum_in_ascii
			//  tdo_line   : "TDO :    ", 32bit_hexnum_in_ascii
			//
			class JtagDriverLoopBack : public JtagDriverAxisToJtag
			{
			private:
				FILE *f_;
				bool skip_;
				bool tdoOnly_;
				unsigned long line_;

			public:
				JtagDriverLoopBack(int argc, char *const argv[], const char *fnam = 0);

				virtual ~JtagDriverLoopBack();

				//! Setup class in python
				static void setup_python();

				virtual unsigned emulWordSize();
				virtual unsigned emulMemDepth();

				virtual bool rdl(char *buf, size_t bufsz);

				virtual unsigned long check(unsigned long val, const char *fmt, bool rdOnly = false);

				virtual void checkTDI(unsigned long val);

				virtual void checkTMS(unsigned long val);

				virtual void checkLEN(unsigned long val);

				virtual unsigned long getTDO();

				virtual unsigned long getValLE(uint8_t *buf, unsigned wsz);

				virtual void setValLE(unsigned long val, uint8_t *buf, unsigned wsz);

				virtual unsigned long
				getMaxVectorSize();

				// main transfer method
				virtual int
				xfer(uint8_t *txb, unsigned txBytes, uint8_t *hdbuf, unsigned hsize, uint8_t *rxb, unsigned size);
			};
		}
	}
}

#endif
