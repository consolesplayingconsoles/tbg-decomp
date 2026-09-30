#!/usr/bin/env python3
"""Dump the CPU-traffic layout of one course's *_MAC_CPU*.DAT blob.

The file is the course's `macCpu1_0x24` blob (loaded in
013ae8_route.c:456). Everything below is derived from the decompiled
026710_traffic.c -- the cited functions are the ground truth:

Relocation (FUN_8c026da4)
  The blob is position-independent: it starts with an array of self-relative
  32-bit offsets from the blob's own base, terminated by 0. Each entry points
  at a run of 0xc-byte records; within a run, each record's dword at +4 is
  itself a self-relative offset (to the record's script), and the run ends at
  the first record whose +4 is 0. The loader rewrites both levels in place to
  absolute pointers; this script keeps them as file offsets instead.

  The outer array is the course's traffic *preset* table: each entry is one
  complete alternative traffic set for the whole course, not a segment of one.
  trafficUpdateTask_8c0275d4 picks
  `var_trafficPresetTable_8c227e18[presetId]` (that symbol being
  CurrentCourse.macCpu1_0x24) using bits 8-15 of var_scenePresetIds_8c1bbd8c,
  and re-seeds its record cursor whenever the id changes.

  That word holds three independent preset ids swapped together -- traffic in
  bits 8-15, pedestrians in 16-23 (var_pedGroupLists_8c228240), and a
  set-piece trigger in 24-31 (the FUMI railway crossing). It was originally
  named after "demo", but it is unrelated to the attract-mode demo/replay
  system.

Record (trafficUpdateTask_8c0275d4, spawnEntry_8c0272b8)
  0xc bytes: { Uint16 typeCode, Uint16 threshold, Sint32 script, float
  progress }. The task walks one record at a time: a per-record counter ticks
  every frame and, once it passes `threshold`, the record is consumed --
  normally by spawnEntry_8c0272b8(typeCode, progress, script).

  typeCode: bit 0x8000 selects the animation kind, bit 0x4000 the
  elevated-road ground-probe pair; the low 12 bits index
  var_routeModelSlots_8c1bbddc (always even -- slot n is the large LOD, n+1
  the small one), and halved they give the 0..15 variant index used for
  init_8c0460d4 (dimensions), init_8c0461c8 and init_8c04622c (-> the body
  model in init_trafficModelFiles_8c043d64). Variants 14/15 (police,
  ambulance, i.e. low byte 0x1c/0x1e) are additionally gated on a
  per-(route, timeOfDay) day bitmask, init_8c046208.

Script (TrafficRunEntryScript_8c027012, TrafficReadScriptArgs_8c026710)
  A stream of 16-bit words starting at the script pointer itself -- the
  interpreter's cursor (entry+0x2fc) is seeded with `script`, so word 0 is
  already the first opcode. It doubles as the script's type discriminator:
  spawnEntry_8c0272b8 and initEntryState_8c026748 both test `*script == 10`,
  a fixed decoration (traffic light, sign) rather than a path-following
  vehicle; every other script opens with opcode 0 (spawn). That is also why
  TrafficReadScriptArgs_8c026710 starts its walk at script+2: it is skipping
  that leading 1-word opcode 0. Each instruction is `op` followed by
  operands, with the total word length given by init_8c0460bc:

      0  spawn                    1 word   (see below)
      1  advance to next block    3 words  op, pathId, laneRatio(/65536)
      2  config A                 3 words  op, a, b
      3  config B                 3 words  op, a, b   (same, 0x430 = 1)
      4  skip / no-op             3 words
      5  decoration slot 5        2 words  op, a
      6  decoration slot 6        3 words  op, a, b
      7  decoration slot 7        2 words  op, a
      8  id-resolved config       4 words  op, a, b, pathId
      9  end                      1 word
      10 place fixed decoration   4 words  op, x*10, z*10, angle

  `pathId` operands (opcodes 1 and 8) index CurrentCourse.lineCpu_0x1c
  (var_8c227e1c), a different blob holding the path blocks.

  Two original quirks, reproduced here rather than "fixed":
    * opcode 9 does not advance the runtime cursor (it parks on itself),
    * opcode 10 advances the runtime cursor by only 3 of its 4 words, so the
      cursor is left parked on the angle operand.
  Neither affects the static walk, which uses init_8c0460bc throughout --
  exactly as TrafficReadScriptArgs_8c026710 does when it pre-resolves the
  opcode-1 path ids.

Usage:
  scripts/dump_traffic_layout.py <course_MAC_CPU1.DAT> [-o out.txt]
"""
import argparse
import collections
import struct
import sys

RECORD_SIZE = 0xC

# init_8c0460bc: total word length of each opcode, indexed by opcode.
OPCODE_LENGTHS = [1, 3, 3, 3, 3, 2, 3, 2, 4, 1, 4, 0]

OPCODE_NAMES = {
    0: "spawn",
    1: "next_block",
    2: "config_a",
    3: "config_b",
    4: "skip",
    5: "deco5",
    6: "deco6",
    7: "deco7",
    8: "config_id",
    9: "end",
    10: "place_deco",
}

# init_8c04622c: variant index (typeCode & 0xfff) >> 1 -> body model index
# into init_trafficModelFiles_8c043d64.
VARIANT_TO_BODY = [0, 0, 1, 2, 2, 2, 3, 4, 4, 5, 6, 6, 7, 8, 9, 10]

# init_trafficModelFiles_8c043d64 body names.
BODY_NAMES = [
    "2do", "4wd", "sed", "tax", "tor", "kto", "dan", "wag", "bus", "pat", "kyu",
]

# The 16 variants, named from init_8c043dc4's slot pairs (large/small LOD).
VARIANT_NAMES = [
    "2do0", "2do1", "4wd", "sed0", "sed1", "sed2", "tax", "tor0",
    "tor1", "kto", "dan0", "dan1", "wag", "bus0", "pat", "kyu",
]

# Per-variant dimensions: 16 records of 4 floats based at init_8c0460c8, read
# by initEntryState_8c026748 as entry+0x23c/0x244/0x24c/0x240 in that memory
# order. Ghidra split the first record's leading three floats off as
# init_8c0460c8/cc/d0 and named the rest init_8c0460d4, so the C file's
# init_8c0460d4 rows are shifted by one float against the real records --
# regrouped correctly here.
VARIANT_DIMS = [
    (2.42, 1.67, 0.65, 3.0500002),
    (2.42, 1.67, 0.65, 3.0500002),
    (2.2, 1.73, 0.76, 3.04),
    (2.8, 1.8, 0.9, 3.8),
    (2.8, 1.8, 0.9, 3.8),
    (2.8, 1.8, 0.9, 3.8),
    (2.8, 1.7, 0.7, 3.9),
    (6.1, 2.03, 1.5, 8.9),
    (6.1, 2.03, 1.5, 8.9),
    (2.7, 1.8, 1.0, 4.0),
    (4.0, 2.07, 2.0, 5.3),
    (4.0, 2.07, 2.0, 5.3),
    (2.6, 1.53, 0.9, 3.5),
    (4.9, 2.33, 2.6, 8.0),
    (2.8, 1.69, 0.74, 3.96),
    (2.6, 1.68, 1.0, 3.8999999),
]


class FormatError(Exception):
    """The blob does not match the format the decompiled loader implements."""


def u16(blob, off):
    return struct.unpack_from("<H", blob, off)[0]


def s32(blob, off):
    return struct.unpack_from("<i", blob, off)[0]


def f32(blob, off):
    return struct.unpack_from("<f", blob, off)[0]


def describe_type_code(code):
    """Decode a record typeCode into its variant / model-slot meaning."""
    slot = code & 0xFFF
    variant = slot >> 1
    flags = []
    if code & 0x8000:
        flags.append("anim4")
    if code & 0x4000:
        flags.append("elevated")
    other = code & ~(0xFFF | 0x8000 | 0x4000)
    if other:
        flags.append("unknown:0x%04x" % other)

    if variant < len(VARIANT_NAMES):
        name = VARIANT_NAMES[variant]
        body = BODY_NAMES[VARIANT_TO_BODY[variant]]
        desc = "%s (3s_%s_x)" % (name, body)
        known = True
    else:
        desc = "variant %d OUT OF RANGE" % variant
        known = False
    if slot & 1:
        desc += " [ODD SLOT]"
        known = False
    if flags:
        desc += " {%s}" % ",".join(flags)
    return desc, known, variant


class Script(object):
    def __init__(self, offset):
        self.offset = offset
        self.kind = None       # first opcode: 10 = fixed decoration, else vehicle
        self.instructions = []   # (word_offset, opcode, [operands])
        self.terminator = None   # 9, or None (ran off the end / bad)
        self.error = None


def parse_script(blob, offset):
    """Static walk of one script, using init_8c0460bc word lengths."""
    script = Script(offset)
    if offset + 2 > len(blob):
        script.error = "script at 0x%x is past end of file" % offset
        return script
    script.kind = u16(blob, offset)

    pos = offset
    while True:
        if pos + 2 > len(blob):
            script.error = "ran off the end of the file at 0x%x" % pos
            return script
        op = u16(blob, pos)
        if op >= len(OPCODE_LENGTHS) or OPCODE_LENGTHS[op] == 0:
            script.error = "unknown opcode %d at 0x%x" % (op, pos)
            return script
        length = OPCODE_LENGTHS[op]
        if pos + length * 2 > len(blob):
            script.error = "opcode %d at 0x%x extends past end of file" % (op, pos)
            return script
        operands = [u16(blob, pos + 2 * i) for i in range(1, length)]
        script.instructions.append((pos, op, operands))
        pos += length * 2
        if op == 9:
            script.terminator = 9
            return script
        if len(script.instructions) > 100000:
            script.error = "script did not terminate within 100000 instructions"
            return script


def format_instruction(word_off, op, operands):
    name = OPCODE_NAMES.get(op, "op%d" % op)
    text = "    %06x  %-11s" % (word_off, name)
    if op == 1 and len(operands) == 2:
        text += " path=%-4d lane=%.5f" % (operands[0], operands[1] / 65536.0)
    elif op == 8 and len(operands) == 3:
        text += " a=%-5d b=%-5d path=%d" % tuple(operands)
    elif op == 10 and len(operands) == 3:
        text += " x=%-9.1f z=%-9.1f angle=0x%04x" % (
            operands[0] / 10.0, operands[1] / 10.0, operands[2])
    elif operands:
        text += " " + " ".join("%d" % v for v in operands)
    return text.rstrip()


def parse(blob):
    """Return (runs, problems). runs is a list of (index, offset, records)."""
    problems = []
    runs = []
    scripts = {}

    i = 0
    while True:
        if (i + 1) * 4 > len(blob):
            raise FormatError(
                "offset array at index %d runs past end of file without a 0 "
                "terminator" % i)
        value = s32(blob, i * 4)
        if value == 0:
            break
        if not 0 <= value < len(blob):
            raise FormatError(
                "offset array entry %d = 0x%x is outside the file "
                "(size 0x%x)" % (i, value, len(blob)))
        runs.append([i, value, []])
        i += 1
    array_end = (i + 1) * 4

    for run in runs:
        idx, run_off, records = run
        rec_off = run_off
        while True:
            if rec_off + RECORD_SIZE > len(blob):
                raise FormatError(
                    "run %d: record at 0x%x extends past end of file" % (idx, rec_off))
            script_rel = s32(blob, rec_off + 4)
            if script_rel == 0:
                break
            if not 0 <= script_rel < len(blob):
                raise FormatError(
                    "run %d: record at 0x%x has script offset 0x%x outside the "
                    "file (size 0x%x)" % (idx, rec_off, script_rel, len(blob)))
            if script_rel not in scripts:
                scripts[script_rel] = parse_script(blob, script_rel)
            records.append({
                "offset": rec_off,
                "typeCode": u16(blob, rec_off),
                "threshold": u16(blob, rec_off + 2),
                "script": script_rel,
                "progress": f32(blob, rec_off + 8),
            })
            rec_off += RECORD_SIZE

    return runs, scripts, array_end, problems


def dump(blob, path, out):
    runs, scripts, array_end, problems = parse(blob)

    first = s32(blob, 0)
    out.write("Traffic layout dump: %s\n" % path)
    out.write("File size: %d bytes (0x%x)\n" % (len(blob), len(blob)))
    out.write("Endianness: little-endian (first offset-array entry reads "
              "0x%x LE / 0x%x BE; the file is 0x%x bytes, so only the LE "
              "reading is in range)\n" %
              (first, struct.unpack_from(">I", blob, 0)[0], len(blob)))
    out.write("Offset array: %d entries + terminator, ends at 0x%x\n" %
              (len(runs), array_end))
    out.write("\n")

    unknown_types = collections.Counter()
    type_counter = collections.Counter()
    opcode_counter = collections.Counter()
    bad_scripts = []
    total_records = 0

    for idx, run_off, records in runs:
        out.write("=" * 72 + "\n")
        out.write("Preset %d @ 0x%06x -- %d records\n" %
                  (idx, run_off, len(records)))
        out.write("=" * 72 + "\n")
        for n, rec in enumerate(records):
            total_records += 1
            desc, known, variant = describe_type_code(rec["typeCode"])
            type_counter[rec["typeCode"]] += 1
            if not known:
                unknown_types[rec["typeCode"]] += 1
            out.write("\n[%d.%d] @0x%06x type=0x%04x threshold=%-5d "
                      "progress=%-10g script=@0x%06x\n" %
                      (idx, n, rec["offset"], rec["typeCode"], rec["threshold"],
                       rec["progress"], rec["script"]))
            out.write("      %s\n" % desc)
            script = scripts[rec["script"]]
            out.write("      script kind: %s\n" %
                      ("fixed decoration (*script == 10)" if script.kind == 10
                       else "path-following vehicle"))
            for word_off, op, operands in script.instructions:
                out.write(format_instruction(word_off, op, operands) + "\n")
            if script.error:
                out.write("    *** %s\n" % script.error)
        out.write("\n")

    for off, script in scripts.items():
        for _, op, _ in script.instructions:
            opcode_counter[op] += 1
        if script.error is not None:
            bad_scripts.append((off, script.error))

    out.write("=" * 72 + "\n")
    out.write("Summary\n")
    out.write("=" * 72 + "\n")
    out.write("Traffic presets:       %d\n" % len(runs))
    out.write("Records:               %d\n" % total_records)
    out.write("Distinct scripts:      %d\n" % len(scripts))
    out.write("\nOpcode histogram (over distinct scripts):\n")
    for op in sorted(opcode_counter):
        out.write("  %2d %-11s %6d\n" %
                  (op, OPCODE_NAMES.get(op, "?"), opcode_counter[op]))

    out.write("\nType-code distribution:\n")
    for code in sorted(type_counter):
        desc, known, variant = describe_type_code(code)
        dims = ""
        if variant < len(VARIANT_DIMS):
            d = VARIANT_DIMS[variant]
            dims = "  dims=%.2f/%.2f/%.2f/%.2f" % d
        out.write("  0x%04x  %6d  %s%s\n" % (code, type_counter[code], desc, dims))

    out.write("\nSanity checks:\n")
    out.write("  offsets in range:    OK (a violation aborts the parse)\n")
    if bad_scripts:
        out.write("  script termination:  %d FAILED\n" % len(bad_scripts))
        for off, err in bad_scripts:
            out.write("    @0x%06x: %s\n" % (off, err))
    else:
        out.write("  script termination:  OK (all %d scripts end on opcode 9)\n"
                  % len(scripts))
    if unknown_types:
        out.write("  type codes known:    %d unmapped codes\n" % len(unknown_types))
        for code in sorted(unknown_types):
            out.write("    0x%04x x%d\n" % (code, unknown_types[code]))
    else:
        out.write("  type codes known:    OK (every code maps to one of the 16 "
                  "variants in init_8c04622c)\n")
    for problem in problems:
        out.write("  %s\n" % problem)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("dat", help="path to a course *_MAC_CPU*.DAT file")
    parser.add_argument("-o", "--output", help="write the dump here (default stdout)")
    args = parser.parse_args()

    with open(args.dat, "rb") as handle:
        blob = handle.read()

    out = open(args.output, "w") if args.output else sys.stdout
    try:
        dump(blob, args.dat, out)
    except FormatError as exc:
        sys.stderr.write("format error: %s\n" % exc)
        return 1
    finally:
        if args.output:
            out.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
