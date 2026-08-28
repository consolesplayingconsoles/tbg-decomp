#ifndef _0206F0_INTERSECT_H
#define _0206F0_INTERSECT_H

/* Intersects segment (a0, a1) with segment (b0, b1) (each {x, y} pairs);
 * writes the intersection point to out and returns 1 on a hit, else 0.
 * Original-game quirk, preserved: when neither segment is axis-vertical,
 * the hit's y is computed from whatever float already sits at out[0]
 * (not from the x just solved for) -- callers must not rely on y unless
 * out[0] was seeded with that same x beforehand. */
int IntersectSegments_8c0206f0(float *a0, float *a1, float *b0, float *b1, float *out);

#endif // _0206F0_INTERSECT_H
