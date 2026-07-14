/* func_8003B3A4 — sandbox distance 0 (re-verified s4, 2026-07-14, annotation
   restored in src after SIXTH hygiene drop — must land in a COMMIT to stick, per
   the 23:22 Judge ruling). s4 (permuter modality) corroborated the exhaustion
   grid by automated search: two fresh-seed random campaigns (clean direct-write
   chassis, 8.7k iters, base 520, best 315; goto-diamond chassis) surfaced ONLY
   known attractor classes (staged-flag local / constant-holder / param-alias
   copy / volatile coercion / identity-inline-fn / clean store reorders) — no
   alias-free form approached the pre-branch $v1 + register-indirect sb shape.
   s3 proved the decl-with-init spelling is an
   equivalence class (split `u8 *p; p = &...;` also 0) — the lever is statement
   position inside the block, not the decl form; shape kept exact per the Judge
   constraint. Exhaustion grid COMPLETE after s2e: direct 6 /
   ternary 6 / diamond-local 8 / offset-fold 6 / multi-use 2 / deref-of-addr 6 /
   fn-scope-pointer 7 / block-scoped annotated alias 0. s2e also proved the inner
   BLOCK SCOPE is load-bearing (fn-scope decl materializes the address into $a2
   before the first diamond → 7); do not flatten the block.
   BLOCKED ON OWNER ONLY: layer-2 FAIL on process ground (Judge self-answered the
   s2b layer-1 NEEDS_USER reserved for the human); question escalated verbatim to
   Trenton in docs/grind/decisions.md "2026-07-13 23:22". Do NOT re-run layer-2
   until Trenton answers (HOLD constraint). */
s32 func_8003B3A4(u8 *arg0) {
    u8 idx;
    u8 a1;
    D_800A3712 = 0;
    idx = (&D_8008D538)[(s8)D_8010277C];
    a1 = (&D_8008D9EC)[idx];
    if (a1 != 0 && D_800A37A0 == 1) {
        a1 = 0;
    }
    {
        /* FAKE: store-only pointer alias — direct symbolic stores expand via the
           assembler sb macro ($at), so the address never enters RA; the pointer
           local makes it an RA-visible pseudo materialized into $v1 pre-branch,
           matching target. Direct/ternary/diamond/offset forms measured 6/6/8/6. */
        u8 *p = &D_8010277D;
        if (a1 != 0) {
            *p = 0xE;
        } else {
            *p = 0x1D;
        }
    }
    D_8010277F = 0;
    {
        u8 v = arg0[0];
        D_800A3680 = v;
        D_800A3671 = v;
    }
    D_80102783 = arg0[1];
    D_800A37B4 = arg0[2];
    D_800A37B5 = arg0[3];
    D_800A37B6 = arg0[4];
    func_8003AF40(1);
    func_8003AFFC();
    return 5;
}
