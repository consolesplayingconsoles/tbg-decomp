/* @unit GroundProbe */

#include <shinobi.h>
#include "includes.h" /* TWO_PI */
#include "020b6c_ground_probe.h"
#include "020914_ground_query.h"

/* ====================
 * Compiler Definitions
 * ====================
 */

#define CELL_SIZE 150.0f

/* Max gap between the probe's y and a candidate polygon's first vertex. */
#define HEIGHT_TOLERANCE 20.0f

/* ====================
 * Functions
 * ====================
 */

/* Per-frame form of GroundQueryFindPolygon_8c020914: *out already holds the polygon
 * match from a previous call. Re-tests that polygon first and only falls back to a
 * full cell search (skipping the just-failed polygon, so it isn't re-tested there)
 * when the point has left it.
 *
 * y is never read -- only ever clobbered as scratch space for the concave test's
 * running cos/angle value (a stack slot the compiler happened to alias to the
 * incoming parameter). Original behaviour. */
void GroundProbeTrackPolygon_8c020b6c(float x, float y, float z, GroundQueryResult *out)
{
    GroundGrid *grid = var_activeGroundGrid_8c2264d4;
    GroundPoly *poly;
    int *slot;
    int n;

    if (out->count_0x0c != 0) {
        int *ids = out->vertexIds_0x08;
        GroundVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        slot = out->polyIdSlot_0x04;
        poly = &grid->polys_0x14[*slot];
        n = out->count_0x0c;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->attr_0x14 >= 0) {
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
            out->attr_0x00 = poly->attr_0x14 & 0x7fffffff;
            out->polyIdSlot_0x04 = slot;
            out->vertexIds_0x08 = poly->vertexIds_0x04;
            out->count_0x0c = n;
            return;
        }
    }

    /* Full cell search, same algorithm as GroundQueryFindPolygon_8c020914, but
     * skipping the polygon just re-tested above (by comparing vertex-id pointers). */
    {
        GroundCell *cell;
        int cx, cz;
        int i;

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
            int *ids;
            GroundVertex *v;
            float pdx, pdz;
            int hit;
            int j;

            slot = &cell->polyIds_0x04[i];
            poly = &grid->polys_0x14[*slot];
            ids = poly->vertexIds_0x04;

            if (ids == out->vertexIds_0x08) {
                continue;
            }

            n = poly->vertexCount_0x00;

            v = &grid->verts_0x18[ids[n - 1]];
            pdx = v->x - x;
            pdz = v->z - z;

            if (poly->attr_0x14 >= 0) {
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
                out->attr_0x00 = poly->attr_0x14 & 0x7fffffff;
                out->polyIdSlot_0x04 = slot;
                out->vertexIds_0x08 = poly->vertexIds_0x04;
                out->count_0x0c = n;
                return;
            }
        }

        out->vertexIds_0x08 = 0;
        out->count_0x0c = 0;
    }
}

/* Every caller of GroundQueryFindPolygon_8c020914/GroundProbeTrackPolygon_8c020b6c
 * follows the lookup with this: snapping a decoration, pedestrian, or spawning
 * vehicle onto the matched polygon's surface. No-match (count_0x0c == 0) leaves
 * point untouched, so callers must check that first.
 *
 * The hit case solves the polygon's plane equation nx*(x-x0) + ny*(y-y0) +
 * nz*(z-z0) = 0 for y, using the polygon's first vertex as the plane point.
 * A vertical plane (ny == 0, e.g. a wall) can't be solved that way, so it just
 * takes the first vertex's height outright. */
void GroundProbeInterpolateHeight_8c020f7e(GroundQueryResult *result, float *point)
{
    GroundGrid *grid = var_activeGroundGrid_8c2264d4;
    GroundPoly *poly;
    GroundVertex *v0;

    if (result->count_0x0c == 0) {
        return;
    }

    poly = &grid->polys_0x14[*result->polyIdSlot_0x04];
    v0 = &grid->verts_0x18[*result->vertexIds_0x08];

    if (poly->normalY_0x0c == 0.0f) {
        point[1] = v0->y;
    } else {
        point[1] = (-(poly->normalX_0x08 * (point[0] - v0->x)) -
                    poly->normalZ_0x10 * (point[2] - v0->z)) / poly->normalY_0x0c +
                   v0->y;
    }
}

/* Same full cell search as GroundQueryFindPolygon_8c020914, except the cell size
 * comes from the grid itself (cellSizeX_0x08/cellSizeZ_0x0c) instead of the fixed
 * CELL_SIZE, and each candidate is additionally rejected when its first vertex's
 * height is too far from y -- this is how an elevated road is told apart from the
 * surface street beneath it. */
void GroundProbeFindPolygonAtHeight_8c020fe4(float x, float y, float z, GroundQueryResult *out)
{
    GroundGrid *grid = var_activeGroundGrid_8c2264d4;
    GroundCell *cell;
    int cx, cz;
    int i;

    cx = (int)(x / grid->cellSizeX_0x08);
    if (cx >= grid->cellsX_0x00) {
        cx = grid->cellsX_0x00 - 1;
    }

    cz = (int)(z / grid->cellSizeZ_0x0c);
    if (cz >= grid->cellsZ_0x04) {
        cz = grid->cellsZ_0x04 - 1;
    }

    /* Only the upper bound is clamped; a negative coordinate indexes before
     * the cell array. Original behaviour. */
    cell = &grid->cells_0x10[cz * grid->cellsX_0x00 + cx];

    for (i = 0; i < cell->count_0x00; i++) {
        int *slot = &cell->polyIds_0x04[i];
        GroundPoly *poly = &grid->polys_0x14[*slot];
        int n = poly->vertexCount_0x00;
        int *ids = poly->vertexIds_0x04;
        GroundVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        if (fabsf(grid->verts_0x18[ids[0]].y - y) > HEIGHT_TOLERANCE) {
            continue;
        }

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->attr_0x14 >= 0) {
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
            out->attr_0x00 = poly->attr_0x14 & 0x7fffffff;
            out->polyIdSlot_0x04 = slot;
            out->vertexIds_0x08 = poly->vertexIds_0x04;
            out->count_0x0c = n;
            return;
        }
    }

    out->vertexIds_0x08 = 0;
    out->count_0x0c = 0;
}

/* Height-filtered form of GroundProbeTrackPolygon_8c020b6c: re-tests *out's
 * previous match first, then falls back to
 * GroundProbeFindPolygonAtHeight_8c020fe4's search, skipping the polygon just
 * re-tested.
 *
 * spawnEntry_8c0272b8 (026710_traffic) installs this instead of
 * GroundProbeTrackPolygon_8c020b6c as a traffic entry's ground-query callback
 * when the entry's type code has bit 0x4000 set -- the elevated-road case. */
void GroundProbeTrackPolygonAtHeight_8c021290(float x, float y, float z, GroundQueryResult *out)
{
    GroundGrid *grid = var_activeGroundGrid_8c2264d4;
    GroundPoly *poly;
    int *slot;
    int n;

    if (out->count_0x0c != 0) {
        int *ids = out->vertexIds_0x08;
        GroundVertex *v;
        float pdx, pdz;
        int hit;
        int j;

        slot = out->polyIdSlot_0x04;
        poly = &grid->polys_0x14[*slot];
        n = out->count_0x0c;

        v = &grid->verts_0x18[ids[n - 1]];
        pdx = v->x - x;
        pdz = v->z - z;

        if (poly->attr_0x14 >= 0) {
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
            out->attr_0x00 = poly->attr_0x14 & 0x7fffffff;
            out->polyIdSlot_0x04 = slot;
            out->vertexIds_0x08 = poly->vertexIds_0x04;
            out->count_0x0c = n;
            return;
        }
    }

    /* Full cell search, shaped like GroundProbeFindPolygonAtHeight_8c020fe4's --
     * grid-defined cell size and the height filter -- but also skipping the
     * polygon just re-tested above (by vertex-id pointer). */
    {
        GroundCell *cell;
        int cx, cz;
        int i;

        cx = (int)(x / grid->cellSizeX_0x08);
        if (cx >= grid->cellsX_0x00) {
            cx = grid->cellsX_0x00 - 1;
        }

        cz = (int)(z / grid->cellSizeZ_0x0c);
        if (cz >= grid->cellsZ_0x04) {
            cz = grid->cellsZ_0x04 - 1;
        }

        cell = &grid->cells_0x10[cz * grid->cellsX_0x00 + cx];

        for (i = 0; i < cell->count_0x00; i++) {
            int *ids;
            GroundVertex *v;
            float pdx, pdz;
            int hit;
            int j;

            slot = &cell->polyIds_0x04[i];
            poly = &grid->polys_0x14[*slot];
            ids = poly->vertexIds_0x04;

            if (fabsf(grid->verts_0x18[ids[0]].y - y) > HEIGHT_TOLERANCE) {
                continue;
            }

            if (ids == out->vertexIds_0x08) {
                continue;
            }

            n = poly->vertexCount_0x00;

            v = &grid->verts_0x18[ids[n - 1]];
            pdx = v->x - x;
            pdz = v->z - z;

            if (poly->attr_0x14 >= 0) {
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
                out->attr_0x00 = poly->attr_0x14 & 0x7fffffff;
                out->polyIdSlot_0x04 = slot;
                out->vertexIds_0x08 = poly->vertexIds_0x04;
                out->count_0x0c = n;
                return;
            }
        }

        out->vertexIds_0x08 = 0;
        out->count_0x0c = 0;
    }
}
