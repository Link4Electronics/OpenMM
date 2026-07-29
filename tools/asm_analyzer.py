#!/usr/bin/env python3
"""
ASM analyzer for Open1560 — reverse engineering toolkit for game.asm (MASM).

Usage:
  ./asm_analyzer.py game.asm <command> [options]

Commands:
  list-funcs       List all functions with sizes
  func <name>      Disassemble a specific function with call graph context
  calls <name>     Show all callers of a function (X-refs)
  strings <name>   Show all string references in a function
  stats            Overall file statistics
  exports          List all ARTS_IMPORT / ARTS_EXPORT declarations
  callgraph <name> Recursive call graph for a function (depth-limited)
  find <pattern>   Search for a pattern in function names
"""

import re
import sys
import os
from collections import defaultdict
from pathlib import Path


# ── Parsing ──────────────────────────────────────────────────────────────

PROC_PAT = re.compile(r'^\s*(\S+)\s+PROC\s+(\w+)', re.IGNORECASE)
ENDP_PAT = re.compile(r'^\s*(\S+)\s+ENDP', re.IGNORECASE)
CALL_PAT = re.compile(r'\bcall\s+(near\s+ptr\s+)?(\S+)', re.IGNORECASE)
JMP_PAT  = re.compile(r'\bjmp\s+(near\s+ptr\s+)?(\S+)', re.IGNORECASE)
STR_REF  = re.compile(r"offset\s+(asc_|a)[0-9A-Fa-f]+|offset\s+(\?\?_7|\?\?_R|string)")
IMPORT_PAT = re.compile(r'ARTS_IMPORT.*(\?\?\d?\w+@\w+@\w+@\d+@\w*)')
EXPORT_PAT = re.compile(r'ARTS_EXPORT.*(\?\?\d?\w+@\w+@\w+@\d+@\w*)')
MANGLED_PAT = re.compile(r'(\?\?0\w+@\w+@@QAE@XZ|\?\?1\w+@@UAE@XZ|\?\?\w+@\w+@@\w+\w+@\w+@\d+@\w*)')
EXTERN_PAT = re.compile(r'extrn\s+\"([^"]+)\"|EXTERN\s+(\?\?\w+)', re.IGNORECASE)


def parse_functions(lines):
    """Return list of dicts: name, visibility, start, end, body_lines."""
    funcs = []
    current = None
    for i, line in enumerate(lines, 1):
        m = PROC_PAT.match(line)
        if m:
            if current is not None:
                # unclosed PROC — close it at previous line
                current['end'] = i - 1
                current['body'] = lines[current['start'] - 1 : current['end']]
                funcs.append(current)
            current = {
                'name': m.group(1),
                'visibility': m.group(2),
                'start': i,
                'end': None,
                'body': None,
            }
            continue
        m = ENDP_PAT.match(line)
        if m and current and m.group(1).lower() == current['name'].lower():
            current['end'] = i
            current['body'] = lines[current['start'] - 1 : current['end']]
            funcs.append(current)
            current = None
    if current is not None:
        current['end'] = len(lines)
        current['body'] = lines[current['start'] - 1 : current['end']]
        funcs.append(current)
    return funcs


def build_call_map(funcs, lines):
    """Return callees[name] = set of called names, callers[name] = set of callers."""
    callees = defaultdict(set)
    callers = defaultdict(set)
    name_set = {f['name'] for f in funcs}
    func_at_line = {}  # line -> function name
    for f in funcs:
        for ln in range(f['start'], f['end'] + 1):
            func_at_line[ln] = f['name']

    call_jmp = re.compile(r'\b(?:call|jmp)\s+(near\s+ptr\s+)?(\S+)', re.IGNORECASE)

    for i, line in enumerate(lines, 1):
        func_name = func_at_line.get(i)
        for m in call_jmp.finditer(line):
            target = m.group(2).rstrip(',;')
            if target not in name_set:
                continue
            if func_name:
                callees[func_name].add(target)
                callers[target].add(func_name)
            else:
                callers[target].add('<data>')

    return dict(callees), dict(callers)


def get_func(name, funcs):
    matches = [f for f in funcs if f['name'].lower() == name.lower()]
    if not matches:
        matches = [f for f in funcs if name.lower() in f['name'].lower()]
    if not matches:
        return None
    if len(matches) > 1:
        print(f"Multiple matches for '{name}':")
        for m in matches:
            print(f"  {m['name']} ({m['start']}-{m['end']}, {m['visibility']})")
        return None
    return matches[0]


# ── Formatters ──────────────────────────────────────────────────────────

def demangle(name):
    """Super rough demangle — just strips MASM decorations."""
    s = name
    if s.startswith('?'):
        # Try simple MASM demangling
        parts = s.split('@@')
        if len(parts) >= 2:
            return parts[0].lstrip('?')
    return s


def fmt_loc(line_no):
    return f"{line_no:>6d}"


def fmt_size(f):
    return f['end'] - f['start'] + 1 if f['end'] else 0


# ── Commands ────────────────────────────────────────────────────────────

def cmd_list_funcs(funcs, args):
    """list-funcs [--sort-by name|size|line] [--min-size N] [--filter PAT]"""
    sort_key = 'line'
    min_size = 0
    filt = None

    i = 0
    while i < len(args):
        if args[i] == '--sort-by' and i + 1 < len(args):
            sort_key = args[i + 1]
            i += 2
        elif args[i] == '--min-size' and i + 1 < len(args):
            min_size = int(args[i + 1])
            i += 2
        elif args[i] == '--filter' and i + 1 < len(args):
            filt = args[i + 1]
            i += 2
        else:
            i += 1

    shown = []
    for f in funcs:
        sz = fmt_size(f)
        if sz < min_size:
            continue
        if filt and filt.lower() not in f['name'].lower():
            continue
        vis = 'PUB' if f['visibility'].upper() == 'PUBLIC' else 'PRV'
        shown.append((f['start'], f['name'], sz, vis))

    if sort_key == 'name':
        shown.sort(key=lambda x: x[1].lower())
    elif sort_key == 'size':
        shown.sort(key=lambda x: -x[2])
    else:
        shown.sort(key=lambda x: x[0])

    print(f"{'Line':>6}  {'Size':>5}  {'Vis':>3}  Name")
    print(f"{'─'*6}  {'─'*5}  {'─'*3}  {'─'*40}")
    for line, name, sz, vis in shown:
        print(f"{line:>6}  {sz:>5}  {vis:>3}  {name}")
    print(f"\nTotal: {len(shown)} functions")


def cmd_func(funcs, callers, callees, args, lines=None):
    """func <name>"""
    if not args:
        print("Usage: func <name>")
        return
    name = args[0]
    f = get_func(name, funcs)
    if f is None:
        print(f"Function '{name}' not found")
        return
    name = f['name']
    print(f"{'─'*60}")
    print(f"  {name}")
    print(f"  Line {f['start']} – {f['end']}  ({fmt_size(f)} lines)  [{f['visibility']}]")
    print(f"{'─'*60}")

    # Callees
    if name in callees and callees[name]:
        print("\n  Calls:")
        for c in sorted(callees[name]):
            cf = get_func(c, funcs)
            sz = fmt_size(cf) if cf else 0
            print(f"    {c}  ({sz} lines)")

    # Callers
    if name in callers and callers[name]:
        print("\n  Called by:")
        for c in sorted(callers[name]):
            print(f"    {c}")

    strings = []
    for line in f['body']:
        if 'offset' in line:
            for m in re.finditer(r'offset\s+(asc_[0-9A-F]+|a[0-9A-F][0-9A-F]*)', line, re.I):
                strings.append(line.strip())
                break

    if strings:
        print(f"\n  String references ({len(strings)}):")
        for s in strings[:20]:
            print(f"    {s}")
        if len(strings) > 20:
            print(f"    ... and {len(strings) - 20} more (use 'strings {name}' for all)")

    print()


def cmd_calls(funcs, callers, args):
    """calls <name> — show all callers of a function (X-refs)"""
    if not args:
        print("Usage: calls <name>")
        return
    name = args[0]
    f = get_func(name, funcs)
    if f is None:
        print(f"Function '{name}' not found")
        return
    name = f['name']

    print(f"Cross-references to {name}:")
    if name in callers and callers[name]:
        for c in sorted(callers[name]):
            cf = get_func(c, funcs)
            if cf:
                print(f"  {cf['start']:>6}  {c}  ({fmt_size(cf)} lines)")
            else:
                print(f"  {'?':>6}  {c}")
    else:
        print("  (none)")


def cmd_strings(funcs, args):
    """strings <name> — show all string references in a function"""
    if not args:
        print("Usage: strings <name>")
        return
    name = args[0]
    f = get_func(name, funcs)
    if f is None:
        print(f"Function '{name}' not found")
        return

    refs = []
    for line in f['body']:
        for m in re.finditer(r'offset\s+(\S+)', line):
            refs.append((line.strip(), m.group(1)))

    if not refs:
        print(f"No string references in {f['name']}")
        return

    print(f"String references in {f['name']}:")
    for line, target in refs:
        print(f"  {target:30s}  {line}")


def cmd_stats(funcs, lines):
    """stats"""
    total_lines = len(lines)
    total_funcs = len(funcs)
    total_size = sum(fmt_size(f) for f in funcs)
    publics = sum(1 for f in funcs if f['visibility'].upper() == 'PUBLIC')
    privates = sum(1 for f in funcs if f['visibility'].upper() != 'PUBLIC')
    avg_size = total_size / total_funcs if total_funcs else 0

    # Size distribution
    sizes = [fmt_size(f) for f in funcs]
    big = sum(1 for s in sizes if s >= 500)
    med = sum(1 for s in sizes if 100 <= s < 500)
    sml = sum(1 for s in sizes if 10 <= s < 100)
    tiny = sum(1 for s in sizes if s < 10)

    print(f"{'Stat':>25}  {'Value':>10}")
    print(f"{'─'*25}  {'─'*10}")
    print(f"{'Total lines':>25}  {total_lines:>10,}")
    print(f"{'Total functions':>25}  {total_funcs:>10,}")
    print(f"{'Public functions':>25}  {publics:>10,}")
    print(f"{'Private functions':>25}  {privates:>10,}")
    print(f"{'Total function lines':>25}  {total_size:>10,}")
    print(f"{'Avg function size':>25}  {avg_size:>10.1f}")
    print(f"{'Functions <10 lines':>25}  {tiny:>10,}")
    print(f"{'Functions 10-99':>25}  {sml:>10,}")
    print(f"{'Functions 100-499':>25}  {med:>10,}")
    print(f"{'Functions 500+':>25}  {big:>10,}")

    # Biggest functions
    print(f"\n{'Biggest functions:':>25}")
    by_size = sorted(funcs, key=lambda f: -fmt_size(f))[:10]
    for f in by_size:
        print(f"  {f['start']:>6}  {f['name']:45s}  {fmt_size(f)} lines")

    # Most-called functions
    print(f"\n{'Most-referenced by callers:'}")


def cmd_exports(lines):
    """exports — find ARTS_IMPORT / ARTS_EXPORT declarations in C++ files"""
    # This actually looks at C++ files, not the .asm
    cpp_dir = Path(__file__).resolve().parent.parent / 'code' / 'midtown'
    if not cpp_dir.exists():
        print("C++ source directory not found at expected path")
        return

    imports = defaultdict(list)
    exports = defaultdict(list)

    for cpp in cpp_dir.rglob('*.h'):
        text = cpp.read_text(encoding='utf-8', errors='replace')
        for i, line in enumerate(text.splitlines(), 1):
            if 'ARTS_IMPORT' in line:
                imports[str(cpp.relative_to(cpp_dir))].append((i, line.strip()))
            if 'ARTS_EXPORT' in line:
                exports[str(cpp.relative_to(cpp_dir))].append((i, line.strip()))

    total_imports = sum(len(v) for v in imports.values())
    total_exports = sum(len(v) for v in exports.values())
    print(f"ARTS_IMPORT declarations: {total_imports}")
    print(f"ARTS_EXPORT declarations: {total_exports}")

    # Group by directory
    by_dir = defaultdict(int)
    for path, entries in imports.items():
        by_dir[path.split('/')[0]] += len(entries)
    print(f"\nBy module directory:")
    for d, count in sorted(by_dir.items(), key=lambda x: -x[1]):
        print(f"  {d:20s}  {count} imports")


def cmd_callgraph(funcs, callers, callees, args):
    """callgraph <name> [--depth N] — recursive call graph"""
    if not args:
        print("Usage: callgraph <name> [--depth N]")
        return
    name = args[0]
    depth = 3
    if '--depth' in args:
        try:
            depth = int(args[args.index('--depth') + 1])
        except (ValueError, IndexError):
            pass

    f = get_func(name, funcs)
    if f is None:
        print(f"Function '{name}' not found")
        return
    root = f['name']

    def print_tree(node, cur_depth=0, visited=None):
        if visited is None:
            visited = set()
        if cur_depth > depth or node in visited:
            return
        visited.add(node)
        indent = '  ' * cur_depth
        suffix = ''
        c = callers.get(node)
        if c and len(c) > 1:
            suffix = f'  [x{len(c)} callers]'
        print(f"{indent}{node}{suffix}")
        for child in sorted(callees.get(node, [])):
            print_tree(child, cur_depth + 1, visited)

    print(f"Call graph for {root} (depth={depth}, calls down, {len(callers.get(root, []))} callers):")
    print_tree(root)


def cmd_find(funcs, args):
    """find <pattern> — search function names"""
    if not args:
        print("Usage: find <pattern>")
        return
    pattern = args[0]
    matches = [f for f in funcs if pattern.lower() in f['name'].lower()]
    if not matches:
        print(f"No functions matching '{pattern}'")
        return
    print(f"Functions matching '{pattern}' ({len(matches)}):")
    for f in sorted(matches, key=lambda x: x['start']):
        print(f"  {f['start']:>6}  {f['name']:50s}  {fmt_size(f):>5} lines  [{f['visibility']}]")


# ── Main ────────────────────────────────────────────────────────────────

def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    asm_path = Path(sys.argv[1])
    if not asm_path.exists():
        print(f"File not found: {asm_path}")
        sys.exit(1)

    command = sys.argv[2] if len(sys.argv) > 2 else 'list-funcs'
    cmd_args = sys.argv[3:]

    print(f"Reading {asm_path}... ", end='', flush=True)
    text = asm_path.read_text(encoding='utf-8', errors='replace')
    lines = text.splitlines()
    print(f"{len(lines):,} lines")

    print("Parsing functions... ", end='', flush=True)
    funcs = parse_functions(lines)
    print(f"{len(funcs)} found")

    # Pre-build call map only for commands that need it
    callees = callers = None
    need_maps = command in ('func', 'calls', 'callgraph')
    if need_maps:
        print("Building call map (single pass)... ", end='', flush=True)
        callees, callers = build_call_map(funcs, lines)
        print("done")

    # Dispatch
    commands = {
        'list-funcs': lambda: cmd_list_funcs(funcs, cmd_args),
        'func': lambda: cmd_func(funcs, callers or {}, callees or {}, cmd_args, lines),
        'calls': lambda: cmd_calls(funcs, callers or {}, cmd_args),
        'strings': lambda: cmd_strings(funcs, cmd_args),
        'stats': lambda: cmd_stats(funcs, lines),
        'exports': lambda: cmd_exports(lines),
        'callgraph': lambda: cmd_callgraph(funcs, callers or {}, callees or {}, cmd_args),
        'find': lambda: cmd_find(funcs, cmd_args),
    }

    handler = commands.get(command)
    if handler is None:
        print(f"Unknown command: {command}")
        print(f"Available: {', '.join(commands.keys())}")
        sys.exit(1)

    handler()


if __name__ == '__main__':
    main()
