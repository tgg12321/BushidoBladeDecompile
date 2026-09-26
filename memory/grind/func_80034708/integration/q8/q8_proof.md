# Q8 proof — func_80034708 / src/code6cac_b3.c (-G8 vs -G0, full per-file pipeline)

## Build command lines

-G8:
```
mipsel-linux-gnu-cpp -Itmp/func_80034708/integ/include -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C tmp/func_80034708/integ/src/code6cac_b3.c | tools/gcc-2.7.2/build/cc1 -O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float | python3 tools/prologue_fix.py | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --expand-lb | sed "s/\.align\t3/.align\t2/" | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o tmp/func_80034708/q8/b3-G8.o
```

-G0:
```
mipsel-linux-gnu-cpp -Itmp/func_80034708/integ/include -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C tmp/func_80034708/integ/src/code6cac_b3.c | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float | python3 tools/prologue_fix.py | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --expand-lb | sed "s/\.align\t3/.align\t2/" | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o tmp/func_80034708/q8/b3-G0.o
```

## D_800A37B8 (s32, 4 bytes) — 4 instructions at -G8, 4 at -G0, identical: True

| -G8 offset | -G8 word | -G8 reloc | -G0 offset | -G0 word | -G0 reloc | insn |
|---|---|---|---|---|---|---|
| 0 | 3c020000 | R_MIPS_HI16 D_800A37B8 | 0 | 3c020000 | R_MIPS_HI16 D_800A37B8 | `lui v0,0x0` |
| 4 | 8c420000 | R_MIPS_LO16 D_800A37B8 | 4 | 8c420000 | R_MIPS_LO16 D_800A37B8 | `lw v0,0(v0)` |
| 38 | 3c010000 | R_MIPS_HI16 D_800A37B8 | 38 | 3c010000 | R_MIPS_HI16 D_800A37B8 | `lui at,0x0` |
| 3c | ac220000 | R_MIPS_LO16 D_800A37B8 | 3c | ac220000 | R_MIPS_LO16 D_800A37B8 | `sw v0,0(at)` |

## D_800A3690 (u8, 1 byte) — 12 instructions at -G8, 12 at -G0, identical: True

| -G8 offset | -G8 word | -G8 reloc | -G0 offset | -G0 word | -G0 reloc | insn |
|---|---|---|---|---|---|---|
| 2e0 | 3c060000 | R_MIPS_HI16 D_800A3690 | 2e8 | 3c060000 | R_MIPS_HI16 D_800A3690 | `lui a2,0x0` |
| 2e4 | 90c60000 | R_MIPS_LO16 D_800A3690 | 2ec | 90c60000 | R_MIPS_LO16 D_800A3690 | `lbu a2,0(a2)` |
| 514 | 3c020000 | R_MIPS_HI16 D_800A3690 | 51c | 3c020000 | R_MIPS_HI16 D_800A3690 | `lui v0,0x0` |
| 518 | 90420000 | R_MIPS_LO16 D_800A3690 | 520 | 90420000 | R_MIPS_LO16 D_800A3690 | `lbu v0,0(v0)` |
| 67c | 3c020000 | R_MIPS_HI16 D_800A3690 | 684 | 3c020000 | R_MIPS_HI16 D_800A3690 | `lui v0,0x0` |
| 680 | 90420000 | R_MIPS_LO16 D_800A3690 | 688 | 90420000 | R_MIPS_LO16 D_800A3690 | `lbu v0,0(v0)` |
| 68c | 3c010000 | R_MIPS_HI16 D_800A3690 | 694 | 3c010000 | R_MIPS_HI16 D_800A3690 | `lui at,0x0` |
| 690 | a0220000 | R_MIPS_LO16 D_800A3690 | 698 | a0220000 | R_MIPS_LO16 D_800A3690 | `sb v0,0(at)` |
| 810 | 3c020000 | R_MIPS_HI16 D_800A3690 | 818 | 3c020000 | R_MIPS_HI16 D_800A3690 | `lui v0,0x0` |
| 814 | 90420000 | R_MIPS_LO16 D_800A3690 | 81c | 90420000 | R_MIPS_LO16 D_800A3690 | `lbu v0,0(v0)` |
| 828 | 3c010000 | R_MIPS_HI16 D_800A3690 | 830 | 3c010000 | R_MIPS_HI16 D_800A3690 | `lui at,0x0` |
| 82c | a0220000 | R_MIPS_LO16 D_800A3690 | 834 | a0220000 | R_MIPS_LO16 D_800A3690 | `sb v0,0(at)` |

## D_800A36F9 (u8, 1 byte) — 14 instructions at -G8, 14 at -G0, identical: True

| -G8 offset | -G8 word | -G8 reloc | -G0 offset | -G0 word | -G0 reloc | insn |
|---|---|---|---|---|---|---|
| 2ac | 3c060000 | R_MIPS_HI16 D_800A36F9 | 2b4 | 3c060000 | R_MIPS_HI16 D_800A36F9 | `lui a2,0x0` |
| 2b0 | 90c60000 | R_MIPS_LO16 D_800A36F9 | 2b8 | 90c60000 | R_MIPS_LO16 D_800A36F9 | `lbu a2,0(a2)` |
| 4f4 | 3c020000 | R_MIPS_HI16 D_800A36F9 | 4fc | 3c020000 | R_MIPS_HI16 D_800A36F9 | `lui v0,0x0` |
| 4f8 | 90420000 | R_MIPS_LO16 D_800A36F9 | 500 | 90420000 | R_MIPS_LO16 D_800A36F9 | `lbu v0,0(v0)` |
| 504 | 3c010000 | R_MIPS_HI16 D_800A36F9 | 50c | 3c010000 | R_MIPS_HI16 D_800A36F9 | `lui at,0x0` |
| 508 | a0220000 | R_MIPS_LO16 D_800A36F9 | 510 | a0220000 | R_MIPS_LO16 D_800A36F9 | `sb v0,0(at)` |
| 65c | 3c020000 | R_MIPS_HI16 D_800A36F9 | 664 | 3c020000 | R_MIPS_HI16 D_800A36F9 | `lui v0,0x0` |
| 660 | 90420000 | R_MIPS_LO16 D_800A36F9 | 668 | 90420000 | R_MIPS_LO16 D_800A36F9 | `lbu v0,0(v0)` |
| 66c | 3c010000 | R_MIPS_HI16 D_800A36F9 | 674 | 3c010000 | R_MIPS_HI16 D_800A36F9 | `lui at,0x0` |
| 670 | a0220000 | R_MIPS_LO16 D_800A36F9 | 678 | a0220000 | R_MIPS_LO16 D_800A36F9 | `sb v0,0(at)` |
| 754 | 3c0a0000 | R_MIPS_HI16 D_800A36F9 | 75c | 3c0a0000 | R_MIPS_HI16 D_800A36F9 | `lui t2,0x0` |
| 758 | 914a0000 | R_MIPS_LO16 D_800A36F9 | 760 | 914a0000 | R_MIPS_LO16 D_800A36F9 | `lbu t2,0(t2)` |
| 808 | 3c010000 | R_MIPS_HI16 D_800A36F9 | 810 | 3c010000 | R_MIPS_HI16 D_800A36F9 | `lui at,0x0` |
| 80c | a0220000 | R_MIPS_LO16 D_800A36F9 | 814 | a0220000 | R_MIPS_LO16 D_800A36F9 | `sb v0,0(at)` |
