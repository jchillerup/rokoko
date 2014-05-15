import sys, signal, threading, time
import serial
from pythonosc import osc_message_builder, udp_client

NODES = [
    "/dev/ttyACM3",
    "/dev/ttyACM4",
    "/dev/ttyACM5",
    "/dev/ttyACM6",
    "/dev/ttyACM7",
    ]

RECIPIENT = "192.168.1.38"
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
    
    def __init__(self, devnode, udpclient):
        threading.Thread.__init__(self)
        print( "Opening %s" % devnode)

        self.devnode = devnode
        self.shortname = devnode.split("/")[2]
        self.descriptor = serial.Serial(device, 57600)
        self.udp = udpclient

        self.descriptor.write(b"g");
        
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


#        print(str(reading))
        return str(reading)

    def get_osc(self):
        msg = osc_message_builder.OscMessageBuilder(address = ("/sensors/%s" % self.shortname))
        msg.add_arg(self.get_reading().replace('\\n', '*'), osc_message_builder.OscMessageBuilder.ARG_TYPE_STRING)

        return msg.build()

    def run(self):
        while self.is_running:
            if self.in_sync:
                msg = self.get_osc()
                self.udp.send(msg)

        print("should have died")
    
    def close(self):
        self.is_running = False
        self._stop() # this is a hack

        

if __name__ == '__main__':
    client = udp_client.UDPClient(RECIPIENT, PORT)

    for device in NODES:
        p = Port(device, client)
        ports.append(p)

        p.start()
        
    num_packets = 0;
        
    # while True:
    #     for port in ports:
    #         package = port.get_osc()
    #         client.send(package)
    #         num_packets += 1

    #         if num_packets % 100 == 0:
    #             print ("Packet count: %d" % num_packets)
    

