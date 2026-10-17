"""Earth's climate anomalies and winds from WorldClim 2.1 (CC BY 4.0), for spec 101 (research note droughts-and-wind, DW).

Inputs (data_raw/geo/worldclim): hist/ CRU TS 4.06 downscaled monthly precipitation 2000-2019 and maximum temperature
2010-2019 (10 arc-minutes); wind/ the 1970-2000 monthly mean wind speed at 10 m. Fields are averaged to 1-degree cells
(6 x 6 pixels, land only) to keep memory small.

Measures: the coefficient of variation of annual rain by mean annual rain; the correlation of monthly rain and
temperature anomalies against distance (the 1/e decay distance); their persistence (lag correlations); the share of land
in drought (a 12-month total under 80 % of normal) by year; the standard deviation of monthly temperature anomalies by
latitude and season; the mean wind speed over land by latitude.
Usage: python -I tools/data/earth_anomalies.py ../data_raw/geo/worldclim
"""
import math
import sys
from pathlib import Path

import numpy as np
import tifffile

B = 6  # 10' pixels per 1-degree cell


def coarse(path):
    with tifffile.TiffFile(path) as t:
        a = t.pages[0].asarray().astype(np.float64)
    a[a < -1e30] = np.nan
    r, c = a.shape
    a = a.reshape(r // B, B, c // B, B)
    ok = np.isfinite(a).sum(axis=(1, 3))
    s = np.nansum(a, axis=(1, 3))
    out = np.where(ok >= B * B // 2, s / np.maximum(ok, 1), np.nan)
    return out


def series(folder, var, years):
    out = []
    for y in years:
        for m in range(1, 13):
            f = next(Path(folder).glob(f"*_{var}_{y}-{m:02d}.tif"))
            out.append(coarse(f))
    return np.stack(out)  # (months, rows, cols)


def decay(anom, lat, lon, rng, pairs=60000):
    """Correlation of anomaly series against great-circle distance; returns (bins, correlations) and the 1/e distance."""
    land = np.argwhere(np.isfinite(anom).all(0))
    i = land[rng.integers(0, len(land), pairs)]
    j = land[rng.integers(0, len(land), pairs)]
    la1, lo1 = np.radians(lat[i[:, 0]]), np.radians(lon[i[:, 1]])
    la2, lo2 = np.radians(lat[j[:, 0]]), np.radians(lon[j[:, 1]])
    d = 6371 * np.arccos(np.clip(np.sin(la1) * np.sin(la2) + np.cos(la1) * np.cos(la2) * np.cos(lo1 - lo2), -1, 1))
    a = anom[:, i[:, 0], i[:, 1]]
    b = anom[:, j[:, 0], j[:, 1]]
    a = (a - a.mean(0)) / (a.std(0) + 1e-12)
    b = (b - b.mean(0)) / (b.std(0) + 1e-12)
    r = (a * b).mean(0)
    edges = [0, 150, 300, 450, 600, 800, 1000, 1300, 1600, 2000, 2500, 3000]
    out = []
    for lo, hi in zip(edges[:-1], edges[1:]):
        m = (d >= lo) & (d < hi)
        out.append((lo, hi, float(np.nanmean(r[m])) if m.sum() > 50 else float("nan"), int(m.sum())))
    e_dist = next((lo for lo, hi, c, n in out if c < 1 / math.e), float("nan"))
    return out, e_dist


def lagcorr(anom, lag):
    a, b = anom[:-lag], anom[lag:]
    ok = np.isfinite(a) & np.isfinite(b)
    a = np.where(ok, a, 0)
    b = np.where(ok, b, 0)
    n = ok.sum(0)
    ma, mb = a.sum(0) / np.maximum(n, 1), b.sum(0) / np.maximum(n, 1)
    cov = (np.where(ok, (a - ma) * (b - mb), 0)).sum(0) / np.maximum(n, 1)
    sa = np.sqrt((np.where(ok, (a - ma) ** 2, 0)).sum(0) / np.maximum(n, 1))
    sb = np.sqrt((np.where(ok, (b - mb) ** 2, 0)).sum(0) / np.maximum(n, 1))
    c = cov / np.maximum(sa * sb, 1e-12)
    return float(np.nanmean(np.where(n > 24, c, np.nan)))


def main(root):
    root = Path(root)
    rng = np.random.default_rng(7)
    P = series(root / "hist", "prec", range(2000, 2020))  # 240 months
    nm, rows, cols = P.shape
    lat = 90 - (np.arange(rows) + 0.5) * 180 / rows
    lon = -180 + (np.arange(cols) + 0.5) * 360 / cols
    w = np.cos(np.radians(lat))[:, None] * np.ones((1, cols))
    land = np.isfinite(P).all(0) & (lat[:, None] > -60)
    print(f"precipitation: {nm} months, {land.sum()} land cells of 1 degree")
    # 1. Annual totals: CV by mean annual rain.
    annual = P.reshape(20, 12, rows, cols).sum(1)
    mean = annual.mean(0)
    cv = annual.std(0) / np.maximum(mean, 1e-9)
    print("CV of annual rain by mean annual rain (mm): bin, area share, CV (area-weighted mean)")
    for lo, hi in [(25, 100), (100, 200), (200, 400), (400, 800), (800, 1600), (1600, 3200), (3200, 10000)]:
        m = land & (mean >= lo) & (mean < hi)
        if m.sum():
            print(f"  {lo}-{hi}: {100 * w[m].sum() / w[land].sum():5.1f} %  CV {np.average(cv[m], weights=w[m]):.3f}")
    # 2. Monthly anomalies (ratio to the calendar month's mean, months with a mean of 10 mm or more).
    clim = P.reshape(20, 12, rows, cols).mean(0)
    ratio = P / np.tile(clim, (20, 1, 1)) - 1
    ratio[np.tile(clim, (20, 1, 1)) < 10] = np.nan
    ratio[:, ~land] = np.nan
    out, e = decay(ratio, lat, lon, rng)
    print("monthly rain anomaly correlation by distance (km): " + ", ".join(f"{lo}-{hi} {c:.2f}" for lo, hi, c, n in out))
    print(f"  falls below 1/e at about {e} km")
    # 3-month totals: smoother, the scale of a drought.
    P3 = P[:-2] + P[1:-1] + P[2:]
    c3 = np.stack([P3[k::12].mean(0) for k in range(12)])
    r3 = P3 / np.tile(c3, (20, 1, 1))[: P3.shape[0]] - 1
    r3[:, ~land] = np.nan
    out3, e3 = decay(r3, lat, lon, rng)
    print("3-month rain anomaly correlation by distance: " + ", ".join(f"{lo}-{hi} {c:.2f}" for lo, hi, c, n in out3))
    print(f"  falls below 1/e at about {e3} km")
    print(f"monthly rain anomaly persistence: lag 1 {lagcorr(ratio, 1):.2f}, lag 2 {lagcorr(ratio, 2):.2f}, lag 3 {lagcorr(ratio, 3):.2f}, lag 6 {lagcorr(ratio, 6):.2f}, lag 12 {lagcorr(ratio, 12):.2f}")
    # 3. Drought: the share of land whose 12-month total (calendar year) is under 80 % of its mean.
    shares = []
    for y in range(20):
        m = land & (mean > 50)
        dry = m & (annual[y] < 0.8 * mean)
        shares.append(w[dry].sum() / w[m].sum())
    print(f"land in drought (calendar-year rain under 80 % of the mean): mean {100 * np.mean(shares):.1f} %, years from {100 * min(shares):.1f} to {100 * max(shares):.1f} %")
    two = np.zeros_like(mean, dtype=bool)
    for y in range(19):
        two |= land & (annual[y] < 0.8 * mean) & (annual[y + 1] < 0.8 * mean)
    print(f"land with two dry years running at least once in 20 years: {100 * w[two].sum() / w[land & (mean > 50)].sum():.1f} %")
    # 4. Temperature (maximum, 2010-2019): monthly anomaly SD by latitude band and season, decay, persistence.
    T = series(root / "hist", "tmax", range(2010, 2020))
    tl = np.isfinite(T).all(0) & (lat[:, None] > -60)
    tclim = T.reshape(10, 12, rows, cols).mean(0)
    ta = T - np.tile(tclim, (10, 1, 1))
    ta[:, ~tl] = np.nan
    print("monthly temperature anomaly SD (C) by band: band, December-February, June-August")
    for b0 in range(-60, 90, 15):
        m = tl & (lat[:, None] >= b0) & (lat[:, None] < b0 + 15)
        if m.sum() < 10:
            continue
        djf = ta.reshape(10, 12, rows, cols)[:, [0, 1, 11]][:, :, m].std()
        jja = ta.reshape(10, 12, rows, cols)[:, [5, 6, 7]][:, :, m].std()
        print(f"  {b0:+4d}..{b0 + 15:+4d}: {djf:.2f} {jja:.2f}")
    outt, et = decay(ta, lat, lon, rng)
    print("monthly temperature anomaly correlation by distance: " + ", ".join(f"{lo}-{hi} {c:.2f}" for lo, hi, c, n in outt))
    print(f"  falls below 1/e at about {et} km")
    print(f"temperature anomaly persistence: lag 1 {lagcorr(ta, 1):.2f}, lag 2 {lagcorr(ta, 2):.2f}, lag 3 {lagcorr(ta, 3):.2f}")
    # 5. Wind: the mean speed over land by latitude band (1970-2000 monthly means).
    W = np.stack([coarse(next((root / "wind").glob(f"*_wind_{m:02d}.tif"))) for m in range(1, 13)])
    wm = W.mean(0)
    wl = np.isfinite(wm) & (lat[:, None] > -60)
    print(f"wind over land: mean {np.average(wm[wl], weights=w[wl]):.2f} m/s; by 10-degree band from -60:")
    print("  " + " ".join(f"{np.average(wm[wl & (lat[:, None] >= b) & (lat[:, None] < b + 10)], weights=w[wl & (lat[:, None] >= b) & (lat[:, None] < b + 10)]) if (wl & (lat[:, None] >= b) & (lat[:, None] < b + 10)).sum() else 0:.2f}" for b in range(-60, 90, 10)))
    season = (W.max(0) - W.min(0)) / np.maximum(wm, 1e-9)
    print(f"  the windiest month over the calmest, relative to the mean: {np.average(season[wl], weights=w[wl]):.2f}")


if __name__ == "__main__":
    main(sys.argv[1])
