import os, sys, serial, struct

devnode = sys.argv[1]

s = serial.Serial(devnode, 57600)

s.write(b'g')

output = s.read(16)

w, x, y, z = struct.unpack('f'*4, output)

print((w,x,y,z))
