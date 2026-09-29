#!/usr/bin/env python3
"""Diff two SH4 objects' data sections via `sh4objtest inspect -x`.

Goal is functional equivalence, so section P (code, incl. literal pools) may differ;
C (const), D (init data) and B (bss) should match when the objects are meant to be
equivalent. Each data section's layout is checked too: its size, and the offset of
every symbol exported by both objects (one exported by only one side is a difference)
-- B has no bytes, so for bss that is the whole check.
Two things are normalised so faithful pointers don't look like diffs:
  * relocated 4-byte slots are masked out before the byte compare (the assembler
    stores addends in the reloc entry, the compiler stores them in-place);
  * relocations are compared as an ordered list of (TARGET, address). TARGET is the
    external symbol name, or internal section letter; address is the addend for
    internal relocs (offset into the target section) or the addend for external
    ones. Address is NOT checked for relocations that target section P: P's layout
    isn't required to match, so an offset into it isn't meaningful to compare.

Best for objects that should be identical: before/after a rename, or isolated
data-only objects. A not-yet-matched unit whose compiler lays constants out in a
different order will (correctly) report a difference.

Usage: scripts/dcdiff.py <a.obj> <b.obj>
Env:   SH4OBJTEST=path/to/sh4objtest   (default: sh4objtest)
"""
import json, os, re, subprocess, sys

SH4OBJTEST = os.environ.get("SH4OBJTEST", "sh4objtest")
SEC = re.compile(r"^Section \d+: (\w)\s+\[")
IDX = re.compile(r"^Section (\d+): (\w)\s+\[")
HEX = re.compile(r"^\s*0x([0-9a-fA-F]+):\s+(.*)$")
INT = re.compile(r"addr=0x([0-9a-fA-F]+)\s+-> section (\d+)\s+addend=(\S+)")
EXT = re.compile(r"addr=0x([0-9a-fA-F]+)\s+(\S+)\s+addend=(0x[0-9a-fA-F]+)\s+width=")


def inspect(obj):
    txt = subprocess.run([SH4OBJTEST, "inspect", obj, "-x"],
                         capture_output=True, text=True, check=True).stdout
    idx2letter, raw, cur = {}, {}, None
    for line in txt.splitlines():
        m = IDX.match(line)
        if m:
            cur = m.group(2)
            idx2letter[m.group(1)] = cur
            raw.setdefault(cur, {"bytes": {}, "rel": []})
            continue
        if cur is None:
            continue
        m = HEX.match(line)
        if m:
            off = int(m.group(1), 16)
            for i, b in enumerate(m.group(2).split("|")[0].split()):
                raw[cur]["bytes"][off + i] = int(b, 16)
            continue
        m = INT.search(line)
        if m:
            # the compiler stores the addend in-place in the section bytes
            # instead of the reloc entry; resolved to a number once we have
            # the raw bytes for this section (below).
            addend = m.group(3) if m.group(3) == "in-place" else int(m.group(3), 16)
            raw[cur]["rel"].append((int(m.group(1), 16), "S", m.group(2), addend))
            continue
        m = EXT.search(line)
        if m:
            raw[cur]["rel"].append((int(m.group(1), 16), "X", m.group(2), int(m.group(3), 16)))
    secs = {}
    for letter, r in raw.items():
        size = (max(r["bytes"]) + 1) if r["bytes"] else 0
        buf = bytearray(size)
        for off, b in r["bytes"].items():
            buf[off] = b
        rel = [(addr, k, v, int.from_bytes(buf[addr:addr + 4], "little") if addend == "in-place" else addend)
               for addr, k, v, addend in r["rel"]]
        offsets = sorted(addr for addr, _, _, _ in rel)
        for o in offsets:
            buf[o:o + 4] = b"\0\0\0\0"                      # mask reloc slots
        entries = sorted(rel)                               # ordered by addr
        targets = [(k, idx2letter.get(v, v) if k == "S" else v) for _, k, v, _ in entries]
        addends = [None if k == "S" and idx2letter.get(v, v) == "P" else addend
                   for _, k, v, addend in entries]           # skip address check into P
        secs[letter] = (buf, targets, addends)
    return secs


def layout(obj):
    """{section letter: (size, {exported symbol: offset})} from inspect's JSON."""
    txt = subprocess.run([SH4OBJTEST, "inspect", obj, "--format=json"],
                         capture_output=True, text=True, check=True).stdout
    doc = json.loads(txt[txt.index("{"):])   # skip any WARN line ahead of the JSON
    return {sec["name"]: (sec["length"], {e["name"]: e["offset"] for e in sec.get("exports", [])})
            for sec in doc["sections"]}


def layout_msgs(la, lb):
    (sa, ea), (sb, eb) = la, lb
    msgs = []
    if sa != sb:
        msgs.append(f"size differs (0x{sa:x} vs 0x{sb:x})")
    moved = [n for n in sorted(ea.keys() & eb.keys(), key=ea.get) if ea[n] != eb[n]]
    if moved:
        n = moved[0]
        msgs.append(f"{len(moved)} symbol offset(s) differ; first {n}: 0x{ea[n]:x} vs 0x{eb[n]:x}")
    for mine, theirs, side in ((ea, eb, sys.argv[1]), (eb, ea, sys.argv[2])):
        only = sorted(mine.keys() - theirs.keys(), key=mine.get)
        if only:
            msgs.append(f"{len(only)} symbol(s) exported only by {side}: "
                        + ", ".join(only[:5]) + (", ..." if len(only) > 5 else ""))
    return msgs


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    a, b = inspect(sys.argv[1]), inspect(sys.argv[2])
    la, lb = layout(sys.argv[1]), layout(sys.argv[2])
    diffs = 0
    for letter in sorted(set(a) | set(b)):
        tag = "code, differences expected" if letter == "P" else None
        if letter not in a or letter not in b:
            print(f"[{letter}] only in {sys.argv[1] if letter in a else sys.argv[2]}")
            diffs += letter != "P"
            continue
        (ba, ta, aa), (bb, tb, ab) = a[letter], b[letter]
        msgs = []
        if ba != bb:
            n = min(len(ba), len(bb))
            off = next((i for i in range(n) if ba[i] != bb[i]), n)
            msgs.append(f"data differs at 0x{off:x} (len {len(ba)} vs {len(bb)})")
        if ta != tb:
            n = min(len(ta), len(tb))
            i = next((k for k in range(n) if ta[k] != tb[k]), n)
            first = f"{ta[i] if i < len(ta) else None} vs {tb[i] if i < len(tb) else None}"
            msgs.append(f"{sum(x != y for x, y in zip(ta, tb)) + abs(len(ta) - len(tb))} "
                        f"reloc target(s) differ; first at #{i}: {first}")
        if aa != ab:
            n = min(len(aa), len(ab))
            i = next((k for k in range(n) if aa[k] != ab[k]), n)
            first = f"{aa[i] if i < len(aa) else None} vs {ab[i] if i < len(ab) else None}"
            msgs.append(f"{sum(x != y for x, y in zip(aa, ab)) + abs(len(aa) - len(ab))} "
                        f"reloc address(es) differ; first at #{i}: {first}")
        if letter != "P":
            msgs += layout_msgs(la[letter], lb[letter])
        if msgs:
            print(f"[{letter}] " + "; ".join(msgs) + (f"  ({tag})" if tag else ""))
            diffs += letter != "P" and bool(msgs)
        else:
            size = la[letter][0] if letter in la else len(ba)
            syms = len(la[letter][1]) if letter in la else 0
            print(f"[{letter}] match ({size} bytes, {len(ta)} relocs, {syms} symbols)")
    print("\nMATCH" if diffs == 0 else f"\n{diffs} section(s) differ")
    sys.exit(1 if diffs else 0)


if __name__ == "__main__":
    main()
