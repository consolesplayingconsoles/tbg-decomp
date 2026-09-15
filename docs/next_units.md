# Next Decompilation Targets

> **All game-code units are decompiled** (2026-08-29). `src/asm/` now holds only
> the excluded set: `04f6c0_SDK` and `02fb50_sh4nlfzn` (library code),
> `sectionB`/`sectionD`/`03327c_strt1_sectionC` (section data), `04ce10_slots`
> (pure data -- a `move-data` job, not a decompile target), `010000`, and the
> parked `02af78_pre_data` (see its file header and `docs/lessons_learned.md`).
>
> The priority table below is kept for its per-unit notes, which remain
> accurate; every unit in it has since landed. Remaining work is no longer
> "which unit next" but the open items in each unit's own notes -- untested
> branches, `FUN_` placeholders whose role is still unclear, and the
> `TrafficEntry.groundProbe_0x190[3]` question (checked across eight units, no
> reader found anywhere).

Relocation-graph analysis regenerated 2026-08-29 by `make graph` after
`022bdc_bus`, `023310_bus_init`, `024b4c_bus_render`, `025870_demo`, `02e51c_attr_query`,
`023938_bus_drive`, `027958_bus_draw` and `025b98_traffic_drive` all landed. The
previous target list is fully exhausted; this is the next round.

"Fan-in" is how many already-decompiled (`.c`) units call into a given unit
(code edges only), per `build/inspect_graph/graph.json`. Ranked by fan-in,
then size. "Bytes" is the raw `src/asm/<unit>.src` file size.

Landing the bus/traffic cluster pulled several units that were fan-in 1 (or
invisible "second wave") last round up to fan-in 2-3, since their callers
are now decompiled themselves. The table is genuinely fan-in-ranked again,
not just size order.

| Fan-in | Exports | Bytes | Unit | Called by |
|---|---|---|---|---|
| 3 | 1 | 6 K | `020594` | `022bdc_bus`, `023310_bus_init`, `027958_bus_draw` |
| 3 | 2 | 4 K | `02081c` | `022bdc_bus`, `023938_bus_drive`, `025b98_traffic_drive` |
| 2 | 2 | 15 K | `02df3c` | `025b98_traffic_drive`, `026710_traffic` |
| 1 | 6 | 38 K | `024280_bus_input` | `022bdc_bus` |
| 1 | 2 | 31 K | `021b9c` | `0222dc_fadecmd` |
| 1 | 2 | 25 K | `02d968` | `012f44_game` |
| 1 | 4 | 17 K | `02e2dc` | `02b464_drive_points` |
| 1 | 1 | 14 K | `020214` | `020528` |
| 1 | 2 | 6 K | `02b2f0` | `02b464_drive_points` |
| 1 | 1 | 6 K | `02412c` | `022bdc_bus` |
| 1 | 3 | 6 K | `02d06c` | `028258_objects` |

**Second wave (fan-in 0 from decompiled code today, but real game code
reachable once its asm caller is decompiled):** `02d19c` (36 K, 9 exports,
called only by still-asm `02d968`). Not priority-ranked yet because nothing
decompiled calls it directly -- but decompiling `02d968` doesn't just surface
`02d19c` itself: `02d19c` in turn calls into `02d06c`, which is *already* a
target above (called today by `028258_objects`). So finishing `02d968` ->
`02d19c` adds a second caller to `02d06c` as a side effect. Also note
`02e2dc` (already a table target) calls into `02081c` -- decompiling `02e2dc`
will bump `02081c`'s fan-in from 3 to 4.

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

The `023310` bus cluster (previous round's pick) is now fully decompiled --
`022bdc_bus`, `023938_bus_drive`, `024b4c_bus_render`, `025870_demo`, `02e51c_attr_query`,
`027958_bus_draw` and `025b98_traffic_drive` all landed. That's exactly why the two
tiny units at the top of the table today (`020594`, `02081c`) sit at fan-in
3: they're leftover callees shared across that whole now-decompiled cluster.

Start with **`020594`** (6 K, fan-in 3) and **`02081c`** (4 K, fan-in 3).
Both are small, already partly named or purely `FUN_`-labelled, and each
closes out one remaining dependency for three already-finished units at
once (`Bus`, `BusInit`/`BusDrive`/`TrafficDrive` respectively). Cheapest
leverage left in the graph.

Next, **`02df3c`** (15 K, fan-in 2) is the last raw-asm callee of *both*
`025b98_traffic_drive` and `026710_traffic` -- decompiling it turns the
entire traffic subsystem fully-C.

After that, **`02d968`** (25 K, fan-in 1 from `012f44_game`) is worth
promoting above its raw fan-in suggests: it's a smaller echo of last round's
`023310` pattern. `02d968` calls into `02d19c` (36 K, 9 exports), which is
currently invisible (fan-in 0, nothing decompiled calls it directly) --
finishing `02d968` immediately makes `02d19c` a ranked target, and `02d19c`
itself calls into `02d06c` (already in the table above, called by
`028258_objects`), so it also bumps `02d06c`'s fan-in from 1 to 2. No other
remaining unit hides a cluster this way -- the rest of the graph (`024280_bus_input`,
`021b9c`, `02e2dc`, `020214`, `02b2f0`, `02412c`, `02d06c`) is flat: one
caller each, no further asm-to-asm fan-out (`02e2dc` -> `02081c` is the only
other asm-to-asm code edge left, and `02081c` is already covered above).

Lower priority, smaller and more isolated once the above land: `024280_bus_input`
(38 K, `022bdc_bus`'s other callee), `021b9c` (`0222dc_fadecmd`'s one
remaining callee), `020214` (`020528`'s callee), `02b2f0`/`02412c`/`02d06c`
(the small single-caller leftovers).

## Open struct-field naming questions

Carried over from `docs/struct_catalog.yaml` when that file was retired (see
below). These are the fields that had enough evidence to name but were never
named; everything else the catalog tracked either lives in the headers now or
was already there. Field-by-field negative results ("written here, read
nowhere") went into the owning header next to the field.

**`BusState.field_0x25c`** -- `024280_bus_input.c:277-280`: the two rear/side-mirror
buttons (`PDS_PERIPHERAL.press` bits 0x400/0x2) drive a small per-button
press/hold/release state here, which toggles `mirror_0x268` between its 0/1/2
modes. Nameable as-is.

**`BusState.field_0x334`** -- `023938_bus_drive.c:249-253,317`: gates
`FUN_8c023e7e`. While set, that function only waits for `laneOffset_0x2c4` to
settle back to 2.0, then clears it and re-arms a new search; it is set to 1
once a new crossing point lands in `laneTargetX_0x0ec`/`laneTargetZ_0x0f0`.
Nameable as-is.

**`BusState.field_0x338`** -- `023938_bus_drive.c:258-306`: 0/1 select
forward/backward search direction through the node table's
`fwdNext_0x00`/`backNext_0x02`, 2 marks the search blocked. Also driven by the
mirror-view input handler (`024280_bus_input.c:281-289`) in mapped-route steering mode.
Nameable as-is.

**`TrafficEntry.field_0x04c`..`0x060`** -- `02786c_vehicle_parts.c:71-100`
binds this run of 6 child-model nodes off the body root, but only for typeCode
0x1a, and no reader exists anywhere. `BusState.bodyModels_0x04c[6]` is
byte-identical and likewise never explicitly read, so both are probably
model-tree caching whose consumer hasn't been found. Name it after
`bodyModels_0x04c` or leave it until a reader turns up.

**`MenuState.field_0x3c` / `field_0x40`** -- the one genuinely contested pair,
and the reason to keep this list. They read as a grid column/row in
`016d2c_course_menu.c` (`selected = field_0x3c + field_0x40 * 5`, plus the
up/down/left/right nav) and in `01c980_profile_file.c`'s unlock grid. But in
every other screen `field_0x3c` is generic `PromptHandleBinary_8c016caa`
output or a plain page index with no row partner (`018644_file_menu.c`,
`0193c8_vm_menu.c`, `01b19c_system_menu.c`, `01bb48_vm_game.c`,
`01e27c_practice_menu.c`), and `field_0x40` is a standalone counter in
`019e98_main_menu.c` / `01b19c_system_menu.c` and a lesson-list scroll offset
in `01e27c_practice_menu.c`. The grid reading fits 2 of 6+ writers. A union,
or two names, or neither -- don't name these without settling that first.

### Why the struct catalog was retired

`docs/struct_catalog.yaml` tracked 910 struct members across 111 structs in
6,176 lines. 783 were already named, which made those rows a second copy of
the headers; 77 of the 111 structs carried no member rows at all, just a
`purpose` restating the struct's own docblock. It had no generator despite a
`generated:` header, nothing validated it, and 35 of its 111 `line:` fields
pointed at the wrong line. Fifteen of its open rows recommended merging
`BusState`'s 0x088-0x0c0 floats into one `NJS_MATRIX` -- work `sectionB.h` had
already done. The negative results were the part worth keeping, and they are
now comments on the fields they describe.

## Two dead functions worth a look

- `FUN_8c01fe84` in `01fa78` -- **RESOLVED.** It was never a separate
  function: the `.src` `.EXPORT`ed a label sitting mid-body inside
  `FUN_8c01fbac` (pure fallthrough into a shared epilogue, zero BSR/JSR
  references anywhere in the tree). Decompiled as `01fa78.c`'s
  `FUN_8c01fe84`, called directly from `FUN_8c01fbac`, with a comment
  explaining the split is cosmetic (mirrors the asm object for testing, not a
  real call boundary). This is case (b) of the Ghidra function-boundary
  problem noted below.
- `FUN_8c02e35a` in `02e2dc` -- **RESOLVED, unlike `FUN_8c01fe84`.** Direct
  inspection of `src/asm/02e2dc.src` shows it is a real, separate function:
  it starts right after `FUN_8c02e2dc`'s `RTS`/delay-slot epilogue (line 81)
  with its own full prologue (`MOV.L R14,@-R15` ... `STS.L PR,@-R15`,
  line 82 on) and its own matching epilogue -- not a fallthrough into a
  shared tail, so this is not case (b) below. It genuinely has zero
  references anywhere in `src/` -- no `BSR`/`JSR`, and no `.DATA.L
  _FUN_8c02e35a` literal-pool pointer either (ruling out case (c), a
  pointer-only caller). It is real, complete, compiled code that the
  shipped binary never calls: confirmed dead code, not a Ghidra artifact.
  Float-heavy (`FMOV`, `FR12`-`FR15` pressure, trig-shaped). When `02e2dc`
  is decompiled, keep it for object parity as `findNextHit_8c02e35a` (`STATIC`,
  `.EXPORT` gated under `.AIFDEF UNIT_TESTING`), per the `unused_8c0104bc`/
  `unused_8c020676` precedent.

## Ghidra function-boundary reliability

Several units in a row now have had wrong function boundaries -- or an
incomplete call graph -- out of Ghidra, in three distinct ways:

1. **Merging separate functions into one range**, dismissing the leftover
   code as "unreachable blocks" (`026710_traffic`'s
   `applyTrafficLighting_8c02756a`/`trafficUpdateTask_8c0275d4`,
   `02c884_bus_stop`'s `drawStopMarker_8c02cd92`, and more found in
   `02b464_drive_points` -- six functions reachable only via TaskPush
   pointers, including the master per-frame `taskCallback_8c02c072`).
2. **A `.EXPORT`ed label mid-body that isn't a function at all**
   (`FUN_8c01fe84`, above -- pure fallthrough into a shared epilogue).
3. **A real, separately `.EXPORT`ed function with no visible caller at all**
   in Ghidra's decompilation, because the only reference is a raw pointer
   sitting in another function's literal pool (`.DATA.L`) rather than any
   `BSR`/`JSR` -- `024b4c_bus_render`'s `drawFrontBusModel_8c024cc8` and
   `027958_bus_draw`'s `drawAhead_8c027a88`/`drawMirror_8c027bac`. Unlike
   (1) these aren't merged into a neighbour -- Ghidra does list them as
   functions -- but nothing in the pseudocode calls them, so a straight
   BSR/JSR-based call-graph reading skips them entirely.

Trusting the export list, Ghidra's function list, or a BSR/JSR-only call
graph misses at least one of these three. The reliable method for (1)/(2)
is walking the asm for prologue/epilogue pairs directly:

    command grep -aoP '\.DATA\.L\s+\K(LAB|_?FUN)_\w+' <unit>.src | sort -u

A bare `LAB_` hit is a candidate unexported function Ghidra folded into a
neighbour.

## Reference: recently completed units

### `025b98_traffic_drive` (ShortUnit `TrafficDrive`) -- done, 2/2 functions

The two per-entity `TaskAction`s `spawnEntry_8c0272b8` (`026710_traffic`)
installs: `TrafficDriveVehicle_8c025b98` for a moving CPU vehicle,
`TrafficDriveDecoration_8c02656a` for a fixed one (traffic light, sign, ...).

`TrafficDriveVehicle_8c025b98` dispatches on `driveState_0x2b4`, a 4-state
field (0/2 normal driving, 1 collision-push, 3 waiting for its own spawn box
to clear) -- named (renamed from a too-narrow `knockbackActive_0x2b4`) once
it turned out to hold more than a boolean push flag. `TrafficDriveDecoration`
shares the same collision-push shape via `driveState_0x2b4 == 1`.

`TrafficEntry.groundProbe_0x190` is declared `[4]` (`026710_traffic.h`), but
both this unit and `027958_bus_draw` only ever read/write indices `[0..2]` -- **no
reader for `groundProbe_0x190[3]` has been found anywhere in the tree.**
Genuinely open; worth a fresh grep once more of the traffic cluster is
decompiled.

### `027958_bus_draw` (ShortUnit `BusDraw`) -- done, 7/7 functions

The re-check `02e51c_attr_query` (below) called for. The unit had been left
hex-only as three unrelated jobs sharing a Ghidra range; reading the bodies
against their callers dissolved that the same way it did for `02e51c`. Every
function is one step of *drawing a vehicle or a signal*, and all of them run
inside the fade command list:

- `BusDrawUpdateModels_8c027958` pushes a frame of animation onto a vehicle's
  model nodes. Its switch on `typeCode_0x000` has exactly the arms
  `VehPartsBind_8c02786c` (`02786c_vehicle_parts`) has -- it drives only the
  nodes that binder cached for this vehicle type, which is what makes the two
  functions readable as a pair rather than as a mystery phase machine.
- `drawAhead_8c027a88`/`drawMirror_8c027bac` are the two draw callbacks,
  `BusDrawPlaceEntity_8c027c3c` picks between them and also owns the entity's
  lean and ground alignment.
- `BusDrawSignal_8c0281ac`/`BusDrawSignalAttachment_8c028206` draw a
  `TrafficSignal` for `028258_objects`.
- `BusDrawFadeLights_8c028022` crossfades the night light rows.

Three corrections came out of it:

1. `BusDrawPlaceEntity_8c027c3c`'s second parameter was documented as a
   heading; `025b98` passes a 4-frame sum of speed deltas, i.e. acceleration.
   It feeds `pitchAngle_0x078` (was `acc_0x078`), which leans the body
   forward under braking -- `024b4c` converts the same field to degrees to
   raise and lower the cockpit camera. `rollAngle_0x07c` (was `ang_0x07c`)
   is the lateral lean, clamped to the same 4 degrees here and in `022bdc`.
2. `bodyModel_0x14` is not a body: it is drawn with `njCnkModDrawObject`
   under `NJD_CONTROL_3D_SHADOW | ..._TRANS_MODIFIER`, so it is the shadow
   volume. Renamed `shadowModel_0x14`, which also stops it colliding with
   `bodyModels_0x04c[6]`, the six actual body variants.
3. `TrafficEntry.steerNode_0x018` steers nothing -- it takes pitch on `ang[0]`
   and roll on `ang[2]`, and the lamp nodes hang off it. Renamed
   `bodyNode_0x018`.

`BusState` and `TrafficEntry` are two views of one 0x00-0x60 prefix and had
drifted into two names per byte (`field_0x000`/`typeCode_0x000`,
`rearWheel_0x024`/`wheelNode3_0x024`, ...). They now agree, and the five lamp
pointers at 0x02c became one `blinkerLights_0x02c[5]` in both, so the
show/hide loop reads off `blinker_0x080`'s low five bits directly. Bit 0 is
the brake lamp; bits 3 and 4 are the night/evening running lights. Still
divergent, and worth a later pass: `bodyModels_0x04c[6]` is six separate
`field_`s on the `TrafficEntry` side.

Two of its seven functions -- `drawAhead_8c027a88`/`drawMirror_8c027bac`,
registered as `FadeCmdPushCall2_8c022420` draw callbacks by
`BusDrawPlaceEntity_8c027c3c` -- are case (3) of the Ghidra
boundary/call-graph note above.

### `023938_bus_drive` (ShortUnit `BusDrive`) -- done, 5/5 functions

Per-frame driving update for the player's bus: `FUN_8c023938` fills 10
corner/lookahead ground-sample points via the bus's ground-query callback
(`BusState.field_0x2c8`, picked once by `busInitPlaceBus_8c023310`);
`FUN_8c023cba` turns those samples into a steering-correction direction and
averaged height, falling back to the private `busDriveDecelerate_8c023bea`
(a braking-pitch sound cue) when the lane-offset path data isn't ready;
`BusDriveStop_8c023bce` resets the drivetrain to idle (`bus_state_0x2b4 ==
2`) after a knockback (called by `02b464`) or after that braking update.
The public entry points are shared between
`busInitPlaceBus_8c023310`/`BusInitStart_8c023610` (one-time setup) and
`BusTask_8c022bdc` (every frame).

### `022bdc_bus` (ShortUnit `Bus`) -- done, 1/1 function

`BusTask_8c022bdc` is the player-bus per-frame dispatcher: door-timer
advance, the driving/knockback/braking state machine, steering, the ground/
junction queries other systems reuse, blinkers, and the camera. Pushed as
the run's task action by `BusInitStart_8c023610` (`023310_bus_init`).

The unit itself is a single cleanly-exported function -- the "hidden
function" discovery in this cluster was one level over, in
`024b4c_bus_render` (`drawFrontBusModel_8c024cc8`, see below): decompiling
`Bus` is what turned that whole render subsystem's dependency into a live,
rankable target rather than dead weight nothing decompiled called yet.

### `023310_bus_init` (ShortUnit `BusInit`) -- done, 2/2 functions

`busInitPlaceBus_8c023310` places the player's bus at the current run's
starting stop (ground/junction-query callback selection included) and
`BusInitStart_8c023610` arms its per-frame task (`BusTask_8c022bdc`). Called
once at the start of a run.

### `024b4c_bus_render` (ShortUnit `BusRender`) -- done, 8/8 functions

Lights, textures, draws and cameras the player's bus model: the gameplay
camera (`BusRenderUpdateCamera_8c025078`, using the private
`positionCamera_8c024d6c` helper), the mirror camera
(`BusRenderUpdateMirrorCamera_8c025604`), the third-person model
(`BusRenderDrawBusModel_8c024bb8`), and the camera-mode plumbing
(`BusRenderApplyCameraMode_8c024f32`, plus the
`BusRenderSaveCameraState_8c024b4c`/`BusRenderRestoreCameraState_8c024b86`
pair that parks the camera state across a stop scene).

`drawFrontBusModel_8c024cc8` (the dashboard-view model) is reachable only
via a function pointer sitting in `BusRenderUpdateCamera_8c025078`'s
literal pool -- case (3) of the Ghidra boundary/call-graph note above.

### `025870_demo` (ShortUnit `Demo`) -- done, 5/5 functions

The attract-mode camera tour. What had been read as "demo playback plus an
in-drive next-stop textbox" is one thing: a per-route table of camera shots,
each with a place name captioned over it.

`DemoStartTour_8c025af4` selects the route's `DemoShot[]` and pushes
`demoShotTask_8c0259e8`; that task watches a **camera cue** -- the top byte of
`var_markDriveFlags_8c1bbd80`, which `BusTask_8c022bdc` refills every frame
from the mark-attribute polygon under the bus -- and on a change cuts to that
id's shot and types its caption. `applyShotPosition_8c0258ba` resolves the
shot's position once, `DemoUpdateCamera_8c025906` places the camera from it
each frame.

**The cues are not bus stops**, though they get painted where stops are. The
low bits of the same word are the HUD's driving instructions (turn signal,
headlights, wipers, gear -- `01fa78`); stop segment ids live in a different
word, `markCueByte_0x3b4`. The counts say the same thing: 63/45/23 cues per
route against a 31-slot stop schedule, and only 21/4/2 of them are named --
most are unnamed bus-relative framings, several in a row at one spot to cut
between angles. What the shared polygon does buy is
`BusStopUpdateArrival_8c02ce48`'s phase 4, which tests the cue byte as a plain
flag to decide a stop is over. `var_markDriveFlags_8c1bbd80` in `sectionB.h`
now carries the full bit map.

Two things worth keeping:

- **`DemoBoardingCamera_8c025870` is not demo code.** Its only caller is
  `StopSpawnInit_8c02d968`, which runs in normal play -- it aims the fade
  camera down the aisle for the passenger boarding shot. It keeps the `Demo`
  prefix because that is the unit it lives in (as `016bf4_demo_input` also
  does), not as a claim about when it runs; its header says so plainly. If
  the prefix ever reads as a lie, the fix is a new ShortUnit for the pair of
  scripted-camera jobs, not a quiet rename.
- The 28 Shift-JIS place names left the `.c` as hex byte arrays and became
  `MSG_PLACE_*` in `strings_ja_jp.sjis.h`/`strings_en_us.h`, written inline in
  the shot tables. They were never constants: they are the compiler's own
  literal pool, and inlining reproduces it byte for byte. Getting there also
  turned up 3 bytes of section-alignment fill that the archived `.src` had
  recorded as data -- dropped from the `.src`, matching build unaffected. Both
  that and why the size cannot be derived with `sizeof` are in
  `lessons_learned.md`; `01e27c_practice_menu` still uses the older
  `TEXT_SJIS_SIZE(n)` form and could get the same treatment.

`demoShotTask_8c0259e8` is a `TaskPush_8c014ae8` action -- exported normally
in the `.src`, but reachable only by address. Worth remembering even when
Ghidra's export list looks complete: a `TaskPush` install site can hide a
function's real call graph the same way a raw literal pool does.

### `02e51c_attr_query` (ShortUnit `AttrQuery`) -- done, 5/5 functions

Queries the *attribute* grid, the sibling of the collision ("atari") grid
that `020914_ground_query` searches. The course supplies both as a pair --
`atariBus_0x04`/`atariCpu_0x18` select `var_activeGroundGrid_8c2264d4`,
`attrBus_0x10`/`attrMark_0x14`/`attrCpu_0x20` select
`var_activeAttrGrid_8c228b3c` -- and every caller sets the two together,
which is where the `AttrQuery`/`GroundQuery` name pairing comes from.

Four lookups return the attribute polygon under a point, in the same
`(x, y, z, out)` shape as `GroundQueryFindPolygon_8c020914` but with a
different 3-field result layout, split along two axes: convex-only
(`8c02e51c`/`8c02eab4`) vs. the `shapeFlag_0x10` convex/concave split
(`8c02e69c`/`8c02ec50`), and plain vs. `AtHeight`.

This unit was previously left hex-only on the grounds that
`AttrQueryRegionOccupied_8c02f08a`, an integer task-scan, was an unrelated
second job. That was wrong, and the mistake is worth keeping: the four
lookups *produce* a polygon's `attr_0x08` (stored as `signalId_0x410` on an
AI entry by `025b98`, as `fallbackTaskMatchId_0x3a0` on the player by
`022bdc` -- same query, same field), and `8c02f08a` is the consumer that
asks which actor currently holds a given one. Reading the odd function out
against its *callers'* fields, rather than against its siblings' shape,
was what dissolved the apparent split. Worth trying before invoking the
"exports span unrelated jobs" exemption elsewhere -- it retired the last
holder of that exemption, `027958_bus_draw` above, for the same reason.

### `02b464_drive_points` (ShortUnit `DrivePoints`) -- done, driving-evaluation/penalty subsystem

Ghidra had hidden six functions reachable only via TaskPush pointers,
including the master per-frame `taskCallback_8c02c072` -- see the boundary
note above. All four of its raw-asm dependencies (`024b4c`, `023938`,
`02e2dc`, `02b2f0`) have since been decompiled; see their own entries.

**The graders are a priority chain, not a list.** `taskCallback_8c02c072`
runs them inside nested `if (cooldown-- < 0)` blocks, so each is gated on the
cooldown that the category *above* it arms, and each arms its own via
`armCooldowns_8c02b578`. One offense therefore suppresses every lesser one
while its cooldown runs -- 150 frames for a collision, 210 for off-course.
`gradeOffCourseSevere_8c02b864` sits outside the chain and is never
suppressed.

That `armCooldowns_8c02b578(type)` argument is also what identifies each
grader's subject, and is where the names below came from rather than
guesswork: 1 collision, 2 off-course, 3 signal, 4 lane, 5 intersection.
`gradeWallHit_8c02b7ea` (was `handleFlags_8c02b7ea`),
`gradeOffCourseSevere_8c02b864`, `gradeOffCourse_8c02b886`,
`gradeSignals_8c02b8b8`, `gradeLaneUse_8c02b986`,
`gradeIntersection_8c02bb1c`, and the ungated per-frame
`gradeFrame_8c02bcd8`. The drive-end group became
`DrivePointsRunComplete_8c02c586`, `driveEndFadeTask_8c02c69a`,
`beginDriveEnd_8c02c738`, `onFadeRunFailed_8c02c76a`,
`onFadeStopEnded_8c02c624` and `DrivePointsOnFadeDriveEnd_8c02c784`.

`gradeIntersection_8c02bb1c` is the weakest of those: its type-5 cooldown
only covers the blocked-intersection branch, and the speeding check is the
bigger half of the function. The three `onFade*` names are an invention --
there is no convention in the codebase for a
`var_fadeCompleteCallback_8c22656c` target, since `022464_fade.c` only ever
assigns `FADE_NO_CALLBACK`.

The 34 `init_8c04b*`/`init_8c04c*` HUD glyph-row tables are left
address-named on purpose: they are only ever reached through
`init_penaltyMsgGlyphs_8c04c35c[msgSet]`, so an individual name would add nothing the table's
own comment doesn't already say.

### `02b2f0_drive_msg` (ShortUnit `DriveMsg`) -- done, 2/2 functions

The banner half of `02b464_drive_points`: it only draws, `02b464` owns the
queue. Glyph ids index a 16x16 cell atlas in `var_markTexlist_8c1bc418` --
the same texlist and id space as `0129cc_pause.c`'s `MARK_*` sprites.

Two corrections came out of this pass, both in code the unit only reads:

- `DriveMsgSlot.duration` is the row's starting x, not a time. `02b464` sets
  it to `(640 - 32 * count) / 2`, which centers a row of 32-wide glyphs.
  Renamed to `x`; the old name had a note in `sectionB.h` saying the draw
  side contradicted it, which is the kind of note that should have been a
  rename.
- `(ResourceGroup *)&var_markTexlist_8c1bc418` was commented here (and in the
  test, and indexed in `gameplay.md`) as an original bug passing the
  variable's own address. It isn't: 8c1bc418/41c/420 are loaded from
  `mark.pvm`, `mark_parts.dat` and `mark.dat` -- the same
  pvm/`_parts.dat`/`.dat` triple `012f44_game.c` loads into the *declared*
  `ResourceGroup var_loadingResourceGroup_8c1bc3f8`. That also settles the
  open doubt in `gameplay.md` about `tanim_0x04` holding a raw dat pointer:
  a real group holds one too. Bug entry removed; folding the three into one
  symbol is a move-data job.

`var_8c2285c8` became `var_runPassed_8c2285c8`: `BusStopUpdateArrival_8c02ce48`
sets it when a drive ends with points left and every owed stop served, and
this unit's only use is to draw mark sprite `0x78` instead of the banner
stack. Nothing ever clears it. What that sprite depicts is unverified -- the
id sits in the middle of the pause menu's `0x74..0x7c` run.

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

### `02f0c8_traffic_path_scan` (ShortUnit `TrafficPathScan`) -- done, 3/3 functions

Answers "who occupies this stretch of path" by sampling a route every 5 units
and testing the player bus and every traffic task against each sample. Junction
yield is its main consumer, but not its only one: `025b98`'s respawn gate and
`026710`'s `spawnEntry_8c0272b8` use the same scan to decide whether there is
room to place a vehicle.

### `02e400_collision` (ShortUnit `Collision`) -- done, 4/4 functions

Everything here tests against one shared box, `var_collisionSelfBox_8c228978`.
No section C/D data of its own.

- `CollisionFindTaskHit_8c02e400(self, entry)` -- transforms `entry`'s
  per-variant box into world space, leaves it in the shared box, and returns
  the first other traffic entry whose own box overlaps it
  (`njCollisionCheckBB`). Called by `TrafficDriveVehicle_8c025b98`.
- `CollisionQueueReset_8c02e486` / `CollisionQueueAdd_8c02e48e(obj)` /
  `CollisionQueueTest_8c02e4ac()` -- a 64-slot pointer queue
  (`var_collisionQueue_8c228a38`, count `var_collisionQueueCount_8c228b38`).
  `028258_objects` resets it each frame and enqueues every pedestrian; `02b464`
  tests it against the shared box (`njCollisionCheckBS`).

The queue test never fills that box, so what it answers depends on who wrote it
last -- and its only caller, `handleBump_8c02b6d4` (`02b464`), calls it *before*
`BusCollisionFindHit_8c02e2dc`. The box at that moment is whatever the last
traffic entity's `CollisionFindTaskHit_8c02e400` left behind, so the pedestrian
near-miss may be scored against an AI car's box rather than the bus's. Whether
that is a bug or relies on frame ordering is unresolved -- it needs the task
order traced, not more reading of these two units.

**Original-game bug preserved and commented:** the self box is built from
`&init_variantBoxes_8c04c940[idx]` -- the address of the table slot -- while a
candidate's uses `init_variantBoxes_8c04c940[idx]`, the box it points at.
Self's "box" is therefore the pointer table's own bytes reinterpreted as
floats. Confirmed against the `.src` object, not corrected.

### `02e2dc_bus_collision` (ShortUnit `BusCollision`) -- done, 2/2 functions

The player bus's half of the collision pair above, and the owner of the box
data both units use: eleven local-space 8-corner boxes plus
`init_variantBoxes_8c04c940`, 16 pointers into them indexed by a traffic
entry's variant index (`entry+0x2e0`) -- the same 16 variants as
`026710_traffic`'s `init_8c04622c` / `init_8c0460c8`. Slot 13 is the bus's own
box, also reached by name as `init_busBox_8c04c820`.

`BusCollisionFindHit_8c02e2dc` prefilters on `busDistance_0x490 < 12.0` (distance to
the bus, refreshed per frame by the drive tasks) before box-testing. It uses
`GeomQuadOverlap_8c020842` where the otherwise identical traffic-vs-traffic
scan next door uses `njCollisionCheckBB` -- an asymmetry that is invisible
unless you read both, and easy to mistake for a decompilation error later.

`findNextHit_8c02e35a` is dead code, kept for object parity (see the
dead-functions section above). It resumes the scan from wherever
`var_collisionScanCursor_8c228974` is sitting and reuses the box already there,
so it would yield the bump after the one just reported.

### `02df3c_traffic_lookahead` (ShortUnit `TrafficLookahead`) -- done, 2/2 functions

Keeps each traffic entry's path-ahead cache (`lookaheadPoints_0x49c`): world
(x, z) sampled every 5 units, `9999.0` terminated.
`TrafficLookaheadInit_8c02df3c` seeds it to 25 units at spawn;
`TrafficLookaheadScan_8c02dfca` tops it back up to 20 (the cache is an
odometer -- driving shrinks it) and reports what is standing on it.

The player's bus is tested against six points -- its current position and the
newest history sample within 2.5 units, four older samples within 2.0 -- and
that predicate appeared **twice**, once as a `||` chain and once as six
separate `if`s, differing only in that one arm `break`s where the other
`return`s to the same value. Now one `BUS_BLOCKS` macro. A macro rather than a
helper function because a new `STATIC` function would have no address for the
`_8c<addr>` suffix that `check_naming.py` requires.

### `02d968_stop_spawn` (ShortUnit `StopSpawn`) -- done, 1/1 function

`StopSpawnInit_8c02d968`, course-start setup for the bus-stop passenger
subsystem: the six bus-interior anchor points, then one `02d19c_passenger` (`Passenger`)
task per already-picked waiting passenger and per scripted schedule slot. Slots
whose stop segment matches the bus's current segment are collected, Fisher-Yates
shuffled, and spawned as `PassengerExitTask_8c02d46c`; the rest spawn directly
as `PassengerSeatedTask_8c02d5ca`.

Seat position in the shuffled loop follows the *shuffled* order
(`init_seatPositions_8c04c3e4[i]`) while `slotIndex_0x24` keeps the original
schedule slot -- commented in place, since it reads like an indexing mistake.

`init_passengerVoiceVariant_8c04c4dc` (was `init_8c04c4dc`) was `STATIC` in the C
but `.EXPORT`ed unconditionally by the `.src`; now gated under
`.AIFDEF UNIT_TESTING` like every other private symbol.

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
its path, `02c884_bus_stop` grounds waiting-passenger sprites. A fourth
caller, `023310_bus_init`'s `busInitPlaceBus_8c023310`, has since landed
too -- it also stashes the function pointer itself in
`BusState.field_0x2c8` for `022bdc_bus`'s per-frame dispatcher to invoke.

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
task running `TrafficDriveVehicle_8c025b98` (moving vehicle) or
`TrafficDriveDecoration_8c02656a` (fixed decoration) -- see the
`025b98_traffic_drive` entry below.

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

### `02d19c_passenger` (ShortUnit `Passenger`) -- done, 8/8 functions

The bus-stop cutscene's passengers. `PassengerBoardTask_8c02d21c` and
`PassengerExitTask_8c02d46c` each walk a passenger through three waypoints
(`var_boardSpot1..3`, `var_exitSpot1..3` in sectionB), but there is no walk
animation: a step only happens on the single frame
`var_passengersFadedOut_8c22895c` is set, which is the frame
`var_passengerFadeColor_8c228960`'s alpha reaches zero. The passenger vanishes
at one spot and reappears at the next in a new pose, and `delay_0x20` staggers
them so they go one at a time.

Was `@unit BusRider`. Renamed because the rest of the codebase already said
"passenger" in seven symbol names across unrelated units and in the string
table, so 02d968 was reading `var_waitingPassengers_8c228798` and spawning
`BusRider*` tasks from it. "Alight" went to "exit" at the same time -- correct
transit English, but not the American reading the rest of the comments are
written in.

### `02d06c_stop_draw` (ShortUnit `StopDraw`) -- done, 3/3 functions

Draws the passengers waiting at the upcoming stop, plus the FadeCallback1 pair
that brackets every passenger draw. `StopDrawLightBegin_8c02d0fc` is where
`njSetConstantMaterial(var_passengerFadeColor_8c228960)` happens -- the alpha
that fades passengers, not the interior: `drawInterior_8c02d1f4` sets no
constant material at all.

`pedestriansTask_8c0293f6` (028258) registers
`StopDrawWaitingPassengers_8c02d06c` once per layer and passes the layer
through as the argument too, so layer 0 draws one facing and layer 1 (the
mirror) the other -- the same 0x22/0x23 standing pair
`drawPassengerSprite_8c02d19c` uses for the two aisle columns.

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
only allocation is `pickWaitingPassengers_8c02c8ae`'s scratch list (16 pointers,
written as `16 * sizeof(void *)`), freed before return. That size is an
unchecked assumption: the scan that fills it is bounded only by the segment's
candidate list, so a segment offering more than 16 active spots would write past
it. Commented in place; not reachable with the shipped route data as far as the
tests exercise it. Waiting passengers land in `var_waitingPassengers_8c228798[16]` of
`WaitingPassengerSlot` (0x14 bytes: spot pointer, `NJS_POINT3` pos, pick-order
float), laid along the stop strip as `pos = origin + n*dir` plus `rand()` jitter
-- except on `ROUTE_OME`, which gets no jitter.

**Original-game bug preserved deliberately:** in `BusStopSetup`, the
segment-scan loop leaves its walking pointer on the table terminator, and the
extra-stop reroll loop then indexes `segments[pick]` off that unreset pointer --
so its `type_0x00` check reads `totalSegments + pick` records past the table
start. The flag-array check is unaffected. Commented in the C as real asm
behavior rather than silently corrected.

The `StopAreaRecord` typedef in the header had been documenting the layout of
`pickWaitingPassengers_8c02c8ae`'s `var_8c22890c` without the function using it
-- four raw `*(float *)(p + n)` reads instead. It now casts to the struct, which
is also what `sectionB.h` points at rather than restating the offsets.

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
