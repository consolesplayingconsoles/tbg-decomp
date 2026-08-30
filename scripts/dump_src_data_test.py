#!/usr/bin/env python3
"""Regression tests for dump_src_data.py.

No existing test harness covers the scripts/ Python checkers, so this is a
minimal stdlib-only unittest module -- run directly, not part of any suite:

    python3 scripts/dump_src_data_test.py

The live-.SDATA byte layout asserted here (test_live_sdata_matches_assembled_bytes)
was verified against the real assembler: `.SDATA "common_parts.dat"` from
src/asm/decompiled/015ab8_title.src was assembled with asmsh.exe and inspected
with `sh4objtest inspect -x`, which showed the literal ASCII bytes of the
string with NO implicit terminator -- section C offset 0x0..0x11 is exactly
"common_parts.dat" (17 bytes), followed by the file's own `.DATA.B H'00` /
`.RES.B 3` for the terminator and alignment padding.
"""
import os
import tempfile
import unittest

from dump_src_data import ascii_safe_comment, emit_block, parse_blocks


def write_src(text):
    f = tempfile.NamedTemporaryFile(mode="wb", suffix=".src", delete=False)
    f.write(text.encode("shift_jis"))
    f.close()
    return f.name


class LiveSdataTest(unittest.TestCase):
    def test_live_sdata_matches_assembled_bytes(self):
        # Mirrors src/asm/decompiled/015ab8_title.src's
        # _const_8c035fb8 / _const_8c035fcc pair.
        path = write_src(
            "          .SECTION    C, DATA, ALIGN=4\n"
            "_const_8c035fb8:        ; from ghidra\n"
            "          .SDATA      \"common_parts.dat\"\n"
            "          .DATA.B     H'00\n"
            "          .RES.B      3\n"
            "_const_8c035fcc:        ; from ghidra\n"
            "          .SDATA      \"common.dat\"\n"
            "          .DATA.B     H'00\n"
            "          .RES.B      1\n"
            "          .END\n"
        )
        try:
            blocks = parse_blocks(path)
        finally:
            os.unlink(path)

        self.assertEqual(len(blocks), 2)
        first, second = blocks

        first_bytes = [v for k, vals in first.items for v in vals]
        self.assertEqual(
            first_bytes,
            list(b"common_parts.dat") + [0, 0, 0, 0],
        )
        self.assertEqual(len(first_bytes), 20)  # 17 + null + 3 pad, 4-aligned

        second_bytes = [v for k, vals in second.items for v in vals]
        self.assertEqual(second_bytes, list(b"common.dat") + [0, 0])

    def test_live_sdata_with_trailing_comment(self):
        # e.g. src/asm/decompiled/012f44_game.src: `.SDATA "..." ; H'8c033364`
        path = write_src(
            "          .SECTION    D, DATA, ALIGN=4\n"
            "_init_8c033364:\n"
            "          .SDATA      \"bus.mlt\" ; H'8c033378\n"
            "          .DATA.B     H'00\n"
            "          .END\n"
        )
        try:
            blocks = parse_blocks(path)
        finally:
            os.unlink(path)

        byte_vals = [v for k, vals in blocks[0].items for v in vals]
        self.assertEqual(byte_vals, list(b"bus.mlt") + [0])

    def test_commented_sdata_is_documentation_only(self):
        # The Shift-JIS case: the real bytes are the raw .DATA.B hex; the
        # commented ;.SDATA line is documentation, not live data.
        path = write_src(
            "          .SECTION    C, DATA, ALIGN=4\n"
            "_const_8c0380a4:        ; from defines\n"
            "          ;.SDATA      \"9/   EXP \"\n"
            "          ;.DATA.B     H'00\n"
            "          ;.RES.B      2\n"
            "          .DATA.B      H'39, H'2F, H'20, H'20, H'20, H'45, H'58, H'50, H'20, H'00, H'00, H'00\n"
            "          .END\n"
        )
        try:
            blocks = parse_blocks(path)
        finally:
            os.unlink(path)

        byte_vals = [v for k, vals in blocks[0].items for v in vals]
        self.assertEqual(len(byte_vals), 12)
        self.assertEqual(blocks[0].sdata, "9/   EXP ")


class PublicLinkageTest(unittest.TestCase):
    def make_block(self):
        path = write_src(
            "          .SECTION    D, DATA, ALIGN=4\n"
            "_var_8c000000:\n"
            "          .DATA.B     H'01, H'02, H'03, H'04\n"
            "          .END\n"
        )
        try:
            return parse_blocks(path)[0]
        finally:
            os.unlink(path)

    def test_defaults_to_static(self):
        block = self.make_block()
        out = emit_block(block, set())
        self.assertIn("STATIC", out)

    def test_public_name_omits_static(self):
        block = self.make_block()
        out = emit_block(block, {"var_8c000000"})
        self.assertNotIn("STATIC", out)


class AsciiSafeCommentTest(unittest.TestCase):
    def test_ascii_text_is_unchanged(self):
        self.assertEqual(ascii_safe_comment("9/   EXP "), "9/   EXP ")

    def test_output_is_pure_ascii(self):
        text = ascii_safe_comment("バスでとうとータ")
        text.encode("ascii")  # raises if any non-ASCII char slipped through

    def test_shift_jis_bytes_are_escaped_not_dropped(self):
        # The Shift-JIS bytes for the character must show up as \xHH escapes,
        # not be silently discarded.
        text = ascii_safe_comment("バ")  # U+30D0 (Katakana BA)
        expected = "".join(f"\\x{b:02x}" for b in "バ".encode("shift_jis"))
        self.assertEqual(text, expected)


if __name__ == "__main__":
    unittest.main()
