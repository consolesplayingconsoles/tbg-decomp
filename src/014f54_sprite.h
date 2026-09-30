/* 8c014f54 */
#ifndef _014F54_SPRITE_H
#define _014F54_SPRITE_H

#include <shinobi.h>
#include "015ab8_title.h"

typedef struct {
    int sprite_no_0x00;
    float x_0x04;
    float y_0x08;
} ResourceGroupSpriteEntry;

void SpriteDraw_8c014f54(
    ResourceGroup *resource_group,
    int texture_id,
    float x,
    float y,
    float priority
);

#endif /* _014F54_SPRITE_H */
