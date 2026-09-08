# SELF-VET — CD_datasync

CONSTRUCTS: local pointer alias `tbl_11dc = D_800A11DC;`, local pointer alias `idx_1494 = &g_cd_status_a;`, local pointer alias `tbl_125c = D_800A125C;`, local pointer alias `pp = &D_800F19C0;`, named intermediate `t0 = idx_1494[0];` placed before the wrap, named intermediate `tb = idx_1494[1];`, `do { ... } while (0);` wrap.

## T1 semantic purpose
- The four pointer aliases: each holds the base address of a global the function
  reads repeatedly (the command-name table, the libcd Intr status block, the
  CD_intstr table, the alarm-callback slot). Every one of them is dereferenced on
  the executed path and its value reaches the printf call, so each has an
  observable role in the function's output. What a simpler form produces is
  measurably DIFFERENT bytes, not identical bytes: ablating each one individually
  gives 18/88, 20/90, 27/89 and 4/91 respectively. None is a no-op.
- `t0` and `tb`: each holds one real volatile byte read (`lbu $a0,0($s1)` and
  `lbu $v0,1($s1)` in the target) that is consumed by the printf call. The reads
  are of a volatile object, so they are not removable and their ORDER is
  semantically meaningful; naming them is how the source fixes that order.
- The `do { } while (0)` wrap: this is the one construct with no semantic purpose
  in the ordinary sense — it is a declared match device, which is precisely what
  the sanctioned family covers, and it is FAKE-annotated. Removing it costs 13.

## T2 human-programmer
- Yes for the aliases: caching `Intr`, the command-name table and the CD_intstr
  table in locals is exactly how period Sony libcd code is written, and the same
  handles appear in this TU's already-matched sibling CD_ready
  (src/system.c:396-397, COMPLETED-C on main 2026-09-06).
- Yes for `t0` / `tb`: reading two adjacent status bytes into named locals before
  formatting them is ordinary, readable C; a reader asks nothing about it.
- The wrap is the one construct a reader WOULD ask about. It is not defended as
  natural code; it is defended as a named sanctioned match device with a
  mandatory annotation, which is the family's own terms.

## T3 GCC-internals justification
Each annotation names the pass that the construct acts through — this is required
by the FAKE template ("mechanism: <named GCC pass>"), not offered as the reason
the construct is legitimate. The legitimacy claim for each construct is its
sanctioned family plus lever-exhaustion; the pass name is the third prong of the
owner's 2026-07-01 three-prong policy, present because the policy demands it.
For the four aliases and the two named intermediates the program-level
explanation stands on its own (cached bases; named byte reads). For the wrap
there is no program-level explanation and none is claimed.

## T4 permuter/search provenance
No permuter, no auto-search, no randomized generator was used this session. Every
form was hand-written from an explicit structural hypothesis (remove the address
intermediate) and every construct in the final body was individually ablated and
re-measured. The body is not a search artifact and does not depend on any
detector failing to recognize a spelling.

## T5 family check
Checked against the forbidden catalog line by line. No register-asm pin, no
hardcoded-$N asm, no scheduling barrier, no INLINE_MOVE_ALIASING, no volatile
coercion by alias-rename / cast / plain extern / (void) discard, no unused local
array, no dead-param assign, no dead conditional store, no empty-body if, no
if(1) wrapper, no dead goto label pad, no DImode chain, no goto-end accumulator,
no param-local-alias declaration-order trick, no opaque `one = 1`, no lowercase
asm block, no build-time assembly rewriting, no asm alias rename, no redundant
width cast, no linker-script reorder. The only `volatile` in the body is the
declared type of `g_cd_status_a`, which is already `extern volatile u8` on main
(src/system.c:374, since commit 7e182728) — the local pointer takes that type, it
does not add a qualifier. Every construct maps onto a frozen-list family below.

## T6 naming-announces-intent
No name in the diff is `pad`, `_pad`, `dummy`, `unused`, `spill`, `sp_*`, `_buf`,
`tail`, `slack` or `_frame_pad`. `t0`, `tb`, `pp`, `tbl_11dc`, `tbl_125c`,
`idx_1494` are descriptive of the values they hold (splat symbol suffixes for the
tables). Every one of them is read on the executed path; none is address-of-only,
discard-only, or declaration-only.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: C-level pointer alias to a global (covers tbl_11dc, idx_1494, tbl_125c, pp)
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a C-level local pointer alias (second handle) to a global — `Type* t = &g_Thing;`, `s16 (*p)[] = &D_xxx;`, FakePrim-style reinterpret views — is allowed as a LAST-RESORT lever with `/* FAKE: ... */` annotation + lever-exhaustion. SOTN idiom incl. self-admitted `// n.b.! unused, required for PSP`. asm(\"sym\") alias-RENAMES stay forbidden."
  PRECEDENT: src/system.c:397

  FAMILY: named-intermediate declaration order (covers t0, tb)
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: .claude/rules/no-new-park-categories.md:205

  FAMILY: do-while(0) wrap
  SCOPE: "SANCTIONED (owner ruling 2026-07-06, supersedes the 2026-06-04 mechanism-scoping): `do { ... } while (0);` (any body, incl. empty) is an allowed pure-C match device for ANY codegen effect incl. register allocation, with mandatory inline FAKE annotation; nested wraps need a single-level-insufficient justification. Hard line unchanged: no regfix/pins/inline-asm/semantic-lie C."
  PRECEDENT: src/system.c:424

Named-intermediate six-prong check (owner clarification 2026-08-17 / relaxation
2026-08-31, .claude/rules/no-new-park-categories.md:208), for t0 and tb:
  (1) once-written — yes, each is assigned exactly once and read exactly once;
  (2) real value — each is one of the target's own `lbu` results
      (asm/funcs/CD_datasync.s:50 and :51), consumed by the printf call;
  (3) byte-neutral — build_insns 91 == target_insns 91;
  (4) fresh local, not a borrow — both are new block-scope locals, no
      staged-value-reused-variable claim is made;
  (5) destination not live-pre-initialized — neither has any prior value;
  (6) FAKE annotation + measurements present — yes, with the s62 ablation
      numbers in the annotation and in memory/grind/CD_datasync/evidence.md.

ANNOTATION-CONFORMANCE:
  /* FAKE: pointer alias (second handle) to the libcd command-name table per pointer-alias-fake-exception (owner ruling 2026-07-01), mechanism: global.c seats the base in $s3 across the poll loop as the target does (asm/funcs/CD_datasync.s:11-12); lever-exhaustion: s62 ablation A4 (direct D_800A11DC[] subscript) = 18/88, plus the 61-session ledger in memory/grind/CD_datasync/hypotheses.md */
  /* FAKE: pointer alias (second handle) to the libcd Intr status block per pointer-alias-fake-exception (owner ruling 2026-07-01); the volatile is the TU's own declaration `extern volatile u8 g_cd_status_a;` (src/system.c, on main since 7e182728; ground truth `static volatile CD_intr Intr`, memory/closer/libcd-identity.md:28) - no cast, no local qualifier; mechanism: global.c seats the base in $s1 across the poll loop as the target does (asm/funcs/CD_datasync.s:13-14); lever-exhaustion: s62 ablation A5 (direct (&g_cd_status_a)[n] subscript) = 20/90 */
  /* FAKE: pointer alias (second handle) to the CD_intstr table per pointer-alias-fake-exception (owner ruling 2026-07-01), mechanism: global.c seats the base in $s0 across the poll loop as the target does (asm/funcs/CD_datasync.s:15-16); lever-exhaustion: s62 ablation A3 (direct D_800A125C[] subscript) = 27/89 */
  /* FAKE: named intermediate for the sync byte, placed BEFORE the wrap (loop depth 1), mechanism: flow.c loop_depth-weighted reg_n_refs feeds local-alloc.c qty_compare - the depth-1 mention leaves the merged chain-A quantity below the second table read's priority, so the chain takes $a0 and the value $v1 exactly as at asm/funcs/CD_datasync.s:50/56/60/65; lever-exhaustion: s62 probe B3 (same read placed inside the wrap) = 4/91, s62 probe B1 (inlined into the call) = 12/91, s60 in-place spelling = 15 */
  /* FAKE: do-while(0) wrap per do-while-zero-exception (owner ruling 2026-07-06), mechanism: sched.c:2081 loop-note barrier on the first insn inside (the ready-byte load) orders the sync-byte load ahead of it and every later register-argument load after it, and flow.c loop_depth ref weighting seats tbl_125c in $s0; lever-exhaustion: s62 ablation A2 (wrap removed) = 13/91, s61 ablation 2 -> 12 */
  /* FAKE: named intermediate for the ready byte (fresh, once-written, once-read, real value = lbu $v0,1($s1) at asm/funcs/CD_datasync.s:51), mechanism: expand argument staging keeps the fifth (stack) argument's chain out of the call sequence so the sw lands at slot 63; lever-exhaustion: s62 probe B1 (tb inlined into the call) = 12/91, s62 probe A6 (all intermediates inlined) = 13/91 */
  /* FAKE: pointer alias (second handle) to the alarm callback slot per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape), mechanism: calls.c:1652-1664 expand_call precomputes a register argument whose rtx_cost > 2 into a pseudo inside a loop (preserve_subexpressions_p), whereas `*pp` is a cheap mem(reg) that stays in the call sequence and cse folds the alias back to the target's `lui $a1 / lw $a1` at asm/funcs/CD_datasync.s:52-53; lever-exhaustion: s62 ablation A1 (direct D_800F19C0 read) = 4/91 */
All seven annotations carry what + mechanism (named GCC pass) + lever-exhaustion.
