# Hypothesis ledger — file_LoadSectors

## s1 (recon, 2026-07-28)
- H-vars-phantom: "a live-locals spelling induces a phantom vars=8 slot (per
  phantom-frame-slots-gcc272)" — KILLED. 7 probes all vars=0 or wrong frame
  composition; the HImode-bitwise trigger needs narrow global loads and dies with
  calls present. See rejected/phantom-vars-slot-all-probes-vars0.c.
- H-args24-deadcall: "target frame is args=24 from a >=5-arg call expression
  expanded then deleted by RTL cse (outgoing_args_size is monotonic)" — CONFIRMED
  mechanically: sandbox 0 on real TU, byte-identical body, zero residue.
  Classification (sanctioned-family vs cheat) sent to ruling.
- Frontier: (1) ruling on the dead-call construct; (2) if refused — sweep for a
  more plausible-as-original deleted-call spelling (macro-shaped error check),
  same mechanism; (3) SOTN-master census for deleted-call/args-area precedent to
  inform the ruling.
