import re,collections,json
sony=set()
d=json.load(open("docs/naming/libscan/libsyms.json"))
for a,v in d.items():
    for e in v: sony.add(e["name"])
sony |= set("SsVabOpenHead SsVabTransBody SsSeqOpen SsSeqPlay SsUtKeyOn SsUtKeyOff CdRead CdReadSync CdSearchFile LoadImage StoreImage MoveImage DrawOTag ClearOTagR PutDrawEnv PutDispEnv VSync DrawSync RotTransPers RotMatrix SetRotMatrix SetTransMatrix printf sprintf rand srand memcpy memset strcpy strlen open read close firstfile nextfile format InitCARD StartCARD Exec LoadExec FlushCache EnterCriticalSection ExitCriticalSection OpenEvent EnableEvent TestEvent SpuSetCommonAttr SpuSetKey SpuSetVoiceAttr SsSetMVol SsSeqClose CdControl CdControlB CdSync CdInit InitPAD StartPAD AddCOMB DelCOMB SetDispMask ResetGraph InitGeom SetGeomOffset SetGeomScreen ApplyMatrixLV MulMatrix0 ratan2 rsin rcos SquareRoot0 GetTPage GetClut SetPolyFT4 SetPolyGT4 SetPolyG4 AddPrim SetSemiTrans SetShadeTex LoadTPage LoadClut".split())
lib = {"comb","display","gpu","ings2","main","main_post","system","text1b_b_tu2","text1b_b_tu3"}
for f in open("tmp/restructure-survey/srcs.txt").read().split():
    if f in lib or "rodata" in f or f in("ings_strings","text1a_filepaths","code6cac_c_ab_pad"): continue
    src=open(f"src/{f}.c",encoding="utf-8",errors="replace").read()
    body=re.sub(r"/\*.*?\*/","",src,flags=re.S)
    body=re.sub(r"^\s*extern.*$","",body,flags=re.M)
    calls=collections.Counter(m for m in re.findall(r"\b([A-Za-z_]\w*)\s*\(",body) if m in sony)
    defs=re.findall(r"^(?:static\s+)?[A-Za-z_][\w \*]*?\b([a-z]+_[A-Za-z0-9_]+)\s*\([^;]*\)\s*\{",src,re.M)
    pre=collections.Counter(n.split("_")[0] for n in defs if not n.startswith("func_"))
    print(f"{f}: prefixes={dict(pre.most_common(6))} sony={dict(calls.most_common(8))}")
