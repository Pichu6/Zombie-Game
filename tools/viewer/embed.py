"""Inline a zs --trace JSON into the replay page.

usage: python3 tools/viewer/embed.py trace.json out.html [--max-mb 12]

A 14-day run's trace is too big for a hosted page; --max-mb thins it: noise/light fields go to one
every N hours, and frames closer than a minimum gap are dropped unless what someone is doing changed.
"""
import json
import pathlib
import sys

here = pathlib.Path(__file__).parent
page = (here / "replay.html").read_text(encoding="utf-8")
trace = pathlib.Path(sys.argv[1]).read_text(encoding="utf-8")
max_mb = float(sys.argv[sys.argv.index("--max-mb") + 1]) if "--max-mb" in sys.argv else None


def thin(t, gap, field_every):
    keep, last = [], None
    for f in t["frames"]:
        doing = (f[4], tuple(p[3] for p in f[-1]), tuple(p[5] for p in f[-1]))
        if last is None or f[0] - last[0] >= gap or doing != last[1]:
            keep.append(f)
            last = (f[0], doing)
    out = dict(t, frames=keep)
    out["fields"] = t["fields"][::field_every]
    return json.dumps(out, separators=(",", ":"))


if max_mb and len(trace) > max_mb * 1e6:
    full = json.loads(trace)
    for gap, every in ((60, 1), (120, 2), (240, 2), (480, 3), (960, 4)):
        trace = thin(full, gap, every)
        if len(trace) <= max_mb * 1e6:
            break
    print(f"thinned to {len(trace) / 1e6:.1f} MB (frame gap {gap} ticks, a field every {every} h)")
pathlib.Path(sys.argv[2]).write_text(page.replace("/*__TRACE__*/null", trace), encoding="utf-8")
