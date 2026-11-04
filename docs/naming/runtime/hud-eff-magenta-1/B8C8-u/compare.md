# Set-and-observe: B8C8-u against baseline

- scenario: Story Mode, story-ending-magenta-slot2-poke script (story-ending-poke: fighter 0x1C poked in as the opponent, P1 KANNUKI CROSS every 20 from 9800, cut at 12800; two magenta-light starts, 12650 and 12745), screenshots over both starts; breakpoint eff_DrawAnimatedQuads, read watches on 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7; poked: D_8009B8C8.u (0x8009B8CC) 0x00 -> 0x80 at fight
- baseline: `..\baseline\capture.manifest.json` (SHA1 2973daee2edb3dd52cfabe64986d3c856e9ada09), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 9b47d54daeae37c856a5b9a1ee348b8f718ec2cb), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes in both runs (baseline's too, not compared): `0x8008F222` (0x8008F222) = 111c0000120301ff131624008300 (was 100710033324121300190a01010f) from phase top_menu (frame 6060), 111c0000120301ff131624008300 at the end
- pokes: `0x8009B8CC` (0x8009B8CC) = 80 (was 00) from phase fight (frame 9800), 80 at the end
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
| fight.12661 (frame 12661) | 271 of 153600 (0.18%) | 356,94-423,104 |
| fight.12665 (frame 12665) | 1766 of 153600 (1.15%) | 343,72-418,125 |
| fight.12669 (frame 12669) | 3452 of 153600 (2.25%) | 336,61-457,126 |
| fight.12673 (frame 12673) | 2985 of 153600 (1.94%) | 319,70-468,135 |
| fight.12748 (frame 12748) | 0 of 153600 (0.00%) | - |
| fight.12752 (frame 12752) | 0 of 153600 (0.00%) | - |
| fight.12757 (frame 12757) | 91 of 153600 (0.06%) | 361,92-400,97 |
| fight.12763 (frame 12763) | 693 of 153600 (0.45%) | 354,77-400,110 |
| fight.12767 (frame 12767) | 1619 of 153600 (1.05%) | 347,68-429,112 |
| fight.12771 (frame 12771) | 1691 of 153600 (1.10%) | 332,71-442,120 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
