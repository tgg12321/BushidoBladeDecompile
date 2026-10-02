import re
base=open('base.c').read()
head,tail0=base.split("    SetIntrMask(D_8009BF84);\n")
CB="((void (*)(void))g_gpu_ctx.drawsync_cb)"
T={}
T['v01_comma']=f"""    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*D_8009BF54 & 0x01000000) && g_gpu_ctx.unk08 != 0 &&
        g_gpu_ctx.drawsync_cb != 0) {{
        g_gpu_ctx.unk08 = 0, {CB}();
    }}
    return (_qin - _qout) & 0x3F;
}}
"""
T['v02_cond_expr']=f"""    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*D_8009BF54 & 0x01000000) && g_gpu_ctx.unk08 != 0) {{
        g_gpu_ctx.drawsync_cb ? (g_gpu_ctx.unk08 = 0, {CB}()) : (void)0;
    }}
    return (_qin - _qout) & 0x3F;
}}
"""
T['v03_ret_in_arm']=f"""    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*D_8009BF54 & 0x01000000) && g_gpu_ctx.unk08 != 0 &&
        g_gpu_ctx.drawsync_cb != 0) {{
        g_gpu_ctx.unk08 = 0;
        {CB}();
        return (_qin - _qout) & 0x3F;
    }}
    return (_qin - _qout) & 0x3F;
}}
"""
T['v04_switch']=f"""    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*D_8009BF54 & 0x01000000)) {{
        switch (g_gpu_ctx.unk08) {{
        case 0:
            break;
        default:
            if (g_gpu_ctx.drawsync_cb != 0) {{
                g_gpu_ctx.unk08 = 0;
                {CB}();
            }}
            break;
        }}
    }}
    return (_qin - _qout) & 0x3F;
}}
"""
T['v05_forbreak']=f"""    SetIntrMask(D_8009BF84);
    for (;;) {{
        if (_qin != _qout || (*D_8009BF54 & 0x01000000) || g_gpu_ctx.unk08 == 0 ||
            g_gpu_ctx.drawsync_cb == 0) {{
            break;
        }}
        g_gpu_ctx.unk08 = 0;
        {CB}();
        break;
    }}
    return (_qin - _qout) & 0x3F;
}}
"""
T['v06_while_once']=f"""    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*D_8009BF54 & 0x01000000) && g_gpu_ctx.unk08 != 0) {{
        while (g_gpu_ctx.drawsync_cb != 0) {{
            g_gpu_ctx.unk08 = 0;
            {CB}();
            break;
        }}
    }}
    return (_qin - _qout) & 0x3F;
}}
"""
T['v07_fp_deref']=f"""    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*D_8009BF54 & 0x01000000) && g_gpu_ctx.unk08 != 0 &&
        g_gpu_ctx.drawsync_cb != 0) {{
        g_gpu_ctx.unk08 = 0;
        (*(void (*)(void))g_gpu_ctx.drawsync_cb)();
    }}
    return (_qin - _qout) & 0x3F;
}}
"""
T['v08_cb_first_local']=f"""    SetIntrMask(D_8009BF84);
    {{
        void (*cb)(void);
        if (_qin == _qout && !(*D_8009BF54 & 0x01000000) && g_gpu_ctx.unk08 != 0 &&
            (cb = (void (*)(void))g_gpu_ctx.drawsync_cb) != 0) {{
            g_gpu_ctx.unk08 = 0;
            cb();
        }}
    }}
    return (_qin - _qout) & 0x3F;
}}
"""
T['v09_goto_twice']=f"""    SetIntrMask(D_8009BF84);
    if (_qin != _qout) goto done;
    if (*D_8009BF54 & 0x01000000) goto done;
    if (g_gpu_ctx.unk08 == 0) goto done;
    if (g_gpu_ctx.drawsync_cb == 0) goto done;
    g_gpu_ctx.unk08 = 0;
    {CB}();
done:
    return (_qin - _qout) & 0x3F;
}}
"""
T['v10_flag_and_assign']=f"""    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*D_8009BF54 & 0x01000000) && g_gpu_ctx.unk08 != 0 &&
        g_gpu_ctx.drawsync_cb != 0 && !(g_gpu_ctx.unk08 = 0)) {{
        {CB}();
    }}
    return (_qin - _qout) & 0x3F;
}}
"""
T['v11_ret_var']=f"""    SetIntrMask(D_8009BF84);
    if (_qin == _qout && !(*D_8009BF54 & 0x01000000) && g_gpu_ctx.unk08 != 0 &&
        g_gpu_ctx.drawsync_cb != 0) {{
        g_gpu_ctx.unk08 = 0;
        {CB}();
    }}
    {{ s32 n = (_qin - _qout) & 0x3F; return n; }}
}}
"""
for k,v in T.items():
    open(k+'.c','w').write(head+v)
# helper variants (static inline helpers placed before the function)
hcall="static void exeque_cb(void (*f)(void)) {\n    f();\n}\n\n"
hclr="static void exeque_clr(s32 *p) {\n    *p = 0;\n}\n\n"
b=head+tail0
open('v12_inline_call.c','w').write(hcall+b.replace(f"        {CB}();\n","        exeque_cb((void (*)(void))g_gpu_ctx.drawsync_cb);\n"))
open('v13_inline_clr.c','w').write(hclr+b.replace("        g_gpu_ctx.unk08 = 0;\n","        exeque_clr(&g_gpu_ctx.unk08);\n"))
print(sorted(T))
