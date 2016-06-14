TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

# For Kate Feng's implementation of devScaler.c, which uses Till
# Straumann's basicIoOps.h to bridge any mismatch between endianness
# of Joerger scaler hardware and endianness of processor.
#USR_CFLAGS += -DHAS_IOOPS_H

#=============================
# build an ioc application

PROD_IOC = zzz

# Note that the order in which .dbd files and libraries are added to a list
# variable is important.  For .dbd files, things must be defined before they
# are referenced by a later .dbd file.  For libraries, things must be referenced
# before they are searched for; otherwise they won't be included.


##################################################################
# First, define zzz_Common_DBD and zzz_Common_LIBS from the modules
# defined in RELEASE
##################################################################
DBD += zzz_Common.dbd
zzz_Common_DBD += base.dbd
# allow iocsh to use "system" command
zzz_Common_DBD += system.dbd

# Custom stuff in development

#ifdef CAPUTLOG
#	zzz_Common_DBD += caPutLog.dbd
#	zzz_Common_LIBS := caPutLog $(zzz_Common_LIBS)
#endif

ifdef CAPUTRECORDER
	zzz_Common_DBD += caputRecorder.dbd
	zzz_Common_LIBS := caputRecorder $(zzz_Common_LIBS)
endif

ifdef SNCSEQ
	ifeq (seq-2-1, $(findstring seq-2-1, $(SNCSEQ)))
		zzz_Common_DBD += devSequencer.dbd
		zzz_Common_LIBS := seqDev seq pv $(zzz_Common_LIBS)
	else ifeq (seq-2.1, $(findstring seq-2.1, $(SNCSEQ)))
		zzz_Common_DBD += devSequencer.dbd
		zzz_Common_LIBS := seqDev seq pv $(zzz_Common_LIBS)
	else
		# seq 2.2.
		zzz_Common_LIBS := seq pv $(zzz_Common_LIBS)
	endif
endif

ifdef ASYN
	zzz_Common_DBD += asyn.dbd drvAsynSerialPort.dbd
	zzz_Common_DBD += drvAsynIPPort.dbd drvVxi11.dbd devGpib.dbd
	zzz_Common_LIBS := asyn $(zzz_Common_LIBS)
endif

ifdef STD
	zzz_Common_DBD += stdSupport.dbd
	zzz_Common_LIBS := std $(zzz_Common_LIBS)
endif

ifdef CALC
	zzz_Common_DBD += calcSupport.dbd
	zzz_Common_LIBS := calc $(zzz_Common_LIBS)
endif

ifdef BUSY
	zzz_Common_DBD += busySupport.dbd
	zzz_Common_LIBS := busy $(zzz_Common_LIBS)
endif

ifdef SSCAN
	zzz_Common_DBD += sscanSupport.dbd
	zzz_Common_LIBS := sscan $(zzz_Common_LIBS)
	ifdef SNCSEQ
		zzz_Common_DBD += sscanProgressSupport.dbd
		zzz_Common_LIBS := scanProgress $(zzz_Common_LIBS)
	endif
endif

ifdef AUTOSAVE
	zzz_Common_DBD += asSupport.dbd
	zzz_Common_LIBS := autosave $(zzz_Common_LIBS)
endif

ifdef OPTICS
	zzz_Common_DBD += opticsSupport.dbd
	zzz_Common_LIBS := optics $(zzz_Common_LIBS)
endif

ifdef MCA
	zzz_Common_DBD += mcaSupport.dbd
	zzz_Common_LIBS := mca $(zzz_Common_LIBS)
endif

ifdef IP
	zzz_Common_DBD += ipSupport.dbd
	zzz_Common_LIBS := ip $(zzz_Common_LIBS)
endif

ifdef MODBUS
	zzz_Common_DBD += modbusSupport.dbd
	zzz_Common_LIBS := modbus $(zzz_Common_LIBS)
endif

ifdef MOTOR
	zzz_Common_DBD += motorSupport.dbd devAcsMotor.dbd devAerotech.dbd
	zzz_Common_DBD += devAttocube.dbd devFaulhaberMotor.dbd devImsMotor.dbd
	zzz_Common_DBD += devKohzuMotor.dbd devMclennanMotor.dbd devMicos.dbd
	zzz_Common_DBD += devMicroMo.dbd devNewFocus.dbd devNewport.dbd
	zzz_Common_DBD += devOriel.dbd devPC6K.dbd devPIJena.dbd devPIMotor.dbd
	zzz_Common_DBD += devSPiiPlus.dbd devSmartMotorMotor.dbd devSoftMotor.dbd
	zzz_Common_DBD += devThorLabs.dbd motorSimSupport.dbd

	zzz_Common_LIBS := motor $(zzz_Common_LIBS)
	zzz_Common_LIBS := Acs acsTech80 Aerotech Attocube Faulhaber Ims KohzuMotor $(zzz_Common_LIBS)
	zzz_Common_LIBS := Mclennan Micos MicroMo NewFocus Newport Oriel PI PIJena $(zzz_Common_LIBS)
	zzz_Common_LIBS := Parker SmartMotor ThorLabs softMotor motorSimSupport $(zzz_Common_LIBS)
endif

ifdef STREAM
	zzz_Common_DBD += stream.dbd streamSynApps.dbd
	zzz_Common_LIBS := stream streamSynApps $(zzz_Common_LIBS)
endif

ifdef DEVIOCSTATS
  ifneq ($(OS_CLASS), WIN32)
	zzz_Common_DBD += devIocStats.dbd iocAdmin.dbd
	zzz_Common_LIBS := devIocStats $(zzz_Common_LIBS)
  endif
endif

ifdef ETHERIP
	zzz_Common_DBD += ether_ip.dbd
	zzz_Common_LIBS := ether_ip $(zzz_Common_LIBS)
endif

ifdef ALIVE
	zzz_Common_DBD += aliveSupport.dbd
	zzz_Common_LIBS := alive $(zzz_Common_LIBS)
endif

ifdef DELAYGEN
	zzz_Common_DBD += delaygenSupport.dbd
	zzz_Common_LIBS := delaygen $(zzz_Common_LIBS)
endif

ifdef VAC
	zzz_Common_DBD += vacSupport.dbd
	zzz_Common_LIBS := vac $(zzz_Common_LIBS)
endif

zzz_Common_LIBS := $(zzz_Common_LIBS) $(EPICS_BASE_IOC_LIBS)

##################################################################


ifeq (win32-x86,$(findstring win32-x86, $(T_A)))

	DBD  += ioczzzWin32.dbd
	ioczzzWin32_DBD += zzz_Common.dbd

	ifdef MCA
		# If your host supports libpcap and libnet and you want to build
		# Canberra network support uncomment the following lines
		#ioczzzWin32_DBD += mcaCanberraSupport.dbd
		#zzz_LIBS_WIN32 += mcaCanberra
		#zzz_LIBS_WIN32 += wpcap Packet
	endif

	ifdef DXP
		# If you want to build parallel port or USB interfaces to Saturn and/or
		# xMAP DXP, then uncomment the following lines
		ioczzzWin32_DBD += dxpSupport.dbd
		zzz_LIBS_WIN32 += dxp handel
		zzz_LIBS_WIN32 += DLPORTIO PlxApi
		zzz_SYS_LIBS_WIN32 += setupapi
	endif

endif

ifeq (windows-x64,$(findstring windows-x64, $(T_A)))
	DBD  += ioczzzWin64.dbd
	ioczzzWin64_DBD += zzz_Common.dbd

	ifdef MCA
		# If your host supports libpcap and libnet and you want to build Canberra network
		# support uncomment the following line
		ioczzzWin64_DBD += mcaCanberraSupport.dbd
		zzz_LIBS_WIN32 += mcaCanberra
		zzz_LIBS_WIN32 += wpcap Packet
	endif

endif

ifeq ($(EPICS_HOST_ARCH), cygwin-x86)
	DBD  += ioczzzCygwin.dbd
	ioczzzCygwin_DBD += zzz_Common.dbd

	ifdef DXP
		# If you want to build parallel port or USB interfaces to Saturn and/or
		# xMAP DXP,then uncomment the following lines
		#ioczzzCygwin_DBD += dxpSupport.dbd
		#zzz_LIBS_cygwin32 += dxp handel
		#zzz_SYS_LIBS_cygwin32 += DLPORTIO PlxApi setupapi
	endif

	zzz_SYS_LIBS_cygwin32 += $(CYGWIN_RPC_LIB)

endif

ifeq (vxWorks,$(findstring vxWorks, $(T_A)))
	DBD  += ioczzzVX.dbd
	ioczzzVX_DBD += zzz_Common.dbd

	ifdef ALLEN_BRADLEY
		zzz_LIBS_vxWorks += allenBradley
	endif

	ifdef QUADEM
		ioczzzVX_DBD += drvAHxxx.dbd
		ioczzzVX_DBD += drvAPS_EM.dbd
		zzz_LIBS_vxWorks += quadEM
	endif

	ifdef AREA_DETECTOR
		# areaDetector provides a Makefile for things it needs and supplies
		PROD_NAME = AD
		-include $(ADCORE)/ADApp/commonDriverMakefile
		ioczzzVX_DBD += ADSupport.dbd NDPluginSupport.dbd
		zzz_LIBS_vxWorks += $(AD_LIBS)
		# To add the simDetector uncomment the following lines
		ioczzzVX_DBD += simDetectorSupport.dbd
		zzz_LIBS_vxWorks += simDetector
	endif

	ifdef VME
		ioczzzVX_DBD += vmeVXSupport.dbd
		zzz_LIBS_vxWorks += vme
	endif

	ifdef STD
		ioczzzVX_DBD += stdVXSupport.dbd
	endif

	ifdef IP
		ioczzzVX_DBD += ipVXSupport.dbd
	endif

	ifdef MCA
		ioczzzVX_DBD += SIS38XXSupport.dbd
		ioczzzVX_DBD += mcaCanberraSupport.dbd
		zzz_LIBS_vxWorks += SIS38XX
		zzz_LIBS_vxWorks += mcaCanberra
	endif

	ifdef IPUNIDIG
		ioczzzVX_DBD += ipUnidigSupport.dbd
		zzz_LIBS_vxWorks += ipUnidig
	endif

	ifdef DAC128V
		ioczzzVX_DBD += dac128VSupport.dbd
		zzz_LIBS_vxWorks += dac128V
	endif

	ifdef IP330
		ioczzzVX_DBD += ip330Support.dbd
		zzz_LIBS_vxWorks += ip330
	endif

	ifdef CAMAC
		ioczzzVX_DBD += camacSupport.dbd
		ioczzzVX_DBD += devCamacSupport.dbd
		zzz_LIBS_vxWorks += camac devCamac
	endif

	ifdef ALLEN_BRADLEY
		ioczzzVX_DBD += allenBradley.dbd
	endif

	ifdef LOVE
		ioczzzVX_DBD += devLove.dbd
		zzz_LIBS_vxWorks += love
	endif

	ifdef IPAC
		ioczzzVX_DBD += tyGSOctal.dbd
		ioczzzVX_DBD += drvIpac.dbd
		zzz_LIBS_vxWorks += TyGSOctal Ipac
		ioczzzVX_DBD += IP520.dbd
		zzz_LIBS_vxWorks += IP520
	endif

	ifdef ASYN
		ioczzzVX_DBD += drvGsIP488.dbd
	endif

	ifdef SOFTGLUE
		ioczzzVX_DBD += softGlueSupport.dbd
		zzz_LIBS_vxWorks += softGlue
	endif

	ifdef MOTOR
		ioczzzVX_DBD += devDeltaTau.dbd
		ioczzzVX_DBD += devOms.dbd
		zzz_LIBS_vxWorks += oms DeltaTau
	endif

	ifdef DEVIOCSTATS
		ioczzzVX_DBD += devVxStats.dbd
	endif
endif

ifneq (,$(findstring linux, $(T_A)))
	DBD  += ioczzzLinux.dbd
	ioczzzLinux_DBD += zzz_Common.dbd

	# for microzed
	ioczzzLinux_DBD += devA32Zed.dbd
	zzz_SRCS_Linux += devA32Zed.c

	ifdef SOFTGLUEZYNQ
		ioczzzLinux_DBD += softGlueZynqSupport.dbd
		zzz_LIBS_Linux += softGlueZynq
	endif

ifeq ($(HAVE_HDF5), YES)
ifeq ($(HAVE_SZIP), YES)

	ifdef QUADEM
		ioczzzLinux_DBD += drvAHxxx.dbd
		zzz_LIBS_Linux += quadEM
	endif

	ifdef AREA_DETECTOR
		# areaDetector provides a Makefile for things it needs and supplies
		PROD_NAME = AD
		-include $(ADCORE)/ADApp/commonDriverMakefile
		ioczzzLinux_DBD += ADSupport.dbd NDPluginSupport.dbd
		zzz_LIBS_Linux += $(AD_LIBS)
		# To add the simDetector uncomment the following lines
		ioczzzLinux_DBD += simDetectorSupport.dbd
		zzz_LIBS_Linux += simDetector
	endif

	ifdef DXP
		# If your host supports parallel port or USB interfaces to Saturn DXP, set
		# LINUX_USB_INSTALLED to YES in <synApps>/configure/CONFIG_SITE.
		ifeq ($(LINUX_USB_INSTALLED), YES)
			ioczzzLinux_DBD += dxpSupport.dbd
			zzz_LIBS_Linux  += dxp handel
			zzz_SYS_LIBS_Linux += usb
		endif
	endif

endif
endif

	ifdef MCA
		# If your host supports libpcap and libnet and you want to build Canberra network
		# support, set LINUX_NET_INSTALLED to YES in <synApps>/configure/CONFIG_SITE.
		ifeq ($(LINUX_NET_INSTALLED), YES)
			ioczzzLinux_DBD += mcaCanberraSupport.dbd
			zzz_LIBS_Linux += mcaCanberra
			zzz_SYS_LIBS_Linux += net pcap
		endif
	endif

	ifdef IPAC
		ioczzzLinux_DBD += drvIpac.dbd
		zzz_LIBS_Linux += Ipac
	endif

	ifdef SOFTGLUE
		ioczzzLinux_DBD += softGlueSupport.dbd
		zzz_LIBS_Linux += softGlue
	endif
endif

ifeq ($(OS_CLASS), Darwin)
	DBD  += ioczzzDarwin.dbd
	ioczzzDarwin_DBD += zzz_Common.dbd
endif



# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
zzz_SRCS_vxWorks  += ioczzzVX_registerRecordDeviceDriver.cpp
zzz_SRCS_Linux    += ioczzzLinux_registerRecordDeviceDriver.cpp     zzzMain.cpp
ifeq (win32-x86,$(findstring win32-x86, $(T_A)))
	zzz_SRCS_WIN32    += ioczzzWin32_registerRecordDeviceDriver.cpp     zzzMain.cpp
endif
ifeq (windows-x64,$(findstring windows-x64, $(T_A)))
	zzz_SRCS_WIN32    += ioczzzWin64_registerRecordDeviceDriver.cpp     zzzMain.cpp
endif
zzz_SRCS_cygwin32 += ioczzzCygwin_registerRecordDeviceDriver.cpp    zzzMain.cpp
zzz_SRCS_solaris  += ioczzz_solaris_registerRecordDeviceDriver.cpp  zzzMain.cpp
zzz_SRCS_Darwin   += ioczzzDarwin_registerRecordDeviceDriver.cpp    zzzMain.cpp



#The following adds support from base/src/vxWorks
zzz_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

zzz_LIBS_vxWorks  += $(zzz_Common_LIBS)
zzz_LIBS_Linux    += $(zzz_Common_LIBS)
zzz_LIBS_solaris  += $(zzz_Common_LIBS)
zzz_LIBS_cygwin32 += $(zzz_Common_LIBS)
zzz_LIBS_WIN32    += $(zzz_Common_LIBS)
zzz_LIBS_Darwin   += $(zzz_Common_LIBS)


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

$(COMMON_DIR)/ioczzzWin32Include.dbd: $(COMMON_DIR)/zzz_Common.dbd
$(COMMON_DIR)/ioczzzWin64Include.dbd: $(COMMON_DIR)/zzz_Common.dbd
$(COMMON_DIR)/ioczzzCygwinInclude.dbd: $(COMMON_DIR)/zzz_Common.dbd
$(COMMON_DIR)/ioczzzVXInclude.dbd: $(COMMON_DIR)/zzz_Common.dbd
$(COMMON_DIR)/ioczzzLinuxInclude.dbd: $(COMMON_DIR)/zzz_Common.dbd
$(COMMON_DIR)/ioczzzDarwinInclude.dbd: $(COMMON_DIR)/zzz_Common.dbd

$(COMMON_DIR)/ioczzzWin32.dbd: $(COMMON_DIR)/zzz_Common.dbd
$(COMMON_DIR)/ioczzzWin64.dbd: $(COMMON_DIR)/zzz_Common.dbd
$(COMMON_DIR)/ioczzzCygwin.dbd: $(COMMON_DIR)/zzz_Common.dbd
$(COMMON_DIR)/ioczzzVX.dbd: $(COMMON_DIR)/zzz_Common.dbd
$(COMMON_DIR)/ioczzzLinux.dbd: $(COMMON_DIR)/zzz_Common.dbd
$(COMMON_DIR)/ioczzzDarwin.dbd: $(COMMON_DIR)/zzz_Common.dbd

ioczzzWin32.dbd$(DEP): $(COMMON_DIR)/zzz_Common.dbd
ioczzzWin64.dbd$(DEP): $(COMMON_DIR)/zzz_Common.dbd
ioczzzCygwin.dbd$(DEP): $(COMMON_DIR)/zzz_Common.dbd
ioczzzVX.dbd$(DEP): $(COMMON_DIR)/zzz_Common.dbd
ioczzzLinux.dbd$(DEP): $(COMMON_DIR)/zzz_Common.dbd
ioczzzDarwin.dbd$(DEP): $(COMMON_DIR)/zzz_Common.dbd
