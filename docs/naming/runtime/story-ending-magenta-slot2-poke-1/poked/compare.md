# Set-and-observe: poked against baseline

- scenario: Story Mode, story-ending-poke script (fighter 0x1C poked in as the opponent, P1 KANNUKI CROSS every 20 from 9800) cut at 12800 (two magenta-light starts, 12650 and 12745); poked run: eff_StartRecycledCommandWithMagentaLight 0x80061EF0 poked to b 0x80061F4C / addiu a0, v1, 1 at intro so every start takes the second slot (status 19, command 0x13)
- baseline: `..\baseline\capture.manifest.json` (SHA1 49b8c90a25ba4ace5cb7d996e9aa5c6a303f7d21), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 026eee767791e711f2ad6c12e3399539beb137f6), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes in both runs (baseline's too, not compared): `0x8008F222` (0x8008F222) = 111c0000120301ff131624008300 (was 100710033324121300190a01010f) from phase top_menu (frame 6060), 111c0000120301ff131624008300 at the end
- pokes: `0x80061EF0` (0x80061EF0) = 1600001001006424 (was 0b00401000000000) from phase intro (frame 8600), 1600001001006424 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| story_text.first (frame 6810) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7360) | 0 of 153600 (0.00%) | - |
| stage_card.8200 (frame 8200) | 0 of 153600 (0.00%) | - |
| intro.8800 (frame 8800) | 0 of 153600 (0.00%) | - |
| intro.9400 (frame 9400) | 0 of 153600 (0.00%) | - |
| fight.10000 (frame 10000) | 0 of 153600 (0.00%) | - |
| fight.10600 (frame 10600) | 0 of 153600 (0.00%) | - |
| fight.11200 (frame 11200) | 0 of 153600 (0.00%) | - |
| fight.11800 (frame 11800) | 0 of 153600 (0.00%) | - |
| fight.12400 (frame 12400) | 0 of 153600 (0.00%) | - |
| fight.12653 (frame 12653) | 0 of 153600 (0.00%) | - |
| fight.12657 (frame 12657) | 0 of 153600 (0.00%) | - |
| fight.12661 (frame 12661) | 0 of 153600 (0.00%) | - |
| fight.12665 (frame 12665) | 0 of 153600 (0.00%) | - |
| fight.12669 (frame 12669) | 0 of 153600 (0.00%) | - |
| fight.12673 (frame 12673) | 0 of 153600 (0.00%) | - |
| fight.12748 (frame 12748) | 0 of 153600 (0.00%) | - |
| fight.12752 (frame 12752) | 0 of 153600 (0.00%) | - |
| fight.12757 (frame 12757) | 0 of 153600 (0.00%) | - |
| fight.12763 (frame 12763) | 0 of 153600 (0.00%) | - |
| fight.12767 (frame 12767) | 0 of 153600 (0.00%) | - |
| fight.12771 (frame 12771) | 0 of 153600 (0.00%) | - |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
