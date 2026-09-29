/* @unit AttrQuery */

#include <shinobi.h>
#include "includes.h" /* TWO_PI */

#include "02e51c_attr_query.h"
#include "026710_traffic.h"  /* TrafficEntry */
#include "1ba1c8_globals.h"

/* ====================
 * Type Declarations
 * ====================
 */

/* Not GroundVertex/GroundPoly (020914_ground_query.c) -- a distinct
 * attribute grid, indexed the same way but with different poly fields. */

typedef struct {
    float x, y, z;
} AttrVertex;

/* One cell of the attribute grid: the polygons whose footprint touches it.
 * Same shape as GroundCell (020914_ground_query.c). */
typedef struct {
    int count_0x00;
    int *polyIds_0x04;
} AttrCell;

/* 24-byte stride, same as GroundPoly, but attr/flag fields sit at different
 * offsets and there is no plane normal -- this grid is only ever used for
 * flat (x, z) containment tests, never height interpolation. */
typedef struct {
    int vertexCount_0x00;
    int *vertexIds_0x04;
    int attr_0x08;          /* returned to the caller as &attr_0x08 on a hit */
    int field_0x0c;         /* never read; copied out to the caller verbatim */
    int shapeFlag_0x10;     /* 0 = convex (cross-product test), else concave (angle-sum test) */
    int field_0x14;         /* never read; copied out to the caller verbatim */
} AttrPoly;

typedef struct {
    int cellsX_0x00;
    int cellsZ_0x04;
    int field_0x08;
    int field_0x0c;
    AttrCell *cells_0x10;
    AttrPoly *polys_0x14;
    AttrVertex *verts_0x18;
} AttrGrid;

/* The result struct written by all four query functions below. Unlike
 * GroundQueryResult (020914_ground_query.h), there is no leading attr field
 * here -- the matched polygon's attr lives at &polys[*slot].attr_0x08
 * instead, and that's what these functions return. count is the hit/miss
 * flag (0 = miss) and, on a hit, gates the track-vs-full-search branch on
 * the next call. */
typedef struct {
    int *slot;
    int *vertexIds;
    int count;
} AttrQueryResult;

#define CELL_SIZE 150.0f

/* How far apart a candidate polygon's first-vertex height may be from the
 * query point's y and still count as a match -- the *AtHeight variants'
 * filter, same tolerance as GroundProbeFindPolygonAtHeight_8c020fe4's. */
#define HEIGHT_TOLERANCE 20.0f

/* =======================
 * Non-initialized Globals
 * =======================
 */

void *var_activeAttrGrid_8c228b3c;


/* ====================
 * Functions
 * ====================
 */

/* Finds the attribute polygon containing world point (x, z) -- y is unused
 * -- in var_activeAttrGrid_8c228b3c, writing the match into *out and
 * returning &polys[slot].attr_0x08 on a hit, NULL on a miss.
 *
 * If *out already holds a previous match (out->count != 0), that polygon is
 * re-tested first, falling back to a full cell search only on a miss --
 * same track-then-search shape as GroundProbeTrackPolygon_8c020b6c
 * (020b6c_ground_probe.c), except this polygon set is only ever tested the
 * convex way (cross-product walk); there is no concave/angle-sum path here. */
void *AttrQueryFindConvexPolygon_8c02e51c(float x, float y, float z, void *outParam)
{
    AttrQueryResult *out = (AttrQueryResult *)outParam;
    AttrGrid *grid = (AttrGrid *)var_activeAttrGrid_8c228b3c;
    AttrCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        AttrVertex *v;
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
        AttrVertex *v;
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
    return NULL;
}

/* Height-filtered counterpart of AttrQueryFindConvexPolygon_8c02e51c: same
 * (x, z) convex-only containment test and track-then-search shape, but the full cell
 * search additionally rejects a candidate whose first vertex's y is more
 * than HEIGHT_TOLERANCE from the query point's y, so an elevated road can be
 * told apart from the surface street beneath it -- same idea as
 * GroundProbeFindPolygonAtHeight_8c020fe4. The track (re-test) path skips
 * the height check -- same asymmetry as GroundProbeTrackPolygonAtHeight_8c021290. */
void *AttrQueryFindConvexPolygonAtHeight_8c02eab4(float x, float y, float z, void *outParam)
{
    AttrQueryResult *out = (AttrQueryResult *)outParam;
    AttrGrid *grid = (AttrGrid *)var_activeAttrGrid_8c228b3c;
    AttrCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        AttrVertex *v;
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
        AttrVertex *v;
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
    return NULL;
}

/* Same track-then-search shape as AttrQueryFindConvexPolygon_8c02e51c, but
 * each candidate is tested one of two ways, chosen by shapeFlag_0x10 (0 = convex, else
 * concave), just like GroundQueryFindPolygon_8c020914's attr-sign split. */
void *AttrQueryFindPolygon_8c02e69c(float x, float y, float z, void *outParam)
{
    AttrQueryResult *out = (AttrQueryResult *)outParam;
    AttrGrid *grid = (AttrGrid *)var_activeAttrGrid_8c228b3c;
    AttrCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        AttrPoly *poly = &grid->polys_0x14[*slot];
        AttrVertex *v;
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

        /* Unlike AttrQueryFindConvexPolygon_8c02e51c's track path, a track
         * hit here does not
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
        AttrPoly *poly = &grid->polys_0x14[*slot];
        int *ids = poly->vertexIds_0x04;
        int n;
        AttrVertex *v;
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
    return NULL;
}

/* AtHeight counterpart of AttrQueryFindPolygon_8c02e69c: same convex/concave split by
 * shapeFlag_0x10, but the full cell search additionally rejects a candidate
 * whose first vertex's y is more than HEIGHT_TOLERANCE from the query
 * point's y -- AttrQueryFindConvexPolygonAtHeight_8c02eab4's relation to
 * AttrQueryFindConvexPolygon_8c02e51c, applied to this pair. */
void *AttrQueryFindPolygonAtHeight_8c02ec50(float x, float y, float z, void *outParam)
{
    AttrQueryResult *out = (AttrQueryResult *)outParam;
    AttrGrid *grid = (AttrGrid *)var_activeAttrGrid_8c228b3c;
    AttrCell *cell;
    int cx, cz;
    int i;

    if (out->count != 0) {
        int *ids = out->vertexIds;
        int *slot = out->slot;
        AttrPoly *poly = &grid->polys_0x14[*slot];
        AttrVertex *v;
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

        /* Unlike AttrQueryFindConvexPolygon_8c02e51c's track path, a track
         * hit here does not
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
        AttrPoly *poly = &grid->polys_0x14[*slot];
        int *ids = poly->vertexIds_0x04;
        int n;
        AttrVertex *v;
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
    return NULL;
}

/* Scans var_tasks_8c1bac28 for a traffic entry (task state, other than
 * self's) whose signalId_0x410 equals id, falling back to the player bus's
 * own BusState.fallbackTaskMatchId_0x3a0 when no other task matches. */
int AttrQueryRegionOccupied_8c02f08a(Task *self, int id)
{
    Task *t;

    for (t = var_tasks_8c1bac28; t->action != NULL; t++) {
        if ((void *)t->action != (void *)-1 && t != self &&
            ((TrafficEntry *)t->state)->signalId_0x410 == id) {
            return 1;
        }
    }

    return var_busState_8c1bb9d0.fallbackTaskMatchId_0x3a0 == id;
}
