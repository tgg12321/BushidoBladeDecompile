/* func_80030580 - session 6 (synthesis).  THIS FORM MEASURES sandbox --disable all = 0
 * (bytes proven on the s6 chassis, 148/148 insns, frame `addiu sp,-0x18` exact).
 *
 * IT IS NOT AN ACCEPTED FORM.  It is held here PENDING AN OWNER/JUDGE RULING on the
 * construct below (session 6 returned `ruling-request`, not `candidate-ready`).  The
 * clean, construct-free pure-C body that stands at the honest floor of 2 is preserved
 * beside this file as `pure-c-floor2-body.c` - restore THAT if the ruling goes against
 * the union, and do not spend this form until the ruling is recorded.
 *
 * THE CONSTRUCT UNDER REVIEW (the last two zero stores of the function):
 *
 *     union { s32 w; u16 h[2]; } z;
 *     ...
 *     z.h[0] = 0;
 *     z.h[1] = 0;
 *     *(u8 *)(obj + 5) = z.h[0];
 *     *(s16 *)obj      = z.h[1];
 *
 * WHAT IT BUYS.  s1-s5 established that the entire residual is the frame: the target
 * reserves 24 bytes of locals and touches none of them, ours reserved 8.  s2's corpus
 * census typed those bytes as three 8-byte `ctx=spill_new` combine-orphan slots, and s3
 * found the only generator known until now - an array-INDEXED global folded into
 * `(mem (plus (reg idx) (symbol_ref)))`, obeying `slots = max(0, sites - 1)`.  Buying two
 * more slots that way needs FOUR source-level `(&Judge)[...]` sites, and the cheapest
 * such form (j4read) still emits both extra lookups: +25 body lines.
 *
 * GENERATOR 4, found this session by a standalone catalog probe
 * (tmp/grind/func_80030580/s6/gen5.c + gen6.c, run through s5/genprobe.py), is a
 * different and much cheaper producer: a 4-byte union local with an SImode member and
 * two HImode members, where BOTH halfword members are written and read back, is
 * register-allocated (zero ($sp) traffic) and yields TWO `ctx=spill_new` orphan slots.
 * Standalone: gen6.c:b1 vars=16 sp=0; the identical data flow through two plain `u16`
 * locals (gen6.c:c1/c2) yields vars=0, and a union written as a word and split into
 * halves (gen6.c:b3) yields vars=0 - so the union wrapper with a word member is
 * load-bearing.  In-function, a bare `struct { u16 a, b; }` or a `u16 z[2]` array
 * (variants z05s / z05a) becomes a real `ctx=stack_temp` with three ($sp) accesses and
 * 11 body-diff lines; only the union stays in registers.
 *
 * WHY THIS PLACEMENT.  Hosting the pair on the function's last two zero stores costs
 * nothing: both members are written from a real value (0), both are read exactly once,
 * and each read lands in an instruction the target already emits (`sb $0,5($7)` and
 * `sh $0,0($7)`).  fdiff against the s1 base is 4 lines, all of them the wanted
 * `subu $sp,$sp,8` -> `subu $sp,$sp,24` prologue/epilogue pair.  Frame census:
 * vars=24, sp=0, three orphan slots (p78, p111, p190).  Equivalent byte-neutral
 * placements measured the same session: z05n (a named `struct { u16 a, b; }` inside the
 * union), z78 (hosted on the `obj+7` / `obj+8` byte stores), z05b (union declared in a
 * block), z05i / z05u (`s16` members / `u32` word member).
 *
 * WHY IT IS A RULING QUESTION AND NOT A SUBMISSION.  Applying the six-test checklist
 * honestly, the construct fails T1 (the program behaves identically without it), T2 (a
 * reader would ask why two zeros are routed through a union) and T3 (the mechanism is a
 * GCC pass - combine's orphaned REG_DEAD note becoming a `(use (reg N))`, which reload's
 * alter_reg then backs with a stack slot that emits nothing).  It is an AGGREGATE-typed
 * local whose only effect is the frame, which sits between two frozen families without
 * being covered by either: `named-local-fake-exception.md` sanctions constant-holder
 * locals but is SCALARS ONLY with arrays/frame coercion forbidden, and the
 * named-intermediate family (narrow-byte-args-packed-call.md + the 2026-08-17
 * clarification) contemplates a fresh once-written/once-read local carrying a real
 * consumed value - which each union MEMBER satisfies, while the union OBJECT itself is
 * the thing doing the work.  Per the first-reach rule the correct outcome is a ruling
 * request.  docs/reference/sotn-construct-index.md does not index union locals at all
 * (zero occurrences of the word), so it is SILENT on this construct - that is not a
 * negative census.
 *
 * ALSO MEASURED AND DEAD THIS SESSION (do not re-propose):
 *  - Symbol-address bias, `(&Judge)[i] == (&Judge + K)[i - K]`, in every spelling
 *    (bias1/bias2/biasboth/biasneg/bias1s/biases/biasp): body-neutral but frame-inert,
 *    vars stays 8.  The constant bias is folded before the address pseudos separate.
 *  - The dead-store spellings of generator 4 (u0/u2/u56h1/u54/u56blk, and u56h0/u56w
 *    which overshoot to vars=32): they reach the frame too, but leave a union member
 *    written and never read.  Strictly worse than z05 on every test.
 *  - The fully-live union pair hosted where the two halves carry DIFFERENT real values
 *    (q54/q54n/q56/q5c/q0a): vars 16-24 but 26-74 body-diff lines, because reading the
 *    members back forces shift/mask composition.  Only an all-zero pair is byte-neutral.
 */
s32 *func_80030580(s32 *arg0, s32 arg1) {
    u8 *obj;
    u8 *src = (u8 *)arg0;
    s16 *tbl;
    s32 i;
    union { s32 w; u16 h[2]; } z;

    obj = (u8 *)&D_80106A78;
    for (i = 0; i < 12; i++, obj += 0x64) {
        if (*(s16 *)(obj + 2) == -1 && *(u8 *)(obj + 0xA) == 0xFF) break;
    }
    *(u8 *)(obj + 0xA) = i;
    *(s16 *)(obj + 2) = arg1;
    *(u8 *)(obj + 7) = 0;
    *(u8 *)(obj + 8) = 0;
    *(u8 *)(obj + 4) = 1;
    *(u8 *)(obj + 6) = *(u16 *)(src + 4);
    *(s32 *)(obj + 0x2C) = *(s32 *)(src + 0xF4);
    *(s32 *)(obj + 0x30) = *(s32 *)(src + 0xF8) - *(s16 *)(src + 0x1A) / 32;
    *(s32 *)(obj + 0x34) = *(s32 *)(src + 0xFC);
    tbl = &D_8008E194 + arg1 * 7;
    *(s32 *)(obj + 0x44) = ((&Judge)[*(u16 *)(src + 0x1CA) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x48) = tbl[3];
    *(s32 *)(obj + 0x4C) = ((&Judge)[(*(s16 *)(src + 0x1CA) + 0x400) & 0xFFF] * tbl[2]) >> 12;
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44);
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48);
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C);
    *(s32 *)(obj + 0x2C) += *(s32 *)(obj + 0x44) / 2;
    *(s32 *)(obj + 0x30) += *(s32 *)(obj + 0x48) / 2;
    *(s32 *)(obj + 0x34) += *(s32 *)(obj + 0x4C) / 2;
    *(Vec3i *)(obj + 0x38) = *(Vec3i *)(obj + 0x2C);
    *(s16 *)(obj + 0x54) = 0;
    *(u16 *)(obj + 0x56) = *(u16 *)(src + 0x1CA);
    *(s16 *)(obj + 0x58) = 0;
    if (tbl[0] == 1) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 2) {
        *(u16 *)(obj + 0x5C) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    } else if (tbl[0] == 3) {
        *(s16 *)(obj + 0x5C) = 0;
        *(u16 *)(obj + 0x5E) = *(u16 *)(tbl + 4);
        *(s16 *)(obj + 0x60) = 0;
    } else {
        *(s16 *)(obj + 0x5C) = 0;
        *(s16 *)(obj + 0x5E) = 0;
        *(s16 *)(obj + 0x60) = 0;
    }
    *(s32 *)(obj + 0x50) = 1;
    z.h[0] = 0;
    z.h[1] = 0;
    *(u8 *)(obj + 5) = z.h[0];
    *(s16 *)obj = z.h[1];
    return (s32 *)obj;
}
