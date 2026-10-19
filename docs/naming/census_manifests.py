"""Read the census's verified naming manifests (docs/naming/build_census.py).

Split out of build_census.py so the precedence rule is testable: MANIFESTS is in
application order (the curated sweeps, then docs/naming/phase3/wave*/ in name order), and
for one address the LATEST CONFIRMed row wins. A reset-contradicted / basis-withdrawn RESET
recorded after a KEEP / RENAME row for the same address retires it, and a KEEP / RENAME
after a RESET supersedes the RESET.
"""
import csv
import os
import re


def read_manifests(manifests, class_tier):
    """manifests: [(path, default origin, default tier)] in application order.
    Returns (apiscan, manifest_resets), both keyed by the upper-case address without 0x:
    apiscan[a] = dict(name, evidence, note, origin, tier); manifest_resets[a] =
    dict(cls, evidence, note, sweep)."""
    apiscan = {}
    manifest_resets = {}
    for ap, dorigin, dtier in manifests:
        if not os.path.exists(ap):
            continue
        with open(ap, newline="", encoding="utf-8", errors="replace") as fh:
            for r in csv.DictReader(fh):
                if (r.get("verdict") or "").strip().upper() != "CONFIRM":
                    continue
                a = (r.get("addr") or "").strip().upper().replace("0X", "")
                if not re.fullmatch(r"[0-9A-F]{8}", a) or not (r.get("proposed_name") or "").strip():
                    continue
                cls = (r.get("evidence_class") or "").strip()
                if cls == "libscan-desync":
                    continue
                # basis-withdrawn (owner ruling Q103): the name's only recorded basis was a
                # name since RESET; consumed exactly like a verified contradiction.
                if cls in ("reset-contradicted", "basis-withdrawn"):
                    manifest_resets[a] = dict(cls=cls, evidence=(r.get("evidence") or "").strip(),
                                              note=(r.get("verifier_note") or "").strip(),
                                              sweep=os.path.basename(os.path.dirname(ap)))
                    apiscan.pop(a, None)  # a later RESET retires an earlier KEEP / RENAME
                    continue
                origin, tier = class_tier.get(cls, (dorigin, dtier))
                if not origin:
                    continue
                apiscan[a] = dict(name=r["proposed_name"].strip(),
                                  evidence=(r.get("evidence") or "").strip(),
                                  note=(r.get("verifier_note") or "").strip(),
                                  origin=origin, tier=tier)
                manifest_resets.pop(a, None)  # a later KEEP / RENAME supersedes a RESET
    return apiscan, manifest_resets
