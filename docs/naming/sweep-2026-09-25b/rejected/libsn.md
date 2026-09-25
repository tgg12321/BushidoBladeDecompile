# libsn vein — examined and dropped (2026-09-25)

Coverage: the downloaded PsyQ 4.0 set (`tmp/libscan/psyq40/LIB`, 19 LIBs) against the committed
`docs/naming/libscan/matches.json`. The only libraries the committed scan does not cover are **LIBSN (53 modules), LIBGUN (2) and LIBMCRD (2)**. For every
overlapping library, module set, status and vaddrs are identical (`cover.py`). The committed scan's LIBC is
not in the download. The download holds only INCLUDE/ and LIB/, with no loose OBJs.
The near scan (`near.py`, whole module plus each XDEF/LOCAL text chunk, masked, early exit at 25%) wrote
`near_LIBSN.json` and `near_LIBGUN_LIBMCRD.json`.

| module / symbol | best hit | why dropped |
|---|---|---|
| LIBSN/WRITE PCwrite | 0/48 @0x8008387C | Masked-identical to READ. The reloc target (`_SN_write` should hold break 0x106, but the EXE callee holds break 0x105 = `_SN_read`) picks READ. |
| LIBSN/SNWRITE _SN_write | 1/6 @0x800836A0 | Lands mid-PCopen, not at a function start. Not linked. |
| LIBSN/_OP_VDEL, _OP_VNEW | 0/8 at 23 addresses | Generic single-tail-call stub shape, so the placement is ambiguous. These are C++ runtime pieces with no reachability. |
| LIBSN/_EH __throw_type_match | 0/8 @0x800168F8 | Same generic stub shape (it is func_800168F8, a lone jal). Not linked. |
| LIBSN/_OP_DELE __builtin_delete | 2/9 @0x800168F4 | Not a function start (the middle of sys_StubEmpty). |
| LIBSN/_NEW_HAN __default_new_handler | 1/8 @0x80016868 | The ResetGraph(1) wrapper (generic jal+return shape). Not linked. |
| LIBSN/_UDIVDI3 __udivdi3 | 2/8 @0x8003E144 | Generic wrapper shape. BB2 has no 64-bit division. |
| LIBSN CREAT, FSINIT, CACHE (SNFlushCache), all libgcc DI/float helpers, _VARARGS, __GCC_BC | > 25% mismatch | Not linked. |
| LIBSN/PUREV __pure_virtual | 1 word | Can't be anchored. It's C++ and not linked. |
| LIBSN/_SHTAB __shtab (128 B .data), _UDIVMOD .rdata (256 B) | exact-bytes search: 0 hits | Not linked. |
| LIBGUN/NEWGUN _ExitGun | 1/26 @0x8007A458 | That address is the verbatim LIBCARD/END `_ExitCard` (VERIFIED). The modules are just code-similar. |
| LIBGUN/GUN SendGUN, EnableGUN / DesableGUN | 1/8, 2/9 @0x80079008 | Generic shapes near the LIBAPI stubs. LIBGUN is not linked (GUN 804 words, no anchor). |
| LIBMCRD MemCardInit | 3/12 @0x8003550C | Game code, and too many mismatches. |
| LIBMCRD MemCardEnd / _card_close | 0/8 @0x800168F8 | The same generic lone-jal stub. |
| LIBMCRD _card_open | 2/12 @0x80083A18 | Lands inside SsEnd. LIBMCRD is not linked (confirms the 09-25 followups finding). |
