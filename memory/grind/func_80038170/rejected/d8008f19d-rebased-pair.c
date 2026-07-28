/* REJECTED (s3, 2026-07-28): one-symbol pair rebased on D_8008F19D —
 * out[0x42] = (&D_8008F19D)[s3 * 2 - 1];
 * out[0x43] = (&D_8008F19D)[s3 * 2 + 0];
 *
 * Why dead: measured sandbox --disable all = 1 (141/141), identical floor to
 * the Judge form. The phantom-frame temp survives (same one-symbol shared-index
 * pair shape), but the reloc addend artifact just moves to the OTHER word:
 * %lo(D_8008F19D)-1 on out[0x42] instead of %lo(D_8008F19C)+1 on out[0x43].
 * Any one-symbol pair covering bytes 0x8008F19C..D necessarily carries a
 * nonzero addend on one of the two accesses, while the stale build/ reference
 * .o (built from the old two-symbol cheat form) has addend 0 on both. This
 * KILLS the last conceivable source-spelling route to sandbox 0 pre-integration:
 * sandbox 0 is unreachable BY CONSTRUCTION until build/ regenerates from the
 * Judge form — which requires retiring the two harmful carriers first
 * (regfix.txt:1250 + tools/prologue_config.json entry, both driver/owner-only).
 * The negative-addend spelling is also strictly worse C than the natural
 * (&D_8008F19C)[s3*2+0/1] form.
 * Probe artifact: tmp/grind/func_80038170/s3/sandbox_d8008f19d_pair_probe.txt
 */
