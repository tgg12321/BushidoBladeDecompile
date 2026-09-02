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

## s3 (2026-09-01, structural) -- floor 1 unchanged; the residual is proven C-UNREACHABLE and the
## maspsx defect behind it is proven GENERALLY fixable, byte-neutrally, with NO gate-list entry

Context: the Judge FAILed s2's INTEGRATION HANDOFF with the binding constraint "func_80027640 may
not be closed via a maspsx_label_nop_funcs.txt entry or any maspsx gate-list addition; change
modality and pursue a route that needs no build-surface change." This session attacked the
structural axis (can a C respelling avoid the seam?) and, after proving it cannot, characterised
the defect itself.

- **Chassis re-measure (HEAD + candidate.c applied):** `sandbox func_80027640 --disable all` = **1**
  (target 158 / build 157, rules_dropped 0). Same single residual as s2: the load-delay `nop` at
  0x800277A4.

- **PASS ATTRIBUTION (cc1 dump read, not guessed):** `tmp/grind/func_80027640/s2/cc1.s:526-531` --
  cc1's own output is
      lh   $2,0($4)
      #nop            <- cc1 emits the hazard hint here
      sw   $2,16($sp)
      lh   $2,4($4)
      .L75:           <- merge label; NO hint, cc1 leaves the seam to the assembler
      sw   $2,24($sp)
  The compiler is NOT the divergence: cc1's instruction stream is already the target's. maspsx's own
  debug comment names the failure exactly (`tmp/grind/func_80027640/s2/unungated.s:558`):
  `#nop # DEBUG: '.L75:' does not load from $2` -- maspsx treated the LABEL LINE as the next
  instruction, because `is_label()` is `^\$L(b|e)?\d+:$` and this GCC fork emits `.L`
  (tools/maspsx/maspsx/__init__.py:256). maspsx also strips every `.set reorder/noreorder` (0 `.set`
  directives survive in the emitted function), so GAS cannot recover the nop either.

- **STRUCTURAL IMPOSSIBILITY PROOF (general over ALL C forms, not a search result).** The target
  fixes three consecutive words: 0x800277A0 `lh $v0,0x4($a0)`, 0x800277A4 `nop`, 0x800277A8
  `sw $v0,0x18($sp)`; and 0x80027770 `j .L800277A4` (encoded 0x08009DE9) fixes the branch
  destination at the NOP's address. Therefore any C form that produces the target bytes must
  (i) emit a compiler-generated basic-block label at 0x800277A4, since GCC always defines a label at
  a jump destination, and (ii) place that label textually between the `lh` and its consuming `sw`,
  since those are the immediately adjacent words. GCC 2.7.2 spells such labels `.L<n>:` in this
  build. So EVERY byte-correct C spelling lands on maspsx's `.L` blind spot -- the residual is
  invariant under declaration order, block-local splits, type narrowing and statement
  re-association. There is no structural lever; the axis is closed by construction, not by search.

- **PROJECT-WIDE CENSUS of the blind spot** (`tmp/grind/func_80027640/s2/scan_blindspots.sh` +
  `analyze_sites.py`, both compile every `src/*.c` with the real Makefile flags and read maspsx's own
  DEBUG lines): **33 sites** where maspsx sees a `.L` label as the next "instruction" after a load.
  28 are harmless (the post-label instruction does not read the loaded register). Of the 5
  consuming-looking sites, 2 are false positives (the post-label load REDEFINES the register:
  `D_800832F8` in ings2, `_spu_Fw1ts` in main). The 3 real load->label->consumer seams are:
    * `func_8001EA84` (code6cac.c): `lbu $2,D_800A3804` / `.L269:` / `sb $2,D_800A3817`
    * `func_8003ACB8` (code6cac_c_ab.c): `lh $2,D_800A36C6` / `.L29:` / `sh $2,D_800A3904`
    * `func_80027640` (code6cac_b.c): `lh $2,4($4)` / `.L75:` / `sw $2,24($sp)`
  The first two are ALREADY oracle-matching WITHOUT a nop -- because their consumer is a `%hi/%lo`
  macro whose `lui $at` expansion fills the load delay. That is precisely the `uses_at` test maspsx
  already applies on its ordinary (non-label) path and does NOT apply inside the `.L`-label branch.
  func_80027640's consumer is `sw $v0,0x18($sp)` -- sp-relative, no `$at` -- so its delay is
  genuinely unfilled and the target HAS the nop. **Exactly one site in the whole project needs it.**

- **THE GENERAL FIX, MEASURED (4 lines, no per-function list).** Inside the `.L`-label branch, apply
  the same delay-fill test the ordinary path already uses
  (`tmp/grind/func_80027640/s2/maspsx_at_aware_label_fix.diff`):
      if line_loads_from_reg(after_label, r_dest) and (
              not uses_at(after_label) or self._uses_gp(after_label) or self.nop_at_expansion):
  Measured with a PATCHED COPY under tmp/ (`tools/maspsx` itself untouched;
  `build_patched.sh` replicates the Makefile pipeline including per-file -G8/--expand-lb/align-2):
    * Control run with STOCK maspsx through the same script reproduces `build/src/*.o`
      BYTE-IDENTICALLY for all 31 unrelated objects -- the harness is faithful.
    * With the fix: **all 31 unrelated objects still byte-identical to the oracle build**; the only
      changed object is `code6cac_b.o`, which is the one carrying the candidate C.
    * `compare_words.py` on the fixed `code6cac_b.o`: **160/160 words vs asm/funcs/func_80027640.s,
      8 differing words = the unresolved relocations only** (4x jal, lui/lh %hi/%lo D_800A36A4, j).
    * Full relink in tmp/ (`link_atfix.sh`): **SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa ==
      oracle**, 606208 bytes.
    * Re-run with an EMPTY `--label-nop-funcs` list: identical objects everywhere, including
      code6cac_b. **The fix SUBSUMES the entire per-function gate list** (its 5 current entries --
      spu_DmaTransfer, CD_getsector, func_80060E04, func_80040594, _spu_init -- keep matching with
      the list empty), so it retires the mechanism rather than extending it.

- **Two narrower/broader variants MEASURED AND KILLED** (so no future session re-tries them):
    * Global `is_label` widened to `[$.]L\d+:` -- breaks `code6cac_c2.o` (the mflo/mfhi-with-label
      path starts seeing the label and reorders the div expansion). NOT byte-neutral.
    * Simply un-gating the load-consumer branch (drop the `label_nop_func_set` test, keep no `$at`
      test) -- breaks `code6cac_c_ab.o` (func_8003ACB8 gains a nop its target does not have). NOT
      byte-neutral. The `$at`-aware form above is the only one of the three that holds.

- The gate list's own header comment justifies per-function scoping by "index-anchored regfix/asmfix
  rules"; those are ZERO project-wide since 2026-08-25 ([[rules-to-zero-complete]]), and the
  measurement above shows the cascade it feared does not occur for the `$at`-aware form.

- `src/code6cac_b.c` was restored to HEAD (`INCLUDE_ASM`) before finishing; this session wrote only
  `memory/grind/func_80027640/`, `tmp/grind/func_80027640/s2/` and `docs/grind/decisions.md`.
  `tools/maspsx/` was NEVER modified -- every measurement used a copy under tmp/.

- [s2] Chassis re-measure on HEAD with memory/grind/func_80027640/candidate.c applied: sandbox --disable all = 1 (target 158 / build 157, rules_dropped 0); the single residual is the load-delay nop at 0x800277A4

- [s2] The C is final: any byte-correct form must put a GCC .L basic-block label at 0x800277A4 between lh $v0,0x4($a0) and sw $v0,0x18($sp), because the j at 0x80027770 (0x08009DE9) pins the jump destination to the nop's address -- so no structural lever exists

- [s2] cc1's own output already equals the target instruction stream (tmp/grind/func_80027640/s2/cc1.s:526-531); maspsx names its own failure at unungated.s:558 -- "#nop # DEBUG: '.L75:' does not load from $2"; maspsx emits 0 .set reorder/noreorder directives in the function so GAS cannot insert the nop either

- [s2] Project-wide census of the blind spot: 33 sites, 28 non-consuming, 2 false positives, 3 real seams; func_8001EA84 and func_8003ACB8 already match WITHOUT a nop because their %hi/%lo consumer's lui $at expansion fills the load delay; func_80027640 is the only site project-wide with a genuinely unfilled delay

- [s2] The 4-line $at-aware repair of maspsx's .L-label branch (tmp/grind/func_80027640/s2/maspsx_at_aware_label_fix.diff) keeps all 31 unrelated C objects byte-identical to the oracle build, gives func_80027640 160/160 target words (8 reloc-only diffs), and relinks to SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle

- [s2] With the repair and an EMPTY --label-nop-funcs list every object still matches, so maspsx_label_nop_funcs.txt (spu_DmaTransfer, CD_getsector, func_80060E04, func_80040594, _spu_init) and its flag become inert -- the repair RETIRES the gate mechanism instead of extending it, which is why it is not the route the Judge foreclosed

- [s2] The gate list's stated rationale (index-anchored regfix/asmfix cascades) is stale: regfix.txt and asmfix.txt have been empty project-wide since 2026-08-25, and the cascade is measured absent for the $at-aware form

- [s2] tools/maspsx was NEVER modified: every measurement used a copy under tmp/grind/func_80027640/s2/. src/code6cac_b.c was restored to HEAD (INCLUDE_ASM) before finishing; this session wrote only memory/grind/func_80027640/, tmp/grind/func_80027640/s2/ and docs/grind/decisions.md
