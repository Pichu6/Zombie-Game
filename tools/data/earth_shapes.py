"""Earth's land shapes from ETOPO 2022, as targets for the planet generator (spec 89, research continents-and-coasts).

Measures, on a 0.25-degree grid: the landmasses (connected land, the date line joined) and their areas; the share of
land within 50 / 100 / 200 / 400 / 800 km of the sea; the land area by 10-degree latitude band and beyond the polar
circles; the share of land in islands under 1 million km2. The sea is ocean-connected water: every below-zero water
body of 400,000 km2 or more (the Caspian stays a lake), as in earth_targets.py.

Usage: python tools/data/earth_shapes.py [data_raw/geo]   (about 1-2 minutes)
"""
import math
import sys
from pathlib import Path

import numpy as np
import tifffile
from scipy import ndimage
from scipy.spatial import cKDTree

R_KM = 6371.0
STEP = 15  # 1-arcminute pixels per 0.25-degree pixel


def geotiff(path):
    with tifffile.TiffFile(path) as tif:
        page = tif.pages[0]
        scale = page.tags["ModelPixelScaleTag"].value
        tie = page.tags["ModelTiepointTag"].value
        arr = page.asarray()
    return arr, (tie[3], tie[4], scale[0], scale[1])


def label_wrapped(mask):
    """8-connected components with the date line joined; returns labels (0 = background) and the count."""
    lab, n = ndimage.label(mask, structure=np.ones((3, 3), dtype=int))
    parent = np.arange(n + 1)

    def find(a):
        while parent[a] != a:
            parent[a] = parent[parent[a]]
            a = parent[a]
        return a

    H = mask.shape[0]
    for r in range(H):
        for dr in (-1, 0, 1):
            rr = r + dr
            if 0 <= rr < H:
                a, b = lab[r, 0], lab[rr, -1]
                if a and b:
                    fa, fb = find(a), find(b)
                    if fa != fb:
                        parent[max(fa, fb)] = min(fa, fb)
    roots = np.array([find(i) for i in range(n + 1)])
    return roots[lab], n


def main(root):
    root = Path(root)
    elev, geo = geotiff(root / "etopo" / "ETOPO_2022_v1_60s_surface.tif")
    H1, W1 = elev.shape
    H, W = H1 // STEP, W1 // STEP
    land_frac = (elev[: H * STEP, : W * STEP] > 0).reshape(H, STEP, W, STEP).mean(axis=(1, 3))
    del elev
    land = land_frac >= 0.5
    lat = 90 - (np.arange(H) + 0.5) * 180 / H
    dlat, dlon = math.radians(180 / H), math.radians(360 / W)
    band_area = R_KM ** 2 * dlon * (np.sin(np.radians(lat) + dlat / 2) - np.sin(np.radians(lat) - dlat / 2))
    area = np.repeat(band_area[:, None], W, axis=1)

    # The sea: water components of 400,000 km2 or more.
    wlab, wn = label_wrapped(~land)
    wareas = ndimage.sum(area, wlab, index=np.arange(wn + 1))
    sea = (~land) & (wareas[wlab] >= 4e5)
    land_area = float(area[land].sum())
    print(f"grid {H} x {W}; land {land_area / 1e6:.1f} million km2 ({100 * land_area / area.sum():.1f} % of the surface)")

    # Landmasses.
    llab, ln = label_wrapped(land)
    lareas = ndimage.sum(area, llab, index=np.arange(ln + 1))
    sizes = np.sort(lareas[1:][lareas[1:] > 0])[::-1]
    print("largest landmasses (million km2, share of land): " +
          ", ".join(f"{a / 1e6:.1f} ({100 * a / land_area:.1f} %)" for a in sizes[:8]))
    for t in (1e6, 1e5, 1e4):
        print(f"landmasses of {t:,.0f} km2 or more: {int((sizes >= t).sum())}")
    print(f"land in landmasses under 1 million km2 (islands): {100 * sizes[sizes < 1e6].sum() / land_area:.1f} %")

    # Distance from land to the sea (great circle to the nearest sea pixel touching land).
    lat2 = np.repeat(lat[:, None], W, axis=1)
    lon2 = np.repeat((-180 + (np.arange(W) + 0.5) * 360 / W)[None, :], H, axis=0)
    def xyz(m):
        la, lo = np.radians(lat2[m]), np.radians(lon2[m])
        return np.stack([np.cos(la) * np.cos(lo), np.cos(la) * np.sin(lo), np.sin(la)], axis=-1)
    coast_sea = sea & ndimage.binary_dilation(land, structure=np.ones((3, 3)))
    tree = cKDTree(xyz(coast_sea))
    d, _ = tree.query(xyz(land))
    km = 2 * R_KM * np.arcsin(np.clip(d / 2, 0, 1))
    la = area[land]
    print("land within 50 / 100 / 200 / 400 / 800 km of the sea: " +
          " / ".join(f"{100 * la[km <= x].sum() / land_area:.1f}" for x in (50, 100, 200, 400, 800)) + " %")
    print(f"median distance of land to the sea: {np.median(np.repeat(km, 1)):.0f} km (area-weighted: "
          f"{np.sort(km)[np.searchsorted(np.cumsum(la[np.argsort(km)]), land_area / 2)]:.0f} km)")

    # Latitude.
    rows = []
    for b in range(-90, 90, 10):
        m = land & (lat2 >= b) & (lat2 < b + 10)
        rows.append(f"{b:+d}..{b + 10:+d}: {100 * area[m].sum() / land_area:.1f}")
    print("land by latitude band (% of land): " + "; ".join(rows))
    north = area[land & (lat2 >= 66.56)].sum() / land_area
    south = area[land & (lat2 <= -66.56)].sum() / land_area
    print(f"land beyond the Arctic circle {100 * north:.1f} %, beyond the Antarctic circle {100 * south:.1f} %")
    nh = area[land & (lat2 >= 0)].sum() / land_area
    print(f"land in the northern hemisphere {100 * nh:.1f} %")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else Path(__file__).resolve().parents[3] / "data_raw" / "geo")
