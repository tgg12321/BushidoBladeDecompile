# Evidence bank — calc_fc_frame

## s1 (recon, 2026-07-28) — SOLVED: sandbox 0, pure C, zero rules

Baseline: verdict C, honest floor 14, 72/72 insns (pure RA/scheduling gap; 11
regfix rules masked it). Final: **sandbox --disable all = 0** with plain C in
src/text1a_c.c (candidate.c here is the exact body).

Measured facts (each confirmed by sandbox + objdump diff):

1. **The old C was semantically wrong.** It did `src_base += 4;` then called
   `func_800520B8(src_base + start, ...)` — i.e. (base+4)+start. Target calls
   with ORIGINAL base + start (`addu a0,$s6,a0`, $s6 = un-incremented a0).
   Rules `insert addiu $4,$4,4 @28` / `delete @34` / `subst $22→$4 @37` were
   patching that semantic difference. Fix: `src_orig = src_base;` copy at top
   (→ $s6), call uses `src_orig + start`; the param itself is incremented
   (`src_base += 4`) and stays live in $a0 (→ `addiu a0,a0,4` in place,
   `move s1,a0` for src_ptr).
2. **Param liveness drives the a1/a2 temp allocation.** With src_base live in
   $a0 until mid-function, the count/scan temporaries take $a2/$a1 (target).
   Any form that frees $a0 early (computing +4 off $s6) shifts them to
   $a1/$a0 (the old `$4<->$5` rules).
3. **Round-down shift must be two statements** (`size = (u32)size >> 2;
   size = size << 2;`) for in-place `srl s0,s0,2; sll s0,s0,2` — identical to
   completed sibling hirahira_w_frie. Single expression goes through $v0
   (old subst rules @49/50).
4. **dest cursor = the a1 param directly** (`*dest_arr = count; dest_arr++`),
   `orig_dest` is the copy → cursor $s4 from a1 in prologue, `move s5,s4`
   sinks into the first beq delay slot (old `$20<->$21` rules).
5. **s6/s7 allocation is decided by DECLARATION order** (pseudo creation):
   `s32 src_orig;` must be declared BEFORE `s32 sentinel;` when the
   src_orig statement precedes orig_dest's. Statement-order-only fixes flip
   src_orig to $s7 (floor 8/12 — see rejected/).
6. **`fp++` is unconditional in the original** (`val = *fp; fp++; if (...)`).
   The target's beq delay slot `addiu s3,s3,2` executes on both paths; with
   fp++ between the lh and the branch in RTL it is reorg's nearest eligible
   backward candidate and fills the slot, which keeps `addiu a0,a0,4` at its
   source position (otherwise the a0 increment sinks into the slot — the
   final floor-2 residual).

Artifacts: tmp/grind/calc_fc_frame/s1/ (build*.dis, v5_score2.c,
final_match.dis).
