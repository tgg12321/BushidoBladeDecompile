/* REJECTED — s12: the debug_printf PROTOTYPE axis is byte-inert.
 * HEAD declares `extern void debug_printf(void *, void *, s32, s32, s32);`.
 * Re-declaring it `(void *, ...)` (a real printf-style varargs prototype, which
 * is almost certainly the original Sony signature) and `()` (K&R, no prototype)
 * were both measured against three bodies: the candidate (7), arg4-inline (13)
 * and arg5-named+arg4-inline (13). Every one is byte-identical to the fixed
 * prototype, so expand_call's stdarg/promotion path contributes nothing to the
 * argument block here and the axis is dead. (Contrast the
 * fake-varargs-explicit-homing technique, which needs the opposite symptom:
 * bulk pre-subu arg homes. This function has none.)
 *
 * The measured spelling, for the record:
 *     extern void debug_printf(void *, ...);
 * with the candidate body unchanged.
 */
