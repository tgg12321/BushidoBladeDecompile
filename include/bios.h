#ifndef BIOS_H
#define BIOS_H

/* PSX BIOS A/B/C-vector trampolines: a stub that jumps to the kernel vector
 * (0xA0 / 0xB0 / 0xC0) with the function number loaded into $t1 in the delay
 * slot, plus one nop of padding. GCC 2.7.2 cannot emit this, so it is
 * hand-written asm, as in sotn-decomp's include/bios.h BIOS_FUNCTION.
 * Non-PSX builds (M2CTX / PERMUTER) expand it to nothing, like INCLUDE_ASM. */

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
