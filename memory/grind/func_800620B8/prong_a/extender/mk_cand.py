"""Build the landable split-symbol candidate from the accepted split body (s3/split.c)."""
D = 'tmp/func_800620B8/s4/'
s = open('tmp/func_800620B8/s3/split.c', encoding='utf-8').read()


def sub(a, b):
    global s
    assert s.count(a) == 1, a
    s = s.replace(a, b)


sub('    s16 height;\n', '''    s16 height;
    /* FAKE: pointer aliases of the four sprite tables (pointer-alias-fake-exception).
       All four stay live across the loop; strip32 is set at the loop top so loop.c
       hoists it into the pre-header, after the entry test, where the target sets $fp.
       The other three lose global allocation and reload rebuilds each address in
       $t0 at its use, as the target does. Using the tables directly: 47. */
    u16 (*strip32)[4]; /* FAKE: alias of D_8009BA00 */
    u16 *alt32; /* FAKE: alias of D_8009BA50 */
    u16 (*strip16)[4]; /* FAKE: alias of D_8009BA30 */
    u16 *alt16; /* FAKE: alias of D_8009BA58 */
''')
sub('''    *(s32 *)D_800A3490 = 0x2F;
    for (i = 0; D_800F1198[i].unk0 & 1; i++) {
''', '''    *(s32 *)D_800A3490 = 0x2F;
    alt32 = D_8009BA50; /* FAKE: alias */
    strip16 = D_8009BA30; /* FAKE: alias */
    alt16 = D_8009BA58; /* FAKE: alias */
    for (i = 0; D_800F1198[i].unk0 & 1; i++) {
        strip32 = D_8009BA00; /* FAKE: alias, set here so loop.c hoists it */
''')
sub('''            D_800A348C = D_800A3488 = (s32)D_8009BA00[(u32)D_800A32B8 % 6];
            if (D_8009BD44[0] & 1) {
                D_800A348C = (s32)D_8009BA50;''', '''            /* FAKE: `- strip32 + strip32` round trip (chain-extender): combine folds
               it away (zero bytes), but flow.c has already counted the two extra
               uses of strip32, so global.c ranks it above sv/flag and gives it $fp,
               as the target has it. Without it: 15. */
            D_800A348C = D_800A3488 = (s32)strip32[(u32)D_800A32B8 % 6] - (s32)strip32 + (s32)strip32;
            if (D_8009BD44[0] & 1) {
                D_800A348C = (s32)alt32;''')
sub('''            D_800A348C = D_800A3488 = (s32)D_8009BA30[D_800A32B8 & 3];
            if (D_8009BD44[0] & 1) {
                D_800A348C = (s32)D_8009BA58;''', '''            /* FAKE: the same round trip (chain-extender); combine re-emits the add
               with the index first, `addu $v0,$v0,$t0` as the target. Without it: 1. */
            D_800A348C = D_800A3488 = (s32)strip16[D_800A32B8 & 3] - (s32)strip16 + (s32)strip16;
            if (D_8009BD44[0] & 1) {
                D_800A348C = (s32)alt16;''')
open(D + 'cand_split0.c', 'w', encoding='utf-8', newline='\n').write(s)
print('ok')
