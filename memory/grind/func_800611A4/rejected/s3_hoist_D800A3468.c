/* s3 V2 — REJECTED score=19 build_insns=43. Hoisting
 * `D_800A3468 = (s32)v1;` above the sp[0]/sp[1] halfword reads (as first
 * pre-call statement) reshuffles pre-call scheduling: the v1 address
 * setup lands in a bad slot vs the halfword-load block. Pre-call was
 * already byte-identical to target at baseline; any reorder made it
 * worse. Killed the "hoist v1-addr for downstream RA state" arm of the
 * s2 pre-call-freelist frontier hypothesis. */
