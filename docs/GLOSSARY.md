# Glossary

Terminology used throughout the BB2 decompilation. Organized by domain.

## PsyQ / PlayStation hardware

| Term | Definition |
|---|---|
| **PsyQ** | Sony's official PlayStation 1 development SDK (a.k.a. DTL-S3000 series). Version 3.5 is the toolchain BB2 was built with: GCC 2.7.2 (SN Systems fork) + ASPSX 2.34 assembler + PsyQ libraries (libgpu, libcd, libapi, libspu). |
| **ASPSX** | The PsyQ assembler. The BB2 build was made with ASPSX 2.34. Notable differences from GNU `as`: different pseudo-instruction expansion (`la`, `lb sym`, `sw reg,sym`), GP-relative addressing rules, `.set reorder` behavior, lb/lh expansion. The project replicates ASPSX with `maspsx` (a GNU `as` wrapper). |
| **cc1psx** | The C-frontend of GCC 2.7.2 SN Systems fork. PsyQ 3.5 ships with cc1psx that identifies as `GNU C 2.7.2.SN.1`. The vanilla decompals build identifies as `GNU C 2.7.2`. Byte-identical machine code across test inputs; the `.SN.1` differences are assembler-output format only. |
| **VAB** | Voice Attribute Bank — Sony's audio bank format. BB2 uses several: `GAME.BNK`, `GAMECHAN.BNK`, `GAMEDAN.BNK`, `GAMESTO.BNK`, `GAMETRA.BNK`. |
| **TIM** | Sony's standard texture image format. 4bpp / 8bpp / 16bpp paletted images for VRAM upload. |
| **TMD** | Sony's standard 3D model format. Sequences of textured triangles/quads for the GPU. |
| **BBM** | BB2-specific "Bushido Blade Motion" character animation bundle. Format undocumented publicly. |
| **STR** | Sony's standard streaming format for FMV. `disc/STR/OPENING.STR` is the opening movie. Uses MDEC video + XA-ADPCM audio. |
| **XA-ADPCM** | Compressed audio format used for streaming BGM and voice on PS1. `disc/XA_0/` and `disc/XA_1/` contain music/voice tracks. |
| **MDEC** | The PS1's "Macroblock Decoder" — a hardware video decoder for STR FMV playback. BB2's overlay (`MOVOVL.EXE`) contains MDEC decode functions. |
| **GPU** | PlayStation Graphics Processing Unit. Programmed via GP0 (data) / GP1 (control) registers and DMA. Renders into VRAM (1 MB, separate bus). |
| **SPU** | PlayStation Sound Processing Unit. 24 ADPCM voices, 512 KB sound RAM, reverb. Programmed via memory-mapped I/O. |
| **GTE** | "Geometry Transformation Engine" — PS1's coprocessor 2. Hardware vector/matrix math (rotation, translation, projection, perspective division). Programmed via `cop2`, `mtc2`, `mfc2`, `ctc2`, `cfc2`, `lwc2`, `swc2` instructions. PsyQ wraps the common ops in `gte_*()` C macros in `include/gte.h`. |
| **BIOS** | PS1 kernel ROM at `0xBFC00000`. Provides syscalls reached via A-vector (`jr $t2` to `0xA0`), B-vector (`0xB0`), and C-vector (`0xC0`) trampolines. The function number goes in `$t1` in the `jr` delay slot. BB2 has ~40 such trampolines, all authorized as canonical inline asm. |
| **Scratchpad RAM** | 1 KB of fast on-CPU SRAM at `0x1F800000`–`0x1F8003FF`. 8 ns access vs 250 ns for main RAM. Some BB2 functions use it for hot temporaries. |
| **KSEG0 / KSEG1 / KUSEG** | MIPS memory segments. KSEG0 (`0x80000000`+) is cached kernel space — where BB2 lives. KSEG1 (`0xA0000000`+) is uncached mirror for I/O. KUSEG (`0x00000000`+) is unused. |
| **VRAM** | The GPU's 1 MB texture/framebuffer memory on a separate bus. Not addressable from the CPU directly — accessed via GPU commands. |

## MIPS R3000A architecture

| Term | Definition |
|---|---|
| **R3000A** | The PS1's main CPU. 33.8 MHz MIPS I architecture. 32 general-purpose registers, 4 KB instruction cache, 1 KB data cache. |
| **MIPS I** | First-generation MIPS instruction set. 32-bit, RISC, fixed 4-byte instruction words, branch delay slots. |
| **Delay slot** | The instruction immediately after every branch and `jal`/`jr` ALWAYS executes, regardless of whether the branch was taken. Compilers fill delay slots with useful instructions when possible; otherwise `nop`. |
| **Branch-likely** | Variant branch instructions (e.g., `beql`, `bnel`) where the delay-slot instruction is annulled if the branch is NOT taken. GCC 2.7.2 doesn't emit these; ASPSX with `.set reorder` may. |
| **`$sp`** | Stack pointer ($29). Aligned to 8 bytes. Grows downward from `0x801FFFF0`. |
| **`$ra`** | Return address ($31). Set by `jal`, read by `jr $ra` for function return. Saved by callee in prologue. |
| **`$gp`** | Global pointer ($28). Set to `0x800A30CC` at startup. Loads addressed `gp + signed_16bit_offset` can reach `0x800A2CCC`–`0x800A48CC` in one instruction. |
| **`$at`** | Assembler temporary ($1). Reserved for the assembler's pseudo-instruction expansion. C code can't use it directly. |
| **`$v0`, `$v1`** | Return values ($2, $3). |
| **`$a0`–`$a3`** | Argument registers ($4–$7). First 4 args of a function call pass through these; rest go on the stack. |
| **`$t0`–`$t9`** | Temporary registers ($8–$15, $24, $25). Caller-saved (callee may clobber). |
| **`$s0`–`$s7`** | Saved registers ($16–$23). **Callee-saved** — if a function uses them, it must save them in prologue and restore in epilogue. |
| **`$k0`, `$k1`** | Kernel temporaries ($26, $27). Reserved for the kernel; user code may not touch them. |
| **`$gp`, `$sp`, `$fp`, `$ra`** | Special-purpose ($28, $29, $30, $31). |
| **GP-relative addressing** | `lw $reg, sym($gp)` — single-instruction load using `$gp` as base. Restricted to symbols in `.sdata`/`.sbss` (small data) within `±32 KB` of `$gp`. The project's `sdata_funcs.txt` and `sdata_syms.txt` control which symbols and functions use it. |
| **Pseudo-instruction** | An assembly mnemonic that expands to multiple machine instructions. Examples: `la $reg, sym` → `lui + ori` (or `lui + addiu`). `lb $reg, sym` → `lui + lbu + sll + sra` (in ASPSX with `.set noreorder`). `move $rd, $rs` → `addu $rd, $rs, $zero`. |
| **`.set noreorder`** | Assembler directive: don't auto-fill delay slots, treat each instruction literally. The default for PsyQ-style code. |
| **`.set reorder`** | Assembler directive: auto-fill branch delay slots by pulling the next instruction in, treat instruction ordering as semantic. ASPSX uses this for some hot loops. |
| **`.set at` / `.set noat`** | Allow / forbid the assembler to use `$at` for pseudo-instruction expansion. Inline asm that wants to use `$at` itself must do `.set noat`. |
| **Tail call (`jr $ra` without saving)** | A leaf function that just dispatches to another function — uses `j` (or `jr $tN`) to the target with the original `$ra` still in place, so the target returns directly to the caller. |
| **mflo / mfhi** | Move-from-LO / move-from-HI. Reads the low/high 32 bits of the 64-bit result of `mult`/`multu` or the quotient/remainder of `div`/`divu`. Has a 2-cycle latency hazard; ASPSX inserts `nop`s to avoid it. |

## Decompilation

| Term | Definition |
|---|---|
| **Matching decomp** | A decompilation methodology where the goal is to write source code that compiles to byte-identical output as the original. Distinguished from non-matching decomp (semantic reconstruction only) and disassembly (asm only). |
| **Byte-identical** | The rebuild produces the same bytes as the original. Verified via SHA1 comparison. |
| **splat** | A binary splitter ([ethteck/splat](https://github.com/ethteck/splat)) that takes a PS1/N64/etc. executable plus a YAML config and emits per-function assembly files, segmented data files, a linker script, and symbol-address tables. BB2 uses splat 0.39.0 via `make setup`. |
| **m2c** | An assembly-to-pseudo-C decompiler ([matt-kempster/m2c](https://github.com/matt-kempster/m2c)) used as a starting point for translating an `asm/funcs/<func>.s` into a `base.c` for matching. The output is rarely matching out of the box. |
| **decomp-permuter** | A randomized C-variation tool ([simonlindholm/decomp-permuter](https://github.com/simonlindholm/decomp-permuter)) that mutates a `base.c` and re-compiles, looking for variations that compile to a lower score (i.e., closer to target). |
| **Score** | A metric the permuter computes: lower is better, 0 is matching. Weighted (regs ×5, reorderings ×60, ins/del ×100) inside the permuter. A separate "objdump score" used by some scripts is just the raw count of differing 32-bit instruction words — never compare the two. |
| **Penalty profile** | The breakdown of score components: Insertions / Deletions / Reorderings / Register Differences / Stack Differences / Branch Differences. `bash tools/dc.sh debug <func_dir>` prints this. Used to route the next technique (see [`MATCHING.md`](MATCHING.md)). |
| **base.c** | The standalone C source used by the permuter for compilation. Lives at `permuter/<func>/base.c`. Often includes duplicate typedefs/externs that don't survive into the real `src/<file>.c`. |
| **target.s / target.o** | The original disassembly (`target.s`) compiled to object form (`target.o`), used by the permuter as the matching target. |
| **MATCH** | Verified outcome: function compiles to byte-identical asm as target. Reported by `dc.sh verify <func>` and by `make` (whole-binary). |
| **MISMATCH** | Function compiles but produces different bytes than target. Build succeeds; SHA1 fails. The diagnostic state. |
| **HARD / NEAR_MISS / MATCHED / SKIPPED** | Reports from `dc.sh attempt <func>`. SKIPPED = classifier said permanently blocked / canonical asm / not-a-function. NEAR_MISS = score ≤ 200 after the mechanical pipeline. HARD = score > 200. MATCHED = score = 0. |
| **regfix** | The per-function assembly-stream post-pass in BB2's build pipeline. Rewrites maspsx output before the assembler sees it. Configured by [`regfix.txt`](../regfix.txt). Operations: swap, subst, delete, insert, insert_after, insert_label, reorder, fill_delay, drain_delay. |
| **asmfix** | The per-function source-text post-pass that runs after regfix. Configured by [`asmfix.txt`](../asmfix.txt). Operations: rename, replace_first, delete_between, insert_before, `replace_with_asmfile`. |
| **Bridge** (a.k.a. **`replace_with_asmfile`**) | An asmfix rule that substitutes raw hand-disassembled MIPS for a function's C body at build time. Lets the build SHA1-match while real C body work is deferred. Counts as not-done. The project has ~148 active bridges. |
| **Retirement** (of a bridge) | Replacing a `replace_with_asmfile` bridge with a real C body that produces matching bytes. Uses `dc.sh retire <func>` (comments out the bridge with `# RETIRE:`), iterate, `dc.sh verify-c <func>` (refuses while bridge is active). |
| **INCLUDE_ASM stub** | A macro form (`INCLUDE_ASM("asm/funcs", funcname);`) historically used in `src/*.c` to declare "this function still lives in asm." All 1,410 stubs were cleared in the 2026-04-27 zero-stub sweep. Replaced now by inline `__asm__()` blocks or matched C bodies. |
| **Inline `__asm__()` block** | A file-scope `__asm__()` directive in `src/*.c` that emits raw assembly for a function. Used as the interim form for functions awaiting full pure-C decomp; the audit (`tools/audit_asm_cheats.py`) tracks these as `inline_asm_debt`. |
| **Canonical inline asm** | Functions whose ORIGINAL implementation was hand-written assembly (BIOS trampolines, GTE primitives, custom-ABI math kernels). The accepted final form is a file-scope `__asm__()` block. Tracked in [`inline_asm_canonical.txt`](../inline_asm_canonical.txt) with per-function justifications. |
| **LICM** | Loop-Invariant Code Motion. GCC optimization that hoists invariant computations out of loops into the preheader. Common matching pain point — when target keeps a constant inline but GCC hoists, you need a regfix unhoist recipe (C-level defeats don't work). |
| **CSE** | Common Subexpression Elimination. GCC optimization that deduplicates identical computations. Defeated via opaque barriers (`__asm__ volatile("" : "=r"(x) : "0"(x))`) or by introducing differences between expressions. |
| **DCE** | Dead Code Elimination. GCC optimization that removes computations whose results are unused. Can cause frame-size mismatches when target reserved a slot for an eliminated computation. |
| **Permuter base** | The starting `base.c` you hand to the permuter. Usually starts as m2c output, then manually cleaned. |
| **Stub** | A placeholder function definition that matches the prototype but doesn't compute correctly. Used during bringing-up work; not allowed as a final state. |
| **Cascade regression** | A change to one function silently breaks another (often via `.L<N>` label-counter shift in the same compilation unit, or via caller-arity mismatches). Detected by `dc.sh verify --all` and `post-match-validate`. |
| **Label drift** | GCC's per-file `.L<N>` label counter shifts when functions are added or removed from the same `.c` file. regfix substs that hardcode `.L<N>` literals silently break. `dc.sh regfix-drift-immune` rewrites them to `\.L\d+` regex. |
| **`.L<N>`** | GCC's auto-generated label format for local labels (jump targets within a function). `<N>` is an integer that increments monotonically per compilation unit. |
| **Sibling regression** | Specifically: a label-drift cascade regression where matching one function breaks others in the same `.c` file. |
| **maspsx** | A wrapper around GNU `as` ([mkst/maspsx](https://github.com/mkst/maspsx)) that emulates ASPSX 2.34 behavior. Vendored in `tools/maspsx/`. |
| **work queue** | The canonical ordered list of remaining functions to decompile. Lives at [`WORK_QUEUE.md`](../WORK_QUEUE.md). Three sub-queues: active decomp, structural split, asmfix retirement. |
| **active marker** | The single in-progress function name, stored in `.bb2_active_func`. Set by `dc.sh next`, cleared by a successful matching commit. The hook (`tools/hooks/active_func_guard.sh`) gates `git commit`, `git checkout`, and queue pulls based on its state. |
| **Kengo reference** | *Kengo: Master of Bushido* (PS2, 2000) was built by the same studio (Lightweight) on the same Marionation engine. Its debug symbols (~2,500 named functions extracted via `ccc`/`stdump`) live in `Kengo/` and are used as a naming source for BB2 functions via `tools/kengo_match.py`. |

## BB2-specific game terminology

Many function and global names in the source use Japanese romanization, inherited from the original developers' naming. The Kengo PS2 symbols (same studio, same engine) provided a Rosetta stone for most of these.

| Term | Meaning |
|---|---|
| **Marionation** | The proprietary engine name (string `"Marionation over flow. No.%d (-%dbyte)"` in BB2's rodata). Marionette + animation. Used in BB2 (PS1) and later in *Kengo: Master of Bushido* (PS2). |
| **CHANBARA** (チャンバラ) | Chambara/sword-fighting genre. Likely the core combat-system name (`CHANBARA0.DAT`, `CHANBARA1.DAT` data files). |
| **saTan** (`saTan0`, `saTan1`, ...) | "Sa" = sato (developer surname), "tan" = tantou / tan(?). Sato-san's source files in Kengo: `sa_tan0`–`sa_tan5` and `sa_tan_m`. Likely effect/gauge modules. BB2's `saTan0GaugeDraw`, `saTan2Main`, `saTan4GaugeInit`, etc. mirror these. |
| **saSe** | Sato's sound-effect module (Kengo: `sa_se`). |
| **saFid** | Sato's file-handling module. `saFidLoad` is a generic file loader. |
| **myRob** | Myama's robot/character-model module (Kengo: `my_rob`). Robot = the character mesh. |
| **myEff** | Myama's effects module (Kengo: `my_eff`). |
| **myHira** / **hirahira** (ひらひら) | Cloth/leaf flutter simulation (Kengo: `my_hirahira`). |
| **nm_camera**, **nm_cpu**, **nm_single_game**, **nm_mario_cam** | Numata's modules: camera, AI ("CPU"), single-player game, Mario-style camera. |
| **is_action**, **is_coli**, **is_motion**, **is_pad**, **is_replay**, **is_status** | Ishito's combat core: action, collision, motion, controller, replay, status. |
| **hi_gnd**, **hi_landhit**, **hi_curpad** | Hide's modules: ground/terrain, landing/hit, current-pad. |
| **am_rmd**, **am_jimaku** | Amami's 3D models, subtitles. |
| **se_fc**, **se_qt** | Sekikawa's face-control, qt (subtitle quality?). |
| **kamae** (構え) | Stance (combat ready position). Animation states: `KAMAE`, `KAMAE_KA`. |
| **buki** (武器) | Weapon. `AllocBukiRmd` allocates a weapon model. |
| **zangeki** (斬撃) | Slash attack. `action_CheckHitZangeki` checks if a slash hit. |
| **maai** (間合い) | Combat distance / spacing. |
| **kawashi** (躱し) | Dodge. |
| **sundome** (寸止め) | Pulled strike (stopping a sword just before contact). |
| **ougi** (奥義) | Special move / secret technique. |
| **katanuki** / **katinuki** (勝ち抜き) | Survival mode / elimination mode (literally "winning through"). |
| **kaishaku** (介錯) | Execution / coup de grâce. Animation state: `KAISHAKU_ST`. |
| **karami** (絡み) | Grapple. Animation states: `KARAMI`, `KARAMI_ED`. |
| **noboru** (登る) / **anobori** | Climb. Animation states: `NOBORI_S` (start), `NOBORI_E` (end), `ANOBORI`. |
| **furi** (振り) | Swing. Animation states: `FURI1`, `FURI2`. |
| **yuri** (揺り) | Sway. Animation states: `YURI1`, `YURI2`. |
| **homaru** / **hom** | Homing (attack). Animation states: `HOM_ST` (start), `HOM_AT` (attack), `HOM_ED` (end). |
| **chakuti** (着地) | Landing. Animation state: `CHAKUTI`. |
| **syagami** (しゃがみ) | Crouch. Animation state: `SYAGAMI`. |
| **gokaku** (互角) | Equal stance / parity. Animation state: `GOKAKU`. |
| **uke** (受け) | Receive / block. Animation state: `UKE`. |
| **haji** (はじ?) | Parry / deflect. Animation state: `HAJI`. |
| **dth** | Death. Animation state: `DTH`. |
| **okiagari** (起き上がり) | Get up (after being knocked down). Animation state: `OKIAGARI`. |
| **suna** (砂) | Sand. Animation state: `SUNA`. |
| **seri** (せり) | Attack series / combo. Animation states: `SERI`, `SERIEXIT`. |
| **syastep** | Step (variant). Animation state: `SYASTEP`. |
| **subwep** | Sub-weapon. Animation state: `SUBWEP`. |
| **md_game** | Game-mode state machine (Kengo: `src/common/md_game.c`). |
| **tslPrintScreen**, **tslTm2LoadImage**, etc. | Sato's "tsl" rendering engine (texture/sprite library?). |
| **jimaku** (字幕) | Subtitles (Kengo: `am_jimaku`). |
| **tanren** (鍛錬) | Training mode. |
| **fade** (フェード) | Screen fade in/out (`src/common/fade.c` in Kengo). |
| **clip** | Clipping (rendering). |
| **judge** | Judge / referee logic (round end, winner determination). |
| **practice** (`func_8001A820`, `DispPracticeMenuTex_A`/`_B`/`_C`, etc.) | Practice mode. |
| **replay** (`replay_camera_*`) | Replay system. |
| **special_camera** | Special-camera (cinematic) system. |
| **gnd** (ground) | Stage geometry / terrain. |

## File formats (BB2-specific)

| Extension | Format |
|---|---|
| **`.BBM`** | Bushido Blade Motion — character animation bundle. Undocumented. Loaded from `disc/MOTION/`. |
| **`.SE`** | Sound effect data. Loaded from `disc/LOADSE/`, `disc/LOADSE1/`. |
| **`.BNK`** | Sound bank (Sony VAB format). `GAME.BNK`, `GAMECHAN.BNK`, etc. |
| **`STAGE##.BIN`** | Stage geometry. 38 stages in `disc/LOADSE*/`. |
| **`STG##.BIN`** | Stage background image. In `disc/U_PIC/`. |
| **`NDATA.DAT`** + **`NDATA.INF`** | Large packed data archive (~55 MB) + index. Contains many in-game assets. |
| **`MAR00.DAT`** – **`MAR22.DAT`** | "Marionation" data — likely animation/skeleton definitions per-character. In `disc/` data dirs. |
| **`M000.DAT`** – **`M306.DAT`** | Character/model data. Embedded path: `DATA0\`. |
| **`CHANBARA0.DAT`** / **`CHANBARA1.DAT`** | Core combat data. Embedded path: `DATA0\`. |
| **`OPENING.STR`** | The opening FMV. Sony STR format (MDEC video + XA audio). |
| **`MOVOVL.EXE`** | FMV playback overlay. PS-X EXE format, loads at `0x801D8800`. Contains MDEC decode functions. |
| **`TITLE.TIM`** | Title screen image. Sony TIM format. |
