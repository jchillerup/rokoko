import os, sys, serial, struct, time

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

    def calibrate_accelerometer(self):
        self.descriptor.write(b'a')
        time.sleep(0.25)

        print(self.descriptor.readline())
        time.sleep(10);
        print(self.descriptor.readline())
        print(self.descriptor.readline())

        return True
        # if self.descriptor.read(3) == 'ca\n':
        #     time.sleep(10)

        #     if self.descriptor.read(5) == 'AS\nCA':
        #         return True
        # else:
        #     return False

    def calibrate_magnometer(self):
        self.descriptor.write(b'm')
        time.sleep(0.25)

        print(self.descriptor.readline())
        time.sleep(10);
        print(self.descriptor.readline())
        print(self.descriptor.readline())

        return True
                
        # if self.descriptor.read(3) == 'cm\n':
        #     time.sleep(10)

        #     if self.descriptor.read(5) == 'MS\nCM':
        #         return True
        # else:
        #     return False

    

    def rename(self, new_name):
        if (len(new_name) > 16):
            print("Name too long")
            return

        command = bytes('I' + new_name + " "* (16-len(new_name)), 'ascii')
        self.descriptor.write(command)

        return self.get_identifier() == new_name

    def close(self):
        self.descriptor.close()

