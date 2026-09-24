# compute vein - rejected / not proposed (2026-09-24)

Scope examined: every census-AUTO function with no non-verified callees (101 leaves + 21 verified-callee-only), plus every AUTO user of the Judge sine table, plus the libsn break-trap stubs next to the AUTO PC* wrappers. Byte-duplication check (tmp/naming_sweep/compute/dupe.py, relocation-masked instruction sequences): no AUTO body duplicates a VERIFIED library body except trivial shape-only matches (single-call wrappers / global getters, targets differ) - no byte evidence found.

| addr | insns | reason |
|---|---|---|
| 0x8007E08C | 2 | D_8007E08C data-as-code blob (census note) - not a function |
| 0x8003E2A0 | 3 | returns a game global (lw gp_rel D_800A3228) - accessor, no computation |
| 0x80042864 | 4 | returns game global D_800F6650 (lh) - accessor |
| 0x800545F4 | 4 | 4 words of data-as-code (lui/ori constants 0x4C808080/0x48808080 patched by func_80041398) - not a function body |
| 0x80077B20 | 4 | stores 1 to game global D_800A35E4 - setter |
| 0x80077D00 | 4 | returns &D_8009BD24 - address getter of a game table |
| 0x80052C10 | 5 | stores 0 to scratchpad 0x1F800400 - game state reset |
| 0x8006D324 | 5 | writes 5 to two fields of a game struct via global D_800A34FC |
| 0x8001BE08 | 6 | zero/-1 init of fields 8..0x14 of a caller struct - game-struct init, no math |
| 0x80022568 | 6 | init of fields 0x26C..0x272 of a game struct |
| 0x80048B8C | 6 | D_800A33E4 += a0 - game-global accumulator |
| 0x8003DDF8 | 7 | stores a0&0xFFFFFF into (*D_800A378C)+0x3FFC - game buffer write (OT-like), not pure |
| 0x8003FFC4 | 7 | if (p->0x24) p->0x24->6 = 1 - game-struct flag set |
| 0x8006E480 | 7 | (p[0]&0x1F) + p[1]*128 + a1 over a byte pair (andi 0xFE1F on an lbu) - computation clear but no neutral name; the packing is a game data format |
| 0x8003877C | 8 | sets 4 game globals |
| 0x80038148 | 10 | zero-fills the 512-byte game global D_800F33D8 - fixed global target, a name would have to name the global |
| 0x8003A2DC | 11 | thin wrapper of _comb_control with game-chosen constant args - api-restatement vein, not computation |
| 0x80036034 | 12 | CdFlush wrapper - api-restatement vein |
| 0x8003D2C4 | 12 | LoadImage wrapper - api-restatement vein |
| 0x8005B6FC | 12 | SsVabClose wrapper - api-restatement vein |
| 0x80060E04 | 13 | selects one of two game globals by a0 and copies to two others |
| 0x800404D8 | 14 | zero-fills two 3-word game tables (D_80094B88, D_800A9A10) |
| 0x8003D2F4 | 15 | initialises a set of game globals (0xF0F0F0 colour, toggles D_800A3218) |
| 0x8003043C | 16 | fills 12 records of game table D_80106A7A (stride 0x64) with -1/0xFF |
| 0x80030D04 | 17 | game table D_80106A7A scan: clears entries whose value is in 0x12..0x1D |
| 0x80040068 | 18 | loop over caller struct array clearing bytes +0x41A / +0xE - game struct |
| 0x800400B0 | 18 | stores a1 into +0x34 of each 0xD0-byte record - game struct |
| 0x8004939C | 18 | fills game table D_800EF9F2 with -1 and sets globals |
| 0x80064E90 | 18 | copies *D_800A347C (3 words) into game globals - one of a family of 7 per-slot copies (also 0x8006505C/0x800650A4/0x800650EC/0x800652AC/0x8006517C/0x800651F0) |
| 0x8006505C | 18 | see 0x80064E90 (game-global copy family) |
| 0x800650A4 | 18 | see 0x80064E90 (game-global copy family) |
| 0x800650EC | 18 | see 0x80064E90 (game-global copy family) |
| 0x800652AC | 18 | see 0x80064E90 (game-global copy family) |
| 0x8004001C | 19 | sets bytes +0x41A/+0xE to 1 in game struct array (twin of 0x80040068) |
| 0x8005509C | 19 | clears 8 byte-pairs in record a0 of game table D_80101EC8 (stride 0x41C) |
| 0x80027334 | 22 | writes 9 fixed constants into game-struct fields 0x3C..0x52 |
| 0x8003A308 | 22 | thin wrapper of _comb_control - api-restatement vein |
| 0x80060C60 | 22 | zeroes game tables D_800F10D0/D_800F1150 and 9 globals |
| 0x80030524 | 23 | game table D_80106A7A scan with flag D_80106A80 |
| 0x80045694 | 23 | searches game table D_800EED10 (count D_800A33AC) for a key and stores a1 |
| 0x80045230 | 25 | high-water mark of a game heap pointer vs D_800A9D10, calls func_80052C10 (AUTO) - not pure, non-verified callee |
| 0x80044098 | 26 | relocates an offset table in a game resource (table D_80103608) - pointer fixup, game data format |
| 0x8003D330 | 27 | links an entry of game table D_800A3D30 into a list rooted at D_800A374C (0xE100001F GPU code) - game state |
| 0x80040400 | 27 | finds free slot (stride 0x68) and initialises a game record |
| 0x8006517C | 29 | see 0x80064E90 (game-global copy family) |
| 0x800651F0 | 29 | see 0x80064E90 (game-global copy family) |
| 0x80033498 | 30 | switch on game global D_800A36A4 returning 0..5/0xFF |
| 0x800213A0 | 33 | game-struct counter wrap using table D_800A3860 |
| 0x80027438 | 33 | jump table on game global |
| 0x8003FE40 | 35 | applies a -1-terminated run list (value, indices...) into 8-byte records - game data format, no neutral name |
| 0x80086130 | 35 | game globals D_800F65E0/D_80102A78 |
| 0x80040CB8 | 36 | initialises 0x68-byte game records from table D_80094B9E |
| 0x80030B10 | 38 | history-list push on game-struct fields 0x330/0x332 |
| 0x80041398 | 38 | patches the data-as-code words at 0x800545F4..0x80054600 (self-modifying code) - not computation |
| 0x80062FEC | 38 | game globals (8) |
| 0x80045510 | 39 | game table D_800EED10 search |
| 0x80034200 | 40 | game globals (6) |
| 0x8002738C | 43 | jump table + game globals |
| 0x8003553C | 43 | AddPrim/SetPolyG4 with game data - api-restatement vein |
| 0x80056FE8 | 43 | game-struct fields + tables D_8009A830/38/40 -> game value (+0x12C) |
| 0x80033D38 | 47 | game globals (6) |
| 0x80037F40 | 51 | game tables D_80106A50/70 |
| 0x8006288C | 52 | game globals (12) |
| 0x80036064 | 55 | Cd* sequence - api-restatement vein |
| 0x8003D39C | 55 | game globals (5) |
| 0x8003FECC | 55 | parses a -2-terminated s16 record stream into caller struct fields 0x84..0x94 - game data format |
| 0x80017FA0 | 61 | copies caller-struct words (<<7, <<2) into fixed scratchpad slots 0x1F800060.. - game scratchpad layout |
| 0x8006CBD4 | 61 | game globals |
| 0x8001CD68 | 62 | game global D_800A3858 |
| 0x800692C0 | 67 | game globals |
| 0x800324D0 | 68 | jump-table dispatch on game state |
| 0x80021280 | 72 | game globals (5) |
| 0x8002FC80 | 76 | triangle normal via GTE OP through fixed scratchpad temporaries, then ratan2(n.x,n.z) (+0x800 if n.y>0); inputs truncated to 16 bits by the IR/R regs - compound, no honest short name |
| 0x800401CC | 78 | SetDrawMove - api-restatement vein |
| 0x800645B0 | 78 | rand + game globals/struct - not pure |
| 0x80036FD4 | 79 | CdControlB/CdPosToInt - api vein |
| 0x8003800C | 79 | game globals |
| 0x80068D88 | 81 | game globals (9) |
| 0x80047A90 | 84 | uses sine table but reads/writes sound globals D_800EF558/D_800EF59C/D_800EF800 - not pure |
| 0x800340A0 | 88 | game globals (24) |
| 0x8007526C | 91 | game global D_800A36A0 |
| 0x8002FDB0 | 92 | GTE OP (cross product) of two edges of a triangle fetched from fixed scratchpad slots 0x1F8000B4.. and indexed by a game-struct field; returns n.y>0 - scratchpad layout is game-specific |
| 0x80040B44 | 93 | builds game record tables (stride 0x68, +0x1A34 index) from a 0xFFFF-terminated list - game data |
| 0x80033BC0 | 94 | game globals (24) |
| 0x800475A4 | 101 | sine table + sound globals (16); non-verified callees |
| 0x8002EA24 | 110 | game table D_8008D118 + GTE; already COMPLETED-INLINE-ASM-CANONICAL, game physics |
| 0x80032314 | 111 | game tables D_8008D118/D_80101EC8/D_80104E88 |
| 0x80057CC8 | 111 | ratan2 + sine table on a game polyline struct (count @+3, points @+4, scale @+2) - game struct |
| 0x8002E838 | 123 | already COMPLETED-INLINE-ASM-CANONICAL rotate-velocity block on game struct |
| 0x800393C8 | 123 | game globals |
| 0x800204C0 | 124 | GTE + non-verified callee func_80032854 |
| 0x80048864 | 134 | LoadImage/StoreImage/DrawSync on game VRAM rects - api vein |
| 0x8003EB84 | 143 | game globals/tables (8) |
| 0x80063BD0 | 144 | game globals (18) |
| 0x80030580 | 148 | sine table + game tables D_8008E194/D_80106A78 |
| 0x8002D518 | 154 | game table D_8008D118 |
| 0x800233AC | 167 | sine table + game table D_8008EB40 + scratchpad; non-verified callees |
| 0x8002EBDC | 188 | RotMatrixX/Y + ratan2 on game struct |
| 0x800338CC | 189 | rand + game globals |
| 0x800520B8 | 202 | patches code words at 0x800521AC.. (self-modifying) - not computation |
| 0x80044800 | 204 | sine table + sound globals; non-verified callees |
| 0x8002DAD0 | 212 | RotMatrixX/Y + ratan2 on game struct |
| 0x800283D0 | 215 | sine table + game globals; non-verified callees |
| 0x8003EDC0 | 234 | game globals (12) |
| 0x80048BA4 | 237 | sine table + 31 globals; non-verified callees |
| 0x8003E2D8 | 242 | non-verified callee func_8003F388; game data |
| 0x8002C22C | 252 | game globals (15) |
| 0x800571C0 | 287 | sine table + game globals + scratchpad; non-verified callees |
| 0x80018300 | 317 | game table D_8008D118 + fixed scratchpad slots |
| 0x8002CD58 | 370 | RotMatrixX/Y + ratan2 on game struct |
| 0x8008B488 | 387 | libspu-internal callees (_spu_*); libscan vein |
| 0x80065800 | 1456 | 1456-insn game renderer |

Not examined in depth (outside this vein: have non-verified callees, so not leaf/near-leaf computation): 186 AUTO functions.
