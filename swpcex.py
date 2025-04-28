# swpc-eng.py

import os,sys,d88
 
 
 # DISK A
ignore_ranges = [ 
(0, 0, 0x8370),
(0, 0x8563,  0xedfe),
#(0, 0x12b76, 0x2f2cb),
(0, 0x140ec, 0x1a563), 
(0,0x1ab60,0x1f363),
(0, 0x20400, 0x2f000),
(0,0x2f6f8,0x3d6ca),
 #(0, 0x2a028, 0x2eabc), 
#(0, 0x30703, 0x3c61c), 
 #(0, 0x44442, 0x7d5da), 
 #(0, 0xa5a22, 0x133d52), 
#(0,0x3c8bf,0x3d6c9),
(0, 0x3deee, 0x4064a),
(0,0x40714, 0x41346),
(0,0x41386, 0x4164a),
(0,0x4164a, 0x416a7),
(0,0x42d90, 0x432c8),
(0,0x4371f, 0x43c5f),
(0,0x43689,0x8139e),
#(0,0x4371f, 0x8139e),
#(0,0x7cfbd, 0x801d5), 
#(0,0x7fa9a, 0x7fa9a+1724), 
#(0,0x801e9, 0x8139d),
(0,0x8206b, 0x8206b+360),
(0,0x82416, 0x850fb),
(0,0x88101, 0x884e5),
(0,0x88aa1, 0x88b2b),
(0,0x88c8e, 0x88e7a),
(0,0x9184a, 0x91b08),
(0,0x97fc7, 0x98239),
(0,0x98ec7, 0x9fc3c),
(0,0xa034d, 0xa52d4),
#(0,0xa0668, 0xa4d59),
(0,0xa52d4, 0x133e11),
 ]
'''
ignore_ranges = [ 
	(0, 0,          0x8800), 
	(0, 0x8e09,     0xab06), 
	(0, 0xb770,     0xe002),
	(0, 0x9cd19,    0xa2258),
	(0, 0x14ace,    0x1a9ec),
	(0, 0x29860,    0x2fee6), 
	(0, 0x4641b,    0x766af),
	(0, 0x76f32,    0x7fc81),
	(0, 0x30f1c,    0x3da4f),
	(0, 0xa4aac,    0xa7811),
	(0, 0x1c4bd,    0x1f45e),
	(0, 0xe424,     0xf25c),
	(0, 0xa9c18,    0x138b95),
	(0, 0x20ed0,    0x291e0)
	]
'''
word_list = []

# 813F to 81EE+15
# 823F to 84BE+15
# 873F to 9FEE+15
# E03F to EA9E+15
# ED40 to FC4F

sjisrange = [ (0x813f, 0x81ee+15), (0x823f, 0x84be+15), (0x873f, 0x9fee+15), (0xe03f, 0xea9e+15), (0xed40, 0xfc4f)]


class loc: 
	def __init__(self, address=0x0, disk=0):
		self.address = address
		self.disk = disk
	###
###

class tlword:
	def __init__(self, text=''):
		self.text = text 
		self.locs = []
		self.count = 0
		self.translation = []
		self.bad = False
		self.complete = False
		self.bytect = 0
	###
###

def load_all_disks():
	# get all disks 
	numdisks = len(sys.argv) - 1
	print(numdisks, " disks found")
	_disks = []
	while numdisks > 0:
		#f = open(sys.argv[numdisks], "rb")
		#_by = f.read()
		#f.close()
		temp = d88.disk(sys.argv[numdisks])
		# append all bytes to the global group
		#inbytes = []
		inbytes = temp.HarvestBytes()
		#print(len(inbytes))
		#_i = 0
		#while _i < len(_by):
		#	inbytes.append(_by[_i])
		#	_i += 1
		_disks.append(inbytes)
		numdisks -= 1
	return _disks 
###

def check_hex_range(c):
	r = 0
	while r < len(sjisrange):
		if(c >= sjisrange[r][0]) and (c <= sjisrange[r][1]):
			return True 
		r += 1
	return False 
###

if __name__ == "__main__":
### 1. Load all disk data 
	disks = load_all_disks()

	iTRANSLATED_TEXT = 0
	iORIGINAL_TEXT = 1
	iBYTESIZE = 2
	iBADSTRING = 3
	iLOCATION_LIST = 4
	try:
		f = open("translation.dat", "rb")
		_tl = f.read()
		_rows = _tl.split(bytes([0x0d, 0x0a])) # split by row 
		print(len(_rows))
		_i = 1
		while _i < len(_rows) - 1: # skip last line, its empty
			_m = _rows[_i].split(bytes([0])) # then by element
			_w = tlword()
			_w.text = _m[iORIGINAL_TEXT].decode("shiftjis")
			# TODO: ADDRESS MULTIPLES 

			#word_list.append(_w)
			#_w.count = int(_m[1])
			_i += 1
		f.close()
	except:
		print("no translation.dat found or failed to open. creating new...")

### 2. extract all text 
	D = 0
	while D < len(disks):
		i = 0
		charct = 0
		duct = 0
		badct = 0
		inbytes = disks[D]
		cur_range = 0
		word = ''
		wordisbad = False
		wc=0
		while i < len(inbytes):
			# skip ranges that are defined as code regions
			_r = 0
			while _r < len(ignore_ranges):
				if i >= ignore_ranges[_r][1] and i <= ignore_ranges[_r][2] and D == ignore_ranges[_r][0]:
					i = ignore_ranges[_r][2]
					break
				_r += 1
			# get the first byte that is non zero
			while (inbytes[i] == 0):
				i += 1
			# add to the current word until we hit another 0 byte  
			charct = 0
			tempword = []
			while (i < len(inbytes)):
				if(inbytes[i] != 0):
					tempword.append(inbytes[i])
					charct += 1
					i += 1
				else:
					break
			#print(len(tempword))
			try:
				twb = bytes(tempword)
				#print(twb.decode("shiftjis"), end="")
				skip = False
				if(twb.decode("shiftjis").strip() == ""):
					skip = True 
				if(len(twb) < 2):
					skip = True
				for tw in word_list:
					if tw.text == twb:
						skip = True
						duct += 1
						tw.locs.append(loc(disk=D, address = i - charct))
						break
				if skip == False: 
					_w = tlword()
					_w.text = twb 
					_w.translation = twb
					# and new location 
					_w.locs.append(loc(disk=D, address = i - charct))
					_w.bytect = charct
					word_list.append(_w) # and add to word list 
					wc += 1
			except UnicodeDecodeError: 
				continue
			
			i += 1
			#charct += 1
		D += 1
	print(wc, "words found")
	print(duct, "duplicates found")
# TLDAT header
# bytect[2] \0 transl[bytect] \0 text[bytect] \0 bad[1] \0 complete[1] \0 locct[1] \0 locs[ [disk[1] addr[3]] ][locct] \0d \0a
### 3. Write the output file 
	outbin = b'TLDAT'
	i = 0
	while i < len(word_list):
		if(len(word_list[i].locs) < 256):
			outbin += bytes([(word_list[i].bytect & 0xff00)>>8, (word_list[i].bytect & 0xff)]) + bytes([0]) #' #+ word_list[i].text + "," + str(word_list[i].bytect) + "," + str(word_list[i].bad) + ",[" 
			outbin += word_list[i].translation
			outbin += bytes([0]) 
			outbin += word_list[i].text + bytes([0]) 
			outbin += bytes([word_list[i].bad]) + bytes([0])
			outbin += bytes([word_list[i].complete]) + bytes([0])
			# length of location list 
			outbin += bytes([len(word_list[i].locs)]) + bytes([0]) # x4 bytes: disc, addr 0xabcdef
			_ll = 0
			while _ll < len(word_list[i].locs):
				print(str(word_list[i].locs[_ll].disk) + hex(word_list[i].locs[_ll].address) + " | " + word_list[i].text.decode("shiftjis"))
				_bt = bytes([word_list[i].locs[_ll].disk, (word_list[i].locs[_ll].address & 0xff0000) >> 16, (word_list[i].locs[_ll].address & 0xff00) >> 8, word_list[i].locs[_ll].address & 0xff ])
				outbin += _bt
				_ll += 1
			outbin += bytes([0x0d, 0x0a])
		i += 1
	f = open("translation.dat", "wb")
	f.write(outbin)
	f.close()

###


### DO NOT USE 
'''
# write ascii as-is 
if (inbytes[i] >= 0x20) and (inbytes[i] < 0x7f):
	word += chr(inbytes[i])
	charct += 1
	badct+=1
else:
	# combine the next two bytes 
	char = (inbytes[i] << 8) | (inbytes[i+1])
	if (check_hex_range(char) == True):
		a = bytes( [inbytes[i], inbytes[i+1]] )
		try:
			word += a.decode("shiftjis")
			badct += 2
			charct += 2
			i += 1      # its OK, so skip the next byte
			if inbytes[i+1] == 0x0: # is this the end of the str? 
				# before we make a new one, check if it exists 
				skip = False
				for tw in word_list:
					if tw.text == word:
						#print(word, tw.text)
						if word == tw.text:
							skip = True
							tw.locs.append(loc(disk=D, address = i - charct + 1))
							break
				#print(skip)
				if (skip == False) and (word.strip() != ""):
					# then make a new word
					_w = tlword()
					_w.text = word 
					_w.translation = word
					# and new location 
					_w.locs.append(loc(disk=D, address = i - charct + 1))
					_w.bytect = charct
					word_list.append(_w) # and add to word list 
				word = ''
				charct = 0
				i += 1 # go to next byte 
		except UnicodeDecodeError:
			pass 
'''