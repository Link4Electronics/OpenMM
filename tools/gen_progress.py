#!/usr/bin/env python3
"""Generate the decompilation progress gauge (reccmp-report/progress.svg).

Counts functions in the original linker map (game/midtown.map) against
unique `// FUNCTION: MIDTOWN 0x...` annotations under code/midtown,
mirroring what reccmp reports once an MSVC verification build exists.

Usage:
  python3 tools/gen_progress.py [--out reccmp-report]
"""

import argparse
import json
import re
import sys
from collections import defaultdict
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
from map_annotate import MODULE, CODE_DIR, parse_map  # noqa: E402

FUNC_PAT = re.compile(rf"^//\s*FUNCTION:\s*{MODULE}\s*0x([0-9A-Fa-f]+)\s*$", re.M)


def annotated_addresses():
    addrs = set()
    for f in list(CODE_DIR.rglob("*.cpp")) + list(CODE_DIR.rglob("*.h")):
        for m in FUNC_PAT.finditer(f.read_text(errors="replace")):
            addrs.add(int(m.group(1), 16))
    return addrs


def render_svg(done, total):
    pct = (100.0 * done / total) if total else 0.0
    width, height = 480.0, 40.0
    pad = 4.0
    label = f"{done} / {total} functions ({pct:.1f}%)"
    fill_w = max(0.0, min(1.0, done / total)) * (width - 2 * pad) if total else 0.0

    return f"""<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">
  <title>OpenMM decompilation progress: {label}</title>
  <rect x="0" y="0" width="{width}" height="{height}" rx="8" ry="8" fill="#1b1f24"/>
  <rect x="{pad}" y="{pad}" width="{width - 2 * pad}" height="{height - 2 * pad}" rx="5" ry="5" fill="#30363d"/>
  <clipPath id="round"><rect x="{pad}" y="{pad}" width="{width - 2 * pad}" height="{height - 2 * pad}" rx="5" ry="5"/></clipPath>
  <rect x="{pad}" y="{pad}" width="{fill_w}" height="{height - 2 * pad}" clip-path="url(#round)" fill="#2ea44f"/>
  <text x="{width / 2}" y="{height / 2 + 5}" font-family="Verdana,Geneva,DejaVu Sans,sans-serif" font-size="14" font-weight="bold" fill="#ffffff" text-anchor="middle">{label}</text>
</svg>
"""


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--out", default="reccmp-report", help="output directory (relative to repo root)")
    args = ap.parse_args()

    total_funcs = parse_map()
    total = len(total_funcs)
    addrs = set(a for a, _ in total_funcs)
    done_addrs = annotated_addresses()
    done = len(done_addrs & addrs) if addrs else len(done_addrs)

    outdir = ROOT / args.out
    outdir.mkdir(parents=True, exist_ok=True)
    (outdir / "progress.svg").write_text(render_svg(done, total))
    (outdir / "progress.json").write_text(json.dumps({
        "generated": datetime.now(timezone.utc).isoformat(),
        "total_functions": total,
        "implemented": done,
        "percent": round(100.0 * done / total, 2) if total else 0.0,
    }, indent=2) + "\n")

    print(f"progress: {done}/{total} functions ({100.0 * done / total if total else 0:.1f}%)")
    print(f"wrote {outdir}/progress.svg")


if __name__ == "__main__":
    main()
