"""Debug-only instrumentation for a PRIVATE copy of cc1 in /tmp/gccdbg (never the build cc1):
log 3->2 combinations and orphan USE placement to stderr when BB2_COMB_DEBUG is set."""
p = '/tmp/gccdbg/combine.c'
s = open(p).read()
anchor = '    /* Distribute all the LOG_LINKS and REG_NOTES from I1, I2, and I3.  */\n'
assert anchor in s
dbg = anchor + '''    if (getenv ("BB2_COMB_DEBUG"))
      {
        extern char *current_function_name;
        fprintf (stderr, "COMBDBG %s i1=%d i2=%d i3=%d newi2=%d elim_i2=%d elim_i1=%d\\n",
                 current_function_name, i1 ? INSN_UID (i1) : 0, INSN_UID (i2), INSN_UID (i3),
                 newi2pat != 0, elim_i2 ? REGNO (elim_i2) : -1, elim_i1 ? REGNO (elim_i1) : -1);
        if (newi2pat) { fprintf (stderr, "  newi2: "); debug_rtx (newi2pat); fprintf (stderr, "\\n"); }
        fprintf (stderr, "  newpat: "); debug_rtx (newpat); fprintf (stderr, "\\n");
      }
'''
s = s.replace(anchor, dbg, 1)
anchor2 = '''		  place
		    = emit_insn_after (gen_rtx (USE, VOIDmode, XEXP (note, 0)),
				       tem);
'''
assert anchor2 in s
s = s.replace(anchor2, anchor2 + '''		  if (getenv ("BB2_COMB_DEBUG"))
		    fprintf (stderr, "ORPHAN reg %d after uid %d (from_insn %d, i3 %d)\\n",
			     REGNO (XEXP (note, 0)), INSN_UID (tem),
			     from_insn ? INSN_UID (from_insn) : 0, INSN_UID (i3));
''', 1)
if 'extern char *getenv' not in s[:3000]:
    s = s.replace('#include "config.h"', '#include "config.h"\nextern char *getenv ();', 1)
open(p, 'w').write(s)
print('patched')
