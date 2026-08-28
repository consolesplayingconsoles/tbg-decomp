/* @unit VehParts */

#include <shinobi.h>

#include "02786c_vehicle_parts.h"

/* ====================
 * Functions
 * ====================
 */

/* Caches the vehicle model's articulated sub-objects into fixed slots of the
 * traffic entry, so per-frame code can pose them without re-walking the tree.
 * The first-level children also get NJD_EVAL_UNIT_ANG cleared, i.e. their
 * rotation starts being honoured. Which of the deeper slots get filled depends
 * on the entry's type code. */
void VehPartsBind_8c02786c(void *entry, Uint32 typeCode)
{
    Uint8 *e = (Uint8 *)entry;
    NJS_OBJECT *nj;
    NJS_OBJECT *node;

    *(Uint32 *)(e + 0x00) = typeCode;

    nj = *(NJS_OBJECT **)(e + 0x0c);

    node = nj->child;
    *(NJS_OBJECT **)(e + 0x18) = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    node = node->sibling;
    *(NJS_OBJECT **)(e + 0x1c) = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    node = node->sibling;
    *(NJS_OBJECT **)(e + 0x20) = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    node = node->sibling;
    *(NJS_OBJECT **)(e + 0x24) = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    if (typeCode == 0x14 || typeCode == 0x16 || typeCode == 0x0e || typeCode == 0x10) {
        node = node->sibling;
        *(NJS_OBJECT **)(e + 0x28) = node;
        node->evalflags &= ~NJD_EVAL_UNIT_ANG;
    }

    /* Second walk re-reads entry+0x0c rather than reusing the entry+0x18
     * cache above -- matches the original asm. */
    nj = *(NJS_OBJECT **)(e + 0x0c);
    node = nj->child->child;
    *(NJS_OBJECT **)(e + 0x2c) = node;

    node = node->sibling;
    *(NJS_OBJECT **)(e + 0x30) = node;

    node = node->sibling;
    *(NJS_OBJECT **)(e + 0x34) = node;

    node = node->sibling;
    *(NJS_OBJECT **)(e + 0x38) = node;

    node = node->sibling;
    *(NJS_OBJECT **)(e + 0x3c) = node;

    if (typeCode == 0x1a) {
        *(NJS_OBJECT **)(e + 0x58) = node->child;
        *(NJS_OBJECT **)(e + 0x5c) = node->child->sibling;
        *(NJS_OBJECT **)(e + 0x60) = node->child->sibling->sibling;
    }

    node = node->sibling;
    if (node != NULL) {
        if (typeCode == 0x1c || typeCode == 0x1e) {
            *(NJS_OBJECT **)(e + 0x40) = node;
        }

        node = node->sibling;
        if (node != NULL) {
            if (typeCode == 0x1a) {
                *(NJS_OBJECT **)(e + 0x48) = node;
            }

            if (typeCode == 0x14 || typeCode == 0x16) {
                *(NJS_OBJECT **)(e + 0x44) = node;
            }

            if (typeCode == 0x1a) {
                node = node->sibling->child;
                *(NJS_OBJECT **)(e + 0x4c) = node;

                node = node->sibling;
                *(NJS_OBJECT **)(e + 0x50) = node;

                *(NJS_OBJECT **)(e + 0x54) = node->sibling;
            }
        }
    }
}
