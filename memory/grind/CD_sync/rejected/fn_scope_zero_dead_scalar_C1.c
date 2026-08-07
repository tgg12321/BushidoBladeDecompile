/* s47 C1 KILLED: `s32 zero = 0;` fn-body-top dead-unread scalar, FAKE-annotated
   per named-local-fake-exception.md SOTN archetype.
   Result: masked=2 INERT, build_insns=160 unchanged.
   Mechanism: tree-level DCE + cse fold the literal init and delete the dead
   local before local-alloc even sees it. The reload-renumbering hypothesis
   (F2 primary) cannot fire on a DCE'd local.
   Site: line 389 (before decl block). */
s32 zero = 0; /* FAKE: SOTN archetype constant-holder; mechanism: shift local-alloc qty birth-luid; DCE'd pre-alloc, INERT */
