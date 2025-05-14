#!/usr/bin/python3 

# makearf.py 

# directory as input 
#  compressed/ uncompressed/ hidden/

#  1. Count all files 
#  2. Populate file class w/ bytes - set all compressed type to uncompressed
#  3. Remove "_" from compressed fn
#  4. ! Change manually binary file extracted size 
#  5. Write file count
#  6. Write file headers 
#  7. Write file bytes 

import os,sys


class STRFile:
    def __init__(self, _by):
        self.bytes = _by
        self.name = []
        self.type = 0
        self.uncomp_size = 0
        self.compr_size = 0
        self.offset = 0
        return
    ###
    def __repr__(self):
        return self.name
###

compdir=sys.argv[1] + "/compressed/"
uncompdir=sys.argv[1] + "/uncompressed/"
hiddendir=sys.argv[1] + "/hidden/"

if(not os.path.isdir(compdir)):
    print("compressed/ folder not found.")
    sys.exit()
if(not os.path.isdir(uncompdir)):
    print("uncompressed/ folder not found.")
    sys.exit()
if(not os.path.isdir(hiddendir)):
    print("hidden/ folder not found.")
    sys.exit()

compr_files = os.listdir(compdir)
uncomp_files = os.listdir(uncompdir)
hidden_files = os.listdir(hiddendir)

file_ct = len(compr_files) + len(uncomp_files) + len(hidden_files)

print(file_ct, "files found. (Expected: STRACTOR.ARF[474] )")

files = []

i = 0
while i < len(compr_files):
    f = open(compdir + compr_files[i], "rb")
    inby = f.read() 
    a = STRFile(inby)
    f.close() 
    a.name = compr_files[i]
    if "STR_" in a.name:
        a.name = a.name.replace("STR_", "STR")
    a.type = 2 # 2 = uncompressed
    a.uncomp_size = os.path.getsize(compdir + compr_files[i])
    a.compr_size = a.uncomp_size
    #a.offset = _ofs 
    files.append(a)
    #_ofs += a.compr_size 
    i += 1

i = 0
while i < len(uncomp_files):
    f = open(uncompdir + uncomp_files[i], "rb")
    inby = f.read() 
    a = STRFile(inby)
    f.close() 
    a.name = uncomp_files[i]
    #print(a.name)
    a.type = 2 # uncompressed 
    a.uncomp_size = os.path.getsize(uncompdir + uncomp_files[i])
    a.compr_size = a.uncomp_size
    #a.offset = _ofs 
    files.append(a)
    #_ofs += a.compr_size 
    i += 1

i = 0
while i < len(hidden_files):
    f = open(hiddendir + hidden_files[i], "rb")
    inby = f.read() 
    a = STRFile(inby)
    f.close() 
    a.name = hidden_files[i]
    #print(a.name)
    a.type = 1 # binary 
    a.uncomp_size = os.path.getsize(hiddendir + hidden_files[i])
    a.compr_size = a.uncomp_size## FIXME!!!!
    #a.offset = _ofs 
    files.append(a)
    #_ofs += a.compr_size 
    i += 1

outfiles = sorted(files, key=lambda x:x.name)

# finally, fix offsets 
_ofs = (file_ct * 22) + 2
for f in outfiles:
    f.offset = _ofs 
    _ofs += f.compr_size 

outbytes = [ (file_ct & 0xff), (file_ct & 0xff00) >> 8]


for f in outfiles:
    # fix decomp size for binaries 
    if f.name == "ACTOR023.STR":
        f.uncomp_size = 694
    elif f.name == "ACTOR085.STR":
        f.uncomp_size = 838
    elif f.name == "ACTOR091.STR":
        f.uncomp_size = 222
    elif f.name == "ACTOR093.STR":
        f.uncomp_size = 329
    elif f.name == "ACTOR104.STR":
        f.uncomp_size = 415
    elif f.name == "ACTOR110.STR":
        f.uncomp_size = 961
    elif f.name == "ACTOR159.STR":
        f.uncomp_size = 707
    elif f.name == "ACTOR161.STR":
        f.uncomp_size = 592
    elif f.name == "ACTOR168.STR":
        f.uncomp_size = 269
    elif f.name == "ACTOR264.STR":
        f.uncomp_size = 43
    elif f.name == "ACTOR267.STR":
        f.uncomp_size = 426
    elif f.name == "LBRAD05A.STR":
        f.uncomp_size = 996
    elif f.name == "LOGCV00A.STR":
        f.uncomp_size = 1652
    elif f.name == "LOTIS03A.STR":
        f.uncomp_size = 396
    # write 13 bytes for FN w null term 
    fl = 0
    al = 0 
    while fl < 12:
        while al < len(f.name):
            outbytes.append(ord(f.name[al]))
            fl+=1
            al+=1
        outbytes.append(0)
        fl+=1
    #if f.name == "END.STR":
    #    outbytes.append(0)
    #elif f.name == "LEMA00A.STR":
    #    outbytes.append(0)
    #elif f.name == "LEMA00B.STR":
    #    outbytes.append(0)
    #elif f.name == "LEMA01A.STR":
    #    outbytes.append(0)
    #elif f.name == "LEMA02A.STR":
    #    outbytes.append(0)
    #elif f.name == "LEMA03A.STR":
    #    outbytes.append(0)
    if len(f.name) < 12:
        outbytes.append(0)
    #outbytes.append(0)
    # ftype 
    outbytes.append(f.type & 0xff)
    outbytes.append(f.uncomp_size & 0xff)
    outbytes.append((f.uncomp_size & 0xff00) >> 8)
    outbytes.append(f.compr_size & 0xff)
    outbytes.append((f.compr_size & 0xff00) >> 8)
    outbytes.append(f.offset & 0xff)
    outbytes.append((f.offset & 0xff00) >> 8)
    outbytes.append((f.offset & 0xff0000) >> 16)
    outbytes.append(0 & 0xff)

# now write file bytes 

for f in outfiles:
    for b in f.bytes:
        outbytes.append(b)

f = open("STRACTOR-E.ARF", "wb")
f.write(bytes(outbytes))
f.close()
## Format 
#struct ARFFile { 
#    u16 file_ct;
#    ARFFileHeader[file_ct] file_headers;
#    const char[file_ct][compr_size] file_dat;
#};
#struct ARFFileHeader { 
#    char name[13];     : (12+null) (bytes between 0 and index 13 are junk)
#    u8 file_type;      : 00 is lzss, 01 is binary, 02 is uncompressed 
#    u16 uncomp_size;
#    u16 compr_size;
#    u24 file_offset;
#    u16 nullterm = 0;
#};

print("Written to STRACTOR-E.ARF")