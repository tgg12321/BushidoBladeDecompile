# func_8002A458 — hypotheses (manual session 1, 2026-09-25)

Scores are sandbox --disable all on the full candidate (416 target insns)
unless marked "fast" (objdump diff-line count on a trimmed TU,
tmp/a458/fast.py; 0 == sandbox 0 in every case checked).

## Ablations of the landed form (each removes ONE construct from candidate.c)
| id | change | score |
|----|--------|-------|
| A10 | no do-while(0) around the rec init | 52 (rec/scr and id/obj seats swap) |
| B2 | separate `h_sq` squared length, separate `tbl` (no len/lzc_in reuse) | 2 (copy lands in $v1) |
| B3 | `len` reuse kept, separate `tbl` | 2 (copy in $v1) |
| B4 | separate `h_sq`, lzc_in holds the table byte | 4, 417 insns (lzc_in becomes CSE-canonical) |
| A5 | end block with six fresh locals | 28, 417 insns |
| A6 | end block with a fresh trio reused for both distances | 28 |
| A7 | `shift = 0x16 - (sp_tmp & ~1)` at site 1 | 3 |
| A9 | dx/dy/dz declared in natural order (was dz,dy,dx) | 0 (natural order kept) |

## Killed before the do-while(0) was found
| id | spelling | result |
|----|----------|--------|
| a | end block with fresh temporaries | s2/s3/s1 vs $v0; KILLED |
| b | end block reusing `len_sq` for the first distance | 66; KILLED |
| d | `s16 id` | 116, 420 insns; KILLED |
| e | rec recomputed per iteration (`&D_800F5F68[id*0x1B8 + i*0x14]` etc.) | 87-164, 411-413 insns; KILLED |
| f | rec as `struct hitrec *` walked with `rec++` | same as u8* walk; neutral |
| g | rec as `struct hitrec *` indexed `rec[i]` | 108, 411 insns; KILLED |
| h | LZC copy inside the else arm | combine folds it into the asm; KILLED |
| i | one copy variable shared by both sqrt sites | site-2 sum moves to $a1; KILLED |
| j | scalar 3-word copy instead of the struct copy | 83, 414 insns; KILLED |
| k | id load before the struct copy (no wrap) | obj beats id; KILLED |
| l | `((s16 *)obj)[2]` in-struct id load | neutral |
| m | explicit `off = id * 0x108` | 20 / 49; KILLED |
| n | pre-header orders (i=0;rec / comma forms / do-while loop) | 6 / 40 / 44 / 40 |
| o | `rec = D_800F5F68; rec += id * 0x1B8;` (Ruling-4 split) | 9 (la lands in rec's reg) |
| p | do-while(0) wrapping the if-block / the loop / from printf to done | 184-211; KILLED |
| q | param alias `u8 *obj = arg0;` variants | 32-40; KILLED |
| r | loop temporaries reusing dx/dy/dz (i:=dy, pos:=dx, bit:=dz) | 110-130; KILLED |
| s | header-exact PsyQ islands (6 statements per gte_Lzc) | fixes rec/scr by itself, but the engine strips the GPR-only statements (sandbox 99); banked rejected/header-exact-islands-objid-swap.c |

Permuter: perm1 (v17 chassis, random mode) 8,098 iters, 0 finds; perm2
(header-exact chassis) zero at iter 716 = do-while(0) around the final
func_80032854 call (loop-depth ref weighting on id). Both harvested/stopped.
