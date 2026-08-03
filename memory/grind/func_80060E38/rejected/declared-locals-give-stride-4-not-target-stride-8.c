/* REJECTED (grind session 2, structural) — hold the nine values in declared locals.
 *
 * IDEA: an ordinary declared local takes assign_stack_local's align == 0 path
 * (alignment = GET_MODE_ALIGNMENT(SImode)/8 = 4, size NOT rounded up), so its
 * bigend_correction is 4 - 4 = 0 and it lands at +0 instead of +4 — apparently the
 * congruence target needs.
 *
 * MEASURED RESULT: offsets 16,20,24,28,32,36,40,44,48 — stride 4, not stride 8.
 * The same align == 0 path that drops the +4 also drops the size rounding, so a block
 * of SImode locals is contiguous. Target has nine slots at stride 8, ALL congruent to
 * 0 — the signature of the align == -1 reload-spill path, which is precisely the path
 * that carries the +4. For a 4-byte value the two properties are mutually exclusive
 * under this cc1. Getting stride 8 out of locals would need 8-byte-aligned local
 * objects (long long / double), i.e. the forbidden dead-vars-local-array frame-
 * coercion family, and would change the stores from single sw to two-word accesses.
 */
/* rederive probe: could the 9 stack slots be DECLARED LOCALS rather than reload
   spills?  A declared local takes assign_stack_local's align == 0 path (alignment
   = GET_MODE_ALIGNMENT(SImode)/8 = 4, size NOT rounded up, bigend_correction =
   4 - 4 = 0), so its offsets should be stride 4 and congruent to 0 mod 8 on
   alternate slots.  Target needs nine slots at stride 8, ALL congruent to 0.
   This probe measures the stride a forced-to-memory local block actually gets. */
typedef int s32;

extern s32 D_800A3468, D_800A346C, D_800A3470, D_800A3474, D_800A3480;
extern s32 D_800A3484, D_800A3488, D_800A348C, D_800A3490, D_800A3494;
extern void sink(s32 *);

void locals_probe(s32 arg0) {
    s32 a, b, c, d, e, f, g, h, i;
    a = 0x1F800000; b = 0x1F800018; c = 0x1F800020;
    d = 0x1F800030; e = 0x1F800050; f = 0x1F800058;
    g = 0x1F80005C; h = 0x1F800060; i = 0x1F800062;
    sink(&a); sink(&b); sink(&c); sink(&d); sink(&e);
    sink(&f); sink(&g); sink(&h); sink(&i);
    D_800A3468 = a; D_800A346C = b; D_800A3470 = c; D_800A3474 = d;
    D_800A3480 = e; D_800A3484 = f; D_800A3488 = g; D_800A348C = h;
    D_800A3490 = i; D_800A3494 = arg0;
}
