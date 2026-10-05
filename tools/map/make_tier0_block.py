"""Draws the first version of the Tier 0 block map (data/tier0/map/block.toml).

The TOML file is the source of truth and can be hand-edited afterwards; this script only made
the first draft. Legend: data/tier0/map/README.md. Coordinates are cells (x right, y down).
Run: python3 tools/map/make_tier0_block.py > data/tier0/map/block.toml
"""

W, H = 32, 28
LEVELS = [-1, 0, 1]


class Level:
    def __init__(self, z, fill):
        self.z = z
        self.floor = [[fill] * W for _ in range(H)]
        self.h = [[" "] * W for _ in range(H + 1)]   # top edge of cell (x, y)
        self.v = [[" "] * (W + 1) for _ in range(H)]  # left edge of cell (x, y)

    def area(self, x0, y0, x1, y1, f):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.floor[y][x] = f

    def box(self, x0, y0, x1, y1, c):
        """Walls around the cells x0..x1, y0..y1."""
        for x in range(x0, x1 + 1):
            self.h[y0][x] = c
            self.h[y1 + 1][x] = c
        for y in range(y0, y1 + 1):
            self.v[y][x0] = c
            self.v[y][x1 + 1] = c

    def hwall(self, y, x0, x1, c):  # along the top edge of row y
        for x in range(x0, x1 + 1):
            self.h[y][x] = c

    def vwall(self, x, y0, y1, c):  # along the left edge of column x
        for y in range(y0, y1 + 1):
            self.v[y][x] = c

    def text(self):
        rows = []
        for y in range(H + 1):
            rows.append("".join("+" + self.h[y][x] for x in range(W)) + "+")
            if y < H:
                rows.append("".join(self.v[y][x] + self.floor[y][x] for x in range(W)) + self.v[y][W])
        return "\n".join(r.rstrip() for r in rows)


ground = Level(0, '"')
first = Level(1, " ")
basement = Level(-1, " ")

# Street and alley.
ground.area(0, 0, W - 1, 2, "_")
ground.area(17, 3, 18, H - 1, ":")

# Main building, ground floor: front room, kitchen, workshop.
ground.area(2, 4, 15, 9, ".")
ground.area(2, 10, 8, 15, ".")
ground.area(9, 10, 15, 15, ",")
ground.box(2, 4, 15, 15, "#")
ground.hwall(10, 2, 15, "-")
ground.h[10][5] = "D"
ground.h[10][12] = "D"
ground.vwall(9, 10, 15, "|")
ground.v[12][9] = "D"
ground.h[4][8] = "E"   # front door to the street
ground.h[16][12] = "E"  # back door from the workshop to the yard
for x in (4, 12):
    ground.h[4][x] = "W"
for y in (6, 12):
    ground.v[y][16] = "W"
for y in (7, 12):
    ground.v[y][2] = "W"

# First floor: hallway and three bedrooms.
first.area(2, 4, 15, 15, ".")
first.box(2, 4, 15, 15, "#")
first.hwall(7, 2, 15, "-")
for x in (4, 9, 13):
    first.h[7][x] = "D"
first.vwall(7, 7, 15, "|")
first.vwall(12, 7, 15, "|")
for x in (6, 11):
    first.h[4][x] = "W"
for x in (4, 9, 13):
    first.h[16][x] = "W"

# Basement: storage and boiler room, no windows.
basement.area(2, 4, 15, 15, ",")
basement.box(2, 4, 15, 15, "#")
basement.vwall(9, 4, 15, "|")
basement.v[8][9] = "D"

# Yard behind the main building, fenced, with a gate to the alley.
ground.vwall(2, 16, 24, "=")
ground.vwall(16, 16, 24, "=")
ground.hwall(25, 2, 15, "=")
ground.v[20][16] = "G"

# House A: living room, bedroom, kitchen.
ground.area(20, 4, 28, 11, ".")
ground.box(20, 4, 28, 11, "#")
ground.hwall(8, 20, 28, "-")
ground.h[8][22] = "D"
ground.h[8][26] = "D"
ground.vwall(25, 8, 11, "|")
ground.h[4][24] = "E"
for x in (21, 27):
    ground.h[4][x] = "W"
for y in (6, 9):
    ground.v[y][20] = "W"
for y in (6, 10):
    ground.v[y][29] = "W"

# House B: living room, kitchen, windowless storeroom.
ground.area(20, 15, 28, 22, ".")
ground.box(20, 15, 28, 22, "#")
ground.hwall(19, 20, 28, "-")
ground.h[19][22] = "D"
ground.vwall(26, 19, 22, "|")
ground.v[20][26] = "D"
ground.v[16][20] = "E"
for x in (22, 26):
    ground.h[15][x] = "W"
ground.v[21][20] = "W"
ground.h[23][22] = "W"

stairs = [((0, 3, 5), (1, 3, 5)), ((0, 3, 14), (-1, 3, 14))]
objects = [
    ("pop_start", (0, 8, 7), "player"),
    ("survivor", (0, 22, 10), "neighbour"),
    ("survivor", (0, 27, 21), "technician"),
    ("arrival", (0, 17, 20), "stranger"),
    ("generator", (0, 10, 18), ""),
    ("power_panel", (0, 15, 13), ""),
    ("fridge", (0, 2, 11), ""),
    ("stove", (0, 4, 10), ""),
    ("hot_plate", (0, 6, 10), ""),
    ("radio", (0, 14, 11), ""),
    ("stockpile", (0, 13, 14), "workshop"),
    ("lamp", (0, 8, 6), "front_room"),
    ("lamp", (0, 5, 13), "kitchen"),
    ("lamp", (0, 12, 13), "workshop"),
    ("lamp", (1, 4, 11), "bedroom_1"),
    ("lamp", (-1, 5, 9), "basement"),
]

print("# Tier 0 block: main building (basement, ground, first floor) with a yard, and two houses.")
print("# Spec: docs/tier0/specs/09-site-map.md. Legend: README.md next to this file.")
print("# First drawn by tools/map/make_tier0_block.py; this file is the source of truth.\n")
print("[map]")
print('name = "Tier 0 block"')
print(f"width = {W}")
print(f"height = {H}")
print("build_quality = 50\n")
for lv in (basement, ground, first):
    print("[[level]]")
    print(f"z = {lv.z}")
    print("grid = '''")
    print(lv.text())
    print("'''\n")
for a, b in stairs:
    print("[[stairs]]")
    print(f"a = [{a[0]}, {a[1]}, {a[2]}]")
    print(f"b = [{b[0]}, {b[1]}, {b[2]}]\n")
for kind, at, name in objects:
    print("[[object]]")
    print(f'kind = "{kind}"')
    print(f"at = [{at[0]}, {at[1]}, {at[2]}]")
    if name:
        print(f'id = "{name}"')
    print()
