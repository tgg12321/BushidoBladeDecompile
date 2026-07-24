/* REJECTED (s2, structural) — every legitimate small-local shape for the
 * residual empty 8-byte frame is DEAD. Add ONE of these local decls to the
 * candidate.c body to reserve target's `subu sp,-8 ... addu sp,8` zero-store
 * frame; each is either a forbidden dead-vars cheat or emits stores target
 * lacks. Measured via tmp/grind/func_80017FA0/s2/frameprobe2.{c,s}
 * (cc1 -O2 -G0 -funsigned-char -mcpu=3000). Do NOT re-propose any of these.
 *
 *   struct { s32 a, b; } pp;              // g1: vars=8, ZERO stores -> reproduces
 *                                         //     target EXACTLY but pp is unused =
 *                                         //     FORBIDDEN dead-vars aggregate.
 *
 *   union { s32 w[2]; s32 v; } uu;        // g2: vars=8, ZERO stores -> same
 *                                         //     forbidden dead aggregate.
 *
 *   s32 x, y;  ... (&x == &y) ? 0 : ...   // g3: vars=8, ZERO stores. x,y are
 *                                         //     semantically dead; the address
 *                                         //     compare folds to 0 at compile
 *                                         //     time. Contrived dead-var frame
 *                                         //     coercion = FORBIDDEN.
 *
 *   struct { s32 a, b; } pp;              // g4: GENUINE use (read once) ->
 *   pp = *(struct pair *)ptr;             //     vars=8 but emits sw $2,0($sp);
 *   ... pp.a ... pp.b ...                 //     sw $3,4($sp) + reloads = stores
 *                                         //     the target LACKS. Wrong.
 *
 * The dichotomy is complete and closes the structural axis for the frame:
 *   - zero-store vars=8  <=>  the local is DEAD  <=>  forbidden dead-vars-local-array
 *                             (WRITTEN carve-out inapplicable: target has 0 frame stores)
 *   - genuine use of a >=5-byte local  =>  emits frame stores target lacks
 * There is no legitimate C shape that produces target's zero-store 8-byte frame.
 * -> endgame-lock-disposition; escalation-modality OWNER-ESCALATION is the next step.
 */
