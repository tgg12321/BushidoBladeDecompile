# SELF-VET — func_8005C2A8

CONSTRUCTS: none. The diff is an ordinary C function body plus ordinary extern
declarations (callee prototypes, two global array declarations copied verbatim
from the pre-existing declarations at src/text1b.c:2645-2646, and one
const-char-array declaration for the printf format string already defined at
src/text1a_b_pre_rodata.c:359). No FAKE construct, no dead store, no dead local,
no pad, no volatile, no goto, no asm, no register pin, no pointer alias, no
do/while(0) wrap, no duplicated statement, no reused or borrowed local.

## T1 semantic purpose: Every construct in the diff is semantically load-bearing.
Each statement computes or stores a value the function observably needs: the
mode check and early return, the VAB-slot close/zero path, the 16-slot SPU
address accumulation loop, the D_800A3404 / D_800A3408 / D_800A340C bookkeeping,
the three header self-relocations (hdr[0], hdr[1], hdr[2] += (s32)hdr), the
snd_VabOpen call, the error printf, the two table stores, and the returned byte
offset. Delete any one of them and the function does something different. The
only two spelling choices made this session are (a) the `s32` return type on the
local snd_VabOpen prototype and (b) which arm of the `id != -1` test carries the
success tail. Both are choices about how to write ordinary C — a prototype and
an if/else orientation — not constructs added to the program. (b) in particular
REMOVES an instruction (a redundant `j` to the shared epilogue) rather than
adding one.

## T2 human-programmer: Yes. A human writing this function from its
specification writes exactly this: check the mode, close any previously loaded
VAB in this slot, recompute the SPU allocation watermark, relocate the header's
three internal offsets to absolute addresses, open the VAB, then either report
"vab id:%d mistake" or record the new slot and return the header size. The
`if (id != -1) { ...success path...; return size; } printf(...); return 0;`
shape — success path in the arm, error report trailing — is one of the two
completely ordinary ways to spell an error check, and it is the more readable
one here because the success path is ten lines and the error path is two.
Nothing in the body would make a reader ask "why is this here?".

## T3 GCC-internals justification: No construct is justified by a GCC internal.
evidence.md records the MEASURED codegen consequence of the two spelling choices
(where the sign-extension lands; the physical order of the two exit blocks)
because that is this session's finding and the next session's inheritance — but
neither construct exists FOR that reason in the sense this test targets: both
are normal C with a normal reading, and both remain the natural spelling if you
delete every word of codegen commentary. No GCC pass is the mechanism of
anything here: no allocator lever, no scheduling barrier, no DCE-defeating
value, no reg_n_refs manipulation, no LUID / label_num / INSN_PRIORITY
reasoning, no reorg.c or combine.c dependency.

## T4 permuter/search provenance: No permuter, no auto-search, no randomized
search of any kind ran this session. The body was derived by reading
asm/funcs/func_8005C2A8.s instruction by instruction; the two refinements came
from reading the objdump diff and reasoning about C semantics (a callee's
declared return type determines where a narrowing conversion happens; source arm
order determines emitted block order). Nothing in the diff is present because a
detector failed to catch a particular spelling.

## T5 family check: No construct matches any forbidden family, by analogy or
otherwise, because there is no extra construct at all. Checked explicitly
against the catalog: no register-asm pin, no hardcoded-$N asm injection, no
scheduling barrier, no INLINE_MOVE_ALIASING, no volatile in any spelling
(alias-rename, cast, plain extern, or (void) discard), no unused local array or
frame coercion, no dead-param-assign, no dead conditional store, no empty-body
if, no `if (1)` wrapper, no dead goto label pad, no DImode chain, no
goto-end-with-ret-val accumulator, no param-local alias declaration-order trick,
no opaque `s32 one = 1;`, no lowercase asm block, no build-time assembly
rewriting, no `asm("sym")` alias rename, no redundant width cast (the two
`(s32) hdr` casts convert a pointer to the integer the header arithmetic
actually stores, and the `s16` locals are the function's real narrow values),
no linker-script or rodata reorder. Consequently I claim NO sanctioned family
either — this is plain C.

## T6 naming-announces-intent: No name in the diff announces coercion intent.
The locals are `i` (loop counter), `id` (the VAB slot id), `vab` (the
caller-supplied vab id), and the parameters `hdr`, `vabid`, `arg2`. Every one is
read for its value in the emitted code; none is a pad/dummy/unused/spill/slack/
tail/buf, none is unused, none is only address-taken, and none is discarded.

SANCTIONED-FAMILY-CLAIMS: none — the diff is ordinary C and claims no exception.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.
