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

/* One 20-byte entry of the per-route stop tables (init_stopsShinjuku_8c045674/
 * init_stopsWangan_8c045b60/init_stopsOme_8c045ee4): {kind, pos, name}. */
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

STATIC const Uint8 const_stopNameNakanoBashi_8c039f78[] = {
    0x92, 0x86, 0x83, 0x6D, 0x8B, 0xB4, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameNone_8c039f80[] = {
    0x00, 0x00, 0x00, 0x00,
};

/* "\x88\xea\x83\x6D\x8b\xb4" -- SHIFT-JIS place name (dump_src_data.py
 * doesn't decode live .SDATA directives, only commented-out ones). */
STATIC const Uint8 const_stopNameIchinoBashi_8c039f84[] = {
    0x88, 0xEA, 0x83, 0x6D, 0x8B, 0xB4, 0x00, 0x00,
};

/* "\x94\xd1\x91\x71\x95\xd0\x92\xac" -- SHIFT-JIS place name (same
 * dump_src_data.py .SDATA limitation as const_stopNameIchinoBashi_8c039f84). */
STATIC const Uint8 const_stopNameIikuraKatamachi_8c039f8c[] = {
    0x94, 0xD1, 0x91, 0x71, 0x95, 0xD0, 0x92, 0xAC, 0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameRoppongiKousaten_8c039f98[] = {
    0x98, 0x5A, 0x96, 0x7B, 0x96, 0xD8, 0x8C, 0xF0,
    0x8D, 0xB7, 0x93, 0x5F, 0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameRoppongi_8c039fa8[] = {
    0x98, 0x5A, 0x96, 0x7B, 0x96, 0xD8, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameBoueichouMae_8c039fb0[] = {
    0x96, 0x68, 0x89, 0x71, 0x92, 0xA1, 0x91, 0x4F,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameAkasaka8Chome_8c039fbc[] = {
    0x90, 0xD4, 0x8D, 0xE2, 0x94, 0xAA, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameMinamiAoyama1Chome_8c039fc8[] = {
    0x93, 0xEC, 0x90, 0xC2, 0x8E, 0x52, 0x88, 0xEA,
    0x92, 0x9A, 0x96, 0xDA, 0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameAoyama1Chome_8c039fd8[] = {
    0x90, 0xC2, 0x8E, 0x52, 0x88, 0xEA, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameAoyama1ChomeKousaten_8c039fe4[] = {
    0x90, 0xC2, 0x8E, 0x52, 0x88, 0xEA, 0x92, 0x9A,
    0x96, 0xDA, 0x8C, 0xF0, 0x8D, 0xB7, 0x93, 0x5F,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameAkasakaGoyouchi_8c039ff8[] = {
    0x90, 0xD4, 0x8D, 0xE2, 0x8C, 0xE4, 0x97, 0x70,
    0x92, 0x6E, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameAoyamaToeiApartMae_8c03a004[] = {
    0x90, 0xC2, 0x8E, 0x52, 0x93, 0x73, 0x89, 0x63,
    0x83, 0x41, 0x83, 0x70, 0x81, 0x5B, 0x83, 0x67,
    0x91, 0x4F, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameMeijiKinenkan_8c03a018[] = {
    0x96, 0xBE, 0x8E, 0xA1, 0x8B, 0x4C, 0x94, 0x4F,
    0x8A, 0xD9, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameJinguuGaien_8c03a024[] = {
    0x90, 0x5F, 0x8B, 0x7B, 0x8A, 0x4F, 0x89, 0x91,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameShinanomachiEki_8c03a030[] = {
    0x90, 0x4D, 0x94, 0x5A, 0x92, 0xAC, 0x89, 0x77,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameShinanomachiEkiMae_8c03a03c[] = {
    0x90, 0x4D, 0x94, 0x5A, 0x92, 0xAC, 0x89, 0x77,
    0x91, 0x4F, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameYotsuya3Chome_8c03a048[] = {
    0x8E, 0x6C, 0x92, 0x4A, 0x8E, 0x4F, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameYotsuya4Chome_8c03a054[] = {
    0x8E, 0x6C, 0x92, 0x4A, 0x8E, 0x6C, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameShinjuku1Chome_8c03a060[] = {
    0x90, 0x56, 0x8F, 0x68, 0x88, 0xEA, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameShinjuku2Chome_8c03a06c[] = {
    0x90, 0x56, 0x8F, 0x68, 0x93, 0xF1, 0x92, 0x9A,
    0x96, 0xDA, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameShinjukuDoori_8c03a078[] = {
    0x90, 0x56, 0x8F, 0x68, 0x92, 0xCA, 0x82, 0xE8,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameShiokazeKouenIriguchi_8c03a084[] = {
    0x92, 0xAA, 0x95, 0x97, 0x8C, 0xF6, 0x89, 0x80,
    0x93, 0xFC, 0x8C, 0xFB, 0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameDaibaEkiMae_8c03a094[] = {
    0x91, 0xE4, 0x8F, 0xEA, 0x89, 0x77, 0x91, 0x4F,
    0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameFujiTerebiMae_8c03a0a0[] = {
    0x83, 0x74, 0x83, 0x57, 0x83, 0x65, 0x83, 0x8C,
    0x83, 0x72, 0x91, 0x4F, 0x00, 0x00, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameOdaibaKaihinKouenEkiMae_8c03a0b0[] = {
    0x82, 0xA8, 0x91, 0xE4, 0x8F, 0xEA, 0x8A, 0x43,
    0x95, 0x6C, 0x8C, 0xF6, 0x89, 0x80, 0x89, 0x77,
    0x91, 0x4F, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameHigashiOmeEkiMae_8c03a0c4[] = {
    0x93, 0x8C, 0x90, 0xC2, 0x94, 0x7E, 0x89, 0x77,
    0x91, 0x4F, 0x00, 0x00,
};

STATIC const Uint8 const_stopNameOmeShiyakushoMae_8c03a0d0[] = {
    0x90, 0xC2, 0x94, 0x7E, 0x8E, 0x73, 0x96, 0xF0,
    0x8F, 0x8A, 0x91, 0x4F, 0x00, 0x00, 0x00, 0x00,
};

/* Per-route stop-announcement tables, picked by var_route_8c18ad1c
 * (FUN_8c025af4): init_stopsShinjuku_8c045674 = ROUTE_SHINJUKU,
 * init_stopsWangan_8c045b60 = ROUTE_WANGAN, init_stopsOme_8c045ee4 =
 * ROUTE_OME. The original .src marks two internal boundaries
 * (init_8c046000/init_8c04608a) that don't land on a record boundary
 * (mid-record split, not real symbols -- nothing else in the game
 * references them), so they're folded into init_stopsOme_8c045ee4 here. */
STATIC StopRecord init_stopsShinjuku_8c045674[] = {
    {1, {4.0f, 0.5f, -4.0f}, (char *)const_stopNameNakanoBashi_8c039f78},
    {0, {4138.0f, 13.0f, 4452.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {3943.0f, 19.0f, 4413.0f}, (char *)const_stopNameIchinoBashi_8c039f84},
    {0, {3924.0f, 4.0f, 4298.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {3948.0f, 0.30000001192092896f, 3718.0f}, (char *)const_stopNameIikuraKatamachi_8c039f8c},
    {2, {-2.0f, 1.0f, -10.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {-2.0f, 1.0f, 6.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {3.0f, 0.800000011920929f, 10.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-4.0f, 2.5f, -10.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {3.0f, 2.5f, -25.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {3.0f, 2.5f, -25.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {7.5f, 1.7999999523162842f, 0.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {11.0f, 1.7999999523162842f, 0.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {3474.0f, 1.0f, 3511.0f}, (char *)const_stopNameRoppongiKousaten_8c039f98},
    {0, {3438.0f, 40.0f, 3417.0f}, (char *)const_stopNameRoppongi_8c039fa8},
    {2, {-3.0f, 2.0f, -6.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {-6.300000190734863f, 2.0f, -6.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {3154.0f, 4.800000190734863f, 3185.0f}, (char *)const_stopNameBoueichouMae_8c039fb0},
    {2, {-5.0f, 3.0f, 9.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {-2.5f, 3.0f, -3.0f}, (char *)const_stopNameAkasaka8Chome_8c039fbc},
    {2, {-4.0f, 15.5f, 20.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2924.699951171875f, 5.0f, 2583.0f}, (char *)const_stopNameMinamiAoyama1Chome_8c039fc8},
    {2, {-2.0f, 0.699999988079071f, 8.0f}, (char *)const_stopNameAoyama1Chome_8c039fd8},
    {0, {2764.0f, 7.0f, 2481.0f}, (char *)const_stopNameAoyama1ChomeKousaten_8c039fe4},
    {2, {30.0f, 0.5f, 0.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {30.0f, 0.5f, 0.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2607.0f, 1.0f, 2244.0f}, (char *)const_stopNameAkasakaGoyouchi_8c039ff8},
    {0, {2565.0f, 0.5f, 2165.0f}, (char *)const_stopNameAoyamaToeiApartMae_8c03a004},
    {2, {-5.0f, 0.5f, -3.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {-5.0f, 0.5f, -3.0f}, (char *)const_stopNameMeijiKinenkan_8c03a018},
    {0, {2400.0f, 6.0f, 1707.0f}, (char *)const_stopNameJinguuGaien_8c03a024},
    {2, {-15.0f, 5.5f, -7.0f}, (char *)const_stopNameShinanomachiEki_8c03a030},
    {1, {-15.0f, 5.5f, -7.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-6.0f, 1.0f, 0.0f}, (char *)const_stopNameShinanomachiEkiMae_8c03a03c},
    {2, {-6.0f, 1.0f, -5.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {-5.0f, 1.0f, -5.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-5.0f, 1.0f, 5.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2416.0f, 35.0f, 770.0f}, (char *)const_stopNameYotsuya3Chome_8c03a048},
    {2, {0.0f, 7.0f, 25.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {0.0f, 7.0f, 25.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2047.0f, 0.5f, 833.0f}, (char *)const_stopNameYotsuya4Chome_8c03a054},
    {2, {3.0f, 0.5f, 7.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-3.0f, 0.5f, 7.0f}, (char *)const_stopNameShinjuku1Chome_8c03a060},
    {0, {1619.0f, 6.0f, 734.0f}, (char *)const_stopNameShinjuku2Chome_8c03a06c},
    {0, {1448.0f, 7.0f, 694.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {0.0f, 2.5f, -20.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {1171.0f, 1.0f, 524.0f}, (char *)const_stopNameShinjukuDoori_8c03a078},
    {0, {1171.0f, 1.0f, 524.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {1171.0f, 1.0f, 524.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {1024.0f, 1.0f, 465.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {1047.0f, 1.0f, 479.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {-3.0f, 0.800000011920929f, -3.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {947.0f, 5.0f, 398.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-4.0f, 5.0f, -15.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {-4.0f, 5.0f, -15.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {658.0f, 5.0f, 300.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {658.0f, 5.0f, 300.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {652.0f, 1.5f, 210.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {584.0f, 0.0f, 171.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-4.0f, 0.5f, 5.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {0.0f, 2.5f, 10.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {558.0f, 10.0f, 318.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {447.0f, 0.0f, 352.0f}, (char *)const_stopNameNone_8c039f80},
};

STATIC StopRecord init_stopsWangan_8c045b60[] = {
    {0, {3792.0f, 1.2000000476837158f, 2893.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {3947.0f, 13.0f, 2850.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-3.0f, 3.0f, -8.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {3.0f, 3.0f, -8.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {4978.0f, 8.0f, 3180.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {4978.0f, 6.0f, 3180.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {3.0f, 0.5f, -7.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {4138.0f, 13.0f, 4452.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {3.0f, 13.0f, -5.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-3.0f, 13.0f, -5.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {3598.0f, 2.0f, 3434.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {5.0f, 4.0f, -20.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-3.0f, 0.30000001192092896f, 2.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {3202.0f, 1.5f, 3670.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-3.0f, 2.5f, 0.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2660.0f, 1.0f, 4071.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {10.0f, 1.5f, 0.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-3.0f, 0.5f, 10.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {-3.0f, 0.5f, 10.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2742.0f, 31.5f, 4706.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {14.5f, 1.5f, 13.0f}, (char *)const_stopNameShiokazeKouenIriguchi_8c03a084},
    {1, {14.5f, 1.5f, 13.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {1898.0f, 2.0f, 3789.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2100.0f, 25.0f, 3770.0f}, (char *)const_stopNameDaibaEkiMae_8c03a094},
    {0, {2116.0f, 0.5f, 3780.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2097.0f, 0.800000011920929f, 3654.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2348.0f, 12.0f, 3543.0f}, (char *)const_stopNameFujiTerebiMae_8c03a0a0},
    {2, {1.0f, 1.0f, -8.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {1.0f, 1.0f, -8.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {8.0f, 1.5f, -11.0f}, (char *)const_stopNameOdaibaKaihinKouenEkiMae_8c03a0b0},
    {1, {8.0f, 1.5f, -11.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {8.0f, 1.5f, 0.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {8.0f, 1.5f, 0.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2779.0f, 13.0f, 3196.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2779.0f, 9.800000190734863f, 3196.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-2.0999999046325684f, 0.5f, 7.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2625.89990234375f, 9.699999809265137f, 2858.60009765625f}, (char *)const_stopNameNone_8c039f80},
    {0, {1138.0f, 0.5f, 549.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-3.0f, 0.5f, 6.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {1113.0f, 0.5f, 462.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {0.0f, 16.5f, 40.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {970.0f, 0.5f, 345.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {3.0f, 1.5f, -20.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {3.0f, 1.5f, -20.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {692.0f, 1.5f, 293.0f}, (char *)const_stopNameNone_8c039f80},
};

STATIC StopRecord init_stopsOme_8c045ee4[] = {
    {0, {6992.0f, 0.699999988079071f, 6210.2001953125f}, (char *)const_stopNameHigashiOmeEkiMae_8c03a0c4},
    {0, {7007.0f, 0.5f, 6225.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-3.0f, 0.699999988079071f, 8.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {7190.0f, 6.400000095367432f, 6400.5f}, (char *)const_stopNameNone_8c039f80},
    {0, {7190.0f, 6.400000095367432f, 6400.5f}, (char *)const_stopNameOmeShiyakushoMae_8c03a0d0},
    {2, {-3.0f, 0.699999988079071f, -10.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {-3.0f, 0.699999988079071f, -10.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {7262.0f, 0.5f, 6169.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-3.0f, 1.5f, 3.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {5.0f, 1.5f, -10.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {5.0f, 1.5f, -10.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {0.0f, 3.0f, 10.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {6854.0f, 2.0f, 5824.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {6889.0f, 18.0f, 5736.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2707.0f, -1.0f, 1460.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {-4.0f, 1.0f, -18.0f}, (char *)const_stopNameNone_8c039f80},
    {1, {-4.0f, 1.0f, -18.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {3.0f, 0.5f, -3.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2283.0f, 4.0f, 1286.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2225.0f, 35.0f, 1169.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {2135.0f, 8.0f, 1107.0f}, (char *)const_stopNameNone_8c039f80},
    {2, {0.0f, 4.0f, 10.0f}, (char *)const_stopNameNone_8c039f80},
    {0, {1992.0f, 12.0f, 797.0f}, (char *)const_stopNameNone_8c039f80},
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
 * per-route init_stopsShinjuku_8c045674/init_stopsWangan_8c045b60/
 * init_stopsOme_8c045ee4 tables. */
void FUN_8c025af4(void)
{
    Task *task;
    StopTextboxState *state;

    switch (var_route_8c18ad1c) {
    case ROUTE_SHINJUKU:
        var_8c227e0c = (int *)init_stopsShinjuku_8c045674;
        break;
    case ROUTE_WANGAN:
        var_8c227e0c = (int *)init_stopsWangan_8c045b60;
        break;
    case ROUTE_OME:
        var_8c227e0c = (int *)init_stopsOme_8c045ee4;
        break;
    }

    TaskPush_8c014ae8(var_tasks_8c1ba5e8, &stopTextboxTask_8c0259e8, &task, (void **)&state, 0xc);
    state->phase_0x00 = 0;

    ObjectsOpenTextbox_8c02ae3e(0x20, 0x180, -1.0f, 0x023E, 0x40, 0, 0, -1);
    var_8c227e10 = 1;
}

