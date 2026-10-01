#ifndef BIOS_H
#define BIOS_H

/* PSX BIOS A/B/C-vector trampolines.
 *
 * Every BIOS call in the PsyQ runtime is a 3-instruction stub that jumps to
 * the kernel vector table with the function number in $t1:
 *
 *     addiu $t2, $zero, <vector>    ; 0xA0 / 0xB0 / 0xC0
 *     jr    $t2
 *     addiu $t1, $zero, <id>        ; delay slot
 *
 * followed by one nop of padding. GCC 2.7.2 cannot emit this (a fixed-vector
 * tail jump with the id loaded into $t1 in the delay slot), so the stub is
 * canonical hand-written asm (inline_asm_canonical.txt, BIOS trampolines).
 * Same shape as sotn-decomp's include/bios.h BIOS_FUNCTION.
 *
 * The engine recognises a BIOS_[ABC]_FUNCTION(name, id) invocation as a
 * whole-body asm function (engine/inlineasm.py include_asm_spans).
 * Non-PSX builds (M2CTX / PERMUTER) expand it to nothing, like INCLUDE_ASM.
 */

#if !defined(M2CTX) && !defined(PERMUTER)

#define BIOS_FUNCTION(name, vector, id)                                        \
    __asm__(                                                                   \
        ".section .text\n"                                                     \
        "    .set noat\n"                                                      \
        "    .set noreorder\n"                                                 \
        "glabel " #name "\n"                                                   \
        "    addiu $t2, $zero, " #vector "\n"                                  \
        "    jr    $t2\n"                                                      \
        "    addiu $t1, $zero, " #id "\n"                                      \
        "    nop\n"                                                            \
        "endlabel " #name "\n"                                                 \
        "    .set reorder\n"                                                   \
        "    .set at\n"                                                        \
    )

#else

#define BIOS_FUNCTION(name, vector, id)

#endif

#define BIOS_A_FUNCTION(name, id) BIOS_FUNCTION(name, 0xA0, id)
#define BIOS_B_FUNCTION(name, id) BIOS_FUNCTION(name, 0xB0, id)
#define BIOS_C_FUNCTION(name, id) BIOS_FUNCTION(name, 0xC0, id)

#endif /* BIOS_H */
