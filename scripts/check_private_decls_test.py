#!/usr/bin/env python3
"""Regression tests for check_private_decls.py's comment-stripping.

No existing test harness covers the scripts/ Python checkers, so this is a
minimal stdlib-only unittest module -- run directly, not part of any suite:

    python3 scripts/check_private_decls_test.py
"""
import unittest

from check_private_decls import header_declares


class HeaderDeclaresTest(unittest.TestCase):
    def test_real_prototype_matches(self):
        header = "void Foo_8c010000(int x);\n"
        self.assertTrue(header_declares(header, "Foo_8c010000", "func"))

    def test_doc_comment_naming_function_does_not_match(self):
        # A doc comment that names a function ahead of a "(" (e.g. an
        # unrelated call example) must not false-match as a declaration --
        # this is the bug that cost real debugging time (see NEEDS_ATTENTION).
        header = "/* calls Foo_8c010000(x) internally, not declared here */\n"
        self.assertFalse(header_declares(header, "Foo_8c010000", "func"))

    def test_multiline_block_comment_does_not_match(self):
        header = (
            "/* Foo_8c010000(\n"
            " * still just a comment\n"
            " */\n"
        )
        self.assertFalse(header_declares(header, "Foo_8c010000", "func"))

    def test_prototype_after_block_comment_still_matches(self):
        header = (
            "/* Foo_8c010000( looks like a call in a comment */\n"
            "void Foo_8c010000(int x);\n"
        )
        self.assertTrue(header_declares(header, "Foo_8c010000", "func"))

    def test_data_symbol_in_comment_does_not_match(self):
        header = "// see var_8c010000 for details\n"
        self.assertFalse(header_declares(header, "var_8c010000", "data"))

    def test_data_symbol_declared_matches(self):
        header = "extern int var_8c010000;\n"
        self.assertTrue(header_declares(header, "var_8c010000", "data"))


if __name__ == "__main__":
    unittest.main()
