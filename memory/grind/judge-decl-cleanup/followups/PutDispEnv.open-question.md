# Lane X3 open items (no patch)
## PutDispEnv (display.c:265-299): owner question, SOTN condition (2)
Eight `*(volatile s16 *)&g_gpu_ctx.disp_env.{screen,disp}.{x,y,w,h}` reads, FAKE-labelled, citing
SOTN src/main/psxsdk/libspu/s_m_m.c:48 @aa53500 (identical at db41b28). Checked:
- SOTN's own PutDispEnv (src/main/psxsdk/libgpu/sys.c:336-402, 3.3 rev, matched) has NO
  volatile: it compares x/y and w/h as two LOW() words (sys.c:351-352, 367-369). BB2's 4.0 rev
  compares four halfwords, and the target loads the saved side as `lhu; sll 16; sra 16`
  (asm/funcs/PutDispEnv.s:57-60), the env side `lh`. Only a volatile HImode read stops combine
  from folding that into `lh`.
- The cited s_m_m.c:48 construct is real, matched, PS1 (conditions 1, 4 hold), but it sits in a
  different library and function and has a different codegen effect (forced reload, not a kept
  zero-extend). So the citation is by analogy, not "the same thing" at this statement.
- No FAKE-free spelling found: a `(s16)*(u16 *)&field` view is still a codegen-only cast (no fewer
  FAKEs); a volatile libgpu context (as Sony declares some libgpu internals, sys.c:58) would retype
  g_gpu_ctx for every libgpu function.
Question: does sotn-precedent-suffices condition (2) admit a construct citation from another
function (same spelling, same RAM class), or must the cited site be the corresponding statement?
If the latter, PutDispEnv has no admissible route for these 8 casts.
## g_vab_rec_ptr[] / g_vab_vb_sbaddr[] element handles (text1b_tu1b.c): deferred
Lane X1's 05-sndpool.patch edits the same declarations/bodies (func_8005B644, snd_VabFakeOpen
area), and the coordinator narrowed this lane to main.c/display.c. Not started, to avoid a
conflicting patch; open after X1 lands (or fold into X1).
## CD_getsector2 (system.c): dropped per coordinator (orchestrator owns system.c).
