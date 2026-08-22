#!/usr/bin/env python3
"""Annotate reimplemented functions with original binary addresses.

Pairs symbols from game/midtown.map (original linker map, build 1560)
with out-of-class C++ definitions under code/midtown and inserts
reccmp-style annotations:

    // FUNCTION: MIDTOWN 0x00401030

Matching is conservative: a definition is annotated only when its
`Class::Method` qualified name maps to exactly one symbol in the map.
Overloads and free functions must be annotated manually.

Usage:
  python3 tools/map_annotate.py            # dry run (default)
  python3 tools/map_annotate.py --write    # insert annotations
  python3 tools/map_annotate.py --write code/midtown/mmui
"""

import argparse
import collections
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MAP_FILE = ROOT / "game" / "midtown.map"
CODE_DIR = ROOT / "code" / "midtown"
MODULE = "MIDTOWN"

# Publics-by-value entry: seg:off  mangled  rva+base  f(i)  lib:obj
MAP_PAT = re.compile(
    r"^ ([0-9a-fA-F]{4}):([0-9a-fA-F]{8})\s+(\S+)\s+([0-9a-fA-F]{8})\s+f\s+(\S+)"
)

# CRT / system objects that are not part of the game code we re-implement
IGNORED_OBJ_PREFIXES = (
    "libcmt", "libcd", "setargv", "dinput", "dxguid", "uuid",
    "kernel32", "user32", "gdi32", "advapi32", "winmm", "comdlg32",
    "shell32", "ole32", "oledlg", "ddraw", "dsound", "wininet",
    "wsock32", "msvcrt", "<common>",
)

# Out-of-class definition candidates: qualified name preceded only by
# whitespace/type tokens at line start.
DEF_PAT = re.compile(
    r"^[ \t]*(?:virtual\s+|static\s+)?[\w:<>,&*\s]+?"
    r"\b([A-Za-z_]\w*(?:::~?[A-Za-z_]\w*)+)\s*\(",
    re.M,
)


def parse_map():
    funcs = []
    for line in MAP_FILE.read_text(errors="replace").splitlines():
        m = MAP_PAT.match(line)
        if not m:
            continue
        mangled, rva, obj = m.group(3), int(m.group(4), 16), m.group(5)
        if obj.lower().startswith(IGNORED_OBJ_PREFIXES):
            continue
        funcs.append((rva, mangled))
    return funcs


def demangle(mangled_names):
    """Batch-demangle MSVC names with llvm-undname."""
    if not mangled_names:
        return {}
    raw = subprocess.run(
        ["llvm-undname"],
        input="\n".join(mangled_names).encode(),
        capture_output=True,
    ).stdout.decode()
    lines = [l for l in raw.splitlines() if l.strip()]
    pairs = {lines[i]: lines[i + 1] for i in range(0, len(lines) - 1, 2)}
    return pairs


def qualified_name(demangled):
    """Extract Class::Method (dtors normalized to Class::Method) or None."""
    if not demangled:
        return None
    s = demangled.replace("virtual ", "").replace("static ", "")
    s = re.sub(r"__(thiscall|cdecl|stdcall|fastcall)\s+", "", s)
    if "[thunk]" in s or "operator" in s or "`" in s:
        return None
    s = re.sub(r"^(public|protected|private):\s*", "", s)
    m = re.match(r"^.*?\b(\w+(?:::~?~?\w+)+)\s*\(", s)
    return m.group(1).replace("~", "") if m else None


def build_symbol_table(funcs, demangled):
    table = collections.defaultdict(list)
    for rva, mangled in funcs:
        q = qualified_name(demangled.get(mangled)) if mangled.startswith("?") else None
        if q:
            table[q].append(rva)
    unique = {q: addrs[0] for q, addrs in table.items() if len(addrs) == 1}
    return unique, len(table)


def is_definition(src, open_paren_pos):
    """True if the paren group ending after pos is followed by '{' (a body)."""
    i, depth = open_paren_pos, 0
    while i < len(src):
        c = src[i]
        if c == "(":
            depth += 1
        elif c == ")":
            depth -= 1
            if depth == 0:
                break
        elif c == ";" or c == "}":
            return False
        i += 1
    else:
        return False
    rest = src[i + 1 :]
    m = re.match(r"\s*(?:const\s+|override\s+|noexcept\s+)*\{", rest)
    return m is not None


def annotate_file(path, table, write):
    src = path.read_text(errors="replace")
    insertions = []  # (offset_in_lines, addr)
    seen_spans = []
    for m in DEF_PAT.finditer(src):
        q = m.group(1).replace("~", "")
        addr = table.get(q)
        if addr is None:
            continue
        if not is_definition(src, m.end() - 1):
            continue
        # The type-token run may start on an earlier line (regex \s crosses
        # newlines); walk back to the real start of the signature without
        # swallowing comment lines above it.
        p = m.start(1)
        while p > 0:
            c = src[p - 1]
            if not re.match(r"[\w\s<>,&*:~]", c):
                break
            if c == "\n":
                pl_start = src.rfind("\n", 0, p - 1) + 1
                prev_line = src[pl_start : p - 1].lstrip()
                if prev_line.startswith("//") or prev_line.endswith("*/"):
                    break
            p -= 1
        while p < len(src) and src[p].isspace():
            p += 1
        line_start = src.rfind("\n", 0, p) + 1
        # skip overlaps from greedy multi-candidates on the same header
        if any(line_start < b <= m.end() for _, b in seen_spans):
            continue
        seen_spans.append((line_start, m.end()))
        # don't double-annotate
        prev = src[:line_start].rstrip()
        if prev.endswith("*/") or prev.splitlines()[-1].lstrip().startswith("//"):
            continue
        insertions.append((line_start, addr))

    if not insertions:
        return 0

    if write:
        lines = src.splitlines(keepends=True)
        for line_start, addr in sorted(insertions, reverse=True):
            idx = src.count("\n", 0, line_start)
            indent = re.match(r"[ \t]*", lines[idx]).group(0)
            comment = f"{indent}// FUNCTION: {MODULE} 0x{addr:08X}\n"
            lines.insert(idx, comment)
        path.write_text("".join(lines), errors="replace")
    return len(insertions)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--write", action="store_true", help="insert annotations (default: dry run)")
    ap.add_argument("paths", nargs="*", help="restrict to these files/dirs (relative to repo root)")
    args = ap.parse_args()

    funcs = parse_map()
    print(f"map: {len(funcs)} game functions")
    demangled = demangle([n for _, n in funcs])
    table, total_q = build_symbol_table(funcs, demangled)
    print(f"unique member-function names: {len(table)} / {total_q}")

    targets = []
    for p in args.paths:
        pp = Path(p if p.startswith("/") else ROOT / p)
        targets.append(pp if pp.is_dir() else pp)
    files = (
        sorted(CODE_DIR.rglob("*.cpp")) if not targets else
        sorted({f for t in targets for f in (t.rglob("*.cpp") if t.is_dir() else [t])})
    )

    mode = "ANNOTATING" if args.write else "DRY RUN"
    grand = 0
    for f in files:
        n = annotate_file(f, table, args.write)
        if n:
            print(f"  [{mode}] {f.relative_to(ROOT)}: {n}")
            grand += n
    print(f"total: {grand} function{'s' if grand != 1 else ''} {'annotated' if args.write else 'matchable'}")


if __name__ == "__main__":
    sys.exit(main())
