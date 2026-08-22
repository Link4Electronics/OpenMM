# reccmp — decompilation verification workflow

OpenMM uses [reccmp](https://github.com/isledecomp/reccmp)
to track which original functions have been reimplemented
and, eventually, verify them byte-for-byte.

## How it works

1. Every function that is reimplemented in C++ carries an annotation
   comment naming its address in the original binary:

   ```cpp
   // FUNCTION: MIDTOWN 0x00401030
   void mmGameManager::BeDone()
   {
   ```

2. `game/midtown.map` (the original linker map) provides the address of
   every symbol in the beta build. `tools/map_annotate.py` pairs map
   symbols with C++ definitions and inserts annotations automatically.
3. A build compiled with a period-correct MSVC toolchain is compared by
   reccmp against the original `midtown.exe`, producing per-function
   diffs and progress reports (`tools/reccmp.sh`).
4. `tools/gen_progress.py` renders `reccmp-report/progress.svg` — the
   gauge embedded in README.md — from the annotations alone, so it works
   even before an MSVC verification build exists.

## Annotating functions

```sh
# dry run: show what would be annotated
python3 tools/map_annotate.py --dry

# insert // FUNCTION: MIDTOWN 0x... comments into code/midtown/**
python3 tools/map_annotate.py --write

# annotate only specific files/dirs
python3 tools/map_annotate.py --write code/midtown/mmui
```

Matching is conservative: only out-of-class definitions whose
`Class::Method` name is *unique* in the linker map get annotated;
overloads are skipped and must be annotated by hand.

## Verification build (byte-compare)

Byte-exact comparison requires compiling with the same compiler family
as the original (MSVC, x86, ~1999). On Linux this runs under Wine:

1. Set up a Wine prefix with VC6 (see dethrace's `reccmp/` docker
   environment for a template).
2. Place the original beta exe in the repo root as `midtown.exe`.
3. Install the tooling:

   ```sh
   python3 -m venv /tmp/reccmp-venv
   /tmp/reccmp-venv/bin/pip install reccmp
   export PATH=/tmp/reccmp-venv/bin:$PATH
   reccmp-project detect --search-path .
   ```

4. Run the full loop:

   ```sh
   tools/reccmp.sh                 # configure + build + report
   tools/reccmp.sh 0x4b0f40        # diff one function
   ```

The CMake side is wired through `-DOPENMM_RECCMP=ON` (MSVC only): the
executable is renamed `midtown.exe`, a PDB is emitted, and
`reccmp-build.yml` is generated pointing reccmp at the result.

> **Note:** platform backends (SDL3/OpenGL) currently need dev packages
> visible to the MSVC/Wine environment for the verification build to
> link; decoupling those layers like dethrace does with
> `MSVC_42_FOR_RECCMP` is future work.

## Reports

- `reccmp-report/progress.svg` — committed gauge shown in README.md,
  regenerated via `python3 tools/gen_progress.py` (CI-friendly).
- `build_msvc/reccmp-report.html` — full per-function diff report once
  the verification build exists.
