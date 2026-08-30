/* @unit VehParts */

#include <shinobi.h>

#include "02786c_vehicle_parts.h"
#include "026710_traffic.h" /* TrafficEntry */

/* ====================
 * Functions
 * ====================
 */

/* Caches the vehicle model's articulated sub-objects into fixed slots of the
 * traffic entry, so per-frame code can pose them without re-walking the tree.
 * The first-level children also get NJD_EVAL_UNIT_ANG cleared, i.e. their
 * rotation starts being honoured. Which of the deeper slots get filled depends
 * on the entry's type code.
 *
 * `entry` is TrafficEntry* for a CPU vehicle, but 023310_bus_init.c also
 * calls this with the player's BusState*, which shares the same 0x00-0x60
 * prefix. */
void VehPartsBind_8c02786c(TrafficEntry *entry, Uint32 typeCode)
{
    NJS_OBJECT *nj;
    NJS_OBJECT *node;

    entry->field_0x000 = typeCode;

    nj = entry->modelLarge_0x0c;

    node = nj->child;
    entry->field_0x018 = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    node = node->sibling;
    entry->field_0x01c = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    node = node->sibling;
    entry->field_0x020 = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    node = node->sibling;
    entry->field_0x024 = node;
    node->evalflags &= ~NJD_EVAL_UNIT_ANG;

    if (typeCode == 0x14 || typeCode == 0x16 || typeCode == 0x0e || typeCode == 0x10) {
        node = node->sibling;
        entry->field_0x028 = node;
        node->evalflags &= ~NJD_EVAL_UNIT_ANG;
    }

    /* Second walk re-reads entry->modelLarge_0x0c rather than reusing the
     * entry->field_0x018 cache above -- matches the original asm. */
    nj = entry->modelLarge_0x0c;
    node = nj->child->child;
    entry->field_0x02c = node;

    node = node->sibling;
    entry->field_0x030 = node;

    node = node->sibling;
    entry->field_0x034 = node;

    node = node->sibling;
    entry->field_0x038 = node;

    node = node->sibling;
    entry->field_0x03c = node;

    if (typeCode == 0x1a) {
        entry->field_0x058 = node->child;
        entry->field_0x05c = node->child->sibling;
        entry->field_0x060 = node->child->sibling->sibling;
    }

    node = node->sibling;
    if (node != NULL) {
        if (typeCode == 0x1c || typeCode == 0x1e) {
            entry->field_0x040 = node;
        }

        node = node->sibling;
        if (node != NULL) {
            if (typeCode == 0x1a) {
                entry->field_0x048 = node;
            }

            if (typeCode == 0x14 || typeCode == 0x16) {
                entry->field_0x044 = node;
            }

            if (typeCode == 0x1a) {
                node = node->sibling->child;
                entry->field_0x04c = node;

                node = node->sibling;
                entry->field_0x050 = node;

                entry->field_0x054 = node->sibling;
            }
        }
    }
}
