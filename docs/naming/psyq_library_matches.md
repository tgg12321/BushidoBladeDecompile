# PsyQ Library Match Proposals

High-confidence proposals that match PsyQ stdlib/BIOS idioms. These are the lowest-risk renames in the corpus. The detection heuristics in `tools/propose_function_names.py` are conservative and require both shape signals AND tight loop body pairing.


- PsyQ memcpy/memset: **5**
- BIOS A0/B0/C0 jumptable wrappers: **27**
- Raw syscall/break wrappers: **3**

## BIOS jumptable wrappers

| address | current | proposed BIOS name | evidence |
|---|---|---|---|
| `0x80078824` | `func_80078824` | `bios_local_80078824` | address_neighborhood=bios_local_80078824 |
| `0x800788B0` | `func_800788B0` | `bios_local_800788B0` | address_neighborhood=bios_local_800788B0 |
| `0x80078A68` | `func_80078A68` | `bios_local_80078A68` | address_neighborhood=bios_local_80078A68 |
| `0x80078B04` | `func_80078B04` | `bios_local_80078B04` | address_neighborhood=bios_local_80078B04 |
| `0x80078B3C` | `func_80078B3C` | `bios_local_80078B3C` | address_neighborhood=bios_local_80078B3C; string_adjacent_info=? |
| `0x80078B70` | `func_80078B70` | `bios_local_80078B70` | address_neighborhood=bios_local_80078B70; string_adjacent_info=? |
| `0x80078BA8` | `func_80078BA8` | `bios_local_80078BA8` | address_neighborhood=bios_local_80078BA8 |
| `0x80078BE0` | `func_80078BE0` | `bios_local_80078BE0` | address_neighborhood=bios_local_80078BE0 |
| `0x80078BF0` | `func_80078BF0` | `bios_local_80078BF0` | address_neighborhood=bios_local_80078BF0 |
| `0x80078C00` | `func_80078C00` | `bios_local_80078C00` | address_neighborhood=bios_local_80078C00 |
| `0x80078D38` | `func_80078D38` | `bios_local_80078D38` | address_neighborhood=bios_local_80078D38 |
| `0x80078DA0` | `func_80078DA0` | `bios_local_80078DA0` | address_neighborhood=bios_local_80078DA0 |
| `0x80078E58` | `func_80078E58` | `bios_local_80078E58` | address_neighborhood=bios_local_80078E58 |
| `0x80078EC0` | `func_80078EC0` | `bios_local_80078EC0` | address_neighborhood=bios_local_80078EC0 |
| `0x80078F60` | `func_80078F60` | `bios_local_80078F60` | address_neighborhood=bios_local_80078F60 |
| `0x80078F88` | `func_80078F88` | `bios_local_80078F88` | address_neighborhood=bios_local_80078F88 |
| `0x80078FF0` | `func_80078FF0` | `bios_local_80078FF0` | address_neighborhood=bios_local_80078FF0 |
| `0x80079028` | `func_80079028` | `bios_local_80079028` | address_neighborhood=bios_local_80079028 |
| `0x800790A4` | `func_800790A4` | `bios_local_800790A4` | address_neighborhood=bios_local_800790A4 |
| `0x800790C0` | `func_800790C0` | `bios_local_800790C0` | address_neighborhood=bios_local_800790C0 |
| `0x8007A28C` | `func_8007A28C` | `bios_local_8007A28C` | address_neighborhood=bios_local_8007A28C |
| `0x8007A318` | `func_8007A318` | `bios_local_8007A318` | address_neighborhood=bios_local_8007A318 |
| `0x8007A400` | `func_8007A400` | `bios_local_8007A400` | address_neighborhood=bios_local_8007A400 |
| `0x8007A458` | `func_8007A458` | `bios_local_8007A458` | address_neighborhood=bios_local_8007A458 |
| `0x8007A4D8` | `func_8007A4D8` | `bios_local_8007A4D8` | address_neighborhood=bios_local_8007A4D8 |
| `0x800832A0` | `func_800832A0` | `bios_local_800832A0` | address_neighborhood=bios_local_800832A0 |
| `0x8008339C` | `func_8008339C` | `bios_local_8008339C` | address_neighborhood=bios_local_8008339C |

## PsyQ memcpy/memset

| address | current | proposed | evidence |
|---|---|---|---|
| `0x80016768` | `func_80016768` | `psyq_memset_80016768` | psyq_idiom=psyq_memset_80016768 |
| `0x8003043C` | `func_8003043C` | `psyq_memset_8003043C` | psyq_idiom=psyq_memset_8003043C; sole_caller_path=mario_test_Exec_helper_8003043C |
| `0x80038148` | `func_80038148` | `psyq_memset_80038148` | psyq_idiom=psyq_memset_80038148; sole_caller_path=pad_FuncAnalog_helper_80038148 |
| `0x8005509C` | `func_8005509C` | `psyq_memset_8005509C` | psyq_idiom=psyq_memset_8005509C |
| `0x8007DEE4` | `func_8007DEE4` | `psyq_memset_8007DEE4` | psyq_idiom=psyq_memset_8007DEE4 |

## Syscall / break wrappers

| address | current | proposed | evidence |
|---|---|---|---|
| `0x800164F8` | `func_800164F8` | `syscall_wrapper_break_800164F8` | syscall_wrapper=syscall_wrapper_break_800164F8 |
| `0x80083698` | `func_80083698` | `syscall_wrapper_break_80083698` | syscall_wrapper=syscall_wrapper_break_80083698 |
| `0x8008393C` | `func_8008393C` | `syscall_wrapper_break_8008393C` | syscall_wrapper=syscall_wrapper_break_8008393C |

