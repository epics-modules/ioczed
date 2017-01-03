# Linux startup script

# This doesn't do any good at the moment, because the MicroZed doesn't have ntp.
epicsEnvSet("EPICS_TS_NTP_INET", "164.54.100.129")

# For devIocStats
epicsEnvSet("ENGINEER","engineer")
epicsEnvSet("LOCATION","location")
epicsEnvSet("GROUP","group")

< envPaths

epicsEnvSet("PREFIX", "zzz:")

# save_restore.cmd needs the full path to the startup directory, which
# envPaths currently does not provide
epicsEnvSet(STARTUP,$(TOP)/iocBoot/$(IOC))

# Increase size of buffer for error logging from default 1256
errlogInit(20000)

# Specify largest array CA will transport
# Note for N doubles, need N*8 bytes+some overhead
epicsEnvSet EPICS_CA_MAX_ARRAY_BYTES 4000100

################################################################################
# Tell EPICS all about the record types, device-support modules, drivers,
# etc. in the software we just loaded (zzz.munch)
dbLoadDatabase("../../dbd/ioczzzLinux.dbd")
ioczzzLinux_registerRecordDeviceDriver(pdbbase)

### save_restore setup
< save_restore.cmd

# Access Security
dbLoadRecords("$(TOP)/zzzApp/Db/Security_Control.db","P=$(PREFIX)")
asSetFilename("$(TOP)/iocBoot/accessSecurity.acf")
asSetSubstitutions("P=$(PREFIX)")

### caputRecorder

# trap listener
dbLoadRecords("$(CAPUTRECORDER)/caputRecorderApp/Db/caputPoster.db","P=$(PREFIX),N=300")
doAfterIocInit("registerCaputRecorderTrapListener('$(PREFIX)caputRecorderCommand')")

# GUI database
dbLoadRecords("$(CAPUTRECORDER)/caputRecorderApp/Db/caputRecorder.db","P=$(PREFIX),N=300")

# second copy of GUI database
#dbLoadRecords("$(CAPUTRECORDER)/caputRecorderApp/Db/caputRecorder.db","P=$(PREFIX)A:,N=300")

# custom stuff in development

#< piezoDiffractionExpt.cmd

< softGlueZynq.iocsh

# devA32ZedConfig(card,a32base,nreg,iVector,iLevel) 
#    card    = card number                          
#    a32base = base address of AXI component                 
#    nreg    = number of A32 registers on this card 
#    iVector = interrupt vector (MRD100 ONLY !!)    
#    iLevel  = interrupt level  (MRD100 ONLY !!)    

var devA32ZedDebug,1

# softGlue 300 IO component
#devA32ZedConfig(0, 0x43C00000, 140, 0, 0)
devA32ZedConfig(0, "softGlue_", 0, 140)
dbLoadTemplate("zedLOreg0.substitutions")

# softGlue reg32 component
#devA32ZedConfig(1, 0x43C20000, 64, 0, 0)
devA32ZedConfig(1, "softGlueReg32_", 0, 64)
dbLoadTemplate("zedLOreg1.substitutions")

# pixelFIFO 0
#devA32ZedConfig(2, 0x43C30000, 4, 0, 0)
devA32ZedConfig(2, "pixelFIFO_", 0, 4)
dbLoadTemplate("zedLOreg2.substitutions")

# interrupt part of softGlue component
#devA32ZedConfig(4, 0x43C10000, 5, 0, 0)
devA32ZedConfig(4, "softGlue_" 1, 5)
dbLoadTemplate("zedLOreg4.substitutions")


# dynamic clock config
#devA32ZedConfig(6, 0x43c50200, 5, 0, 0)
devA32ZedConfig(6, "clk_wiz", 0, 160)
dbLoadTemplate("zedLOreg6.substitutions")


# test MCS code (pixelFIFO 1)
#dbLoadRecords("$(SOFTGLUEZYNQ)/db/MCS.db","P=$(PREFIX),Q=MCS,N=5000")
#doAfterIocInit("seq &MCS, 'P=$(PREFIX),Q=MCS,H=SG:,N=4000,AXI_Addr=0x43C40000'")

# test DMA code
#dbLoadRecords("$(SOFTGLUEZYNQ)/db/DMA.db","P=$(PREFIX),Q=DMA,N=50000")
#doAfterIocInit("seq &DMA, 'P=$(PREFIX),Q=DMA,H=SG:,N=4000,AXI_Addr=0x40400000'")

# test pixelTrigger code (pixelFIFO 0)
#dbLoadRecords("$(SOFTGLUEZYNQ)/db/MCS.db","P=$(PREFIX),Q=MCSp,N=5000")
#doAfterIocInit("seq &MCS, 'P=$(PREFIX),Q=MCSp,H=SG:,N=4000,AXI_Addr=0x43C30000'")

# if you have hdf5 and szip, you can use this
#< areaDetector.cmd

# soft scaler for testing
< softScaler.cmd

# user-assignable ramp/tweak
dbLoadRecords("$(STD)/stdApp/Db/ramp_tweak.db","P=$(PREFIX),Q=rt1")
dbLoadRecords("$(STD)/stdApp/Db/ramp_tweak.db","P=$(PREFIX),Q=rt2")

# serial support
#< serial.cmd

# Motors
#dbLoadTemplate("basic_motor.substitutions")
#dbLoadTemplate("motor.substitutions")
#dbLoadTemplate("softMotor.substitutions")
#dbLoadTemplate("pseudoMotor.substitutions")
< motorSim.cmd
# motorUtil (allstop & alldone)
dbLoadRecords("$(MOTOR)/db/motorUtil.db", "P=$(PREFIX)")
# Run this after iocInit:
doAfterIocInit("motorUtilInit('$(PREFIX)')")

### Scan-support software
# crate-resident scan.  This executes 1D, 2D, 3D, and 4D scans, and caches
# 1D data, but it doesn't store anything to disk.  (See 'saveData' below for that.)
dbLoadRecords("$(SSCAN)/sscanApp/Db/standardScans.db","P=$(PREFIX),MAXPTS1=1000,MAXPTS2=1000,MAXPTS3=1000,MAXPTS4=1000,MAXPTSH=1000")
dbLoadRecords("$(SSCAN)/sscanApp/Db/saveData.db","P=$(PREFIX)")
# Run this after iocInit:
doAfterIocInit("saveData_Init(saveData.req, 'P=$(PREFIX)')")
dbLoadRecords("$(SSCAN)/sscanApp/Db/scanProgress.db","P=$(PREFIX)scanProgress:")
# Run this after iocInit:
doAfterIocInit("seq &scanProgress, 'S=$(PREFIX), P=$(PREFIX)scanProgress:'")

# caQtDM only
dbLoadRecords("$(SSCAN)/sscanApp/Db/caScan2D_oldData.db","P=$(PREFIX)")

# configMenu example.
#dbLoadRecords("$(AUTOSAVE)/asApp/Db/configMenu.db","P=$(PREFIX),CONFIG=scan1")
# Note that the request file MUST be named $(CONFIG)Menu.req
# If the macro CONFIGMENU is defined with any value, backup (".savB") and
# sequence files (".savN") will not be written.  We don't want these for configMenu.
# Run this after iocInit:
#doAfterIocInit("create_manual_set('scan1Menu.req','P=$(PREFIX),CONFIG=scan1,CONFIGMENU=1')")
# You could make scan configurations read-only:
#dbLoadRecords("$(AUTOSAVE)/asApp/Db/configMenu.db","P=$(PREFIX),CONFIG=scan1,ENABLE_SAVE=0")

# read-only configMenu example.  (Read-only, because we're not calling create_manual_set().)
#dbLoadRecords("$(AUTOSAVE)/asApp/Db/configMenu.db","P=$(PREFIX),CONFIG=scan2")

# A set of scan parameters for each positioner.  This is a convenience
# for the user.  It can contain an entry for each scannable thing in the
# crate.
dbLoadTemplate("scanParms.substitutions")

### Slits
dbLoadRecords("$(OPTICS)/opticsApp/Db/2slit.db","P=$(PREFIX),SLIT=Slit1V,mXp=m3,mXn=m4")
dbLoadRecords("$(OPTICS)/opticsApp/Db/2slit.db","P=$(PREFIX),SLIT=Slit1H,mXp=m5,mXn=m6")

### Optical tables
dbLoadRecords("$(OPTICS)/opticsApp/Db/table.db","P=$(PREFIX),Q=Table1,T=table1,M0X=m1,M0Y=m2,M1Y=m3,M2X=m4,M2Y=m5,M2Z=m6,GEOM=SRI")

### Monochromator support ###
# Kohzu and PSL monochromators: Bragg and theta/Y/Z motors
# standard geometry (geometry 1)
dbLoadRecords("$(OPTICS)/opticsApp/Db/kohzuSeq.db","P=$(PREFIX),M_THETA=m1,M_Y=m2,M_Z=m3,yOffLo=17.4999,yOffHi=17.5001")
# Run this after iocInit:
doAfterIocInit("seq &kohzuCtl, 'P=$(PREFIX), M_THETA=m1, M_Y=m2, M_Z=m3, GEOM=1, logfile=kohzuCtl.log'")

# Load single element Canberra AIM MCA and ICB modules
#< canberra_1.cmd

### Stuff for user programming ###
< calc.iocsh


# Slow feedback
#dbLoadTemplate "pid_slow.substitutions"
#dbLoadTemplate "async_pid_slow.substitutions"
#dbLoadTemplate "fb_epid.substitutions"

# Miscellaneous PV's, such as burtResult
dbLoadRecords("$(STD)/stdApp/Db/misc.db","P=$(PREFIX)")

# devIocStats
dbLoadRecords("$(DEVIOCSTATS)/db/iocAdminSoft.db","IOC=zzz")

### Load database records for Femto amplifiers
#dbLoadRecords("$(STD)/stdApp/Db/femto.db","P=$(PREFIX),H=fem01:,F=seq01:")
# Run this after iocInit:
#doAfterIocInit("seq femto,'name=fem1,P=$(PREFIX),H=fem01:,F=seq01:,G1=$(PREFIX)Unidig1Bo6,G2=$(PREFIX)Unidig1Bo7,G3=$(PREFIX)Unidig1Bo8,NO=$(PREFIX)Unidig1Bo10'")

### Load database records for dual PF4 filters
#dbLoadRecords("$(OPTICS)/opticsApp/Db/pf4common.db","P=$(PREFIX),H=pf4:,A=A,B=B")
#dbLoadRecords("$(OPTICS)/opticsApp/Db/pf4bank.db","P=$(PREFIX),H=pf4:,B=A")
#dbLoadRecords("$(OPTICS)/opticsApp/Db/pf4bank.db","P=$(PREFIX),H=pf4:,B=B")
# Run this after iocInit:
#doAfterIocInit("seq pf4,'name=pf1,P=$(PREFIX),H=pf4:,B=A,M=$(PREFIX)userTran10.I,B1=$(PREFIX)userTran10.A,B2=$(PREFIX)userTran10.B,B3=$(PREFIX)userTran10.C,B4=$(PREFIX)userTran10.D'")
#doAfterIocInit("seq pf4,'name=pf2,P=$(PREFIX),H=pf4:,B=B,M=$(PREFIX)userTran10.I,B1=$(PREFIX)userTran10.E,B2=$(PREFIX)userTran10.F,B3=$(PREFIX)userTran10.G,B4=$(PREFIX)userTran10.H'")

### Load database records for alternative PF4-filter support
#dbLoadTemplate "filter.substitutions"
# Run this after iocInit:
#doAfterIocInit("seq filterDrive,'NAME=filterDrive,P=$(PREFIX),R=filter:,NUM_FILTERS=16'")

### Load database record for alive heartbeating support.
# RHOST specifies the IP address that receives the heartbeats.
#dbLoadRecords("$(ALIVE)/aliveApp/Db/alive.db", "P=$(PREFIX),RHOST=164.54.100.11")

###############################################################################
iocInit
###############################################################################

# write all the PV names to a local file
dbl > dbl-all.txt

# Report  states of database CA links
dbcar(*,1)

# print the time our boot was finished
date

