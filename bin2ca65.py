# bin2ca65.py

import sys

with open(sys.argv[1], "rb") as file:
	count = 0
	byte = file.read(1)
	while byte != "":
		if count % 16 == 0:
			sys.stdout.write("\n\t.byte ");
		byteHexString = format(ord(byte), 'x')
		sys.stdout.write("$" + byteHexString)
		if count % 16 < 15:
			sys.stdout.write(", ");
		count += 1
		byte = file.read(1)
	sys.stdout.write("\n")
		