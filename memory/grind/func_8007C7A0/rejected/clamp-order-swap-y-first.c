/* REJECTED s13 (driver session 8, 2026-08-10): clamp statement order swapped
 * (full Y clamp before X clamp) on the 5-chassis. sandbox --disable all =
 * 19 @ build 50 (vs floor 5 @ 50). Target's X-first statement order is
 * load-bearing: the limit loads and Y-clamp placement all shift. Structure
 * identical to candidate.c with the two clamp blocks exchanged. */
