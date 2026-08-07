/* s47 C3 KILLED: `s32 zero = 0;` fn-body-top + `D_800F19BC = zero;` at
   prologue store (line 408). Single use PRIOR to block=3, no cross-call
   preservation demanded.
   Result: masked=2 INERT, build_insns=160 unchanged.
   Mechanism: cse folds `zero` back to literal 0 at the single-use site
   before local-alloc; the local never acquires a distinct pseudo. Same
   outcome as C1 (DCE) but via a different pass.
   Confirms: no `s32 zero` shape reaches local-alloc as a live pseudo
   without spanning a callee-clobbering call (which triggers C2's +3 insn
   regression). The named-local FAKE mechanism has no middle ground on
   csmd4's h5 base. */
s32 zero = 0;
/* ... D_800F19BC = zero; at line 408 ... */
