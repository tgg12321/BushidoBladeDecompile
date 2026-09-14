# SELF-VET — func_8006D3DC

CONSTRUCTS: (1) TU-local `typedef struct EnvA` — the 0x2C-byte draw descriptor type;
(2) `u8 dim = 0x40;` — a named u8 local holding the dim colour, read three times in the
dim arm; (3) `s.has_color = 1;` as a member of the pre-loop descriptor-init block, with
`s.has_color = 1;` also at the top of the loop body; (4) declaration order
`s32 semi = 0; s16 i = 0; u8 dim = 0x40;` with the loop written `for (; i < 6; i++)`.

The governing fact for constructs (2) and (3): **both materialise in the target bytes.**
They are not coercion carriers that vanish under DCE and perturb allocation — they emit
the exact instructions the shipped binary contains, so the FAKE ladder is not in play:
  - (2) -> `/* 5DC00 8006D400 */ addiu $s2, $zero, 0x40`, consumed by
    `sb $s2, 0x43/0x42/0x41($sp)` at 8006D4D8-8006D4E0 in asm/funcs/func_8006D3DC.s.
  - (3) -> `/* 5DC20 8006D420 */ addiu $v0, $zero, 0x1` +
    `/* 5DC28 8006D428 */ sb $v0, 0x40($sp)`, immediately before the loop label
    .L8006D42C in asm/funcs/func_8006D3DC.s.

## T1 semantic purpose:
(1) EnvA gives the descriptor its fields and its 0x2C size; without it the code cannot
    name `header/table/out/semi/ot_idx/x/y/has_color/col_r/col_g/col_b` at all. Real.
(2) `dim` carries the value 0x40 into the three colour-byte stores. Its value is read
    and stored; removing it does not merely change codegen, it removes the register the
    target keeps that constant in. It is a named magic number, the plainest form of
    semantic purpose a constant can have.
(3) This is a struct-field initialiser in a descriptor-init block, and it emits a store
    that is present in the shipped binary. It is redundant with respect to the loop-top
    assignment (the loop always runs), so in the abstract-machine sense the stored value
    is not read — but it is NOT eliminated by GCC and it is NOT a phantom: the original
    program performed this store. Reproducing a store the target performs is
    decompilation, not coercion.
(4) Declaration order has no semantic effect by construction — it is a spelling choice
    among semantically identical bodies, not a construct added to the program. Declared
    plainly here so the reviewer can see it was chosen deliberately: it fixes the first
    scheduler pass's tie-break among three mutually independent `sw $sN` / `init $sN`
    pairs. Measured ladder: dim,semi,i = 10; semi,dim,i = 4; semi,i,dim = 0.

## T2 human-programmer:
(1) Yes — the same file already declares two sibling descriptor types for the same
    func_8007352C call (S69E18 at src/text1b.c:5627, EnvB at src/text1b.c:6019).
(2) Yes — naming the dim shade `dim` and the bright one inline is how someone writing a
    6-entry menu highlight loop would spell it; nothing about the name or the use site
    makes a reader ask "why is this here?".
(3) Yes — and the sibling func_8006BB68 (src/text1b.c:5818-5824) proves it: it
    initialises the SAME descriptor slot (S69E18.byte28, offset 0x28 = has_color) in its
    own pre-loop init block. The original author initialised the whole descriptor up
    front and then re-set the per-item fields inside the loop.
(4) A reader would not remark on `s16 i = 0;` + `for (; i < 6; i++)` — it is an ordinary
    C idiom. I record honestly that among the semantically identical orders I chose the
    one that matches.

## T3 GCC-internals justification:
No construct's EXISTENCE is justified by a GCC pass. Pass reasoning appears only where it
explains why one spelling of the same program matched and another did not, which is the
ordinary content of a decomp derivation note:
  - `u8` vs `s32` for `dim`: an s32 holder makes GCC emit a QImode copy `move v0,s2`
    ahead of the sb triple (probe B, 22). The u8 spelling is also simply the field's own
    type, so it is what the code would say anyway.
  - a bare `0x40` literal: the constant lands in a QImode pseudo inside a conditional arm
    (tmp/grind/func_8006D3DC/dumps/text1b.loop.8006D3DC insn 170,
    `(set (reg:QI 122) (const_int 64))`) and loop.c does not hoist it, whereas the `1`
    used for has_color is an SImode pseudo and IS hoisted (insn 323, emitted before
    code_label 37). That is why the target's 0x40 must come from a variable and the
    target's 1 need not.
None of this is the reason a construct is in the program; the reason is that the target
executes it.

## T4 permuter/search provenance:
No permuter, no automated search, no enumeration was run this session. Every form was
hand-derived from asm/funcs/func_8006D3DC.s read against its two in-file siblings
(func_8006DD94, func_8006BB68), then measured. Six probes total (A 27, B 22, C 10,
D 6, E 4, F/G 0).

## T5 family check:
None of the forbidden families is present: no register-asm pin, no `__asm__` of any
spelling, no scheduling barrier, no volatile anywhere, no alias rename, no unused local
array or frame pad, no `(void)`/address-of discard, no dead-param assign, no empty-body
`if`, no `if (1)`, no dead goto/label, no DImode chain, no opaque-arithmetic variable, no
redundant width cast, no rodata reorder. `dim` is read, not discarded, and its value
appears in the target bytes; nothing in the diff is dropped before the final build.
By analogy the closest family is the dead-store carve-out
(.claude/rules/dead-store-fake-exception.md) for construct (3). I considered claiming it
and concluded it does not apply: that rule's mechanism is a store GCC DELETES whose prior
existence perturbs RA/scheduling ("GCC DCEs the store; its existence influences RA /
scheduling / flow analysis upstream of DCE"). Construct (3) is not deleted — it emits
8006D420/8006D428, two instructions of the shipped function. A store that survives into
the target bytes is the program, not a lever on the program, so no FAKE ladder,
lever-exhaustion record or family grant is required for it. If the reviewer reads that
boundary the other way, the correct disposition is a ruling, not a respelling: there is
no other spelling — the target contains the store.

## T6 naming-announces-intent:
Names used: `s`, `rect`, `q`, `c`, `hdr`, `semi`, `i`, `dim`, and the EnvA field names.
None of `pad/_pad/dummy/unused/spill/sp_*/_buf/tail/slack/_frame_pad` appears as a name
I introduced. `EnvA.pad0C/pad20/pad24` are unreferenced struct members carried over
verbatim from the existing sibling type EnvB (src/text1b.c:6019) — they name gaps in a
descriptor layout that other call sites do use, which is the ordinary way an unknown
field is spelled in this repo, and they are not locals.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C. Every construct's bytes are
present in asm/funcs/func_8006D3DC.s, so no coercion family is being invoked and no
exception is being spent.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.
