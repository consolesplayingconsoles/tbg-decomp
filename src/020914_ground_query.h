#ifndef _020914_GROUND_QUERY_H
#define _020914_GROUND_QUERY_H

/* Ground-polygon match written by GroundQueryFindPolygon_8c020914 and consumed
 * by GroundProbeInterpolateHeight_8c020f7e (020b6c_ground_probe.h), which interpolates the point's height from it.
 * Only count_0x0c is cleared on a miss, so it is the hit/miss flag. */
typedef struct {
    int attr_0x00;          /* polygon attribute word, sign bit stripped */
    int *polyIdSlot_0x04;   /* the matching entry in the grid cell's polygon list */
    int *vertexIds_0x08;    /* polygon's vertex indices into the grid's vertex array */
    int count_0x0c;         /* vertex count; 0 = no polygon under the point */
} GroundQueryResult;

/* Looks up the ground polygon under world point (x, z) -- y is unused -- in the
 * grid currently selected by var_activeGroundGrid_8c2264d4, writing the match
 * (or a zeroed count on miss) to *out. */
void GroundQueryFindPolygon_8c020914(float x, float y, float z, GroundQueryResult *out);

#endif // _020914_GROUND_QUERY_H
