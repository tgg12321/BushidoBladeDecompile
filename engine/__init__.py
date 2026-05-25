"""BB2 Decomp Engine v2 — greenfield decompilation workflow.

Phase 0 surface: a clean, programmable build pipeline (replacing the Makefile
macro-soup) plus an immutable byte-match oracle. Later phases add the
cheat-invisible scoring sandbox, the two-layer verifier, and the deterministic
ladder controller.

Everything here assumes:
  * cwd == repo root (a worktree root is fine)
  * it runs inside WSL (the GNU/PsyQ toolchain is Linux-only)
"""

__all__ = ["buildconfig", "pipeline", "oracle"]
