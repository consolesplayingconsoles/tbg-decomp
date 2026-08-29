/* @unit Demo */

#include <shinobi.h>

#include "serial_debug.h"
#include "sectionB.h"
#include "013ae8_route_load.h"
#include "014a9c_tasks.h"
#include "0222dc_fadecmd.h"
#include "024b4c_bus_render.h"
#include "028258_objects.h"
#include "025870.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* One 20-byte entry of the per-route stop tables (init_8c045674/
 * init_8c045b60/init_8c045ee4): {kind, pos, name}. A cast-only view over
 * those flat int arrays -- see the comment above init_8c045674 for why the
 * arrays themselves stay untyped. */
typedef struct {
    int kind_0x00;
    NJS_POINT3 pos_0x04;
    char *name_0x10;
} StopRecord;

/* stopTextboxTask_8c0259e8's TaskPush_8c014ae8 state, exactly the 0xc it
 * asks for. */
typedef struct {
    int phase_0x00;
    int revealCount_0x04;
    int textboxHandle_0x08;
} StopTextboxState;

/* ====================
 * Initialized Globals
 * ====================
 */

STATIC const Uint8 const_8c039f78[] = {
    0x92, 0x86, 0x83, 0x6D, 0x8B, 0xB4, 0x00, 0x00,
};

STATIC const Uint8 const_8c039f80[] = {
    0x00, 0x00, 0x00, 0x00,
};

/* "\x88\xea\x83\x6D\x8b\xb4" -- SHIFT-JIS place name (dump_src_data.py
 * doesn't decode live .SDATA directives, only commented-out ones). */
STATIC const Uint8 const_8c039f84[] = {
    0x88, 0xEA, 0x83, 0x6D, 0x8B, 0xB4, 0x00, 0x00,
};

/* "\x94\xd1\x91\x71\x95\xd0\x92\xac" -- SHIFT-JIS place name (same
 * dump_src_data.py .SDATA limitation as const_8c039f84). */
STATIC const Uint8 const_8c039f8c[] = {
    0x94, 0xD1, 0x91, 0x71, 0x95, 0xD0, 0x92, 0xAC, 0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_8c039f98[] = {
    0x98, 0x5A, 0x96, 0x7B, 0x96, 0xD8, 0x8C, 0xF0,
    0x8D, 0xB7, 0x93, 0x5F, 0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_8c039fa8[] = {
    0x98, 0x5A, 0x96, 0x7B, 0x96, 0xD8, 0x00, 0x00,
};

STATIC const Uint8 const_8c039fb0[] = {
    0x96, 0x68, 0x89, 0x71, 0x92, 0xA1, 0x91, 0x4F,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_8c039fbc[] = {
    0x90, 0xD4, 0x8D, 0xE2, 0x94, 0xAA, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_8c039fc8[] = {
    0x93, 0xEC, 0x90, 0xC2, 0x8E, 0x52, 0x88, 0xEA,
    0x92, 0x9A, 0x96, 0xDA, 0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_8c039fd8[] = {
    0x90, 0xC2, 0x8E, 0x52, 0x88, 0xEA, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_8c039fe4[] = {
    0x90, 0xC2, 0x8E, 0x52, 0x88, 0xEA, 0x92, 0x9A,
    0x96, 0xDA, 0x8C, 0xF0, 0x8D, 0xB7, 0x93, 0x5F,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_8c039ff8[] = {
    0x90, 0xD4, 0x8D, 0xE2, 0x8C, 0xE4, 0x97, 0x70,
    0x92, 0x6E, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a004[] = {
    0x90, 0xC2, 0x8E, 0x52, 0x93, 0x73, 0x89, 0x63,
    0x83, 0x41, 0x83, 0x70, 0x81, 0x5B, 0x83, 0x67,
    0x91, 0x4F, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a018[] = {
    0x96, 0xBE, 0x8E, 0xA1, 0x8B, 0x4C, 0x94, 0x4F,
    0x8A, 0xD9, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a024[] = {
    0x90, 0x5F, 0x8B, 0x7B, 0x8A, 0x4F, 0x89, 0x91,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a030[] = {
    0x90, 0x4D, 0x94, 0x5A, 0x92, 0xAC, 0x89, 0x77,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a03c[] = {
    0x90, 0x4D, 0x94, 0x5A, 0x92, 0xAC, 0x89, 0x77,
    0x91, 0x4F, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a048[] = {
    0x8E, 0x6C, 0x92, 0x4A, 0x8E, 0x4F, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a054[] = {
    0x8E, 0x6C, 0x92, 0x4A, 0x8E, 0x6C, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a060[] = {
    0x90, 0x56, 0x8F, 0x68, 0x88, 0xEA, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a06c[] = {
    0x90, 0x56, 0x8F, 0x68, 0x93, 0xF1, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a078[] = {
    0x90, 0x56, 0x8F, 0x68, 0x92, 0xCA, 0x82, 0xE8,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a084[] = {
    0x92, 0xAA, 0x95, 0x97, 0x8C, 0xF6, 0x89, 0x80,
    0x93, 0xFC, 0x8C, 0xFB, 0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a094[] = {
    0x91, 0xE4, 0x8F, 0xEA, 0x89, 0x77, 0x91, 0x4F,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a0a0[] = {
    0x83, 0x74, 0x83, 0x57, 0x83, 0x65, 0x83, 0x8C,
    0x83, 0x72, 0x91, 0x4F, 0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a0b0[] = {
    0x82, 0xA8, 0x91, 0xE4, 0x8F, 0xEA, 0x8A, 0x43,
    0x95, 0x6C, 0x8C, 0xF6, 0x89, 0x80, 0x89, 0x77,
    0x91, 0x4F, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a0c4[] = {
    0x93, 0x8C, 0x90, 0xC2, 0x94, 0x7E, 0x89, 0x77,
    0x91, 0x4F, 0x00, 0x00,
};

STATIC const Uint8 const_8c03a0d0[] = {
    0x90, 0xC2, 0x94, 0x7E, 0x8E, 0x73, 0x96, 0xF0,
    0x8F, 0x8A, 0x91, 0x4F, 0x00, 0x00, 0x00, 0x00,
};

/* Per-route stop-announcement tables, picked by var_route_8c18ad1c
 * (FUN_8c025af4): init_8c045674 = ROUTE_SHINJUKU, init_8c045b60 =
 * ROUTE_WANGAN, init_8c045ee4 = ROUTE_OME. Flat records of 5 words each --
 * {kind, x, y, z, name} -- kept untyped since the fields interleave int and
 * float and the original .src marks two internal boundaries
 * (init_8c046000/init_8c04608a) that don't land on a record boundary
 * (mid-record split, not real symbols -- nothing else in the game
 * references them), so they're folded into init_8c045ee4 here. */
STATIC int init_8c045674[] = {
    0x00000001, 0x40800000, 0x3F000000, 0xC0800000,
    (int)const_8c039f78, 0x00000000, 0x45815000, 0x41500000,
    0x458B2000, (int)const_8c039f80, 0x00000000, 0x45767000,
    0x41980000, 0x4589E800, (int)const_8c039f84, 0x00000000,
    0x45754000, 0x40800000, 0x45865000, (int)const_8c039f80,
    0x00000000, 0x4576C000, 0x3E99999A, 0x45686000,
    (int)const_8c039f8c, 0x00000002, 0xC0000000, 0x3F800000,
    0xC1200000, (int)const_8c039f80, 0x00000001, 0xC0000000,
    0x3F800000, 0x40C00000, (int)const_8c039f80, 0x00000002,
    0x40400000, 0x3F4CCCCD, 0x41200000, (int)const_8c039f80,
    0x00000002, 0xC0800000, 0x40200000, 0xC1200000,
    (int)const_8c039f80, 0x00000002, 0x40400000, 0x40200000,
    0xC1C80000, (int)const_8c039f80, 0x00000001, 0x40400000,
    0x40200000, 0xC1C80000, (int)const_8c039f80, 0x00000002,
    0x40F00000, 0x3FE66666, 0x00000000, (int)const_8c039f80,
    0x00000002, 0x41300000, 0x3FE66666, 0x00000000,
    (int)const_8c039f80, 0x00000000, 0x45592000, 0x3F800000,
    0x455B7000, (int)const_8c039f98, 0x00000000, 0x4556E000,
    0x42200000, 0x45559000, (int)const_8c039fa8, 0x00000002,
    0xC0400000, 0x40000000, 0xC0C00000, (int)const_8c039f80,
    0x00000001, 0xC0C9999A, 0x40000000, 0xC0C00000,
    (int)const_8c039f80, 0x00000000, 0x45452000, 0x4099999A,
    0x45471000, (int)const_8c039fb0, 0x00000002, 0xC0A00000,
    0x40400000, 0x41100000, (int)const_8c039f80, 0x00000001,
    0xC0200000, 0x40400000, 0xC0400000, (int)const_8c039fbc,
    0x00000002, 0xC0800000, 0x41780000, 0x41A00000,
    (int)const_8c039f80, 0x00000000, 0x4536CB33, 0x40A00000,
    0x45217000, (int)const_8c039fc8, 0x00000002, 0xC0000000,
    0x3F333333, 0x41000000, (int)const_8c039fd8, 0x00000000,
    0x452CC000, 0x40E00000, 0x451B1000, (int)const_8c039fe4,
    0x00000002, 0x41F00000, 0x3F000000, 0x00000000,
    (int)const_8c039f80, 0x00000002, 0x41F00000, 0x3F000000,
    0x00000000, (int)const_8c039f80, 0x00000000, 0x4522F000,
    0x3F800000, 0x450C4000, (int)const_8c039ff8, 0x00000000,
    0x45205000, 0x3F000000, 0x45075000, (int)const_8c03a004,
    0x00000002, 0xC0A00000, 0x3F000000, 0xC0400000,
    (int)const_8c039f80, 0x00000001, 0xC0A00000, 0x3F000000,
    0xC0400000, (int)const_8c03a018, 0x00000000, 0x45160000,
    0x40C00000, 0x44D56000, (int)const_8c03a024, 0x00000002,
    0xC1700000, 0x40B00000, 0xC0E00000, (int)const_8c03a030,
    0x00000001, 0xC1700000, 0x40B00000, 0xC0E00000,
    (int)const_8c039f80, 0x00000002, 0xC0C00000, 0x3F800000,
    0x00000000, (int)const_8c03a03c, 0x00000002, 0xC0C00000,
    0x3F800000, 0xC0A00000, (int)const_8c039f80, 0x00000001,
    0xC0A00000, 0x3F800000, 0xC0A00000, (int)const_8c039f80,
    0x00000002, 0xC0A00000, 0x3F800000, 0x40A00000,
    (int)const_8c039f80, 0x00000000, 0x45170000, 0x420C0000,
    0x44408000, (int)const_8c03a048, 0x00000002, 0x00000000,
    0x40E00000, 0x41C80000, (int)const_8c039f80, 0x00000002,
    0x00000000, 0x40E00000, 0x41C80000, (int)const_8c039f80,
    0x00000000, 0x44FFE000, 0x3F000000, 0x44504000,
    (int)const_8c03a054, 0x00000002, 0x40400000, 0x3F000000,
    0x40E00000, (int)const_8c039f80, 0x00000002, 0xC0400000,
    0x3F000000, 0x40E00000, (int)const_8c03a060, 0x00000000,
    0x44CA6000, 0x40C00000, 0x44378000, (int)const_8c03a06c,
    0x00000000, 0x44B50000, 0x40E00000, 0x442D8000,
    (int)const_8c039f80, 0x00000002, 0x00000000, 0x40200000,
    0xC1A00000, (int)const_8c039f80, 0x00000000, 0x44926000,
    0x3F800000, 0x44030000, (int)const_8c03a078, 0x00000000,
    0x44926000, 0x3F800000, 0x44030000, (int)const_8c039f80,
    0x00000000, 0x44926000, 0x3F800000, 0x44030000,
    (int)const_8c039f80, 0x00000000, 0x44800000, 0x3F800000,
    0x43E88000, (int)const_8c039f80, 0x00000000, 0x4482E000,
    0x3F800000, 0x43EF8000, (int)const_8c039f80, 0x00000001,
    0xC0400000, 0x3F4CCCCD, 0xC0400000, (int)const_8c039f80,
    0x00000000, 0x446CC000, 0x40A00000, 0x43C70000,
    (int)const_8c039f80, 0x00000002, 0xC0800000, 0x40A00000,
    0xC1700000, (int)const_8c039f80, 0x00000001, 0xC0800000,
    0x40A00000, 0xC1700000, (int)const_8c039f80, 0x00000000,
    0x44248000, 0x40A00000, 0x43960000, (int)const_8c039f80,
    0x00000000, 0x44248000, 0x40A00000, 0x43960000,
    (int)const_8c039f80, 0x00000000, 0x44230000, 0x3FC00000,
    0x43520000, (int)const_8c039f80, 0x00000000, 0x44120000,
    0x00000000, 0x432B0000, (int)const_8c039f80, 0x00000002,
    0xC0800000, 0x3F000000, 0x40A00000, (int)const_8c039f80,
    0x00000002, 0x00000000, 0x40200000, 0x41200000,
    (int)const_8c039f80, 0x00000000, 0x440B8000, 0x41200000,
    0x439F0000, (int)const_8c039f80, 0x00000000, 0x43DF8000,
    0x00000000, 0x43B00000, (int)const_8c039f80,
};

STATIC int init_8c045b60[] = {
    0x00000000, 0x456D0000, 0x3F99999A, 0x4534D000,
    (int)const_8c039f80, 0x00000000, 0x4576B000, 0x41500000,
    0x45322000, (int)const_8c039f80, 0x00000002, 0xC0400000,
    0x40400000, 0xC1000000, (int)const_8c039f80, 0x00000002,
    0x40400000, 0x40400000, 0xC1000000, (int)const_8c039f80,
    0x00000000, 0x459B9000, 0x41000000, 0x4546C000,
    (int)const_8c039f80, 0x00000000, 0x459B9000, 0x40C00000,
    0x4546C000, (int)const_8c039f80, 0x00000002, 0x40400000,
    0x3F000000, 0xC0E00000, (int)const_8c039f80, 0x00000000,
    0x45815000, 0x41500000, 0x458B2000, (int)const_8c039f80,
    0x00000002, 0x40400000, 0x41500000, 0xC0A00000,
    (int)const_8c039f80, 0x00000002, 0xC0400000, 0x41500000,
    0xC0A00000, (int)const_8c039f80, 0x00000000, 0x4560E000,
    0x40000000, 0x4556A000, (int)const_8c039f80, 0x00000002,
    0x40A00000, 0x40800000, 0xC1A00000, (int)const_8c039f80,
    0x00000002, 0xC0400000, 0x3E99999A, 0x40000000,
    (int)const_8c039f80, 0x00000000, 0x45482000, 0x3FC00000,
    0x45656000, (int)const_8c039f80, 0x00000002, 0xC0400000,
    0x40200000, 0x00000000, (int)const_8c039f80, 0x00000000,
    0x45264000, 0x3F800000, 0x457E7000, (int)const_8c039f80,
    0x00000002, 0x41200000, 0x3FC00000, 0x00000000,
    (int)const_8c039f80, 0x00000002, 0xC0400000, 0x3F000000,
    0x41200000, (int)const_8c039f80, 0x00000001, 0xC0400000,
    0x3F000000, 0x41200000, (int)const_8c039f80, 0x00000000,
    0x452B6000, 0x41FC0000, 0x45931000, (int)const_8c039f80,
    0x00000002, 0x41680000, 0x3FC00000, 0x41500000,
    (int)const_8c03a084, 0x00000001, 0x41680000, 0x3FC00000,
    0x41500000, (int)const_8c039f80, 0x00000000, 0x44ED4000,
    0x40000000, 0x456CD000, (int)const_8c039f80, 0x00000000,
    0x45034000, 0x41C80000, 0x456BA000, (int)const_8c03a094,
    0x00000000, 0x45044000, 0x3F000000, 0x456C4000,
    (int)const_8c039f80, 0x00000000, 0x45031000, 0x3F4CCCCD,
    0x45646000, (int)const_8c039f80, 0x00000000, 0x4512C000,
    0x41400000, 0x455D7000, (int)const_8c03a0a0, 0x00000002,
    0x3F800000, 0x3F800000, 0xC1000000, (int)const_8c039f80,
    0x00000002, 0x3F800000, 0x3F800000, 0xC1000000,
    (int)const_8c039f80, 0x00000002, 0x41000000, 0x3FC00000,
    0xC1300000, (int)const_8c03a0b0, 0x00000001, 0x41000000,
    0x3FC00000, 0xC1300000, (int)const_8c039f80, 0x00000002,
    0x41000000, 0x3FC00000, 0x00000000, (int)const_8c039f80,
    0x00000001, 0x41000000, 0x3FC00000, 0x00000000,
    (int)const_8c039f80, 0x00000000, 0x452DB000, 0x41500000,
    0x4547C000, (int)const_8c039f80, 0x00000000, 0x452DB000,
    0x411CCCCD, 0x4547C000, (int)const_8c039f80, 0x00000002,
    0xC0066666, 0x3F000000, 0x40E00000, (int)const_8c039f80,
    0x00000000, 0x45241E66, 0x411B3333, 0x4532A99A,
    (int)const_8c039f80, 0x00000000, 0x448E4000, 0x3F000000,
    0x44094000, (int)const_8c039f80, 0x00000002, 0xC0400000,
    0x3F000000, 0x40C00000, (int)const_8c039f80, 0x00000000,
    0x448B2000, 0x3F000000, 0x43E70000, (int)const_8c039f80,
    0x00000002, 0x00000000, 0x41840000, 0x42200000,
    (int)const_8c039f80, 0x00000000, 0x44728000, 0x3F000000,
    0x43AC8000, (int)const_8c039f80, 0x00000002, 0x40400000,
    0x3FC00000, 0xC1A00000, (int)const_8c039f80, 0x00000001,
    0x40400000, 0x3FC00000, 0xC1A00000, (int)const_8c039f80,
    0x00000000, 0x442D0000, 0x3FC00000, 0x43928000,
    (int)const_8c039f80,
};

STATIC int init_8c045ee4[] = {
    0x00000000, 0x45DA8000, 0x3F333333, 0x45C2119A,
    (int)const_8c03a0c4, 0x00000000, 0x45DAF800, 0x3F000000,
    0x45C28800, (int)const_8c039f80, 0x00000002, 0xC0400000,
    0x3F333333, 0x41000000, (int)const_8c039f80, 0x00000000,
    0x45E0B000, 0x40CCCCCD, 0x45C80400, (int)const_8c039f80,
    0x00000000, 0x45E0B000, 0x40CCCCCD, 0x45C80400,
    (int)const_8c03a0d0, 0x00000002, 0xC0400000, 0x3F333333,
    0xC1200000, (int)const_8c039f80, 0x00000001, 0xC0400000,
    0x3F333333, 0xC1200000, (int)const_8c039f80, 0x00000000,
    0x45E2F000, 0x3F000000, 0x45C0C800, (int)const_8c039f80,
    0x00000002, 0xC0400000, 0x3FC00000, 0x40400000,
    (int)const_8c039f80, 0x00000002, 0x40A00000, 0x3FC00000,
    0xC1200000, (int)const_8c039f80, 0x00000001, 0x40A00000,
    0x3FC00000, 0xC1200000, (int)const_8c039f80, 0x00000002,
    0x00000000, 0x40400000, 0x41200000, (int)const_8c039f80,
    0x00000000, 0x45D63000, 0x40000000, 0x45B60000,
    (int)const_8c039f80, 0x00000000, 0x45D74800, 0x41900000,
    0x45B34000, (int)const_8c039f80, 0x00000000, 0x45293000,
    0xBF800000, 0x44B68000, (int)const_8c039f80, 0x00000002,
    0xC0800000, 0x3F800000, 0xC1900000, (int)const_8c039f80,
    0x00000001, 0xC0800000, 0x3F800000, 0xC1900000,
    (int)const_8c039f80, 0x00000002, 0x40400000, 0x3F000000,
    0xC0400000, (int)const_8c039f80, 0x00000000, 0x450EB000,
    0x40800000, 0x44A0C000, (int)const_8c039f80, 0x00000000,
    0x450B1000, 0x420C0000, 0x44922000, (int)const_8c039f80,
    0x00000000, 0x45057000, 0x41000000, 0x448A6000,
    (int)const_8c039f80, 0x00000002, 0x00000000, 0x40800000,
    0x41200000, (int)const_8c039f80, 0x00000000, 0x44F90000,
    0x41400000, 0x44474000, (int)const_8c039f80,
};

/* Public: referenced by 012f44_game.c/012f44_game.src. */
char init_8c0460b0[] = {
    0x06, 0x13, 0x33, 0x00, 0x0A, 0x14, 0x1D, 0x25,
    0x00, 0x0E, 0x00, 0x00,
};

/* ====================
 * Functions
 * ====================
 */

/* Sets up the fade camera (var_8c1bb984, owned by 022464) at a fixed vantage
 * point -- likely a canned close-up shot used by a fade/transition. */
void FUN_8c025870(void)
{
    njInitCamera(&var_8c1bb984);
    njSetCameraAngle(&var_8c1bb984, 12743);
    njSetCameraDepth(&var_8c1bb984, -1.0f, -15.0f);
    njTranslateCameraPosition(&var_8c1bb984, -0.2f, 1.8f, -1.6f);
    njPointCameraInterest(&var_8c1bb984, 0.0f, 1.8f, 3.0f);
}

/* Positions the bus's draw position (posX_0x2fc/posY_0x300/posZ_0x304) for
 * DemoUpdateCamera_8c025906's var_8c227d9c==5/6 modes: state 5 pins it to
 * var_8c227e00 directly (y relative to ground); state 6 transforms
 * var_8c227e00 by the bus's world matrix instead. No-op for any other
 * state. Called by DemoUpdateCamera_8c025906's LAB_8c0259e8 helper task
 * (025870, undecompiled). */
STATIC void FUN_8c0258ba(void)
{
    switch (var_8c227d9c) {
    case 5:
        var_busState_8c1bb9d0.posX_0x2fc = var_8c227e00.x;
        var_busState_8c1bb9d0.posY_0x300 = var_busState_8c1bb9d0.posY_0x0f8 + var_8c227e00.y;
        var_busState_8c1bb9d0.posZ_0x304 = var_8c227e00.z;
        break;
    case 6:
        njCalcPoint((NJS_MATRIX *)&var_busState_8c1bb9d0.field_0x084,
                    &var_8c227e00,
                    (NJS_POINT3 *)&var_busState_8c1bb9d0.posX_0x2fc);
        break;
    default:
        break;
    }
}

/* Called by BusTask_8c022bdc (022bdc) with no arguments each frame during
 * demo playback (var_playMode_8c1bb8d0 == 2); updates the camera from the
 * recorded demo instead of live input. */
void DemoUpdateCamera_8c025906(void)
{
    njInitCamera(&var_8c1bb904);
    njSetCameraAngle(&var_8c1bb904, 10194);
    njSetCameraDepth(&var_8c1bb904, -1.0f, -300.0f);

    if (var_8c227d9c == 7) {
        njCalcPoint((NJS_MATRIX *)&var_busState_8c1bb9d0.field_0x084,
                    &var_8c227e00,
                    (NJS_POINT3 *)&var_busState_8c1bb9d0.posX_0x2fc);
    }

    if (var_8c227d9c == 5 || var_8c227d9c == 6 || var_8c227d9c == 7) {
        njTranslateCameraPosition(&var_8c1bb904,
                                   var_busState_8c1bb9d0.posX_0x2fc,
                                   var_busState_8c1bb9d0.posY_0x300,
                                   var_busState_8c1bb9d0.posZ_0x304);
        njPointCameraInterest(&var_8c1bb904,
                               var_busState_8c1bb9d0.posX_0x0f4,
                               var_busState_8c1bb9d0.posY_0x0f8,
                               var_busState_8c1bb9d0.posZ_0x0fc);
        var_busState_8c1bb9d0.field_0x308 =
            var_busState_8c1bb9d0.posX_0x2fc - var_busState_8c1bb9d0.posX_0x0f4;
        var_busState_8c1bb9d0.field_0x310 =
            var_busState_8c1bb9d0.posZ_0x304 - var_busState_8c1bb9d0.posZ_0x0fc;
    }
}

/* TaskAction for the "next stop" textbox, armed by FUN_8c025af4. Its trigger
 * is the top byte of var_8c1bbd80's raw bits -- a stop id set elsewhere,
 * packed the same way as the byte fields documented on
 * var_scenePresetIds_8c1bbd8c -- read here regardless of var_8c1bbd80's
 * NJS_POINT3 type.
 *
 * Phase 0 watches for that id to change (or var_8c227e10 to re-arm it):
 * looks up the matching StopRecord in var_8c227e0c, switches the demo
 * camera into that record's state (var_8c227d9c = kind+5), copies its
 * position into var_8c227e00, opens (or clears) the message box for its
 * name, repositions the bus draw point via FUN_8c0258ba, and moves to phase
 * 1. Phase 1 just waits for the id to drop back to 0 to return to phase 0.
 * Every call then advances the textbox's reveal counter and reschedules
 * itself for the next fade-layer-0 callback. */
STATIC void stopTextboxTask_8c0259e8(Task *task, void *stateArg)
{
    StopTextboxState *state = (StopTextboxState *)stateArg;
    Uint32 marker;

    marker = *(Uint32 *)&var_8c1bbd80 & 0xFF000000;

    switch (state->phase_0x00) {
    case 0:
        if (marker != 0 || var_8c227e10 != 0) {
            int stopId = (int)(marker >> 24);

            if (var_8c227e10 != 0) {
                stopId = var_8c227dd4;
                var_8c227dd4 = -1;
                var_8c227e10 = 0;
            }

            if (var_8c227dd4 != stopId) {
                StopRecord *rec = (StopRecord *)var_8c227e0c + stopId;

                var_8c227dd4 = stopId;
                var_8c227d9c = rec->kind_0x00 + 5;
                var_8c227e00 = rec->pos_0x04;

                if (rec->name_0x10[0] != 0) {
                    state->textboxHandle_0x08 = ObjectsSwapMessageBoxFor_8c02aefc(rec->name_0x10);
                    state->revealCount_0x04 = 0;
                } else {
                    state->textboxHandle_0x08 = 0;
                }

                FUN_8c0258ba();
                state->phase_0x00 = 1;
            }
        }
        break;

    case 1:
        if (marker == 0) {
            state->phase_0x00 = 0;
        }
        break;
    }

    if (state->textboxHandle_0x08 != 0) {
        state->revealCount_0x04++;
        ObjectsMenuTextboxText_8c02af1c(state->revealCount_0x04 >> 1);
    }

    FadeCmdPushCall1_8c0223ea(0, (FadeCallback1)FUN_8c024bb8, 0);
}

/* Opens the "next stop" textbox and arms stopTextboxTask_8c0259e8. Picks
 * the current route's stop table (var_8c227e0c) among the three
 * per-route init_8c045674/init_8c045b60/init_8c045ee4 tables. */
void FUN_8c025af4(void)
{
    Task *task;
    StopTextboxState *state;

    switch (var_route_8c18ad1c) {
    case ROUTE_SHINJUKU:
        var_8c227e0c = init_8c045674;
        break;
    case ROUTE_WANGAN:
        var_8c227e0c = init_8c045b60;
        break;
    case ROUTE_OME:
        var_8c227e0c = init_8c045ee4;
        break;
    }

    TaskPush_8c014ae8(var_tasks_8c1ba5e8, &stopTextboxTask_8c0259e8, &task, (void **)&state, 0xc);
    state->phase_0x00 = 0;

    ObjectsOpenTextbox_8c02ae3e(0x20, 0x180, -1.0f, 0x023E, 0x40, 0, 0, -1);
    var_8c227e10 = 1;
}

