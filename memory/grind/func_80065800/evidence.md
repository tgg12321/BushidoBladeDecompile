# Evidence bank — motion_SetExMotion

- [s1] Queue entry: distance=1453, verdict=ASM-PARTIAL, rules=1. The single rule is asmfix.txt:220 `motion_SetExMotion: replace_with_asmfile "asm/funcs/motion_SetExMotion.s"` — whole body is original asm; src/text1b.c:14815 is a stub returning 0.

- [s1] Structure (tmp/blitz/motion_SetExMotion_map.txt): 1456 insns @0x80065800-0x80066EBC, frame 0x58, saves s0-s7+fp+ra (all callee-saves). Single jr $ra at 0x80066EB8, returns constant 1 (u8 to callers).

- [s1] Signature: u8 motion_SetExMotion(s32 mode), mode 0..0x11 (18 values). 18 call sites in src/text1b.c:14624-14800 each pass a distinct constant 0..0x11 — per-effect wrapper functions.

- [s1] Two 18-entry jumptables: jtbl_800158F8 (dispatch at 0x800659DC, switch on mode for color/texture setup) and jtbl_80015940 (dispatch at 0x80066160, switch on mode for poly corner placement). Both defined as const u32 arrays in src/text1a_b_pre_rodata.c:365,387. Both switches gated by `sltiu $v0,$s3,0x12; beqz`.

- [s1] GTE region 0x80065874-0x80065958 (asm lines 29-91, all splat-marked 'handwritten instruction'): ctc2 $0-$4 = SetRotMatrix from *D_800A3474; ctc2 $5-$7 = SetTransVector from s0+0x14; lwc2 $0,$1 vertex from s0+0x64; rtps; swc2 $14 (SXY2) -> *D_800A34B8; swc2 $8 -> s0+0x10; cfc2 $31 (FLAG) -> *D_800A34CC; mfc2 $19 (SZ3) >>2 -> *D_800A34D0. Canonical GTE inline-asm category per inline-asm-policy — this is why verdict is ASM-PARTIAL. A call to func_8007EA0C is interleaved mid-region (args s0+0x54, s0+0x14).

- [s1] Full m2c decompile SUCCEEDS: tmp/blitz/motion_SetExMotion_m2c.c (637 lines, 0 structural failures; only the cop2 ops are M2C_ERROR placeholders). Required synthesizing the jtbls as .s (tmp/blitz/make_jtbl_s.py -> tmp/blitz/motion_SetExMotion_jtbl.s) because rodata segments were retired to C arrays.

- [s1] Semantics: draws a 0x28-byte POLY_FT4 effect prim. s0=*D_800A34EC (effect state struct), s2=*D_800A37D4 (prim cursor), advances by 0x28 with pool-bound check `-((s2-D_800A3720)*0x33333333)>>3 < 0x1C1` (magic div by 40); D_800A37D4 written back at exit. ot_Link(D_800A374C+4, prim) links into OT.

- [s1] The lone backward j (0x80066E24 -> .L80065988, span 1319 insns) is NOT a while loop: switch-2 cases 6/7/10/11 end with `if (mode < 9) { ot_Link; advance prim; mode += 2; goto top; }` — re-runs the whole body for the paired effect (mode+2). Loop label sits AFTER the GTE/RTPS region (at the D_800A34A8=0x20 store, 0x80065988).

- [s1] Switch 1 has cross-case control flow: case 5's else-arm falls into case 0's body (m2c lines 230-241: shared `var_v0_4 = &D_8009B8C8` + store block); cases 3/4 share tail block_43 with cases 8/9 (`*D_800A34A8 = var_v1; *D_800A34AC = var_v1`) via different var_v1 (0xC0 vs 0x100); cases 10/11 tail block_35 also reached from 6/7 via goto. Original source used gotos/shared labels across cases.

- [s1] Switch 2 case group 6/10 vs 7/11: case 6/10 head does a conditional `*s4 = -*s4` then FALLS INTO 7/11 (m2c lines 550-571) — a genuine C fallthrough after an if.

- [s1] Only one real div instruction (signed div for *D_800A34B0 / *D_800A34D0 = H*1000/SZ3 screen-scale); all other divisions are strength-reduced multiplies: /105, /3, /15, /10, /255, /4551 (m2c shows them literally).

- [s1] Second small loop 0x8006640C-0x80066458 (19 insns) = the do{...}while(var_s1<4) in switch-2 case 1/2/16/17: 4 corners, sign flip by (i&1)/(i&2), func_8007E8AC(vec, dst) per corner after motutil_GetWalkDir + gte_SetRotMatrix(sp10).

- [s1] 24 calls / 11 unique callees: math_Sin x9, math_Cos x5, ot_Link x2, func_8007EA0C, gte_GetH, initPolyFT4, gpu_SetRawTexture, gpu_SetSemiTransp, motutil_GetWalkDir, gte_SetRotMatrix, func_8007E8AC. gte_SetRotMatrix is called AND inlined-via-ctc2 in the prologue — the prologue block is hand-inline GTE, the case-1 call is the library fn.

- [s1] No WIP checkpoint existed (memory/wip/ has only README). Ledger was a bare init_ledger skeleton (session_count=0) from a prior aborted blitz.

- [s1] regfix.txt contains NO rules for this function (grep clean) — the only cheat is the asmfix replace_with_asmfile.
