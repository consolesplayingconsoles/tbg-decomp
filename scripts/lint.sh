#!/usr/bin/env bash
# Naming/header conventions + dead .IMPORT/.EXPORT check.
# See AGENTS.md "Naming Conventions" and scripts/check_naming.py's docstring.
#
# Every check runs even when an earlier one reports violations, and the exit
# status is decided at the end: a standing violation in one check must not
# silently skip the checks after it.
set -e

# --no-build: skip `make clean all` and check whatever objects already exist
# under build/output (and build/output_matching, if present). Mid-unit, a full
# build fails BY DESIGN -- other units still call the not-yet-decompiled
# functions -- so this is the only way to run these checks before a unit is
# finished. Default behavior (full clean build first) is unchanged.
no_build=0
if [ "${1:-}" = "--no-build" ]; then
  no_build=1
  shift
fi

if [ "$no_build" -eq 0 ]; then
  # Pin the canonical (Japanese) config: the naming rules trace every symbol back
  # to an address in the original binary, which only this build has. Passed on the
  # command line so an exported GAME_LANG can't override the Makefile's `?=`.
  make clean all GAME_LANG=ja
fi

set +e
fail=0

echo "=== check_naming.py ==="
python3 scripts/check_naming.py || fail=1

echo
echo "=== check_private_decls.py ==="
python3 scripts/check_private_decls.py || fail=1

echo
echo "=== clear_unused_imports.py --dry-run ==="
unused=$(python3 scripts/clear_unused_imports.py --dry-run src/asm/*.src src/asm/decompiled/*.src)
echo "$unused"
if echo "$unused" | grep -q "^Would remove [1-9]"; then
  echo "FAIL: unused .IMPORT/.EXPORT directives found -- run scripts/clear_unused_imports.py to fix"
  fail=1
fi

# Informational: never blocks, but a non-zero exit here means the script itself
# broke rather than that it found mismatches.
echo
echo "=== check_test_naming.py (informational) ==="
python3 scripts/check_test_naming.py || echo "NOTE: check_test_naming.py exited non-zero"

echo
if [ $fail -ne 0 ]; then
  echo "FAIL: one or more checks reported violations"
else
  echo "All blocking checks passed"
fi
exit $fail
