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
	epics.caput("zzz:softGlue:sseq1.PROC","0", wait=True, timeout=1000000.0)
	time.sleep(.2)
	q = epics.caget("zzz:softGlue:UpCntr-1_COUNTS")
	ck = epics.caget("zzz:softGlue:UpCntr-2_COUNTS")
	divisor = epics.caget("zzz:softGlue:UpDnCntr-1_PRESET")
	err = 1 - (1.*ck/q)/divisor
	passed = abs(err) < 1.e-6
	if passed:
		print "encoderTest: Passed"
	else:
		print "encoderTest: Failed: fractional err=", err
	return passed


def _dnCntrTest():
	epics.caput("zzz:softGlue:sseq1.PROC","0", wait=True, timeout=1000000.0)
	time.sleep(1.5)
	divBy = []
	divBy.append(epics.caget("zzz:softGlue:DnCntr-1_PRESET"))
	divBy.append(epics.caget("zzz:softGlue:DnCntr-2_PRESET"))
	divBy.append(epics.caget("zzz:softGlue:DnCntr-3_PRESET"))
	divBy.append(epics.caget("zzz:softGlue:DnCntr-4_PRESET"))
	cnt = []
	cnt.append(epics.caget("zzz:softGlue:UpCntr-1_COUNTS"))
	cnt.append(epics.caget("zzz:softGlue:UpCntr-2_COUNTS"))
	cnt.append(epics.caget("zzz:softGlue:UpCntr-3_COUNTS"))
	cnt.append(epics.caget("zzz:softGlue:UpCntr-4_COUNTS"))
	cntClock = epics.caget("zzz:softGlue:UpDnCntr-1_COUNTS")

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
		#print "cntClock=", cntClock, "cnt=", cnt, "divBy=", divBy
	return success

def circuitTests(test='all'):
	print "\nsoftGlue circuit tests"

	### encoder_HWOR_Test
	if (test=="encoder_HWOR_Test" or test=="all"):
		_selectTest("encoder_HWOR_Test")
		epics.caput("zzz:softGlue:sseq1.PROC","0", wait=True, timeout=1000000.0)
		time.sleep(.2)
		q = epics.caget("zzz:softGlue:UpCntr-1_COUNTS")
		ck = epics.caget("zzz:softGlue:UpCntr-2_COUNTS")
		divisor = epics.caget("zzz:softGlue:UpDnCntr-1_PRESET")
		err = 1 - (1.*ck/q)/divisor
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
			epics.caput("zzz:softGlue:20MHZ_CLOCK_Signal", "")
			epics.caput("zzz:softGlue:10MHZ_CLOCK_Signal", "ck")
			print "\tTry again with slower clock."
			passed = _encoderTest()


	### divByNTest
	if (test=="divByNTest" or test=="all"):
		_selectTest("divByNTest")
		epics.caput("zzz:softGlue:sseq1.PROC","0", wait=True, timeout=1000000.0)
		time.sleep(1.5)
		cnt = []
		cnt.append(epics.caget("zzz:softGlue:UpCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-4_COUNTS"))
		cntClock = epics.caget("zzz:softGlue:UpDnCntr-1_COUNTS")
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
		epics.caput("zzz:softGlue:BUFFER-1_IN_Signal","0")
		time.sleep(.2)
		epics.caput("zzz:softGlue:busy1","Busy")
		time.sleep(1.5)
		# Ideally, an interrupt should return busy to "Done".
		epics.caput("zzz:softGlue:busy1","Done")

		cnt = []
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-4_COUNTS"))
		cntClock = epics.caget("zzz:softGlue:UpCntr-1_COUNTS")

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
		epics.caput("zzz:softGlue:sseq1.PROC","0", wait=True, timeout=1000000.0)
		time.sleep(1.5)
		cnt = []
		cnt.append(epics.caget("zzz:softGlue:UpCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-4_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-3_COUNTS"))
		cntClock = epics.caget("zzz:softGlue:UpDnCntr-4_COUNTS")

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
		epics.caput("zzz:softGlue:BUFFER-2_IN_Signal","1!", wait=True, timeout=1000000.0)
		epics.caput("zzz:softGlue:sseq1.PROC","0", wait=True, timeout=1000000.0)
		time.sleep(1.5)
		dly = []
		dly.append(epics.caget("zzz:softGlue:GateDly-1_DLY"))
		dly.append(epics.caget("zzz:softGlue:GateDly-2_DLY"))
		dly.append(epics.caget("zzz:softGlue:GateDly-3_DLY"))
		dly.append(epics.caget("zzz:softGlue:GateDly-4_DLY"))
		cnt = []
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-4_COUNTS"))

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
		epics.caput("zzz:softGlue:sseq1.PROC","0", wait=True, timeout=1000000.0)
		time.sleep(1.5)
	
		cnt = []
		cnt.append(epics.caget("zzz:softGlue:UpCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-4_COUNTS"))
		cntClock = epics.caget("zzz:softGlue:UpDnCntr-1_COUNTS")

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
		epics.caput("zzz:softGlue:BUFFER-1_IN_Signal","0")
		epics.caput("zzz:softGlue:busy1","Busy")
		time.sleep(1.5)
		# Ideally, an interrupt should return busy to "Done".
		epics.caput("zzz:softGlue:busy1","Done")
	
		cnt = []
		cnt.append(epics.caget("zzz:softGlue:UpCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-3_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpCntr-4_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-1_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-2_COUNTS"))
		cnt.append(epics.caget("zzz:softGlue:UpDnCntr-3_COUNTS"))
		cntClock = epics.caget("zzz:softGlue:UpDnCntr-4_COUNTS")

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
			epics.caput("zzz:softGlue:50MHZ_CLOCK_Signal", "")
			epics.caput("zzz:softGlue:20MHZ_CLOCK_Signal", "ck")
			print "\tTry again with slower clock."
			passed = _dnCntrTest()

		if not passed:
			# Try again with slower clock
			epics.caput("zzz:softGlue:20MHZ_CLOCK_Signal", "")
			epics.caput("zzz:softGlue:10MHZ_CLOCK_Signal", "ck")
			print "\tTry again with slower clock."
			passed = _dnCntrTest()
