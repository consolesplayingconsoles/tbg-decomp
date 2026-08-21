# Next Decompilation Targets

Snapshot from a relocation-graph analysis (2026-07-12, right after finishing
`013ae8_route_load.c`); entry #1 refreshed 2026-07-13 with a full Ghidra read of
`02171c`. Method: `sh4objtest inspect --format=json` on every built object
(`build/output/src/*.obj` + `build/output/src/asm/*.obj`), map exports to owning
units, then external relocations give unit-to-unit edges. Re-run the analysis
when this list goes stale; it only takes a few minutes.

Ranking criteria used: continuation of the route-load pipeline, plus fan-in
from already-decompiled units.

## 0. `01c980` / `01d7fc` / `01e27c` -- profile file, results, course confirm (2026-08-21) -- CURRENT, next

Picked ahead of the ranking below because a player screenshot pass nailed
down what each of these three consecutive units is, from a shallow Ghidra
read (light -- verify per-function during actual decompilation). Not part of
the route-load pipeline; these are course-menu-adjacent screens.

### `01c980_profile_file` (5.5 KB, 6 functions) -- PROFILE FILE menu

The event/character-profile screen referenced in `gameplay.md`'s Story mode
notes ("PROFILE FILE" notebook UI). State lives behind `DAT_8c1bc7c0`
(screen-state enum, same pattern as the other two units below).

- `FUN_8c01c980` -- scans `init_8c044ffc[55]` (one entry per grid slot) and
  marks a slot unlocked in `var_8c2263b4[55]` if *any* flag in that slot's
  list tests true via `EventHasProgressFlagAlt_8c02aff0`.
- `FUN_8c01d1c4` -- task init (`TaskSetAction` -> `FUN_8c01ccec`), called from
  `SystemMenuWriteToVmu_8c01b26c`.
- `FUN_8c01ccec` -- per-frame task body: drives a 2D cursor
  (`DAT_8c1bc7e4`=col 0-9, `DAT_8c1bc7e8`=row 0-5) over the 55-slot grid
  (`CourseMenuInterpolateCursor_8c016d2c` for the slide animation), swaps in a
  resource group per selected row (`init_8c045148[row]`) -- the detail page's
  portrait+bio+checklist -- and on confirm returns to
  `courseMenuStoryMenuTask_8c017718`/`courseMenuFreeRunMenuTask_8c017ada`.
- `FUN_8c01c9f2` / `FUN_8c01cac8` -- draw helpers: the former paints the
  55-slot unlock grid, the latter paints the selected slot's episode
  checkbox row, walking the *same* `init_8c044ffc[row*10+col]` flag list and
  drawing a checkmark (sprite widget 9) per flag that's currently set.

**Confirmed link to `02af78_event`:** read `init_8c044ffc`'s first pointer's
data directly out of Ghidra memory -- it's `{50, 51, 124, 52, 125, 53, 126,
59, 0xff}` (flag ids `0x32, 0x33, 0x7c, 0x34, 0x7d, 0x35, 0x7e, 0x3b`). Those
are exactly the first two `ACTION_SET_PROGRESS` flags in
`init_wanganEvents_8c04abb0` (`02af78_event.c`): `0x32` and `0x33`. So each of
the 55 grid slots is a character, and its flag list is the set of
`EventEntry.actions_0x0c` progress flags (`ACTION_SET_PROGRESS`) that
character's episodes set -- i.e. `init_8c044ffc` should let us recover the
character roster/order and which episodes belong to which character, for
free, by cross-referencing flag ids already named in `02af78_event.c`.
Resolves that file's "revisit when dialog strings found" note.

### `01d7fc` (2.9 KB, 6 functions) -- post-run results + VMU save

Screenshot-confirmed: score reveal digit-counter, then a save-to-VMU YES/NO
prompt.

- `FUN_8c01e0b4` -- tallies the run's score on entry: time bonus
  (`var_8c2263ec`), fare (`var_8c2263f0`), exp*10 (`var_8c2263f4`), an
  "award" tier 0-3 from distance thresholds (0x46/0x50/0x5a) worth
  0/50/100/200 points (`var_8c2263f8`), plus two more per-stat bonuses
  (`var_8c2263fc`/`var_226400`); sums into `var_8c226404`, added to
  `var_exp_8c1ba25c` (capped 99999). Falls through to `FUN_8c01df8e`.
- `FUN_8c01e24e` -- same entry but skips the tally (sets a "no tally" flag
  first) -- the failed-run path. Also falls through to `FUN_8c01df8e`.
- `FUN_8c01df8e` -- task init: pushes `task_8c01d8e0` under `GameTask_8c012f44`.
- `task_8c01d8e0` -- the results screen's state machine (14 states): fade in,
  odometer-style digit-reveal of the score (`FUN_8c01d7fc` draws the scrolling
  digit sprites), award-star icons revealed one at a time
  (`DAT_8c1bc80c` 0..7), a save-to-VMU yes/no prompt
  (`PromptHandleBinary_8c016caa`), the actual write
  (`SystemMenuWriteToVmu_8c01b26c` / `BupGetInfo_8c014bba` / `buStat`) with
  success/failure message boxes, then routes onward to title, a story dialog
  sequence, or the course/free-run menu depending on progress.
- `FUN_8c01d864` -- draws the message-box text overlay for the results screen.

### `01e27c` (8.9 KB, 9 functions) -- course-select confirmation card

Screenshot-confirmed: the course info card (route icon, distance, stop
count, passenger/traffic/difficulty star ratings, description text) ending
in a "Is this course OK?" YES/NO prompt.

- `FUN_8c01e576` / `FUN_8c01e27c` -- sets up and runs the description-text
  reveal: `DAT_8c1bc804` walks a page range sized from
  `init_8c0451b4/b5[var_8c22640c]` (per-course text length, `var_8c22640c`
  = selected course id), ending in `PromptHandleBinary_8c016caa` for the
  YES/NO.
- `FUN_8c01e920` / `FUN_8c01e63c` -- the star-rating reveal: `DAT_8c1bc7e8`
  counts up to `init_8c0451ec[var_8c22640c]` (per-course star count),
  animating the passenger/traffic/difficulty star fill-in;
  `CourseMenuDrawDateAndExp_8c016ee6` draws the surrounding date/exp text.
- `FUN_8c01f114` / `FUN_8c01f21c` -- confirm-YES entry points: sets
  `var_playMode_8c1bb8d0=1`, builds the dialog queue (`FUN_8c01e992`), pushes
  `GameTask_8c012f44`, requests the course's resource group -- this is where
  the drive actually starts loading.
- `FUN_8c01ead8` -- draw helper (unclear yet, likely another digit-row like
  `FUN_8c01d7fc`/`FUN_8c01d864`).

## 1. `02171c` -- map-tile streaming engine (1.1 KB, 8 functions) -- READY

Most direct continuation of route_load, and fully understood via Ghidra
(2026-07-13). Unblocked: every import already resolves -- `AsqRequestTexlist_8c01181c`
lives in the decompiled `011120_asset_queues`, the rest are SDK (`njReadBinary`/
`njReleaseTexture`/`njSetTexture`/`njCnkSimpleDrawObject`/`njControl3D`/`njFog*`/
`njTranslate`, `syMalloc`/`syFree`).

The unit owns a grid of streamed map tiles. State lives in one static block:
- `var_8c22650c[5]` -- layer descriptors, each a pointer to `{width, height, ...}`,
  copied from the route's `var_8c1bb8a4` at init.
- `var_8c226520[5]` -- per-layer tile buffers, each `malloc(width*height*8)`: a grid
  of `{NJS_TEXLIST *texlist, void *cnkModel}` slots (8 B each). `var_8c226520 == -1`
  is the "not allocated" sentinel.
- `var_currentTileRegionList_8c226534` -- the region-rect list (see gain below).

The 8 functions:
- `clearUnknownVar_8c02171c` -- one-time reset (buffers sentinel = -1), called from `njUserInit`.
- `FUN_8c02175a` -- init: copy layer dims from the route, `malloc`+zero the 5 tile grids.
- `FUN_8c0217de` -- tile lookup in a datFile: `datFile[col + row*width + 2]` offset table, returns the tile blob pointer or 0.
- `FUN_8c021810` -- load: for each of the 4 texel layers, walk the region rects, look up each tile in its `var_datFiles_8c18adb4[i]` and `njReadBinary` the texlist+model pair into the grid; frees the datFiles after.
- `FUN_8c02190a` -- for the region, `AsqRequestTexlist(var_pvrDir_8c18ad4c, ...)` every loaded tile (async VRAM upload).
- `FUN_8c021a24` -- release all: `njReleaseTexture`+`syFree` every loaded slot across the full grid.
- `FUN_8c021724` -- teardown: `021a24` then free the 5 grid buffers, set sentinel = -1.
- `FUN_8c021b34` -- draw one tile: `njControl3D`/fog off, `njTranslate` to the bus
  position (`var_busState_8c1bb9d0`), `njSetTexture(slot.texlist)`,
  `njCnkSimpleDrawObject(slot.model)`.

**Gain:** resolves `CourseSegment.tileRegionList_0x0c` (currently `void *`,
"unconfirmed"): it points to an array of 4-byte rects `{startCol, startRow,
endCol, endRow}` (bytes), terminated by a rect whose `endCol==0 && endRow==0`.
Also nails down the `var_datFiles_8c18adb4[4]` layout (header `{width, height}`
+ a per-tile int offset table at word 2) and confirms the 4 datFiles map 1:1 to
the 4 renderable tile layers. Note the 5th layer (`var_8c226530`, model-only, no
texlist) is allocated here but populated by another unit -- a loose thread, not a
blocker.

## 2. `026710` -- traffic vehicle spawner (4.4 KB, 13 functions)

Consumes `slots_0x04[8]` = `*_mac_cpu1.dat` (CPU-vehicle placement). Uses
trig (`njSin`/`njCos`/`acosf`), the Asq RNG, pushes tasks. Calls into
`02c884` (`FUN_8c02cd6a`); route_load calls `02c884`'s `FUN_8c02caba` in the
same post-load block, so **`02c884` (2 KB, 9 functions) is a natural
follow-up pairing**.

**Gain:** names the `_cpu` asset category consumers end-to-end; connects to
the vehicle model code tables (`3s_`/`3t_`, sed/tax/tor...) in route_load's
data.

## 3. `022464` -- screen fade tasks (1.9 KB, 8 functions, half-named already)

Not load-related but best leverage-per-byte: 6 decompiled units
(`016d2c_course_menu`, `019e98_main_menu`, `0193c8_vm_menu`, `015ab8_title`,
`01d290_album`, `012f44_game`) extern `push_fadein_8c022a9c` /
`push_fadeout_8c022b60`; 16 units reference it overall. Exports already
meaningfully named (`task_fadein`, `draw`, ...).

**Gain:** mostly mechanical quick win; closes a hole every menu depends on.

## 4. `028258` -- pedestrians + scene objects + message box (11.5 KB, 41 functions)

Highest-fan-in code unit remaining (19 referencing units, 6 decompiled). Owns
route_load's remaining mysteries: the pedestrian chain (slots 11/12 =
`*_mac_hum_g0/m0.dat`, `task_pedgroup_8c029078`, `task_pedestrians_8c0293f6`),
the typed scene-object streamer `FUN_8c029ad4` (type 6 = O_FUMI railroad
crossing), and the menu message-box API (`swapMessageBoxFor_8c02aefc`,
`menuTextboxText_8c02af1c`) used by course_menu.

**Gain:** biggest payoff by far, but 10x the size of the others -- better
after one or two small ones, or carved into the pedestrian half first.

## Honorable mention

- `0222dc` (392 B, 4 functions): referenced by 12 asm units, trivial to knock
  out, but only `012f44_game` among decompiled units uses it.

## Suggested order

`02171c` -> `022464` (palate cleanser) -> `026710`+`02c884` -> `028258`.

## Done since this snapshot

- `02af78_event` -- story event selection. See `docs/gameplay.md`.
- `01614c_debug_menu` -- VMU save / demo-recording related.
- `01bb48` -> `01bb48_vm_game.c` -- VMU LCD (`VmGameSetLcdSlot_8c01c8fc/8c01c910`).
