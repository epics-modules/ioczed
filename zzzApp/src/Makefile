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

PROD_IOC = jak

# Note that the order in which .dbd files and libraries are added to a list
# variable is important.  For .dbd files, things must be defined before they
# are referenced by a later .dbd file.  For libraries, things must be referenced
# before they are searched for; otherwise they won't be included.


##################################################################
# First, define jak_Common_DBD and jak_Common_LIBS from the modules
# defined in RELEASE
##################################################################
DBD += jak_Common.dbd
jak_Common_DBD += base.dbd
# allow iocsh to use "system" command
jak_Common_DBD += system.dbd

# Custom stuff in development

#ifdef CAPUTLOG
#	jak_Common_DBD += caPutLog.dbd
#	jak_Common_LIBS := caPutLog $(jak_Common_LIBS)
#endif

ifdef CAPUTRECORDER
	jak_Common_DBD += caputRecorder.dbd
	jak_Common_LIBS := caputRecorder $(jak_Common_LIBS)
endif

ifdef SNCSEQ
	ifeq (seq-2-1, $(findstring seq-2-1, $(SNCSEQ)))
		jak_Common_DBD += devSequencer.dbd
		jak_Common_LIBS := seqDev seq pv $(jak_Common_LIBS)
	else ifeq (seq-2.1, $(findstring seq-2.1, $(SNCSEQ)))
		jak_Common_DBD += devSequencer.dbd
		jak_Common_LIBS := seqDev seq pv $(jak_Common_LIBS)
	else
		# seq 2.2.
		jak_Common_LIBS := seq pv $(jak_Common_LIBS)
	endif
endif

ifdef ASYN
	jak_Common_DBD += asyn.dbd drvAsynSerialPort.dbd
	jak_Common_DBD += drvAsynIPPort.dbd drvVxi11.dbd devGpib.dbd
	jak_Common_LIBS := asyn $(jak_Common_LIBS)
endif

ifdef STD
	jak_Common_DBD += stdSupport.dbd
	jak_Common_LIBS := std $(jak_Common_LIBS)
endif

ifdef CALC
	jak_Common_DBD += calcSupport.dbd
	jak_Common_LIBS := calc $(jak_Common_LIBS)
endif

ifdef BUSY
	jak_Common_DBD += busySupport.dbd
	jak_Common_LIBS := busy $(jak_Common_LIBS)
endif

ifdef SSCAN
	jak_Common_DBD += sscanSupport.dbd
	jak_Common_LIBS := sscan $(jak_Common_LIBS)
	ifdef SNCSEQ
		jak_Common_DBD += sscanProgressSupport.dbd
		jak_Common_LIBS := scanProgress $(jak_Common_LIBS)
	endif
endif

ifdef AUTOSAVE
	jak_Common_DBD += asSupport.dbd
	jak_Common_LIBS := autosave $(jak_Common_LIBS)
endif

ifdef OPTICS
	jak_Common_DBD += opticsSupport.dbd
	jak_Common_LIBS := optics $(jak_Common_LIBS)
endif

ifdef MCA
	jak_Common_DBD += mcaSupport.dbd
	jak_Common_LIBS := mca $(jak_Common_LIBS)
endif

ifdef IP
	jak_Common_DBD += ipSupport.dbd
	jak_Common_LIBS := ip $(jak_Common_LIBS)
endif

ifdef MODBUS
	jak_Common_DBD += modbusSupport.dbd
	jak_Common_LIBS := modbus $(jak_Common_LIBS)
endif

ifdef MOTOR
	jak_Common_DBD += motorSupport.dbd devAcsMotor.dbd devAerotech.dbd
	jak_Common_DBD += devAttocube.dbd devFaulhaberMotor.dbd devImsMotor.dbd
	jak_Common_DBD += devKohzuMotor.dbd devMclennanMotor.dbd devMicos.dbd
	jak_Common_DBD += devMicroMo.dbd devNewFocus.dbd devNewport.dbd
	jak_Common_DBD += devOriel.dbd devPC6K.dbd devPIJena.dbd devPIMotor.dbd
	jak_Common_DBD += devSPiiPlus.dbd devSmartMotorMotor.dbd devSoftMotor.dbd
	jak_Common_DBD += devThorLabs.dbd motorSimSupport.dbd

	jak_Common_LIBS := motor $(jak_Common_LIBS)
	jak_Common_LIBS := Acs acsTech80 Aerotech Attocube Faulhaber Ims KohzuMotor $(jak_Common_LIBS)
	jak_Common_LIBS := Mclennan Micos MicroMo NewFocus Newport Oriel PI PIJena $(jak_Common_LIBS)
	jak_Common_LIBS := Parker SmartMotor ThorLabs softMotor motorSimSupport $(jak_Common_LIBS)
endif

ifdef STREAM
	jak_Common_DBD += stream.dbd streamSynApps.dbd
	jak_Common_LIBS := stream streamSynApps $(jak_Common_LIBS)
endif

ifdef DEVIOCSTATS
  ifneq ($(OS_CLASS), WIN32)
	jak_Common_DBD += devIocStats.dbd iocAdmin.dbd
	jak_Common_LIBS := devIocStats $(jak_Common_LIBS)
  endif
endif

ifdef ETHERIP
	jak_Common_DBD += ether_ip.dbd
	jak_Common_LIBS := ether_ip $(jak_Common_LIBS)
endif

ifdef ALIVE
	jak_Common_DBD += aliveSupport.dbd
	jak_Common_LIBS := alive $(jak_Common_LIBS)
endif

ifdef DELAYGEN
	jak_Common_DBD += delaygenSupport.dbd
	jak_Common_LIBS := delaygen $(jak_Common_LIBS)
endif

ifdef VAC
	jak_Common_DBD += vacSupport.dbd
	jak_Common_LIBS := vac $(jak_Common_LIBS)
endif

jak_Common_LIBS := $(jak_Common_LIBS) $(EPICS_BASE_IOC_LIBS)

##################################################################


ifeq (win32-x86,$(findstring win32-x86, $(T_A)))

	DBD  += iocjakWin32.dbd
	iocjakWin32_DBD += jak_Common.dbd

	ifdef MCA
		# If your host supports libpcap and libnet and you want to build
		# Canberra network support uncomment the following lines
		#iocjakWin32_DBD += mcaCanberraSupport.dbd
		#jak_LIBS_WIN32 += mcaCanberra
		#jak_LIBS_WIN32 += wpcap Packet
	endif

	ifdef DXP
		# If you want to build parallel port or USB interfaces to Saturn and/or
		# xMAP DXP, then uncomment the following lines
		iocjakWin32_DBD += dxpSupport.dbd
		jak_LIBS_WIN32 += dxp handel
		jak_LIBS_WIN32 += DLPORTIO PlxApi
		jak_SYS_LIBS_WIN32 += setupapi
	endif

endif

ifeq (windows-x64,$(findstring windows-x64, $(T_A)))
	DBD  += iocjakWin64.dbd
	iocjakWin64_DBD += jak_Common.dbd

	ifdef MCA
		# If your host supports libpcap and libnet and you want to build Canberra network
		# support uncomment the following line
		iocjakWin64_DBD += mcaCanberraSupport.dbd
		jak_LIBS_WIN32 += mcaCanberra
		jak_LIBS_WIN32 += wpcap Packet
	endif

endif

ifeq ($(EPICS_HOST_ARCH), cygwin-x86)
	DBD  += iocjakCygwin.dbd
	iocjakCygwin_DBD += jak_Common.dbd

	ifdef DXP
		# If you want to build parallel port or USB interfaces to Saturn and/or
		# xMAP DXP,then uncomment the following lines
		#iocjakCygwin_DBD += dxpSupport.dbd
		#jak_LIBS_cygwin32 += dxp handel
		#jak_SYS_LIBS_cygwin32 += DLPORTIO PlxApi setupapi
	endif

	jak_SYS_LIBS_cygwin32 += $(CYGWIN_RPC_LIB)

endif

ifeq (vxWorks,$(findstring vxWorks, $(T_A)))
	DBD  += iocjakVX.dbd
	iocjakVX_DBD += jak_Common.dbd

	ifdef ALLEN_BRADLEY
		jak_LIBS_vxWorks += allenBradley
	endif

	ifdef QUADEM
		iocjakVX_DBD += drvAHxxx.dbd
		iocjakVX_DBD += drvAPS_EM.dbd
		jak_LIBS_vxWorks += quadEM
	endif

	ifdef AREA_DETECTOR
		# areaDetector provides a Makefile for things it needs and supplies
		PROD_NAME = AD
		-include $(ADCORE)/ADApp/commonDriverMakefile
		iocjakVX_DBD += ADSupport.dbd NDPluginSupport.dbd
		jak_LIBS_vxWorks += $(AD_LIBS)
		# To add the simDetector uncomment the following lines
		iocjakVX_DBD += simDetectorSupport.dbd
		jak_LIBS_vxWorks += simDetector
	endif

	ifdef VME
		iocjakVX_DBD += vmeVXSupport.dbd
		jak_LIBS_vxWorks += vme
	endif

	ifdef STD
		iocjakVX_DBD += stdVXSupport.dbd
	endif

	ifdef IP
		iocjakVX_DBD += ipVXSupport.dbd
	endif

	ifdef MCA
		iocjakVX_DBD += SIS38XXSupport.dbd
		iocjakVX_DBD += mcaCanberraSupport.dbd
		jak_LIBS_vxWorks += SIS38XX
		jak_LIBS_vxWorks += mcaCanberra
	endif

	ifdef IPUNIDIG
		iocjakVX_DBD += ipUnidigSupport.dbd
		jak_LIBS_vxWorks += ipUnidig
	endif

	ifdef DAC128V
		iocjakVX_DBD += dac128VSupport.dbd
		jak_LIBS_vxWorks += dac128V
	endif

	ifdef IP330
		iocjakVX_DBD += ip330Support.dbd
		jak_LIBS_vxWorks += ip330
	endif

	ifdef CAMAC
		iocjakVX_DBD += camacSupport.dbd
		iocjakVX_DBD += devCamacSupport.dbd
		jak_LIBS_vxWorks += camac devCamac
	endif

	ifdef ALLEN_BRADLEY
		iocjakVX_DBD += allenBradley.dbd
	endif

	ifdef LOVE
		iocjakVX_DBD += devLove.dbd
		jak_LIBS_vxWorks += love
	endif

	ifdef IPAC
		iocjakVX_DBD += tyGSOctal.dbd
		iocjakVX_DBD += drvIpac.dbd
		jak_LIBS_vxWorks += TyGSOctal Ipac
		iocjakVX_DBD += IP520.dbd
		jak_LIBS_vxWorks += IP520
	endif

	ifdef ASYN
		iocjakVX_DBD += drvGsIP488.dbd
	endif

	ifdef SOFTGLUE
		iocjakVX_DBD += softGlueSupport.dbd
		jak_LIBS_vxWorks += softGlue
	endif

	ifdef MOTOR
		iocjakVX_DBD += devDeltaTau.dbd
		iocjakVX_DBD += devOms.dbd
		jak_LIBS_vxWorks += oms DeltaTau
	endif

	ifdef DEVIOCSTATS
		iocjakVX_DBD += devVxStats.dbd
	endif
endif

ifneq (,$(findstring linux, $(T_A)))
	DBD  += iocjakLinux.dbd
	iocjakLinux_DBD += jak_Common.dbd

	ifdef SOFTGLUEZYNQ
		iocjakLinux_DBD += softGlueZynqSupport.dbd
		jak_LIBS_Linux += softGlueZynq
	endif

ifeq ($(HAVE_HDF5), YES)
ifeq ($(HAVE_SZIP), YES)

	ifdef QUADEM
		iocjakLinux_DBD += drvAHxxx.dbd
		jak_LIBS_Linux += quadEM
	endif

	ifdef AREA_DETECTOR
		# areaDetector provides a Makefile for things it needs and supplies
		PROD_NAME = AD
		-include $(ADCORE)/ADApp/commonDriverMakefile
		iocjakLinux_DBD += ADSupport.dbd NDPluginSupport.dbd
		jak_LIBS_Linux += $(AD_LIBS)
		# To add the simDetector uncomment the following lines
		iocjakLinux_DBD += simDetectorSupport.dbd
		jak_LIBS_Linux += simDetector
	endif

	ifdef DXP
		# If your host supports parallel port or USB interfaces to Saturn DXP, set
		# LINUX_USB_INSTALLED to YES in <synApps>/configure/CONFIG_SITE.
		ifeq ($(LINUX_USB_INSTALLED), YES)
			iocjakLinux_DBD += dxpSupport.dbd
			jak_LIBS_Linux  += dxp handel
			jak_SYS_LIBS_Linux += usb
		endif
	endif

endif
endif

	ifdef MCA
		# If your host supports libpcap and libnet and you want to build Canberra network
		# support, set LINUX_NET_INSTALLED to YES in <synApps>/configure/CONFIG_SITE.
		ifeq ($(LINUX_NET_INSTALLED), YES)
			iocjakLinux_DBD += mcaCanberraSupport.dbd
			jak_LIBS_Linux += mcaCanberra
			jak_SYS_LIBS_Linux += net pcap
		endif
	endif

	ifdef IPAC
		iocjakLinux_DBD += drvIpac.dbd
		jak_LIBS_Linux += Ipac
	endif

	ifdef SOFTGLUE
		iocjakLinux_DBD += softGlueSupport.dbd
		jak_LIBS_Linux += softGlue
	endif
endif

ifeq ($(OS_CLASS), Darwin)
	DBD  += iocjakDarwin.dbd
	iocjakDarwin_DBD += jak_Common.dbd
endif



# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
jak_SRCS_vxWorks  += iocjakVX_registerRecordDeviceDriver.cpp
jak_SRCS_Linux    += iocjakLinux_registerRecordDeviceDriver.cpp     jakMain.cpp
ifeq (win32-x86,$(findstring win32-x86, $(T_A)))
	jak_SRCS_WIN32    += iocjakWin32_registerRecordDeviceDriver.cpp     jakMain.cpp
endif
ifeq (windows-x64,$(findstring windows-x64, $(T_A)))
	jak_SRCS_WIN32    += iocjakWin64_registerRecordDeviceDriver.cpp     jakMain.cpp
endif
jak_SRCS_cygwin32 += iocjakCygwin_registerRecordDeviceDriver.cpp    jakMain.cpp
jak_SRCS_solaris  += iocjak_solaris_registerRecordDeviceDriver.cpp  jakMain.cpp
jak_SRCS_Darwin   += iocjakDarwin_registerRecordDeviceDriver.cpp    jakMain.cpp



#The following adds support from base/src/vxWorks
jak_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

jak_LIBS_vxWorks  += $(jak_Common_LIBS)
jak_LIBS_Linux    += $(jak_Common_LIBS)
jak_LIBS_solaris  += $(jak_Common_LIBS)
jak_LIBS_cygwin32 += $(jak_Common_LIBS)
jak_LIBS_WIN32    += $(jak_Common_LIBS)
jak_LIBS_Darwin   += $(jak_Common_LIBS)


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

$(COMMON_DIR)/iocjakWin32Include.dbd: $(COMMON_DIR)/jak_Common.dbd
$(COMMON_DIR)/iocjakWin64Include.dbd: $(COMMON_DIR)/jak_Common.dbd
$(COMMON_DIR)/iocjakCygwinInclude.dbd: $(COMMON_DIR)/jak_Common.dbd
$(COMMON_DIR)/iocjakVXInclude.dbd: $(COMMON_DIR)/jak_Common.dbd
$(COMMON_DIR)/iocjakLinuxInclude.dbd: $(COMMON_DIR)/jak_Common.dbd
$(COMMON_DIR)/iocjakDarwinInclude.dbd: $(COMMON_DIR)/jak_Common.dbd

$(COMMON_DIR)/iocjakWin32.dbd: $(COMMON_DIR)/jak_Common.dbd
$(COMMON_DIR)/iocjakWin64.dbd: $(COMMON_DIR)/jak_Common.dbd
$(COMMON_DIR)/iocjakCygwin.dbd: $(COMMON_DIR)/jak_Common.dbd
$(COMMON_DIR)/iocjakVX.dbd: $(COMMON_DIR)/jak_Common.dbd
$(COMMON_DIR)/iocjakLinux.dbd: $(COMMON_DIR)/jak_Common.dbd
$(COMMON_DIR)/iocjakDarwin.dbd: $(COMMON_DIR)/jak_Common.dbd

iocjakWin32.dbd$(DEP): $(COMMON_DIR)/jak_Common.dbd
iocjakWin64.dbd$(DEP): $(COMMON_DIR)/jak_Common.dbd
iocjakCygwin.dbd$(DEP): $(COMMON_DIR)/jak_Common.dbd
iocjakVX.dbd$(DEP): $(COMMON_DIR)/jak_Common.dbd
iocjakLinux.dbd$(DEP): $(COMMON_DIR)/jak_Common.dbd
iocjakDarwin.dbd$(DEP): $(COMMON_DIR)/jak_Common.dbd
