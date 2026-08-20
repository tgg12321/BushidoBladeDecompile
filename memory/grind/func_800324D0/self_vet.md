# SELF-VET — func_800324D0

CONSTRUCTS: base (fresh u8* local, once-written, twice-read), ff (fresh u8
local = 0xFF, once-written, read by 7 stores), deletion of 4 register-asm
pins, semantic renames (v1/v0/a2/a1/a0 → ptr/c/cmd/val/pad — order-preserving,
codegen-neutral, re-measured 0 after renaming).

## T1 semantic purpose
- `base`: holds the stream base pointer loaded from `pad+0x58`; consumed twice
  (`base[4]` reads the first opcode, `base + 5` derives the walking pointer
  past the 5-byte header). Every write is read; every read feeds an emitted
  instruction (`lw`, `lbu $v0,4`, `addiu +5` are all in the target bytes).
  Removing it changes the emitted output (the register assignment of 27
  instructions) — it is not byte-inert dead code. PASS.
- `ff`: holds the 0xFF default value stored into 7 of the 11 initialization
  bytes; all 7 reads are real `sb` stores present in target (`li $v0,0xFF` is
  target's own instruction). Not dead, not unused, not address-taken. PASS.

## T2 human-programmer
`u8 *base = *(u8 **)(pad + 0x58); c = base[4]; ptr = base + 5;` is the
natural reading of "the stream has a 5-byte header; byte 4 is the first
opcode": a programmer names the base, reads the header field, and starts the
cursor past the header. `u8 ff = 0xFF;` naming the repeated default before 11
initialization stores is ordinary style (7 of the stores use it). Neither
would draw a "why is this here?" from a reader. The previous m2c body's
single-variable spelling was equally natural; both are ordinary C — this diff
chooses between two natural spellings, it does not add semantically empty
material. PASS.

## T3 GCC-internals justification
The evidence ledger does explain WHY this spelling matches via GCC internals
(local-alloc ordering, set_preference, find_reg pass 0) — that is the
matching analysis, recorded for the audit trail. The constructs themselves are
explained by program logic (T1/T2): both locals hold real values with real
consumers and their instructions exist in the target bytes. No construct here
exists ONLY as an invisible analysis-steering artifact; the choice between
byte-different natural spellings is the core matching activity, not a
coercion. PASS (with the internals reasoning disclosed, not hidden).

## T4 permuter/search provenance
No permuter or auto-search was used. The spelling was derived from RA
forensics (instrumented cc1 find_reg dumps) and verified by the sandbox. PASS.

## T5 family check
Checked against the forbidden catalog: no pins (they were REMOVED), no asm of
any kind, no volatile, no alias renames, no dead stores/self-assigns, no
unused locals or arrays, no `(void)` discards, no do-while(0) wrapper, no
goto/label constructs, no scheduling barriers, no constant-holder that is
dead (ff is read 7×). Closest neighboring families considered: the
named-intermediate fresh-local carve-out (not claimed — base is twice-read
live code, not a once-read no-op copy) and the dead-scalar constant-holder
(not applicable — ff is live). No forbidden family matches by spelling or by
analogy. PASS.

## T6 naming-announces-intent
No `pad`/`dummy`/`unused`/`spill`-style dead names. The PARAMETER is named
`pad` because the function initializes and parses a gamepad-config stream
(naming census: `is_pad/Pad_Prs`, kengo:HIGH) — it is the primary data object,
read/written by 20+ real instructions, not a discarded declaration (T6's FAIL
shape is dead/discard-only uses). `ff` names the 0xFF value it holds; `base`,
`ptr`, `c`, `cmd`, `val` are ordinary program terms. PASS.

SANCTIONED-FAMILY-CLAIMS: none — every construct in the diff is live,
consumed, byte-visible ordinary C; no last-resort family is invoked.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct (no family claimed; no rule
mandating an annotation applies to live named locals with real consumers).
