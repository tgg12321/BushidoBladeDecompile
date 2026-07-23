/* REJECTED: score unchanged (36). split-init `s0 = arg0; s0 *= 1100;` does
   NOT unify the web — the final `sll s0,v0,2` already writes s0 directly, and
   `s0 *= 1100` still expands the mult into a temp then copies to the variable
   pseudo, so the cse.c canon_reg copy (move s2,s0) survives. Web unification
   requires RECOMPUTE-INLINE of arg0*1100 (no s0 variable at all), not split-
   init. Decl-order and statement-reorder likewise inert (qty birth follows
   RTL first-use; scheduler reorders regardless of source). */
