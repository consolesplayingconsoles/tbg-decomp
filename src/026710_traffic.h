/* 8c026710 */
#ifndef _026710_TRAFFIC_H
#define _026710_TRAFFIC_H

#include <shinobi.h>

/* One record of a path block: a run of these terminated by length == 0.
 * TrafficAdvanceOnPath_8c026ca2 walks them to find the record containing the
 * entry's distance-along-path, then projects the world position from the
 * record's origin along its direction. */
typedef struct {
    float length_0x00;
    float x_0x04;
    float z_0x08;
    float dirX_0x0c;
    float dirZ_0x10;
} PathRecord;

/* One CPU-vehicle placement record from the course's *_MAC_CPU1.DAT, walked by
 * trafficUpdateTask_8c0275d4. A run of these is terminated by script == NULL;
 * TrafficRelocatePlacementTable_8c026da4 relocates `script` from a self-relative offset to a pointer at
 * load time. `progress` is the entry's start distance along its path. */
typedef struct {
    Uint16 typeCode_0x00;
    Uint16 threshold_0x02;
    Uint16 *script_0x04;
    float progress_0x08;
} TrafficPlacement;

/* The 0x514-byte task state spawnEntry_8c0272b8 allocates for every CPU
 * vehicle and fixed decoration. Fields are named only where this unit,
 * 02786c_vehicle_parts and 02e400_collision establish a meaning; the rest stay
 * field_/padding_ until the per-frame drivers FUN_8c025b98 (moving vehicle)
 * and FUN_8c02656a (decoration) are decompiled -- they own most of it. */
typedef struct {
    Uint32 field_0x000;
    NJS_TEXLIST *texlistLarge_0x04;
    NJS_TEXLIST *texlistSmall_0x08;
    NJS_OBJECT *modelLarge_0x0c;
    void *modelSmall_0x10;
    void *bodyModel_0x14;
    NJS_OBJECT *field_0x018;
    NJS_OBJECT *field_0x01c;
    NJS_OBJECT *field_0x020;
    NJS_OBJECT *field_0x024;
    NJS_OBJECT *field_0x028;
    NJS_OBJECT *field_0x02c;
    NJS_OBJECT *field_0x030;
    NJS_OBJECT *field_0x034;
    NJS_OBJECT *field_0x038;
    NJS_OBJECT *field_0x03c;
    NJS_OBJECT *field_0x040;
    NJS_OBJECT *field_0x044;
    NJS_OBJECT *field_0x048;
    NJS_OBJECT *field_0x04c;
    NJS_OBJECT *field_0x050;
    NJS_OBJECT *field_0x054;
    NJS_OBJECT *field_0x058;
    NJS_OBJECT *field_0x05c;
    NJS_OBJECT *field_0x060;
    Uint32 field_0x064;
    Uint32 field_0x068;
    Uint32 field_0x06c;
    Uint32 field_0x070;
    Uint32 field_0x074;
    Uint32 field_0x078;
    Uint32 field_0x07c;
    Uint32 field_0x080;
    NJS_MATRIX worldMatrix_0x84;
    float field_0x0c4;
    float field_0x0c8;
    float field_0x0cc;
    float field_0x0d0;
    float field_0x0d4;
    float field_0x0d8;
    float field_0x0dc;
    float field_0x0e0;
    float field_0x0e4;
    float field_0x0e8;
    float field_0x0ec;
    float field_0x0f0;
    float posX_0xf4;
    float posY_0xf8;
    float posZ_0xfc;
    float field_0x100;
    float field_0x104;
    float field_0x108;
    float field_0x10c;
    Uint32 field_0x110;
    float field_0x114;
    float field_0x118;
    float field_0x11c;
    float field_0x120;
    float field_0x124;
    float field_0x128;
    float field_0x12c;
    Uint8 padding_0x130[0x68];
    Uint32 field_0x198;
    Uint32 field_0x19c;
    Uint8 padding_0x1a0[0x8];
    Uint32 field_0x1a8;
    Uint32 field_0x1ac;
    Uint8 padding_0x1b0[0x8];
    Uint32 field_0x1b8;
    Uint32 field_0x1bc;
    Uint8 padding_0x1c0[0x8];
    Uint32 field_0x1c8;
    Uint32 field_0x1cc;
    Uint8 padding_0x1d0[0x6c];
    float width_0x23c;
    float length_0x240;
    float height_0x244;
    float halfHeight_0x248;
    float groundOffset_0x24c;
    Sint32 heading_0x250;
    Sint32 headingAlt_0x254;
    Uint32 field_0x258;
    Uint32 field_0x25c;
    Uint32 field_0x260;
    Uint32 field_0x264;
    Uint32 field_0x268;
    Sint32 field_0x26c;
    float field_0x270;
    float field_0x274;
    float field_0x278;
    float field_0x27c;
    Uint32 field_0x280;
    Uint32 field_0x284;
    Uint8 padding_0x288[0x8];
    float field_0x290;
    Uint8 padding_0x294[0x20];
    Uint32 field_0x2b4;
    PathRecord *pathRecord_0x2b8;
    float pathDistance_0x2bc;
    float pathDistanceCopy_0x2c0;
    float projectDistance_0x2c4;
    void *field_0x2c8;
    void *field_0x2cc;
    Uint32 field_0x2d0;
    Uint32 field_0x2d4;
    Uint32 field_0x2d8;
    Uint32 field_0x2dc;
    Sint32 variantIndex_0x2e0;
    Sint32 field_0x2e4;
    float field_0x2e8;
    float originJitterX_0x2ec;
    float originJitterZ_0x2f0;
    Sint32 field_0x2f4;
    Uint16 *scriptCursor_0x2f8;
    Uint16 *scriptBase_0x2fc;
    Sint32 field_0x300;
    /* inline array of per-path-block record pointers, indexed by
     * field_0x300; TrafficReadScriptArgs_8c026710 fills it from the
     * script's opcode-1 path ids and terminates it with -1 */
    PathRecord *resolvedArgs_0x304[64];
    Uint32 field_0x404;
    Uint32 field_0x408;
    Uint32 field_0x40c;
    Uint32 field_0x410;
    float field_0x414;
    float field_0x418;
    float field_0x41c;
    Uint32 field_0x420;
    Uint32 field_0x424;
    Uint32 field_0x428;
    Uint32 field_0x42c;
    Sint32 field_0x430;
    Uint32 field_0x434;
    Uint32 field_0x438;
    Uint8 padding_0x43c[0xc];
    Uint32 field_0x448;
    Uint32 field_0x44c;
    Uint32 field_0x450;
    Uint32 field_0x454;
    Uint32 field_0x458;
    Uint32 field_0x45c;
    Uint32 field_0x460;
    Uint32 field_0x464;
    Uint32 field_0x468;
    Uint32 field_0x46c;
    Uint32 field_0x470;
    Uint32 field_0x474;
    Uint32 field_0x478;
    Sint32 field_0x47c;
    Uint32 field_0x480;
    Sint32 field_0x484;
    Uint32 field_0x488;
    Sint32 field_0x48c;
    float field_0x490;
    Uint32 field_0x494;
    Uint32 field_0x498;
    Uint8 padding_0x49c[0x50];
    Sint32 field_0x4ec;
    Uint32 field_0x4f0;
    PathRecord *field_0x4f4;
    Uint32 field_0x4f8;
    float field_0x4fc;
    Uint32 field_0x500;
    Uint32 field_0x504;
    Uint32 field_0x508;
    Uint32 field_0x50c;
    Uint32 field_0x510;
} TrafficEntry;

void TrafficReadScriptArgs_8c026710(TrafficEntry *entry, Uint16 *script);
void TrafficUpdateHeading_8c026bc4(float unused, TrafficEntry *entry);
Sint32 TrafficAdvanceOnPath_8c026ca2(float unused, TrafficEntry *entry);
void TrafficRelocatePlacementTable_8c026da4(void *handle);
void TrafficMarkSignalIdsInUse_8c026dcc(int maxId);
float TrafficComputeBlockedSpeed_8c026eaa(TrafficEntry *entry, TrafficEntry *other);
void TrafficUpdateFrameFlags_8c026f7e(TrafficEntry *entry);
float TrafficRemainingPathDistance_8c026fb0(TrafficEntry *entry);
void TrafficSeekPathRecord_8c026fcc(TrafficEntry *entry, PathRecord *seg);
Sint32 TrafficRunEntryScript_8c027012(TrafficEntry *entry);
void TrafficInit_8c02769e(void);

#endif // _026710_TRAFFIC_H
