# rejected: r11-work-delta-reuse-layer2-fail-0 (2026-09-28, fresh layer-2 FAIL)

Body: sandbox 0 (644/644), oracle-green when spliced; islands, auth row, owner-grant row, region
hashes and five of seven Ruling 11 locals (idx, nforce, temp incl. its Q28 copy, nbits, nbits2) PASSED.

FAILED (Ruling 11 (D)(3) necessity) for `work` and `delta`: a one-variable-per-value spelling plus one
sanctioned FAKE dead store reaches 0/662 for each (work: `sq1 = 0;` after temp's last use; delta:
`dg = 0;` after the ellipsoid loop; both together too). A dead write reaches no read, so it forms no
Ruling 11 value; the split spelling is one-variable-per-value and has the property. The proof's
property text was wrong: regno_last_uid counts SETs (regclass.c:1763-1764), so a dead set gives
the cse class head (cse.c:840-857) without any reuse. My ledger probe for work had put the dead
store before temp's last use (it could not test the mechanism). The func_8008B488 precedent does not
help: there necessity rested on a value no FAKE reached. Ruling 1(4) chooses among admissible forms;
it does not supply (D)(3).

Secondary: lz[6] annotation cited a missing proof section and banked no phantom-slot producer census;
header comment made the state -0xFF..-1 path exclusive (it pulls, then integrates); "rope/cloth" /
RopeScratch naming unevidenced.

Next form (session 3 continued): work -> sq1 + dist1 and delta -> depth + dy0 as single-value locals,
each closed by a FAKE dead store (dead-store-fake-exception); census banked; naming neutral.
