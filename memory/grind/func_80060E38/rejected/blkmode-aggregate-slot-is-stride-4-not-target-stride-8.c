/* REJECTED FORM — func_80060E38, grind session 5 (permuter, 2026-08-03).
 *
 * WHY IT IS DEAD (measured, 8 compiled variants via tmp/grind/func_80060E38/s5/gen5.py):
 *
 * The idea (session 4's frontier item I2 / session 3's H3): route the nine spilled
 * constants through an AGGREGATE, so the stack slot is created by assign_stack_temp
 * instead of by reload's alter_reg. The mechanism is real and was confirmed at source
 * this session:
 *     function.c:879  p->slot = assign_stack_local (mode, size, mode == BLKmode ? -1 : 0);
 *     function.c:702  if (BYTES_BIG_ENDIAN && mode != BLKmode) bigend_correction = ...
 * A BLKmode slot therefore takes the align == -1 path (alignment 8, size CEIL_ROUNDed to 8)
 * WITHOUT the +4 big-endian correction every SImode reload spill carries. It is the only
 * such route in function.c — exactly the combination target's nine slots appear to have.
 *
 * MEASURED RESULT — the route reaches 0 mod 8 but cannot reach target's SHAPE:
 *   v_structinit / v_arrinit / v_structcopy   aggregate slot at base 0 (0 mod 8, correct)
 *                                             but only 91 insns: GCC turns the brace
 *                                             initialiser into a memcpy from a rodata
 *                                             image, destroying target's 32 lui/ori stream.
 *   v_structfield / v_structfield_at (161)    aggregate at 0..32, i.e. 0 mod 8 — and the
 *                                             nine SImode reload spills alongside it are
 *                                             STILL at 44,52,60,...,108, all == 4 (mod 8).
 *                                             A BLKmode slot in the same frame does not
 *                                             move the spill congruence at all.
 *   v_structsmall (144)                       word slots 44,52,...,100, all == 4 (mod 8).
 *   v_structret (85)                          the genuine assign_stack_temp site
 *                                             (function.c:3736, struct returned by value):
 *                                             no word slot at 0 mod 8, and it needs a call,
 *                                             so the function stops being a leaf.
 *   v_structpad (184)  DIAGNOSTIC ONLY        struct { s32 v; s32 pad; } p[32] DOES give
 *                                             word slots at 0,8,16,... — target's exact
 *                                             congruence and stride — but only by
 *                                             manufacturing frame padding, which is the
 *                                             forbidden dead-vars / frame-coercion family
 *                                             (score-inert under the cheat-invisible
 *                                             sandbox anyway), and it costs 184 insns
 *                                             against target's 139.
 *
 * THE CLOSED TAXONOMY (all three slot-creation routes in function.c, now measured):
 *   reload spill (align == -1) : stride 8, offset == 4 (mod 8)   [s1/s2/s3]
 *   declared local (align == 0): stride 4, offset == 0 (mod 8)   [s2 v_locals.c]
 *   aggregate temp (BLKmode)   : base 0 (mod 8), members stride 4 [s5, this file]
 * For a 4-byte value, "stride 8" and "congruence 0" are mutually exclusive in this fork
 * unless the C manufactures 8-byte padding (a cheat) or uses 8-byte modes (two-word
 * accesses, killed in s3). Target needs both properties on nine single-word slots.
 *
 * Do not re-propose any aggregate / struct / array / memcpy spelling for this function.
 */

/* The sharpest disproving variant, verbatim (s5/v_structfield.c, abridged to its shape):

    struct N { s32 f0; ... s32 f8; };

    void func_80060E38(s32 arg0, s32 arg1) {
        struct N n;
        n.f0 = 0x1F800000;  ...  n.f8 = 0x1F800062;
        D_800A34A0 = 0x1F800064;  ...  (the 23 scalar stores)
        D_800A3468 = n.f0;  ...  D_800A349C = n.f8;
        *(s32 *)0x1F800004 = arg0;
        *(s32 *)0x1F800008 = arg1;
    }

   -> 161 insns; aggregate slot at 0..32 (0 mod 8); nine reload spills at 44,52,...,108,
      every one == 4 (mod 8). Full sources and offset reports: tmp/grind/func_80060E38/s5/.
*/
