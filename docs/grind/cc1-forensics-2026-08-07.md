# cc1 forensics — what the shipped oracle compiler actually does differently

**Date:** 2026-08-07 · **Scope:** Phase 1 of the oracle-compiler baseline
resolution ([docs/ORACLE-COMPILER.md](../ORACLE-COMPILER.md)) · **Tree
changes:** none (scratch compiles only; `tools/gcc-2.7.2/` verified
byte-unchanged against the manifest at the end of the session).

---

## The answer in one paragraph

`tools/gcc-2.7.2/build/cc1` — the frozen oracle compiler — is stock GCC 2.7.2
**with exactly one optimisation deleted**: the PLUS→IOR conversion at the tail
of `case PLUS:` in `combine.c` (lines 3618-3626), which rewrites `a + b` into
`a | b` when `nonzero_bits(a) & nonzero_bits(b) == 0`. Nothing else in
`simplify_rtx` differs, and nothing outside `simplify_rtx` differs anywhere in
the 4.2 MB binary. Deleting those nine lines from a pristine `combine.c` and
rebuilding under the recovered recipe produces a `simplify_rtx` of **exactly**
the shipped size (0x39ab) and byte-identical assembly output on **all 32 TUs**.
The original PsyQ compiler (`cc1psx`) **performs** this conversion, exactly like
stock GCC — so the modification was **not** a fidelity correction. **Verdict:
World A**, with a concrete and cheap migration path.

---

## Q1 — WHAT the shipped compiler does differently

### Disassembly delta

Both binaries place `simplify_rtx` at the same address; the shipped body is
14,763 bytes (0x39ab), the recipe-rebuilt body 14,865 (0x3a11) — 102 bytes and
22 instructions smaller in the shipped build.

Aligning the two disassemblies with absolute addresses, rip-relative
displacements and self-branch targets normalised away leaves **8 hunks**, of
which:

- **6 hunks (0,1,3,4,5,6,7)** are pure basic-block *relocation* — two blocks
  (37 and 53 instructions) sit inline in one build and in the cold tail of the
  other. Instruction-for-instruction identical. This is the ordinary
  `basic-block placement` consequence of removing code, not a semantic delta.
- **1 hunk (2)** is the real difference: 13 instructions present in the
  rebuilt binary with **no counterpart anywhere** in the shipped one.

Call-site census confirms it is an absence rather than a move:

| signature inside `simplify_rtx` | shipped | rebuilt |
|---|---|---|
| `call <nonzero_bits>` | 10 | 12 |
| `call <gen_binary>` | 49 | 50 |
| `mov $0x4a,%edi` (RTX code `IOR`) | 9 | 10 |

### The missing block, decompiled

```
1263ae: shl  $0x3,%eax          ; GET_MODE_BITSIZE (mode)  [mode_size*BITS_PER_UNIT]
1263b1: cmp  $0x20,%eax
1263b4: jg   <bail>             ;   <= HOST_BITS_PER_WIDE_INT (32)
1263ba: mov  0x8(%r12),%rdi     ; XEXP (x, 0)
1263bf: mov  %r15d,%esi         ; mode
1263c2: call <nonzero_bits>
1263c7: mov  %eax,%ebx
1263c9: mov  0x10(%r12),%rdi    ; XEXP (x, 1)
1263ce: mov  %r15d,%esi
1263d1: call <nonzero_bits>
1263d6: test %eax,%ebx          ;   & ... == 0
1263d8: jne  <bail>
1263de: mov  0x10(%r12),%rcx
1263eb: mov  $0x4a,%edi         ; IOR
1263f0: call <gen_binary>
```

RTX codes read off `tools/gcc-2.7.2/rtl.def`: `0x4a = IOR`, `0x4b = XOR`,
`0x4d = ASHIFT`, `0x4f = ASHIFTRT`. The block immediately *preceding* it in
both builds is combine.c:3605-3616 (`(plus x -1)` → `ashiftrt(ashift(xor x 1))`),
which pins the location unambiguously.

### Source characterisation

`tools/gcc-2.7.2/combine.c`, last statement of `case PLUS:` in `simplify_rtx`:

```c
      /* If we are adding two things that have no bits in common, convert
	 the addition into an IOR.  This will often be further simplified,
	 for example in cases like ((a & 1) + (a & 2)), which can
	 become a & 3.  */

      if (GET_MODE_BITSIZE (mode) <= HOST_BITS_PER_WIDE_INT
	  && (nonzero_bits (XEXP (x, 0), mode)
	      & nonzero_bits (XEXP (x, 1), mode)) == 0)
	return gen_binary (IOR, mode, XEXP (x, 0), XEXP (x, 1));
      break;
```

**In two sentences:** the shipped oracle compiler does not rewrite an addition
whose two operands have provably disjoint nonzero bits into a bitwise OR, so
where stock GCC 2.7.2 emits `ori`/`or`, it emits `addiu`/`addu`. The reported
"constant-folding" divergence is a downstream cascade of the same single change —
once the PLUS survives as a PLUS, later combine steps distribute it into the
adjacent shift/subtract instead of stopping at an IOR.

### Draft patch (EVIDENCE ONLY — never applied to the tree)

```diff
--- a/combine.c
+++ b/combine.c
@@ -3615,15 +3615,6 @@
 				 GET_MODE_BITSIZE (mode) - 1),
 	   GET_MODE_BITSIZE (mode) - 1);
 
-      /* If we are adding two things that have no bits in common, convert
-	 the addition into an IOR.  This will often be further simplified,
-	 for example in cases like ((a & 1) + (a & 2)), which can
-	 become a & 3.  */
-
-      if (GET_MODE_BITSIZE (mode) <= HOST_BITS_PER_WIDE_INT
-	  && (nonzero_bits (XEXP (x, 0), mode)
-	      & nonzero_bits (XEXP (x, 1), mode)) == 0)
-	return gen_binary (IOR, mode, XEXP (x, 0), XEXP (x, 1));
       break;
 
     case MINUS:
```

**Verification of the patch (`tmp/fx_patchtest.sh`, scratch tree only):**

```
simplify_rtx size:   SHIPPED 0x39ab   PATCHED 0x39ab   STOCK 0x3a11
patched-cc1 vs SHIPPED over 32 TUs:  (no differences)
*** CONFIRMED: removing combine.c's PLUS->IOR conversion reproduces the
    shipped compiler's behaviour on all 32 TUs. ***
```

The patched binary is not byte-identical to the shipped one (host-toolchain
metadata differs, as the prior investigation already established for every
rebuild) — but it is **behaviourally identical**, and the one symbol that
previously differed now matches to the byte.

### The behavioural consequence, in full

Five instruction lines across three sites in two TUs — the complete set:

| # | TU / function | shipped (oracle) | stock GCC |
|---|---|---|---|
| A | `ings` / `gnd_disp_loop_ctrl` | `addu $17,$2,4` | `ori $17,$2,0x4` |
| B | `code6cac_b` / `func_80033DF4` | `addu $4,$4,$2` | `or $4,$4,$2` |
| C | `ings` / `cpu_set_move_command_and_dir_for_no_action_2` | `sll $3,$3,8` · `addu $3,$3,-128` | `addu $3,$3,-1` · `sll $3,$3,8` · `addu $3,$3,128` |

---

## Q2 — The World A/B discriminator

### Three-way comparison on minimal reproducers

`tmp/fx_repro.c`, compiled by all three compilers with the project's flags
(`tmp/fx_threeway.sh`). "IOR" = the transform fired.

| probe | C expression | rebuilt (stock) | shipped (oracle) | cc1psx (PsyQ) |
|---|---|---|---|---|
| r1 | `(x & 3) + 4` — site A shape | `ori` | **`addu`** | `ori` |
| r2 | `(t<<2) + t`, `t∈{0,1}` — site B shape | `or` | **`addu`** | `or` |
| r3 | `(a&1) + (a&2)` — the comment's example | `andi 3` | `andi 3` | `andi 3` |
| r4 | `((v-1)<<8) + 128` — site C shape | `ori` | **`addu`** | `ori` |
| r5 | `(hi<<8) + lo`, `lo` u8 | `or` | **`addu`** | `or` |

Extended to the 20 further spellings in `tmp/fx_sweep.c` and
`tmp/fx_sweepbc.c`: **cc1psx agrees with stock GCC on every single variant,
and disagrees with the shipped compiler on every variant where the transform
is reachable.** There is no probe on which shipped and cc1psx agree against
stock.

### Verdict: **World A** — the modification is not a fidelity correction

Per the discriminator rubric, `cc1psx == rebuilt != shipped` is the strong
World A signal. The genuine PsyQ compiler performs the PLUS→IOR conversion;
the shipped `build/cc1` is the only compiler of the three that does not.
**Confidence: high** — five independent expression shapes, plus twenty spelling
variants, all one-directional.

### The apparent counter-evidence, and why it does not survive

The original game binary sides with the *shipped* compiler at every divergent
site — `asm/funcs/gnd_disp_loop_ctrl.s:55` has `addiu $s1, $v0, 0x4` after
`andi $v0, $v0, 0x3`, and `asm/6CAC.s:31636` (0x80033F74) has
`sll $a0,$v0,2 ; addu $a0,$a0,$v0`. A repo-wide scan finds **zero** instances
of the `sll d,s,2 ; or d,d,s` multiply-by-five shape anywhere in the game.

That looks like World B evidence, and it is the reason this question needed
settling rather than asserting. It does not hold up: **the target shape is
reachable in pure C under stock GCC.** The transform only fires when combine
can substitute the masking/shifting operation directly into the PLUS, which
requires the intermediate to be single-use. Give it a second use, or let the
result feed further index arithmetic — the natural shapes for what these
functions actually do — and stock GCC emits the add:

```c
/* tmp/fx_sweep.c v12 — stock GCC and cc1psx both emit the TARGET shape */
int v12(void) { int i, v = src(); v &= 3; v += 4;
                for (i = 0; i < v; i++) sink(i); return v; }
    ->  andi  $17,$2,0x0003
        addu  $17,$17,4          <- target: addiu $s1, $v0, 0x4
        beq   $17,$0,.L17        <- target: beqz  $s1, .L800176E4
```

That is the target's instructions, in the target's order, in the target's
register. The current `src/ings.c:707-709` writes the result into a *different*
variable (`v0 &= 3; s1_var = v0 + 4;`), which is exactly the single-use shape
that lets combine substitute and fold. The oracle compiler's missing transform
is masking a C-source reconstruction that is one statement away from correct.

Site B behaves the same way: `tmp/fx_sweepbc.c` `v3` — the multiply-by-five
feeding an index add, which is what `func_80033DF4` actually does — emits
`sll ; addu` under stock GCC and under cc1psx.

### Where the modification came from

The compiler repo was cloned at **2026-05-18 00:09:50** (`git reflog` in
`tools/gcc-2.7.2`); `build/cc1` is dated 13 minutes later. That day's project
history is entirely a "is the compiler wrong?" investigation —
`19fdf974` cc1psx-vs-decompals byte diagnostic, `32b2da96` "cc1psx diagnostic
at scale: 0/16 functions are COMPILER_FIXABLE", `bc92cf02` the
`compiler-patch-low-roi` finding, `7f03bfc9` the KMC-vs-PSX identity check.
No commit names the combine.c experiment — as expected, since
`tools/gcc-2.7.2/` is gitignored and the edit left no trace. The shape of the
finding matches that day's activity exactly: a compiler-patch experiment run
during the "compiler-patch route" evaluation, whose output binary then became
the project's compiler (`Makefile:12 CC1 := tools/gcc-2.7.2/build/cc1`) and was
never reverted. The `compiler-patch-low-roi` memory records the *conclusion*
that the route was worthless — it does not record that a patched binary had
already been left in place.

Note the layering this created: the instrumented diagnostic cc1
(`tools/gcc-2.7.2/cc1`) has the transform, so it and the oracle compiler
disagree — which is precisely the `UNFAITHFUL_STEMS` gap in
`tools/ra_solver/local_extract.py`.

---

## Recommended Phase-2 path

**Migrate to the reproducible baseline.** The World A verdict removes the only
argument for keeping the frozen binary: it is not preserving original-compiler
fidelity, it is preserving an undocumented local patch that silently
contradicts both stock GCC 2.7.2 and Sony's own compiler, and that stands in
direct tension with the project's own `no-compiler-divergence` hard rule.

The migration is small and bounded:

1. Rebuild cc1 from pristine sources under the recovered recipe (`-O0`
   everywhere, `combine.o` at `-O`, `-fgnu89-inline`) — already demonstrated
   to work; `tmp/ab/cc1.mixed` is such a binary.
2. Re-match three sites, not seven — sites A and B have demonstrated pure-C
   forms already (`tmp/fx_sweep.c v12`, `tmp/fx_sweepbc.c v3`); site C is in
   `cpu_set_move_command_and_dir_for_no_action_2`, which carries 25 regfix
   rules and is INCOMPLETE regardless, and its `addu $3,$3,-128` is *already*
   patched by a regfix `subst` today.
3. The two C fixes are the ordinary single-variable accumulation shape the
   project already sanctions ([[split-init-accumulation-sanctioned]]) — they
   are not cheats and need no new policy.

Keeping the frozen binary (option (a) in `ORACLE-COMPILER.md`) remains
*possible*, but it should now be recorded as knowingly building the game with
a compiler that no upstream, and no original-hardware vendor, ever shipped —
and as leaving at least one C function (`gnd_disp_loop_ctrl`) permanently
mis-reconstructed with no way to detect it, because the compiler patch hides
the symptom.

The frozen binary must not be replaced, rebuilt over, or "fixed" until the
owner rules; nothing in this session touched it.

---

## Reproduction

All scripts are scratch-only and never write to `tools/gcc-2.7.2/` or `build/`.

| script | what it does |
|---|---|
| `tmp/fx_dis.sh` | disassemble `simplify_rtx` from both binaries |
| `tmp/fx_align.py` | address-normalised alignment → the 8 hunks |
| `tmp/fx_probe.sh` | call-site census + RTX code numbering from `rtl.def` |
| `tmp/fx_tudiff.sh` | the 5-line behavioural delta across `ings` / `code6cac_b` |
| `tmp/fx_target6.py` | what the ORIGINAL binary has at each site; repo-wide scan |
| `tmp/fx_threeway.sh` | the three-way discriminator table |
| `tmp/fx_sweep2.py` | C-spelling sweeps (`fx_sweep.c`, `fx_sweepbc.c`) |
| `tmp/fx_patchtest.sh` | builds the patched cc1 in scratch; proves 32/32 parity |

Compiler-tree integrity after the session (matches the
`ORACLE-COMPILER.md` manifest):

```
24c5952113d88cbb96f5c9f7e7152147d1efb8a7  combine.c
045c9543d39ab8109583b92137c7adde084f7a25  build/cc1
8384fd47cb51da369462a0ba0b83590eea88513a  cc1
```
