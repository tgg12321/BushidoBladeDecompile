
## Process improvements 2026-08-18 (owner-approved, "all except #6")

Five changes landed at the 2026-08-18 boundary: (1) agent deaths whose
agent.log terminal record is `api_error` 5xx/429 take the environmental
backoff-retry path regardless of duration — they no longer feed the
circuit breaker (the 3x192s-529 incident class); (2) the driver measures
the HEAD honest floor at dispatch and injects a CHASSIS CHECK block into
every brief (stale-ledger-floor class); (3) build_brief auto-sweeps
memory/**, decisions.md, borderline.md and .claude/rules for the
function name and attaches file:line hits (the libcd-twins
knowledge-silo class); (4) tools/grinder/dump.ps1 captures the TU's
cc1 -da pass dumps into tmp/grind/<func>/dumps/ and the brief contract
mandates reading them before pass-attribution hypotheses; (5) a
zero-yield permuter session gates all later permuter slots for that
function (first one still runs). Also: state.json
`owner_continue_until_session: N` suppresses the exhaustion trigger
until session N — the owner's continue-directive mechanism for reversing
a park without falsifying floor history. NOT implemented: #6
(stuck-escalation to the recon model) — owner deferred pending
metrics-side modality data.
