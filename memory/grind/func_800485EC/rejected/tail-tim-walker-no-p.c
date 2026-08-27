/* REJECTED (s1, measured 39): walking tim itself in the tail, no p:
 *
 *     tim += 2;
 *     spr->x = x; spr->y = y;
 *     spr->h = ((u16 *)tim)[1];
 *     spr->w = *tim++;
 *     spr->pixdata = tim;
 *
 * Defeats the cse fold (self-increment) and hits 5 insns / correct shapes,
 * but the walker stays in callee-saved s1 (tim's pseudo lives across the
 * GetClut call) while target reads/stores through caller-saved v1, and the
 * extended live range perturbs allocation broadly. Score 39 — far worse
 * than the folded form (7). The walker must be a FRESH variable dying
 * before the GetTPage call.
 */
