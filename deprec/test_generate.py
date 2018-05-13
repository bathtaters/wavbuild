import math

def getSin(sec, freq):
    val = 2 * math.pi * ((sec * freq) % 1.0)
    return math.sin(val), 0
    
def getTri(sec, freq):
    val = 4 * sec * freq
    off = 2 * (math.ceil((val+1)/2) - 1)
    if off % 4 == 2: val = -val
    else: off = -off
    return val,off

def getSaw(sec, freq):
    val = 2 * sec * freq
    off = -2 * (math.ceil((val+1)/2) - 1)
    return val,off

def getSqr(sec, freq):
    if math.floor(2 * sec * freq) % 2 == 1: return -1.0, 0
    return 1.0, 0

def getSil(sec, freq):
    return 0.0, 0

def printGen(sec, freq, gen):
    val,off = gen(sec, freq)
    print("%7.4f = %7.4f + %7.4f" % (off+val,val,off))
	
def getVals(secs, gen = getSil, freq = 50, step = 0.001):
    s = 0.0
    while s < secs:
        print("%5.3f: " % s, end='')
        printGen(s, freq, gen)
        s += step
