import sys
import serial
import pygame
from pygame.locals import *

def 

if __name__ == '__main__':
    s = serial.Serial('/dev/ttyACM3')

    pygame.init()
    screen = pygame.display.set_mode((300, 300))

    # Blit everything to the screen
    screen.fill((255,0,0))
    pygame.display.flip()

    while True:
        line = str(s.readline().rstrip())

        try:
            r,g,b,c,what = line.split(";")
            r = int(255*(float(r)/float(c)))
            g = int(255*(float(g)/float(c)))
            b = int(255*(float(b)/float(c)))
            c = int(c)

            if c > 25:
                screen.fill((r,g,b))
            else:
                screen.fill((0,0,0))
            pygame.display.flip()
        except ValueError as err:
            print err
