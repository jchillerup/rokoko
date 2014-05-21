import sys, signal, threading, time
import serial
from pythonosc import osc_message_builder, udp_client

NODES = [
    # "/dev/ttyACM3",
    # "/dev/ttyACM4",
    # "/dev/ttyACM5",
    # "/dev/ttyACM6",
    # "/dev/ttyACM7,"
    ]

with open("nodes", "r") as fp:
    for line in fp:
        NODES.append(line.rstrip())
    
RECIPIENT = "192.168.1.61"
PORT = 14040

ports = []

def signal_handler(signal, frame):
    print("Exiting gracefully")
        
    for port in ports:
        port.close();
        
signal.signal(signal.SIGINT, signal_handler)


class Port(threading.Thread):
    devnode = None
    shortname = None
    descriptor = None
    in_sync = False
    is_running = True
    udp = None
    identifier = None
    num_packets_received = 0
    
    def __init__(self, devnode, udpclient):
        threading.Thread.__init__(self)
        print( "Opening %s" % devnode)

        self.devnode = devnode
        self.shortname = devnode.split("/")[2]
        self.descriptor = serial.Serial(device, 57600)
        self.udp = udpclient

        self.identifier = str(self.get_identifier().rstrip())

        print ("Identifier is: %s" % self.identifier)
        
        self.descriptor.write(b"g");
        
        self.sync()

    def sync(self):
        cur = 'x'

        while cur != b'&':
            cur = self.descriptor.read()
            
        print ("Synced")
        self.in_sync = True

    def get_reading(self):
        self.descriptor.write(b'g')
        reading = b""
        cur = 'x'
        while cur != b'&':
            cur =  self.descriptor.read()
            reading += cur

        return str(reading)

    def get_osc(self):
        msg = osc_message_builder.OscMessageBuilder(address = ("/sensors/%s" % self.shortname))
        msg.add_arg(self.get_reading().replace('\\n', '*'), osc_message_builder.OscMessageBuilder.ARG_TYPE_STRING)

        return msg.build()

    def get_identifier(self):
        self.descriptor.write(b"i")
        reading = b""
        cur = 'x'
        while cur != b'\n':
            cur =  self.descriptor.read()
            reading += cur

        return reading
    
    def run(self):
        while self.is_running:
            if self.in_sync:
                msg = self.get_osc()
                self.num_packets_received += 1
                self.udp.send(msg)

        print("should have died")
    
    def close(self):
        self.is_running = False
        self._stop() # this is a hack

    def get_and_reset_num_packets(self):
        num = self.num_packets_received
        self.num_packets_received = 0
        return num
        

if __name__ == '__main__':
    client = udp_client.UDPClient(RECIPIENT, PORT)

    for device in NODES:
        p = Port(device, client)
        ports.append(p)

        p.start()


    while True:
        for port in ports:
            print ("%s: %d packets" % (port.shortname, port.get_and_reset_num_packets()))

            time.sleep(1/len(NODES))
            
    # while True:
    #     for port in ports:
    #         package = port.get_osc()
    #         client.send(package)
    #         num_packets += 1

    #         if num_packets % 100 == 0:
    #             print ("Packet count: %d" % num_packets)
    

