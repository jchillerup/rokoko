import sys, signal
import serial
from pythonosc import osc_message_builder, udp_client

NODES = ["/dev/ttyACM6", "/dev/ttyACM7", "/dev/ttyACM8",  "/dev/ttyACM9", "/dev/ttyACM10",]

ports = []

def signal_handler(signal, frame):
    print("Exiting gracefully")
        
    for port in ports:
        port.close();

    sys.exit(0);
        
signal.signal(signal.SIGINT, signal_handler)


class Port():
    devnode = None
    shortname = None
    descriptor = None
    in_sync = False
    
    def __init__(self, devnode):
        print( "Opening %s" % devnode)

        self.devnode = devnode
        self.shortname = devnode.split("/")[2]
        self.descriptor = serial.Serial(device, 57600)

        self.sync()

    def sync(self):
        cur = 'x'

        while cur != b'&':
            cur = self.descriptor.read()
            
        print ("Synced")
        self.in_sync = True

    def get_reading(self):
        reading = b""
        cur = 'x'
        while cur != b'&':
            cur =  self.descriptor.read()
            reading += cur
        
        return str(reading)

    def get_osc(self):
        msg = osc_message_builder.OscMessageBuilder(address = ("/sensors/%s" % self.shortname))
        msg.add_arg(self.get_reading(), osc_message_builder.OscMessageBuilder.ARG_TYPE_STRING)

        return msg.build()
    
    def close(self):
        return self.descriptor.close()
        

if __name__ == '__main__':
    client = udp_client.UDPClient("127.0.0.1", 35000)

    for device in NODES:
        ports.append(Port(device))

    num_packets = 0;
        
    while True:
        for port in ports:
            package = port.get_osc()
            client.send(package)
            num_packets += 1

            if num_packets % 100 == 0:
                print ("Packet count: %d" % num_packets)
    

