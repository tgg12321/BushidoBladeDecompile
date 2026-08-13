# Evidence — func_800470B0 (src/sound.c)

## Session 1 (2026-08-12, modality: recon) — MATCHED

Outcome: sandbox `--disable all` distance **0**, full build SHA1 ==
`62efab4f73f992798c43e8c730aa43baa10bb4fa` (oracle). Zero regfix/asmfix rules,
zero cheat-asm, zero sanctioned-exception constructs.

### Floor history this session
| form | score | insns (ours/target) |
|---|---|---|
| HEAD (with `register asm("s1"/"s2")` pins + `volatile s32 _sp_pad[2]`) | 22 | 92/88 |
| pins + pad removed, plain locals | 17 | 88/88 |
| `s16 sp10[9]` -> `MATRIX sp10` | 7 | 88/88 |
| params retyped `void *` -> `s32 *`, param aliases dropped | 7 | 88/88 |
| `func_80052930(&sp10, var_s0, arg2)` (third argument restored) | **0** | 88/88 |

Note the HEAD body's cheats were actively COSTING four instructions: stripping the
pins and the volatile pad dropped 22 -> 17 and brought the instruction count from
92 to an exact 88/88. The pins were never load-bearing.

### Facts established
1. **`canonical` verdict is C** (`asm_insns` 0, total 88, distance 22 <= 50).
2. **The stack local is a PsyQ `MATRIX`, not `s16[9]`.** Target reserves a 0x40
   frame: 0x00-0x0F incoming-arg home, 0x10-0x2F locals (32 bytes), 0x30-0x3F the
   four saved regs (s0, s1, s2, ra). Only `m[3][3]` (sp+0x10..0x21) is ever
   written; the remaining 10 bytes are `MATRIX`'s `u16 pad` + `s32 t[3]` tail.
   `include/gte.h:29` — `typedef struct MATRIX { s16 m[3][3]; u16 pad; s32 t[3]; }`
   — is exactly 32 bytes. `s16 sp10[9]` gives 18 -> 24 rounded, i.e. a 56-byte
   frame, and that 8-byte shortfall was what the removed `volatile s32 _sp_pad[2]`
   had been faking. `src/sound.c` did NOT include `gte.h` before this session.
3. **`func_80052930` takes THREE parameters and the decompilation was passing
   two.** Corroboration, all pre-existing in-tree: `src/sound.c:46`
   `extern void func_80052930(void *, void *, void *);`; `src/sound.c:424`
   `camera_Transform` calls `func_80052930(&g_cam_matrix, a0, a1)` i.e.
   (matrix, src, dst); `src/text1a_post.c:166` calls it with three arguments;
   `include/m2c_context.h:738` prototypes `func_80052930(GameObj *, GameObj *,
   GameObj *)`. Restoring the dropped destination argument closed the last 7
   points at zero instruction cost — `arg2` is already live in `$a2` from the
   incoming ABI at the call point, so GCC coalesces the argument copy away.
4. **Why the missing argument showed up as a register swap.** Before the fix, the
   entire residual was arg2/arg3 landing in the wrong callee-saves: `.greg` showed
   pseudo 74 (arg2) in hard reg 18 = `$s2` and pseudo 75 (arg3) in hard reg 17 =
   `$s1`; target wants arg2 -> `$s1`, arg3 -> `$s2`. Both pseudos had n_refs == 4,
   but arg3's last use is the second `subu` while arg2's is the final `sw`, six
   insns later — so under `global.c`'s `allocno_compare`
   (priority ~ `floor_log2(n_refs)*n_refs*size / live_length`) arg3 outranked arg2
   and was allocated the lower-numbered callee-save first. The third call argument
   raises arg2's n_refs to 5, `floor_log2(5)*5 = 10 > floor_log2(4)*4 = 8`, which
   flips the ranking. This is the DIAGNOSIS of the symptom; the JUSTIFICATION for
   the change is fact 3 (the callee's own prototype), not this paragraph.
5. **Store order among the three destination writes is load-bearing.** The source
   order must be `arg2[5]`, `arg2[6]`, `arg2[7]`; moving the `arg2[6] = arg3;`
   store to first or to last each regressed 7 -> 8. Banked in `rejected/`.
6. **Symbol identities confirmed** (`symbol_addrs.txt:145-147`):
   `g_cam_fov_x = 0x800F62F8`, `g_cam_fov_div = 0x800F62FA`,
   `g_cam_fov_z = 0x800F62FC` — the `arg0 * 0x60`-strided per-camera table the
   target indexes via `lui $at / addu $at,$at,$v1 / lh`.
7. **Cosmetic, byte-neutral:** retyping `arg1`/`arg2` from `void *` to `s32 *` and
   indexing `arg1[5]` instead of `*(s32 *)((s8 *)arg1 + 0x14)` measured identical
   (7 both ways, pre-fix). Kept for readability and for parity with the sibling
   `camera_Transform(s32 *a0, s32 *a1, s32 a2)`.

### Transferable lesson for the rest of the queue
Two generic defects, both found by *reading the target's shape* rather than by
searching C variants:
- A stack local whose reserved size exceeds the decompiled array's size is a
  candidate **type** correction (a real SDK struct with a trailing tail), not a
  frame-padding problem. Check `sizeof` of the SDK type the callee expects before
  reaching for any frame lever.
- A callee-save register **swap** between two same-refcount parameters is an
  `allocno_compare` ranking tie decided by live_length; the honest way to move it
  is a genuine extra *use* of the losing parameter. Before treating that as a
  coercion problem, check whether the function is under-passing arguments relative
  to the callee's prototype and its other call sites — a dropped argument is free
  refcount AND a correctness bug.

### Artifacts
- `tmp/grind/func_800470B0/s1/sound.i.greg` — RTL register dispositions (the
  pseudo 74/75 -> hard reg 18/17 evidence).
- `tmp/grind/func_800470B0/s1/sound.i.{rtl,jump,cse,loop,cse2,flow,combine,sched,lreg,jump2,dbr,sched2}`
  — full `-da` dump set.
- `tmp/grind/func_800470B0/s1/sbs.txt`, `target.txt`, `ours.txt` — side-by-side
  instruction diff at the 17-point state (frame size + s1/s2 swap visible).
- `tmp/grind/func_800470B0/s1/diffit.sh`, `greg.sh` — the two probe scripts.
- `tmp/grind/func_800470B0/s1/sound_head.c` — HEAD copy of src/sound.c before edits.
