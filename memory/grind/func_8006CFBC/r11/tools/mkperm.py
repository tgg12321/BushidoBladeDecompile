import sys, os
# mkperm.py <candidate body .c> <workspace dir>: minimal TU for the permuter
body = open(sys.argv[1]).read().replace("\r\n", "\n")
hdr = """typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef struct GameObj GameObj;
extern s32 AddPrim(s32, s32);

"""
os.makedirs(sys.argv[2], exist_ok=True)
open(os.path.join(sys.argv[2], "base.c"), "w", newline="\n").write(hdr + body)
