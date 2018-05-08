SYNCWORD = '0011111111111101'
FRAMESIZE = (64,80)
HEX_UBITS = True
DEF_FPS = 30

def ltc(bm, fps_25=False, ubits_hex=HEX_UBITS):
    # Get bitmap
    if isinstance(bm,int): 
        bm = bin(bm)[2:][::-1]
    elif not isinstance(bm, str): return -1
    bm += '0'*(FRAMESIZE[0] - len(bm))
    if len(bm) > FRAMESIZE[0]: bm += '0'*(FRAMESIZE[1] - len(bm))

    # Get timecode
    tc = [0,0,0,0]
    for i in range(4):
        x = 2
        if i in (1,2): x = 1
        tc[3-i] = int(bm[i*16:i*16+4][::-1],2) + 10*int(bm[i*16+8:i*16+12-x][::-1],2)

    # Get userbits
    ub = [0,0,0,0]
    for i in range(4):
        ub[3-i] = int(bm[i*16+12:i*16+16][::-1] + bm[i*16+4:i*16+8][::-1], 2)

    # Get flags
    flagmap = { 'DropFrame' : 10,
                'ColorSync' : 11,
                'BGF0'      : 43,
                'BGF1'      : 58,
                'BGF2'      : 59,
                'Parity'    : 27 }
    if fps_25:
        flagmap['BGF0'] = 27
        flagmap['BGF2'] = 43
        flagmap['Parity'] = 59
    flags = {}
    for f,i in flagmap.items():
        flags.update([(f, True if int(bm[i]) else False)])
    ubits_str = (flags['BGF2'] << 1) + flags['BGF0'] == 1

    # Get syncword
    syncw = SYNCWORD
    if len(bm) == 80: syncw = bm[64:]

    # Output:
    print('LTC Frame as Unsigned Int:',int(bm,2),'('+hex(int(bm,2))+')')
    print('Bitmap:',bm)

    # Errors
    if syncw != SYNCWORD:
        print('ERROR! Invalid sync word:',syncw)
    if not (int(len(bm) > FRAMESIZE[0]) + bm.count('1')) % 2:
        print('ERROR! Bad ones count:',bm.count('1'))
    if len(bm) not in FRAMESIZE:
        print('ERROR! Invalid bit count:',len(bm))
    print()

    # Decoded:
    print('TC:',':'.join(str(t).zfill(2) for t in tc))
    print('UB:','"' if ubits_str else '',end='')
    for u in ub:
        if ubits_str: print(chr(u),end='')
        elif ubits_hex: print(hex(u)[2:].zfill(2),end=' ')
        else: print(str(u).zfill(2),end=' ')
    print('" (str)' if ubits_str else ('(hex)' if ubits_hex else ''))
    print('FPS:','25' if fps_25 else 'Not 25')
    print('Flags:')
    for f,v in sorted(flags.items()):
        print('  ',f,' ('+str(flagmap[f])+')',': ',v,sep='')
    print('  Format:',getFormat(flags))

def getFormat(flags):
    # Clock format
    fmt = 'Unspecified'
    if flags['BGF1']: fmt = 'Clock time'
    fmt = '\n    Timecode: ' + fmt

    # Userbits format
    ubFmt = ('Unspecified','Text string','Date & time zone','Page & line')
    fmt += '\n    User bits: ' + ubFmt[(flags['BGF2'] << 1) + flags['BGF0']]

    return fmt

def frames(c, fps=DEF_FPS):
    tc = [0]*4
    tc[3] = c % fps
    c = c // fps
    tc[2] = c % 60
    c = c // 60
    tc[1] = c % 60
    tc[0] = c // 60
    print(':'.join(str(t).zfill(2) for t in tc),'@',fps,'fps')
    return tc

def tc(hh,mm,ss,ff, fps=DEF_FPS):
    c = ff
    c += ss * fps
    c += mm * 60 * fps
    c += hh * 60 * 60 * fps
    print(c,'frames')
    return c

def readbm(bm):
    arr = []
    byte = ''
    for i,b in enumerate(bm):
        byte += b
        if i % 4 == 3:
            arr.append(byte)
            byte = ''
    for i in range(len(arr)//2):
        print(arr[0+i*2][::-1], arr[1+i*2][::-1], '\t',
              hex(int(arr[0+i*2][::-1],2))+hex(int(arr[1+i*2][::-1],2))[2:],'\t',
              str(int(arr[0+i*2][::-1],2)).zfill(2),str(int(arr[1+i*2][::-1],2)).zfill(2),
        )
    
        
def drop(frames, fps=DEF_FPS):
    return frames - 2 * ((frames//(60*fps)) - ((frames//(60*fps)) // 10))

def drop2(frames, fps=DEF_FPS):
    return (frames * (1 - (9 // (5 * 60 * fps))))
    
def undrop(frames, fps=DEF_FPS):
    return frames - (frames * 9 // (60 * fps * 5))
