# Next Decompilation Targets

Relocation-graph analysis regenerated 2026-08-29 by `scripts/generate-graph.sh`
after `02f0c8`, `02f320`, `02b464`, `01f3c0` and `01fa78` all landed. The
previous target list is fully exhausted; this is the next round.

"Fan-in" is how many already-decompiled (`.c`) units call into a given unit
(code edges only), per `build/inspect_graph/graph.json`. Ranked by fan-in,
then size. "Bytes" is the raw `src/asm/<unit>.src` file size.

Every remaining raw-asm unit now sits at fan-in <= 1 -- the graph's
high-fan-in nodes are exhausted, so size (and the leverage notes below) is
doing the tie-breaking now, not fan-in.

| Fan-in | Exports | Bytes | Unit | Called by |
|---|---|---|---|---|
| 1 | 7 | 56 K | `024b4c` | `02b464_drive_points` |
| 1 | 2 | 50 K | `025b98` | `026710_traffic` |
| 1 | 5 | 46 K | `02e51c` | `026710_traffic` |
| 1 | 5 | 41 K | `025870` | `012f44_game` |
| 1 | 5 | 39 K | `027958` | `028258_objects` |
| 1 | 5 | 34 K | `023938` | `02b464_drive_points` |
| 1 | 2 | 31 K | `021b9c` | `0222dc_fadecmd` |
| 1 | 2 | 27 K | `023310` | `012f44_game` |
| 1 | 2 | 25 K | `02d968` | `012f44_game` |
| 1 | 4 | 17 K | `02e2dc` | `02b464_drive_points` |
| 1 | 2 | 15 K | `02df3c` | `026710_traffic` |
| 1 | 1 | 14 K | `020214` | `020528` |
| 1 | 2 | 6 K | `02b2f0` | `02b464_drive_points` |
| 1 | 3 | 6 K | `02d06c` | `028258_objects` |

**Second wave (fan-in 0 from decompiled code today, but real game code
reachable once their asm callers are decompiled):** `022bdc` (32 K, called
only by still-asm `023310` -- see below), `024280` (38 K, called only by
still-asm `022bdc`), `02412c` (6 K, ditto), `020594` (6 K, ditto),
`02081c` (4 K, ditto), `02d19c` (36 K, called only by still-asm `02d968`).
These aren't priority-ranked yet because nothing decompiled calls them; they
surface into the table above as their callers land.

**Not real decompilation targets -- excluded from the table:**
- `04f6c0_SDK.src` (~44k lines) and `02fb50_sh4nlfzn.src` (~7.8k) -- SDK/library
  code, not game code.
- `02af78_pre_data.src` -- a parked Ghidra data extraction, not wireable as-is.
  See its file header and the "A raw Ghidra data extraction isn't wireable
  just because it exists" entry in `docs/lessons_learned.md`.
- `sectionB.src`, `sectionD.src`, `03327c_strt1_sectionC.src` -- data/section
  files, not units.
- `04ce10_slots.src` (75 K) -- also data-only (`hasCode: false` in the graph,
  no BSR/JSR in the file at all): three `init_*` tables consumed by the
  already-decompiled `013ae8_route_load`. A `move-data` job, not a
  `decompile-function` one.
- `010000.src` -- the IP.BIN-era boot header; not even in either Makefile's
  `SRCS`.

## Suggested order

Start with **`023310`** (27 K, fan-in 1 from `012f44_game`).

It looks unremarkable by size, but it's the on-ramp to the biggest cluster of
undecompiled gameplay code left in the project: `023310` calls
`task_bus_8c022bdc` (`022bdc`, 32 K, already partly named -- literally "the
bus task"), which is a per-frame dispatcher fanning out into **eight** further
units: `020594` (`move_bus_model_8c020594`, also already named), `02081c`,
`023938`, `024280`, `02412c`, `024b4c` (incl.
`gameplayRenderBusUpdateCamera_8c025078`), `025870`
(`DemoUpdateCamera_8c025906`), and `027958` (`FUN_8c028022`). None of
that fan-out is visible in the fan-in table above because `022bdc` currently
has zero decompiled callers -- decompiling `023310` is what turns the whole
cluster from invisible into a ranked, attackable second wave. This is the
main player-bus driving/physics/camera subsystem; it's worth more to unlock
now than its 27 K would suggest.

In parallel or right after, close out the two subsystems that landed tonight
-- both still call out to several raw-asm units each, and finishing those
calls turns `02b464_drive_points` and `026710_traffic` fully-C:

- `02b464_drive_points` still depends on four: `024b4c` -> `023938` ->
  `02e2dc` -> `02b2f0` (note `024b4c` is shared with the `022bdc` cluster
  above -- decompile it once, it satisfies both).
- `026710_traffic` still depends on three: `025b98` (the moving-vehicle /
  fixed-decoration task bodies `FUN_8c025b98`/`FUN_8c02656a` that
  `spawnEntry_8c0272b8` installs -- see the `026710_traffic` entry below) ->
  `02e51c` -> `02df3c`.

Lower priority, smaller and more isolated: `021b9c` (`0222dc_fadecmd`'s one
remaining callee), `02d968`/`020214` (chase these after `012f44_game`'s other
dependents), `02d06c` (`028258_objects`'s other callee).

## Two dead functions worth a look

- `FUN_8c01fe84` in `01fa78` -- **RESOLVED.** It was never a separate
  function: the `.src` `.EXPORT`ed a label sitting mid-body inside
  `FUN_8c01fbac` (pure fallthrough into a shared epilogue, zero BSR/JSR
  references anywhere in the tree). Decompiled as `01fa78.c`'s
  `FUN_8c01fe84`, called directly from `FUN_8c01fbac`, with a comment
  explaining the split is cosmetic (mirrors the asm object for testing, not a
  real call boundary). This is case (b) of the Ghidra function-boundary
  problem noted below.
- `FUN_8c02e35a` in `02e2dc` -- still open. Verified again this pass: still
  `.EXPORT`ed, still zero BSR/JSR references anywhere in `src/` or `tests/`.
  Float-heavy (`FMOV`, `FR12`-`FR15` pressure, trig-shaped), unreachable and
  unread. `02e2dc` is itself a decompile target now (see table above, called
  by `02b464_drive_points`); worth a fresh look once that unit is open in
  Ghidra.

## Ghidra function-boundary reliability

Four units in a row now have had wrong function boundaries out of Ghidra, in
two distinct ways:

1. **Merging separate functions into one range**, dismissing the leftover
   code as "unreachable blocks" (`026710_traffic`'s
   `applyTrafficLighting_8c02756a`/`trafficUpdateTask_8c0275d4`,
   `02c884_bus_stop`'s `drawStopMarker_8c02cd92`, and more found in
   `02b464_drive_points` -- six functions reachable only via TaskPush
   pointers, including the master per-frame `taskCallback_8c02c072`).
2. **A `.EXPORT`ed label mid-body that isn't a function at all**
   (`FUN_8c01fe84`, above -- pure fallthrough into a shared epilogue).

Trusting the export list or Ghidra's function list misses both. The reliable
method is walking the asm for prologue/epilogue pairs directly:

    command grep -aoP '\.DATA\.L\s+\K(LAB|_?FUN)_\w+' <unit>.src | sort -u

A bare `LAB_` hit is a candidate unexported function Ghidra folded into a
neighbour.

## Reference: recently completed units

### `02b464_drive_points` (ShortUnit `DrivePoints`) -- done, driving-evaluation/penalty subsystem

Ghidra had hidden six functions reachable only via TaskPush pointers,
including the master per-frame `taskCallback_8c02c072` -- see the boundary
note above. Still depends on four raw-asm units: `024b4c`, `023938`,
`02e2dc`, `02b2f0` (see the priority table and suggested order above).

### `02f320_replay_codec` (ShortUnit `ReplayCodec`) -- done

Confirmed **LZW with LRU eviction**, not lzhuf/adaptive-Huffman as first
hypothesised from the `Sint16`-counter shape. See
`docs/lessons_learned.md`'s `02f320_replay_codec` entries (EXTS.W and the
codec details).

### `01f3c0_ending` (ShortUnit `Ending`) -- done

Post-game staff roll, gated on `var_progress_8c1ba1cc.days_0x00 > 30`. Credit
strings were extracted to `strings_ja_jp.sjis.h`, which cost the unit's
section-data byte-match -- now in `check_data_match.sh`'s NOT_MATCHING
allowlist.

### `01fa78` -- done, left deliberately hex-only (no `@unit` tag)

Turned out broader than the "violation checker" working hypothesis: it's a
general in-drive HUD -- violation popup icons, next-stop icon, driver-points
meter, turn signals, rotating needle, speedometer, and two timers. See the
`FUN_8c01fe84` note above for its Ghidra-boundary quirk.

**Open item:** `FUN_8c01ff48`'s wiper/gear/lane-change/headlight/
traffic-signal/points-ramp branches are UNTESTED. Worth a follow-up pass with
targeted tests before the unit is considered fully verified.

### `02f0c8` -- done, left hex-only

Spawn-clearance path scanner + continuation, plus an unrelated marker-group
lookup.

### `02e400_collision` (ShortUnit `Collide`) -- done, 4/4 functions

Two independent collision services sharing one file. No section C/D data.

- `CollideFindTaskHit_8c02e400(self, entry)` -- box-vs-box between traffic
  entries. Transforms `entry`'s per-variant oriented box into world space with
  `njCalcPoints` (matrix at `entry+0x84`, variant index at `entry+0x2e0`), then
  scans the whole task array `var_tasks_8c1bac28` through the module-global
  cursor `var_collideScanCursor_8c228974`, skipping `self` and any `action ==
  -1` slot, and returns the first candidate entry whose box overlaps per
  `njCollisionCheckBB`. Callers are in the still-asm `025b98` (moving-vehicle
  task).
- `CollideQueueReset_8c02e486` / `CollideQueueAdd_8c02e48e(obj)` /
  `CollideQueueTest_8c02e4ac()` -- a 64-slot pointer queue
  (`var_collideQueue_8c228a38`, count `var_collideQueueCount_8c228b38`).
  `028258_objects` resets it each frame and enqueues every pedestrian; `02b464`
  then asks `CollideQueueTest` for the first queued entry whose sphere hits
  `var_collideSelfBox_8c228978` (`njCollisionCheckBS`) -- i.e. the box that
  `CollideFindTaskHit_8c02e400` last filled in.

`init_8c04c940` (in the still-asm `02e2dc`, now declared by the new minimal
`src/02e2dc.h`) is 16 pointers to 8-point local-space boxes, one per traffic
variant -- the same 16 variants as `026710_traffic`'s `init_8c04622c` /
`init_8c0460c8`.

**Original-game bug preserved and commented:** the self box is built from
`&init_8c04c940[idx]` -- the address of the table slot -- while a candidate's
uses `init_8c04c940[idx]`, the box it points at. Self's "box" is therefore the
pointer table's own bytes reinterpreted as floats. Confirmed against the `.src`
object, not corrected.

### `02786c_vehicle_parts` (ShortUnit `VehParts`) -- done, 1/1 function

`VehPartsBind_8c02786c(entry, typeCode)` caches a spawned traffic vehicle's
articulated sub-objects into fixed slots of the entry blob so the per-frame code
can pose them without re-walking the model tree, and clears
**`NJD_EVAL_UNIT_ANG`** (bit `0x02`, "ignore rotation") on the first-level ones
-- not `NJD_EVAL_HIDE`, which is bit 3.

It walks `entry+0x0c` (the model root `NJS_OBJECT`): the root's child and
sibling chain into `entry+0x18..0x24` (plus `+0x28` for typeCodes 0x0e, 0x10,
0x14, 0x16), then re-reads `entry+0x0c` and walks `child->child` and its
siblings into `+0x2c..0x3c`. Deeper slots are typeCode-gated: `+0x58/0x5c/0x60`
for 0x1a, `+0x40` for 0x1c/0x1e, `+0x48` and `+0x4c/0x50/0x54` for 0x1a, `+0x44`
for 0x14/0x16. Each further step is guarded by a NULL sibling check that bails
out of the rest of the function.

### `0206f0_intersect` (ShortUnit `Intersect`) -- done, 1/1 function

`IntersectSegments_8c0206f0(a0, a1, b0, b1, out)` intersects two 2D segments
given as `{x, y}` float pairs; `out` is the 5th argument and arrives on the
stack. It fits `y = m*x + b` to each segment, special-casing a vertical one
(zero x-delta) three ways -- both vertical rejects, a vertical uses `x = a0.x`,
b vertical uses `x = b0.x` -- and otherwise solves `x = (ba-bb)/(mb-ma)`,
rejecting parallel lines. The hit is accepted only if `x` lies within both
segments' x-extents, inclusive.

`028258_objects` uses it for the pedestrian crosswalk / bus stop-line tests
(does the bus's stop line cross this path segment).

**Original-game bug preserved and commented:** in the fully general branch the
compiled code evaluates `y` from whatever float already sits at `out[0]`, not
from the `x` it just solved for. Harmless in practice -- both callers read only
the return value.

### `020b6c_ground_probe` (ShortUnit `GroundProbe`) -- done, 4/4 functions

The rest of the ground-polygon query family; `020914_ground_query`'s sibling. Four
functions, all four `.EXPORT`ed and no address-taken extras (the `.DATA.L LAB_`
scan comes back empty). No section C/D/B data -- every literal lives in an
in-code pool.

Three of the four are polygon locators writing the same `GroundQueryResult`
(`020914_ground_query.h`); the fourth turns one into a height:

- `GroundProbeInterpolateHeight_8c020f7e(result, point)` -- the one every caller
  pairs with a lookup. Solves the matched polygon's plane equation for y:
  `point[1] = (-(nx*(point[0]-v0.x)) - nz*(point[2]-v0.z))/ny + v0.y`, using the
  polygon's precomputed normal (`GroundPoly.normalX_0x08`..`normalZ_0x10`, which
  were opaque ints until this unit needed them) and its first vertex. `ny == 0`
  (a vertical plane) falls back to `v0.y` outright. A miss (`count_0x0c == 0`)
  leaves `point` untouched, so callers must test that first.
- `GroundProbeTrackPolygon_8c020b6c(x, y, z, out)` -- per-frame form of
  `GroundQueryFindPolygon_8c020914`. `*out` arrives holding the previous match;
  it re-tests that polygon first and only falls back to the full cell search
  when the point has left it -- and the fallback **skips the just-rejected
  polygon** rather than retesting it. `y` is dead here, as in `020914`.
- `GroundProbeFindPolygonAtHeight_8c020fe4(x, y, z, out)` -- full lookup, but
  divides by the grid's own `cellSizeX_0x08`/`cellSizeZ_0x0c` instead of the
  fixed 150, and filters candidates by height: a polygon whose first vertex is
  more than `HEIGHT_TOLERANCE` (20.0f) off `y` is skipped before the containment
  test. `y` is live.
- `GroundProbeTrackPolygonAtHeight_8c021290(x, y, z, out)` -- the previous two
  combined: re-test, then a height-filtered fallback search. `026710_traffic`'s
  `spawnEntry_8c0272b8` installs this instead of `GroundProbeTrackPolygon_8c020b6c`
  when the entry's type code has bit `0x4000` set.

The height filter is how overlapping road layers are told apart -- an elevated
expressway above a surface street.

The two placeholder headers `020b6c.h` and `021290.h` are gone, folded into
`src/020b6c_ground_probe.h`.

Preserved as original behaviour, commented: the same missing lower clamp on the
cell index as `020914`, and the same `TWO_PI` spelled `6.283184f`.

### `020914_ground_query` (ShortUnit `GroundQuery`) -- done, 1/1 function

`GroundQueryFindPolygon_8c020914(x, y, z, out)` is the point-in-ground-polygon
lookup. It divides x and z by 150 to pick a cell in the grid selected by
`var_activeGroundGrid_8c2264d4`, then walks that cell's candidate polygon list.
Each polygon gets one of two containment tests chosen by the sign of its
attribute word: a cross-product edge walk for convex polys, or a winding-angle
sum in BAMS via `njSqrt`/`acosf` (inside when it exceeds half a turn) for
concave ones. On a hit it writes `{attr with sign bit stripped, matching slot in
the cell's id list, the poly's vertex-index array, vertex count}`; on a miss only
the count and vertex-ids are zeroed. **`y` is a dead argument** (FR5 never read),
as it is in its sibling `GroundProbeTrackPolygon_8c020b6c`.

All three callers pair it with `GroundProbeInterpolateHeight_8c020f7e` (in `020b6c_ground_probe`), which interpolates
a point's height from the returned polygon: `028258_objects` snaps decorations
and pedestrians to the ground, `026710_traffic` grounds a spawning vehicle on
its path, `02c884_bus_stop` grounds waiting-passenger sprites. A fourth,
still-undecompiled caller lives in `src/asm/023310.src`.

The real 4-field `GroundQueryResult` now lives in `020914_ground_query.h`; the
opaque local duplicates in `028258_objects.c` and `02c884_bus_stop.c` are gone.
The struct is 16 bytes, but `026710_traffic.c` keeps a 20-byte `groundBuf` with
a cast -- retyping it shifts the C frame and breaks 5 tests that assert fixed
asm addresses.

Preserved as original behaviour, commented: no lower clamp on the cell index
(negative coordinates index before the array), and the 2*pi literal
`6.283184f`, 3 ULP short of the true value but the exact literal encoding to the
asm's `H'40C90FD8`.

### `026710_traffic` (ShortUnit `Traffic`) -- done, 15/15 functions

The `.src` exported 14 symbols and Ghidra showed 13 functions; the unit has 15.
`applyTrafficLighting_8c02756a` and `trafficUpdateTask_8c0275d4` are never
exported and only address-taken, so they had been merged into
`spawnEntry_8c0272b8`. Find that class of function with:

    command grep -aoP '\.DATA\.L\s+\K(LAB|_?FUN)_\w+' <unit>.src | sort -u

Anything that comes back as a bare `LAB_` is an unexported function.

**`*_mac_cpu1.dat`** (`slots_0x04[8]`, via `CurrentCourse.macCpu1_0x24`) is the
CPU-vehicle placement table, relocated in place at load by `TrafficRelocatePlacementTable_8c026da4` -- a
two-level fixup turning self-relative dwords into absolute pointers over
0xc-byte records. `TrafficInit_8c02769e` caches the base;
`trafficUpdateTask_8c0275d4` walks it as `{typeCode, threshold, script, progress}`.

**Spawning** is threshold-driven: each frame the task selects the CPU-vehicle
collision meshes as the active ground grid and advances a per-record counter;
crossing `threshold` calls `spawnEntry_8c0272b8`, which gates some type codes
behind a per-(route, time-of-day) day bitmask (`init_8c046208`) and allocates a
task running `FUN_8c025b98` (moving vehicle) or `FUN_8c02656a` (fixed
decoration).

**Stepping** is a bytecode VM: `TrafficRunEntryScript_8c027012` dispatches
opcodes 0 spawn, 1 advance path block, 2/3 config, 4 skip, 5/6/7 decoration
slots, 8 id-resolved config, 9 end, 10 place decoration. Motion walks blocks of
`{len, x, y, dx, dy}` floats terminated by `len == 0` --
`TrafficAdvanceOnPath_8c026ca2` finds the containing record and re-projects
world position, `TrafficUpdateHeading_8c026bc4` derives the heading basis and
four body corners, `TrafficComputeBlockedSpeed_8c026eaa` scans other entries
(and the player's bus via the `var_8c1bbd9c` sentinel) for a speed limit,
defaulting to `9999.0` when clear.

**Model selection:** `init_8c04622c[16]` maps a variant index to one of the 11
`3s_` body codes in route_load's `init_trafficModelFiles_8c043d64` (2do, 4wd,
sed, tax, tor, kto, dan, wag, bus, pat, kyu). The 16 variants line up 1:1 with
route_load's `init_8c043dc4` (day) and `init_8c043ecc` (night, `_sn` textures),
chosen by `var_timeOfDay_8c18ad20 == 2`. Each variant has a 4-float dimension
record in `init_8c0460c8` (bus/truck 6.1 x 2.03 x 1.5 x 8.9 vs a car's
2.42 x 1.67 x 0.65 x 3.05) and one float in `init_8c0461c8`.

Two original-game quirks preserved and commented: an unloaded model slot frees
its just-allocated task but still returns success, and opcode 10 leaves the
script cursor parked mid-instruction.

### `02c884_bus_stop` (ShortUnit `BusStop`) -- done, 10/10 functions

Bus stops: which route segments have one, the passengers waiting at them, and
the arrival/departure state machine. The `.src` exported 9 symbols but the unit
has 10 functions -- `drawStopMarker_8c02cd92` is never exported and never
directly called, only address-taken by `FadeCmdPushCall1_8c0223ea`, so Ghidra
had merged it into its neighbour. Worth expecting more of these.

What `026710` needs from it:

- `BusStopGetSegment_8c02cd6a(i)` -> `CourseSegment *` = `&courseConfig->segments_0x08[i]`
  (0x2c-byte records). This is the call `026710` makes -- it gets the segment
  record and reads its fields directly.
- `BusStopGetStopArea_8c02cd7a(i)` -> `StopAreaRecord *` =
  `var_stopAreaTable_8c1bb870[seg->stopAreaId_0x02]` (8-byte-stride pointer
  table). Record is `{ukn_0x00, x_0x04, z_0x08, dx_0x0c, dz_0x10}` -- origin and
  direction of the stop's passenger spawn strip.
- `var_8c2286a4` is `int[24]`, one word per course segment, `1` = "this segment
  has an active stop this run". Filled in three passes: one flag per candidate
  story event (`EventScanCandidates_8c02b03c`, each event's `segmentId_0x02`),
  then every segment with `type_0x00 == 2` (forced stop), then random fill until
  the count reaches a per-run draw from
  `courseConfig->[randomStopCountMin_0x14, randomStopCountMax_0x18)`, never
  picking `type_0x00 == 3`. Stop layout is re-rolled every run, with story and
  mandatory stops pinned.
- `var_startStopIndex_8c228704` (was `var_8c228704`) seeds
  `var_nextStopSegment_8c228710`; `var_prevStopSegment_8c22870c` trails one
  behind. It is the debug menu's "start partway along the route" index --
  `init_debugMenuEntries_8c04429c` passes `{courseId, startStopIndex, inputMapSel}`
  and the normal entry point `GamePushLoadingTask_8c013310` hardcodes it to 0.

`BusStopSetup_8c02caba` allocates nothing; it is pure state init. The unit's
only allocation is `pickWaitingPassengers_8c02c8ae`'s `syMalloc(0x40)` scratch
list, freed before return. Waiting passengers land in `var_8c228798[16]` of
`WaitingPassengerSlot` (0x14 bytes: spot pointer, `NJS_POINT3` pos, pick-order
float), laid along the stop strip as `pos = origin + n*dir` plus `rand()` jitter
-- except on `ROUTE_OME`, which gets no jitter.

**Original-game bug preserved deliberately:** in `BusStopSetup`, the
segment-scan loop leaves its walking pointer on the table terminator, and the
extra-stop reroll loop then indexes `segments[pick]` off that unreset pointer --
so its `type_0x00` check reads `totalSegments + pick` records past the table
start. The flag-array check is unaffected. Commented in the C as real asm
behavior rather than silently corrected.

## Done since last snapshot (2026-07-12/13)

- `01d7fc_results` -- post-run results + VMU save.
- `01c980_profile_file`, `01e27c_practice_menu`, `01bb48_vm_game`
  (`018644_file_menu` earlier), `01a148_option`.
- `02af78_event` -- story event selection. See `docs/gameplay.md`.
- `01614c_debug_menu` -- VMU save / demo-recording related. Its
  `init_debugMenuEntries_8c04429c` is a level-select warp table of
  `{courseId, startStopIndex, inputMapSel}`; `_AUTO` rows drive the game from
  recorded input. `startReplaySave_8c016924` is unreachable -- the replay
  recorder runs and playback works, but the save-to-VMU path was cut.
