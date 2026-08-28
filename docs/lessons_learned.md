# Lessons Learned

Indexed, non-obvious findings from decompiling this codebase. Each entry is a
mistake made once, or a quirk of the toolchain/original build, worth not
re-discovering. Add an entry whenever you hit something surprising; keep each
one short and dated with the unit where it was found.

## Index

- [Struct fields can be separately-imported symbols in asm](#struct-fields-can-be-separately-imported-symbols-in-asm)
- [Calls to sibling functions in the same TU are still mocked](#calls-to-sibling-functions-in-the-same-tu-are-still-mocked)
- [A nested single-statement `if(cond){break;}` can compile to unreachable bytes](#a-nested-single-statement-ifcondbreak-can-compile-to-unreachable-bytes)
- [`asmsh -debug` embeds symbols in the .obj; `-debug=d` writes a side .DWF](#asmsh--debug-embeds-symbols-in-the-obj--debugd-writes-a-side-dwf)
- [Marking known-dead asm lines with coverage tags](#marking-known-dead-asm-lines-with-coverage-tags)
- [Renaming an exported symbol: unit tests won't catch a missed caller](#renaming-an-exported-symbol-unit-tests-wont-catch-a-missed-caller)
- [Strength-reduced loops and nested ifs fold back to idiomatic C](#strength-reduced-loops-and-nested-ifs-fold-back-to-idiomatic-c)

## Struct fields can be separately-imported symbols in asm

**Found in:** `02af78_event` (2026-07-12)

A field we already model as part of a C struct (e.g. `PlayerProgress.field_0x04`
within `var_progress_8c1ba1cc`) can show up in some *other* unit's asm as its
own separately-`.IMPORT`ed symbol (e.g. `_var_8c1ba1d0`) that happens to sit at
that struct's field address — visible because the owning unit's `.src` exports
both the base symbol and the field-offset symbol at contiguous addresses.

If you leave the separate import as-is in a decompiled-and-tested unit, the C
object (which addresses the field via the struct) and the src object (which
addresses it via the standalone symbol) both resolve to the right address in
the real build, but the **unit test** must satisfy both — normally caught as
`Trying to read from unresolved relocation _var_<addr>` when only the struct's
base symbol was sized/allocated.

**Fix:** don't work around this in the test with `rellocate()`. Instead, treat
the standalone import as the wrongly-disassembled artifact and correct the
asm: drop the `.IMPORT`, and rewrite every use (typically a `.DATA.L` literal
pool entry) as `_baseSymbol+H'<offset>` sum-expression addressing instead of
the separate name. This keeps one C struct as the single source of truth and
avoids the test-side alias entirely. The owning unit's now-unreferenced
`.EXPORT` line for the dropped symbol can be left alone if that unit isn't
decompiled yet — it's harmless dead export, not worth touching out of scope.

## Calls to sibling functions in the same TU are still mocked

**Found in:** `02af78_event` (2026-07-12)

A function that calls another function decompiled earlier *in the same
`.c`/`.src` file* (e.g. `scanUnlockCandidates_8c02b03c` calling
`hasProgressFlag_8c02afbe`, both in `02af78_event.c`) does **not** execute the
callee for real during a unit test, even though both end up in the same
object file. The test harness still intercepts the `BSR`/call and requires
an explicit `shouldCall(...)->andReturn(...)` expectation, exactly as for a
genuinely external/cross-TU call. Omitting it fails with `Unexpected
function call to _<callee>`.

**Fix:** always mock intra-TU calls with `shouldCall()`/`andReturn()` rather
than seeding the callee's real backing memory and expecting it to run for
real -- this is simpler anyway and keeps the test focused on the function
under test.

## A nested single-statement `if(cond){break;}` can compile to unreachable bytes

**Found in:** `02af78_event` (2026-07-12)

Pattern:
```c
if (mode_matches) {
    if (some_call(...) != 0) {
        break;
    }
} else if (other_mode && other_call(...) == 0) {
    break;
}
```
For the *first* arm only, this compiler (SHC, 1997) folds the outer and inner
condition into one instruction that branches straight past the loop body to
the post-loop code -- but it still emits the literal `break;`'s own bytes
(an unconditional branch to the same target) immediately after, as dead
filler that no control-flow edge ever reaches. `sh4objtest --coverage` then
reports that line as permanently uncovered no matter what the test does,
because it genuinely never executes -- confirmed via `sh4objtest inspect
--format=json`'s `debugLines` (per-line address ranges) plus `-d` trace: the
branch for the enclosing `if` jumps directly over the `break;`'s address
range.

**Fix (best, in C):** split the `if/else if` into two independent top-level
`if`s, ending the first arm with an explicit `continue;` instead of letting
it fall out of the `if`:
```c
if (mode_matches) {
    if (some_call(...) != 0) {
        break;
    }
    continue;
}
if (other_mode && other_call(...) == 0) {
    break;
}
```
This isn't just cosmetic -- it changes codegen. With the `else if` chain, the
compiler has to jump *past* the second condition entirely when the first
arm's outer test is true, and that extra unconditional jump is what leaves
the dead `break;` bytes behind. With two independent `if`s, the first arm's
only fallthrough target *is* `continue`'s target, so the compiler folds the
`break;`'s jump-out-of-loop directly into the same conditional branch that
tests `some_call(...) != 0` -- confirmed via `sh4objtest inspect
--format=json`'s `debugLines`: the `break;` line has no address range of its
own at all (zero-width), rather than a range coverage reports as dead.
Result: no coverage gap, no exclusion tag needed, and the shape (guard
clause + independent checks) is arguably clearer than the original nested
`if/else if` anyway.

**Fix (when the `continue` restructuring doesn't apply, e.g. no loop to
`continue` in, or sh4objtest < v0.1.35):** rewrite the first arm to match
the second arm's shape -- combine the two conditions with `&&` into a single
`if (mode_matches && some_call(...) != 0) { break; }` instead of nesting.
Same behavior, one direct conditional branch, no leftover dead bytes.

**Fix (last resort):** tag the dead `break;` with `coverage:ignore-next-line`
(see the entry below, requires sh4objtest >= v0.1.35) and leave the
`if/else if` shape as-is. Only reach for this when neither restructuring
above is a natural fit -- an exclusion tag hides a genuine dead-code
artifact instead of removing it, so a real fix is always preferable when one
exists.

The dead-bytes pattern itself also shows up verbatim in the *original*
`.src` asm (built by the same-era SHC compiler -- confirmed present in
`02af78_event.src`'s `_scanUnlockCandidates_8c02b03c`/`_pickUnlockCandidate_8c02b170`).
There, restructuring is never an option regardless of sh4objtest version --
the archived asm must stay byte-identical to the real game binary -- so the
coverage tags below are the only fix.

## Marking known-dead asm lines with coverage tags

**Found in:** `02af78_event` (2026-07-12), sh4objtest v0.1.35+

`sh4objtest suite --coverage` supports source-line exclusion tags, matched as
plain substrings so they work in both `//`/`/* */` (C) and `;` (asm)
comments:

- `coverage:disable` / `coverage:enable` -- exclude an inclusive block
- `coverage:ignore-next-line` -- exclude only the following line

They require `tests.php` to declare a `sourcePaths` map from the Wine debug
path prefix to a host directory relative to `tests.php`'s own location, e.g.
`'sourcePaths' => ['Z:\\app\\src' => 'src']`. Without it the tags are
silently ignored (`ignoredLinesFor()` returns nothing when the debug path
doesn't resolve).

**Use case:** original `.src` asm legitimately contains code that's
unreachable in practice and that we must not rewrite (see the entry above,
and negative-index-handling branches from `CMP/PZ`-guarded shift/modulo
codegen that no real caller ever triggers, and one-off alignment `.RES.W`
padding words in a literal pool, which aren't code at all). Tag these with a
short comment explaining *why* they're unreachable instead of leaving
`--coverage` to report them as gaps indistinguishable from a genuine missing
test -- also requires bumping `docker/Dockerfile`'s sh4objtest version and
rebuilding the local image (`docker build -t lhsazevedo/tbg-decomp docker/`)
to pick up a version that supports the tags -- `docker-run.sh`/`docker-shell.sh`
use a prebuilt image tag, so editing the Dockerfile alone does nothing until
it's rebuilt.

## Renaming an exported symbol: unit tests won't catch a missed caller

**Found in:** `02af78_event` (2026-07-12)

When you rename a `.EXPORT`ed function/variable, the rename has to reach
*every* caller across the whole tree, not just the owning unit -- and a
green test suite does **not** prove you got them all. Two structural blind
spots in the harness hide a missed caller:

- Each unit test loads only its own object(s), and cross-unit calls are
  mocked (see the sibling-mock entry), so no test ever links the renamed
  symbol against a stale importer in another unit.
- A single-file compile of the renamed unit succeeds too -- nothing it
  builds references the old name.

The miss only surfaces at the **full `make` link** as
`105 UNDEFINED EXTERNAL SYMBOL(<unit>._<oldname>)`, because still-undecompiled
asm units keep `.IMPORT`ing the old name.

**Fix:** blanket-sed the old name across the whole tree in one shot --
`find src tests -type f -exec sed -i 's/<old>/<new>/g' {} +` -- rather than
grepping first and sed'ing a filtered list. There's no need to know *where*
the callers are up front; a single recursive pass covers `.c`/`.h`, *all*
`.src` (callers hold it as `.IMPORT` + `.DATA.L _oldname` literal-pool
entries), `tests/` mocks, and comments together. Omit the leading `_` so
`foo`->`bar` also catches the asm `_foo`. Then run the full `make` link (not
just `run_tests.sh`): that link, via UNDEFINED EXTERNAL SYMBOL, is the only
check that proves the sed reached every caller -- it's the verification step,
not a pre-sed grep.

## Strength-reduced loops and nested ifs fold back to idiomatic C

**Found in:** `02af78_event` (2026-07-12)

Because the goal is functional equivalence -- and the exact original
structure stays archived byte-for-byte in `.src` -- the decompiled C is free
to take a more idiomatic shape than the Ghidra/asm output, and usually
should. Reshaping isn't just cosmetic: it changes codegen, and can remove
compiler dead-code artifacts (see the break-bytes entry). Two recurring
foldbacks:

- **Pointer-walk + separate trailing counter -> plain indexed loop.** A loop
  that advances a struct pointer (`entry++` in the `for` step) while a
  *second* variable counts (`index++` at the bottom of the body, used only to
  index a parallel array) is the compiler's strength-reduction of one indexed
  loop. When `index == entry - base` holds every iteration, drop the walking
  pointer: `for (index = 0; base[index]...; index++)` with a local
  `entry = &base[index]`. One variable instead of two kept in lockstep, and
  `continue` guards then work correctly (they still reach the loop's own
  `index++`, which a hand-placed bottom-of-body `index++` would skip).

- **Nested `if`/`else if` chains -> guard clauses with early
  `continue`/`return`.** Invert per-iteration `if (match) { body }` nests into
  `if (!match) continue;` + flat body, split an `if/else if` pair into two
  independent `if`s where the first ends in `continue`, and duplicate a
  trivial tail write (e.g. `cutsceneActive = 0`) across early-return exits
  rather than nesting to share it. Flatter, and -- per the break-bytes entry
  -- the independent-`if`+`continue` form is what actually deletes the dead
  `break;` bytes.

Both are behavior-identical; prove it with the dual-object test plus
`--coverage` (100% on both objects, no new uncovered ranges).

## `asmsh -debug` embeds symbols in the .obj; `-debug=d` writes a side .DWF

`asmsh`'s `-debug` puts debug info **inside the .obj**; `-debug=d` writes it to a
separate `.DWF` and leaves the .obj without it. So `sh4objtest inspect
--format=json` only reports `debugSymbols` (asm emits type `Label`, `shc` emits
`Func`/`Var`) under plain `-debug`. Those debug symbols are the only place
**statics** appear -- `exports` lists just the public symbols. `Makefile.matching`
uses `-debug` for this reason; `scripts/sync_ghidra_symbols.py` depends on it.

Real symbols carry a leading `_`; internal auto-labels (`LAB_`, `LP_GEN_`, the
section symbol `P`) do not -- a clean filter.

## After a public rename in a decompiled `.src`, rebuild the matching object before `check_naming`

`check_naming.py` cross-checks a decompiled unit's `.c` object against its
**matching** object (`build/output_matching/.../<unit>.obj`), which is assembled
from the archived `src/asm/decompiled/<unit>.src`. A `./scripts/run_tests.sh -c`
run only refreshes `build/output_test/` objects, so after `sed`-renaming a symbol
in the `.src` the matching object stays stale and the check reports
`symbol at <addr> differs: .c has '<New>', .src has 'FUN_<addr>' -- rename both`
even though both source files already agree. Run `make -f Makefile.matching` to
refresh it, then re-check. Only **public** (exported) renames trip this; a
`STATIC` rename is invisible in the C object so the cross-check skips it.

## Compute-then-store: a conditional overwrite of a global emits two writes

The asm for a value that's conditionally refined usually computes it fully in a
register and stores the global **once** at the end (e.g. `var = base` vs
`var = base + (page-1)*stride`, then a single `MOV.L Rn,@global`). Translating that
as `global = a; if (c) global = b;` compiles to **two** stores to the global. The
matching build (which uses the asm) is unaffected, but the dual-object test compares
the memory-write trace, so the extra `global = a` write shows up as an
`Unexpected write value ... to _global` failure on the C object only (the `.src`
object passes). Fix: compute into a **local**, store the global once. Only globals
are trace-checked, so intermediate writes to the local are free.

## A Ghidra global that's really `base_symbol + offset` computed inline

Ghidra sometimes invents a standalone global (`var_exp_8c1ba25c`) for what the
asm actually computes as arithmetic on a different symbol's address
(`var_progress_8c1ba1cc + 0x90`, i.e. a struct field) rather than a direct
relocation. A dual-object test using `setSize()`/`addressOf()` on the invented
name gets its own independent fake address, so the `_src.obj` write lands at a
totally different (and seemingly nonsensical) address than expected --
`Unexpected write address 0x... Expecting ... to _invented_name(0x...)`. When
that happens, read the relevant asm block directly: if the base register comes
from `MOV.L @(disp,PC),Rn` loading a *different* imported symbol followed by
`ADD #offset,Rn`, the "global" is actually `otherSymbol + offset` -- express it
as a struct field access in C and in the test (`addressOf('_other') + offset`),
not a separate `setSize()`. Note this can be genuinely per-caller: another unit
that imports the invented name **directly** (`.IMPORT _var_exp_8c1ba25c`) is
using a real, separate linker symbol that only *coincides* with the struct
field's address because sectionB.src lays the two out back to back -- don't
"fix" that caller too without checking its own asm first.

## A same-object `const` used in a new test needs `.EXPORT` in the archived `.src` too

A `STATIC const` array already sitting in the C file (message box text, etc.)
compiles and links fine even when the matching label in
`src/asm/decompiled/<addr>.src` isn't `.EXPORT`ed -- nothing needs it *until*
a test's `addressOf('_const_...')` tries to resolve it against the `_src.obj`.
Without the export, `addressOf()` can't find the real symbol there and falls
back to an auto-allocated placeholder address, which happens to coincide
across unrelated tests (each fresh test VM allocates in the same order), so
every affected test fails identically with something like
`Unexpected argument for _swapMessageBoxFor_8c02aefc ... Expected 0x80034c,
got 0x...` -- the C object passes throughout, only `_src.obj` mismatches, and
every failing call reports the *same* "Expected" address regardless of which
constant it actually is. Fix: add the label under the unit's existing
`.AIFDEF UNIT_TESTING` / `.AENDI` `.EXPORT` block in the `.src` file (mechanical
`sed`/`Edit`, not a read of the asm body).

## `muls.w` (0x200f) was missing from sh4objtest's simulator until this session

Compiler-emitted `(short)x * 100` lowers to `MULS.W`, not `MUL.L`; sh4objtest's
`Simulator.php` only had `MUL.L` (`0x0007`) and threw `Unknown instruction`
for `0x200f` on both objects identically (a real tooling gap, not a C bug).
Fixed upstream in the sh4objtest repo (mirrors the `MUL.L` case, sign-extends
the 16-bit operands into `macl`) and the `tbg-decomp` Docker image rebuilt.

## Ghidra can label a raw address inside another struct as its own fresh global

`TileStreamInit_8c02175a` (`02171c`) reads a 5-pointer array Ghidra decompiled
as a standalone global `var_8c1bb8a4`. The address `0x8c1bb8a4` is actually
`var_currentCourse_8c1bb868 + 0x3c`, i.e. `slots_0x04[14]` inside the existing
`CurrentCourse` struct -- Ghidra has no notion of the struct field here, since
the instruction loads a plain absolute address from the literal pool, so it
just names that address like any other global. A dual-object test caught it:
`setSize`-ing a fake `var_8c1bb8a4` gave the C object a plausible mock address
that happened not to match the `_src.obj`'s real relocation target, so only
the `.src` side failed with an unrelated-looking garbage value. Fixed by
indexing into the real struct (`var_currentCourse_8c1bb868.slots_0x04[14 +
i]`) instead of declaring a new global. Worth checking whenever a Ghidra
global's address falls inside an already-known struct's range.

## Every call to an exported symbol is intercepted, even same-object ones

sh4objtest's `matchBranch` treats *any* branch whose target resolves to a
named symbol as a call that must be in the expectation queue -- it doesn't
distinguish "external, must be mocked" from "same-object, could just run for
real". So once a `STATIC` helper is exported under `.AIFDEF UNIT_TESTING`
(required to test it directly), every caller's test must also add a
`shouldCall()` entry for it, even calls you'd rather let execute for real.
`TileStreamLoad_8c021810`'s test mocks `lookupTile_8c0217de` outright instead
(already covered by its own test file), which is simpler than trying to let
it "pass through" -- there's no such option in the DSL.

For stack-local out-params (e.g. `njReadBinary`'s `&fpos`/`&rtype`) whose
address you can't predict, don't reach for `WildcardArgument` or hardcoded
per-object addresses without checking first: run the test with a guessed
placeholder, read the real value off the `Unexpected argument ... got 0x...`
error (or `-v --disasm`), and hardcode that. It's usually the same address
for both objects; branch on `$this->objectFile` (see
`tests/016d2c_course_menu/8c0170c6_FUN_pushDialogTask.php` for the pattern)
only if it isn't.
