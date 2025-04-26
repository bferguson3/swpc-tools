# swpc-eng.py

import os,sys,d88

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
		self.translation = ''
		self.bad = False
		self.bytect = 0
	###
###

def load_all_disks():
	# get all disks 
	numdisks = len(sys.argv) - 1
	print(numdisks, " disks found")
	_disks = []
	while numdisks > 0:
		f = open(sys.argv[numdisks], "rb")
		_by = f.read()
		f.close()
		#temp = d88.disk(sys.argv[numdisks])
		# append all bytes to the global group
		inbytes = []
		#inbytes = temp.HarvestBytes()
		#print(len(inbytes))
		_i = 0
		while _i < len(_by):
			inbytes.append(_by[_i])
			_i += 1
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

			word_list.append(_w)
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
		badct = 0
		inbytes = disks[D]
		cur_range = 0
		word = ''
		while i < len(inbytes) - 1:
			# skip ranges that are defined as code regions
			_r = 0
			while _r < len(ignore_ranges):
				if i >= ignore_ranges[_r][1] and i <= ignore_ranges[_r][2] and D == ignore_ranges[_r][0]:
					i = ignore_ranges[_r][2]
					break
				_r += 1
			# write ascii as-is 
			if (inbytes[i] >= 0x20) and (inbytes[i] < 0x7f):
				word += chr(inbytes[i])
				badct+=1
			else:
				# combine the next two bytes 
				char = (inbytes[i] << 8) | (inbytes[i+1])
				if (check_hex_range(char) == True):
					a = bytes( [inbytes[i], inbytes[i+1]] )
					try:
						word += a.decode("shiftjis")
						badct += 2
						i += 1      # its OK, so skip the next byte
						charct += 1
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
			i += 1
			charct += 1
		D += 1

### 3. Write the output file 
	outbin = b''
	i = 0
	while i < len(word_list):
		outbin += bytes(word_list[i].translation, encoding="shiftjis") + bytes([0]) 
		outbin += bytes(word_list[i].text, encoding="shiftjis") + bytes([0]) 
		outbin += bytes([(word_list[i].bytect & 0xff00)>>8, (word_list[i].bytect & 0xff)]) + bytes([0]) #' #+ word_list[i].text + "," + str(word_list[i].bytect) + "," + str(word_list[i].bad) + ",[" 
		outbin += bytes([word_list[i].bad]) + bytes([0])
		# length of location list 
		#print(len(word_list[i].locs), end=" ")
		outbin += bytes([len(word_list[i].locs)]) + bytes([0]) # x4 bytes: disc, addr 0xabcdef
		#print(len(word_list[i].locs))
		_ll = 0
		while _ll < len(word_list[i].locs):
			_bt = bytes([word_list[i].locs[_ll].disk, (word_list[i].locs[_ll].address & 0xff000) >> 16, (word_list[i].locs[_ll].address & 0xff00) >> 8, word_list[i].locs[_ll].address & 0xff ])
			outbin += _bt
			#print(_bt)
			_ll += 1
		outbin += bytes([0x0d, 0x0a])
		i += 1
	f = open("translation.dat", "wb")
	f.write(outbin)
	f.close()

###