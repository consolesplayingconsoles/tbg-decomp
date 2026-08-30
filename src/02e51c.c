/* @unit: not stamped -- this TU groups two unrelated jobs (see 02e51c.h). */

#include <shinobi.h>
#include "includes.h" /* TWO_PI */

#include "02e51c.h"             /* FUN_8c02e51c etc, FUN_8c02f08a */
#include "026710_traffic.h"     /* TrafficEntry */
#include "sectionB.h"           /* var_tasks_8c1bac28, var_busState_8c1bb9d0, var_8c228b3c */

/* ====================
 * Type Declarations
 * ====================
 */

/* Not GroundVertex/GroundPoly (020914_ground_query.c) -- a distinct
 * attribute grid, indexed the same way but with different poly fields. */

typedef struct {
    float x, y, z;
} JunctionVertex;

/* One cell of the attribute grid: the polygons whose footprint touches it.
 * Same shape as GroundCell (020914_ground_query.c). */
typedef struct {
    int count_0x00;
    int *polyIds_0x04;
} JunctionCell;

/* 24-byte stride, same as GroundPoly, but attr/flag fields sit at different
 * offsets and there is no plane normal -- this grid is only ever used for
 * flat (x, z) containment tests, never height interpolation. field_0x0c and
 * field_0x14 are never read by any of this unit's functions; they are
 * copied out to the caller (through the pointer FUN_8c02e51c/FUN_8c02e69c
 * and friends return) verbatim alongside attr_0x08 and shapeFlag_0x10. */
typedef struct {
    int vertexCount_0x00;
    int *vertexIds_0x04;
    int attr_0x08;          /* returned to the caller as &attr_0x08 on a hit */
    int field_0x0c;
    int shapeFlag_0x10;     /* 0 = convex (cross-product test), else concave (angle-sum test) */
    int field_0x14;
} JunctionPoly;

typedef struct {
    int cellsX_0x00;
    int cellsZ_0x04;
    int field_0x08;
    int field_0x0c;
    JunctionCell *cells_0x10;
    JunctionPoly *polys_0x14;
    JunctionVertex *verts_0x18;
} JunctionGrid;

/* The result struct written by all four query functions below. Unlike
 * GroundQueryResult (020914_ground_query.h), there is no leading attr field
 * here -- the matched polygon's attr lives at &polys[*slot].attr_0x08
 * instead, which is what these functions return. count is the hit/miss
 * flag (0 = miss) and, on a hit, gates the track-vs-full-search branch on
 * the next call. */
typedef struct {
    int *slot;
    int *vertexIds;
    int count;
} JunctionQueryResult;

#define CELL_SIZE 150.0f

/* How far apart a candidate polygon's first-vertex height may be from the
 * query point's y and still be considered a match -- the *AtHeight variants'
 * filter, same tolerance as GroundProbeFindPolygonAtHeight_8c020fe4's. */
#define HEIGHT_TOLERANCE 20.0f

/* ====================
 * Functions
 * ====================
 */

/* Looks up the road junction under world point (x, z) -- y is unused -- in
 * the grid selected by var_8c228b3c, writing the match into *out and
 * returning &polys[slot].attr_0x08 on a hit, NULL on a miss.
 *
 * *out already holding a previous match (out->count != 0) re-tests that
 * polygon first and only falls back to a full cell search on a miss --
 * same track-then-search shape as GroundProbeTrackPolygon_8c020b6c
 * (020b6c_ground_probe.c), except this polygon set is only ever tested the
 * convex way (cross-product walk); there is no concave/angle-sum path here. */
void *FUN_8c02e51c(float x, float y, float z, void *outParam)
{
    JunctionQueryResult *out = (JunctionQueryResult *)outParam;
    JunctionGrid *grid = (JunctionGrid *)var_8c228b3c;
    JunctionCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        JunctionVertex *v;
        float pdx, pdz;
        int n = out->count;
        int hit;
        int j;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        for (j = 0; j < n; j++) {
            float dx, dz;

            v = &grid->verts_0x18[ids[j]];
            dx = v->x - x;
            dz = v->z - z;

            if (0.0f > pdx * dz - pdz * dx) {
                break;
            }

            pdx = dx;
            pdz = dz;
        }

        hit = (j == n);

        if (hit) {
            out->slot = slot;
            out->vertexIds = grid->polys_0x14[*slot].vertexIds_0x04;
            out->count = n;
            return &grid->polys_0x14[*slot].attr_0x08;
        }
    }

    cx = (int)(x / CELL_SIZE);
    if (cx >= grid->cellsX_0x00) {
        cx = grid->cellsX_0x00 - 1;
    }

    cz = (int)(z / CELL_SIZE);
    if (cz >= grid->cellsZ_0x04) {
        cz = grid->cellsZ_0x04 - 1;
    }

    cell = &grid->cells_0x10[cz * grid->cellsX_0x00 + cx];

    for (i = 0; i < cell->count_0x00; i++) {
        int *slot = &cell->polyIds_0x04[i];
        int *ids = grid->polys_0x14[*slot].vertexIds_0x04;
        int n;
        JunctionVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        if (ids == out->vertexIds) {
            continue;
        }

        n = grid->polys_0x14[*slot].vertexCount_0x00;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        for (j = 0; j < n; j++) {
            float dx, dz;

            v = &grid->verts_0x18[ids[j]];
            dx = v->x - x;
            dz = v->z - z;

            if (0.0f > pdx * dz - pdz * dx) {
                break;
            }

            pdx = dx;
            pdz = dz;
        }

        hit = (j == n);

        if (hit) {
            out->slot = slot;
            out->vertexIds = ids;
            out->count = n;
            return &grid->polys_0x14[*slot].attr_0x08;
        }
    }

    out->vertexIds = 0;
    out->count = 0;
    return 0;
}

/* Height-filtered counterpart of FUN_8c02e51c: same (x, z) convex-only
 * containment test and the same track-then-search shape, but the full cell
 * search additionally rejects a candidate whose first vertex's y is more
 * than HEIGHT_TOLERANCE from the query point's y -- how an elevated road is
 * told apart from the surface street beneath it, same idea as
 * GroundProbeFindPolygonAtHeight_8c020fe4. The track (re-test) path does
 * not height-check -- same asymmetry as GroundProbeTrackPolygonAtHeight_8c021290. */
void *FUN_8c02eab4(float x, float y, float z, void *outParam)
{
    JunctionQueryResult *out = (JunctionQueryResult *)outParam;
    JunctionGrid *grid = (JunctionGrid *)var_8c228b3c;
    JunctionCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        JunctionVertex *v;
        float pdx, pdz;
        int n = out->count;
        int hit;
        int j;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        for (j = 0; j < n; j++) {
            float dx, dz;

            v = &grid->verts_0x18[ids[j]];
            dx = v->x - x;
            dz = v->z - z;

            if (0.0f > pdx * dz - pdz * dx) {
                break;
            }

            pdx = dx;
            pdz = dz;
        }

        hit = (j == n);

        if (hit) {
            out->slot = slot;
            out->vertexIds = grid->polys_0x14[*slot].vertexIds_0x04;
            out->count = n;
            return &grid->polys_0x14[*slot].attr_0x08;
        }
    }

    cx = (int)(x / CELL_SIZE);
    if (cx >= grid->cellsX_0x00) {
        cx = grid->cellsX_0x00 - 1;
    }

    cz = (int)(z / CELL_SIZE);
    if (cz >= grid->cellsZ_0x04) {
        cz = grid->cellsZ_0x04 - 1;
    }

    cell = &grid->cells_0x10[cz * grid->cellsX_0x00 + cx];

    for (i = 0; i < cell->count_0x00; i++) {
        int *slot = &cell->polyIds_0x04[i];
        int *ids = grid->polys_0x14[*slot].vertexIds_0x04;
        int n;
        JunctionVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        if (fabsf(grid->verts_0x18[ids[0]].y - y) > HEIGHT_TOLERANCE ||
            ids == out->vertexIds) {
            continue;
        }

        n = grid->polys_0x14[*slot].vertexCount_0x00;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        for (j = 0; j < n; j++) {
            float dx, dz;

            v = &grid->verts_0x18[ids[j]];
            dx = v->x - x;
            dz = v->z - z;

            if (0.0f > pdx * dz - pdz * dx) {
                break;
            }

            pdx = dx;
            pdz = dz;
        }

        hit = (j == n);

        if (hit) {
            out->slot = slot;
            out->vertexIds = ids;
            out->count = n;
            return &grid->polys_0x14[*slot].attr_0x08;
        }
    }

    out->vertexIds = 0;
    out->count = 0;
    return 0;
}

/* Same track-then-search shape as FUN_8c02e51c, but each candidate is now
 * tested one of two ways, chosen by shapeFlag_0x10 (0 = convex, else
 * concave), just like GroundQueryFindPolygon_8c020914's attr-sign split. */
void *FUN_8c02e69c(float x, float y, float z, void *outParam)
{
    JunctionQueryResult *out = (JunctionQueryResult *)outParam;
    JunctionGrid *grid = (JunctionGrid *)var_8c228b3c;
    JunctionCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        JunctionPoly *poly = &grid->polys_0x14[*slot];
        JunctionVertex *v;
        float pdx, pdz;
        int n = out->count;
        int hit;
        int j;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->shapeFlag_0x10 == 0) {
            for (j = 0; j < n; j++) {
                float dx, dz;

                v = &grid->verts_0x18[ids[j]];
                dx = v->x - x;
                dz = v->z - z;

                if (0.0f > pdx * dz - pdz * dx) {
                    break;
                }

                pdx = dx;
                pdz = dz;
            }

            hit = (j == n);
        } else {
            int acc = 0;

            for (j = 0; j < n; j++) {
                float dx, dz, cross, cosT;
                int angle;

                v = &grid->verts_0x18[ids[j]];
                dx = v->x - x;
                dz = v->z - z;

                cross = pdx * dz - pdz * dx;
                cosT = (pdx * dx + pdz * dz)
                     / (njSqrt(pdx * pdx + pdz * pdz) * njSqrt(dx * dx + dz * dz));

                if (cosT > 1.0f) {
                    cosT = 1.0f;
                } else if (-1.0f > cosT) {
                    cosT = -1.0f;
                }

                angle = (int)(acosf(cosT) * 65536.0f / TWO_PI);
                if (0.0f > cross) {
                    acc -= angle;
                } else {
                    acc += angle;
                }

                pdx = dx;
                pdz = dz;
            }

            hit = acc > 0x8000;
        }

        /* Unlike FUN_8c02e51c's track path, a track hit here does not
         * rewrite *out -- it already holds this same match, so the asm
         * jumps straight to computing the return address. */
        if (hit) {
            return &poly->attr_0x08;
        }
    }

    cx = (int)(x / CELL_SIZE);
    if (cx >= grid->cellsX_0x00) {
        cx = grid->cellsX_0x00 - 1;
    }

    cz = (int)(z / CELL_SIZE);
    if (cz >= grid->cellsZ_0x04) {
        cz = grid->cellsZ_0x04 - 1;
    }

    cell = &grid->cells_0x10[cz * grid->cellsX_0x00 + cx];

    for (i = 0; i < cell->count_0x00; i++) {
        int *slot = &cell->polyIds_0x04[i];
        JunctionPoly *poly = &grid->polys_0x14[*slot];
        int *ids = poly->vertexIds_0x04;
        int n;
        JunctionVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        if (ids == out->vertexIds) {
            continue;
        }

        n = poly->vertexCount_0x00;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->shapeFlag_0x10 == 0) {
            for (j = 0; j < n; j++) {
                float dx, dz;

                v = &grid->verts_0x18[ids[j]];
                dx = v->x - x;
                dz = v->z - z;

                if (0.0f > pdx * dz - pdz * dx) {
                    break;
                }

                pdx = dx;
                pdz = dz;
            }

            hit = (j == n);
        } else {
            int acc = 0;

            for (j = 0; j < n; j++) {
                float dx, dz, cross, cosT;
                int angle;

                v = &grid->verts_0x18[ids[j]];
                dx = v->x - x;
                dz = v->z - z;

                cross = pdx * dz - pdz * dx;
                cosT = (pdx * dx + pdz * dz)
                     / (njSqrt(pdx * pdx + pdz * pdz) * njSqrt(dx * dx + dz * dz));

                if (cosT > 1.0f) {
                    cosT = 1.0f;
                } else if (-1.0f > cosT) {
                    cosT = -1.0f;
                }

                angle = (int)(acosf(cosT) * 65536.0f / TWO_PI);
                if (0.0f > cross) {
                    acc -= angle;
                } else {
                    acc += angle;
                }

                pdx = dx;
                pdz = dz;
            }

            hit = acc > 0x8000;
        }

        if (hit) {
            out->slot = slot;
            out->vertexIds = poly->vertexIds_0x04;
            out->count = n;
            return &poly->attr_0x08;
        }
    }

    out->vertexIds = 0;
    out->count = 0;
    return 0;
}

/* AtHeight counterpart of FUN_8c02e69c: same convex/concave split by
 * shapeFlag_0x10, but the full cell search additionally rejects a candidate
 * whose first vertex's y is more than HEIGHT_TOLERANCE from the query
 * point's y, same as FUN_8c02eab4 is to FUN_8c02e51c. */
void *FUN_8c02ec50(float x, float y, float z, void *outParam)
{
    JunctionQueryResult *out = (JunctionQueryResult *)outParam;
    JunctionGrid *grid = (JunctionGrid *)var_8c228b3c;
    JunctionCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        JunctionPoly *poly = &grid->polys_0x14[*slot];
        JunctionVertex *v;
        float pdx, pdz;
        int n = out->count;
        int hit;
        int j;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->shapeFlag_0x10 == 0) {
            for (j = 0; j < n; j++) {
                float dx, dz;

                v = &grid->verts_0x18[ids[j]];
                dx = v->x - x;
                dz = v->z - z;

                if (0.0f > pdx * dz - pdz * dx) {
                    break;
                }

                pdx = dx;
                pdz = dz;
            }

            hit = (j == n);
        } else {
            int acc = 0;

            for (j = 0; j < n; j++) {
                float dx, dz, cross, cosT;
                int angle;

                v = &grid->verts_0x18[ids[j]];
                dx = v->x - x;
                dz = v->z - z;

                cross = pdx * dz - pdz * dx;
                cosT = (pdx * dx + pdz * dz)
                     / (njSqrt(pdx * pdx + pdz * pdz) * njSqrt(dx * dx + dz * dz));

                if (cosT > 1.0f) {
                    cosT = 1.0f;
                } else if (-1.0f > cosT) {
                    cosT = -1.0f;
                }

                angle = (int)(acosf(cosT) * 65536.0f / TWO_PI);
                if (0.0f > cross) {
                    acc -= angle;
                } else {
                    acc += angle;
                }

                pdx = dx;
                pdz = dz;
            }

            hit = acc > 0x8000;
        }

        /* Unlike FUN_8c02e51c's track path, a track hit here does not
         * rewrite *out -- it already holds this same match, so the asm
         * jumps straight to computing the return address. */
        if (hit) {
            return &poly->attr_0x08;
        }
    }

    cx = (int)(x / CELL_SIZE);
    if (cx >= grid->cellsX_0x00) {
        cx = grid->cellsX_0x00 - 1;
    }

    cz = (int)(z / CELL_SIZE);
    if (cz >= grid->cellsZ_0x04) {
        cz = grid->cellsZ_0x04 - 1;
    }

    cell = &grid->cells_0x10[cz * grid->cellsX_0x00 + cx];

    for (i = 0; i < cell->count_0x00; i++) {
        int *slot = &cell->polyIds_0x04[i];
        JunctionPoly *poly = &grid->polys_0x14[*slot];
        int *ids = poly->vertexIds_0x04;
        int n;
        JunctionVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        if (fabsf(grid->verts_0x18[ids[0]].y - y) > HEIGHT_TOLERANCE ||
            ids == out->vertexIds) {
            continue;
        }

        n = poly->vertexCount_0x00;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->shapeFlag_0x10 == 0) {
            for (j = 0; j < n; j++) {
                float dx, dz;

                v = &grid->verts_0x18[ids[j]];
                dx = v->x - x;
                dz = v->z - z;

                if (0.0f > pdx * dz - pdz * dx) {
                    break;
                }

                pdx = dx;
                pdz = dz;
            }

            hit = (j == n);
        } else {
            int acc = 0;

            for (j = 0; j < n; j++) {
                float dx, dz, cross, cosT;
                int angle;

                v = &grid->verts_0x18[ids[j]];
                dx = v->x - x;
                dz = v->z - z;

                cross = pdx * dz - pdz * dx;
                cosT = (pdx * dx + pdz * dz)
                     / (njSqrt(pdx * pdx + pdz * pdz) * njSqrt(dx * dx + dz * dz));

                if (cosT > 1.0f) {
                    cosT = 1.0f;
                } else if (-1.0f > cosT) {
                    cosT = -1.0f;
                }

                angle = (int)(acosf(cosT) * 65536.0f / TWO_PI);
                if (0.0f > cross) {
                    acc -= angle;
                } else {
                    acc += angle;
                }

                pdx = dx;
                pdz = dz;
            }

            hit = acc > 0x8000;
        }

        if (hit) {
            out->slot = slot;
            out->vertexIds = poly->vertexIds_0x04;
            out->count = n;
            return &poly->attr_0x08;
        }
    }

    out->vertexIds = 0;
    out->count = 0;
    return 0;
}

/* Scans var_tasks_8c1bac28 for a traffic entry (task state, other than
 * self's) whose field_0x410 equals value; falls back to the player bus's
 * own BusState.field_0x3a0 when no other task matches. Returns nonzero on
 * either match.
 *
 * BusState.field_0x3a0 is imported by the original asm as a standalone
 * symbol (var_8c1bbd70), but that address is inside var_busState_8c1bb9d0 --
 * a linker coincidence, not a separate global. */
int FUN_8c02f08a(Task *self, int value)
{
    Task *t;

    for (t = var_tasks_8c1bac28; t->action != NULL; t++) {
        if ((void *)t->action != (void *)-1 && t != self &&
            ((TrafficEntry *)t->state)->field_0x410 == value) {
            return 1;
        }
    }

    return var_busState_8c1bb9d0.field_0x3a0 == value;
}
