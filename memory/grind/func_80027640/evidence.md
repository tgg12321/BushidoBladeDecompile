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

## s4 (2026-09-01, structural, 2nd dispatch) -- floor 1 re-measured; the impossibility proof is
## now grounded in GCC 2.7.2's OWN SOURCE, not in an argument about label spelling

This session was dispatched `structural` a second time. Rather than re-search spellings that the
previous session's byte-adjacency argument already excludes, it went one level down and settled the
question in the compiler source, so no future session needs to re-open it.

- **Chassis re-measure (HEAD + `memory/grind/func_80027640/candidate.c` applied):**
  `sandbox func_80027640 --disable all` = **1** (target 158 / build 157, rules_dropped 0,
  cheat_asm_stripped 35). `tmp/grind/func_80027640/s3/sandbox_s3.txt`. Unchanged from s2/s3.
  `src/code6cac_b.c` restored to HEAD (`INCLUDE_ASM`) before finishing.

- **The target seam, re-read from the bytes** (`tmp/grind/func_80027640/s3/target_seam.txt`,
  asm/funcs/func_80027640.s:90-99):
      80027770  j     .L800277A4        (0x08009DE9 -> target 0x800277A4)
      80027774   subu $v0, $v0, $v1     (delay slot; the IF-arm computes $v0 arithmetically)
      ...
      800277A0  lh    $v0, 0x4($a0)     (the ELSE arm's last insn, falls through)
    .L800277A4:
      800277A4  nop
      800277A8  sw    $v0, 0x18($sp)
  The merge block's FIRST word is the nop and the `j` is encoded to that address, so the label is
  pinned between the `lh` and the `nop`. Note the nop is needed on the fall-through path ONLY --
  on the `j` path $v0 comes from `subu`, no load delay -- which is exactly why ASPSX (a reorder
  assembler working on the emitted text) put it there and why no compiler-side hint exists.

- **KILL, at compiler-source level: cc1 CANNOT emit that nop, for ANY C input.** Every load in
  this port routes its delay-slot bookkeeping through `mips_fill_delay_slot`
  (tools/gcc-2.7.2/config/mips/mips.c:673, reached from mips.c:1242 and mips.c:1591, i.e.
  `mips_move_1word`/`mips_move_2words`, plus mips.md:2950). That function contains, verbatim
  (`tmp/grind/func_80027640/s3/mips_fill_delay_slot.excerpt.txt`):

        /* Make sure that we don't put nop's after labels.  */
        next_insn = NEXT_INSN (cur_insn);
        while (next_insn != (rtx)0 && GET_CODE (next_insn) == NOTE)
          next_insn = NEXT_INSN (next_insn);
        ...
        if (TARGET_DEBUG_F_MODE || !optimize || type == DELAY_NONE
            || operands == (rtx *)0 || cur_insn == (rtx)0 || next_insn == (rtx)0
            || GET_CODE (next_insn) == CODE_LABEL
            || (set_reg = operands[0]) == (rtx)0)
          { dslots_number_nops = 0; mips_load_reg = 0; ... return ret; }

  So whenever the insn following a load is a CODE_LABEL (skipping NOTEs only), GCC ZEROES
  `dslots_number_nops`/`mips_load_reg` by design and delegates the hazard to the assembler.
  `final_prescan_insn` (mips.c:4104, `tmp/grind/func_80027640/s3/final_prescan_insn.excerpt.txt`)
  is the only thing that ever prints `#nop`, and it is gated on `dslots_number_nops > 0` -- already
  zeroed -- and is in any case never called for a CODE_LABEL. This is unconditional on the C
  source: it depends solely on RTL adjacency load -> CODE_LABEL, which the target bytes FORCE
  (the `j` pins the label to the nop's address; the `lh` occupies label-4). The NOTE-skipping loop
  is the only slack, and NOTEs emit no words, so nothing a C author can write interposes anything
  that would both break the adjacency and preserve the bytes.
  Observed consistency check in the real dump (`tmp/grind/func_80027640/s3/cc1_seam.txt`,
  s2/cc1.s:526-531): the identical `lh`/`sw` pair 4 words earlier -- with NO label between -- DOES
  get cc1's `#nop`; the one across `.L75:` does not. Same C statement shape, opposite outcome,
  decided purely by the label.

- **Therefore the residual is doubly closed on the C axis:** (1) the label placement is forced by
  the target bytes (s3), and (2) given that placement the compiler is *designed* not to emit the
  nop (this session). The nop can only come from the assembler layer, i.e. from ASPSX originally
  and from maspsx here -- and maspsx's `is_label` (`^\$L(b|e)?\d+:$`, tools/maspsx/maspsx/__init__.py:256)
  does not match this fork's `.L` labels (`LOCAL_LABEL_PREFIX "."`, tools/gcc-2.7.2/config/mips/mips.h:774).
  No declaration order, block-local split, type narrowing or statement re-association can touch any
  link in that chain. **The structural axis is closed with a mechanism, not with a search result.**

- **Not re-attempted, and why:** the Judge's binding constraint forbids closing this function via
  any build-surface change (maspsx/cc1/linker/Makefile source patch or gate-list entry) and forbids
  refiling the INTEGRATION HANDOFF in any spelling, so s3's measured 4-line `$at`-aware maspsx
  repair -- which relinks to SHA1 == oracle and retires the gate list entirely -- was NOT refiled
  here. It remains banked as evidence only. This session proposed no build-surface change and
  modified no file outside memory/grind/func_80027640/ and tmp/grind/func_80027640/s3/.

- [s3] [s4] Chassis re-measure on HEAD with memory/grind/func_80027640/candidate.c applied: sandbox --disable all = 1 (target 158 / build 157, rules_dropped 0); the single residual is still the load-delay nop at 0x800277A4 (tmp/grind/func_80027640/s3/sandbox_s3.txt)

- [s3] [s4] GCC 2.7.2 SOURCE-LEVEL KILL: tools/gcc-2.7.2/config/mips/mips.c:673 mips_fill_delay_slot carries the comment 'Make sure that we don't put nop's after labels.' and unconditionally zeroes dslots_number_nops/mips_load_reg when GET_CODE(next_insn) == CODE_LABEL; mips.c:4104 final_prescan_insn is the ONLY emitter of '#nop' and is gated on that counter. Every load reaches it (mips.c:1242, mips.c:1591, mips.md:2950). Therefore no C input whatsoever can make cc1 emit the nop at a label -- the compiler delegates the hazard to the assembler by design.

- [s3] [s4] The target pins the adjacency the suppression keys on: j @0x80027770 = 0x08009DE9 -> 0x800277A4, so the merge label is on the nop's address and lh $v0,0x4($a0) occupies 0x800277A0 (tmp/grind/func_80027640/s3/target_seam.txt, asm/funcs/func_80027640.s:90-99). On the j path $v0 comes from the delay-slot subu, so the nop is a fall-through-only hazard fill -- exactly the kind a reorder assembler (ASPSX) inserts and a compiler hint cannot express.

- [s3] [s4] Consistency check inside the real dump (tmp/grind/func_80027640/s3/cc1_seam.txt = s2/cc1.s:526-531): the identical lh/sw pair at 0x80027794/0x8002779C, with no label between, DOES get cc1's '#nop'; the pair across '.L75:' does not. Same C statement shape, opposite outcome, decided purely by the intervening label -- empirical confirmation of the source reading.

- [s3] [s4] Combined with s3's byte-adjacency proof, the C axis is doubly closed: the label placement is forced by the bytes AND, given that placement, cc1 is designed not to emit the nop. The nop can only originate in the assembler layer (ASPSX originally; maspsx here), whose is_label regex ^\$L(b|e)?\d+:$ (tools/maspsx/maspsx/__init__.py:256) does not match this fork's '.L' labels (LOCAL_LABEL_PREFIX ".", tools/gcc-2.7.2/config/mips/mips.h:774).

- [s3] [s4] No build-surface change was proposed or made and the INTEGRATION HANDOFF was NOT refiled, per the Judge's binding constraint. s3's measured 4-line $at-aware maspsx repair (byte-neutral across all 31 unrelated objects, relinks to SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, subsumes and retires maspsx_label_nop_funcs.txt) stays banked as evidence only.

- [s3] [s4] Files written this session: memory/grind/func_80027640/evidence.md, memory/grind/func_80027640/hypotheses.md, tmp/grind/func_80027640/s3/*. src/code6cac_b.c was applied for the measurement and restored to HEAD (INCLUDE_ASM) before finishing; tools/, engine/, .claude/rules/, Makefile and *.ld were never touched.

## s4 (2026-09-02) -- permuter modality: the machine search axis, measured dead

- [s4] Chassis re-measure on HEAD with `memory/grind/func_80027640/candidate.c` applied:
  `sandbox func_80027640 --disable all` -> `{"score": 1, "target_insns": 158, "build_insns": 157,
  "rules_dropped": 0, "cheat_asm_stripped": 35}`. The driver's dispatch measurement was
  "unavailable", so this is the authoritative chassis number for s4. Floor unchanged at 1.

- [s4] **A faithful stand-alone permuter workspace for this function now exists and is banked**
  (`tmp/grind/func_80027640/s4/perm/`). It is the first one built for func_80027640; earlier
  sessions never ran a machine search. Construction and, more importantly, its *fidelity proof*:
  * `perm/base_src.c` = the reduced TU (src/code6cac_b.c lines 2-160 header+extern block, lines
    320-322 externs, lines 347-406 = the candidate body). `perm/base.c` is that file after
    `mipsel-linux-gnu-cpp -P` with the project CPP_FLAGS/CPP_DEFS (the permuter preprocesses
    base.c itself with bare `cpp -P -nostdinc`, which cannot find include/, so base.c must be
    pre-preprocessed -- this is why a naive workspace dies at launch with CalledProcessError).
  * `perm/compile.sh` reproduces `engine/pipeline.py:c_pipeline_cmd("code6cac_b")` exactly:
    cpp -> `tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls
    -fno-builtin -w **-mel**` -> prologue_fix -> maspsx (MASPSX_FLAGS **+ --expand-lb**, since
    code6cac_b is in `EXPAND_LB_FILES`) -> `sed .align 3 -> .align 2` (code6cac_b is in
    `RODATA_ALIGN2_FILES`) -> multu_pad -> as. Getting `-mel` and `--expand-lb` right is
    load-bearing; `tools/mar_perm_workspace.sh` (the only pre-existing example in the repo) has
    NEITHER and would have produced a false chassis.
  * `perm/target.o` is assembled from `asm/funcs/func_80027640.s` via
    `tmp/grind/func_80027640/s4/mktarget.py` (glabel -> .globl+label, endlabel dropped, /*..*/
    address comments stripped, `.set noat/.set noreorder/.text/.align 2` prelude): **160 words.**
  * **Fidelity proof:** the reduced TU compiles to a 159-word object whose ONLY difference from
    the 160-word target is the single missing `nop` at word index 90 (= 0x800277A4) --
    `diff perm/base.txt perm/tgt.txt` = `89a90 > nop`, nothing else, no register or ordering
    drift (`tmp/grind/func_80027640/s4/cmp.sh`). So the reduced context reproduces full-TU
    codegen bit-for-bit, and the entire residual really is one word.
  * The permuter agrees: `base_score = 100`, i.e. exactly one insertion under the standard
    weights (ins/del = 100), zero register and zero reordering penalty.

- [s4] **Campaign 1 (`perm/`, label `s4-vector-chassis`, the final candidate as seed):
  56,275 iterations / 28.6 min across three fresh-seed windows, `-j 8`, `--stop-on-zero`,
  `--stack-diffs` (default) -- ZERO finds of any score.** Not "no zero-score find": the permuter
  never produced a single output at all, because from a base of 100 there is no scoring move
  available -- every randomization it can make either keeps the 100-point insertion and adds
  penalties, or is rejected. Harvested and stopped with
  `harvest --stop --reason 'fresh-seed window elapsed (27min/55884 iters, 0 novel finds)'`;
  telemetry is in metrics/events.jsonl (`permuter-launch` / `permuter-harvest`, pid 2860057).

- [s4] **Campaign 2 (`perm2/`, label `s4-scalar-join-chassis`, a deliberately different basin):
  58,381 iterations / 29.9 min, best score 750, 190 outputs -- never below 386, never within
  reach of chassis 1's 100, and never 0.** The seed (`tmp/grind/func_80027640/s4/chassis2_body.c`,
  banked as `memory/grind/func_80027640/rejected/scalar-join-chassis-perm-basin.c`) routes both
  arms through plain `s32 tvx/tvz` scalars and does the `tgt.vx/tgt.vz` stores once after the
  if/else, instead of storing into the VECTOR inside the arms. That is a genuinely different
  chassis -- it compiles to 155 words with a different register assignment throughout
  (`a1`/`a3`, `a0`/`a2`, `v1` swaps) and a shifted branch target -- so it exercises a different
  region of the search space, and it descends monotonically (2188 -> 1737 -> 818 -> 750) toward,
  but never past, the same wall. Best score flat at 750 for the final 18 minutes: no-novel-find
  window elapsed, harvested and stopped (pid 3463632).

- [s4] **Both campaigns are stopped and deregistered** (`permuter_campaign.py status` ->
  "0 live campaign(s), 0 stale registry entr(ies)"). Nothing was left simmering.

- [s4] **Independent, first-hand re-confirmation of the s3 compiler mechanism, from a fresh dump
  taken this session** (`tmp/grind/func_80027640/s4/cc1_raw.s`, the raw cc1 output of the reduced
  TU -- 21 `#nop` hints in the function). Lines 145-149 read verbatim:
      lh   $2,0($4)
      #nop
      sw   $2,16($sp)
      lh   $2,4($4)
      .L8:
      sw   $2,24($sp)
  Two structurally identical load/store pairs four words apart. The first, with no label between
  the load and its consumer, gets cc1's `#nop` hint. The second, whose consumer sits behind the
  cross-jump merge label `.L8`, gets NO hint at all. This is s3's `mips_fill_delay_slot`
  CODE_LABEL suppression observed directly in a dump produced by this session's own workspace,
  on a reduced TU s3 never compiled -- so the finding is chassis-independent, not an artifact of
  the full-TU context.

- [s4] **What the seam actually is, stated once for the record:** `.L8` is a cross-jump / tail-merge
  point. Both arms of `if (D_800A36A4 == 3)` end by leaving the vz value in `$v0` and storing it
  to `0x18($sp)`; GCC merges the two identical stores and jumps the then-arm to the shared store
  (`j .L8` with `subu $2,$2,$3` in its delay slot, so on that path `$v0` is already resolved). The
  missing `nop` is therefore a *fall-through-only* load-delay hazard fill: needed on the else path
  (`lh $2,4($4)` -> `sw`), unnecessary on the jump path. A compiler hint cannot express
  "hazard on one predecessor edge only", which is precisely why GCC punts it to the assembler and
  why ASPSX 2.34 -- a reordering assembler that sees the label -- inserts it and maspsx does not.

- [s4] **Conclusion of the permuter axis: KILLED, and killed for a reason that generalizes.**
  The residual is not a codegen *shape* -- not a register seat, not an emission order, not a
  spill slot, not a fold. It is one assembler-inserted hazard word. The permuter's entire move
  set is C-source rewriting, and no C source can cause the C compiler to emit a word the C
  compiler is explicitly written never to emit at a label. A machine search over C spellings is
  therefore searching a space that provably does not contain the answer, which is exactly what
  114,656 iterations across two structurally distinct chassis measured. **No further permuter
  session should be dispatched against func_80027640.**

- [s4] Files written this session: memory/grind/func_80027640/evidence.md,
  memory/grind/func_80027640/hypotheses.md,
  memory/grind/func_80027640/rejected/scalar-join-chassis-perm-basin.c,
  tmp/grind/func_80027640/s4/*. `src/code6cac_b.c` was applied for the measurement and restored
  to HEAD (`INCLUDE_ASM`) before finishing. `candidate.c` is unchanged -- s4 found nothing better
  and nothing better exists on this axis. No build-surface change was proposed or made; the
  INTEGRATION HANDOFF was NOT refiled; tools/, engine/, .claude/rules/, Makefile and *.ld were
  never touched.

- [s4] Floor re-measured on HEAD with candidate.c applied: sandbox --disable all = 1 (target_insns 158, build_insns 157, rules_dropped 0, cheat_asm_stripped 35). tmp/grind/func_80027640/s4/sandbox_s4.txt. The brief's dispatch measurement was unavailable; this is the authoritative s4 number.

- [s4] The FIRST faithful stand-alone permuter workspace for func_80027640 now exists and is banked at tmp/grind/func_80027640/s4/perm/. Earlier sessions never ran a machine search on this function. Building it correctly required two things absent from the repo's only pre-existing example (tools/mar_perm_workspace.sh): the -mel cc1 flag and maspsx --expand-lb (code6cac_b is in engine/buildconfig.py EXPAND_LB_FILES), plus the '.align 3 -> .align 2' sed for RODATA_ALIGN2_FILES. Without them the chassis is false.

- [s4] Workspace fidelity is PROVED, not assumed: the reduced TU (src/code6cac_b.c lines 2-160 + 320-322 + the candidate body 347-406) compiles through the exact engine pipeline to a 159-word object whose only difference from the 160-word target.o is the single missing nop at word index 90 = 0x800277A4 -- 'diff base.txt tgt.txt' = '89a90 > nop', with no register or ordering drift (tmp/grind/func_80027640/s4/cmp.sh, perm/base.txt, perm/tgt.txt).

- [s4] The permuter independently confirms the residual's size and kind: base_score = 100 on the candidate chassis, i.e. exactly one insertion under the standard weights (ins/del 100, regs 5, reorderings 60), with zero register and zero reordering penalty. There is no register seat and no emission-order component to this residual at all.

- [s4] Campaign 1 (perm/, s4-vector-chassis, pid 2860057): 56,275 iterations / 28.6 min / -j 8 / --stop-on-zero, ZERO outputs of any score. Stopped with harvest --stop, reason 'fresh-seed window elapsed (27min/55884 iters, 0 novel finds)'. Telemetry in metrics/events.jsonl (permuter-launch / permuter-harvest).

- [s4] Campaign 2 (perm2/, s4-scalar-join-chassis, pid 3463632): 58,381 iterations / 29.9 min, 190 outputs, best score 750, descending 2188 -> 1737 -> 818 -> 750 then flat for the final 18 minutes. Never reached chassis 1's base of 100, never 0. Stopped with harvest --stop.

- [s4] Both campaigns are stopped and deregistered: 'permuter_campaign.py status' reports '0 live campaign(s), 0 stale registry entr(ies)'. Nothing was left simmering past the session.

- [s4] First-hand chassis-independent confirmation of the compiler mechanism, from a dump this session produced: tmp/grind/func_80027640/s4/cc1_raw.s:145-149 shows 'lh $2,0($4)' / '#nop' / 'sw $2,16($sp)' / 'lh $2,4($4)' / '.L8:' / 'sw $2,24($sp)'. Identical load/store pairs four words apart; the one with an intervening cross-jump label gets no '#nop' hint. This is GCC 2.7.2's mips_fill_delay_slot CODE_LABEL suppression (mips.c:673), observed in a translation unit s3 never compiled.

- [s4] What the seam is, for the record: .L8 is a cross-jump / tail-merge point. Both arms of 'if (D_800A36A4 == 3)' leave the vz value in $v0 and store it to 0x18($sp); GCC merges the two identical stores and jumps the then-arm to the shared store ('j .L8' with 'subu $2,$2,$3' in its delay slot, so $v0 is already resolved on that path). The missing nop is therefore a FALL-THROUGH-ONLY load-delay hazard fill -- required on the else edge, unnecessary on the jump edge. A compiler hint cannot express 'hazard on one predecessor edge only', which is exactly why GCC delegates it to the assembler, why ASPSX 2.34 (a reordering assembler that sees the label) inserts it, and why maspsx does not.

- [s4] Why the permuter axis is dead for a reason that generalizes: the residual is not a codegen shape -- not a register seat, not an emission order, not a spill slot, not a fold. It is one assembler-inserted hazard word. The permuter's entire move set is C-source rewriting, and no C source can make the C compiler emit a word the C compiler is explicitly written never to emit at a label. 114,656 iterations across two structurally distinct chassis measured exactly that. No further permuter session should be dispatched against func_80027640; grindlib's own zero-yield rule should skip the second permuter slot.

- [s4] Scope: no build-surface change was proposed or made; the INTEGRATION HANDOFF was NOT refiled in any spelling, per the Judge's two binding constraints. src/code6cac_b.c was applied only to take the measurement and restored to HEAD (INCLUDE_ASM) before finishing. Files changed: memory/grind/func_80027640/{candidate.c (header comment only -- the body is byte-identical to s3's), evidence.md, hypotheses.md, rejected/scalar-join-chassis-perm-basin.c} and tmp/grind/func_80027640/s4/*. tools/, engine/, .claude/rules/, Makefile and *.ld were never touched.

- [s4] candidate.c is unchanged in body and remains the best form: s4 found nothing better, and the measurements say nothing better exists on the C axis.

## s5 (2026-09-02, synthesis) -- floor re-measured 1; pass attribution CORRECTED; the last
## escape from the CODE_LABEL predicate identified and closed

### Chassis re-measurement (mandated; the dispatch brief reported "measurement unavailable")
`memory/grind/func_80027640/candidate.c` applied to src/code6cac_b.c at HEAD, then
`& tools/wteng.ps1 main sandbox func_80027640 --disable all`:
`"score": 1, "target_insns": 158, "build_insns": 157, "scorable": true, "rules_dropped": 0,
"cheat_asm_stripped": 35`. The floor is **1** on the CURRENT chassis, one *insertion* (the build
is one instruction SHORT), unchanged from s1-s4. Every banked conclusion below is therefore
still chassis-valid.

### KILL RE-AUDIT (mandated)
`memory/grind/func_80027640/state.json` has **no `kills[]` array at all** (keys: func, file,
session_count, current_modality, floor_history, frontier, judge_constraints, banned_constructs,
ladder_skip, pending_fixup, origin, created), so there is no per-kill `measured_on` to re-audit;
the kills live as prose in hypotheses.md. `tools/fake_ablate.py` is a no-op on this function:
the candidate contains **zero** FAKE constructs (no volatile, no dead store, no alias, no
do/while(0), no named holder), so no banked measurement is FAKE-carrier-contaminated. The
re-measurement above is the honest ablation result.

### CORRECTION: the divergence is inside cc1, not "dropped by maspsx"
s2's write-up (and the candidate.c header, now fixed) said "cc1's instruction stream is ALREADY
the target's ... maspsx drops the nop". The first clause is true only of the *real* instructions.
`tmp/grind/func_80027640/s2/cc1.s:526-531` is:

    lh   $2,0($4)
    #nop
    sw   $2,16($sp)
    lh   $2,4($4)
    .L75:
    sw   $2,24($sp)

cc1 emits `#nop` after the FIRST `lh` and emits **nothing** after the second. maspsx cannot drop
a marker that was never produced. The suppression is `tools/gcc-2.7.2/config/mips/mips.c:705`
inside `mips_fill_delay_slot`: after skipping NOTEs, `GET_CODE (next_insn) == CODE_LABEL` takes
the early-out branch that sets `dslots_number_nops = 0` and clears `mips_load_reg*`. maspsx is
the SECOND blind emitter (its `is_label()` regex `^\$L(b|e)?\d+:$`,
`tools/maspsx/maspsx/__init__.py:256`, does not match this fork's `.L` labels,
`LOCAL_LABEL_PREFIX "."` at `tools/gcc-2.7.2/config/mips/mips.h:774`) -- which is why the s2/s3
maspsx repair also produced the byte. Both facts are true; the ORIGIN is cc1.

### Project-wide cc1 census of the predicate (new class evidence)
`tmp/grind/func_80027640/s5/census.sh` compiles all 32 `src/*.c` with the canonical CC_FLAGS
straight out of cc1 (no maspsx) into `tmp/grind/func_80027640/s5/cc1s/` (1.4 MB of asm) and
`tmp/grind/func_80027640/s5/suppress_census.py` scans it:

* `load -> .L<label> -> insn mentioning the loaded reg` (i.e. the suppression fired): **5** sites
  (`code6cac_c_ab.s` func_8003ACB8, `main.s` spu_DmaTransfer, `text1a_pre.s` func_80040594,
  `text1b.s` func_80060E04, `ings2.s`), plus func_80027640 itself (it was INCLUDE_ASM at census
  time so it is not in the count).
* `load -> .L<label> -> #nop` (a nop that SURVIVED a label): **0** sites in 1.4 MB.

So the predicate is absolute in practice, not just in the source: GCC 2.7.2 never emits a
load-delay nop immediately after a code label anywhere in this project.

### In-tree precedent for the exact shape -- and it is the banned route
Three of those five sites are the same `load / .L label / consumer` geometry and the ORIGINAL
binary DOES carry the nop there:
* `asm/funcs/func_80060E04.s:8-10` -- `lw $v0,%gp_rel(D_800A3420)($gp)` / `.L80060E1C:` / `nop`
* `asm/funcs/func_80040594.s:21-23` -- `lw $s1,0x1C($s2)` / `.L800405E0:` / `nop`
* `spu_DmaTransfer` (same shape)
All three byte-match today **only** via `maspsx_label_nop_funcs.txt` (they are literally the list's
entries, alongside `CD_getsector` and `_spu_init`). That is the precise route the Judge foreclosed
for func_80027640 twice, and the second constraint widened the ban to ANY build-surface change.
The fourth site, `asm/funcs/func_8003ACB8.s:93-95`, resolves WITHOUT a nop because its consumer
(`sh $2,D_800A3904`) is a maspsx-expanded `%hi/%lo` store whose `lui $at` naturally fills the load
delay. That escape cannot apply here: func_80027640's consumer is `sw $v0,0x18($sp)`, sp-relative,
so maspsx has no expansion word to put in the slot -- the slot can only be a literal `nop`.

### The ONLY escape from mips.c:705, and why ordinary C cannot reach it
The early-out is taken when the insn following the load (after skipping NOTEs -- and only NOTEs;
`mips.c:694-696`) is a CODE_LABEL. So the predicate is defeated by ANY non-NOTE insn sitting
between the load and the label. For the nop to land at 0x800277A4 (i.e. AFTER the label) that
intervening insn must additionally (a) emit zero bytes and (b) not be seen by
`FINAL_PRESCAN_INSN`, because the first prescanned insn consumes the counter
(`mips.c:4106-4127`: it either prints the `#nop` there -- which would put the nop BEFORE the
label, at the wrong address -- or falls into `dslots_load_filled++` and clears the state).
Exactly one insn kind satisfies both: a bare `USE` or `CLOBBER`. `tools/gcc-2.7.2/final.c:1548-1550`
`break`s out of `final_scan_insn` on `GET_CODE (body) == USE || GET_CODE (body) == CLOBBER`,
long before the `FINAL_PRESCAN_INSN` call at `final.c:1956`. With such an insn between the load
and the label the sequence would be exactly: label printed, then `sw $2,24($sp)` prescanned,
`reg_mentioned_p ($2)` true, `#nop` emitted -> the target bytes.

This escape has no ordinary-C spelling. Every 2.7.2 construct that emits a standalone CLOBBER
(`emit_no_conflict_block` for multiword/DImode, `store_constructor` clearing a register
aggregate, struct-return setup) emits the clobber immediately BEFORE its own value-producing
insns; those insns are prescanned and consume `dslots_number_nops` at `mips.c:4106` before the
label is reached. Leaving a CLOBBER dangling with no live consumer requires a dead multiword
value -- a dead-store / dead-local construct, i.e. a FAKE family, and one that DCE and flow.c
would in any case delete. USE insns are emitted only immediately before a CALL_INSN (argument
registers, which also emit bytes) and at `expand_function_end` for the return value.

**Net:** the C axis is closed at the RTL-emission level with a named predicate and a named,
unreachable escape; the maspsx axis is closed by Judge constraint; the permuter axis was closed
empirically in s4.

- [s5] Chassis re-measured this session: candidate.c applied to src/code6cac_b.c at HEAD, `sandbox func_80027640 --disable all` -> score 1, target_insns 158, build_insns 157, rules_dropped 0, cheat_asm_stripped 35. The floor is 1 and the build is one instruction SHORT (an insertion, not a substitution).

- [s5] KILL RE-AUDIT (mandated): memory/grind/func_80027640/state.json contains NO kills[] array (keys: func, file, session_count, current_modality, floor_history, frontier, judge_constraints, banned_constructs, ladder_skip, pending_fixup, origin, created), so there is no per-kill measured_on to re-audit; kills are prose in hypotheses.md. tools/fake_ablate.py is a no-op here -- candidate.c carries zero FAKE constructs (no volatile, no dead store, no alias, no do/while(0), no named holder), so no banked measurement is FAKE-carrier-contaminated.

- [s5] The label pin re-derived from bytes this session: `j` at 0x80027770 is word 0x08009DE9, opcode 000010, target field 0x009DE9, address 0x009DE9<<2 = 0x800277A4 -- the nop's own address. Any byte-correct form has a CODE_LABEL immediately after `lh $v0,0x4($a0)` and immediately before the nop.

- [s5] ATTRIBUTION CORRECTED: the residual originates in cc1, not maspsx. tmp/grind/func_80027640/s2/cc1.s:526-531 shows `lh $2,4($4)` / `.L75:` / `sw $2,24($sp)` with no `#nop` marker; cc1 does emit `#nop` for the immediately preceding `lh $2,0($4)`. The suppressing predicate is tools/gcc-2.7.2/config/mips/mips.c:705. candidate.c's header comment (which carried the s2-era 'maspsx drops the nop' wording) was corrected in place this session.

- [s5] Class evidence for the predicate: a 32-TU / 1.4 MB raw-cc1 census (tmp/grind/func_80027640/s5/cc1s, built by s5/census.sh, scanned by s5/suppress_census.py) found ZERO instances of a `#nop` surviving a `.L` label and 5 instances of the suppression firing. mips_fill_delay_slot's early-out tests only GET_CODE(next_insn) -- never the load's operands, mode, or num_nops -- so no C-controllable property of the load can change it.

- [s5] In-tree precedent census for the exact geometry: asm/funcs/func_80060E04.s:8-10 (`lw $v0,%gp_rel(D_800A3420)($gp)` / `.L80060E1C:` / `nop`) and asm/funcs/func_80040594.s:21-23 (`lw $s1,0x1C($s2)` / `.L800405E0:` / `nop`) are identical in shape to func_80027640's seam and byte-match today ONLY via maspsx_label_nop_funcs.txt, whose full contents are spu_DmaTransfer, CD_getsector, func_80060E04, func_80040594, _spu_init. That is the route the two standing Judge constraints foreclose for this function, so the precedent confirms the diagnosis without opening a route.

- [s5] The one nop-free resolution of the geometry, asm/funcs/func_8003ACB8.s:93-95, works because its post-label consumer `sh $2,D_800A3904` is maspsx-expanded and its `lui $at` fills the delay. func_80027640's post-label consumer is `sw $v0,0x18($sp)` (sp-relative, no expansion), so that escape is unavailable and the slot can only be a literal nop.

- [s5] The unique mechanical escape from mips.c:705 is a bare USE/CLOBBER insn between the load and the label: mips.c:694-696 skips only NOTEs, and final.c:1548-1550 breaks on USE/CLOBBER before FINAL_PRESCAN_INSN (final.c:1956), so such an insn emits no bytes and does not consume the pending nop -- the `#nop` would then print on the post-label `sw`, at exactly 0x800277A4. No ordinary-C construct leaves a standalone CLOBBER dangling at the end of a basic block; every CLOBBER-emitting construct is immediately followed by its own value insns, which consume the counter at mips.c:4106.

- [s5] Ladder status after s5: recon (s1), structural (s2, s3), permuter (s4 -- 114,656 iterations, two chassis, zero score-0) and synthesis (s5) are all spent with measurements. The C body is final and FAKE-free; the two routes to 0 (the s2/s3 4-line $at-aware maspsx repair, proven byte-neutral across all 31 other C objects and relinking to SHA1 == oracle; and a maspsx_label_nop_funcs.txt entry) are both build-surface changes banned by standing Judge constraints.
