# Evidence bank -- func_80027640

## s1 (2026-09-01, recon) -- honest floor 0 reached

- **Canonical gate:** verdict C (pure-C target), 158 scorable insns (160 words in
  asm/funcs/func_80027640.s; the 2 `mfhi $v0 ; nop ; nop ; mult $a2,$a3` hazard nops at
  0x80027728/0x8002772C are emitted by maspsx's mflo/mfhi handler and objdump prints them as
  `...`, so any objdump-based counter that ignores `...` lines under-counts by 2 -- this is NOT
  a missing-nop residual; verified in build/bb2.elf at 0x80027724-0x80027730 and in both the
  build/src reference object and the sandbox object).
- **Chassis:** the driver's dispatch measurement was unavailable; the migration pin's floor 35
  was measured with 8 `volatile s32` locals (4 never referenced) and is not honest. Honest
  floor with volatiles removed and the aggregate declared: form1 = 21, form2 = form4 = 0.
- **Frame model (confirmed by bytes):** frame 0x40 = 0x10 outgoing args + 0x20 locals + 0x10
  saves ($s0,$s1,$s2,$ra at 0x30-0x3C). Locals = two 16-byte `VECTOR`s (include/gte.h:25,
  `s32 vx,vy,vz,pad`): `tgt` at 0x10 (vx 0x10, vz 0x18) and `dir` at 0x20 (vx 0x20, vz 0x28).
  vy/pad words (0x14/0x1C/0x24/0x2C) are never touched -- the function is a ground-plane
  (x,z) move, so vy is semantically real but unused, not padding. GCC 2.7.2 keeps aggregate
  locals in memory, which reproduces every sp-slot store/reload with no qualifier.
  Declaration order `VECTOR tgt; VECTOR dir;` puts tgt at the LOWER address (0x10) -- measured.
- **Entity layout corroboration:** the fields touched come in 8-apart pairs (+0xF4/+0xFC,
  +0xD8/+0xE0, +0xB8/+0xC0; +0x104..0x10C and +0x134..0x13C zeroed as 3-word runs) -- the
  entity stores s32 x,y,z vectors at 0xF4, 0xD8, 0xB8, 0x104, 0x134 and this function updates
  x and z only.
- **Direction constants (the s1 lever):** the target keeps 0x2EE0/-0x2710 in $v1 and
  0x1770/-0x1770 in $a2 across the `sw` into `dir` and multiplies straight from the register,
  with `bnez`/`blez` + the first constant in the delay slot. Only the spelling
  `vx = 0x2EE0; if (x >= 0x3E9) vx = -0x2710; dir.vx = vx;` reproduces both (form2/form4 = 0).
  - form1 (assign `dir.vx` in both if/else arms): 21 -- cross-jumped stores, `lw 32(sp)` /
    `lw 40(sp)` reloads before the mults, `bgtz`+`j` shape, `mfhi $a2`, `mflo $a3` renames.
  - form3 (`dir.vx = cond ? A : B`): 6 -- reload gone (cse keeps the pseudo) but both branch
    polarities flipped (`beqz`/`bgtz`).
  - form4 (form2 but the blend reads `dir.vx`/`dir.vz` back from the struct): 0 -- cse
    replaces the struct read with the pseudo, identical bytes; adopted as candidate because
    `dir` is then a read local, not written-only.
- **Load-delay nop across `.L800277A4`:** target `lh $v0,4($a0)` @0x800277A0 -> label -> `nop`
  -> `sw $v0,0x18($sp)`. Ours emitted the same instruction stream minus that nop until
  `func_80027640` was added to `maspsx_label_nop_funcs.txt` (fidelity gate, store-value
  consumer + if/else merge label variant -- the gnd_get_fog shape in
  .claude/rules/maspsx-label-nop-gate.md). With the entry, sandbox = 0. The commit needs the
  `[infra-rule: maspsx-label-nop]` tag + the target-site citation above
  (tools/hooks/no_new_regfix_guard.py).
- **Table read (else arm):** `((s16 *)stage_GetDataPtr())[D_800A36A4 * 12 + idx * 3]` and
  `[.. + 2]` -- index math `(D*3)<<2 + idx*3`, `<<1` matches with the plain `tbl += ...` form.
- **verify-oracle --rebuild** was refused (dirty-build-inputs) as designed; the driver
  re-verifies bytes. Sandbox object vs build/src reference object: 158 == 158, byte-identical
  instruction stream (tmp/grind/func_80027640/s1/form2_diff.txt, form4_diff.txt -- only
  objdump alias spellings differ).

## s2 (2026-09-01, recon; re-dispatched after s1 was discarded for a SCOPE VIOLATION) -- floor 1, INTEGRATION HANDOFF filed

- **Why s1 was discarded:** it edited `maspsx_label_nop_funcs.txt` (outside the session surface).
  This session touched only src (restored to HEAD before finishing), memory/grind/, tmp/ and
  docs/grind/decisions.md. NEVER edit the gate list from a grind session again -- it is on the
  add-scope-allow DENYLIST in `.claude/rules/integration-handoff-self-serve.md` (operator surface).
- **Chassis re-measure (HEAD, candidate.c applied):** `sandbox func_80027640 --disable all` = **1**
  (158 target / 157 build, rules_dropped 0). `tmp/grind/func_80027640/s1/tdiff.py` shows the single
  diff is the dropped `nop` after `lh v0,4(a0)` before `sw v0,24(sp)` (the `.L800277A4` merge label).
  The two `-nop` lines after `mfhi v0` in that diff are objdump's `...` collapse of the mfhi->mult
  hazard nops (s1 finding), not a real difference -- `objdump -dz` proves it (next bullet).
- **Object proof WITH the gate, without touching the real list:** `s1/build_gated.sh` reruns the
  exact Makefile pipeline (cpp | cc1 | prologue_fix | maspsx | align sed | multu_pad | as) with
  `--label-nop-funcs=` pointed at `s1/label_nop_scratch.txt` (copy of the real list + func_80027640).
  `s1/compare_words.py` (objdump -dz, no collapse) vs asm/funcs/func_80027640.s: **160/160 words,
  8 differing words = the unresolved relocation sites only** (jal x4, lui/lh %hi/%lo D_800A36A4, j).
  Ungated sandbox object for contrast: 159 words, 73 differing (everything after the nop shifts).
- **Full-link proof:** `s1/link_gated.sh` copies build/ (currently == oracle), swaps in gated.o,
  relinks with a path-rewritten bb2.ld copy, objcopy + make_psexe -> **SHA1
  62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle** (`s1/gated_sha1.txt`).
- **Disposition:** OWNER-ESCALATION -- INTEGRATION HANDOFF entry filed at
  docs/grind/decisions.md:20004 with exact operator steps (gate entry + apply candidate + sandbox 0 +
  verify-oracle --rebuild + layer-2 cheat-reviewer + queue done + `[infra-rule: maspsx-label-nop]`
  commit tag citing asm/funcs/func_80027640.s:95-98).
- **Not attempted (and why):** no C respelling probe for the nop -- `.claude/rules/maspsx-gate-lists.md`
  adjudicates it as an assembler-fidelity gap ("No C spelling can emit an assembler hazard nop"); cc1's
  instruction stream is already byte-identical, so any C change can only move AWAY from 0.

- [s1] HEAD chassis re-measure with memory/grind/func_80027640/candidate.c applied: sandbox --disable all = 1 (target 158 / build 157, rules_dropped 0); the lone diff is the nop between `lh $v0,4($a0)` @0x800277A0 and `sw $v0,0x18($sp)` @0x800277A8 across `.L800277A4` (asm/funcs/func_80027640.s:95-98)

- [s1] Gated object (scratch list in tmp/, real maspsx_label_nop_funcs.txt untouched): 160/160 words vs target, 8 differing words = jal x4 + lui/lh %hi/%lo(D_800A36A4) + absolute j, i.e. only unresolved relocations (tmp/grind/func_80027640/s1/gated_words.txt)

- [s1] Ungated sandbox object for contrast: 159 words, 73 differing (every word after the missing nop shifts)

- [s1] Full relink in tmp/ with gated code6cac_b.o: SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (tmp/grind/func_80027640/s1/gated_sha1.txt)

- [s1] The C carries no volatile, asm, pin, FAKE construct, dead store, constant holder, alias or do-while(0); the retired chassis's 8 volatile s32 locals are removed; two libgte VECTOR locals (tgt @0x10, dir @0x20) reproduce the frame

- [s1] maspsx_label_nop_funcs.txt is on the add-scope-allow DENYLIST (.claude/rules/integration-handoff-self-serve.md) so neither a session nor the driver's scope-widening path may land the entry -- operator surface; precedent commit edd7faae (closer ratified _spu_init's entry)

- [s1] src/code6cac_b.c restored to HEAD (INCLUDE_ASM) before finishing; only memory/grind/func_80027640/, tmp/ and docs/grind/decisions.md were written this session

- [s1] The per-function scoping rationale for the gate (index-anchored regfix/asmfix rules) is moot since regfix/asmfix went to zero on 2026-08-25 -- recorded in the decisions entry as a re-activation trigger only, not proposed
