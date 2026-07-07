# Evidence bank — math_Distance3D

- Audit diagnosis (regressions.md): out[4] has a dead 4th element (out[3] is never written by the callee or read by the caller) that inflates the array to 16 bytes to coerce $ra to sp+0x30 and the frame to 0x38 — frame-coercion by dead array element. Worker must find a pure-C form that reaches the correct frame layout without the dead element, or confirm via GCC run whether out[3] produces the same layout.  (committed code flagged by the re-audit patrol; review and re-do in pure C if confirmed. The byte-correct construct stays on main until a clean replacement lands.)

- [s1] [fable-blitz 2026-07-07] Target layout (asm/funcs/func_80017748.s): frame 0x38, $ra at sp+0x30, in[] at sp+0x10..0x18, out[] at sp+0x20 with only sp+0x20/0x24/0x28 read back after the func_8007F0BC call. The 4th out word (sp+0x2C) is never written or read in target bytes either — it is pure frame space, no dead stores.

- [s1] [fable-blitz 2026-07-07] Callee func_8007F0BC is user-authorized canonical hand-written GTE sqr-1 leaf (inline_asm_canonical.txt:265): lwc2 $9/$10/$11 from *a0, sqr 1, swc2 $25/$26/$27 (MAC1-3) to *a1 — writes EXACTLY 3 words. Judge's dead-4th-element diagnosis is factually confirmed: out[3] is never written by the callee nor read by the caller (src/ings.c:789 sums out[0]+out[1]+out[2]).

- [s1] [fable-blitz 2026-07-07] Alignment mechanism (named pass: cc1 frame layout, function.c assign_stack_local ~line 678): BLKmode locals get alignment = BIGGEST_ALIGNMENT/BITS_PER_UNIT = 8 bytes (mips.h:1082 BIGGEST_ALIGNMENT 64). This is why out lands at sp+0x20, not sp+0x1C: frame_offset CEIL_ROUNDs 0x0C->0x10 above the locals base. The 4-byte gap at sp+0x1C exists in target and is produced by alignment alone, independent of out's element count.

- [s1] [fable-blitz 2026-07-07] Frame-size arithmetic (mips.c compute_frame_size): locals span with out[4] = 0x10(in,padded)+0x10 = 0x20; with out[3] = 0x10+0x0C = 0x1C, and MIPS_STACK_ALIGN(0x1C) = 0x20 — SAME var_size, hence same total 0x38, same $ra offset 0x30, same in/out addresses (sp+0x10 / sp+0x20). Prediction: `s32 out[3];` is byte-identical; the out[4] frame-coercion construct is unnecessary, exactly the judge's suggested confirm-via-GCC-run branch.

- [s1] [fable-blitz 2026-07-07] The plausible ORIGINAL type is PsyQ's VECTOR struct {long vx,vy,vz,pad;} (16 bytes with a semantically-real pad member) — this function is a GTE vector-magnitude wrapper (subtract, /4, gte-sqr, sum, sqrt-helper func_8007E43C, *4), the exact usage domain of VECTOR. If out[3] somehow diverges, a VECTOR-typed local is the clean human-natural 16-byte spelling, not a cheat.

- [s1] [fable-blitz 2026-07-07] Twin function math_Distance3D_16 (0x800177C8, src/ings.c:791) has the identical body with >>4/<<4 and the identical s32 out[4] construct, but was marked reaudit-clean 2026-06-26 (docs/fleet/log.jsonl:1907) — inconsistent patrol verdicts on the same construct. Whatever fix lands here should be mirrored there for consistency.

- [s1] [fable-blitz 2026-07-07] No memory/wip/math_Distance3D/ checkpoint exists (memory/wip holds only README.md); nothing to convert-wip.
