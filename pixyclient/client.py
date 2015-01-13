import sys, os, time, struct, serial

from pythonosc import osc_message_builder
from pythonosc import udp_client

ANDERS = "192.168.0.114"
osc_client = udp_client.UDPClient(ANDERS, 14040)

ser = serial.Serial(sys.argv[1], 115200, timeout=1)

def get_packet():
    pl = ""
    num_blocks = -1;
    buf = ""
    
    while "*" not in pl:
        try:
            pl = ser.readline()
            pl = pl.decode("ascii", "replace").rstrip()
        except:
            pass
    
    try:
        num_blocks = int(pl[1:])
    except ValueError:
        return

    buf = struct.pack("i", num_blocks)

    for i in range(num_blocks):
        try:
            block = ser.readline()
        except Exception:
            continue

        block = block.decode("ascii", "replace").rstrip()

        if block.find("*") != -1:
            return

        block = block.split(";")
        # print(block)
        try:
            buf += struct.pack("iiiii", int(block[1]), int(block[2]), int(block[3]), int(block[4]), int(block[5]))
        except:
            pass

    # add contours to make total = 21
    for i in range(21-num_blocks):
        buf += struct.pack("iiiii", -1, 0, 0, -1, -1)

    # add eyes
    buf += struct.pack("iiiii", -1, 0, 0, -1, -1)
    buf += struct.pack("iiiii", -1, 0, 0, -1, -1)

    return buf

i=0

time.sleep(2)

ser.write(b'j')

while True:
    pkt = get_packet()

    if pkt is None:
        continue

    msg = osc_message_builder.OscMessageBuilder(address = "/rokoko-udoo2/face")
    msg.add_arg(pkt)
    msg = msg.build()
    osc_client.send(msg)

    print(i)
    i+=1
