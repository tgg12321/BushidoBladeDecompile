# Evidence bank — func_8008C464

- [s1] Queue entry: distance=761, verdict=ASM-STRUCTURAL, rules=1, status=parked (2026-06-09 audit REJECTED auto-auth). Single rule: asmfix.txt:129 replace_with_asmfile. src/main.c body is a stub. regfix.txt: zero rules.

- [s1] Structure (tmp/blitz/func_8008C464_map.txt): 763 insns @0x8008C464-0x8008D04C (the very END of .text — code ends ~0x8008D070), frame 0x20, saves only s0/s1/ra. ZERO loops. 3 jumptable dispatches (jr $v0). 9 jr $ra sites.

- [s1] MULTI-FUNCTION BLOB: only the jr at 0x8008C970 follows a register-restoring epilogue (lw ra/s1/s0; addiu sp,0x20). The other 8 returns (0x8008C9EC, 0x8008CD84, 0x8008CEA4, 0x8008CEAC, 0x8008CEB4, 0x8008CF54, 0x8008D040, 0x8008D048) sit AFTER it — the splat extent contains the dispatcher (0x8008C464-0x8008C970) plus ~5-8 separate un-symboled functions (0x8008C974+). All jtbl targets are < 0x8008C960, so the trailing functions are reached via function POINTERS (e.g. the handler struct registered by bios_SysEnqIntRP at 0x8008CE1C / deregistered by SysDeqIntRP at 0x8008CE10 and 0x8008CF0C) or direct jals from elsewhere in the libapi tail.

- [s1] Dispatcher shape: s32 func_8008C464(u32 cmd, u32 sub, ...): `sltiu v0,a0,6; beqz -> return 0` then jr through table at func_800164AC+0 (6 entries); two nested sub-dispatches on a1: tables at func_800164AC+0x18 (up to 8 entries, one entry is literal 0x00000000 = unused slot) and +0x38 (5 entries). Default path returns s0 (=0).

- [s1] THE JUMP TABLES ARE NOT IN THIS TU: func_800164AC @0x800164AC is the FIRST address of .text (file 0x6CAC) — 19 .word entries, all targets inside func_8008C464's body. Currently emitted by src/ings.c:91-116 as a file-scope __asm__ glabel blob of hardcoded .word addresses (plus func_800164F8, a real 7-word busy-wait delay function). src/ings_strings.c documents the fixed-address constraint (rodata-cleanup 2026-06-09).

- [s1] Why the tables are there: PsyQ section order .rodata->.text; the LAST linked object's .rodata lands at the END of the rodata region = immediately before .text start. func_8008C464 is in the LAST object (libapi tail) — its GCC-emitted switch jtbls landed at 0x800164AC. So the ORIGINAL was plain C switches; byte-matching in pure C requires this TU's emitted .rodata jtbls to be PLACED at 0x800164AC (bb2.ld attribution work, the sanctioned evidence-based re-attribution workflow per jtbl-rodata-split-infrastructure / no-new-park-categories) — or the dispatcher keeps referencing the extern blob (non-switch spelling, likely unmatchable since target insns are the standard casesi sequence %hi/%lo(sym+K)).

- [s1] PsyQ library identity (strong signals): callees are EnterCriticalSection x3, ExitCriticalSection, bios_DeliverEvent x3, bios_SysDeqIntRP x2, bios_SysEnqIntRP, gpu_SetMode, SetPacketData, func_8008BEA4, func_8008C184, cpu_side_move_dir_3 (misnamed Kengo label in the libapi tail). Zero game-struct access patterns; lui-heavy (117) absolute addressing; div+divu singletons. This is SDK event/device management code (card/pad async event family), NOT Lightweight game code.

- [s1] Consequence for the grind: the 761 distance aggregates ~6-9 SEPARATE small functions. The work decomposes: (a) identify the trailing functions' entry addresses (scan for their pointers in .data / the SysEnqIntRP handler block construction around 0x8008CE00-0x8008CE30), (b) each is a small leaf/near-leaf C function, (c) the dispatcher itself is two nested switch statements + case bodies.

- [s1] Case-body character: short hardware/state sequences — lhu/sh flag RMW on device state words (e.g. `lhu; andi 0xFBFF; sh` clear-bit at 0x8008C9D8-0x8008C9E4), DeliverEvent calls with constant descriptors, EnterCriticalSection-guarded sections, one gpu_SetMode call at 0x8008C954 reached from the a0-switch. Mnemonic profile lui(117)/lw(91)/lhu(47)/sh(40)/andi(26) = absolute-address flag machine.

- [s1] m2c CANNOT run as-is: the dispatch reads %lo(func_800164AC+K) — symbol+offset table refs aren't recognized as jump tables ('jump table is not provided'). To m2c it later: synthesize a .s defining three separate jtbl labels at the right offsets AND rewrite the lw lines to reference them, or m2c per-region with --start/--stop. Deferred to the structural session.

- [s1] Sibling ledgers already exist for neighbors in this tail: func_8008BEA4, func_8008C184 (both in memory/grind/) — the libapi-tail cluster should share struct/global naming when drafted; check their evidence before drafting this one.

- [s1] The 0x00000000 entry inside the +0x18 table (ings.c line 108, word index 13) means one a1 value is an unreachable/unused case — a C switch with a `case N:` absent (GCC fills with the table's default-fill = 0? GCC 2.7.2 fills missing cases with the default label address, NOT 0) — more likely the +0x18 table has 7 real entries and the 0 word is inter-table padding/alignment, i.e. table sizes are 6/7/5 not 6/8/5. Verify against the sltiu bounds at each dispatch when drafting (bounds visible at 0x8008C4A0 region asm).

- [s1] Ledger existed as empty skeleton (session_count=0); no WIP checkpoint.
