import json,re
C=json.load(open("/tmp/q56/census.json")); R=json.load(open("/tmp/q56/results.json"))
for k,c in C.items():
    if c.get("gp_tu_nongp_direct"): print("defining-TU non-gp:", k, c["gp_tu_nongp_direct"], c["gp_user_tus"])
