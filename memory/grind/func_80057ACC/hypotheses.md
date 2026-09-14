# Hypothesis ledger — func_80057ACC

## s1 (recon, 2026-09-14)

H1 — DECLARATION/OBJECT MODEL. "func_80057ACC reads or writes a file-scope
global whose declared type or aggregation is wrong." KILLED (instance, measured
on HEAD chassis with no FAKE constructs present): the function references no
global at all; all memory traffic is through arg0, arg1 and two address-taken
frame locals. There was no declaration to fix, so this axis is empty rather
than dead. See evidence [s1-a].

H2 — SHORT-TYPED COUNTERS. "The target's `sll 16`/`sra 13` and `sll 16`/
`sra 14` index fusion, and its ability to keep arg2/arg3 in $s7/$fp, come from
the loop counters being declared `short` rather than `int` with explicit
truncating casts." CONFIRMED: the int-with-casts spelling measures 91 and shows
two extra shifted induction pseudos plus two argument spills; changing only the
declared types of i/j/k/n to s16 (and inlining the four vertex-table loads)
measures 0 and the full build reproduces the oracle SHA1. Mechanism: GCC 2.7.2
promotes a `short` local to SImode and re-extends at each use, which gives
combine the adjacent shift pair to fuse and gives loop.c no reducible giv; the
explicit-cast int spelling instead presents loop.c with a linear induction
expression it strength-reduces into a second live register per counter.

FRONTIER: empty — function is matched at distance 0 with an oracle-clean build.
