#!/usr/bin/env python3
"""Generates tile data for Game Boy (2bpp) and Master System (4bpp planar) from one set of 4-colour art.
Index 0 = transparent/space, 1 = dark, 2 = mid, 3 = bright.  Run from the project root:  python3 tools/gen_tiles.py"""
import os, sys
from PIL import Image, ImageDraw, ImageFont

def T(*rows):
    assert len(rows) == 8 and all(len(r) == 8 for r in rows), rows
    return [[int(c) if c != '.' else 0 for c in r] for r in rows]
def mirror(t): return [r[::-1] for r in t]
def split16(big):  # 16x16 -> TL,TR,BL,BR
    return [[row[x:x+8] for row in big[y:y+8]] for y in (0, 8) for x in (0, 8)]

P_R_T = T("..3333..", ".333311.", ".333311.", ".333333.", "..2222..", "2233332.", "2233332.", "2233332.")
P_R_B = T("2233332.", ".233332.", "..2222..", "..3..3..", "..3..3..", "..3..3..", ".11..11.", ".11..11.")
FLAME0 = T("..3333..", "..3223..", "...22...", "...22...", "...11...", "........", "........", "........")
FLAME1 = T("..3333..", "...33...", "...22...", "....2...", "...11...", "........", "........", "........")
BEAM = T("........", "........", "..2222..".replace('.', '2'), "33333333", "33333333", "22222222", "........", "........")
EXP0 = T("........", "..3..3..", "...33...", ".3.33.3.", ".3.33.3.", "...33...", "..3..3..", "........")
EXP1 = T("3..3..3.", ".3.22.3.", "..2..2..", "32.33.23", "32.33.23", "..2..2..", ".3.22.3.", "3..3..3.")
POD = T("..3333..", ".333333.", ".322223.", ".322223.", ".322223.", ".322223.", ".333333.", "..1111..")
SAUCER0 = T("........", "...22...", "..2332..", ".333333.", "22222222", ".3.33.3.", "........", "........")
SAUCER1 = T("........", "...22...", "..2332..", ".333333.", "22222222", "..3.33.3", "........", "........")
SPORE0 = T("3.2222.3", ".233332.", "23133132", "23333332", "23333332", ".233332.", "3.2222.3", "........")
SPORE1 = T(".32222.3", "32333323", ".231132.", "23333332", "23333332", ".231132.", "32333323", ".32222.3")
ORB0 = T("..2222..", ".233222.", "23322222", "22222222", "22222222", "22222112", ".222112.", "..2222..")
ORB1 = T("........", "..2222..", ".233222.", "23322222", "22222222", ".222112.", "..2222..", "........")
BAT0 = T("3......3", "33.22.33", ".332233.", "..3223..", "...22...", "........", "........", "........")
BAT1 = T("........", "........", "..3223..", ".332233.", "33.22.33", "3......3", "........", "........")

def body(c): return "23333321"[c]
def seg0():
    g = [[0]*16 for _ in range(16)]
    for r in range(16):
        for c in range(8): g[r][4+c] = int(body(c))
    for c in range(8): g[3][4+c] = 1
    for r in range(6, 16):
        w = min(r-5, 4)
        for k in range(w):
            g[r][3-k] = 2 if k else 1
            g[r][12+k] = 2 if k else 1
    for c in range(6, 10): g[15][c] = 1
    return g
def seg1():
    g = [[0]*16 for _ in range(16)]
    for r in range(16):
        for c in range(8): g[r][4+c] = int(body(c))
    for c in range(8): g[0][4+c] = 1; g[15][4+c] = 1
    for (r, cols) in {5: {7: 1, 8: 1}, 6: {6: 1, 7: 2, 8: 2, 9: 1}, 7: {6: 1, 7: 2, 8: 3, 9: 1}, 8: {6: 1, 7: 2, 8: 2, 9: 1}, 9: {6: 1, 7: 2, 8: 2, 9: 1}, 10: {7: 1, 8: 1}}.items():
        for c, v in cols.items(): g[r][c] = v
    return g
def seg2():
    g = [[0]*16 for _ in range(16)]
    for r in range(16):
        w = 2 + 2*((r*3)//15); left = 8 - w//2
        for k in range(w):
            g[r][left+k] = 2 if k == 0 else (1 if k == w-1 else 3)
    return g

PL_L = T("..2222..", "..3..3..", ".3222232"[:8].replace('2', '2'), "..3333..", "........", "........", "........", "........")  # placeholder (unused)
PLM = T("33333333", "22322232", "22222222", "12212221", "11111111", "........", "........", "........")
PLL = T(".3333333", "22322232", "22222222", "12212221", ".1111111", "........", "........", "........")
PLR = mirror(PLL)
GROUND = T("33333333", "22222222", "21222122", "11211121", "11111111", "12111211", "11111111", "11111111")
STAR0 = T("........", "........", ".....2..", "........", "........", "........", "........", "........")
STAR1 = T("........", "........", "........", "........", "........", "..3.....", "........", "........")

# ---- font from a TTF (8px), logo letters scaled 2x
def find_font():
    for p in ["/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", "/usr/share/fonts/dejavu/DejaVuSansMono-Bold.ttf",
              "/usr/share/fonts/truetype/liberation/LiberationMono-Bold.ttf"]:
        if os.path.exists(p): return p
    return None
FONT = find_font()
def glyph(ch):
    img = Image.new("L", (8, 8), 0)
    d = ImageDraw.Draw(img)
    f = ImageFont.truetype(FONT, 8) if FONT else ImageFont.load_default()
    w = d.textlength(ch, font=f)
    d.text(((8-w)/2, 0), ch, font=f, fill=255)
    return [[3 if img.getpixel((x, y)) > 110 else 0 for x in range(8)] for y in range(8)]
CHARS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
LOGO = "JETPAK"
def logo(ch):
    g = glyph(ch)
    # draw bigger: render at 14px for a crisper 16x16 letter
    img = Image.new("L", (16, 16), 0); d = ImageDraw.Draw(img)
    f = ImageFont.truetype(FONT, 15) if FONT else ImageFont.load_default()
    w = d.textlength(ch, font=f); d.text(((16-w)/2, 0), ch, font=f, fill=255)
    big = [[3 if img.getpixel((x, y)) > 110 else 0 for x in range(16)] for y in range(16)]
    # add a mid-tone drop shadow
    for y in range(15, 0, -1):
        for x in range(15, 0, -1):
            if big[y][x] == 0 and big[y-1][x-1] == 3: big[y][x] = 1
    return split16(big)

SPR = [("PR_T", P_R_T), ("PR_B", P_R_B), ("PL_T", mirror(P_R_T)), ("PL_B", mirror(P_R_B)), ("FLAME0", FLAME0), ("FLAME1", FLAME1),
       ("BEAM", BEAM), ("EXP0", EXP0), ("EXP1", EXP1), ("POD", POD),
       ("AL0", SAUCER0), ("AL0b", SAUCER1), ("AL1", SPORE0), ("AL1b", SPORE1), ("AL2", ORB0), ("AL2b", ORB1), ("AL3", BAT0), ("AL3b", BAT1)]
for i, g in enumerate((seg0(), seg1(), seg2())):
    for j, t in enumerate(split16(g)): SPR.append((f"SEG{i}_{j}", t))
BKG = [("BLANK", T(*["........"]*8)), ("STAR0", STAR0), ("STAR1", STAR1), ("PLL", PLL), ("PLM", PLM), ("PLR", PLR), ("GROUND", GROUND)]
FONT_BASE = len(BKG)
for ch in CHARS: BKG.append(("F_"+ch, glyph(ch)))
LOGO_BASE = len(BKG)
for ch in LOGO:
    for j, t in enumerate(logo(ch)): BKG.append((f"L_{ch}{j}", t))

def enc_gb(t):
    out = []
    for row in t:
        lo = sum(((v & 1) << (7-i)) for i, v in enumerate(row)); hi = sum((((v >> 1) & 1) << (7-i)) for i, v in enumerate(row))
        out += [lo, hi]
    return out
def enc_sms(t):
    out = []
    for row in t:
        for plane in range(4):
            out.append(sum((((v >> plane) & 1) << (7-i)) for i, v in enumerate(row)))
    return out
def carr(name, tiles, enc):
    data = [b for _, t in tiles for b in enc(t)]
    lines = [", ".join("0x%02X" % b for b in data[i:i+16]) for i in range(0, len(data), 16)]
    return f"const unsigned char {name}[] = {{\n  " + ",\n  ".join(lines) + "\n};\n"

root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ids = ["/* generated by tools/gen_tiles.py - do not edit */", "#ifndef TILEIDS_H", "#define TILEIDS_H"]
sidx = {n: i for i, (n, _) in enumerate(SPR)}
ids += [f"#define S_{n} {i}" for n, i in sidx.items() if n in ("PR_T", "PR_B", "PL_T", "PL_B", "FLAME0", "FLAME1", "BEAM", "EXP0", "EXP1", "POD")]
ids += [f"#define S_AL {sidx['AL0']}", f"#define S_SEG {sidx['SEG0_0']}", f"#define N_SPR {len(SPR)}"]
bidx = {n: i for i, (n, _) in enumerate(BKG)}
ids += [f"#define B_{n} {bidx[n]}" for n in ("BLANK", "STAR0", "STAR1", "PLL", "PLM", "PLR", "GROUND")]
ids += [f"#define B_FONT {FONT_BASE}", f"#define B_LOGO {LOGO_BASE}", f"#define N_BKG {len(BKG)}", "#endif", ""]
open(os.path.join(root, "common/tileids.h"), "w").write("\n".join(ids))
open(os.path.join(root, "gb/tiles_gb.h"), "w").write("#include \"tileids.h\"\n" + carr("spr_tiles", SPR, enc_gb) + carr("bkg_tiles", BKG, enc_gb))
open(os.path.join(root, "sms/tiles_sms.h"), "w").write("#include \"tileids.h\"\n" + carr("spr_tiles", SPR, enc_sms) + carr("bkg_tiles", BKG, enc_sms))

# preview sheet (GB greys on the left, SMS-ish colours on the right)
def sheet(tiles, pal, path, cols=16, sc=6):
    rows = (len(tiles)+cols-1)//cols
    im = Image.new("RGB", (cols*8*sc, rows*8*sc), pal[0])
    for n, (_, t) in enumerate(tiles):
        for y in range(8):
            for x in range(8):
                for dy in range(sc):
                    for dx in range(sc):
                        im.putpixel(((n % cols)*8+x)*sc+dx, ((n//cols)*8+y)*sc+dy) if False else None
                c = pal[t[y][x]]
                ox, oy = ((n % cols)*8+x)*sc, ((n//cols)*8+y)*sc
                im.paste(c, (ox, oy, ox+sc, oy+sc))
    im.save(path)
gbpal = [(15, 56, 15), (48, 98, 48), (139, 172, 15), (155, 188, 15)]
gbpal = [(8, 24, 32), (52, 104, 86), (136, 192, 112), (224, 248, 208)]
smspal = [(0, 0, 0), (255, 0, 0), (0, 255, 255), (255, 255, 255)]
sheet(SPR, smspal, os.path.join(root, "art_sprites_sms.png")); sheet(SPR, gbpal, os.path.join(root, "art_sprites_gb.png"))
sheet(BKG, gbpal, os.path.join(root, "art_background_gb.png"))
print("sprites", len(SPR), "bkg", len(BKG), "font:", FONT)
