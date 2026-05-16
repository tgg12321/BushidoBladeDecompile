# CPU / AI Behavior

BB2's AI runs the same fighter pipeline as the player — it just provides the
move-command input that a human would have entered via the pad. AI logic
lives almost entirely in `code6cac_b.c` under the `cpu_*` function prefix.

The naming "CPU" here means "the AI opponent" (as in "P1 vs CPU"), not the
MIPS R3000 itself. The codebase is consistent: anything `cpu_*` is AI move
decision code.

## What "AI" means in BB2

BB2 is a one-strike-kill fighting game. There are no combo trees in the
modern sense — each "move" is a discrete attack with weapon hit-volume,
direction, and timing. The AI's job is to:

1. Decide what stance to be in (high, middle, low — `D_800A36A4` selects
   stance / mode group).
2. Decide what direction to face the opponent (4-way: forward / back /
   side-left / side-right).
3. Decide when to commit to an attack.
4. Commit a move from the available waza pool given the current stance,
   character, and tactical situation.
5. While the move animates, react if the opponent attacks first
   (block, sidestep, or eat the hit).

The same fighter struct and move-execution pipeline ([combat.md](combat.md))
runs whether the input came from a pad or the AI. The AI feeds the move
queue at offset `0x330..0x33B` of the fighter struct.

## Key entry points

### `cpu_set_move_command_and_dir` (`code6cac_b.c:2540`)

The "commit a chosen move" function — see [combat.md](combat.md). The AI
calls this with `(fighter, move_id, vec3 velocity)` to:

- Allocate a slot in `D_80106A78[12]` for the new move.
- Set up initial velocity from `arg2`.
- Randomize cosmetic parameters (blood spray angle, spark direction) from
  `rng_Next()`.
- Set the slot's direction byte (offset 0xB) from the fighter's facing.

This is the boundary between "AI decision" and "engine execution". Once
committed, the move runs to completion under the per-frame pipeline.

### `cpu_set_move_command_and_dir_for_no_action` (`code6cac_b.c:3510`)

"No action" = AI is between moves. Builds a shuffled list of which moves
are currently legal (from a mask in `D_80106A50` against waza-enable bits in
`D_8008D538[stance]`):

1. Walks bits 0..26 of `D_80106A50` masked against the stance's enable
   bitmap; for each enabled bit, appends the move ID to a buffer at
   `D_801077B0`.
2. Shuffles the buffer 108 times (`func_80079154() % count` is the RNG —
   `func_80079154` is a pseudo-RNG that returns a 31-bit value).
3. Applies special-case overrides — if both moves 5 and 0x10 (`0x20` and
   `0x10000` bits) are enabled and the AI is in the right stance, prefer
   one over the other based on another coin-flip.
4. Stores the chosen tactical direction in `D_801077BA`.
5. Sets `D_800A3834 = 4` (gameplay-execute mode).

This is the AI's "what should I do next" decision. It's intentionally
randomized (no learning, no state machine — each "no-action" frame picks
fresh from the legal moves).

### `cpu_set_move_command_and_dir_for_no_action_2` (`ings.c:584`)

DIFFERENT FUNCTION — same name due to Kengo collision. This is the main
loop entry point, NOT AI code. See [main_loop.md](main_loop.md).

### `cpu_get_dist` (`code6cac_b.c:2287`)

Per-frame "look at the opponent" calculation. Given the AI fighter `a0` and
the opponent's reference position `a1`, rotates `a0`'s velocity vector into
the opponent-relative frame:

1. Reads angle = `single_game_getEnemyCharId(a1[0], a1[2])` — this is the
   atan2 helper that returns the 12-bit direction from `a0` to `a1`.
2. Reads `sin_val = Judge[angle & 0xFFF]`, `cos_val = Judge[(angle+0x400) & 0xFFF]`
   — sine/cosine LUT lookup.
3. Multiplies the 3D velocity at fighter offset 0x44/0x48/0x4C by the rotation
   matrix and stores back at the same offsets.

Effect: the AI fighter's velocity is now expressed in "facing-the-opponent"
local coordinates, where +X is "toward opponent" and +Z is "their right".
This is what the AI's tactical comparisons against `D_8008E194` waza-tables
are evaluated in.

### `cpu_get_dist_2` (`code6cac_b.c:2975`)

Per-frame bytecode interpreter that decodes the AI's command stream — see
[combat.md](combat.md) for the opcode table.

### `cpu_check_same_dir_timer` (`code6cac_b.c:3262`)

Per-frame "should I change my mind about this move" check — see
[combat.md](combat.md).

### `cpu_check_tubazeri` / `cpu_check_tubazeri_2` (`code6cac_b.c:1907, 2461`)

Sword-clash detection — see [combat.md](combat.md).

### `cpu_check_run_attack` (`code6cac.c:332`)

After `marionation_camera_Exec` updates the camera, this function tests
whether the AI fighter should commit to a charging "run attack". It walks
the data at fighter offset 0xC/0x10 (the move-set pointers) and reads a
list of (radius, angle, threshold) triples from `0x1F8000BC` in the
scratchpad to decide. Uses the GTE scratchpad set up by
`marionation_camera_Exec`.

### `cpu_side_move_dir` (`code6cac_b.c:311`)

Handles the "side-move" tactical retreat or sidestep. Reads the fighter's
side-move cooldown counter (offset 0x34C, capped at 0x40), computes a target
position from the stage-bound table `stage_GetDataPtr()`, then advances the
fighter position toward it. Updates eight body/shadow position offsets
(0xF4/0xFC + 0xD8/0xE0 + 0xB8/0xC0 + 0x104..0x13C).

The side-move target depends on `D_800A36A4` (stance):
- Stance 3: hardcoded `(0x2EE0, 0x1770)` (or sign-flipped based on current
  position).
- Other stances: read from `stage_ptr + (D_800A36A4*0xC + temp_s2*3) * 2`
  — per-stance, per-fighter-index lookup.

### `cpu_check_move_dir_pattern_enemy_attack` (`code6cac.c:1293`)

"Set up a fixed camera shot of the enemy's attack" — called from the
practice/replay modes. Copies fighter offset 0x174 (the head bone position)
to `D_800F6608` (the camera target) and sets the camera distance/rotation
parameters at `D_800F661A..D_800F6620` to a fixed `(0x120, arg1, 0, 0x1162)`.

This is the "watch the enemy do their move" replay-camera helper, not an
attack decision.

### `cpu_get_move_pattern_table_number` (`code6cac.c:1816`)

The per-frame "practice mode" handler — runs the move-pattern table
selection for the practice/tutorial mode. Increments `D_800A37B8` (a frame
counter), calls `cpu_check_move_dir_pattern_enemy_attack` to set up the
camera, then waits for pad input. The `D_800A3817` register holds the
"current selection" within the pattern table; pad-direction events cycle
through it.

This function is also where many of the `D_800A3834` transitions out of
practice mode are decided — selections 0/1/2 each set a different next mode.

## Move-selection logic — `func_8003047C` (`code6cac_b.c:2326`)

Despite its `func_` name, this is "load the waza queue for the current
stance". For fighter `a0`:

1. Zeroes the move-queue header at `a0[0x330]`.
2. Walks 5 entries from `D_8008E338 + a0[0xA] * 5` (5-byte waza lists per
   character).
3. For each entry, writes it into `a0[0x332 + i*2]` and increments the queue
   count if the byte != -1.
4. Sets the head waza ID (`*(&D_800A36F2 + a0[4])` = `D_8008E338[a0[0xA] * 5]`)
   — this is the move that's about to be committed.

`D_8008E338` is therefore a per-character, per-stance lookup table of 5
waza IDs. The character's "personality" lives in this table — what attacks
they tend to do in each stance.

## AI difficulty

The codebase does not (yet) expose an explicit "difficulty" knob. Several
plausible mechanisms are visible:

- `D_800A36F2 ^ 0xE` (`code6cac_b.c:2642`) is XORed against `0xE` for
  difficulty-modifier behavior in `func_80030BA8` (the queue advancer).
- The RNG seeded from `g_file_heap_base` (set by `rng_SetSeed`,
  `ings.c:574`) is used everywhere — different RNG sequences produce
  different AI behavior.
- The `func_80079154() % count` shuffling in
  `cpu_set_move_command_and_dir_for_no_action` is the primary "AI randomness"
  source.
- Per-character `D_8008E338`-style tables and a stance-mask
  `D_8008D538[stance]` determine which moves are even legal at any time.
- `D_8010277E..D_80102787` (initialized in `func_8001C444`, `code6cac.c:1337`)
  are small tactical AI flags — probably "be aggressive", "be defensive",
  etc.

True difficulty scaling — if BB2 has it — would probably be expressed by
varying these constants per game mode. Tracing the actual difficulty knob
is a research item.

## Trash AI fields (cleanup helpers)

When an AI fighter is removed from play (e.g., dies, leaves screen), several
functions blank fields:

- `func_8003043C` (`code6cac_b.c:2314`) — clears the 12-slot `D_80106A78`
  active-move array.
- `func_80030524` (`code6cac_b.c:2362`) — selectively clears slots that have
  their "consumed" flag set.
- `func_80030D04` (`code6cac_b.c:2658`) — selectively retires move-IDs in
  range 0x12..0x1D from the queue.

These run during scene transitions to keep the AI from carrying stale state
into a new round.

## Where unmatched AI asm still hides

Many `cpu_*` and `func_8002XXXX` helpers in `code6cac_b.c` and `text1b.c`
are still asm-only. The 1410-function `asm/funcs/` list includes `cpu_*`
files such as:

- `cpu_check_run_attack.s` — wait, this is in C now; sample listed in
  `asm/funcs/` may be stale.

Run `bash tools/dc.sh next` against the project's WORK_QUEUE.md to see
what's next. CPU/AI functions in the queue are tagged with their blocker
class.
