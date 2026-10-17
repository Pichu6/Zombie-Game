"""Earth's rainy seasons and Köppen-Geiger classes from WorldClim 2.1 monthly normals (1970-2000, 10 arc-minutes, CC BY 4.0),
for spec 100 (research note rain-seasons, RS). Land pixels weighted by area (cos latitude).

Measures: the Köppen-Geiger main groups and second letters by Peel et al. (2007)'s rules (validated against their global
totals); by 10-degree latitude band: the share of the year's rain in the local summer half (April-September north,
October-March south), the share in the wettest three consecutive months, and Walsh and Lawler's seasonality index.
Usage: python -I tools/data/earth_seasons.py ../data_raw/geo/worldclim/unzipped
"""
import sys
from pathlib import Path

import numpy as np
import tifffile


def grid(path):
    with tifffile.TiffFile(path) as t:
        a = t.pages[0].asarray().astype(np.float64)
    a[a < -1e30] = np.nan
    return a


def koppen(P, T, lat):
    """Peel et al. (2007) Table 1. P, T: (12, n) monthly mm and degC; lat: (n,). Returns class strings."""
    n = P.shape[1]
    MAP = P.sum(0)
    MAT = T.mean(0)
    Thot, Tcold = T.max(0), T.min(0)
    Tmon10 = (T > 10).sum(0)
    Pdry = P.min(0)
    north = lat >= 0
    summer = np.zeros_like(P, dtype=bool)
    summer[3:9, :] = north  # AMJJAS
    summer[[0, 1, 2, 9, 10, 11], :] = ~north
    Psum = np.where(summer, P, 0).sum(0)
    Pwin = MAP - Psum
    Psdry = np.where(summer, P, np.inf).min(0)
    Pwdry = np.where(~summer, P, np.inf).min(0)
    Pswet = np.where(summer, P, -np.inf).max(0)
    Pwwet = np.where(~summer, P, -np.inf).max(0)
    Pth = np.where(Pwin >= 0.7 * MAP, 2 * MAT, np.where(Psum >= 0.7 * MAP, 2 * MAT + 28, 2 * MAT + 14))
    out = np.empty(n, dtype=object)
    for i in range(n):
        if Thot[i] < 10:
            out[i] = "ET" if Thot[i] > 0 else "EF"
            continue
        if MAP[i] < 10 * Pth[i]:
            w = "W" if MAP[i] < 5 * Pth[i] else "S"
            out[i] = "B" + w + ("h" if MAT[i] >= 18 else "k")
            continue
        if Tcold[i] >= 18:
            if Pdry[i] >= 60:
                out[i] = "Af"
            elif Pdry[i] >= 100 - MAP[i] / 25:
                out[i] = "Am"
            else:
                out[i] = "Aw"
            continue
        g = "C" if Tcold[i] > 0 else "D"
        s = Psdry[i] < 40 and Psdry[i] < Pwwet[i] / 3
        w = Pwdry[i] < Pswet[i] / 10
        if s and w:
            s, w = (False, True) if Psum[i] > Pwin[i] else (True, False)
        out[i] = g + ("s" if s else "w" if w else "f")
    return out


def main(root):
    root = Path(root)
    P = np.stack([grid(root / f"wc2.1_10m_prec_{m:02d}.tif") for m in range(1, 13)])
    T = np.stack([grid(root / f"wc2.1_10m_tavg_{m:02d}.tif") for m in range(1, 13)])
    rows, cols = P.shape[1:]
    lat = 90 - (np.arange(rows) + 0.5) * 180 / rows
    LAT = np.repeat(lat[:, None], cols, 1)
    ok = np.isfinite(P).all(0) & np.isfinite(T).all(0)
    w = np.cos(np.radians(LAT))[ok]
    p = P[:, ok]
    t = T[:, ok]
    la = LAT[ok]
    cls = koppen(p, t, la)
    total = w.sum()
    nonant = la > -60
    print(f"land pixels {ok.sum()}; area weights sum {total:.0f}")
    for label, mask in (("all land", np.ones_like(nonant)), ("without Antarctica", nonant)):
        tw = w[mask].sum()
        grp = {}
        for c, ww in zip(cls[mask], w[mask]):
            grp[c[0]] = grp.get(c[0], 0) + ww
        print(label, "groups:", {k: round(100 * v / tw, 1) for k, v in sorted(grp.items())})
    # Second letters among A, C and D without Antarctica (the classes the generator reproduces).
    tw = w[nonant].sum()
    sub = {}
    for c, ww in zip(cls[nonant], w[nonant]):
        key = c if c[0] == "A" else (c[:2] if c[0] in "CD" else c[0] + c[1] if c[0] == "B" else c)
        sub[key] = sub.get(key, 0) + ww
    print("classes (% of land without Antarctica):", {k: round(100 * v / tw, 1) for k, v in sorted(sub.items())})
    # By latitude band: summer share, wettest-three-months share, seasonality index, mean rain.
    annual = p.sum(0)
    north = la >= 0
    summer = np.where(north, p[3:9].sum(0), p[[0, 1, 2, 9, 10, 11]].sum(0))
    roll = np.stack([p[[m % 12, (m + 1) % 12, (m + 2) % 12]].sum(0) for m in range(12)])
    wet3 = roll.max(0)
    si = np.abs(p - annual / 12).sum(0) / np.maximum(annual, 1e-9)
    has = annual > 50
    print("band  land%  rain_mm  summer_share  wettest3_share  SI")
    for b0 in range(-60, 90, 10):
        m = (la >= b0) & (la < b0 + 10) & has
        if w[m].sum() <= 0:
            continue
        ww = w[m] * annual[m]  # rain-weighted shares
        print(f"{b0:+4d} {100 * w[(la >= b0) & (la < b0 + 10)].sum() / total:6.1f} {np.average(annual[m], weights=w[m]):8.0f} "
              f"{(summer[m] * w[m]).sum() / ww.sum():8.2f} {(wet3[m] * w[m]).sum() / ww.sum():8.2f} {np.average(si[m], weights=w[m]):8.2f}")


if __name__ == "__main__":
    main(sys.argv[1])
