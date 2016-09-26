#!/usr/bin/env python

import rogue.hardware.pgp
import rogue.utilities 
import rogue.interfaces.stream
import time

# Microblaze console printout
class mbDebug(rogue.interfaces.stream.Slave):

   def __init__(self):
      rogue.interfaces.stream.Slave.__init__(self)

   def acceptFrame(self,frame):
      p = bytearray(frame.getPayload())
      frame.read(p,0)
      print("-------- Microblaze Console --------")
      print(p.decode("utf-8"))

pgpVc0 = rogue.hardware.pgp.PgpCard("/dev/pgpcard_0",0,0)
pgpVc1 = rogue.hardware.pgp.PgpCard("/dev/pgpcard_0",0,1)
pgpVc3 = rogue.hardware.pgp.PgpCard("/dev/pgpcard_0",0,3)

print("PGP Card Version: %x" % (pgpVc0.getInfo().version))

# Create and Connect SRP
srp = rogue.protocols.srp.Bridge(0)
srp.setSlave(pgpVc0)
pgpVc0.setSlave(srp)

# Connect Data
dataRx = rogue.utilities.Prbs()
pgpVc1.setSlave(dataRx)

# Create registers
fwVersion  = rogue.interfaces.memory.Block(0x00000,4)
scratchpad = rogue.interfaces.memory.Block(0x00004,4)
deviceDna  = rogue.interfaces.memory.Block(0x00008,8)
heartbeat  = rogue.interfaces.memory.Block(0x00024,4)
buildstamp = rogue.interfaces.memory.Block(0x00800,256)
prbsCont   = rogue.interfaces.memory.Block(0x30000,4)
prbsLength = rogue.interfaces.memory.Block(0x30004,4)

# Connect to SRP
fwVersion.setSlave(srp)
scratchpad.setSlave(srp)
deviceDna.setSlave(srp)
heartbeat.setSlave(srp)
buildstamp.setSlave(srp)
prbsCont.setSlave(srp)
prbsLength.setSlave(srp)

# Post read transactions
fwVersion.doTransaction(False,False,1)
scratchpad.doTransaction(False,False,1000)
deviceDna.doTransaction(False,False,1000)
heartbeat.doTransaction(False,False,1000)
buildstamp.doTransaction(False,False,1000)
prbsCont.doTransaction(False,False,1000)
prbsLength.doTransaction(False,False,1000)

# Print results
print("")
print("Fw Version: 0x%08x"  % ( fwVersion.getUInt32(0)))
print("Scratchpad: 0x%08x"  % ( scratchpad.getUInt32(0)))
print("DeviceDna:  0x%016x" % ( deviceDna.getUInt64(0)))
print("Heartbeat:  0x%08x"  % ( heartbeat.getUInt32(0)))
print("BuildStamp: %s"      % ( buildstamp.getString()))
print("PrbsCont:   0x%08x"  % ( prbsCont.getUInt32(0)))
print("PrbsLength: 0x%08x"  % ( prbsLength.getUInt32(0)))
print("")

# Set scratchpad
print("Set scratchpad: 0x11111111")
scratchpad.setUInt32(0,0x11111111)
scratchpad.doTransaction(True,False,1000) # Write
scratchpad.setUInt32(0,0x00000000) # test clear
scratchpad.doTransaction(False,False,1000) # Read
print("Get Scratchpad: 0x%08x" % ( scratchpad.getUInt32(0)))
print("Set scratchpad: 0x22222222")
scratchpad.setUInt32(0,0x22222222)
scratchpad.doTransaction(True,False,1000) # Write
scratchpad.setUInt32(0,0x00000000) # test clear
scratchpad.doTransaction(False,False,1000) # Read
print("Get Scratchpad: 0x%08x" % ( scratchpad.getUInt32(0)))
print("")

# Setup PRBS
prbsCont.setUInt32(0,0x01)
prbsCont.doTransaction(True,False,1000)
prbsLength.setUInt32(0,0x7F)
prbsLength.doTransaction(True,False,1000)

# Wait a moment before connecting microblaze
print("Enabling MB Debug")
time.sleep(1)
mbCon = mbDebug()
pgpVc3.setSlave(mbCon)

print("Entering Loop")
while (True):
   time.sleep(1)
   prbsCont.setUInt32(0,0x11)
   prbsCont.doTransaction(True,False,1000)
   print("-------- PRBS Status ---------------")
   print(" Prbs: Count %i, Bytes %i, Errors %i" % (dataRx.getRxCount(),dataRx.getRxBytes(),dataRx.getRxErrors()))
   print("")
