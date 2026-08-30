# Gameplay Reference

How the game actually plays, from someone who has played it. Useful context
when naming functions/data: the code was written to implement *this*.

Tokyo Bus Guide (1999, Dreamcast): a Japanese bus simulator. You drive a city
bus along a predefined route with predefined passenger stop requests.

## Modes

- **Main modes** (3 routes x 3 times of day = 9 courses; see
  `reference_route_assets` / `013ae8_route_load.c` for the courseId scheme):
  - **Story mode**: at some stops, after the interior view, a cutscene event
    may trigger showing passenger dialog: pre-rendered slideshows with audio
    and textboxes. Seen events unlock passenger profiles in the event menu
    ("PROFILE FILE" notebook UI -- grid of portraits, per-character bio pages
    with a checkbox grid of seen events). This is NOT `01d290_album`: the
    album is a separate screen showing a few pictures unlocked throughout
    story mode. The event menu's owning unit is `01c980_profile_file` -- see
    `next_units.md` #1 for the shallow analysis and its confirmed link to
    `02af78_event`'s progress flags.
  - **Free run**: same courses, no cutscenes/events.
- **Practice**: 11 runs teaching individual mechanics, each preceded by a
  slideshow guide. Runs take place on sections (segments) of the main route
  maps. Hypothesis: these are the extra courseTable entries 27+
  (`init_course27..47_`) -- unconfirmed.

## Driving loop

- Follow driving rules: speed limit, turn signals, traffic lights, smooth
  acceleration/braking, and the time schedule.
- Committing a penalty displays a text message on screen.
- A single **score bar** drains with penalties; gameplay ends if it empties.
  Passing bus stops (both boarding stops and drive-bys) replenishes it a bit.
- Reaching the final stop ends the run: score screen, then back to course
  select.
- The VMU LCD shows art during gameplay (see `01bb48` `vmsLcd_*`).

## Stops and segments

- **A passenger stop = a CourseSegment boundary.** The loading screen at a
  stop is the SEGMENT_RELOAD state machine in `013ae8_route_load.c`; while it
  loads, the bus interior view is shown with passengers entering/leaving.
- All pedestrians and interior passengers are rendered as low-resolution
  billboards.
- During the interior scene, passengers move in a low-fps stop-motion manner;
  no texture animation.

## Story event selection (`02af78_event`)

Which cutscene fires at a stop is resolved by `02af78_event`. Each route
(Shinjuku/Wangan/Ome) has an `EventEntry` table
(`init_8c04b1f0`/`init_8c04abb0`/`init_8c04b920`); an entry is eligible for a
segment if its time-of-day matches the current course, its packed
day-of-week mask contains the in-run day (`var_progress.days_0x00` -- so day-
of-month IS an input, confirming the prior guess), and its packed
prerequisite codes against progress flags pass. `scanEventCandidates_8c02b03c`
computes eligible entries once per course load;
`pickSegmentEvent_8c02b170` narrows to the current segment and randomly
picks one, arming `var_cutsceneActive_8c1bb900`; `applyEventFlags_8c02b292`
then applies the chosen entry's actions once the cutscene plays: persistent
progress flags, and/or a per-day "run flag" (`var_runEventFlags`) that lets an
event fired on an earlier segment gate a later segment's pick. Run flags are
cleared at the start of each scan and never saved. One scan == one course
attempt == one day: the day counter advances whether the course is passed or
failed (confirmed by play), so run flags are effectively per-day scratch state.
Practice mode and the course menu skip selection entirely.

## Known shipped bugs (preserved)

Real defects in the SHIPPED game, found while decompiling and deliberately
kept (correct for functional equivalence -- do not "fix" these). Each is
also noted in a code comment at its site; this is the index. All three so
far are the same shape: the address of a pointer variable (`&ptr`) passed
where the pointer's value (`ptr`) was meant.

- **`FUN_8c023e7e`** (`023938_bus_drive.c`) calls `sdMidiPlay` with
  `&var_midiHandles_8c0fcd28[0]` -- the array's address -- where every other
  call site in the codebase passes the handle value
  `var_midiHandles_8c0fcd28[0]`.
- **`DriveMsgDraw_8c02b388`** (`02b2f0.c`) passes `&var_markTexlist_8c1bc418`
  -- the pointer variable's own address -- to `TxtDrawSprite_8c014f54`, while
  the `njSetTexture` call five lines below it in the same function correctly
  dereferences the same variable. See the caveat below: the same `&ptr` cast
  is also used, consistently, at many other call sites for "mark"/"bus stop"
  sprites -- worth a second look before treating this one as clear-cut.
- **`CollideFindTaskHit_8c02e400`** (`02e400_collision.c`) builds the "self"
  bounding box from `&init_8c04c940[idx]` (the table slot's address) while a
  candidate's box comes from `init_8c04c940[idx]` (the box it points at), so
  self is built from the pointer table's own bytes reinterpreted as floats.

**Caveat on the `DriveMsgDraw` case:** `var_markTexlist_8c1bc418` is
immediately followed in memory by `var_markPartsDat_8c1bc41c` and then
`var_markDat_8c1bc420` (`sectionB.h`), byte-contiguous and exactly
`sizeof(ResourceGroup)` (12 bytes: `tlist_0x00`/`tanim_0x04`/`contents_0x08`).
The same `(ResourceGroup *)&var_markTexlist_8c1bc418` cast used in
`DriveMsgDraw_8c02b388` also appears, unremarked, at ~9 call sites in
`0129cc_pause.c` and one in `02b464_drive_points.c` (plus the analogous
`&var_busStopTexlist_8c1bc424` cast 4x in `022464_fade.c`) -- always for
drawing these same "mark"/"bus stop" sprites. That looks less like a typo
than a deliberate inline-struct-via-adjacent-globals layout (the same kind
of representation the project already tracks as "ResourceGroup ptr debt",
e.g. `var_resourceGroup_8c2263a8` in `016108.c`), which would make this
usage *correct* rather than buggy -- except that `tanim_0x04` then reads
`var_markPartsDat_8c1bc41c` (a raw dat-file pointer, not a real
`NJS_TEXANIM*`), so whether it is truly harmless depends on whether
`njDrawSprite2D` (SDK, not decompiled here) reads `tanim` for these sprite
IDs. Flagging for a second opinion rather than deciding unilaterally; the
code comment and this entry are left as originally written pending that call.



- Practice runs = courseId 27+ is a player recollection, not verified in
  code.
