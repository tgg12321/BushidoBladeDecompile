# Naming Proposals -- High confidence

**High confidence**: deterministic pattern (BIOS jumptable, raw syscall, data-as-code, recognizable PsyQ stdlib idiom) OR a Kengo `name-unique` match with diff <=1 instruction. These are the lowest-risk renames; review and apply in small batches with SHA1 verification.

**Risk**: BIOS A0/B0/C0 lookups are deterministic from the trampoline shape; PsyQ memcpy/memset proposals are address-suffixed to avoid name collisions if multiple wrappers exist for the same primitive.

**Workflow**: copy the proposed name into `named_syms.txt`, run `make setup && make`, verify SHA1 unchanged, commit.

Total High: **10**

## Primary evidence: `psyq_idiom` (5)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80016768` | `func_80016768` | `psyq_memset_80016768` | psyq_idiom=psyq_memset_80016768 | [md](evidence/func_80016768.md) |
| `0x8003043C` | `func_8003043C` | `psyq_memset_8003043C` | psyq_idiom=psyq_memset_8003043C; sole_caller_path=mario_test_Exec_helper_8003043C | [md](evidence/func_8003043C.md) |
| `0x80038148` | `func_80038148` | `psyq_memset_80038148` | psyq_idiom=psyq_memset_80038148; sole_caller_path=pad_FuncAnalog_helper_80038148 | [md](evidence/func_80038148.md) |
| `0x8005509C` | `func_8005509C` | `psyq_memset_8005509C` | psyq_idiom=psyq_memset_8005509C | [md](evidence/func_8005509C.md) |
| `0x8007DEE4` | `func_8007DEE4` | `psyq_memset_8007DEE4` | psyq_idiom=psyq_memset_8007DEE4 | [md](evidence/func_8007DEE4.md) |

## Primary evidence: `syscall_wrapper` (3)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x800164F8` | `func_800164F8` | `syscall_wrapper_break_800164F8` | syscall_wrapper=syscall_wrapper_break_800164F8 | [md](evidence/func_800164F8.md) |
| `0x80083698` | `func_80083698` | `syscall_wrapper_break_80083698` | syscall_wrapper=syscall_wrapper_break_80083698 | [md](evidence/func_80083698.md) |
| `0x8008393C` | `func_8008393C` | `syscall_wrapper_break_8008393C` | syscall_wrapper=syscall_wrapper_break_8008393C | [md](evidence/func_8008393C.md) |

## Primary evidence: `data_as_code` (1)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x800164AC` | `func_800164AC` | `data_as_code_lb_table_800164AC` | data_as_code=data_as_code_lb_table_800164AC | [md](evidence/func_800164AC.md) |

## Primary evidence: `kengo_pattern` (1)

| address | current | proposed | evidence_summary | evidence_file |
|---|---|---|---|---|
| `0x80060758` | `func_80060758` | `replay_camera_check_stage_80060758` | kengo_pattern=replay_camera_check_stage_80060758; sole_caller_path=SetCurrentCursor_helper_80060758 | [md](evidence/func_80060758.md) |

