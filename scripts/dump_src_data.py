#!/usr/bin/env python3
"""Mechanically dump a .src file's data sections (C and/or D) as C code.

Parses `.DATA.B` / `.DATA.L` / `.RES.B` / (live, i.e. not commented-out)
`.SDATA "..."` records under `.SECTION C` and `.SECTION D` into ordered
(label -> bytes/pointers) blocks, then emits one C global per block:

  * A block with no `.DATA.L` (pure bytes, `.RES.B` zero-filled) becomes
    `STATIC Uint8 name[] = { 0x.., ... };`.
  * A block that contains any `.DATA.L` (a pointer to another label in the
    same file -- the only kind these units use) becomes
    `STATIC int name[] = { (int)target, 0x.., ... };`, packing each run of
    raw bytes between pointers into little-endian 32-bit words. This only
    works when every such run's length is a multiple of 4; the script
    aborts otherwise rather than guess a layout.
  * Every symbol is `STATIC` by default; pass `--public NAME,NAME,...` to
    emit specific ones without it, for symbols that need external linkage.

Labels are emitted in file order, which is also dependency order: every
`.DATA.L` target in these units is defined earlier in the same file (checked
by --check-forward-refs), so no forward declarations are needed.

This is a mechanical first pass -- it produces byte-identical (verify with
scripts/dcdiff.py) but untyped data. Give a symbol a real struct type by hand
afterward where it's worth the readability (see docs / move-data skill).

Usage:
  scripts/dump_src_data.py <unit.src> [--exclude NAME,NAME,...] > out.c
  scripts/dump_src_data.py <unit.src> --only NAME,NAME,...
  scripts/dump_src_data.py <unit.src> --public NAME,NAME,...
  scripts/dump_src_data.py <unit.src> --check-forward-refs
"""
import argparse
import re
import sys

LABEL_RE = re.compile(r"^_(\w+):")
SECTION_RE = re.compile(r"^\s*\.SECTION\s+([A-Z]+)\s*,\s*DATA\b")
OTHER_SECTION_RE = re.compile(r"^\s*\.SECTION\b")
DATA_B_RE = re.compile(r"^\s*\.DATA\.B\s+(.*)$")
DATA_L_RE = re.compile(r"^\s*\.DATA\.L\s+_(\w+)")
RES_B_RE = re.compile(r"^\s*\.RES\.B\s+(\S+)")
SDATA_COMMENT_RE = re.compile(r'^\s*;\.SDATA\s+"(.*)"\s*$')
SDATA_LIVE_RE = re.compile(r'^\s*\.SDATA\s+"(.*)"\s*(?:;.*)?$')
END_RE = re.compile(r"^\s*\.END\b")


class Block:
    def __init__(self, name, section):
        self.name = name
        self.section = section
        self.items = []  # list of ('B', [int, ...]) | ('L', name)
        self.sdata = None  # original string literal, if this block held one


def parse_blocks(path, sections=("C", "D")):
    blocks = []
    cur = None
    cur_section = None
    with open(path, encoding="shift_jis", errors="replace") as f:
        lines = f.read().splitlines()

    for line in lines:
        m = SECTION_RE.match(line)
        if m:
            cur_section = m.group(1)
            cur = None
            continue
        if OTHER_SECTION_RE.match(line) or END_RE.match(line):
            cur_section = None
            cur = None
            continue
        if cur_section not in sections:
            continue

        m = LABEL_RE.match(line)
        if m:
            cur = Block(m.group(1), cur_section)
            blocks.append(cur)
            continue
        if cur is None:
            continue

        m = SDATA_COMMENT_RE.match(line)
        if m and not cur.items:
            # Documentation only: the real bytes are the .DATA.B hex that
            # follows (used where the string isn't plain ASCII, e.g. Shift-JIS
            # text the assembler can't take as a string literal).
            cur.sdata = m.group(1)
            continue

        m = SDATA_LIVE_RE.match(line)
        if m:
            # A live .SDATA emits exactly the string's ASCII bytes with no
            # implicit terminator -- callers follow it with their own
            # .DATA.B H'00 / .RES.B for termination and alignment padding.
            if not cur.items:
                cur.sdata = m.group(1)
            cur.items.append(("B", list(m.group(1).encode("ascii"))))
            continue

        m = DATA_B_RE.match(line)
        if m:
            vals = [v.strip() for v in m.group(1).split(",") if v.strip()]
            byte_vals = [int(v.lstrip("H'"), 16) if v.startswith("H'") else int(v, 0)
                         for v in vals]
            cur.items.append(("B", byte_vals))
            continue

        m = DATA_L_RE.match(line)
        if m:
            cur.items.append(("L", m.group(1)))
            continue

        m = RES_B_RE.match(line)
        if m:
            cur.items.append(("B", [0] * int(m.group(1), 0)))
            continue

    return blocks


def check_forward_refs(blocks):
    pos = {b.name: i for i, b in enumerate(blocks)}
    bad = []
    for i, b in enumerate(blocks):
        for kind, val in b.items:
            if kind == "L" and pos[val] > i:
                bad.append((b.name, val))
    return bad


def emit_byte_array(name, byte_vals, const, public):
    qual = "const " if const else ""
    storage = "" if public else "STATIC "
    lines = [f"{storage}{qual}Uint8 {name}[] = {{"]
    for i in range(0, len(byte_vals), 8):
        row = ", ".join(f"0x{v:02X}" for v in byte_vals[i:i + 8])
        lines.append(f"    {row},")
    lines.append("};")
    return "\n".join(lines)


def emit_int_array(name, block, const, public):
    qual = "const " if const else ""
    storage = "" if public else "STATIC "
    # Merge into a flat token stream, then pack consecutive byte runs into
    # little-endian 32-bit words.
    words = []
    pending = []

    def flush_pending():
        if not pending:
            return
        if len(pending) % 4 != 0:
            raise ValueError(
                f"{name}: {len(pending)}-byte run isn't a multiple of 4; "
                "can't pack alongside pointer entries")
        for i in range(0, len(pending), 4):
            b0, b1, b2, b3 = pending[i:i + 4]
            val = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24)
            words.append(f"0x{val:08X}")
        pending.clear()

    for kind, val in block.items:
        if kind == "B":
            pending.extend(val)
        else:
            flush_pending()
            words.append(f"(int){val}")
    flush_pending()

    lines = [f"{storage}{qual}int {name}[] = {{"]
    for i in range(0, len(words), 4):
        lines.append("    " + ", ".join(words[i:i + 4]) + ",")
    lines.append("};")
    return "\n".join(lines)


def ascii_safe_comment(text):
    """Render a (possibly Shift-JIS) string for a C comment using only ASCII,
    per the src/ ASCII-only rule -- non-ASCII bytes become \\xHH escapes of
    their original Shift-JIS encoding rather than raw UTF-8."""
    out = []
    for ch in text:
        if ord(ch) < 0x80:
            out.append(ch)
        else:
            for b in ch.encode("shift_jis", errors="replace"):
                out.append(f"\\x{b:02x}")
    return "".join(out)


def emit_block(block, public_names):
    # The compiler places `const`-qualified globals in its own object's C
    # (rodata) section and everything else in D, regardless of which section
    # the label came from in the .src -- so reproducing the original C/D
    # split means: section-C labels (always plain `const_*` byte data here)
    # must be marked const, section-D labels (`init_*`) must not be.
    const = block.section == "C"
    public = block.name in public_names
    out = []
    if block.sdata is not None:
        out.append(f"/* \"{ascii_safe_comment(block.sdata)}\" */")
    has_ptr = any(k == "L" for k, _ in block.items)
    if has_ptr:
        out.append(emit_int_array(block.name, block, const, public))
    else:
        byte_vals = [v for k, vals in block.items for v in vals]
        out.append(emit_byte_array(block.name, byte_vals, const, public))
    return "\n".join(out)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("src")
    ap.add_argument("--sections", default="C,D", help="comma-separated section letters (default C,D)")
    ap.add_argument("--exclude", default="", help="comma-separated label names to skip (already moved)")
    ap.add_argument("--only", default="", help="comma-separated label names to emit (skip everything else)")
    ap.add_argument("--public", default="", help="comma-separated label names to emit without STATIC (external linkage)")
    ap.add_argument("--check-forward-refs", action="store_true",
                     help="only report .DATA.L targets defined later in the file, then exit")
    args = ap.parse_args()

    sections = tuple(args.sections.split(","))
    blocks = parse_blocks(args.src, sections)

    bad = check_forward_refs(blocks)
    if args.check_forward_refs:
        if bad:
            for name, target in bad:
                print(f"forward ref: {name} -> {target}", file=sys.stderr)
            sys.exit(1)
        print(f"OK: {len(blocks)} blocks, no forward references", file=sys.stderr)
        return
    if bad:
        print(f"warning: {len(bad)} forward reference(s) found; emitted code "
              "will not compile without reordering. Run with "
              "--check-forward-refs for the full list.", file=sys.stderr)

    exclude = set(n for n in args.exclude.split(",") if n)
    only = set(n for n in args.only.split(",") if n)
    public = set(n for n in args.public.split(",") if n)

    for block in blocks:
        if block.name in exclude:
            continue
        if only and block.name not in only:
            continue
        print(emit_block(block, public))
        print()


if __name__ == "__main__":
    main()
