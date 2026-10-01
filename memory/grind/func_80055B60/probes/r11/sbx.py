# sandbox func_80055B60 with fix1's include/game.h (tmp copy) ahead of include/. Never touches src/ or include/.
import sys
sys.path.insert(0, '.')
from engine import buildconfig as cfg
cfg.CPP_FLAGS = '-Itmp/b60/inc/include ' + cfg.CPP_FLAGS
from engine import cli
sys.argv = ['engine.cli', 'sandbox', 'func_80055B60', '--disable', 'all', '--diff', '--candidate', sys.argv[1]]
sys.exit(cli.main())
