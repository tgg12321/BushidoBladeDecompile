# Evidence bank — _addque2

## s1 (2026-09-06, recon) — 184 -> 0 (raw) / 18 (honest un-granted); verify-oracle ok

OBJECT MODEL: D_80103680 / D_80103684 / D_80103688 / D_8010368C — MISMATCH (measured).
The four splat per-word scalars are one object: a 64-slot table of 0x60-byte records
{ func @0, arg @+4, count @+8, data[21] @+0xC }. Evidence independent of this session:
(a) stride — every slot access in asm/funcs/_addque2.s computes index*0x60
(sll 1 / addu / sll 5 = x96) and adds it to %hi/%lo(D_8010368x) (0x8007D598-5A8,
5C8-5F4, 60C-620, 630-644, 654-668); asm/funcs/_exeque.s does the same on D_8009BF7C;
(b) base-register — the copy loop keeps `&D_8010368C` in $t0 and stores through
$a0 = i*4 + slot*0x60 + $t0; (c) the committed naming census (named_syms.txt:1004,
:1005, :1272, :2558) already names them "64 entries x 96 bytes ... 0x60-byte slot",
and the SOTN original is `static volatile struct QueueItem D_80037F54[0x40]`
(sotn-decomp src/main/psxsdk/libgpu/sys.c:95, PSX GCC 2.7.2 module). Measured: the
per-word form has no C body (184); the aggregate `extern GpuQueueItem D_80103680[64]`
measures 29 with all slot stores byte-identical (%lo(D_80103684/88/80) + index*0x60;
pairdiff tmp/grind/_addque2/s1/h1_pairdiff.txt shows no diff on them).
D_8009BF78 / D_8009BF7C — MATCH (volatile s32, allowlist lines 76 / 29; the plain reads
re-load per use exactly as the target does; no `*(volatile *)&` casts needed — the
retired-chassis body's casts are obsolete). D_8009BF68 — MATCHES as
`extern volatile s32 D_8009BF68[]` (allowlist line 75): `D_8009BF68[0] = (s32)func`
emits the target's unfolded lui/addiu/sw 0($v0). D_8009BE75 (u8), D_8009BE7C,
D_8009BE80, D_8009BF6C, D_8009BF70, D_8009BF80 (s32) — MATCH as declared.
D_8009BF48 — MISMATCH at the :720 declaration site (measured 29 -> 22): declared
`s32 *` there but `volatile s32 *` at :747; the :720 site is the one _addque2 sees
(function-at-a-time compile), and the .loop dump (tmp/grind/_addque2/dumps/display.loop
lines 7498-7534) shows loop.c hoisting `mem:SI (reg 94)` — the GPU_STATUS read — out
of the spin-wait, leaving a self-branching `beqz`. Pointee = 0x1F801814, type-level
MMIO volatile (mmio-volatile-type-level), ordinary C. D_8009BF54 — MISMATCH (measured
22 -> 20): `s32 *` at :732 and :795 -> `volatile s32 *` (pointee DMA2_CHCR 0x1F8010A8;
same class as get_alarm's g_gpu_dma_madr lever (b), decisions.md 2026-09-04 PASS);
_reset's two stores through it still build to the oracle bytes.

- Chassis: HEAD a7147993, INCLUDE_ASM baseline 184, cheat_asm_stripped 147 (146 once a C
  body replaces the INCLUDE_ASM; +1 = 147 when the un-granted `extern volatile
  GpuQueueItem D_80103680[64]` is present and stripped).
- Measurement chain (sandbox --disable all unless noted): 184 -> 29 (aggregate body,
  SOTN sys.c:744 shape) -> 22 (BF48 pointee volatile) -> 20 (BF54 pointee volatile)
  -> 2 (volatile queue array; measured with the census name `g_gpu_packet_queue_base`,
  see the detector note) -> 0 (`arg[i]` copy loop). Under the splat name `D_80103680`
  the same body measures 18 honest (stripped) and 0 with `--keep-cheat-asm`
  (184/184). `verify-oracle` with the volatile queue live: ok, build_sha1 ==
  62efab4f73f992798c43e8c730aa43baa10bb4fa (whole-image proof incl. _reset, _sync,
  get_alarm, which share these declarations).
- MECHANISM of the volatile queue (why nothing non-volatile reproduces it): reorg.c
  resource_conflicts_p treats a MEM_VOLATILE_P store as conflicting with everything
  (reorg.c:363/668 set res->volatil, :752 predicate), so the copy-loop slot store
  cannot fill the back-jump delay slot and reorg falls back to the loop-top `move v0,s1`
  (thread fill) — the target's exact shape; and sched.c:811/831/855/873 make volatile
  refs mutually dependent, which pins the three D_8009BF78 reloads after the preceding
  volatile slot stores instead of interleaving them (h3_pairdiff hunks at 140-165).
  This is ORIGINAL SEMANTICS: SOTN's sys.c declares the queue volatile.
- DETECTOR NOTE (do not exploit): engine/volatile_cheats.py:52 exempts `^g_` names, so
  `extern volatile GpuQueueItem g_gpu_packet_queue_base[64]` scored 0 without being
  stripped (cheat_asm_stripped stayed 146). The candidate deliberately uses the splat
  name `D_80103680` so the gate sees it; landing needs the allowlist grant, not a name.
- Family: `extern volatile T G;` on Sony census module state = Ruling 4 class grant
  (commit c80d976e; text at `git show cd19d7a2^:docs/closer/rulings.md` lines 68-83).
  Census prong: D_80103680 consumers are exactly _addque2 (0x8007D3F8), _exeque
  (0x8007D6D8), _reset (0x8007D9C4; memset), all inside the verbatim LIBGPU/SYS region
  0x8007AE7C-0x8007DF10 (memory/closer/psyq-library-census.md:76; row :244 names
  _addque2). Codegen prong: 18 -> 0 on the same chassis. Precedent grants: allowlist
  lines 75-76 (D_8009BF68, D_8009BF78), Judge PASS docs/grind/decisions.md 2026-09-04
  14:33. The IRQ carve-out is NOT claimed (the use sites are stores, not spin-waits).
- Aggregate merge prongs: (a) stride + base-register + census schema above; (b) struct
  of records; (c) D_80103684/88/8C removed from C (grep src/ include/: only display.c
  named them; asm/funcs/_exeque.s still does -> rows stay with the `alias of
  D_80103680+N; retire with _exeque` suffix); (d) header-canonical placement is the
  handoff step (include/gpu.h is outside session scope); (e) verify-oracle ok.
- Prototype: `s32 _addque2(s32 (*func)(s32 *, s32), s32 *arg, s32 len, s32 count)`;
  `_addque` (matched, :717) calls it with s32 args — warning only, bytes unchanged
  (oracle ok). SOTN's is `int _addque2(void (*)(int*, int), int*, int, int)`.
- Artifacts: tmp/grind/_addque2/s1/{h1,h3,h4}_pairdiff.txt, candidate-diff.txt
  (git diff -U0 of src/display.c), display_h5_gname_score0.c, display_h3.c,
  tmp/grind/_addque2/dumps/display.{loop,sched,combine}.
