#!/usr/bin/env python

from argparse import ArgumentParser

__version__ = "1.0"

def main():

    parser = ArgumentParser(description="Bin to H converter",
                            epilog="Copyright (C) 2014 Juan J Martinez <jjm@usebox.net>",
                            )

    parser.add_argument("--version", action="version", version="%(prog)s "  + __version__)
    parser.add_argument("file", help="file to convert")
    parser.add_argument("id", help="variable to use")

    args = parser.parse_args()

    with open(args.file, "rb") as fd:
        data = bytearray(fd.read())

    data_out = ""
    for part in range(0, len(data), 8):
        if data_out:
            data_out += ",\n"
        data_out += ', '.join(["0x%02x" % b for b in data[part: part + 8]])

    print("#include <stdint.h>\n")
    print("/* file: %s */" % args.file)
    print("#define %s_LEN %d\n" % (args.id.upper(), len(data)))
    print("uint8_t %s[] = {\n%s\n};\n" % (args.id, data_out))

if __name__ == "__main__":
    main()

