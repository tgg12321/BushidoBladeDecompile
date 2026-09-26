#!/bin/bash
# make a standalone permuter base.c from a sandbox-form candidate: mkbase.sh <cand.c> <permdir>
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
C="$1"; W="$2"
mkdir -p "$W"
{ cat <<'EOF'
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
extern u8 D_80101EC8;
extern s16 D_800A38DC;
extern u8 D_800A3783;
extern u8 D_800A3680;
extern u8 D_800A3671;
extern u8 D_800A37A0;
extern u8 D_800A389A;
extern u8 D_800A37D2;
extern u8 D_800A38E2;
extern void func_8005509C(s32 arg0);
typedef struct {
    u16 flags;
    u8 unk2;
    u8 unk3;
    u8 unk4[0x18 - 4];
} StatusFlagRec;
extern StatusFlagRec D_80099D88[];
extern u8 cpu_practice_honmokuroku_data_tbl[][4];
EOF
sed -e '1,7d' -e '/asm("cpu_practice/d' -e 's/g_sfr\[/D_80099D88[/g' -e 's/g_cpt\[/cpu_practice_honmokuroku_data_tbl[/g' "$C"; } > "$W/base.c"
for f in compile.sh mktarget.sh settings.toml target.o prelude_r3k.inc target_r3k.s; do
  [ "$W" != tmp/func_80055138/perm ] && cp tmp/func_80055138/perm/$f "$W/" 2>/dev/null
done
grep -c . "$W/base.c"
