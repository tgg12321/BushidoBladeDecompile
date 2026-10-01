"""Locate a C function definition's text span (start of the declarator line .. closing brace)."""
import re


def _span(s, func):
    ms = list(re.finditer(rf"^(?!extern)[A-Za-z_][\w \*]*\b{func}\s*\([^;{{]*\)\s*\{{", s, re.M))
    assert len(ms) == 1, (func, len(ms))
    i = ms[0].end()
    depth = 1
    while depth:
        depth += {"{": 1, "}": -1}.get(s[i], 0)
        i += 1
    return ms[0].start(), i
