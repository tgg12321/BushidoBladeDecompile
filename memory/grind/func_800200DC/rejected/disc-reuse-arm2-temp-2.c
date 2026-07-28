/* s4 REJECTED at sandbox 2 (superseded by dy-reuse = 0). Permuter score-10
 * find: reusing disc itself as the arm-2 temp. Gets the /32 quotient to $v1
 * (the {3} pref stays on disc's own pseudo through the chain) but the temp
 * coalesces into disc's pseudo whose home is $v1, while target wants the
 * subu/mult dest in $v0:
 *   ours:   subu v1,a2,v1 / mult v1,s1
 *   target: subu v0,a2,v1 / mult v0,s1
 * Fix is the dy-reuse spelling (dy home = $v0). Do not re-propose. */
/* ... same function as candidate.c except: */
                if (a0 < 0) {
                    disc = a2 - disc;
                    a0 = (disc * dist) / dy2 / 32;
                }
