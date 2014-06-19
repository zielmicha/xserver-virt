import pygame
import argparse
import time

parser = argparse.ArgumentParser(description='Display raw image (typically a framebuffer).')
parser.add_argument('file')
parser.add_argument('width', type=int)
parser.add_argument('height', type=int)
parser.add_argument('--format', default='RGBX')

args = parser.parse_args()

size = (args.width, args.height)
surf = pygame.display.set_mode((size[0] + 2, size[1] + 2))

start = time.time()
frame = 0

while True:
    frame += 1
    pygame.event.pump()
    pygame.draw.rect(surf, (255, 255, 255), (0, 0, size[0], size[1]))

    data = open(args.file, 'r').read()
    img = pygame.image.fromstring(data, size, args.format)
    surf.blit(img, (1, 1))
    pygame.display.flip()

    if frame % 200 == 0:
        print '%d FPS' % (frame / (time.time() - start))
