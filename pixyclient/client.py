import sys, os, time, struct, serial

from pythonosc import osc_message_builder
from pythonosc import udp_client

ANDERS = "192.168.0.114"
osc_client = udp_client.UDPClient(ANDERS, 14040)

ser = serial.Serial(sys.argv[1], 9600)

def get_packet():
    pl = ""
    num_blocks = -1;
    buf = ""

    ser.write('h')
    
    while "*" not in pl:
        pl = ser.readline().decode("utf-8").rstrip()

    num_blocks = int(pl[1:])
    
    buf = struct.pack("i", num_blocks)

    for i in range(num_blocks):
        block = ser.readline().decode("utf-8").rstrip().split(";")
        buf += struct.pack("iiiii", int(block[1]), int(block[2]), int(block[3]), int(block[4]), int(block[5]))

    # add contours to make total = 21
    for i in range(21-num_blocks):
        buf += struct.pack("iiiii", -1, 0, 0, -1, -1)

    # add eyes
    buf += struct.pack("iiiii", -1, 0, 0, -1, -1)
    buf += struct.pack("iiiii", -1, 0, 0, -1, -1)

    return buf

i=0

while True:
    pkt = get_packet()

    msg = osc_message_builder.OscMessageBuilder(address = "/rokoko-udoo2/face")
    msg.add_arg(pkt)
    msg = msg.build()
    osc_client.send(msg)

    print(i)
    i+=1
