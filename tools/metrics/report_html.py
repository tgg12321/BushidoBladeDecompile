"""tools/metrics/report_html.py — generate a self-contained metrics dashboard.

A single HTML file (inline CSS + inline-SVG charts; NO external/CDN/JS deps, so
it opens anywhere, offline) summarizing how the agents performed. Run sync first.

    python tools/metrics/report_html.py                 # -> metrics/report.html
    python tools/metrics/report_html.py --open          # also open in browser
    python tools/metrics/report_html.py --days 7 --theme light --title "Last week"

Configurable: --out, --title, --days (experiment window), --theme dark|light,
--sections (comma list of: summary,leaderboard,experiments,models), --dsn.
"""
from __future__ import annotations

import argparse
import html
import sys
import webbrowser
from datetime import datetime, timezone
from pathlib import Path

try:
    import psycopg
except ImportError:
    sys.exit("psycopg not installed. Run:  python -m pip install \"psycopg[binary]\"")

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_OUT = ROOT / "metrics" / "report.html"
DSN = "host=127.0.0.1 port=5432 user=postgres dbname=bb2_metrics"

THEMES = {
    "dark":  {"bg": "#0d1117", "panel": "#161b22", "panel2": "#1c2230", "fg": "#e6edf3",
              "muted": "#8b949e", "border": "#30363d", "accent": "#58a6ff",
              "good": "#3fb950", "bad": "#f85149", "track": "#21262d"},
    "light": {"bg": "#f6f8fa", "panel": "#ffffff", "panel2": "#f0f3f6", "fg": "#1f2328",
              "muted": "#656d76", "border": "#d0d7de", "accent": "#0969da",
              "good": "#1a7f37", "bad": "#cf222e", "track": "#eaeef2"},
}


def q(cur, sql, args=()):
    cur.execute(sql, args)
    cols = [d.name for d in cur.description]
    return [dict(zip(cols, r)) for r in cur.fetchall()]


def esc(v):
    return html.escape("" if v is None else str(v))


def fmt_usd(v):
    return "—" if v is None else f"${float(v):.3f}"


def fmt_num(v, n=0):
    return "—" if v is None else f"{float(v):,.{n}f}"


# ---------------------------------------------------------------------------
def svg_bars(rows, c, fmt, unit="", color_key="accent", height=None):
    """rows: list of (label, value). Horizontal bars as inline SVG."""
    rows = [(l, float(v) if v is not None else 0.0) for l, v in rows]
    if not rows:
        return "<p class='muted'>no data yet</p>"
    maxv = max((v for _, v in rows), default=0) or 1.0
    lblw, barw, rh, pad = 200, 360, 26, 8
    W, H = lblw + barw + 90, len(rows) * (rh + pad) + pad
    color = c[color_key]
    out = [f"<svg width='100%' viewBox='0 0 {W} {H}' class='svg'>"]
    y = pad
    for label, v in rows:
        w = max(2, int(barw * (v / maxv)))
        out.append(f"<text x='0' y='{y + rh*0.68}' class='svglbl'>{esc(label)}</text>")
        out.append(f"<rect x='{lblw}' y='{y}' width='{barw}' height='{rh}' rx='4' fill='{c['track']}'/>")
        out.append(f"<rect x='{lblw}' y='{y}' width='{w}' height='{rh}' rx='4' fill='{color}'/>")
        out.append(f"<text x='{lblw + barw + 8}' y='{y + rh*0.68}' class='svgval'>{esc(fmt(v))}{esc(unit)}</text>")
        y += rh + pad
    out.append("</svg>")
    return "".join(out)


def table(headers, rows, aligns=None):
    aligns = aligns or ["left"] * len(headers)
    th = "".join(f"<th style='text-align:{a}'>{esc(h)}</th>" for h, a in zip(headers, aligns))
    body = []
    for r in rows:
        tds = "".join(f"<td style='text-align:{a}'>{c}</td>" for c, a in zip(r, aligns))
        body.append(f"<tr>{tds}</tr>")
    if not rows:
        body = [f"<tr><td colspan='{len(headers)}' class='muted'>no data yet</td></tr>"]
    return f"<table><thead><tr>{th}</tr></thead><tbody>{''.join(body)}</tbody></table>"


def badge(ok, c):
    if ok is None:
        return "<span class='badge muted-badge'>n/a</span>"
    col, txt = (c["good"], "yes") if ok else (c["bad"], "no")
    return f"<span class='badge' style='background:{col}22;color:{col};border:1px solid {col}55'>{txt}</span>"


# ---------------------------------------------------------------------------
def build(cur, args, c):
    days = args.days
    window = f"ts > now() - interval '{int(days)} days'" if days else "true"
    where = window + (f" AND func = '{args.func}'" if getattr(args, "func", None) else "")
    parts = []

    # --- summary cards
    if "summary" in args.sections:
        s = q(cur, f"""SELECT count(*) n, count(*) FILTER (WHERE matched) m,
                      coalesce(sum(total_cost_usd),0) spend,
                      avg(total_cost_usd) FILTER (WHERE matched) cpm
                      FROM experiments WHERE {where}""")[0]
        rate = (100.0 * s["m"] / s["n"]) if s["n"] else 0
        cards = [("Experiments", fmt_num(s["n"])), ("Match rate", f"{rate:.0f}%"),
                 ("Total spend", fmt_usd(s["spend"])), ("Avg $/match", fmt_usd(s["cpm"]))]
        cells = "".join(f"<div class='card'><div class='cardv'>{v}</div>"
                        f"<div class='cardk'>{esc(k)}</div></div>" for k, v in cards)
        parts.append(f"<div class='cards'>{cells}</div>")

    # --- leaderboard: model x effort
    if "leaderboard" in args.sections:
        lb = q(cur, f"""SELECT model, effort, count(*) runs,
                   count(*) FILTER (WHERE matched) matches,
                   round(100.0*count(*) FILTER (WHERE matched)/count(*),0) rate,
                   avg(total_cost_usd) FILTER (WHERE matched) cpm,
                   avg(num_turns) turns, avg(wall_s) wall
                   FROM experiments WHERE {where}
                   GROUP BY model, effort ORDER BY model, effort""")
        rowlabel = lambda r: f"{r['model'].replace('claude-','')}/{r['effort']}"
        trows = [[esc(r["model"]), esc(r["effort"]), fmt_num(r["runs"]),
                  f"{fmt_num(r['matches'])} ({fmt_num(r['rate'])}%)",
                  fmt_usd(r["cpm"]), fmt_num(r["turns"], 1), fmt_num(r["wall"], 0)]
                 for r in lb]
        tbl = table(["Model", "Effort", "Runs", "Matched", "Avg $/match", "Avg turns", "Avg wall s"],
                    trows, ["left", "left", "right", "right", "right", "right", "right"])
        ch1 = svg_bars([(rowlabel(r), r["rate"]) for r in lb], c, lambda v: f"{v:.0f}", "%", "good")
        ch2 = svg_bars([(rowlabel(r), r["cpm"]) for r in lb], c, lambda v: f"${v:.2f}", "", "accent")
        parts.append(f"""<div class='panel'><h2>Model × effort</h2>{tbl}
            <div class='charts'>
              <div><h3>Match rate</h3>{ch1}</div>
              <div><h3>Avg cost per match</h3>{ch2}</div>
            </div></div>""")

    # --- match grid: agent (row) x function (column)
    if "grid" in args.sections:
        g = q(cur, f"""SELECT func, model, effort, bool_or(matched) m,
                   avg(total_cost_usd) c, count(*) n
                   FROM experiments WHERE {where} GROUP BY func, model, effort""")
        funcs = sorted({r["func"] for r in g})
        agents = sorted({(r["model"], r["effort"]) for r in g})
        cell = {(r["func"], r["model"], r["effort"]): r for r in g}
        rows = []
        for m, e in agents:
            row = [esc(f"{m.replace('claude-', '')}/{e}")]
            for fn in funcs:
                r = cell.get((fn, m, e))
                if r is None:
                    row.append("<span class='muted'>—</span>")
                else:
                    row.append(f"{badge(r['m'], c)} <span class='muted'>{fmt_usd(r['c'])}</span>")
            rows.append(row)
        tbl = table(["Agent"] + funcs, rows, ["left"] + ["center"] * len(funcs))
        parts.append(f"<div class='panel'><h2>Match grid — agent × function</h2>{tbl}</div>")

    # --- individual experiments
    if "experiments" in args.sections:
        ex = q(cur, f"""SELECT ts, func, model, effort, matched, full_sha1_match,
                   total_cost_usd, num_turns, wall_s, terminal_reason
                   FROM experiments WHERE {where} ORDER BY ts DESC LIMIT 60""")
        trows = [[esc(str(r["ts"])[:16]), esc(r["func"]), esc(r["model"].replace("claude-", "")),
                  esc(r["effort"]), badge(r["matched"], c), badge(r["full_sha1_match"], c),
                  fmt_usd(r["total_cost_usd"]), fmt_num(r["num_turns"]),
                  fmt_num(r["wall_s"], 0), esc(r["terminal_reason"])]
                 for r in ex]
        tbl = table(["When", "Function", "Model", "Effort", "Match", "SHA1", "Cost",
                     "Turns", "Wall s", "Terminal"], trows,
                    ["left", "left", "left", "left", "center", "center", "right", "right", "right", "left"])
        parts.append(f"<div class='panel'><h2>Experiments</h2>{tbl}</div>")

    # --- organic model usage (secondary; estimated)
    if "models" in args.sections:
        mu = q(cur, """SELECT model, count(*) runs, count(*) FILTER (WHERE is_subagent) sub,
                  sum(input_tokens+output_tokens) tok, round(sum(est_cost_usd)::numeric,2) usd
                  FROM agent_runs GROUP BY model ORDER BY usd DESC NULLS LAST LIMIT 12""")
        trows = [[esc(r["model"]), fmt_num(r["runs"]), fmt_num(r["sub"]),
                  fmt_num(r["tok"]), fmt_usd(r["usd"])] for r in mu]
        tbl = table(["Model", "Runs", "Subagents", "Tokens", "Est $ (≈)"], trows,
                    ["left", "right", "right", "right", "right"])
        parts.append(f"<div class='panel'><h2>All session activity "
                     f"<span class='muted'>(estimated — see note)</span></h2>{tbl}"
                     f"<p class='muted note'>Estimated from transcript token sums (an upper bound; "
                     f"caches recur per turn). Experiment costs above are exact (CLI-billed).</p></div>")
    return "\n".join(parts)


def render(body, args, c):
    gen = datetime.now(timezone.utc).astimezone().strftime("%Y-%m-%d %H:%M %Z")
    scope = f"function {args.func}" if getattr(args, "func", None) else "all functions"
    win = (f"last {args.days} days" if args.days else "all time") + " · " + scope
    return f"""<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{esc(args.title)}</title>
<style>
 :root {{ color-scheme: {'dark' if args.theme=='dark' else 'light'}; }}
 * {{ box-sizing: border-box; }}
 body {{ margin:0; background:{c['bg']}; color:{c['fg']};
   font:14px/1.5 -apple-system,Segoe UI,Roboto,Helvetica,Arial,sans-serif; }}
 .wrap {{ max-width:1100px; margin:0 auto; padding:32px 20px 64px; }}
 header h1 {{ margin:0 0 2px; font-size:24px; letter-spacing:-.3px; }}
 header .sub {{ color:{c['muted']}; font-size:13px; margin-bottom:24px; }}
 .cards {{ display:grid; grid-template-columns:repeat(auto-fit,minmax(160px,1fr)); gap:14px; margin-bottom:24px; }}
 .card {{ background:{c['panel']}; border:1px solid {c['border']}; border-radius:12px; padding:18px 20px; }}
 .cardv {{ font-size:28px; font-weight:650; font-variant-numeric:tabular-nums; }}
 .cardk {{ color:{c['muted']}; font-size:12px; text-transform:uppercase; letter-spacing:.6px; margin-top:4px; }}
 .panel {{ background:{c['panel']}; border:1px solid {c['border']}; border-radius:12px; padding:20px 22px; margin-bottom:22px; }}
 .panel h2 {{ margin:0 0 14px; font-size:16px; }}
 .panel h3 {{ margin:6px 0 8px; font-size:12px; color:{c['muted']}; text-transform:uppercase; letter-spacing:.5px; }}
 table {{ width:100%; border-collapse:collapse; font-variant-numeric:tabular-nums; }}
 th,td {{ padding:8px 10px; border-bottom:1px solid {c['border']}; white-space:nowrap; }}
 th {{ color:{c['muted']}; font-weight:600; font-size:11px; text-transform:uppercase; letter-spacing:.5px; }}
 tbody tr:hover {{ background:{c['panel2']}; }}
 .charts {{ display:grid; grid-template-columns:1fr 1fr; gap:24px; margin-top:18px; }}
 @media(max-width:760px) {{ .charts {{ grid-template-columns:1fr; }} }}
 .svg {{ overflow:visible; }}
 .svglbl {{ fill:{c['fg']}; font-size:12px; }}
 .svgval {{ fill:{c['muted']}; font-size:12px; font-variant-numeric:tabular-nums; }}
 .badge {{ padding:2px 9px; border-radius:20px; font-size:12px; font-weight:600; }}
 .muted-badge {{ background:{c['track']}; color:{c['muted']}; }}
 .muted {{ color:{c['muted']}; }} .note {{ font-size:12px; margin:12px 0 0; }}
 td .badge {{ display:inline-block; }}
</style></head><body><div class="wrap">
<header><h1>{esc(args.title)}</h1>
<div class="sub">Bushido Blade 2 decomp · {esc(win)} · generated {esc(gen)}</div></header>
{body}
</div></body></html>"""


def main():
    ap = argparse.ArgumentParser(description="generate the metrics HTML dashboard")
    ap.add_argument("--out", default=str(DEFAULT_OUT))
    ap.add_argument("--title", default="Decomp agent metrics")
    ap.add_argument("--days", type=int, default=0, help="experiment window in days (0 = all)")
    ap.add_argument("--func", default=None, help="scope the whole report to one function")
    ap.add_argument("--theme", choices=["dark", "light"], default="dark")
    ap.add_argument("--sections", default="summary,leaderboard,grid,experiments,models")
    ap.add_argument("--dsn", default=DSN)
    ap.add_argument("--open", action="store_true", help="open the file in a browser")
    a = ap.parse_args()
    import re
    if a.func and not re.fullmatch(r"[A-Za-z0-9_]+", a.func):
        sys.exit("invalid --func (expected an identifier)")
    a.sections = [s.strip() for s in a.sections.split(",") if s.strip()]
    c = THEMES[a.theme]

    with psycopg.connect(a.dsn, connect_timeout=5) as conn, conn.cursor() as cur:
        body = build(cur, a, c)
    out = Path(a.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(render(body, a, c), encoding="utf-8")
    print(f"wrote {out}")
    if a.open:
        webbrowser.open(out.as_uri())


if __name__ == "__main__":
    main()
