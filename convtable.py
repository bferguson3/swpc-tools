f = open("convtable.bin", "rb")
inbytes = f.read()
f.close()

print("unsigned char convTable[" + str(len(inbytes)) + "] = { \n")
i = 0
while i < len(inbytes):
    print(str(inbytes[i]) + ", ", end="")
    if i % 16 == 0:
        print("")
    i += 1

print("};")