#!/usr/bin/env python

# scancode (hex)  normal  shift comment

SCAN_CODES = """
76 1b 00
0e ` |
16 1 !
1e 2 "
26 3 9c
25 4 $
2e 5 %
36 6 ^
3d 7 &
3e 8 *
46 9 (
45 0 )
4e - _
55 = +
5d # ~
66 08 00
0d 09 00
15 q Q
1d w W
24 e E
2d r R
2c t T
35 y Y
3c u U
43 i I
44 o O
4d p P
54 [ {
5b ] }
5a 0a 00
1c a A
1b s S
23 d D
2b f F
34 g G
33 h H
3b j J
42 k K
4b l L
4c ; :
52 27 @
1a z Z
22 x X
21 c C
2a v V
32 b B
31 n N
3a m M
41 , <
49 . >
4a / ?
61 5c |
29 20 00
75 1e 8
72 1f 2
6b 11 4
74 10 6
"""

def main():
    codes = SCAN_CODES.strip().split("\n")

    table = { }
    for line in codes:
        sc, norm, shifted = line.split(" ")

        if len(norm) > 1:
            norm = "\\x" + norm
        if len(shifted) > 1:
            shifted = "\\x" + shifted

        table[ord(sc.decode("hex"))] = (norm, shifted)

    max_sc = max(table.keys())
    out = []
    for idx in range(max_sc + 1):
        if idx in table:
            norm, shift = table[idx]
            out.append("{ '%s', '%s' }" % (norm, shift))
        else:
            out.append("{ 0, 0 }")

    print "#define KEYB_MAX_SCANCODE 0x%x" % max_sc
    print "PROGMEM const unsigned char keyb_map[][2] = {"
    print ", \n".join(out)
    print "}; // keyb_map ends\n"

if __name__ == "__main__":
    main()

