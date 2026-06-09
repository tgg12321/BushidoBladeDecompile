/* 4-byte rodata padding sub-TU split out for the 101C.rodata_post cluster
 * (rodata-cleanup project, docs/rodata-cleanup-project.md, 2026-06-09).
 *
 * The original asm/data block contained exactly one `.word 0x00000000`
 * (4 bytes of zero) at address 0x80010D88, between code6cac_c_ab.o(.rodata)
 * and code6cac_c2.o(.rodata). It has no symbolic references anywhere in the
 * codebase (verified via grep across src/, asm/funcs/, asm/data/, rule
 * files) — it appears to be either splat alignment padding extracted as a
 * separate block, or genuinely unused data preserved by the original
 * compiler.
 *
 * Empirical test (commit f705d02e, 2026-06-09) confirmed removing it
 * without compensation breaks the oracle by a 4-byte cascade affecting
 * 200+ downstream symbols. This sub-TU emits the same 4 zero bytes via
 * a file-scope `__asm__` block (mirroring the asm/data file's directive
 * exactly), placed at the same bb2.ld slot to preserve all downstream
 * addresses.
 *
 * Named after the upstream segment (code6cac_c_ab) since the bytes
 * functionally extend that file's rodata into the next 4-byte slot. */
__asm__(
    ".section .rodata\n"
    "    .word 0x00000000\n"
);
