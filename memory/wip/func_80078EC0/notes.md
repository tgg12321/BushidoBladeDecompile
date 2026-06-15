# func_80078EC0 (text1b_b.c) — WIP, floor 3 (HEAD 10), BLOCKED

## TL;DR (2026-06-14)
Tiny 16-insn predicate `return (p[1]&1) && (p[0]&1)`. HEAD matched via 2
register pins (`p asm("$3")`, `volatile ret asm("$2")`) + a FORBIDDEN
`ret=1; ret=0` dead-store trick + 5 regfix rules (frame removal + $4->$2
renames). The natural **three-return form** retires ALL of that and gets the
registers (p->$v1, ret->$v0) and framelessness for FREE — floor drops 10 -> 3.

## The residual 3 (the whole gap)
Registers + frame already MATCH. The only difference: GCC folds
`if (p[0]&1) return 1; return 0;` -> `return p[0]&1;` (boolean if-to-return
simplification), emitting `andi v0,v0,1; jr ra; nop`. Target keeps the verbose
`andi v0,v0,1; bnez v0,end; li v0,1; move v0,zero` (3 more insns). HEAD forced
the verbose branch with the dead `ret=1` (forbidden).

## Resume steps
1. Paste candidate.c (three-return form); confirm sandbox 3.
2. Find a CLEAN (no dead store) C form that defeats the boolean if-to-return
   fold so target's `bnez; li 1; move 0` survives. Verify via retire (full SHA1).
3. If none exists, this is not pure-C-closable -> canonical-asm auth / plateau.
   Floor-3 (registers + frame correct) is the honest pure-C limit.

## Ruled out (do not re-derive)
- `ret=1; ret=0;` dead-store trick: FORBIDDEN (Lever D); it's how HEAD cheated.
- shared-end-label goto form: still folds (ret=0 in 2 of 3 paths) — floor 3, no
  better than three-return.
- explicit if/else: folds identically.

## Pointers
- `.claude/rules/shared-end-label.md` (tried; the 0/1 boolean folds harder than
  per-case constants), `register-alloc-pure-c.md` (Lever D = dead store forbidden),
  `do-while-zero-exception.md` (for reorg.c, not this jump-opt fold).
