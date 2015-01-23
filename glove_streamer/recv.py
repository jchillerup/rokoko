import sys, time, struct
import serial
import pygame
from pygame.locals import *

if __name__ == '__main__':
    s = serial.Serial('/dev/ttyACM3')

    pygame.init()
    screen = pygame.display.set_mode((300, 300))

    # Blit everything to the screen
    screen.fill((255,0,0))
    pygame.display.flip()

    while True:
        s.write(b"g")
        time.sleep(0.1)

        try:
	    r, g, b, c, ra, ga, ba, ca = struct.unpack("HHHHHHHH", s.read(16))
            r = int(255*(float(r)/float(c)))
            g = int(255*(float(g)/float(c)))
            b = int(255*(float(b)/float(c)))
            c = int(c)

	    print "%d, %d, %d, %d" % (r,g,b,c)

            if c > 25:
                screen.fill((r,g,b))
            else:
                screen.fill((0,0,0))
            pygame.display.flip()
        except ValueError as err:
            print err
