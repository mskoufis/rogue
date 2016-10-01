#!/usr/bin/env python
#-----------------------------------------------------------------------------
# Title      : Python package test 
#-----------------------------------------------------------------------------
# File       : exoTest.py
# Author     : Ryan Herbst, rherbst@slac.stanford.edu
# Created    : 2016-09-29
# Last update: 2016-09-29
#-----------------------------------------------------------------------------
# Description:
# Python package test
#-----------------------------------------------------------------------------
# This file is part of the rogue software platform. It is subject to 
# the license terms in the LICENSE.txt file found in the top-level directory 
# of this distribution and at: 
#    https://confluence.slac.stanford.edu/display/ppareg/LICENSE.html. 
# No part of the rogue software platform, including this file, may be 
# copied, modified, propagated, or distributed except according to the terms 
# contained in the LICENSE.txt file.
#-----------------------------------------------------------------------------
import rogue.hardware.pgp
import pyrogue.devices.axi
import pyrogue.utilities
import pyrogue.utilities.fileio
import yaml
import time

# Microblaze console printout
class MbDebug(rogue.interfaces.stream.Slave):

   def __init__(self):
      rogue.interfaces.stream.Slave.__init__(self)
      self.enable = False

   def _acceptFrame(self,frame):
      if self.enable:
         p = bytearray(frame.getPayload())
         frame.read(p,0)
         print('-------- Microblaze Console --------')
         print(p.decode('utf-8'))

def wyaml(top):
   with open('data.yml','w') as outfile:
      yaml.dump(top.getStructure())

# Set base
root = pyrogue.Root()

# Create the PGP interfaces
pgpVc0 = rogue.hardware.pgp.PgpCard('/dev/pgpcard_0',0,0)
pgpVc1 = rogue.hardware.pgp.PgpCard('/dev/pgpcard_0',0,1)
pgpVc3 = rogue.hardware.pgp.PgpCard('/dev/pgpcard_0',0,3)

print("")
print("PGP Card Version: %x" % (pgpVc0.getInfo().version))

# Create and Connect SRP to VC0
srp = rogue.protocols.srp.SrpV0()
pyrogue.streamConnectBiDir(pgpVc0,srp)

# File writer 
dataFile = pyrogue.utilities.fileio.StreamWriterDevice(root,'writer')

# Add data stream to file as tag 1, type 8
pyrogue.streamConnect(pgpVc1,dataFile.getWriter().getPort(0x1,0x8))

## Add console stream to file as tag 2, type 10
pyrogue.streamConnect(pgpVc3,dataFile.getWriter().getPort(0x2,0xA))

# PRBS Receiver as secdonary receiver for VC1
prbsRx = pyrogue.utilities.PrbsDevice(root,'prbsRx')
pyrogue.streamTap(pgpVc1,prbsRx.getPrbs())

# Microblaze console connected to VC2
mbcon = MbDebug()
pyrogue.streamTap(pgpVc3,mbcon)

# Add Devices
version = pyrogue.devices.axi.AxiVersion(parent=root,name='axiVersion',memBase=srp)
