import sys, signal
import serial

NODES = ["/dev/ttyACM6", "/dev/ttyACM7", "/dev/ttyACM8",  "/dev/ttyACM9", "/dev/ttyACM10",]

ports = []

def signal_handler(signal, frame):
    print "Exiting gracefully"
        
    for port in ports:
        port.close();

    sys.exit(0);
        
signal.signal(signal.SIGINT, signal_handler)


class Port:
    devnode = None
    descriptor = None
    in_sync = False
    
    def __init__(self, devnode):
        print "Opening %s" % devnode

        self.devnode = devnode
        self.descriptor = serial.Serial(device, 57600)

        self.sync()

    def sync(self):
        cur = 'x'

        while cur != '&':
            cur = self.descriptor.read()

        print "Synced"
        self.in_sync = True

    def get_next_reading(self):
        reading = ""
        cur = 'x'
        while cur != '&':
            cur =  self.descriptor.read()
            reading += cur
        
        return reading

    def close(self):
        return self.descriptor.close()
        

if __name__ == '__main__':
    for device in NODES:
        ports.append(Port(device))

    while True:
        for port in ports:
            reading = port.get_next_reading().replace("\n", " *** ")
            print "%s: %s" % (port.devnode, reading)
            
    

