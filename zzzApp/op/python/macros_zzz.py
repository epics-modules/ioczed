#!/bin/env python

# examples of recorded macros, and recorded macros that have been edited
# to use arguments

import os
import time
import epics

# The function "_abort" is special: it's used by caputRecorder.py to abort an
# executing macro
def _abort(prefix):
	print "%s.py: _abort() prefix=%s" % (__name__, prefix)
	epics.caput(prefix+"AbortScans", "1")
	epics.caput(prefix+"allstop", "stop")
	epics.caput(prefix+"scaler1.CNT", "Done")

def motorscan(motor="m1", start=0, end=1, npts=11):
	epics.caput("zzz:scan1.P1PV",("zzz:%s.VAL" % motor), wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.P1SP",start, wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.P1EP",end, wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.NPTS",npts, wait=True, timeout=1000000.0)
	# example of time delay
	time.sleep(.1)
	epics.caput("zzz:scan1.EXSC","1", wait=True, timeout=1000000.0)

def flyscan(motor='zzz:m1',start=0.0,end=3.14,npts=100,dwell=.2,scaler='zzz:scaler1'):
	recordDate = "Sun Mar  1 21:28:45 2015"
	# save prev speed
	oldSpeed = epics.caget(motor+".VELO")
	maxSpeed = epics.caget(motor+".VMAX")
	baseSpeed = epics.caget(motor+".VBAS")
	accelTime = epics.caget(motor+".ACCL")
	if maxSpeed == 0: maxSpeed = oldSpeed

	# send motor to start at normal speed
	epics.caput(motor+".VAL",start, wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.R1PV",motor+".RBV", wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.P1PV",motor+".VAL", wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.P1SP",start, wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.P1EP",end, wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.NPTS",npts, wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.T1PV",scaler+".CNT", wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.D01PV",scaler+".S1", wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.D02PV",scaler+".S2", wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.P1SM","FLY", wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.P1AR","ABSOLUTE", wait=True, timeout=1000000.0)
	epics.caput(scaler+".TP",dwell, wait=True, timeout=1000000.0)
	#epics.caput("zzz:scan1.PASM","STAY", wait=True, timeout=1000000.0)

	# calc speed including accelTime and baseSpeed
	totalTime = npts*dwell
	totalDist = abs(end-start)
	if totalTime > 2*accelTime:
		speed = (totalDist - accelTime*baseSpeed) / (totalTime - accelTime)
	else:
		speed = 2*totalDist/totalTime - baseSpeed

	if speed <= baseSpeed:
		print "desired speed=%f less than base speed %f" % (speed, baseSpeed)
		speed = baseSpeed+.01
	if speed > maxSpeed:
		print "desired speed=%f greater than max speed %f" %(speed, maxSpeed)
		speed = maxSpeed

	#print "speed=%f" % speed
	epics.caput(motor+".VELO",speed, wait=True, timeout=1000000.0)
	epics.caput("zzz:scan1.EXSC","1", wait=True, timeout=1000000.0)
	epics.caput(motor+".VELO",oldSpeed, wait=True, timeout=1000000.0)


def copyMotorSettings(fromMotor="zzz:m1", toMotor="zzz:m2"):
	motorFields = [".MRES", ".ERES", ".RRES", ".SREV", ".DIR", ".DHLM", ".DLLM", ".TWV",  
	".VBAS", ".VELO", ".ACCL", ".BDST", ".BVEL", ".BACC", ".RDBD", ".DESC",
	".EGU", ".RTRY", ".UEIP", ".URIP", ".DLY", ".RDBL", ".PREC", ".DISA",
	".DISP", ".FOFF", ".OFF", ".FRAC", ".OMSL", ".JVEL", ".JAR", ".VMAX",
	".PCOF", ".ICOF", ".DCOF", ".HVEL", ".NTM", ".NTMF", ".INIT", ".PREM",
	".POST", ".FLNK", ".RMOD"]
	for field in motorFields:
		val = epics.caget(fromMotor+field)
		epics.caput(toMotor+field, val)



def checkResult(pv, desiredValue, sleep):
	if (sleep > 0):
		time.sleep(sleep) # wait for softGlue's read update
	r = epics.caget(pv)
	if (r != desiredValue):
		return(1)
	return(0)

andTable = {(0,0):0, (1,0):0, (0,1):0, (1,1):1}
orTable  = {(0,0):0, (1,0):1, (0,1):1, (1,1):1}
xorTable = {(0,0):0, (1,0):1, (0,1):1, (1,1):0}
#mux2Table = {(0,0,0):0, (0,0,1):0, (0,1,0):0, (0,1,1):1, (1,0,0):1, (1,0,1):0, (1,1,0):1, (1,1,1):1}
def truth(gate, inputs):
	if gate == "AND":
		return(andTable[inputs])
	elif gate == "OR":
		return(orTable[inputs])
	elif gate == "XOR":
		return(xorTable[inputs])
	else:
		print "truth: unrecognized gate ", gate
		return(0)

def testGate(prefix="zzz:", H="softGlue:", gate="AND", N="1", sleep=0.2):
	recordDate = "Thu May 19 12:52:55 2016"
	base = prefix + H + gate + "-" + N + "_"
	errors = 0
	epics.caput("zzz:caputRecorderComment.VAL","testing...", wait=True, timeout=1000000.0)
	epics.caput(base+"IN1_Signal","1", wait=True, timeout=1000000.0)
	errors += checkResult(base+"IN1_BI", 1, sleep)
	epics.caput(base+"IN2_Signal","1", wait=True, timeout=1000000.0)
	errors += checkResult(base+"IN2_BI", 1, sleep)
	q = truth(gate,(1,1))
	errors += checkResult(base+"OUT_BI", q, sleep)
	epics.caput(base+"IN1_Signal","0", wait=True, timeout=1000000.0)
	q = truth(gate,(0,1))
	errors += checkResult(base+"OUT_BI", q, sleep)
	epics.caput(base+"IN2_Signal","0", wait=True, timeout=1000000.0)
	q = truth(gate,(0,0))
	errors += checkResult(base+"OUT_BI", q, sleep)
	epics.caput(base+"IN1_Signal","1", wait=True, timeout=1000000.0)
	q = truth(gate,(1,0))
	errors += checkResult(base+"OUT_BI", q, sleep)
	msg = "%d errors" % errors
	epics.caput("zzz:caputRecorderComment.VAL",msg, wait=True, timeout=1000000.0)
	#print errors, " errors"

def _selectTest(name='noTest'):
	epics.caput("zzz:SGMenu:name", "clear", wait=True, timeout=1000000.0)
	epics.caput("zzz:SGMenu:name", name, wait=True, timeout=1000000.0)

def _encoderTest():
	epics.caput("zzz:SG:sseq1.PROC","0", wait=True, timeout=1000000.0)
	time.sleep(.2)
	q = epics.caget("zzz:SG:UpCntr-1_COUNTS")
	ck = epics.caget("zzz:SG:UpCntr-2_COUNTS")
	divisor = epics.caget("zzz:SG:UpDnCntr-1_PRESET")
	if q > 0:
		err = 1 - (1.*ck/q)/divisor
	else :
		err = 1.e6

	passed = abs(err) < 1.e-6
	if passed:
		print "encoderTest: Passed"
	else:
		print "encoderTest: Failed: fractional err=", err
	return passed


def _dnCntrTest():
	epics.caput("zzz:SG:sseq1.PROC","0", wait=True, timeout=1000000.0)
	time.sleep(1.5)
	divBy = []
	divBy.append(epics.caget("zzz:SG:DnCntr-1_PRESET"))
	divBy.append(epics.caget("zzz:SG:DnCntr-2_PRESET"))
	divBy.append(epics.caget("zzz:SG:DnCntr-3_PRESET"))
	divBy.append(epics.caget("zzz:SG:DnCntr-4_PRESET"))
	cnt = []
	cnt.append(epics.caget("zzz:SG:UpCntr-1_COUNTS"))
	cnt.append(epics.caget("zzz:SG:UpCntr-2_COUNTS"))
	cnt.append(epics.caget("zzz:SG:UpCntr-3_COUNTS"))
	cnt.append(epics.caget("zzz:SG:UpCntr-4_COUNTS"))
	cntClock = epics.caget("zzz:SG:UpDnCntr-1_COUNTS")

	success = True
	diff = -1
	for i in range(4):
		d = abs(cntClock/divBy[i] - cnt[i])
		if d > 1:
			success = 0
			if d > diff:
				diff = d

	if success:
		print "dnCntrTest: Passed"
	else:
		print "dnCntrTest: Failed: max diff=", diff
		print "cntClock=", cntClock, "cnt=", cnt, "divBy=", divBy
	return success

def _UpDnCntrTest():
	epics.caput("zzz:SG:sseq1.PROC","0", wait=True, timeout=1000000.0)
	time.sleep(1.5)
	divBy = []
	divBy.append(epics.caget("zzz:SG:UpDnCntr-1_PRESET"))
	divBy.append(epics.caget("zzz:SG:UpDnCntr-2_PRESET"))
	divBy.append(epics.caget("zzz:SG:UpDnCntr-3_PRESET"))
	divBy.append(epics.caget("zzz:SG:UpDnCntr-4_PRESET"))
	cnt = []
	cnt.append(epics.caget("zzz:SG:UpCntr-1_COUNTS"))
	cnt.append(epics.caget("zzz:SG:UpCntr-2_COUNTS"))
	cnt.append(epics.caget("zzz:SG:UpCntr-3_COUNTS"))
	cnt.append(epics.caget("zzz:SG:UpCntr-4_COUNTS"))
	#cntClock = epics.caget("zzz:SG:DnCntr-1_COUNTS")  # doesn't exist yet
	cts = epics.caget("zzz:reg1_RB8")
	preset = epics.caget("zzz:SG:DnCntr-1_PRESET")
	cntClock = preset - cts
	success = True
	diff = -1
	for i in range(4):
		d = abs(cntClock/divBy[i] - cnt[i])
		if d > 1:
			success = 0
			if d > diff:
				diff = d

	if success:
		print "UpDnCntrTest: Passed"
	else:
		print "UpDnCntrTest: Failed: max diff=", diff
		#print "cntClock=", cntClock, "cnt=", cnt, "divBy=", divBy
	return success

def circuitTests(test='all'):
	print "\nsoftGlue circuit tests"

	### encoder_HWOR_Test
	if (test=="encoder_HWOR_Test" or test=="all"):
		_selectTest("encoder_HWOR_Test")
		epics.caput("zzz:SG:sseq1.PROC","0", wait=True, timeout=1000000.0)
		time.sleep(.2)
		q = epics.caget("zzz:SG:UpCntr-1_COUNTS")
		ck = epics.caget("zzz:SG:UpCntr-2_COUNTS")
		divisor = epics.caget("zzz:SG:UpDnCntr-1_PRESET")
		if q > 0:
			err = 1 - (1.*ck/q)/divisor
		else :
			err = 1.e6
		if (abs(err) < 1.e-6):
			print "encoder_HWOR_Test: Passed"
		else:
			print "encoder_HWOR_Test: Failed: fractional err=", err

	### encoderTest
	if (test=="encoderTest" or test=="all"):
		_selectTest("encoderTest")

		passed = _encoderTest()
		if not passed:
			# Try again with slower clock
			epics.caput("zzz:SG:20MHZ_CLOCK_Signal", "")
			epics.caput("zzz:SG:10MHZ_CLOCK_Signal", "ck")
			print "\tTry again with slower clock."
			passed = _encoderTest()


	### divByNTest
	if (test=="divByNTest" or test=="all"):
		_selectTest("divByNTest")
		epics.caput("zzz:SG:sseq1.PROC","0", wait=True, timeout=1000000.0)
		time.sleep(1.5)
		cnt = []
		cnt.append(epics.caget("zzz:SG:UpCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-4_COUNTS"))
		cntClock = epics.caget("zzz:SG:UpDnCntr-1_COUNTS")
		success = True
		diff = -1
		for i in range(4):
			d = abs(cntClock - cnt[i])
			if d > 1:
				success = 0
				if d > diff:
					diff = d
	
		if success:
			print "divByNTest: Passed"
		else:
			print "divByNTest: Failed: max diff=", diff
			print "cnt=", cnt, "cntClock=", cntClock
	

	### gateDlyTest
	if (test=="gateDlyTest" or test=="all"):
		_selectTest("gateDlyTest")
		epics.caput("zzz:SG:BUFFER-1_IN_Signal","0")
		time.sleep(.2)
		epics.caput("zzz:SG:busy1","Busy")
		time.sleep(1.5)
		# Ideally, an interrupt should return busy to "Done".
		epics.caput("zzz:SG:busy1","Done")

		cnt = []
		cnt.append(epics.caget("zzz:SG:UpDnCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-4_COUNTS"))
		cntClock = epics.caget("zzz:SG:UpCntr-1_COUNTS")

		success = True
		diff = -1
		for i in range(4):
			d = abs(cntClock - cnt[i])
			if d > 1:
				success = 0
				if d > diff:
					diff = d
	
		if success:
			print "gateDlyTest: Passed"
		else:
			print "gateDlyTest: Failed: max diff=", diff
			print "cnt=", cnt, "cntClock=", cntClock

	### counterTest
	if (test=="counterTest" or test=="all"):
		_selectTest("counterTest")
		epics.caput("zzz:SG:sseq1.PROC","0", wait=True, timeout=1000000.0)
		time.sleep(1.5)
		cnt = []
		cnt.append(epics.caget("zzz:SG:UpCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-4_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-3_COUNTS"))
		cntClock = epics.caget("zzz:SG:UpDnCntr-4_COUNTS")

		success = True
		diff = -1
		for i in range(7):
			d = abs(cntClock - cnt[i])
			if d > 1:
				success = 0
				if d > diff:
					diff = d

		if success:
			print "counterTest: Passed"
		else:
			print "counterTest: Failed: max diff=", diff
			print "cnt=", cnt, "cntClock=", cntClock

	### gateDlyTest1
	if (test=="gateDlyTest1" or test=="all"):
		_selectTest("gateDlyTest1")
		epics.caput("zzz:SG:BUFFER-2_IN_Signal","1!", wait=True, timeout=1000000.0)
		epics.caput("zzz:SG:sseq1.PROC","0", wait=True, timeout=1000000.0)
		time.sleep(1.5)
		dly = []
		dly.append(epics.caget("zzz:SG:GateDly-1_DLY"))
		dly.append(epics.caget("zzz:SG:GateDly-2_DLY"))
		dly.append(epics.caget("zzz:SG:GateDly-3_DLY"))
		dly.append(epics.caget("zzz:SG:GateDly-4_DLY"))
		cnt = []
		cnt.append(epics.caget("zzz:SG:UpDnCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-4_COUNTS"))

		success = True
		diff = -1
		for i in range(4):
			d = abs(dly[i] - cnt[i])
			if d > 1:
				success = 0
				if d > diff:
					diff = d

		if success:
			print "gateDlyTest1: Passed"
		else:
			print "gateDlyTest1: Failed: max diff=", diff
			print "cnt=", cnt, "dly=", dly
	
	### DFFTest
	if (test=="DFFTest" or test=="all"):
		_selectTest("DFFTest")
		epics.caput("zzz:SG:sseq1.PROC","0", wait=True, timeout=1000000.0)
		time.sleep(1.5)
	
		cnt = []
		cnt.append(epics.caget("zzz:SG:UpCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-4_COUNTS"))
		cntClock = epics.caget("zzz:SG:UpDnCntr-1_COUNTS")

		success = True
		diff = -1
		for i in range(4):
			d = abs(cntClock/2 - cnt[i])
			if d > 1:
				success = 0
				if d > diff:
					diff = d
	
		if success:
			print "DFFTest: Passed"
		else:
			print "DFFTest: Failed: max diff=", diff
			print "cnt=", cnt, "cntClock/2=", cntClock/2

	### gatedScalerTest
	if (test=="gatedScalerTest" or test=="all"):
		_selectTest("gatedScalerTest")
		epics.caput("zzz:SG:BUFFER-1_IN_Signal","0")
		epics.caput("zzz:SG:busy1","Busy")
		time.sleep(1.5)
		# Ideally, an interrupt should return busy to "Done".
		epics.caput("zzz:SG:busy1","Done")
	
		cnt = []
		cnt.append(epics.caget("zzz:SG:UpCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpCntr-4_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:SG:UpDnCntr-3_COUNTS"))
		cntClock = epics.caget("zzz:SG:UpDnCntr-4_COUNTS")

		success = True
		diff = -1
		for i in range(7):
			d = abs(cntClock - cnt[i])
			if d > 1:
				success = 0
				if d > diff:
					diff = d

		if success:
			print "gatedScalerTest: Passed"
		else:
			print "gatedScalerTest: Failed: max diff=", diff
			print "cnt=", cnt, "cntClock=", cntClock

	### dnCntrTest
	if (test=="dnCntrTest" or test=="all"):
		_selectTest("dnCntrTest")
		passed = _dnCntrTest()
	
		if not passed:
			# Try again with slower clock
			epics.caput("zzz:SG:50MHZ_CLOCK_Signal", "")
			epics.caput("zzz:SG:20MHZ_CLOCK_Signal", "ck")
			print "\tTry again with slower clock."
			passed = _dnCntrTest()

		if not passed:
			# Try again with slower clock
			epics.caput("zzz:SG:20MHZ_CLOCK_Signal", "")
			epics.caput("zzz:SG:10MHZ_CLOCK_Signal", "ck")
			print "\tTry again with slower clock."
			passed = _dnCntrTest()

	### UpDnCntrTest
	if (test=="UpDnCntrTest" or test=="all"):
		_selectTest("UpDnCntrTest")
		passed = _UpDnCntrTest()
	
		if not passed:
			# Try again with slower clock
			epics.caput("zzz:SG:50MHZ_CLOCK_Signal", "")
			epics.caput("zzz:SG:20MHZ_CLOCK_Signal", "ck")
			print "\tTry again with slower clock."
			passed = _UpDnCntrTest()

		if not passed:
			# Try again with slower clock
			epics.caput("zzz:SG:20MHZ_CLOCK_Signal", "")
			epics.caput("zzz:SG:10MHZ_CLOCK_Signal", "ck")
			print "\tTry again with slower clock."
			passed = _UpDnCntrTest()

def checkMcsData():
	c=epics.caget("zzz:MCSs0")
	for j in range(8):
		c = epics.caget("zzz:MCSs%d" % j)
		for i in range(len(c)-1):
			if c[i] != c[i+1]-1:
				print "input %d, i=%d, c[i]=%d, c[i+1]=%d" % (j, i, c[i], c[i+1])

def piezoDiffractZED(rampFreq=1000.,acqTime_s=10.,
amplitude=5.0, offset=2.5, waveGenPV="zzz:33220A:1:"):
	"""

	Configure softGlueZynq, and an Agilent 33220A waveform generator, to apply a
	ramp voltage to a piezo sample, and count detector pulses as a function of
	ramp voltage.  Assumes the configuration "SG_piezoDiffract.cfg" is loaded
	into softGlueZynq.

	Raw data follows the ramp output, beginning at the SYNC pulse, so it starts
	at ramp midpoint, to ramp high point, to ramp low point, and finally to ramp
	midpoint.  Reordered data (not implemented) is from ramp low point to ramp
	high point to ramp low point.

	This function is intended to do everything needed for a single measurement
	specified by conditions and data-acquisition time.  In practice, I expect we
	will set the diffractometer to a desired angle and execute this function;
	spec will retrieve the data from the following PVs:

	zzz:userArrayCalc3.AVAL	detector 1 summed counts
	zzz:userArrayCalc3.AVAL	detector 1 summed counts, reordered

	Note that "amplitude" is the amplitude of the signal from the waveform
	generator, and doesn't include the effect of any amplifier of that signal.

	"""

	maxRampFreq = 800000
	if rampFreq > maxRampFreq:
		rampFreq = maxRampFreq
		print "Max ramp freq is ", maxRampFreq

	recordDate = "Tue Nov 8 16:30:47 2016"
	softGlue = "zzz:SG:"
	clockFreq = 50.0e6
	epics.caput(waveGenPV+"enable","Enable", wait=True, timeout=1000000.0)
	out = epics.caget(waveGenPV+"out")
	if out == 0:
		epics.caput(waveGenPV+"output.PROC",1, wait=True, timeout=1000000.0)
	time.sleep(.2)

	# initialize divByN for detTest pulses, just in case users wants to try it
	epics.caput(softGlue + "DivByN-3_RESET_Signal","1!", wait=True, timeout=1000000.0)

	divTicks = clockFreq/(rampFreq*20)
	divTicks = int(round(divTicks))
	rampFreqQuantized = clockFreq/(divTicks*20)
	while rampFreqQuantized > maxRampFreq:
		#print "rampFreqQuantized %f > maxRampFreq %f" % (rampFreqQuantized, maxRampFreq)
		divTicks += 1
		rampFreqQuantized = clockFreq/(divTicks*20)

	# for testing, avoid aliasing by changing the frequency by 1%
	#rampFreqQuantized *= 1.01
	print "rampFreqQuantized = %f\n" % (rampFreqQuantized)
	if (epics.caget(waveGenPV+"enable") == 0):
		epics.caput(waveGenPV+"enable", "Enable", wait=True, timeout=1000000.0)
		time.sleep(.5)
	enableSignal = softGlue+"BUFFER-1_IN_Signal"
	clearSignal = softGlue+"BUFFER-2_IN_Signal"
	epics.caput(softGlue+"DivByN-2_N",divTicks, wait=True, timeout=1000000.0)
	epics.caput(enableSignal,"0", wait=True, timeout=1000000.0)
	epics.caput(clearSignal,"1!", wait=True, timeout=1000000.0)
	epics.caput(waveGenPV+"freq_Hz.A",rampFreqQuantized, wait=True, timeout=1000000.0)
	epics.caput(waveGenPV+"ampl_V.A",amplitude, wait=True, timeout=1000000.0)
	epics.caput(waveGenPV+"offset_V.A",offset, wait=True, timeout=1000000.0)
	epics.caput(waveGenPV+"symmetry_percent.A"," 5.00000000000e+01", wait=True, timeout=1000000.0)
	
	epics.caput(softGlue + "UpCntr-1_CLEAR_Signal","1!", wait=True, timeout=1000000.0)
	epics.caput(enableSignal,"1", wait=True, timeout=1000000.0)
	time.sleep(acqTime_s)
	epics.caput(enableSignal,"0", wait=True, timeout=1000000.0)
	out = epics.caget(waveGenPV+"out")
	if out == 1:
		epics.caput(waveGenPV+"output.PROC",1, wait=True, timeout=1000000.0)


"""
We're going to acquire a series of 'n' 20-time-bin acquisitions, and we want to
discriminate against 60 Hz noise by spreading it equally over all 20 time bins.
This means we want each time bin to start at the zero crossing of the 60 Hz signal
the same number of times that all the other time bins start at the zero crossing.

    |.........|.........|.........|.........|.........|..........	n = 5 acqs
    ^--------------------------------------------------^			m = 1 60 Hz
     ^--------------------------------------------------^
      ^--------------------------------------------------^
       ...

or

    |.........|.........|.........|.........|.........|.........|.	n = 5 acqs
    ^--------------------------^-----------------------^			m = 2 60 Hz
     ^--------------------------^-----------------------^
      ^--------------------------^-----------------------^
       ...

"""
def _calcf(f_ramp=10, max_n=10, max_m=10):
	mindiff = 1.e6
	f_bin_best = 0.
	n_best=0
	m_best=0
	for n in range(1,max_n):
		#print "\nn=%d",
		for m in range(1,max_m):
			f_bin = 60. * (n*20.+1.)/m
			l = 50.e6/f_bin
			#print " f_bin=%8.1f" % f_bin,
			#print " f_ramp=%8.1f" % (f_bin/20.),
			if abs(f_ramp - f_bin/20.) < mindiff:
				mindiff = abs(f_ramp - f_bin/20.)
				f_bin_best = f_bin
				n_best = n
				m_best = m
	ticks = 50.e6/f_bin_best
	print "for f_ramp=%.2f, best f_bin=%8.1f (f_bin/20=%.2f)" % (f_ramp, f_bin_best, f_bin_best/20.)
	print "	n_best=%d, m_best=%d, ticks=%d" % (n_best, m_best, ticks)
	
	return (ticks, n_best, m_best)

def piezoDiffractZED60(rampFreq=1000.,acqTime_s=10.,
amplitude=5.0, offset=2.5, waveGenPV="zzz:33220A:1:", dis60=1):
	"""

	Configure softGlueZynq, and an Agilent 33220A waveform generator, to apply a
	ramp voltage to a piezo sample, and count detector pulses as a function of
	ramp voltage.  Assumes the configuration "SG_piezoDiffract.cfg" is loaded
	into softGlueZynq.

	Raw data follows the ramp output, beginning at the SYNC pulse, so it starts
	at ramp midpoint, to ramp high point, to ramp low point, and finally to ramp
	midpoint.  Reordered data (not implemented) is from ramp low point to ramp
	high point to ramp low point.

	This function is intended to do everything needed for a single measurement
	specified by conditions and data-acquisition time.  In practice, I expect we
	will set the diffractometer to a desired angle and execute this function;
	spec will retrieve the data from the following PVs:

	zzz:userArrayCalc3.AVAL	detector 1 summed counts
	zzz:userArrayCalc3.AVAL	detector 1 summed counts, reordered

	Note that "amplitude" is the amplitude of the signal from the waveform
	generator, and doesn't include the effect of any amplifier of that signal.

	"""

	maxRampFreq = 800000
	if rampFreq > maxRampFreq:
		rampFreq = maxRampFreq
		print "Max ramp freq is ", maxRampFreq

	recordDate = "Tue Nov 8 16:30:47 2016"
	softGlue = "zzz:SG:"
	clockFreq = 50.0e6
	epics.caput(waveGenPV+"enable","Enable", wait=True, timeout=1000000.0)
	out = epics.caget(waveGenPV+"out")
	if out == 0:
		epics.caput(waveGenPV+"output.PROC",1, wait=True, timeout=1000000.0)
	time.sleep(.2)

	# initialize divByN for detTest pulses, just in case users wants to try it
	epics.caput(softGlue + "DivByN-3_RESET_Signal","1!", wait=True, timeout=1000000.0)

	if (dis60==0 or rampFreq>6000):
		divTicks = clockFreq/(rampFreq*20)
		divTicks = int(round(divTicks))
		n = 1
		m = 1
	else:
		max_n = int(max(30, 2*rampFreq/60))
		(divTicks, n, m) = _calcf(f_ramp=rampFreq, max_n=max_n, max_m=30)

	rampFreqQuantized = clockFreq/(divTicks*20)
	while rampFreqQuantized > maxRampFreq:
	   print "rampFreqQuantized %f > maxRampFreq %f" % (rampFreqQuantized, maxRampFreq)
	   divTicks += 1
	   rampFreqQuantized = clockFreq/(divTicks*20)

	# adjust acquisition time so it's a multiple of m/60
	acqTime_s = int(acqTime_s/(m/60.) + 1)*(m/60.)
	print "acqTime_s = %f" % (acqTime_s)
	
	numSyncs = int(acqTime_s * rampFreqQuantized + 0.5)
	epics.caput(softGlue+"UpDnCntr-1_PRESET",numSyncs, wait=True, timeout=1000000.0)

	print "rampFreqQuantized = %f, numSyncs=%d" % (rampFreqQuantized, numSyncs)
	if (epics.caget(waveGenPV+"enable") == 0):
		epics.caput(waveGenPV+"enable", "Enable", wait=True, timeout=1000000.0)
		time.sleep(.5)

	resetSignal = softGlue+"DFF-3_CLEAR_Signal"
	enableSignal = softGlue+"DFF-4_SET_Signal"
	enableReadback = softGlue+"DFF-4_OUT_BI"
	clearSignal = softGlue+"BUFFER-2_IN_Signal"
	disableSignal = softGlue+"DFF-4_CLEAR_Signal"
	epics.caput(softGlue+"DivByN-2_N",divTicks, wait=True, timeout=1000000.0)

	epics.caput(resetSignal,"0!", wait=True, timeout=1000000.0)
	epics.caput(disableSignal,"0!", wait=True, timeout=1000000.0)
	epics.caput(clearSignal,"1!", wait=True, timeout=1000000.0)
	epics.caput(waveGenPV+"freq_Hz.A",rampFreqQuantized, wait=True, timeout=1000000.0)
	epics.caput(waveGenPV+"ampl_V.A",amplitude, wait=True, timeout=1000000.0)
	epics.caput(waveGenPV+"offset_V.A",offset, wait=True, timeout=1000000.0)
	epics.caput(waveGenPV+"symmetry_percent.A"," 5.00000000000e+01", wait=True, timeout=1000000.0)
	
	epics.caput(enableSignal,"0!", wait=True, timeout=1000000.0)
	time.sleep(.5)
	working = epics.caget(enableReadback)
	elapsedTime = 0
	while (working):
		time.sleep(0.1)
		working = epics.caget(enableReadback)
		elapsedTime += 0.1
		# if something goes wrong, don't just hang
		if elapsedTime>acqTime_s*1.1:
			print "softGlue didn't finish on time"
			working = 0

	epics.caput(resetSignal,"0!", wait=True, timeout=1000000.0)
	epics.caput(disableSignal,"0!", wait=True, timeout=1000000.0)

	out = epics.caget(waveGenPV+"out")
	if out == 1:
		epics.caput(waveGenPV+"output.PROC",1, wait=True, timeout=1000000.0)

	num60HzWavelengths = acqTime_s * rampFreqQuantized
	print "done"

def readCounterFFT(readCounter="zzz:SG_RdCntr:", readFreq=1000, arrayCalc="zzz:SG_RdCntr:fft", acqTime_s=10, softGlue="zzz:SG:"):
	if readFreq > 10000:  readFreq = 10000
	source = arrayCalc+".AA"
	dest = arrayCalc+".BB"
	t = arrayCalc+".CC"
	normdata = arrayCalc+".DD"
	trig = readCounter+"acquire"
	oneshot = readCounter+"oneshot"
	#softGlue = "32idcTXM:SG1:"
	# save state of "oneshot" switch
	save_oneshot = epics.caget(oneshot)
	# make sure softGlue is triggering interrupts
	epics.caput(softGlue+"In_18IntEdge", "None")
	epics.caput(softGlue+"In_18IntEdge", "Rising")
	# set encoder-read frequency
	epics.caput(softGlue+"DivByN-3_N", 8.e6/readFreq)
	# set plot-update rate
	epics.caput("32idcTXM:SG_RdCntr:cVals.SCAN", "1 second")
	# set readCounter to "oneshot" mode
	epics.caput(oneshot, "oneshot", wait=True, timeout=100)
	# set number of points to acquire
	maxN = epics.caget("32idcTXM:SG_RdCntr:cVals.NELM")
	n = min(maxN, acqTime_s*readFreq)
	epics.caput("32idcTXM:SG_RdCntr:cVals.NUSE", n)
	epics.caput(trig, "Busy", wait=True, timeout=10000)
	epics.caput(arrayCalc+".PROC", 1, wait=True)
	d = epics.caget(source)
	n = epics.caget(arrayCalc+".NUSE")
	d = d[0:n]*.4
	d = d-np.average(d)
	epics.caput(normdata, d, wait=True)
	f = np.fft.rfft(d, norm="ortho")
	ticksPerSample = epics.caget(softGlue+"DivByN-3_N")
	dt = 125e-9 * ticksPerSample
	freq = np.fft.rfftfreq(len(d), d=dt)
	epics.caput(dest, abs(f.real))
	epics.caput(t, freq)
	# restore "oneshot" mode
	#epics.caput(oneshot, save_oneshot)
