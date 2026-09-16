# REJECTED s7 (2026-09-16) — every SINGLE-EXPRESSION bank-address preamble

All of the forms below were measured on the s7 chassis (the score-0 body of
`candidate.c` with ONLY the `bank` preamble replaced). Every one of them
measures **score 2, build_insns 200, target_insns 200** — the whole residual
being that the `la $v1,_ss_score` insn is emitted AFTER both halves of the
index shift instead of between them (target: `sll $v0,$a3,16` /
`la $v1,_ss_score` / `sra $v0,$v0,14`).

Sources are in `tmp/grind/_SsSndCrescendo/s7/`:

| file | `bank = ...` spelling | score |
|---|---|---|
| v5.c | `(s32 *)((u8 *)&_ss_score + ((s32)(a0 << 16) >> 14))` | 2 |
| v6.c | `(s32 *)(((s32)(a0 << 16) >> 14) + (u8 *)&_ss_score)` | 2 |
| v7.c | `&_ss_score + a0` | 2 |
| wa.c | `(s32 *)((u8 *)&_ss_score + (s16)a0 * 4)` | 2 |
| wb.c | `(s32 *)((s32)&_ss_score + ((s32)(a0 << 16) >> 14))` | 2 |
| wc.c | `(s32 *)((u8 *)&_ss_score + (((s32)a0 << 16) >> 14))` | 2 |
| wd.c | `&_ss_score + (s32)a0` | 2 |
| we.c | `(s32 *)(((s32)(a0 << 16) >> 16) * 4 + (u8 *)&_ss_score)` | 2 |
| x1.c | `sc = &_ss_score;` then `sc + a0` (alias FIRST, no index local) | 2 |
| x2.c | `&((s32 *)&_ss_score)[a0]` (re-audit of the s2 instance kill) | 2 |
| y1.c | `idx = a0;` then `(s32 *)&_ss_score + idx` | 2 |
| y2.c | `(s32 *)((u8 *)&_ss_score + (s32)a0 * 4)` | 2 |
| y3.c | `idx = (s32)a0 * 4;` then `(s32 *)((u8 *)&_ss_score + idx)` | 2 |
| y4.c | `idx = a0;` then `(s32 *)((u8 *)&_ss_score + idx * 4)` | 2 |

Mechanism (cc1 `-da` dumps read this session, `main.{rtl,cse,combine,sched}`):
expand emits the s16->int conversion as `ashift:16` + `ashiftrt:16`;
combine.c `try_combine` folds that `ashiftrt:16` with the scale-by-4
`ashift:2` into one `ashiftrt:14` placed at the LATER insn's slot; sched.c
`rank_for_schedule` leaves the two independent insns in RTL/LUID order, so it
never reorders them afterwards. `fold()` moves the constant `ADDR_EXPR` to
operand 1 of any pointer sum, so a single expression always evaluates the
whole index (both shifts AND the scale) before the address — putting the
address insn after the folded `ashiftrt:14` (v5/v6/v7/w*/y*), and putting the
alias statement first puts it before the `ashift:16` (x1). Only the ordered
pair of statements `bank_no = a0;` then `score_tbl = (s32 *)&_ss_score;`
lands it between the two. See `candidate.c` and hypotheses.md H8.
