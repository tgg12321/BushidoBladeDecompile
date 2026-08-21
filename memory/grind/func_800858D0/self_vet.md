# SELF-VET — func_800858D0

CONSTRUCTS: goto-spelled loop (label `loop:` + trailing `if (var_s0 < D_80101BCC) goto loop;`), byte-offset pointer-cast stores to global s16 scalars at stride 54 (`*(s16 *)((u8 *)&D_800F4E1A + offset)` etc.), named live locals (s32 buf[16], s16 var_s0, s32 offset, s16 ff), direct constant stores/init ordering (buf[1] first, 0x3C store last), shift `buf[0] = 1 << var_s0`

## T1 semantic purpose
- goto loop: it IS the iteration — removing it removes the loop entirely; the emitted bytes contain the backward branch, the loop body executes D_80101BCC times. Full observable effect. PASS.
- byte-offset stores: they write the six per-entry s16 fields at entry*54 into the D_800F4E1x block — the function's core observable behavior. PASS.
- locals: buf is the 64-byte parameter block passed to func_8008B488 (written 9 fields, address passed); var_s0 is the loop counter (stored to D_8010280A each iteration); offset is the consumed stride product; ff (0xFF) is stored twice per iteration. All written AND read; zero dead stores, zero unused declarations, zero constant-holder scalars. PASS.
- statement order / direct constants: ordinary init sequencing; every store lands in the bytes. PASS.

## T2 human-programmer
Every statement is plain program logic a programmer writes from the spec ("init a
command buffer; for each of N entries write six fields and dispatch"). The one
choice a reader might ask about is goto-vs-do-while for the loop — but a
goto-spelled loop is ordinary (if dated) C, common in shipped PSX-era code:
SOTN master ships the identical shape in fully matched PSX code
(docs/reference/sotn-construct-index.md:1033 = sotn src/main/main.c:40
`main_search_loop_1:` with backward goto; :1015 = src/dra/5F60C.c:579; :1035 =
libc/sprintf.c:96 `loop_30:`). There is no dead code, no unused variable, no
padding, no wrapper with an empty body. PASS.

## T3 GCC-internals justification
The DISCOVERY of the spelling used GCC internals (loop.c note-dependence;
sched.c birthing_insn_p/adjust_priority launch; evidence.md s2) — that is
diagnosis, and the family-selection table treats spelling choices whose
mechanism is a compiler pass as ordinary C when the construct is live semantic
code (mixed exit forms via cross-jump; switch-vs-ifchain via branch-sense;
sub-word reads). The construct itself is explained entirely by program logic:
a loop that runs while the counter is below the limit. Nothing in the body
exists "for the compiler only": there is no statement whose removal leaves
behavior unchanged. The goto spelling and the do-while spelling are equally
valid implementations of the same specification; the target bytes select the
goto one (the target's own emission profile — zero hoisted invariants,
launch-adjacent constant loads — is affirmative evidence the ORIGINAL
compilation unit had no loop notes here, i.e. this recovers the original
structure rather than coercing ours). PASS.

## T4 permuter/search provenance
No auto-search was used this session. The form was derived analytically from
the target disassembly, the s1 ledger, and sched.c/loop.c reads, then measured
once with sandbox (0). Not a detector-evasion find. PASS.

## T5 family check
Checked against the forbidden catalog: no register pins, no __asm__, no
volatile, no dead stores/params/locals, no constant-holders, no empty-body
`if`/`if(1)` wraps, no dead-goto label-pad (the label has a live backward goto
that forms the loop — the pad family is DEAD gotos/labels; this one is
load-bearing control flow), no alias renames, no redundant width casts (the
formerly-present `(s16)` casts on var_s0 were measured byte-inert and REMOVED;
final body has none), no do-while(0) wrapper, no borrowed carriers (the s1
multi-set scratch machinery was deleted, not respelled). The goto-loop shape
is not a member of any forbidden family and is shipped by SOTN master in
matched PSX code (citations under T2). PASS.

## T6 naming-announces-intent
Names: buf (the dispatched buffer), var_s0 (splat-conventional counter name,
carried over from the prior ledger candidate), offset, ff, loop. No pad/dummy/
unused/spill/slack names; every named object is read and written. PASS.

SANCTIONED-FAMILY-CLAIMS: none — the diff contains no /* FAKE */ construct and
claims no last-resort family; every statement is live, consumed program logic
in ordinary C (the goto-spelled loop is a control-flow spelling choice of the
same ordinary-C class as the family table's "mixed exit forms" row, with SOTN
master shipping the exact shape in matched PSX code:
docs/reference/sotn-construct-index.md:1033, :1015, :1035).

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
