# Completion integrity repair — 2026-09-19

This repair closes six confirmed defects found in the completed-function audit.
The acceptance standard remains byte-identical output from genuine C, with
assembly reserved for original or otherwise canonical assembly.

1. **A stale executable could certify changed source.** Full builds now write a
   content-bound receipt for every source, assembly, configuration, toolchain,
   and output artifact. Oracle verification and completion checks reject a
   missing or stale receipt.
2. **`make check` returned success after a checksum mismatch.** The checksum
   recipe now propagates failure, and `clean-check` runs clean and check in
   sequence rather than as unordered prerequisites.
3. **Compiler and pipeline failures were ignored.** Bash pipe failure handling
   is enabled in both Make and the engine. Failed recipes remove partial
   targets. Redundant and conflicting declarations that previously made cc1
   fail while still emitting assembly were corrected; every translation unit
   now completes cc1 successfully.
4. **Five mixed C/GTE functions were counted as pure C.** `ReadGeomScreen`,
   `SetBackColor`, `SetFarColor`, `SetGeomOffset`, and `SetGeomScreen` are now
   classified as mixed canonical assembly. Their exact assembly islands are
   reviewed independently from the surrounding C.
5. **Seventeen GTE wrappers used misleading fixed-register `.word` blocks.**
   Twelve full original LIBGTE routines now use their canonical assembly bodies
   through `INCLUDE_ASM`. The five mixed functions use operand-aware `cfc2` and
   `ctc2` mnemonics, so compiler-selected operands agree with the encoded
   instruction.
6. **Canonical authorization exempted an entire mixed function.** A canonical
   C body now needs an exact list of reviewed assembly-region hashes. The hash
   covers opcode text, operands, constraints, and clobbers. All C outside those
   regions continues through the ordinary cheat checks, and any region change
   reopens the function.

Verification at repair time:

- clean rebuild SHA1: `62efab4f73f992798c43e8c730aa43baa10bb4fa`
- engine regression suite: 411 passed
- completion integrity: 1,158 completed C, 201 canonical assembly, 117 queued;
  no violations
- direct cc1 error audit: no failing translation units
