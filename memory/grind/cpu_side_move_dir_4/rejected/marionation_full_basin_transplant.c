/* s9 REJECTED: full marionation basin transplant.
 * form: honest idx_1495 = idx_1494 + 1 (marionation ships this shape)
 *       + marionation-hybrid arg4-named/arg5-inline inline block.
 * score: masked 20 (baseline h5 = 2). +18 regression.
 * mechanism: the honest respelling and the marionation-hybrid inline
 *   block do NOT compose on csmd4 because csmd4's s-reg web is
 *   materially different from marionation's (marionation carries an
 *   extra idx_1496 local, a `check asm("$6")` pin, a volatile
 *   D_800A147C_2 alias-rename, and a different D_80016248 sentinel).
 *   The +5 (P1 hybrid) + +13 (s8 probe1 honest respell on h5) do NOT
 *   sum linearly to +18 - the two levers COMPOUND at the s-reg-web
 *   allocation level.
 * verdict: KILLED. Sibling transplant from marionation's shape lands
 *   in a strictly worse basin than any csmd4-native form measured.
 *   Confirms the twin's ROUTE is not a fungible template - shared
 *   surface residual (t0/arg5 pair swap), disjoint s-reg web (idx_1496
 *   + register pin + alias-rename all load-bearing at marionation).
 */
