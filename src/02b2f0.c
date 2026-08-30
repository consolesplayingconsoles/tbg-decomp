/* @unit DriveMsg */

#include <shinobi.h>
#include "includes.h" /* STATIC */

#include "02b2f0.h"
#include "014f54_text.h"        /* TxtDrawSprite_8c014f54 */
#include "sectionB.h"           /* var_driveMsgQueue_8c228564, DriveMsgSlot, ... */

/* ====================
 * Functions
 * ====================
 */

/* Draws `count` 32x32 glyph quads in a row, starting at (x, y) and
 * advancing x by 32.0 each, sourcing each glyph's UV from a 16x16 cell
 * atlas: `ids[i]`'s low nibble picks the column, high nibble the row. */
STATIC void drawMsgGlyphRow_8c02b2f0(Uint32 *ids, int count, float x, float y)
{
    NJS_QUAD_TEXTURE q;
    int i;

    q.y1 = y;
    q.y2 = y + 32.0f;

    for (i = 0; i < count; i++) {
        Uint32 id = ids[i];
        float col, row;

        q.x1 = x;
        x += 32.0f;
        q.x2 = x;

        col = (float)(id & 0xf);
        q.u1 = col / 16.0f;
        q.u2 = q.u1 + 0.0625f;

        row = (float)(id & 0xfffffff0);
        q.v1 = row / 256.0f;
        q.v2 = q.v1 + 0.0625f;

        njDrawQuadTexture(&q, 0.82644623f);
    }
}

/* FadeCallback1 for the drive-message HUD banner -- see 02b2f0.h. */
void DriveMsgDraw_8c02b388(int unused)
{
    int i;
    float y;

    if (var_8c2285c8 != 0) {
        /* Some other UI element temporarily owns the screen: draw its mark
         * sprite instead. Original bug, preserved: passes
         * &var_markTexlist_8c1bc418 -- the variable's own address, not its
         * NJS_TEXLIST* value -- as the ResourceGroup*, unlike njSetTexture's
         * call below which correctly dereferences it. */
        TxtDrawSprite_8c014f54((ResourceGroup *)&var_markTexlist_8c1bc418,
                                0x78, 0.0f, 0.0f, -1.16f);
        return;
    }

    njSetTexture(var_markTexlist_8c1bc418);
    njQuadTextureStart(1);
    njSetQuadTextureG(0x0a8d, 0xffffffff);

    y = 192.0f;
    for (i = 0; i < 4; i++) {
        DriveMsgSlot *slot = &var_driveMsgQueue_8c228564[i];
        if (slot->holdFrames != 0) {
            drawMsgGlyphRow_8c02b2f0((Uint32 *)slot->ids, slot->glyphCount_0x0c,
                                     slot->duration, y);
        }
        y -= 32.0f;
    }

    njQuadTextureEnd();
}
