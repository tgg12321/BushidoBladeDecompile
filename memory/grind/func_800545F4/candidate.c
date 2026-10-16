/* candidate.c -- func_800545F4 (src/main/3AB48.c)
 *
 * No improved form exists as of s3. This entry is ASM-WHOLE
 * (inline_asm_canonical.txt:112), 4 instructions, no_c_body: true, floor 4.
 *
 * H1 (plain C global-data declaration replacing the whole-body INCLUDE_ASM)
 * is KILLED at both class scope (s2: bb2.ld groups all .text blocks before
 * all .data blocks per TU -- bb2.ld:116 vs bb2.ld:343 -- so no same-TU plain
 * C global can land mid-.text) and instance scope (s3: actually spelling it
 * out and measuring gave `scorable: false`, symbol type `D` at .data offset
 * 0x0, confirmed via `nm`; see memory/grind/func_800545F4/rejected/
 * plain_c_data_global.c).
 *
 * The only live disposition is H2: reaffirm the existing ASM-WHOLE grant.
 * That requires `escalation` modality to file (standing auto-ruling); this
 * session's modality was `permuter`. Current body stays:
 *
 *   INCLUDE_ASM("asm/funcs", func_800545F4);
 *
 * at src/main/3AB48.c:657, unchanged.
 */
