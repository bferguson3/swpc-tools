# darf.py 

import os,sys

f = open(sys.argv[1], "rb")
inbytes = f.read()
f.close()

bc = 0
file_ct = int.from_bytes(bytes([ inbytes[0], inbytes[1] ]), "little")

compression_types = ['lzss' , 'hidden',  'uncompressed']

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


class ARFFileEntry:
    def __init__(self, by):
        self.bytes = by
        self.fn = ""#self.get_fn()
        self.loc = 0 #self.get_file_loc() 
        self.compr_type = ""#self.get_compr_type()
        self.compr_size = 0
        self.decomp_size = 0
        
    ###
    
    def get_fn(self):
        _fn = []
        b = 0
        while self.bytes[b] != 0:
            _fn.append(self.bytes[b])
            b += 1
        self.fn = bytes(_fn).decode("ascii")
        
        
    ###
    
    def get_file_loc(self):
        _l = self.bytes[18:22+1]
        self.loc = int.from_bytes(_l, 'little')
        
    ###

    def get_compr_type(self):
        self.compr_type = compression_types[self.bytes[13]]
        return compression_types[self.bytes[13]]
        
    ###

    def get_sizes(self):
        #if(self.get_compr_type() == "lz-package"):
        #    self.compr_size = int.from_bytes(self.bytes[14:15+1], "big")
        #    self.decomp_size = int.from_bytes(self.bytes[16:17+1], "big")
        #else:
        self.compr_size = int.from_bytes(self.bytes[14:15+1], "little")
        self.decomp_size = int.from_bytes(self.bytes[16:17+1], "little")

bc = 2

file_entries = []

i = 0
while i < file_ct:
    e = ARFFileEntry(inbytes[bc:bc+22])
    e.get_fn()
    e.get_compr_type()
    e.get_file_loc()
    e.get_sizes()
    #if(e.compr_type == "lzss"):
    if(e.compr_type == "hidden"):
        print(e.fn, end=" ")
        print(e.get_compr_type(), end = " | ")
        print("size", e.decomp_size, "(extracted", e.compr_size, ")")
        fby = inbytes[e.loc:e.loc+e.decomp_size]
        f = open(e.fn[:12], "wb")
        f.write(fby)
        f.close()
    bc+=22
    i += 1

