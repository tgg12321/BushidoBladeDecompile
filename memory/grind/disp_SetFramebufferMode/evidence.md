# Evidence bank — disp_SetFramebufferMode

- Audit diagnosis (regressions.md): Active volatile coercion via *(vu8 *) casts on non-allowlisted game RAM globals; removing casts raises sandbox score 0→6; the volatile_cheats detector never caught this *(vu8 *)(ptr + N) spelling. Worker must find the pure-C scheduling/structure solution that matches without volatile.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Flagged construct at src/ings.c:202-205: four *(vu8 *) stores inside the i<2 loop; g_disp_fb_flag (0x800f7450) and g_disp_fb_base (0x800f7438) are game RAM (0x800fxxxx), NOT MMIO range - mmio-volatile-type-level does not apply

- [s1] [fable-blitz 2026-07-07] Target asm (build/bb2.elf @ 0x80016768, 16 insns): i=0 (t1); ptr=&g_disp_fb_base (v1); offset=0 (t0); LOOP: lui at/addu at,t0/sb a0,%lo(g_disp_fb_flag)(at) [assembler-macro symbolic+index addressing]; sb a1,25(v1); sb a2,26(v1); sb a3,27(v1); addiu v1,+0x4090; addiu t1,+1; slti v0,t1,2; bnez (delay: addiu t0,+0x4090); jr ra

- [s1] [fable-blitz 2026-07-07] Structural fact: g_disp_fb_flag = g_disp_fb_base+0x18, so all four stores hit consecutive bytes base+0x18..0x1B - yet target addresses store 1 symbolically (indexed off a SECOND 0x4090-stepped induction var t0) and stores 2-4 off the walked pointer v1. Two parallel induction variables stepping by the same 0x4090 survive to the target bytes - loop strength-reduction/IV-merge did NOT collapse them

- [s1] [fable-blitz 2026-07-07] The two symbols are distinct externs, so CSE cannot value-fold the symbolic address into the ptr chain; therefore the no-volatile 6-insn divergence most plausibly comes from (a) loop.c IV elimination merging offset into ptr, and/or (b) LICM hoisting the loop-invariant %hi(g_disp_fb_flag) once the address computation is GCC-split, and/or (c) sched reordering the four independent sb's - which one is UNMEASURED

- [s1] [fable-blitz 2026-07-07] Carve-out check: g_disp_fb_flag has exactly one write site in all of src/ (src/ings.c:202) and no discoverable IRQ-handler mutation - the legitimate-volatile-interrupt-touched two-prong test has no supporting evidence on prong 1; do not plan on the volatile being sanctionable

- [s1] [fable-blitz 2026-07-07] Loop shape: committed for(;i<2;i++) already yields the rotated do-while (bottom-test bnez, offset-increment in the delay slot) - the loop skeleton is not the problem; only the volatile coercion inside the body is
