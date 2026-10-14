"""Real-world targets for spec 86 from open rasters: people against coast, height and big rivers,
measured on Earth at the one world's cell size (about 5,000 km2, frequency 101).

Inputs (data_raw/geo): GHSL GHS-POP E2020 30ss (people 2020), ETOPO 2022 60s surface (height),
HydroRIVERS v1.0 (river reaches with long-term discharge and upstream area).
Usage: python -I tools/data/earth_targets.py ../data_raw/geo   (research note earth-measured-open-data, EM)
"""
import math
import sys
import zipfile
from pathlib import Path

import numpy as np
import shapefile
import tifffile
from scipy import ndimage
from scipy.spatial import cKDTree

R_KM = 6371.0
CELL_KM2 = 510.07e6 / (10 * 101 ** 2 + 2)  # one world's cell area (spec 30, frequency 101)


def geotiff(path):
    with tifffile.TiffFile(path) as tif:
        page = tif.pages[0]
        scale = page.tags["ModelPixelScaleTag"].value
        tie = page.tags["ModelTiepointTag"].value
        arr = page.asarray()
    return arr, (tie[3], tie[4], scale[0], scale[1])  # west, north, dx, dy


def lat_lon_of_rows_cols(shape, geo):
    west, north, dx, dy = geo
    lat = north - (np.arange(shape[0]) + 0.5) * dy
    lon = west + (np.arange(shape[1]) + 0.5) * dx
    return lat, lon


class EqualAreaGrid:
    """Latitude bands about sqrt(CELL_KM2) tall, each cut into cells of about the same area."""

    def __init__(self):
        side_deg = math.sqrt(CELL_KM2) / 111.195
        self.nb = int(round(180 / side_deg))
        self.dlat = 180 / self.nb
        mids = -90 + (np.arange(self.nb) + 0.5) * self.dlat
        self.n_lon = np.maximum(1, np.round(360 * np.cos(np.radians(mids)) / self.dlat)).astype(np.int64)
        self.offset = np.concatenate([[0], np.cumsum(self.n_lon)])
        self.count = int(self.offset[-1])

    def cell(self, lat, lon):
        b = np.clip(((lat + 90) / self.dlat).astype(np.int64), 0, self.nb - 1)
        n = self.n_lon[b]
        j = np.clip(((lon + 180) / 360 * n).astype(np.int64), 0, n - 1)
        return self.offset[b] + j


def unit_vectors(lat, lon):
    la, lo = np.radians(lat), np.radians(lon)
    return np.stack([np.cos(la) * np.cos(lo), np.cos(la) * np.sin(lo), np.sin(la)], axis=-1)


def chord_to_km(c):
    return 2 * R_KM * np.arcsin(np.clip(c / 2, 0, 1))


def weighted_median(values, weights):
    o = np.argsort(values)
    cw = np.cumsum(weights[o])
    return float(values[o][np.searchsorted(cw, cw[-1] / 2)])


def river_points(rdir):
    """Vertices of the reaches of spec 52 rule 4 (100 m3/s and 100,000 km2), flagged when large
    (1,000 m3/s and 500,000 km2); cached beside the shapefile."""
    cache = rdir / "rule4_points.npz"
    if cache.exists():
        z = np.load(cache)
        return z["pts"], z["large"], float(z["max_dis"])
    sf = shapefile.Reader(str(next(rdir.rglob("*.shp"))))
    pts_l, large_l, max_dis = [], [], 0.0
    for k, rec in enumerate(sf.iterRecords(fields=["DIS_AV_CMS", "UPLAND_SKM"])):
        dis, up = rec["DIS_AV_CMS"], rec["UPLAND_SKM"]
        max_dis = max(max_dis, dis)
        if dis >= 100 and up >= 1e5:
            p = np.asarray(sf.shape(k).points)
            pts_l.append(p)
            large_l.append(np.full(len(p), dis >= 1000 and up >= 5e5))
    pts, large = np.concatenate(pts_l), np.concatenate(large_l)
    np.savez(cache, pts=pts, large=large, max_dis=max_dis)
    return pts, large, max_dis


def main(root):
    root = Path(root)
    out = []

    def say(s):
        print(s, flush=True)
        out.append(s)

    # Height, 1 arcmin.
    elev, egeo = geotiff(root / "etopo" / "ETOPO_2022_v1_60s_surface.tif")
    elev = elev.astype(np.float32)
    say(f"ETOPO {elev.shape} geo {egeo}")

    # People, 30 arcsec summed to the ETOPO grid.
    zpath = root / "ghsl" / "GHS_POP_E2020_30ss.zip"
    tif_dir = root / "ghsl" / "unzipped"
    if not tif_dir.exists():
        with zipfile.ZipFile(zpath) as z:
            z.extractall(tif_dir)
    tif_path = next(tif_dir.rglob("*.tif"))
    pop30, pgeo = geotiff(tif_path)
    say(f"GHSL {pop30.shape} geo {pgeo}")
    # Each 30ss pixel goes to the 60s pixel holding its centre (the two grids are not aligned).
    H, W = elev.shape
    plat, plon = lat_lon_of_rows_cols(pop30.shape, pgeo)
    tcol = (np.floor((plon - egeo[0]) / egeo[2]).astype(np.int64)) % W
    trow = np.clip(np.floor((egeo[1] - plat) / egeo[3]).astype(np.int64), 0, H - 1)
    pop = np.zeros(H * W)
    for s in range(0, pop30.shape[0], 512):
        block = pop30[s:s + 512]
        r, c = np.nonzero(block > 0)
        pop += np.bincount(trow[s + r] * W + tcol[c], weights=block[r, c], minlength=H * W)
    del pop30
    pop = pop.reshape(H, W)
    total = pop.sum()
    say(f"people {total / 1e9:.3f} billion")

    lat, lon = lat_lon_of_rows_cols(elev.shape, egeo)
    px_km2 = (1.853 ** 2) * np.cos(np.radians(lat))[:, None] * np.ones((1, elev.shape[1]))

    # Ocean: below-0 water bodies of 400,000 km2 or more (oceans, the Mediterranean, the Black Sea;
    # the Caspian, 371,000 km2, stays inland).
    wet = elev <= 0
    labels, n = ndimage.label(wet)
    areas = ndimage.sum(px_km2, labels, index=np.arange(1, n + 1))
    big = np.nonzero(areas >= 4e5)[0] + 1
    ocean = np.isin(labels, big)
    del labels, wet
    land = ~ocean
    say(f"ocean bodies kept {len(big)}, land {px_km2[land].sum() / 1e6:.1f} M km2")
    unpeopled_sea = pop[ocean].sum() / total
    say(f"people on ocean pixels (coastline slivers) {unpeopled_sea * 100:.2f} %")

    # Height of people.
    pm = pop > 0
    hv, pv = elev[pm], pop[pm]
    say(f"median height of people {weighted_median(hv, pv):.0f} m")
    for cut in (100, 200, 500, 1000, 1500):
        say(f"below {cut} m: {pv[hv < cut].sum() / total * 100:.1f} %")

    # Distance to the sea.
    coast = ocean & ndimage.binary_dilation(land)
    ci, cj = np.nonzero(coast)
    tree = cKDTree(unit_vectors(lat[ci], lon[cj]))
    ri, rj = np.nonzero(pm)
    d_km = np.empty(len(ri))
    for s in range(0, len(ri), 2_000_000):
        q = unit_vectors(lat[ri[s:s + 2_000_000]], lon[rj[s:s + 2_000_000]])
        d_km[s:s + 2_000_000] = chord_to_km(tree.query(q, workers=-1)[0])
    pw = pop[ri, rj]
    for km in (10, 50, 100, 150, 200, 400):
        say(f"within {km} km of the sea: {pw[d_km <= km].sum() / total * 100:.1f} %")
    for km, cut in ((100, 100), (100, 10)):
        m = (d_km <= km) & (elev[ri, rj] < cut)
        say(f"within {km} km and below {cut} m: {pw[m].sum() / total * 100:.1f} %")

    # The one world's cells.
    grid = EqualAreaGrid()
    say(f"equal-area grid: {grid.count} cells of {CELL_KM2:.0f} km2, {grid.nb} bands")
    cell_px = grid.cell(np.repeat(lat, elev.shape[1]).reshape(elev.shape),
                        np.tile(lon, elev.shape[0]).reshape(elev.shape)).ravel()
    land_area = np.bincount(cell_px, weights=(px_km2 * land).ravel(), minlength=grid.count)
    all_area = np.bincount(cell_px, weights=px_km2.ravel(), minlength=grid.count)
    cell_pop = np.bincount(cell_px, weights=pop.ravel(), minlength=grid.count)
    cell_elev = np.bincount(cell_px, weights=(elev * px_km2 * land).ravel(), minlength=grid.count)
    del cell_px
    is_land = land_area >= 0.5 * all_area
    lcells = int(is_land.sum())
    say(f"land cells (half land or more) {lcells}, holding {cell_pop[is_land].sum() / total * 100:.1f} % of people")

    # Concentration across land cells.
    lp = np.sort(cell_pop[is_land])[::-1]
    cum = np.cumsum(lp) / lp.sum()
    for share in (0.5, 0.8, 0.9):
        k = int(np.searchsorted(cum, share)) + 1
        say(f"{share * 100:.0f} % of people live in {k / lcells * 100:.1f} % of land cells")
    dens = cell_pop[is_land] / np.maximum(land_area[is_land], 1)
    for cut in (1, 10, 100, 500):
        say(f"land cells under {cut} people/km2: {(dens < cut).mean() * 100:.1f} %")
    say(f"median land-cell density {np.median(dens):.1f} /km2, pop-weighted median {weighted_median(dens, cell_pop[is_land]):.0f} /km2")
    lp_sorted = np.sort(cell_pop[is_land])
    n = len(lp_sorted)
    gini = (2 * np.sum((np.arange(1, n + 1)) * lp_sorted) / (n * lp_sorted.sum())) - (n + 1) / n
    say(f"Gini of people across land cells {gini:.3f}")

    # Rivers: reaches of spec 52 rule 4 (100 m3/s and 100,000 km2; large 1,000 and 500,000).
    rzip = root / "hydrorivers" / "HydroRIVERS_v10_shp.zip"
    rdir = root / "hydrorivers" / "unzipped"
    if not rdir.exists():
        with zipfile.ZipFile(rzip) as z:
            z.extractall(rdir)
    pts, large, max_dis = river_points(rdir)
    river_cells = set(grid.cell(pts[:, 1], pts[:, 0]).tolist())
    large_cells = set(grid.cell(pts[large, 1], pts[large, 0]).tolist())
    river_pts, large_pts = [pts], [pts[large]]
    say(f"largest discharge {max_dis:.0f} m3/s; {len(pts)} river vertices")
    for name, cells in (("river", river_cells), ("large river", large_cells)):
        m = np.zeros(grid.count, bool)
        m[list(cells)] = True
        m &= is_land
        say(f"{name} cells: {m.sum() / lcells * 100:.1f} % of land cells, "
            f"{land_area[m].sum() / land_area[is_land].sum() * 100:.1f} % of land, "
            f"{cell_pop[m].sum() / cell_pop[is_land].sum() * 100:.1f} % of people")
        ratio = (cell_pop[m].sum() / cell_pop[is_land].sum()) / (land_area[m].sum() / land_area[is_land].sum())
        say(f"  people per km2 in {name} cells against the land's mean: {ratio:.2f}x")
        say(f"  median height of {name} cells {np.median(cell_elev[m] / np.maximum(land_area[m], 1)):.0f} m (all land cells {np.median(cell_elev[is_land] / np.maximum(land_area[is_land], 1)):.0f} m)")

    # Fine scale: people within 10 / 35 km of a river of rule 4.
    for name, pts_list in (("river", river_pts), ("large river", large_pts)):
        pts = np.concatenate(pts_list)
        rt = cKDTree(unit_vectors(pts[:, 1], pts[:, 0]))
        dr = np.empty(len(ri))
        for s in range(0, len(ri), 2_000_000):
            q = unit_vectors(lat[ri[s:s + 2_000_000]], lon[rj[s:s + 2_000_000]])
            dr[s:s + 2_000_000] = chord_to_km(rt.query(q, workers=-1)[0])
        for km in (10, 35, 100):
            say(f"within {km} km of a {name}: {pw[dr <= km].sum() / total * 100:.1f} %")

    (root / "earth_targets_out.txt").write_text("\n".join(out) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main(sys.argv[1])
