/**
 *-----------------------------------------------------------------------------
 * Title      : Common Rogue Functions
 * ----------------------------------------------------------------------------
 * File       : common.h
 * Author     : Ryan Herbst, rherbst@slac.stanford.edu
 * Created    : 2016-10-07
 * Last update: 2016-10-07
 * ----------------------------------------------------------------------------
 * Description:
 * Common Rogue Functions and macros
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
#ifndef __ROGUE_COMMON_H__
#define __ROGUE_COMMON_H__

#include <boost/python.hpp>

PyThreadState * PyRogue_SaveThread();
void PyRogue_RestoreThread(PyThreadState * state);

#define PyRogue_BEGIN_ALLOW_THREADS { PyThreadState *_save; _save = PyRogue_SaveThread();
#define PyRogue_END_ALLOW_THREADS PyRogue_RestoreThread(_save); }

#endif