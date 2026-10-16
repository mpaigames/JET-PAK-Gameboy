#!/usr/bin/env python3
"""Draw a dump from test/dump.c using the generated tile data.  preview.py gb|sms dumpfile out.png"""
import re, sys
from PIL import Image
plat, dump, out = sys.argv[1:4]
src = open(f"{plat}/tiles_{plat}.h").read()
def arr(name):
    m = re.search(name + r"\[\] = \{(.*?)\};", src, re.S)
    return [int(x, 16) for x in re.findall(r"0x[0-9A-Fa-f]+", m.group(1))]
def decode(data, bpt, planes):
    tiles = []
    for t in range(len(data)//bpt):
        px = []
        for y in range(8):
            row = []
            for x in range(8):
                v = 0
                for p in range(planes):
                    b = data[t*bpt + y*planes + p]
                    v |= ((b >> (7-x)) & 1) << p
                row.append(v)
            px.append(row)
        tiles.append(px)
    return tiles
if plat == "gb":
    bt, st = decode(arr("bkg_tiles"), 16, 2), decode(arr("spr_tiles"), 16, 2)
    pal = [(8, 24, 32), (52, 104, 86), (136, 192, 112), (224, 248, 208)]
    bpal = spal = pal; W, H = 160, 144
else:
    bt, st = decode(arr("bkg_tiles"), 32, 4), decode(arr("spr_tiles"), 32, 4)
    c = lambda b: ((b & 3) * 85, ((b >> 2) & 3) * 85, ((b >> 4) & 3) * 85)
    bpal = [c(0x00), c(0x14), c(0x2A), c(0x3F)]; spal = [c(0x00), c(0x03), c(0x3C), c(0x3F)]; W, H = 256, 192
lines = open(dump).read().split("\n")
cols, rows = map(int, lines[0].split()); m = list(map(int, lines[1].split()))
n = int(lines[2]); sp = [tuple(map(int, l.split())) for l in lines[3:3+n]]
im = Image.new("RGB", (W, H))
for r in range(min(rows, H//8)):
    for cc in range(cols):
        t = bt[m[r*cols+cc]]
        for y in range(8):
            for x in range(8): im.putpixel((cc*8+x, r*8+y), bpal[t[y][x]])
for x0, y0, tile in sp:
    t = st[tile]
    for y in range(8):
        for x in range(8):
            v = t[y][x]
            if v and 0 <= x0+x < W and 0 <= y0+y < H: im.putpixel((x0+x, y0+y), spal[v])
sc = 4 if plat == "gb" else 3
im.resize((W*sc, H*sc), Image.NEAREST).save(out)
