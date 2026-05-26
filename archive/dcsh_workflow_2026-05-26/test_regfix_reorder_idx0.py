#!/usr/bin/env python3
"""
Test that regfix `reorder` involving idx 0 keeps the function-label
preamble (`func_NAME:`, `.ent`, `.frame`, `.mask`, `.fmask`) attached to
the line at the new idx-0 position rather than moving it with the
original idx-0 instruction.

Regression scenario: `reorder 1,0,2,3 @ 0-3` previously moved
`func_NAME:` along with the original first instruction, placing the
function symbol mid-body and breaking link addresses (caller jal
resolved to a wrong offset within the function).
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

import regfix


def make_input():
    return """\
.set noat
.set noreorder

.text

\t.globl\tfunc_TEST
\t.ent\tfunc_TEST
func_TEST:
\t.frame\t$sp,0,$31
\t.mask\t0x00000000,0
\t.fmask\t0x00000000,0
\tlw\t$t0, 0($a0)
\tlw\t$t1, 4($a0)
\tlw\t$t2, 8($a0)
\tjr\t$ra
\tnop
\t.end\tfunc_TEST
"""


def parse_lines_for_func(asm_text, func_name):
    """Mirror regfix's process_asm_text func-line collection."""
    lines = asm_text.splitlines(keepends=True)
    out = []
    insn_idx = 0
    in_func = False
    for line in lines:
        if line.strip().startswith(f"{func_name}:"):
            in_func = True
            out.append((line, None))
            continue
        if not in_func:
            continue
        if line.strip().startswith(f".end\t{func_name}") or line.strip().startswith(f".end {func_name}"):
            out.append((line, None))
            break
        # Use regfix.is_instruction-style detection: trimmed non-empty
        # lines starting with mnemonic chars and not '.', not labels, not
        # comments. Simpler heuristic: if the line starts with whitespace
        # then a letter and contains an opcode mnemonic separator.
        s = line.strip()
        is_insn = (
            s
            and not s.startswith('.')
            and not s.startswith('#')
            and not s.endswith(':')
            and not s.startswith('//')
        )
        if is_insn:
            out.append((line, insn_idx))
            insn_idx += 1
        else:
            out.append((line, None))
    return out


def find_first_instr_after_label(processed_lines, label):
    """Return the first instruction line text after the function label."""
    saw_label = False
    for text, idx in processed_lines:
        if not saw_label:
            if label in text and text.strip().startswith(label):
                saw_label = True
            continue
        if idx is not None:
            return text.strip()
    return None


def test_reorder_idx0_does_not_move_label():
    asm = make_input()

    config = {
        'swaps': [],
        'reorders': [(0, 2, [1, 0, 2])],  # swap idx 0 and idx 1
        'inserts': [],
        'insert_afters': [],
        'insert_labels': [],
        'substs': [],
        'deletes': [],
        'fill_delays': [],
        'drain_delays': [],
        '__name__': 'func_TEST',
    }

    parsed = parse_lines_for_func(asm, 'func_TEST')
    processed = regfix.process_function(parsed, config)

    # Find the function label position and the first instruction after it.
    first_insn = find_first_instr_after_label(processed, 'func_TEST:')
    assert first_insn is not None, "label not found in output"

    # After reorder 1,0,2 the new idx-0 instruction must be `lw $t1, 4($a0)`
    # (originally idx 1) — the label must NOT have moved with original idx 0.
    expected_insn = 'lw\t$t1, 4($a0)'
    assert first_insn == expected_insn, (
        f"label moved with idx 0! Expected first insn after func_TEST: to be "
        f"{expected_insn!r}, got {first_insn!r}"
    )
    print("PASS: reorder idx0 keeps label attached at idx 0 position")


def test_reorder_idx0_preamble_directives_preserved():
    """The .ent/.frame/.mask/.fmask directives must still appear before
    the first instruction (and before the function label they describe)."""
    asm = make_input()

    config = {
        'swaps': [],
        'reorders': [(0, 2, [2, 1, 0])],  # rotate
        'inserts': [],
        'insert_afters': [],
        'insert_labels': [],
        'substs': [],
        'deletes': [],
        'fill_delays': [],
        'drain_delays': [],
        '__name__': 'func_TEST',
    }

    parsed = parse_lines_for_func(asm, 'func_TEST')
    processed = regfix.process_function(parsed, config)

    out_text = ''.join(t for t, _ in processed)

    # Order: .globl, .ent, label:, .frame, .mask, .fmask, then first instr
    label_pos = out_text.index('func_TEST:')
    frame_pos = out_text.index('.frame')
    mask_pos = out_text.index('.mask')
    first_insn_pos = out_text.index('lw\t')

    assert frame_pos > label_pos, ".frame must come AFTER func_TEST:"
    assert mask_pos > label_pos, ".mask must come AFTER func_TEST:"
    assert first_insn_pos > mask_pos, "first instruction must come after .mask"

    # And the FIRST instruction text after the preamble must be from idx 2
    # (since we ordered 2,1,0).
    first_insn = find_first_instr_after_label(processed, 'func_TEST:')
    expected = 'lw\t$t2, 8($a0)'
    assert first_insn == expected, (
        f"reorder 2,1,0 should put original idx 2 first; got {first_insn!r}"
    )
    print("PASS: preamble (.frame/.mask/.fmask) stays attached to label")


def test_reorder_no_idx0_unchanged_behavior():
    """Sanity: a reorder that does NOT include idx 0 must still work the
    same as before (the fix is gated on reorder_start == 0)."""
    asm = make_input()

    config = {
        'swaps': [],
        'reorders': [(1, 2, [2, 1])],  # swap idx 1 and idx 2
        'inserts': [],
        'insert_afters': [],
        'insert_labels': [],
        'substs': [],
        'deletes': [],
        'fill_delays': [],
        'drain_delays': [],
        '__name__': 'func_TEST',
    }

    parsed = parse_lines_for_func(asm, 'func_TEST')
    processed = regfix.process_function(parsed, config)

    first_insn = find_first_instr_after_label(processed, 'func_TEST:')
    expected = 'lw\t$t0, 0($a0)'
    assert first_insn == expected, (
        f"non-idx0 reorder should leave idx 0 in place; got {first_insn!r}"
    )
    print("PASS: reorder not spanning idx 0 unchanged")


if __name__ == '__main__':
    test_reorder_idx0_does_not_move_label()
    test_reorder_idx0_preamble_directives_preserved()
    test_reorder_no_idx0_unchanged_behavior()
    print("\nAll tests passed.")
