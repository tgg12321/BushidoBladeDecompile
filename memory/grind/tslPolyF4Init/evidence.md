# Evidence bank — tslPolyF4Init

## Session 1 (recon, 2026-07-19)

- **Canonical verdict:** `C` (pure-C target, distance 8, 81 insns both sides)
- **Sandbox floor:** 8 (honest, --disable all) — 81 vs 81 insns; 4
  non-control-flow differing insns per `diagnose` (MATCHABLE)
- **Register allocation identical** to target across the body — s0=count,
  s1=a1, s2=a2, s3=idx, s4=a0, s5=saved, s6=elem, ra return, sp frame 0x30.
  Diff is **prologue save-slot ORDER**, not allocation.
- **Target callee-save order** (sw order in prologue): s1, s2, s4, s0, s3,
  s5, s6, ra. Corresponding first-value writes interleaved: move s1,a1 →
  move s2,a2 → move s4,a0 → li s0,3 → andi s3,s4,0xff → v1 load → lw s5 →
  sll v0 → addu s6.
- **Build callee-save order** (current HEAD): s4, s1, s2, s3, s5, s6, s0, ra.
  Corresponding first-writes: move s4,a0 → move s1,a1 → move s2,a2 → andi
  s3,s4,0xff → lui/addiu v1 → lw s5 → sll v0 → addu s6 → li s0,3.
- **Delta:** (1) s4 first-written in build but 3rd in target — target
  interleaves s1 (a1) and s2 (a2) BEFORE s4 (a0). (2) s0 last-written in
  build but 4th in target — target puts `count = 3` between s4 assignment
  and s3 (idx = a0 & 0xff) assignment.
- **Cheat siblings in same file** (func_80080258, func_80080390) close the
  save-order gap using `register s32 result asm("s7")` + `unsigned long long
  new_var2 = new_var; count = new_var2;` — this is the register-asm pin +
  dead-scalar-coercion pattern now forbidden by
  `[[inline-asm-policy]]` expanded catalog / `[[dead-vars-local-array]]`
  and `[[register-asm-pins]]`. Not a legal template for this function.
- **Bottom epilogue tail** (cpu_side_move_dir_4 call → `xori v0,v0,0x2 ; sltiu
  v0,v0,1`) matches the current C body's `r ^= 2; return (u32)r < 1;` block
  byte-for-byte; the 8-insn diff is entirely in the prologue region.
- **Artifacts:** `tmp/grind/tslPolyF4Init/s1/build.txt` (sandbox objdump).

- [s1] canonical verdict C, pure-C distance 8, 81 target insns == 81 build insns

- [s1] diagnose: MATCHABLE, 4 non-control-flow differing insns

- [s1] register allocation IDENTICAL to target (s0=count s1=a1 s2=a2 s3=idx s4=a0 s5=saved s6=elem)

- [s1] delta is prologue save ORDER only: target sw seq s1,s2,s4,s0,s3,s5,s6,ra; build sw seq s4,s1,s2,s3,s5,s6,s0,ra

- [s1] target puts sw s0 between move s4,a0 and andi s3,s4,0xff (position 4 of 8 saves); build puts it 7th

- [s1] target puts sw s1 first; build puts sw s4 first

- [s1] epilogue and cpu_side_move_dir_4 tail (xori $v0,0x2; sltiu $v0,1) match byte-for-byte with current C `r ^= 2; return (u32)r < 1;`

- [s1] cheat siblings func_80080258/func_80080390 in same file close the same gap via `register s32 result asm("s7")` + `unsigned long long new_var2 = new_var; count = new_var2;` -- forbidden family per inline-asm-policy / register-asm-pins; NOT a usable template

- [s1] H-s1 measurement kills the naive `s0-first-write == s0-first-save` mapping; save ordering is a joint function of all pseudo first-uses

- [s2] H-s5 CONFIRMED at floor 4: `saved = g_cd_callback_a; count = 3; idx = a0 & 0xFF; base = g_cd_sector_buf; elem = base + idx;` — currently landed in src/system.c.

- [s2] H-s5 mechanism proven by objdump: build now allocates s0=count / s1=a1 / s2=a2 / s3=idx / s4=a0 / s5=saved / s6=elem — IDENTICAL to target allocation.

- [s2] The count/a1 pseudo swap (target: s0=count, s1=a1; old build: s0=a1, s1=count) contributed 4 of the 8-insn old diff (move s0/s1 and li s1/s0 pairs plus their sw offsets); it is now fixed.

- [s2] Remaining 4-insn diff: arg-copy interleave at prologue top. Build starts with `sw s4; move s4, a0` then `sw s1; move s1, a1; sw s2; move s2, a2`. Target starts with `sw s1; move s1, a1; sw s2; move s2, a2` and only after has `sw s4; move s4, a0`.

- [s2] H-s3 KILLED (rejected/elem-inline-drop-base.c): dropping `base` locally regresses to 13. Cheat siblings func_80080258/func_80080390 use `elem = &g_cd_sector_buf[idx]` shape but ALSO require a regfix `reorder` rule to fix the prologue interleave — pure C sibling-shape does not match.

- [s2] H-s4 KILLED (rejected/do-while-loop.c): do-while adds 2 insns (81 -> 83), score 17.

- [s2] H-s5b/c/d/e all KILLED: decl-order, base-before-idx, count-first, and split-init `count = 4; --count;` all fail (neutral or worse) — the specific `saved; count; idx; base; elem` init order is the sweet spot.

- [s2] Sibling evidence (regfix.txt:93 `func_80080258: reorder 3,18,4,17,5,6,7,19,8,9,10,11,12,13,14,15,16 @ 3-19`) shows that even the cheat template requires an assembly-level reorder rule for the arg-copy interleave — strong signal the remaining 4-insn plateau is not reachable via ordinary structural C levers at this allocation.
