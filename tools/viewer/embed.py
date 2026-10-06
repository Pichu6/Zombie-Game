"""Inline a zs_headless --trace JSON into the replay page.

usage: python3 tools/viewer/embed.py trace.json out.html
"""
import pathlib
import sys

here = pathlib.Path(__file__).parent
page = (here / "replay.html").read_text(encoding="utf-8")
trace = pathlib.Path(sys.argv[1]).read_text(encoding="utf-8")
pathlib.Path(sys.argv[2]).write_text(page.replace("/*__TRACE__*/null", trace), encoding="utf-8")
