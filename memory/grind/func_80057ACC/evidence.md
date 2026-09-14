# Evidence bank — func_80057ACC

## s1 (recon, 2026-09-14) — SOLVED, honest distance 0, full-build SHA1 == oracle

[s1-a] OBJECT MODEL: func_80057ACC touches ZERO file-scope globals. Every memory
access is through its own parameters (arg0 = the actor/owner struct, read at
+0xF4 and +0xFC and written at +0x360/+0x361; arg1 = the polygon-list header,
u8 count at +0, table pointer at +4) or through the two address-taken frame
locals passed to func_8005763C as out-parameters. The brief carried no DATA
MODEL flags and a full-file grep confirms no D_/g_ symbol is referenced in the
body, so there is no declaration-fix hypothesis to measure: per-symbol verdict
list is empty — MATCHES by vacuity, no symbol MISMATCH and none left unmeasured.
The two callees (func_8005763C, SquareRoot0) are declared locally in the
established style of src/text1b.c; no header change was required or made.

[s1-b] SEMANTICS. The function is a nearest-edge-intersection search.
Signature: s32 func_80057ACC(s32 owner, u8 *polylist, s32 x, s32 y).
`polylist[0]` = polygon count (u8); `*(s32 *)(polylist + 4)` = array of 8-byte
polygon records. Per record: byte 0 = flags (bit 0x80 = open polyline), byte 3 =
vertex count, word at +4 = pointer to an array of {s16 x, s16 y} vertices.
For every edge (j, j+1 with wrap to 0) of every polygon it calls
func_8005763C(owner+0xF4, owner+0xFC, x, y, ax, ay, bx, by, &hitx, &hity) — a
clip/segment-intersection helper whose 10th and 9th arguments are out-pointers
(confirmed by reading its prologue: `lw $t8, 0x28($sp)` / `lw $t9, 0x2C($sp)`
after `addiu $sp, $sp, -8`, i.e. the caller's 0x20/0x24 outgoing-arg slots).
On a hit it takes SquareRoot0(dx*dx + dy*dy) from the owner's position, keeps
the running minimum (seeded at 100000 = 0x186A0) and records the winning
polygon index and edge index as bytes at owner+0x360 / owner+0x361. The minimum
is returned; caller func_80058580 stores it to its own arg0+0x434.
The open-polyline flag reduces the edge count by one (n = vcount - 1) so the
closing edge is skipped; the wrap of k to 0 is still computed against the
unreduced vcount, which is why the target reloads `poly[3]` inside the loop.

[s1-c] MEASURED: first hand-written form (int-typed counters with explicit
`(s32)(s16)x` casts at every use, and the vertex-table pointer bound to one
`s16 *vtx` local) scored 91/127. Banked at
memory/grind/func_80057ACC/rejected/shifted-giv-int-locals-score91.c.
Objdump diff (tmp/grind/func_80057ACC/s1/build.txt vs target.txt) showed two
independent defects, both register-pressure-linked:
  (1) frame 0x78 vs target 0x58, with `sw a2,48(sp)` / `sw a3,56(sp)` — arg2
      and arg3 were spilled to memory instead of living in $s7/$fp as the
      target has them;
  (2) the index arithmetic came out as a separate `sra 16` + `sll 3` pair
      instead of the target's fused `sll 16` / `sra 13`, and the outer and
      inner counters each carried a SECOND live pseudo holding the
      already-shifted value (`move s4,zero` alongside `move s5,zero`).
Defect (2) is the cause of defect (1): the two extra shifted induction
variables occupied two callee-saved registers, so arg2/arg3 lost their seats.

[s1-d] MEASURED: declaring the four sub-word quantities with the type the data
model implies — `s16 i, j, k, n` instead of s32-with-casts — and writing the
vertex-table dereference `*(s32 *)(poly + 4)` inline at each of its four
call-argument use sites instead of binding it to one local, scores 0/127.
Full clean build SHA1 = 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle.
Body: memory/grind/func_80057ACC/candidate.c, applied at src/text1b.c:1836.
No FAKE construct, no sanctioned-family claim, no volatile, no inline asm.

[s1-e] TRANSFERABLE LESSON for the sibling functions in this cluster
(func_800571C0, func_8005763C, func_80057E84, func_80058580 are all still
INCLUDE_ASM in src/text1b.c and all index the same u8-counted polygon tables):
a `<<16`/`>>16` pair in the target is a SHORT-TYPED LOCAL, not a cast to write
out. Spelling it as `(s32)(s16)x` on an int local is NOT equivalent at -O2 —
loop.c strength-reduces the sign-extended index into an extra live pseudo,
while a `short` local does not offer loop.c a giv to reduce. On a function with
this much register pressure (9 callee-saved registers live) those two extra
pseudos are the whole difference between a match and a 91.
