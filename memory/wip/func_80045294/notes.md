## func_80045294 — WIP

### TL;DR
HEAD's pure-C floor at distance 2 (one extra `nop` in the first loop's load-delay slot).
3 regfix rules (`delete @21; delete @25; insert "addu $16,$16,1" @24`) move
the `i++` from BEFORE the loop's `lw` to BETWEEN `lw` and `addu sum +=`,
filling the load-delay slot. The lever to make cc1 emit this scheduling
naturally has not been found.

### The exact gap
Target first loop body:
```
lui at; addu at,at,v1     # expand-at
lw v0,0(at)
addiu s0,s0,1             # i++ FILLS LOAD DELAY
addu s1,s1,v0             # sum += v0
slt v0,s0,a0
bnez v0,.loop
addiu v1,v1,16            # delay slot
```

cc1's natural emission (HEAD source):
```
addu $16,$16,1            # i++ at TOP of loop body — wrong position
lui $at; addu $at,$at,$3
lw $2,...
nop                       # maspsx-added load-delay nop
addu $17,$17,$2           # sum += v0
slt $2,$16,$4
bne $2,$0,.loop
addu $3,$3,16             # delay slot
```

cc1's sched.c places `addiu $16,$16,1` first; need it between `lw` and `addu $17,$17,$2`.

### Source forms tried (all → sandbox 2)
1. `do { sum += load; i+=1; v1+=0x10; } while(i<count);`  — HEAD form.
2. `do { s32 val = load; i+=1; sum+=val; v1+=0x10; } while(i<count);`  — split load into temp, source-reorder i++ between.
3. `while (i < count) { sum+=load; i+=1; v1+=0x10; }`  — while form.
4. `for (; i < count; i++, v1+=0x10) { sum += load; }`  — for-loop with bottom-update.

Each compiled to the same scheduling — cc1 puts `addu $16,$16,1` at the loop top regardless.

### Why it's hard
cc1's MIPS sched.c picks `addiu i++` over `lw` based on `INSN_PRIORITY` + LUID
tiebreaks. The two instructions' forward chains within the BB:
- `lw` → `addu sum` (1 hop within BB, then dead at backedge)
- `addiu i++` → `slt` → `bnez` (2 hops within BB, control reaches backedge)

The longer in-BB chain for `addiu` likely wins the priority comparison even though
`lw` has higher latency. Source reordering doesn't help because RTL emit normalizes
order; the scheduler picks independently of source statement order.

### Resume hypotheses (untried)
1. Make the loaded value flow into a DIFFERENT in-BB consumer that lengthens
   `lw`'s forward chain (e.g. an extra `addu accum, accum, val` that has no
   sibling — but that emits extra bytes).
2. Restructure to use pointer iteration (`ptr += 0x10`) instead of byte-offset
   `v1 += 0x10`; pointer base in a callee-save with addressed by `0($ptr)` may
   make cc1's RTL produce a different `lw` insn shape with different latency
   modeling. The target uses `D_800EED18($3)` (symbol + reg) addressing, so the
   match form must keep that — a pointer recast in C may still emit the same
   addressing if cc1 inlines via combine.
3. Probe via instrumented cc1 (`BB2_SCHED_DEBUG` per
   [[register-alloc-pure-c]] session 6 recipe) to see actual INSN_PRIORITY
   values for `lw` vs `addiu` in this loop — direct evidence of the priority
   gap to target.
4. Look at matched sibling loops in text1a*.c with similar shape
   (`*ptr += val; i++; ptr++;` in `func_80044100`/etc.) and diff their RTL —
   may surface what makes their `lw` get higher priority.

### Status
Sandbox `--disable all --keep-cheat-asm` = **2**. No lever found this session.
Source untouched (oracle stays green). Function stays INCOMPLETE — parking
with this evidence so the next session can pick up at the instrumented-cc1
probe (avenue 3) rather than re-deriving the form-sweep negative.
