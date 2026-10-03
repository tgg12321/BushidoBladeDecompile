"""Keep the committed queue and add only regen's _SendPAD item (the other 10 regen items are
data-as-code labels the integrity audit excludes; pre-existing regen noise, not work)."""
import json, sys
sys.path.insert(0, ".")
from engine import queue as Q
regen = json.load(open("engine/queue.json"))
item = [i for i in regen["items"] if i["func"] == "_SendPAD"][0]
item["reopen_reason"] = ("Q108 split (hand-off work item D): canonical gate C; best honest C floor 4 "
                         "(candidates memory/grind/_SendPAD/): GCC allocates $v0 and saves $ra at 0x10, the "
                         "original uses $t1 and 0x14; cc1psx gives the same 4. No asm grant without its own evidence.")
head = json.load(open("tmp/wd/queue_head.json"))
with Q._locked():
    tok = Q._fingerprint()
    head["items"] = [item]
    head["counts"] = Q._counts(head["items"])
    Q.save(head, expect=tok)
print(json.dumps(head, indent=1)[:1200])
