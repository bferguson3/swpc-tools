# reinsert.py 

# argv[1] translation.db, argv[2] disk 

# 1. load tldb as word class 
# 2. load disk, extract raw 
# 3. reinsert text into raw bytes 
# 4. reisert raw bytes into d88 
# 5. output new d88 

import os,sys   # path, argv, exit
import d88      # d88
import swpcex   # loc,tlword


def check_null(c):
    if(c != 0):
        print("TLDAT error! Null seperator missing!")
        sys.exit()


def LoadTLDatFile(tldb):
    hdr = ''
    word_list = []
    i = 0
    while i < 5:
        hdr += chr(tldb[i])
        i += 1

    if hdr != 'TLDAT':
        print("Not a valid TLDAT file!!")
        sys.exit()

    # TLDAT header
    # bytect[2] \0 transl[bytect] \0 text[bytect] \0 bad[1] \0 complete[1] \0 locct[1] \0 locs[ [disk[1] addr[3]] ][locct] \0d \0a
    bc = 5
    while(bc < len(tldb)):
        _word = swpcex.tlword()

        _sz = (tldb[bc] << 8) + tldb[bc+1]
        _word.bytect = _sz 
        bc += 2
        check_null(tldb[bc])
        bc += 1

        #print(_word.bytect)

        _c = 0
        _t = bytes([])
        while _c < _sz:
            #_word.translation += bytes([tldb[bc]]).decode('utf-8')
            _t += bytes([tldb[bc+_c]])
            _c += 1
        _word.translation = _t.decode('shiftjis')
        bc += _c 
        check_null(tldb[bc])
        bc += 1

        #print(_word.translation)

        _c = 0
        _t = bytes([])
        while _c < _sz:
            _t += bytes([tldb[bc+_c]])
            _c += 1
        _word.text = _t.decode('shiftjis')
        bc += _c 
        check_null(tldb[bc])
        bc += 1

        #print(_word.text)

        _word.bad = tldb[bc]
        bc += 1
        check_null(tldb[bc])
        bc += 1

        #print(_word.bad)

        _word.complete = tldb[bc]
        bc += 1
        check_null(tldb[bc])
        bc += 1

        #print(_word.complete)

        _lct = tldb[bc]
        bc += 1
        check_null(tldb[bc])
        bc += 1

        #print(_lct)

        i = 0
        while i < _lct:
            _newloc = swpcex.loc()
            _newloc.disk = tldb[bc]
            bc += 1
            _a = (tldb[bc] << 16) | (tldb[bc+1] << 8) | (tldb[bc+2])
            _newloc.address = _a 
            _word.locs.append(_newloc)
            bc += 3
            i += 1

        if(tldb[bc] != 0x0d):
            print("BAD LINE")
        if(tldb[bc+1] != 0x0a):
            print("BAD LINE")
        
        bc += 2

        word_list.append(_word)
    return word_list


f = open (sys.argv[1], "rb")
tldb = f.read()
f.close()
orig_dsk = d88.disk(sys.argv[2])
raw_bytes = orig_dsk.HarvestBytes()
f = open("rb.bin", "wb")
f.write(bytes(raw_bytes))
f.close()
word_list = LoadTLDatFile(tldb)

# reinsert text into raw bytes 
# for each loc, (error check later) reinsert there, 
#  !! default to 0 because we are doing 1 disk at a time !!
wordct = 0
i = 0
while i < len(word_list):
    #if(word_list[i].complete == 1):
    #    if(word_list[i].bad == 0):
    wordct += 1
    for ls in word_list[i].locs:
        # well.. its always disk A for now 
        bl = 0
        _bloc = ls.address
        _tlby = bytes(word_list[i].translation, encoding="shiftjis")
        while bl < word_list[i].bytect:
            raw_bytes[_bloc+bl] = _tlby[bl]
            bl += 1
    i += 1

print("Reinserting", wordct, "words.")
# then call the reinsert bytes function on the disk 
orig_dsk.ReinsertRawBytes(raw_bytes)
orig_dsk.WriteBytes("out.img")
