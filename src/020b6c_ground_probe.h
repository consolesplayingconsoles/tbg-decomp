#ifndef _020B6C_GROUND_PROBE_H
#define _020B6C_GROUND_PROBE_H

#include "020914_ground_query.h"

/* Ground-polygon probes over the grid selected by var_activeGroundGrid_8c2264d4,
 * all writing a GroundQueryResult (020914_ground_query.h).
 *
 * The Track* variants take *out already holding a previous match and re-test that
 * polygon first, falling back to the full cell search on a miss -- the per-frame
 * form of GroundQueryFindPolygon_8c020914. The *AtHeight variants additionally
 * reject candidates whose height is too far off, which is how overlapping road
 * layers (elevated roads over surface streets) are told apart. */

void GroundProbeTrackPolygon_8c020b6c(float x, float y, float z, GroundQueryResult *out);
void GroundProbeFindPolygonAtHeight_8c020fe4(float x, float y, float z, GroundQueryResult *out);
void GroundProbeTrackPolygonAtHeight_8c021290(float x, float y, float z, GroundQueryResult *out);

/* Interpolates point's y from the polygon match in *result -- a no-match
 * (count_0x0c == 0) leaves point untouched. point is {x, y, z}. */
void GroundProbeInterpolateHeight_8c020f7e(GroundQueryResult *result, float *point);

#endif // _020B6C_GROUND_PROBE_H
