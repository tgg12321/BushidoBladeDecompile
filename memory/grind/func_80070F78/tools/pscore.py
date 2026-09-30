import sys
sys.path.insert(0, 'tools/decomp-permuter')
from src.scorer import Scorer
tgt = sys.argv[1]
for o in sys.argv[2:]:
    s = Scorer(tgt, stack_differences=True, algorithm="levenshtein", debug_mode=False, ign_branch_targets=False, objdump_command=None)
    print(o, s.score(o)[0])
