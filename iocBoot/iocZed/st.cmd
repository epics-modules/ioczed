# Linux startup script

# For devIocStats
epicsEnvSet("ENGINEER","engineer")
epicsEnvSet("LOCATION","location")
epicsEnvSet("GROUP","group")

< envPaths

# save_restore.cmd needs the full path to the startup directory, which
# envPaths currently does not provide
epicsEnvSet(STARTUP,$(TOP)/iocBoot/$(IOC))

# Increase size of buffer for error logging from default 1256
errlogInit(20000)

# Specify largest array CA will transport
# Note for N doubles, need N*8 bytes+some overhead
epicsEnvSet EPICS_CA_MAX_ARRAY_BYTES 200100

################################################################################
# Tell EPICS all about the record types, device-support modules, drivers,
# etc. in the software we just loaded (zzz.munch)
dbLoadDatabase("../../dbd/ioczzzLinux.dbd")
ioczzzLinux_registerRecordDeviceDriver(pdbbase)

### save_restore setup
< save_restore.cmd

# Access Security
dbLoadRecords("$(TOP)/zzzApp/Db/Security_Control.db","P=zzz:")
asSetFilename("$(TOP)/iocBoot/accessSecurity.acf")
asSetSubstitutions("P=zzz:")

### caputRecorder

# trap listener
dbLoadRecords("$(CAPUTRECORDER)/caputRecorderApp/Db/caputPoster.db","P=zzz:,N=300")
doAfterIocInit("registerCaputRecorderTrapListener('zzz:caputRecorderCommand')")

# GUI database
dbLoadRecords("$(CAPUTRECORDER)/caputRecorderApp/Db/caputRecorder.db","P=zzz:,N=300")

# second copy of GUI database
#dbLoadRecords("$(CAPUTRECORDER)/caputRecorderApp/Db/caputRecorder.db","P=zzzA:,N=300")

# custom stuff in development
# devA32ZedConfig(card,a32base,nreg,iVector,iLevel) 
#    card    = card number                          
#    a32base = base address of card                 
#    nreg    = number of A32 registers on this card 
#    iVector = interrupt vector (MRD100 ONLY !!)    
#    iLevel  = interrupt level  (MRD100 ONLY !!)    
devA32ZedConfig(0, 0x43C00000, 140, 0, 0)
devA32ZedConfig(1, 0x43C20000, 64, 0, 0)
devA32ZedConfig(2, 0x43C30000, 4, 0, 0)
devA32ZedConfig(3, 0x43C40000, 4, 0, 0)
#dbLoadTemplate("zedLOreg0.substitutions")
dbLoadTemplate("zedLOreg1.substitutions")
dbLoadTemplate("zedLOreg2.substitutions")
dbLoadTemplate("zedLOreg3.substitutions")

# test MCS code
dbLoadRecords("$(SOFTGLUEZYNQ)/db/MCS.db","P=zzz:,Q=MCS,N=50000")
doAfterIocInit("seq &MCS, 'P=zzz:,Q=MCS,H=SG:,N=4000'")



< softGlueZynq.iocsh

# if you have hdf5 and szip, you can use this
#< areaDetector.cmd

# soft scaler for testing
< softScaler.cmd

# user-assignable ramp/tweak
dbLoadRecords("$(STD)/stdApp/Db/ramp_tweak.db","P=zzz:,Q=rt1")

# serial support
#< serial.cmd

# Motors
#dbLoadTemplate("basic_motor.substitutions")
#dbLoadTemplate("motor.substitutions")
#dbLoadTemplate("softMotor.substitutions")
#dbLoadTemplate("pseudoMotor.substitutions")
#< motorSim.cmd
# motorUtil (allstop & alldone)
#dbLoadRecords("$(MOTOR)/db/motorUtil.db", "P=zzz:")
# Run this after iocInit:
#doAfterIocInit("motorUtilInit('zzz:')")

### Scan-support software
# crate-resident scan.  This executes 1D, 2D, 3D, and 4D scans, and caches
# 1D data, but it doesn't store anything to disk.  (See 'saveData' below for that.)
dbLoadRecords("$(SSCAN)/sscanApp/Db/standardScans.db","P=zzz:,MAXPTS1=1000,MAXPTS2=1000,MAXPTS3=1000,MAXPTS4=1000,MAXPTSH=1000")
dbLoadRecords("$(SSCAN)/sscanApp/Db/saveData.db","P=zzz:")
# Run this after iocInit:
doAfterIocInit("saveData_Init(saveData.req, 'P=zzz:')")
dbLoadRecords("$(SSCAN)/sscanApp/Db/scanProgress.db","P=zzz:scanProgress:")
# Run this after iocInit:
doAfterIocInit("seq &scanProgress, 'S=zzz:, P=zzz:scanProgress:'")

# configMenu example.
#dbLoadRecords("$(AUTOSAVE)/asApp/Db/configMenu.db","P=zzz:,CONFIG=scan1")
# Note that the request file MUST be named $(CONFIG)Menu.req
# If the macro CONFIGMENU is defined with any value, backup (".savB") and
# sequence files (".savN") will not be written.  We don't want these for configMenu.
# Run this after iocInit:
#doAfterIocInit("create_manual_set('scan1Menu.req','P=zzz:,CONFIG=scan1,CONFIGMENU=1')")
# You could make scan configurations read-only:
#dbLoadRecords("$(AUTOSAVE)/asApp/Db/configMenu.db","P=zzz:,CONFIG=scan1,ENABLE_SAVE=0")

# read-only configMenu example.  (Read-only, because we're not calling create_manual_set().)
#dbLoadRecords("$(AUTOSAVE)/asApp/Db/configMenu.db","P=zzz:,CONFIG=scan2")

# A set of scan parameters for each positioner.  This is a convenience
# for the user.  It can contain an entry for each scannable thing in the
# crate.
dbLoadTemplate("scanParms.substitutions")

### Stuff for user programming ###
< calc.iocsh


# Slow feedback
#dbLoadTemplate "pid_slow.substitutions"
#dbLoadTemplate "async_pid_slow.substitutions"
#dbLoadTemplate "fb_epid.substitutions"

# Miscellaneous PV's, such as burtResult
dbLoadRecords("$(STD)/stdApp/Db/misc.db","P=zzz:")

# devIocStats
dbLoadRecords("$(DEVIOCSTATS)/db/iocAdminSoft.db","IOC=zzz")


### Load database record for alive heartbeating support.
# RHOST specifies the IP address that receives the heartbeats.
#dbLoadRecords("$(ALIVE)/aliveApp/Db/alive.db", "P=zzz:,RHOST=164.54.100.11")

###############################################################################
iocInit
###############################################################################

# write all the PV names to a local file
dbl > dbl-all.txt

# Report  states of database CA links
dbcar(*,1)

# print the time our boot was finished
date

