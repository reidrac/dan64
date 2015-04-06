#!/usr/bin/env python

__version__ = "0.1"

from argparse import ArgumentParser
from PIL import Image

DEF_WIDTH = 8
DEF_HEIGHT = 8
DEF_NAME = "font"

def main():

    parser = ArgumentParser(description="Bitmap font to C converter",
                            epilog="Copyright (C) 2015 Juan J Martinez <jjm@usebox.net>",
                            )

    parser.add_argument("--version", action="version", version="%(prog)s "  + __version__)

    parser.add_argument("--width", dest="width", default=DEF_WIDTH, type=int,
                        help="character width (default: %d)" % DEF_WIDTH)

    parser.add_argument("--height", dest="height", default=DEF_HEIGHT, type=int,
                        help="character height (default: %d)" % DEF_HEIGHT)

    parser.add_argument("--name", dest="name", default=DEF_NAME, type=str,
                        help="variable name (default: font)")

    parser.add_argument("image", help="image to convert")

    args = parser.parse_args()

    try:
        image = Image.open(args.image)
    except IOError:
        parser.error("failed to open the image")

    (w, h) = image.size

    if w % args.width or h % args.height:
        parser.error("%r size is not multiple of character %d%s" % (args.image, args.width, args.height))

    # guess the background color
    palette = image.getpalette()
    bg = [0, 0, 0]
    for y in range(h):
        for x in range(w):
            pix = image.getpixel((x, y))
            if not isinstance(pix, (list, tuple)) and palette:
                pix *= 3
                pix = palette[pix:pix + 3]
            if pix < bg:
                bg = pix

    data = []
    for y in range(0, h, args.height):
        for x in range(0, w, args.width):
            for j in range(args.height):
                byte = "\t0b"
                for i in range (args.width):
                    pix = image.getpixel((x + i, y + j))
                    if not isinstance(pix, (list, tuple)) and palette:
                        pix *= 3
                        pix = palette[pix:pix + 3]
                    if pix == bg:
                        byte += "1"
                    else:
                        byte += "0"
                data.append(byte)

    char_out = ""
    for char in range(0, len(data), args.height):
        if char_out:
            char_out += ",\n\n"
        char_out += ',\n'.join([byte for byte in data[char:char + args.height]])

    out = "// %d characters\n" % ((w / args.width) * (h / args.height))
    out += "PROGMEM const unsigned char %s[] = {\n" % (args.name)
    out += char_out + "\n"
    out += "};\n // end of %s" % args.name

    print(out)

if __name__ == "__main__":
    main()

