#include "shinobi.h"

#ifndef _02D19C_H_
#define _02D19C_H_

/* Minimal declarations for 02d19c (still asm) -- only the symbols FUN_8c02d968
 * (02d968) actually references. 02d19c itself is not decompiled yet. */

void FUN_8c02d644();
void FUN_8c02d8f0();
void FUN_8c02d21c();
void task_8c02d5ca();
void FUN_8c02d46c();

/* Per-scripted-stop schedule table, 31 {int, float} entries indexed by the
 * var_8c228718 slot index. */
typedef struct {
    int field_0x00;
    float field_0x04;
} InitEntry_8c04c3e4;
extern InitEntry_8c04c3e4 init_8c04c3e4[31];

#endif /* _02D19C_H_ */
