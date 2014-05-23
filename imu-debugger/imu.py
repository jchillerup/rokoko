import os, sys, serial, struct

class IMU:
    descriptor = None
    identifier = None
    
    def __init__(self, devnode):
        self.descriptor = serial.Serial(devnode, 115200)

        # self.get_identifier()
        
        # print ("Connected to: %s" % self.identifier)

    def get_reading(self):
        self.descriptor.write(b'g')
        output = self.descriptor.read(16)
        w, x, y, z = struct.unpack('f'*4, output)
        return((x,y,z,w))

    def get_identifier(self):
        self.descriptor.write(b'i');
        self.identifier = self.descriptor.read(16).decode('ascii').rstrip()

        return self.identifier

    def rename(self, new_name):
        if (len(new_name) > 16):
            print("Name too long")
            return

        command = bytes('I' + new_name + " "* (16-len(new_name)), 'ascii')
        self.descriptor.write(command)

        return self.get_identifier() == new_name
