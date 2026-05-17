# D_800EFB7C -- naming evidence

- Address: `0x800EFB7C`
- Proposed name: `g_sound_buf_e`
- Confidence: **medium**

## Storage / access pattern

.bss block of bytes set to value v by func_8005B43C (a memset-like
loop).  Part of the sound-buffer cluster.

## Usage in src/

src/text1b.c -- `func_80058580`: extern s32 D_800EFB7C;

src/text1b.c -- `func_8005B43C`:
  *(s8 *)((u8 *)&D_800EFB7C + j) = v;

## Why this name

Fifth sound-buffer (extends the A/B/C/D cluster at
D_800EFB38/78/C38).
