# Codex Lab

This directory is an isolated workspace for Codex-led experiments that should not
interrupt the main decomp workflow or Claude's existing scratch files.

Rules for this lab:
- Do not edit the live `permuter/<func>` workspace unless we explicitly decide to
  promote a result back into the main flow.
- Keep strategy notes and experiment logs here.
- Prefer semantically clean experiments over pure score chasing.
- Treat `regfix` as a valid late-stage tool when the C is structurally correct and
  the remaining differences are allocator or scheduler artifacts.

Current active target:
- `tslPrintScreen`

Promotion status:
- the `tslPrintScreen` lab result has been promoted into the live build
- the live repo now matches with a two-stage `regfix` path plus the
  decompiled `src/config.c` body

Related notes:
- [Late Stage Match Strategy](C:/Users/Trenton/Desktop/Bushido%20Blade%202%20Decompile/codex_lab/late_stage_match_strategy.md)
- [tslPrintScreen Lab Notes](C:/Users/Trenton/Desktop/Bushido%20Blade%202%20Decompile/codex_lab/tslPrintScreen/README.md)
