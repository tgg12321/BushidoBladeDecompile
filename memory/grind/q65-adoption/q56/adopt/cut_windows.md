## Cut-window records (pre-move layout at step01 = fbf47cc34, base f5414d005)

### Boundary in `code6cac_b_tu2` for `D_800A3140`

- gp user that must stay in the earlier part: `func_8002AB08`; direct (lui/%lo) user that must be in the later part: `func_800343F0`.
- Window: cut before any of `func_8002BC68`, `func_8002BEA0`, `func_8002C0DC`, `func_8002C22C`, `func_8002C61C`, `func_8002CA8C`, `func_8002CD58`, `func_8002D320`, `func_8002D518`, `func_8002D780`, `func_8002DAD0`, `func_8002DE20`, `func_8002E6B0`, `func_8002E838`, `func_8002EA24`, `func_8002EBDC`, `func_8002EECC`, `func_8002F2D0`, `func_8002F770`, `func_8002FC80`, `func_8002FDB0`, `func_8002FF20`, `func_800300B4`, `func_80030208`, `func_8003032C`, `func_8003043C`, `func_8003047C`, `func_80030524`, `func_80030580`, `func_800307D0`, `func_80030900`, `cpu_set_move_command_and_dir`, `func_80030B10`, `func_80030BA8`, `func_80030D04`, `func_80030D48`, `math_LerpAngle`, `func_80030D7C`, `func_80031890`, `func_80031B24`, `func_80032040`, `func_80032064`, `func_800321E8`, `func_80032314`, `func_800324D0`, `func_800325E0`, `func_80032854`, `func_80032C50`, `cpu_check_same_dir_timer`, `func_80033498`, `func_80033510`, `func_80033550`, `func_800335D8`, `func_80033898`, `func_800338CC`, `func_80033BC0`, `func_80033D38`, `func_80033DF4`, `func_80033FE4`, `func_800340A0`, `func_80034200`, `func_800342A0`, `func_800343F0`.

| cut before | gp symbols reached from both sides | later part's first compiled-rodata user | in the rodata-align window | result |
|---|---|---|---|---|
| `func_8002BC68` | none | `func_800324D0` | no | fails |
| `func_8002BEA0` | none | `func_800324D0` | no | fails |
| `func_8002C0DC` | none | `func_800324D0` | no | fails |
| `func_8002C22C` | none | `func_800324D0` | no | fails |
| `func_8002C61C` | none | `func_800324D0` | no | fails |
| `func_8002CA8C` | none | `func_800324D0` | no | fails |
| `func_8002CD58` | none | `func_800324D0` | no | fails |
| `func_8002D320` | none | `func_800324D0` | no | fails |
| `func_8002D518` | none | `func_800324D0` | no | fails |
| `func_8002D780` | none | `func_800324D0` | no | fails |
| `func_8002DAD0` | none | `func_800324D0` | no | fails |
| `func_8002DE20` | none | `func_800324D0` | no | fails |
| `func_8002E6B0` | none | `func_800324D0` | no | fails |
| `func_8002E838` | none | `func_800324D0` | no | fails |
| `func_8002EA24` | none | `func_800324D0` | no | fails |
| `func_8002EBDC` | none | `func_800324D0` | no | fails |
| `func_8002EECC` | none | `func_800324D0` | no | fails |
| `func_8002F2D0` | none | `func_800324D0` | no | fails |
| `func_8002F770` | none | `func_800324D0` | no | fails |
| `func_8002FC80` | none | `func_800324D0` | no | fails |
| `func_8002FDB0` | none | `func_800324D0` | no | fails |
| `func_8002FF20` | none | `func_800324D0` | no | fails |
| `func_800300B4` | none | `func_800324D0` | no | fails |
| `func_80030208` | none | `func_800324D0` | no | fails |
| `func_8003032C` | none | `func_800324D0` | no | fails |
| `func_8003043C` | none | `func_800324D0` | no | fails |
| `func_8003047C` | none | `func_800324D0` | no | fails |
| `func_80030524` | none | `func_800324D0` | no | fails |
| `func_80030580` | none | `func_800324D0` | no | fails |
| `func_800307D0` | none | `func_800324D0` | no | fails |
| `func_80030900` | D_800A36F2 | `func_800324D0` | no | fails |
| `cpu_set_move_command_and_dir` | D_800A36F2 | `func_800324D0` | no | fails |
| `func_80030B10` | D_800A36F2 | `func_800324D0` | no | fails |
| `func_80030BA8` | D_800A36F2 | `func_800324D0` | no | fails |
| `func_80030D04` | D_800A36F2 | `func_800324D0` | no | fails |
| `func_80030D48` | D_800A36F2 | `func_800324D0` | no | fails |
| `math_LerpAngle` | D_800A36F2 | `func_800324D0` | no | fails |
| `func_80030D7C` | D_800A36F2 | `func_800324D0` | no | fails |
| `func_80031890` | D_800A36F2 | `func_800324D0` | no | fails |
| `func_80031B24` | D_800A36F2 | `func_800324D0` | no | fails |
| `func_80032040` | none | `func_800324D0` | no | fails |
| `func_80032064` | none | `func_800324D0` | no | fails |
| `func_800321E8` | none | `func_800324D0` | no | fails |
| `func_80032314` | none | `func_800324D0` | no | fails |
| `func_800324D0` | none | `func_800324D0` | no | fails |
| `func_800325E0` | none | `func_80032854` | no | fails |
| `func_80032854` | none | `func_80032854` | no | fails |
| `func_80032C50` | none | `func_80032C50` | no | fails |
| `cpu_check_same_dir_timer` | none | `func_80033498` | no | fails |
| `func_80033498` | none | `func_80033498` | no | fails |
| `func_80033510` | none | none | yes | survives |
| `func_80033550` | none | none | yes | survives |
| `func_800335D8` | none | none | yes | survives |
| `func_80033898` | none | none | yes | survives |
| `func_800338CC` | none | none | yes | survives |
| `func_80033BC0` | none | none | yes | survives |
| `func_80033D38` | none | none | yes | survives |
| `func_80033DF4` | none | none | yes | survives |
| `func_80033FE4` | none | none | yes | survives |
| `func_800340A0` | none | none | yes | survives |
| `func_80034200` | none | none | yes | survives |
| `func_800342A0` | none | none | yes | survives |
| `func_800343F0` | none | none | yes | survives (conventional) |

rodata-align conditions for the surviving positions: condition 2 - b_tu3 keeps its rodata start (INCLUDE_RODATA jtbl_8001084C at 0x8001081C; the functions after func_80033498 that could move own no compiled rodata before it); condition 3 - text order is unchanged and no gp symbol is shared across any surviving cut, so every surviving position gives identical bytes; the conventional position (immediately before func_800343F0, the function holding the access b_tu2's assembly cannot produce) is used, the others are recorded here; condition 4 - moves only (splitc.py + mergec.py, verbatim).

### Boundary in `text1a_c` for `D_800A3820`

- gp user that must stay in the earlier part: `func_80044504`; direct (lui/%lo) user that must be in the later part: `func_80044800`.
- Window: cut before any of `func_80044650`, `func_80044670`, `func_8004473C`, `func_80044800`.

| cut before | gp symbols reached from both sides | later part's first compiled-rodata user | in the rodata-align window | result |
|---|---|---|---|---|
| `func_80044650` | none | none | n/a | survives |
| `func_80044670` | none | none | n/a | survives |
| `func_8004473C` | none | none | n/a | survives |
| `func_80044800` | none | none | n/a | survives (conventional) |

current boundary: before func_80061064 (index 233)
valid cuts: before func_80047EE8 .. before func_80048AD0 (indices 1..16)
valid cuts: before func_80048F58 .. before func_8004939C (indices 19..21)
valid cuts: before func_80049E4C .. before func_80052D00 (indices 31..125)
valid cuts: before func_80054440 .. before snd_Init (indices 136..182)
valid cuts: before func_8005C614 .. before func_8005FBC8 (indices 207..219)
valid cuts: before func_800600C8 .. before func_80060758 (indices 221..225)
valid cuts: before func_80060A68 .. before func_80060A68 (indices 227..227)
valid cuts: before func_80068ECC .. before func_80068F70 (indices 326..327)
valid cuts: before func_8006E440 .. before func_8006E49C (indices 371..373)
shared across the current boundary: ['0x800a32bc', '0x800a3444', '0x800a3448', '0x800a345c', '0x800a345e', '0x800a3460', '0x800a3468', '0x800a346c', '0x800a3470', '0x800a3474', '0x800a3478', '0x800a347c', '0x800a3480', '0x800a3484', '0x800a3488', '0x800a348c', '0x800a3490', '0x800a3494', '0x800a3498', '0x800a349c', '0x800a34a0', '0x800a34a4', '0x800a34a8', '0x800a34ac', '0x800a34b0', '0x800a34b4', '0x800a34b8', '0x800a34bc', '0x800a34c0', '0x800a34c4', '0x800a34c8', '0x800a34cc', '0x800a34d0', '0x800a34d4', '0x800a34d8', '0x800a34dc', '0x800a34e0', '0x800a34e4', '0x800a34e8', '0x800a34ec']
