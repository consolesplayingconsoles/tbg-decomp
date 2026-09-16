#ifndef _020914_GROUND_QUERY_H
#define _020914_GROUND_QUERY_H

typedef struct {
    float x, y, z;
} GroundVertex;

/* One 150-unit grid cell: the polygons whose footprint touches it. */
typedef struct {
    int count_0x00;
    int *polyIds_0x04;
} GroundCell;

typedef struct {
    int vertexCount_0x00;
    int *vertexIds_0x04;
    float normalX_0x08;     /* precomputed plane normal */
    float normalY_0x0c;
    float normalZ_0x10;
    int attr_0x14;  /* sign bit selects the concave containment test */
} GroundPoly;

typedef struct {
    int cellsX_0x00;
    int cellsZ_0x04;
    float cellSizeX_0x08;
    float cellSizeZ_0x0c;
    GroundCell *cells_0x10;
    GroundPoly *polys_0x14;
    GroundVertex *verts_0x18;
} GroundGrid;

/* Ground-polygon match written by GroundQueryFindPolygon_8c020914 and consumed
 * by GroundProbeInterpolateHeight_8c020f7e (020b6c_ground_probe.h), which
 * interpolates the point's height from it. A miss zeroes count_0x0c and
 * vertexIds_0x08 only, leaving the other two fields stale. */
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
