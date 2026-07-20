/* s3 V10 — REJECTED score=22 build_insns=44. The pure m2c reconstruction
 * (no `t`, no `mask`, no `v1` alias; direct D_800F116C = 0x21001A;
 * direct D_800A3464 = 0xFFFFEF;) recreates the +1 insn already banked as
 * drop_v1_alias.c (address CSE fails without the alias) AND has worse RA
 * without the load-temp local. Confirms m2c's suggested "original shape"
 * is not reachable by our fork from that literal C — the alias is
 * load-bearing for maspsx's address hoisting. */
