# Evidence bank — func_8003D52C

## s1 (recon, 2026-09-02) — MATCHED, sandbox --disable all = 0 (146/146)
- Chassis: HEAD cfb17b8e, src/code6cac_c2.c, INCLUDE_ASM at dispatch. canonical: verdict C, distance 38.
- Retired-chassis body (memory/grind/func_8003D52C/retired-chassis-2026-08/body.c) matched only with
  `register ... asm("s3"/"s1")` pins plus one regfix insert (`sw $21,1340($29)` @19). Both are cheats;
  stripping the pins and keeping the builtin va_start macro leaves the honest floor at 38.
- Fact A (measured): with va_start = `((ap) = (va_list)(&(last) + 1))` (identical to the macro
  src/main.c:1707 uses) cc1 emits `sw $20,1340($sp)` = target `sw $s5,0x53C($sp)` — the store of
  first_arg into its argument home slot. Mechanism: the parm is copied to a pseudo at
  expand_function_start (body not yet parsed, TREE_ADDRESSABLE unknown); `&first_arg` in the body
  triggers put_var_into_stack → parm_reg_stack_loc (tools/gcc-2.7.2/function.c:1343) and the
  pseudo→home-slot store. With `__builtin_next_arg` the store is absent: measured d1 on the otherwise
  matching v3 body (145 insns).
- Fact B (measured): the first-loop segment write pointer and the second-loop read pointer must be
  ONE local. Separate locals → d38 (segment pointer allocated $v0, whole callee-save chain shifted:
  fmt s1, ap s2, seg-start s3, first_arg s4, seen s5, buf s6, '%' s7). Merged → d0. Mechanism:
  global.c find_reg (tools/gcc-2.7.2/global.c:972) uses fixed_reg_set for a pseudo with
  allocno_calls_crossed == 0, so caller-save $v0 is the first free reg; the merged pseudo crosses
  func_8003D39C so it draws from the callee-save set, and its priority places it right after `ch`
  ($s0) → $s1.
- Fact C (measured): `pct = 0x25` / `seg_start` / `buf_ptr` holder locals from the retired body are
  byte-neutral; the literal-'%', `p = seg`, `buf + strlen(buf)` spelling (v3) scores 0 too.
- Pre-existing tree quirk: include/code6cac.h:484 declares func_8003D52C as (s32,s32,s32,s32) and
  :501 declares sprintf as (s32*,s32,s32,s32); cc1 reports "conflicting types" for both, but the
  committed tree already carried the sprintf conflict (HEAD src/code6cac_c2.c:952) and still built
  to the oracle, and the sandbox emitted all 146 insns. Header untouched this session.
- Artifacts: tmp/grind/func_8003D52C/s1/{v1.c,v2.c,v3.c,apply.py,v1-cc1-output.s},
  tmp/grind/func_8003D52C/dumps/ (cc1 -da pass dumps of the v1 body).
