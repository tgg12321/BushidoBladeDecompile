--
-- PostgreSQL database dump
--

\restrict EGlKCkTiDtQ8XjRKDc2MiaEyVmmkIwTBscls7N0I7AwCY9hJLh5flmKSotDbPG6

-- Dumped from database version 18.4
-- Dumped by pg_dump version 18.4

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: agent_runs; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.agent_runs (
    run_id text NOT NULL,
    parent_run_id text,
    is_subagent boolean DEFAULT false NOT NULL,
    model text,
    branch text,
    cwd text,
    started_at timestamp with time zone,
    ended_at timestamp with time zone,
    wall_clock_s numeric,
    api_duration_s numeric,
    input_tokens bigint,
    output_tokens bigint,
    cache_read_tokens bigint,
    cache_creation_tokens bigint,
    message_count integer,
    est_cost_usd numeric,
    transcript_path text
);


ALTER TABLE public.agent_runs OWNER TO postgres;

--
-- Name: attempt_logs; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.attempt_logs (
    log_id bigint NOT NULL,
    attempt_id bigint,
    kind text,
    transcript_path text,
    line_start integer,
    line_end integer
);


ALTER TABLE public.attempt_logs OWNER TO postgres;

--
-- Name: attempt_logs_log_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.attempt_logs ALTER COLUMN log_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.attempt_logs_log_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: attempt_techniques; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.attempt_techniques (
    attempt_id bigint NOT NULL,
    technique_id bigint NOT NULL,
    relation text
);


ALTER TABLE public.attempt_techniques OWNER TO postgres;

--
-- Name: attempts; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.attempts (
    attempt_id bigint NOT NULL,
    func text NOT NULL,
    file text,
    run_id text,
    model text,
    started_at timestamp with time zone,
    ended_at timestamp with time zone,
    wall_clock_s numeric,
    start_score integer,
    final_score integer,
    outcome text,
    matched boolean DEFAULT false NOT NULL,
    commit_sha text,
    input_tokens bigint,
    output_tokens bigint,
    est_cost_usd numeric,
    n_events integer,
    notes text
);


ALTER TABLE public.attempts OWNER TO postgres;

--
-- Name: attempts_attempt_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.attempts ALTER COLUMN attempt_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.attempts_attempt_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: engine_events; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.engine_events (
    event_id bigint NOT NULL,
    ts timestamp with time zone NOT NULL,
    command text NOT NULL,
    func text,
    file text,
    session_id text,
    git_commit text,
    branch text,
    cwd text,
    pid integer,
    ppid integer,
    exit_code integer,
    score integer,
    verdict text,
    sha1 text,
    ok boolean,
    asm_insns integer,
    target_insns integer,
    build_insns integer,
    total_dropped integer,
    extra jsonb,
    payload jsonb,
    line_hash text
);


ALTER TABLE public.engine_events OWNER TO postgres;

--
-- Name: engine_events_event_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.engine_events ALTER COLUMN event_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.engine_events_event_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: experiments; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.experiments (
    experiment_id bigint NOT NULL,
    ts timestamp with time zone,
    func text,
    file text,
    model text,
    effort text,
    session_id text,
    budget_usd numeric,
    minutes integer,
    matched boolean,
    start_score integer,
    final_score integer,
    full_sha1_match boolean,
    total_cost_usd numeric,
    input_tokens bigint,
    output_tokens bigint,
    cache_read_tokens bigint,
    cache_creation_tokens bigint,
    num_turns integer,
    duration_s numeric,
    wall_s numeric,
    terminal_reason text,
    git_commit text,
    notes text,
    run_key text
);


ALTER TABLE public.experiments OWNER TO postgres;

--
-- Name: experiments_experiment_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.experiments ALTER COLUMN experiment_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.experiments_experiment_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Name: sync_meta; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.sync_meta (
    key text NOT NULL,
    value text,
    ts timestamp with time zone DEFAULT now()
);


ALTER TABLE public.sync_meta OWNER TO postgres;

--
-- Name: techniques; Type: TABLE; Schema: public; Owner: postgres
--

CREATE TABLE public.techniques (
    technique_id bigint NOT NULL,
    slug text,
    content_hash text,
    kind text,
    title text,
    rule_path text,
    first_seen_ts timestamp with time zone,
    first_seen_commit text,
    description text
);


ALTER TABLE public.techniques OWNER TO postgres;

--
-- Name: techniques_technique_id_seq; Type: SEQUENCE; Schema: public; Owner: postgres
--

ALTER TABLE public.techniques ALTER COLUMN technique_id ADD GENERATED ALWAYS AS IDENTITY (
    SEQUENCE NAME public.techniques_technique_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1
);


--
-- Data for Name: agent_runs; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.agent_runs (run_id, parent_run_id, is_subagent, model, branch, cwd, started_at, ended_at, wall_clock_s, api_duration_s, input_tokens, output_tokens, cache_read_tokens, cache_creation_tokens, message_count, est_cost_usd, transcript_path) FROM stdin;
04e7b9c2-b9a8-4e88-b2b5-e87457bfe179	\N	f	\N	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-22 20:38:25.776-05	2026-05-22 20:38:55.527-05	29.751	0	0	0	0	0	0	0	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\04e7b9c2-b9a8-4e88-b2b5-e87457bfe179.jsonl
0710b979-5c23-4279-92c9-17ddb30d0fc2	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 02:11:43.003-05	2026-05-26 02:17:09.04-05	326.037	0	121953	40898	2715037	240394	46	13.476588	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\0710b979-5c23-4279-92c9-17ddb30d0fc2.jsonl
08adaeb4-9997-49e6-ae5a-79704a93e584	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 12:20:00.913-05	2026-05-18 22:11:53.761-05	35512.848	0	4885	297776	31931283	1764794	212	103.393287	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\08adaeb4-9997-49e6-ae5a-79704a93e584.jsonl
0ce66709-01d1-4deb-bbb3-79fc676a77a7	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\decomp-4	2026-05-22 14:52:56.848-05	2026-05-22 15:17:33.227-05	1476.379	0	121668	175553	8147346	949547	87	45.01652	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\0ce66709-01d1-4deb-bbb3-79fc676a77a7.jsonl
0d8232ce-18c5-4ca3-8d90-98ef9e4d9373	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\codex-asmfix-queue	2026-05-23 12:16:49.457-05	2026-05-23 12:25:13.075-05	503.618	0	68926	21909	1798450	196381	36	9.056884	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\0d8232ce-18c5-4ca3-8d90-98ef9e4d9373.jsonl
0d922e81-a4ea-4c5d-85ff-52b39b139da0	\N	f	\N	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-06 17:46:29.733-05	2026-05-06 17:46:30.221-05	0.488	0	0	0	0	0	0	0	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\0d922e81-a4ea-4c5d-85ff-52b39b139da0.jsonl
0f9520bc-2ce1-468a-ae1e-4b9827bb2431	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 22:53:04.225-05	2026-05-18 01:51:48.321-05	10724.096	0	1732	1167633	449226194	2285229	935	804.28579	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\0f9520bc-2ce1-468a-ae1e-4b9827bb2431.jsonl
agent-a3e347ec0d03a68ec	0f9520bc-2ce1-468a-ae1e-4b9827bb2431	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 01:44:58.103-05	2026-05-18 01:48:15.21-05	197.107	0	76	20923	1299304	50780	37	0.298096	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\0f9520bc-2ce1-468a-ae1e-4b9827bb2431\\subagents\\agent-a3e347ec0d03a68ec.jsonl
agent-a760d252a959da48a	0f9520bc-2ce1-468a-ae1e-4b9827bb2431	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:18:20.056-05	2026-05-18 00:20:33.024-05	132.968	0	4959	8321	1229599	397895	23	10.00339	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\0f9520bc-2ce1-468a-ae1e-4b9827bb2431\\subagents\\agent-a760d252a959da48a.jsonl
agent-ace8718b31b49e81b	0f9520bc-2ce1-468a-ae1e-4b9827bb2431	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 01:35:38.535-05	2026-05-18 01:38:42.641-05	184.106	0	150	18174	1363451	93422	41	0.344143	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\0f9520bc-2ce1-468a-ae1e-4b9827bb2431\\subagents\\agent-ace8718b31b49e81b.jsonl
agent-adfb0401cad1c0500	0f9520bc-2ce1-468a-ae1e-4b9827bb2431	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 01:41:33.541-05	2026-05-18 01:44:11.731-05	158.19	0	111	16751	978466	45850	30	0.239025	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\0f9520bc-2ce1-468a-ae1e-4b9827bb2431\\subagents\\agent-adfb0401cad1c0500.jsonl
0feddb91-53a4-4429-8f4b-152b163dc088	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 16:57:55.576-05	2026-05-17 22:49:44.633-05	21109.057	0	10655	477636	66530667	1972714	330	172.766913	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\0feddb91-53a4-4429-8f4b-152b163dc088.jsonl
1030bf50-d8b5-48d6-aea4-b7e5c463bb78	\N	f	claude-opus-4-7	worktree-claude-work	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\claude-work	2026-05-18 22:21:23.472-05	2026-05-18 23:41:21.122-05	4797.65	0	5127	418125	66392077	1037484	364	150.47722	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\1030bf50-d8b5-48d6-aea4-b7e5c463bb78.jsonl
114f803d-d073-47b3-a26e-26dd2914b421	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 01:27:53.281-05	2026-05-16 11:26:59.724-05	35946.443	0	749	539484	213915431	1805105	662	395.1914	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\114f803d-d073-47b3-a26e-26dd2914b421.jsonl
1328c762-f07c-47ed-a832-bb984e60885e	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 01:53:12.919-05	2026-05-26 01:56:48.103-05	215.184	0	91980	20582	905782	149103	27	7.077704	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\1328c762-f07c-47ed-a832-bb984e60885e.jsonl
13d8e212-6f44-490e-900c-1122979921ca	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 21:14:00.202-05	2026-05-16 22:26:22.236-05	4342.034	0	198	112132	21366088	477982	165	49.424165	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\13d8e212-6f44-490e-900c-1122979921ca.jsonl
154213b0-6560-4300-b6c7-72bccdbdfde8	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-07 15:17:39.271-05	2026-05-10 22:18:09.049-05	284429.778	0	9140	6778909	4130969061	25016999	7815	7174.077598	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\154213b0-6560-4300-b6c7-72bccdbdfde8.jsonl
agent-aae006ab36b50d3e7	154213b0-6560-4300-b6c7-72bccdbdfde8	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-08 15:59:05.823-05	2026-05-08 16:33:53.208-05	2087.385	0	159	72586	20051439	368026	149	42.423981	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\154213b0-6560-4300-b6c7-72bccdbdfde8\\subagents\\agent-aae006ab36b50d3e7.jsonl
agent-ae8ece95aa08d8100	154213b0-6560-4300-b6c7-72bccdbdfde8	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-08 19:47:57.669-05	2026-05-08 20:50:00.099-05	3722.43	0	452	119269	71224881	773658	437	130.295364	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\154213b0-6560-4300-b6c7-72bccdbdfde8\\subagents\\agent-ae8ece95aa08d8100.jsonl
agent-af6097d266631cb93	154213b0-6560-4300-b6c7-72bccdbdfde8	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-08 17:50:00.192-05	2026-05-08 18:49:00.92-05	3540.728	0	336	154341	61087477	469368	326	112.01248	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\154213b0-6560-4300-b6c7-72bccdbdfde8\\subagents\\agent-af6097d266631cb93.jsonl
1725cf8f-878f-49ed-b0e1-45169187de36	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-13 23:51:24.496-05	2026-05-14 00:35:29.845-05	2645.349	0	402	358891	34857537	1256290	202	102.764598	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\1725cf8f-878f-49ed-b0e1-45169187de36.jsonl
205a28ec-49ca-49cd-9f32-49777c079823	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 01:47:04.962-05	2026-05-26 01:47:07.602-05	2.64	0	1653	4	0	29994	1	0.587483	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\205a28ec-49ca-49cd-9f32-49777c079823.jsonl
25d978c8-b2a1-4918-88ac-2c0b6fa0783c	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 03:26:38.072-05	2026-05-26 03:45:29.708-05	1131.636	0	67211	148360	8301835	192574	94	28.19868	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\25d978c8-b2a1-4918-88ac-2c0b6fa0783c.jsonl
267918b2-37f1-41f5-b478-26cb68142d70	\N	f	\N	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 00:56:34.218-05	2026-05-26 00:56:34.22-05	0.002	0	0	0	0	0	0	0	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\267918b2-37f1-41f5-b478-26cb68142d70.jsonl
31f28e6e-c26f-4717-98a6-e27c35fb7009	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-14 00:36:44.901-05	2026-05-15 02:32:13.305-05	93328.404	0	3472	5934892	962861020	14147417	1912	2154.724579	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\31f28e6e-c26f-4717-98a6-e27c35fb7009.jsonl
327e722a-86cb-49d9-836c-a3d10f24d2a7	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 17:01:02.283-05	2026-05-16 21:03:02.459-05	14520.176	0	7535	908301	189321016	2026245	592	390.209218	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\327e722a-86cb-49d9-836c-a3d10f24d2a7.jsonl
agent-a030a92b474857923	327e722a-86cb-49d9-836c-a3d10f24d2a7	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a030a92b474857923	2026-05-16 17:19:47.704-05	2026-05-16 17:22:36.634-05	168.93	0	56	8607	2274321	164317	38	7.13879	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\327e722a-86cb-49d9-836c-a3d10f24d2a7\\subagents\\agent-a030a92b474857923.jsonl
agent-ab45e29c1b2eaf463	327e722a-86cb-49d9-836c-a3d10f24d2a7	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 17:04:19.157-05	2026-05-16 17:04:45.455-05	26.298	0	482	956	164973	75262	10	0.115837	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\327e722a-86cb-49d9-836c-a3d10f24d2a7\\subagents\\agent-ab45e29c1b2eaf463.jsonl
agent-ac256b4dc9eb2960b	327e722a-86cb-49d9-836c-a3d10f24d2a7	t	claude-opus-4-7	worktree-agent-ac256b4dc9eb2960b	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ac256b4dc9eb2960b	2026-05-16 17:59:55.769-05	2026-05-16 18:35:09.791-05	2114.022	0	279	61343	32397161	710237	259	66.517595	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\327e722a-86cb-49d9-836c-a3d10f24d2a7\\subagents\\agent-ac256b4dc9eb2960b.jsonl
agent-add27f7c0016664a2	327e722a-86cb-49d9-836c-a3d10f24d2a7	t	claude-opus-4-7	worktree-agent-ac256b4dc9eb2960b	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-add27f7c0016664a2	2026-05-16 18:00:03.517-05	2026-05-16 18:48:25.279-05	2901.762	0	486	99308	81130468	543843	468	139.348148	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\327e722a-86cb-49d9-836c-a3d10f24d2a7\\subagents\\agent-add27f7c0016664a2.jsonl
agent-aea6fb077725a730e	327e722a-86cb-49d9-836c-a3d10f24d2a7	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aea6fb077725a730e	2026-05-16 17:19:41.725-05	2026-05-16 17:22:48.137-05	186.412	0	50	9103	1518724	163863	32	6.033992	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\327e722a-86cb-49d9-836c-a3d10f24d2a7\\subagents\\agent-aea6fb077725a730e.jsonl
379f5971-7080-46f4-8832-b43ce4f7b468	\N	f	claude-sonnet-4-6	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 04:07:27.421-05	2026-05-26 04:27:09.098-05	1181.677	0	56	151987	3524139	294664	52	4.442205	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\379f5971-7080-46f4-8832-b43ce4f7b468.jsonl
391e7dd8-bc5b-4b62-bfff-5d1dac695947	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 23:01:28.274-05	2026-05-18 01:20:44.561-05	8356.287	0	987	836014	166485697	1900674	557	348.082038	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947.jsonl
agent-a0455eefb8b37dd5a	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:28:38.6-05	2026-05-18 00:30:03.419-05	84.819	0	219	6076	2098573	194554	50	0.483649	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-a0455eefb8b37dd5a.jsonl
agent-a0dd53ce45e9d334d	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:42:35-05	2026-05-18 00:43:20.485-05	45.485	0	132	4849	879868	180649	26	0.338175	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-a0dd53ce45e9d334d.jsonl
agent-a13761819699116f3	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:32:04.913-05	2026-05-18 00:32:56.429-05	51.516	0	160	3062	985749	127088	31	0.272905	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-a13761819699116f3.jsonl
agent-a1eac102f27f0665d	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:28:41.926-05	2026-05-18 00:29:50.104-05	68.178	0	274	4668	1996154	384194	50	0.703472	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-a1eac102f27f0665d.jsonl
agent-a227a3ecec04e1d4d	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:17:25.38-05	2026-05-18 00:19:34.34-05	128.96	0	71	6244	1288572	29402	55	0.196901	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-a227a3ecec04e1d4d.jsonl
agent-a45397babba50985e	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 01:04:08.121-05	2026-05-18 01:07:09.188-05	181.067	0	81	9779	2186766	97860	66	0.389978	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-a45397babba50985e.jsonl
agent-a56e702c2eceed105	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:09:11.791-05	2026-05-18 00:10:31.763-05	79.972	0	6104	6017	1889167	146185	53	0.407837	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-a56e702c2eceed105.jsonl
agent-a5c54a1dd2aa7cf64	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:03:36.357-05	2026-05-18 00:04:44.078-05	67.721	0	88	2553	970700	78511	31	0.208062	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-a5c54a1dd2aa7cf64.jsonl
agent-a5e921643cd1c7c4f	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:56:04.57-05	2026-05-18 00:57:25.021-05	80.451	0	38	4847	434870	19044	23	0.091565	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-a5e921643cd1c7c4f.jsonl
agent-a690cf932f09b39ef	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:09:15.862-05	2026-05-18 00:12:03.55-05	167.688	0	238	9745	2978156	116458	76	0.492351	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-a690cf932f09b39ef.jsonl
agent-a93061e63f609f2a2	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:15:03.112-05	2026-05-18 00:17:04.361-05	121.249	0	124	7456	2784025	126671	64	0.474145	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-a93061e63f609f2a2.jsonl
agent-aa841b9b29a5ce268	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:34:56.829-05	2026-05-18 00:36:55.054-05	118.225	0	292	7157	2142732	66252	66	0.333165	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-aa841b9b29a5ce268.jsonl
agent-aca684e2a019ab699	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:34:52.319-05	2026-05-18 00:36:27.624-05	95.305	0	99	5681	1321107	43685	44	0.215221	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-aca684e2a019ab699.jsonl
agent-add7dbaf91f84ca49	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 23:02:34.991-05	2026-05-17 23:04:29.634-05	114.643	0	15974	7977	2244395	242849	63	0.58386	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-add7dbaf91f84ca49.jsonl
agent-aea80766b7825d186	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 01:13:59.013-05	2026-05-18 01:16:13.097-05	134.084	0	46	8556	2232448	85815	73	0.37334	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-aea80766b7825d186.jsonl
agent-af8989ca2a7ceae41	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:55:58.424-05	2026-05-18 00:56:46.452-05	48.028	0	63	3523	264348	47644	15	0.103668	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-af8989ca2a7ceae41.jsonl
agent-afb180135a6ddd826	391e7dd8-bc5b-4b62-bfff-5d1dac695947	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 00:32:09.03-05	2026-05-18 00:33:20.381-05	71.351	0	145	4512	798257	77508	25	0.199416	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\391e7dd8-bc5b-4b62-bfff-5d1dac695947\\subagents\\agent-afb180135a6ddd826.jsonl
39620c3b-bccd-496d-afaf-a867abed8c64	\N	f	\N	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 02:03:52.166-05	2026-05-18 02:03:55.426-05	3.26	0	0	0	0	0	0	0	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\39620c3b-bccd-496d-afaf-a867abed8c64.jsonl
3a76f32f-b878-4337-a0da-4f3649e9ec41	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-06 17:46:38.484-05	2026-05-07 15:17:22.64-05	77444.156	0	1673	2342088	599638717	4702321	1323	1163.308289	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\3a76f32f-b878-4337-a0da-4f3649e9ec41.jsonl
40e83de7-725c-46b1-8565-df6a26490b6d	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 01:14:11.886-05	2026-05-17 02:12:11.066-05	3479.18	0	40398	283583	36521572	539761	235	86.777572	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\40e83de7-725c-46b1-8565-df6a26490b6d.jsonl
41928b72-f379-4c6a-b2ca-21080eefebb8	\N	f	claude-opus-4-7	worktree-decomp-work	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\decomp-work	2026-05-20 23:00:53.009-05	2026-05-21 18:58:17.939-05	71844.93	0	255458	4250431	639694605	13914705	1276	1543.056821	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\41928b72-f379-4c6a-b2ca-21080eefebb8.jsonl
44055c2d-920f-4bf3-9eb0-b8ca31ca8464	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-22 18:14:23.043-05	2026-05-22 18:29:49.524-05	926.481	0	22643	269436	2605236	520103	43	34.20713	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\44055c2d-920f-4bf3-9eb0-b8ca31ca8464.jsonl
466a2bd1-4c94-47f9-8036-16f36df14cfc	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 00:41:08.626-05	2026-05-17 01:26:25.195-05	2716.569	0	401	306090	63908754	775696	314	133.370196	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\466a2bd1-4c94-47f9-8036-16f36df14cfc.jsonl
agent-ae937e345590ba339	466a2bd1-4c94-47f9-8036-16f36df14cfc	t	claude-opus-4-7	worktree-agent-ae937e345590ba339	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ae937e345590ba339	2026-05-17 00:42:48.408-05	2026-05-17 00:46:25.196-05	216.788	0	50966	4715	2192187	254754	29	9.183033	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\466a2bd1-4c94-47f9-8036-16f36df14cfc\\subagents\\agent-ae937e345590ba339.jsonl
4c615811-bd2e-49fa-9d55-6ccd8072590c	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-15 12:40:18.263-05	2026-05-15 22:16:18.854-05	34560.591	0	2535	2325503	1166163575	6653348	2297	2048.446388	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\4c615811-bd2e-49fa-9d55-6ccd8072590c.jsonl
4dc3e7e2-c8df-45ac-a164-31103b9e7177	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\decomp-work	2026-05-19 12:47:09.541-05	2026-05-19 12:56:54.273-05	584.732	0	67	7811	1297746	300869	27	8.174743	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\4dc3e7e2-c8df-45ac-a164-31103b9e7177.jsonl
4e96e912-ecb4-40f0-b936-5fe6b82f3d56	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 03:06:20.496-05	2026-05-26 03:26:00.345-05	1179.849	0	199	185799	9591652	282777	101	33.627457	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\4e96e912-ecb4-40f0-b936-5fe6b82f3d56.jsonl
51c4aa4e-c76a-44fa-8bce-032c3996e7b5	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 00:24:59.975-05	2026-05-17 01:04:59.138-05	2399.163	0	258	283361	23613670	514353	173	66.320569	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\51c4aa4e-c76a-44fa-8bce-032c3996e7b5.jsonl
52d8b5c3-6b23-49f1-98b0-3860c6050800	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\cheat-cleanup	2026-05-21 20:03:25.869-05	2026-05-22 13:20:18.792-05	62212.923	0	426163	4449615	541949036	12959794	1106	1396.033261	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\52d8b5c3-6b23-49f1-98b0-3860c6050800.jsonl
543d88a5-1f83-44fb-9480-2e4038b3e6fe	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-22 19:47:13.174-05	2026-05-22 20:29:49.899-05	2556.725	0	44624	408341	19633249	921158	164	78.016521	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\543d88a5-1f83-44fb-9480-2e4038b3e6fe.jsonl
5b8d684d-bf44-4fa1-8aee-381bba6bd060	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 11:58:36.335-05	2026-05-17 21:38:58.935-05	34822.6	0	4860	1293000	136477667	2511334	472	348.851913	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060.jsonl
agent-a01681790e279fb4f	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-aaf3f6ecc0ed1ef41	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a01681790e279fb4f	2026-05-17 13:11:49.907-05	2026-05-17 13:56:57.702-05	2707.795	0	285	45711	37627300	353736	275	66.5061	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a01681790e279fb4f.jsonl
agent-a041dc7476efb41a0	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a041dc7476efb41a0	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a041dc7476efb41a0	2026-05-17 19:10:15.228-05	2026-05-17 21:35:17.987-05	8702.759	0	628	142394	127306178	1139315	618	223.010393	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a041dc7476efb41a0.jsonl
agent-a12337a9b0f87ed48	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a847163376726fb47	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a12337a9b0f87ed48	2026-05-17 16:28:03.174-05	2026-05-17 17:24:11.449-05	3368.275	0	265	71041	33106916	352421	255	61.600318	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a12337a9b0f87ed48.jsonl
agent-a52856459d634d28a	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a52856459d634d28a	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a52856459d634d28a	2026-05-17 15:29:37.246-05	2026-05-17 16:24:10.176-05	3272.93	0	426	85485	68179770	605669	388	120.043714	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a52856459d634d28a.jsonl
agent-a6481fb724768bd8f	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a6481fb724768bd8f	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a6481fb724768bd8f	2026-05-17 18:50:27.125-05	2026-05-17 20:39:50.469-05	6563.344	0	335	66629	42813401	573511	310	79.975633	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a6481fb724768bd8f.jsonl
agent-a69cfbc0ae1da0e8d	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a01681790e279fb4f	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 12:08:54.167-05	2026-05-17 13:21:15.944-05	4341.777	0	434	115268	74327794	549300	424	130.442676	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a69cfbc0ae1da0e8d.jsonl
agent-a735780f4be32b207	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a7c3708fb6c53518f	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a735780f4be32b207	2026-05-17 14:42:02.729-05	2026-05-17 15:28:24.134-05	2781.405	0	201	46732	25139219	587909	191	52.240037	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a735780f4be32b207.jsonl
agent-a78c681834468242d	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-abbe609d360375e9e	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a78c681834468242d	2026-05-17 12:09:03.672-05	2026-05-17 12:51:07.624-05	2523.952	0	303	58001	35730839	337622	293	64.281291	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a78c681834468242d.jsonl
agent-a7c3708fb6c53518f	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a7c3708fb6c53518f	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a7c3708fb6c53518f	2026-05-17 12:55:40.099-05	2026-05-17 15:15:01.866-05	8361.767	0	2078	165617	254933967	790636	858	409.67782	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a7c3708fb6c53518f.jsonl
agent-a80ee509d004d0371	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a7c3708fb6c53518f	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a80ee509d004d0371	2026-05-17 13:57:44.051-05	2026-05-17 14:57:43.051-05	3599	0	404	109436	72340167	496170	386	126.027198	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a80ee509d004d0371.jsonl
agent-a847163376726fb47	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-aa81f0a030eb09b75	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a847163376726fb47	2026-05-17 16:46:46.718-05	2026-05-17 17:43:13.539-05	3386.821	0	322	105304	52734858	462101	312	95.669311	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a847163376726fb47.jsonl
agent-a8824eec6bb8170e5	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-abb1e497141596368	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a8824eec6bb8170e5	2026-05-17 14:58:04.587-05	2026-05-17 16:04:48.314-05	4003.727	0	514	175215	108422734	604212	504	187.111911	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a8824eec6bb8170e5.jsonl
agent-a8b641975cf0dd6c9	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a12337a9b0f87ed48	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a8b641975cf0dd6c9	2026-05-17 16:05:27.91-05	2026-05-17 16:41:07.784-05	2139.874	0	237	53130	24440314	389509	212	47.95207	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a8b641975cf0dd6c9.jsonl
agent-a8e25a2e3b4b23d30	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-accda67a387de4e42	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a8b641975cf0dd6c9\\.claude\\worktrees\\agent-a8e25a2e3b4b23d30	2026-05-17 16:41:42.624-05	2026-05-17 20:00:51.131-05	11948.507	0	628	182257	153081530	1787163	618	276.810296	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a8e25a2e3b4b23d30.jsonl
agent-a9b8a905082f3cbe3	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a9b8a905082f3cbe3	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a9b8a905082f3cbe3	2026-05-17 12:51:40.053-05	2026-05-17 13:06:20.657-05	880.604	0	112	19142	9042457	183470	102	18.441078	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-a9b8a905082f3cbe3.jsonl
agent-aa27184fa919f548e	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a8e25a2e3b4b23d30	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aa27184fa919f548e	2026-05-17 17:28:46.249-05	2026-05-17 19:31:20.694-05	7354.445	0	468	136589	90999978	556272	458	157.181262	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-aa27184fa919f548e.jsonl
agent-aa81f0a030eb09b75	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-accda67a387de4e42	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aa81f0a030eb09b75	2026-05-17 17:29:00.036-05	2026-05-17 18:47:36.77-05	4716.734	0	340	102298	57468060	501129	330	103.275709	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-aa81f0a030eb09b75.jsonl
agent-aaf3f6ecc0ed1ef41	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-aaf3f6ecc0ed1ef41	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aaf3f6ecc0ed1ef41	2026-05-17 13:21:55.147-05	2026-05-17 14:41:39.113-05	4783.966	0	472	108822	89880800	477259	462	151.938536	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-aaf3f6ecc0ed1ef41.jsonl
agent-abb1e497141596368	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-abb1e497141596368	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-abb1e497141596368	2026-05-17 15:18:13.647-05	2026-05-17 16:55:56.722-05	5863.075	0	7483	190495	222552712	1097069	712	368.798482	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-abb1e497141596368.jsonl
agent-abbe609d360375e9e	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-abbe609d360375e9e	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-abbe609d360375e9e	2026-05-17 12:08:59.33-05	2026-05-17 12:55:17.54-05	2778.21	0	273	48545	28990490	728727	243	60.794336	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-abbe609d360375e9e.jsonl
agent-accda67a387de4e42	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a041dc7476efb41a0	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-accda67a387de4e42	2026-05-17 17:01:49.645-05	2026-05-17 20:50:48.026-05	13738.381	0	812	190216	217287154	957798	797	358.167823	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-accda67a387de4e42.jsonl
agent-aeebb6965b3e4c0a1	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-accda67a387de4e42	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aeebb6965b3e4c0a1	2026-05-17 17:46:58.719-05	2026-05-17 20:17:01.978-05	9003.259	0	658	138477	150589293	892604	635	253.015909	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-aeebb6965b3e4c0a1.jsonl
agent-af7945920adb35e78	5b8d684d-bf44-4fa1-8aee-381bba6bd060	t	claude-opus-4-7	worktree-agent-a8e25a2e3b4b23d30	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-af7945920adb35e78	2026-05-17 16:56:32.545-05	2026-05-17 18:57:12.079-05	7239.534	0	505	122434	90688113	499372	495	154.58552	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5b8d684d-bf44-4fa1-8aee-381bba6bd060\\subagents\\agent-af7945920adb35e78.jsonl
5dcff83b-236a-40bf-a46b-0e35dd842dea	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-20 23:22:10.364-05	2026-05-21 00:21:46.022-05	3575.658	0	24672	355306	23457657	1231855	203	85.301797	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\5dcff83b-236a-40bf-a46b-0e35dd842dea.jsonl
6206d1b3-39eb-45d6-bb1c-f95fd87caeab	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 01:04:32.361-05	2026-05-12 01:44:43.482-05	88811.121	0	16407	1233215	703308833	5538457	1461	1251.546548	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab.jsonl
agent-a01ad22f7ca82ad79	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 13:30:45.882-05	2026-05-11 13:37:04.453-05	378.571	0	85	12588	4167265	104793	75	9.161141	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a01ad22f7ca82ad79.jsonl
agent-a0f6b26176e0e3f8b	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 12:04:37.833-05	2026-05-11 12:18:07.06-05	809.227	0	136	27698	10731529	186239	126	21.668665	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a0f6b26176e0e3f8b.jsonl
agent-a10144365d4c44f5a	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 18:51:16.9-05	2026-05-11 18:54:34.826-05	197.926	0	51	5867	1994672	75249	41	4.843717	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a10144365d4c44f5a.jsonl
agent-a12dbdca6ca2271ba	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 10:52:35.124-05	2026-05-11 10:59:38.371-05	423.247	0	65	10314	1793641	231207	40	7.800118	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a12dbdca6ca2271ba.jsonl
agent-a1ab50a8a3a93c9ad	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 14:10:31.085-05	2026-05-11 14:15:59.841-05	328.756	0	73	10696	3312707	91891	63	7.495312	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a1ab50a8a3a93c9ad.jsonl
agent-a2672e3de77745814	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 10:17:42.94-05	2026-05-11 10:19:39.081-05	116.141	0	40	2355	1022439	139578	25	4.327971	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a2672e3de77745814.jsonl
agent-a278fc15f04e6b1bf	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 13:52:27.422-05	2026-05-11 14:06:19.405-05	831.983	0	142	20428	9682294	201735	127	19.840202	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a278fc15f04e6b1bf.jsonl
agent-a2e2ffe98b1999fee	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 02:37:33.884-05	2026-05-11 02:41:36.245-05	242.361	0	39	5600	938641	83672	24	3.397397	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a2e2ffe98b1999fee.jsonl
agent-a3045927f85520577	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 01:54:34.218-05	2026-05-11 01:58:29.489-05	235.271	0	56	9905	2320791	114039	46	6.363133	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a3045927f85520577.jsonl
agent-a45c5732efd938b46	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 10:07:25.595-05	2026-05-11 10:14:07.217-05	401.622	0	48	6561	1450231	132104	33	5.145092	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a45c5732efd938b46.jsonl
agent-a47f8992b338c99fe	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 01:47:46.505-05	2026-05-11 01:48:44.318-05	57.813	0	24	2439	471798	81944	14	2.427432	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a47f8992b338c99fe.jsonl
agent-a4e6bd39c9e1c5b1f	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 13:08:09.724-05	2026-05-11 13:11:56.04-05	226.316	0	51	7886	1783941	97606	41	5.098239	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a4e6bd39c9e1c5b1f.jsonl
agent-a5278d3dd69245a71	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 12:46:46.633-05	2026-05-11 12:51:51.148-05	304.515	0	7189	5848	1685516	109021	39	5.118853	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a5278d3dd69245a71.jsonl
agent-a529a148c52cc0a71	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 02:44:59.518-05	2026-05-11 02:48:47.369-05	227.851	0	45	4869	947929	104729	25	3.751412	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a529a148c52cc0a71.jsonl
agent-a52d261459610bbb8	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 14:19:33.415-05	2026-05-11 14:27:19.992-05	466.577	0	61	13628	2419094	129622	46	7.082069	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a52d261459610bbb8.jsonl
agent-a5bcd605bc683dbd3	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 11:03:08.543-05	2026-05-11 11:23:06.198-05	1197.655	0	147	24375	9564122	361248	122	22.949913	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a5bcd605bc683dbd3.jsonl
agent-a5ec0480b1e013c51	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 10:28:17.07-05	2026-05-11 10:36:50.322-05	513.252	0	87	12193	4344901	110399	77	9.503113	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a5ec0480b1e013c51.jsonl
agent-a6276a87717912c01	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 09:59:16.693-05	2026-05-11 10:03:11.392-05	234.699	0	34	3620	905297	85844	24	3.23953	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a6276a87717912c01.jsonl
agent-a66bfb2e1e45df705	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 13:15:50.279-05	2026-05-11 13:19:18.346-05	208.067	0	55	5668	1842184	102669	40	5.114245	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a66bfb2e1e45df705.jsonl
agent-a67c586d311d98bc8	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 18:10:06.652-05	2026-05-11 18:21:45.506-05	698.854	0	153	20508	9626548	230495	138	20.301998	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a67c586d311d98bc8.jsonl
agent-a757ecfe3b601790b	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 02:16:48.496-05	2026-05-11 02:18:59.044-05	130.548	0	41	4012	1149536	97203	26	3.848375	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a757ecfe3b601790b.jsonl
agent-a7dc941da2f28db54	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 12:55:54.535-05	2026-05-11 13:02:44.24-05	409.705	0	92	12012	4415935	143329	77	10.213601	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a7dc941da2f28db54.jsonl
agent-a7e1c8381bc4fae73	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 20:58:28.556-05	2026-05-11 21:48:39.781-05	3011.225	0	206	52871	18925586	1096212	191	52.910769	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a7e1c8381bc4fae73.jsonl
agent-a820d944b37cd71e1	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 02:52:17.771-05	2026-05-11 02:57:48.693-05	330.922	0	85	13445	3840588	164927	65	9.862913	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a820d944b37cd71e1.jsonl
agent-a8752b118e746c6d8	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 13:41:10.499-05	2026-05-11 13:48:19.176-05	428.677	0	67	7992	3167338	97294	57	7.175675	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a8752b118e746c6d8.jsonl
agent-a8912463fc8dc3338	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 19:10:57.415-05	2026-05-11 19:52:07.282-05	2469.867	0	98	13304	4895141	348817	88	14.8823	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a8912463fc8dc3338.jsonl
agent-a8dc1c481d57cca52	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 12:22:15.98-05	2026-05-11 12:34:24.796-05	728.816	0	104	15382	5891133	125127	94	12.338041	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a8dc1c481d57cca52.jsonl
agent-a93b6219dcf3323e9	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 11:52:41.317-05	2026-05-11 12:00:46.571-05	485.254	0	87	21034	5142279	168922	77	12.459561	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a93b6219dcf3323e9.jsonl
agent-a9915905856008505	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 10:45:52.877-05	2026-05-11 10:49:05.341-05	192.464	0	42	4206	1547298	95387	32	4.425533	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-a9915905856008505.jsonl
agent-ab44012ed57629787	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 13:22:58.545-05	2026-05-11 13:26:44.507-05	225.962	0	57	5715	2091451	73969	47	4.953575	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-ab44012ed57629787.jsonl
agent-ab89570535404a29f	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 02:02:02.743-05	2026-05-11 02:05:43.187-05	220.444	0	41	4223	1077509	101309	26	3.833147	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-ab89570535404a29f.jsonl
agent-abe80c091457132ac	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 10:40:26.045-05	2026-05-11 10:42:19.295-05	113.25	0	33	4111	949661	67182	23	2.992974	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-abe80c091457132ac.jsonl
agent-acd572658082719ad	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 18:25:02.791-05	2026-05-11 18:28:49.702-05	226.911	0	64	9516	2306070	113444	49	6.30084	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-acd572658082719ad.jsonl
agent-acddaf119684ab02e	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 18:32:32.921-05	2026-05-11 18:47:13.751-05	880.83	0	168	30767	13884781	210454	158	27.083229	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-acddaf119684ab02e.jsonl
agent-acfbb7339a0123676	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 10:23:04.859-05	2026-05-11 10:24:36.183-05	91.324	0	35	3842	787336	89207	20	3.14231	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-acfbb7339a0123676.jsonl
agent-ad24b6d78df6b5266	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 02:10:04.863-05	2026-05-11 02:14:11.104-05	246.241	0	50	4253	1388923	131044	30	4.860184	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-ad24b6d78df6b5266.jsonl
agent-ae2e65cc7801e2211	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 11:26:31.359-05	2026-05-11 11:39:25.864-05	774.505	0	71	8675	2266896	275195	46	9.21194	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-ae2e65cc7801e2211.jsonl
agent-ae785302351924630	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 14:30:53.839-05	2026-05-11 14:38:53.397-05	479.558	0	84	13719	3990676	144325	69	9.722293	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-ae785302351924630.jsonl
agent-ae9d2d1c4eb10a63b	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 02:32:38.378-05	2026-05-11 02:34:06.041-05	87.663	0	35	2619	757097	84397	20	2.915039	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-ae9d2d1c4eb10a63b.jsonl
agent-af0d8c44d28e68442	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 02:22:28.841-05	2026-05-11 02:29:04.96-05	396.119	0	54	5085	1829310	112260	39	5.231025	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-af0d8c44d28e68442.jsonl
agent-af1d5b258e655bfdd	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 14:43:07.683-05	2026-05-11 15:06:17.199-05	1389.516	0	171	29884	12492224	294777	151	26.50927	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-af1d5b258e655bfdd.jsonl
agent-af6811c1e8d612e96	6206d1b3-39eb-45d6-bb1c-f95fd87caeab	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-11 12:38:21.116-05	2026-05-11 12:42:57.489-05	276.373	0	66	8736	2590121	120509	51	6.800915	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6206d1b3-39eb-45d6-bb1c-f95fd87caeab\\subagents\\agent-af6811c1e8d612e96.jsonl
6a45727e-74f2-46bc-9978-71d7484bf876	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-12 13:03:32.112-05	2026-05-12 14:29:50.574-05	5178.462	0	13105	714739	73755131	2702587	400	215.108203	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6a45727e-74f2-46bc-9978-71d7484bf876.jsonl
agent-a4862b3bd68f15eed	6a45727e-74f2-46bc-9978-71d7484bf876	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-12 13:57:47.463-05	2026-05-12 13:57:57.819-05	10.356	0	21	706	65763	56296	5	0.080497	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6a45727e-74f2-46bc-9978-71d7484bf876\\subagents\\agent-a4862b3bd68f15eed.jsonl
6ab25f66-1b4c-4bcf-9991-66a6c23a97c9	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 00:59:06.834-05	2026-05-26 02:43:18.537-05	6251.703	0	163572	833066	68194604	1211077	283	189.93313	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6ab25f66-1b4c-4bcf-9991-66a6c23a97c9.jsonl
agent-a6924086598ea8848	6ab25f66-1b4c-4bcf-9991-66a6c23a97c9	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 01:05:13.783-05	2026-05-26 01:07:44.938-05	151.155	0	323	2266	2500376	163677	59	0.466287	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6ab25f66-1b4c-4bcf-9991-66a6c23a97c9\\subagents\\agent-a6924086598ea8848.jsonl
6ba4bfb7-c623-4ed4-837d-da40b3c69abf	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-22 14:29:15.157-05	2026-05-22 18:12:57.321-05	13422.164	0	36358	1792777	158712383	4281106	449	453.342957	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6ba4bfb7-c623-4ed4-837d-da40b3c69abf.jsonl
717d5904-1f83-4e7d-88e7-7205a02b6adc	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-15 02:32:22.308-05	2026-05-15 12:40:17.667-05	36475.359	0	1018	1523780	509747236	3929208	906	952.592274	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\717d5904-1f83-4e7d-88e7-7205a02b6adc.jsonl
761ac695-3326-4012-a051-76d36752044c	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-22 18:34:41.798-05	2026-05-22 19:19:58.45-05	2716.652	0	13073	377620	17441991	780259	154	69.310438	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\761ac695-3326-4012-a051-76d36752044c.jsonl
762bab6c-8fb2-41d6-a8f0-e6af41508a62	\N	f	claude-opus-4-7	worktree-cheat-cleanup-2	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\cheat-cleanup-2	2026-05-17 01:26:39.626-05	2026-05-17 02:52:49.15-05	5169.524	0	673	567411	128612999	1024647	532	254.69755	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\762bab6c-8fb2-41d6-a8f0-e6af41508a62.jsonl
7fa3bccf-ca49-4687-a0b4-a78b03fb32f8	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 22:47:22.935-05	2026-05-17 23:01:02.936-05	87220.001	0	6252	5197631	2196122262	16260300	4389	3988.980123	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\7fa3bccf-ca49-4687-a0b4-a78b03fb32f8.jsonl
837e755d-8b00-436a-b655-732ad781a41c	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 13:28:18.13-05	2026-05-16 14:44:40.893-05	4582.763	0	464	522533	105620120	1039520	411	217.118115	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\837e755d-8b00-436a-b655-732ad781a41c.jsonl
86752ae6-bf2c-44c4-9f9c-cfbad12ab647	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 00:09:31.574-05	2026-05-26 00:56:31.514-05	2819.94	0	11677	397159	25638950	1256364	194	91.97733	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\86752ae6-bf2c-44c4-9f9c-cfbad12ab647.jsonl
agent-a019c39de058b00ea	86752ae6-bf2c-44c4-9f9c-cfbad12ab647	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 00:12:54.54-05	2026-05-26 00:14:37.157-05	102.617	0	300	7068	1831646	220915	45	0.494948	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\86752ae6-bf2c-44c4-9f9c-cfbad12ab647\\subagents\\agent-a019c39de058b00ea.jsonl
agent-abde28d00945907d9	86752ae6-bf2c-44c4-9f9c-cfbad12ab647	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 00:13:07.134-05	2026-05-26 00:17:10.887-05	243.753	0	178	15892	3346260	83031	77	0.518053	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\86752ae6-bf2c-44c4-9f9c-cfbad12ab647\\subagents\\agent-abde28d00945907d9.jsonl
88a2f7a8-fb43-44e7-9f22-0da9cefd180a	\N	f	claude-opus-4-6	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 04:48:38.212-05	2026-05-26 05:07:25.365-05	1127.153	0	70	146213	5222682	308281	66	24.581317	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\88a2f7a8-fb43-44e7-9f22-0da9cefd180a.jsonl
agent-a67e23d795f42ae38	88a2f7a8-fb43-44e7-9f22-0da9cefd180a	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 04:48:43.967-05	2026-05-26 04:49:06.374-05	22.407	0	8116	832	141496	71009	8	0.115187	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\88a2f7a8-fb43-44e7-9f22-0da9cefd180a\\subagents\\agent-a67e23d795f42ae38.jsonl
8bf6f57f-f743-4999-bee8-8da81f204b28	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-19 01:03:37.298-05	2026-05-19 12:44:10.36-05	42033.062	0	419	326573	36178328	1578909	239	108.371296	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\8bf6f57f-f743-4999-bee8-8da81f204b28.jsonl
91c42a71-8aa2-449d-b800-82353a681033	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-04-28 10:03:49.968-05	2026-05-02 13:32:54.959-05	358144.991	0	11562	8927208	5586217139	31652240	10695	9642.519238	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\91c42a71-8aa2-449d-b800-82353a681033.jsonl
933a254f-4618-4f83-86d9-60e907c20a0b	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-12 19:25:54.304-05	2026-05-13 09:57:33.973-05	52299.669	0	1152	1072762	577249557	1788234	1021	979.878153	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\933a254f-4618-4f83-86d9-60e907c20a0b.jsonl
9550badf-2351-496c-8814-4da15ee8f962	\N	f	claude-opus-4-7	worktree-cheat-cleanup	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\cheat-cleanup	2026-05-20 23:36:29.606-05	2026-05-21 18:50:57.194-05	69267.588	0	236965	4634415	588492389	9849708	1169	1418.556208	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\9550badf-2351-496c-8814-4da15ee8f962.jsonl
96bc30aa-a079-46e1-97e1-09d12e7ecf61	\N	f	claude-opus-4-6	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-04-27 15:45:05.059-05	2026-04-27 18:02:43.923-05	8258.864	0	493	79390	10262708	239815	142	25.852238	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\96bc30aa-a079-46e1-97e1-09d12e7ecf61.jsonl
98513aae-a2f9-41c3-b106-6c612995bfba	\N	f	\N	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-10 22:18:16.845-05	2026-05-10 22:18:17.398-05	0.553	0	0	0	0	0	0	0	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\98513aae-a2f9-41c3-b106-6c612995bfba.jsonl
99e3fd78-6f4b-4361-8b0a-b73911d9fd8d	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 23:31:51.757-05	2026-05-17 13:02:29.621-05	48637.864	0	12510	1622203	445727509	5094221	953	885.960782	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\99e3fd78-6f4b-4361-8b0a-b73911d9fd8d.jsonl
agent-af1daf32cc2bfd38d	99e3fd78-6f4b-4361-8b0a-b73911d9fd8d	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 23:34:24.955-05	2026-05-16 23:41:54.562-05	449.607	0	197	23043	6056932	1754795	117	43.718984	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\99e3fd78-6f4b-4361-8b0a-b73911d9fd8d\\subagents\\agent-af1daf32cc2bfd38d.jsonl
9b42958e-af8d-46b0-b2b6-bb1944af646a	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 00:42:46.282-05	2026-05-17 01:25:23.869-05	2557.587	0	323	155097	31255293	512811	238	68.135266	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\9b42958e-af8d-46b0-b2b6-bb1944af646a.jsonl
9d4eb689-2d0c-4c44-b678-474fbaa4520c	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-03 16:41:30.408-05	2026-05-03 19:24:59.248-05	9808.84	0	102	30690	2039288	284166	42	10.690324	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\9d4eb689-2d0c-4c44-b678-474fbaa4520c.jsonl
agent-a39d7f2246250231b	9d4eb689-2d0c-4c44-b678-474fbaa4520c	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-03 18:37:39.572-05	2026-05-03 19:24:01.407-05	2781.835	0	208	49387	19795016	266327	198	38.3933	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\9d4eb689-2d0c-4c44-b678-474fbaa4520c\\subagents\\agent-a39d7f2246250231b.jsonl
agent-a73c7dcbb36bd4f7c	9d4eb689-2d0c-4c44-b678-474fbaa4520c	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-03 16:47:55.88-05	2026-05-03 18:26:14.579-05	5898.699	0	408	118809	66550504	472465	398	117.60127	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\9d4eb689-2d0c-4c44-b678-474fbaa4520c\\subagents\\agent-a73c7dcbb36bd4f7c.jsonl
agent-aa0de6a6827238990	9d4eb689-2d0c-4c44-b678-474fbaa4520c	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-03 18:30:29.255-05	2026-05-03 18:36:49.848-05	380.593	0	43	6991	1482971	103399	33	4.688158	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\9d4eb689-2d0c-4c44-b678-474fbaa4520c\\subagents\\agent-aa0de6a6827238990.jsonl
a5d50fee-8ab8-496b-890b-9c463980be5b	\N	f	claude-sonnet-4-6	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 03:47:13.738-05	2026-05-26 04:07:10.743-05	1197.005	0	41	199399	2418642	267270	38	4.718963	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\a5d50fee-8ab8-496b-890b-9c463980be5b.jsonl
a612bf1c-4faf-4895-b8d1-26025b72e439	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-15 22:18:08.897-05	2026-05-16 01:27:39.911-05	11371.014	0	1614	772836	181739676	2329998	593	374.283886	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\a612bf1c-4faf-4895-b8d1-26025b72e439.jsonl
aaa398c3-8cfc-4ec7-ada5-d0994b47a279	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-02 22:04:35.933-05	2026-05-03 16:41:27.493-05	67011.56	0	434	518578	67028923	1667212	339	170.70347	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\aaa398c3-8cfc-4ec7-ada5-d0994b47a279.jsonl
agent-a025438b3433addb8	aaa398c3-8cfc-4ec7-ada5-d0994b47a279	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-03 10:41:39.517-05	2026-05-03 11:44:04.161-05	3744.644	0	304	68752	31790299	446258	284	61.213746	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\aaa398c3-8cfc-4ec7-ada5-d0994b47a279\\subagents\\agent-a025438b3433addb8.jsonl
agent-a2255bc6a9567253a	aaa398c3-8cfc-4ec7-ada5-d0994b47a279	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-02 23:25:37.9-05	2026-05-03 00:06:41.575-05	2463.675	0	198	42422	18630951	854012	188	47.143771	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\aaa398c3-8cfc-4ec7-ada5-d0994b47a279\\subagents\\agent-a2255bc6a9567253a.jsonl
agent-a520a13b86295782e	aaa398c3-8cfc-4ec7-ada5-d0994b47a279	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-02 22:08:19.992-05	2026-05-02 22:45:11.589-05	2211.597	0	141	41695	10908780	271894	126	24.590422	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\aaa398c3-8cfc-4ec7-ada5-d0994b47a279\\subagents\\agent-a520a13b86295782e.jsonl
agent-a5614ee58c73e552d	aaa398c3-8cfc-4ec7-ada5-d0994b47a279	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-03 11:46:25.476-05	2026-05-03 13:21:57.528-05	5732.052	0	515	151052	96966102	872646	505	173.14789	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\aaa398c3-8cfc-4ec7-ada5-d0994b47a279\\subagents\\agent-a5614ee58c73e552d.jsonl
agent-abc8e330972649f4b	aaa398c3-8cfc-4ec7-ada5-d0994b47a279	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-03 00:11:00.165-05	2026-05-03 00:25:00.425-05	840.26	0	44	5260	1441350	84216	34	4.136235	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\aaa398c3-8cfc-4ec7-ada5-d0994b47a279\\subagents\\agent-abc8e330972649f4b.jsonl
agent-ad34027cf39476b73	aaa398c3-8cfc-4ec7-ada5-d0994b47a279	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-02 22:48:31.632-05	2026-05-02 23:12:15.967-05	1424.335	0	75	10760	2779426	290410	55	10.422451	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\aaa398c3-8cfc-4ec7-ada5-d0994b47a279\\subagents\\agent-ad34027cf39476b73.jsonl
agent-af8c1bdc1fbd865c1	aaa398c3-8cfc-4ec7-ada5-d0994b47a279	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-03 00:35:07.632-05	2026-05-03 10:36:56.275-05	36108.643	0	106	12087	3157051	254393	71	10.41356	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\aaa398c3-8cfc-4ec7-ada5-d0994b47a279\\subagents\\agent-af8c1bdc1fbd865c1.jsonl
b051d212-8ed9-4c39-a40a-047df874bc51	\N	f	claude-opus-4-7	worktree-cheat-cleanup-2026-05-17	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\cheat-cleanup-2026-05-17	2026-05-17 21:40:48.062-05	2026-05-18 01:11:24.762-05	12636.7	0	448	393089	69094351	1168907	371	155.046928	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\b051d212-8ed9-4c39-a40a-047df874bc51.jsonl
b94009bd-8f01-45f6-a211-9aa67c9c20ec	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 02:12:13.923-05	2026-05-17 03:01:06.287-05	2932.364	0	676	118137	20571966	346888	168	46.232514	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\b94009bd-8f01-45f6-a211-9aa67c9c20ec.jsonl
b954b3dc-33df-49c9-9fd4-d604b34f7032	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 22:29:15.019-05	2026-05-17 00:41:56.952-05	7961.933	0	783	747972	156383061	1403018	553	316.990824	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\b954b3dc-33df-49c9-9fd4-d604b34f7032.jsonl
agent-aa6b3839d623841d2	b954b3dc-33df-49c9-9fd4-d604b34f7032	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aa6b3839d623841d2	2026-05-16 22:33:32.88-05	2026-05-16 23:01:49.975-05	1697.095	0	135	31254	11135020	241029	117	23.567899	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\b954b3dc-33df-49c9-9fd4-d604b34f7032\\subagents\\agent-aa6b3839d623841d2.jsonl
agent-aec6111e8c15b74e0	b954b3dc-33df-49c9-9fd4-d604b34f7032	t	claude-opus-4-7	worktree-agent-aec6111e8c15b74e0	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aec6111e8c15b74e0	2026-05-16 22:33:25.615-05	2026-05-16 23:13:37.629-05	2412.014	0	261	82281	33881801	370534	243	63.945204	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\b954b3dc-33df-49c9-9fd4-d604b34f7032\\subagents\\agent-aec6111e8c15b74e0.jsonl
baa732cd-f1b1-4e24-9990-cf631fb19baf	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-05 23:02:41.774-05	2026-05-06 17:46:28.621-05	67426.847	0	1899	1411303	795542242	3836060	1555	1371.115698	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\baa732cd-f1b1-4e24-9990-cf631fb19baf.jsonl
bb5a007f-f7e6-4cef-86f4-8aecb2b1460d	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 22:29:17.606-05	2026-05-17 00:40:02.833-05	7845.227	0	6800	916272	134936566	1999129	568	308.710918	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\bb5a007f-f7e6-4cef-86f4-8aecb2b1460d.jsonl
agent-ad97f5335148102a8	bb5a007f-f7e6-4cef-86f4-8aecb2b1460d	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ad97f5335148102a8	2026-05-16 22:43:01.917-05	2026-05-16 23:12:38.497-05	1776.58	0	194	46278	18573399	608153	156	42.736727	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\bb5a007f-f7e6-4cef-86f4-8aecb2b1460d\\subagents\\agent-ad97f5335148102a8.jsonl
c04daf29-5b2d-4a19-9611-84cfe21f1a0d	\N	f	claude-opus-4-7	worktree-decomp-3	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\decomp-3	2026-05-20 22:02:19.268-05	2026-05-21 19:19:44.434-05	76645.166	0	2339	4615916	563462105	18434321	1220	1537.065461	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\c04daf29-5b2d-4a19-9611-84cfe21f1a0d.jsonl
c1b6031c-4d21-4963-a29b-699fed5114f5	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 11:27:02.461-05	2026-05-16 11:51:46.257-05	1483.796	0	200	115600	20871033	398567	167	47.452681	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\c1b6031c-4d21-4963-a29b-699fed5114f5.jsonl
c1de0e4a-7f94-4bde-97bd-969a76a6dedf	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-21 00:29:11.634-05	2026-05-21 13:31:54.273-05	46962.639	0	35654	1831764	298978727	4194294	671	665.028213	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\c1de0e4a-7f94-4bde-97bd-969a76a6dedf.jsonl
c36ba981-33ec-481e-a0d3-dd668fbd777b	\N	f	claude-opus-4-6	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 04:28:02.197-05	2026-05-26 04:45:02.43-05	1020.233	0	72	101526	4947452	260615	69	19.923239	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\c36ba981-33ec-481e-a0d3-dd668fbd777b.jsonl
c7b15847-3f15-4a09-a560-6599f53617b8	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-12 14:33:06.894-05	2026-05-12 16:36:06.004-05	7379.11	0	909	724263	219310163	1111983	721	404.148286	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\c7b15847-3f15-4a09-a560-6599f53617b8.jsonl
c7de032e-f7c0-410e-80fe-701b872d04cd	\N	f	claude-opus-4-7	worktree-cheat-cleanup-side	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\cheat-cleanup-side	2026-05-18 12:21:16.42-05	2026-05-18 23:09:31.796-05	38895.376	0	9822	1961680	679553270	11061360	1342	1374.003735	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\c7de032e-f7c0-410e-80fe-701b872d04cd.jsonl
ca0c5f6e-05b1-4771-a9d8-a9c58d0335f5	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-12 16:36:38.624-05	2026-05-12 19:22:57.154-05	9978.53	0	891	724616	276710285	2206188	716	510.791018	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\ca0c5f6e-05b1-4771-a9d8-a9c58d0335f5.jsonl
cac0b3d8-cc98-4ac3-8b5b-7b571cb53ebc	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-04-27 18:02:46.895-05	2026-04-28 02:51:06.994-05	31700.099	0	108727	1338712	647489676	4284759	1328	1153.60805	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\cac0b3d8-cc98-4ac3-8b5b-7b571cb53ebc.jsonl
agent-a7ba1dba7b5b4beb6	cac0b3d8-cc98-4ac3-8b5b-7b571cb53ebc	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-04-27 18:09:26.558-05	2026-04-27 18:12:32.849-05	186.291	0	58	4492	1083946	32778	36	0.171885	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\cac0b3d8-cc98-4ac3-8b5b-7b571cb53ebc\\subagents\\agent-a7ba1dba7b5b4beb6.jsonl
agent-a9c20e2df1fef9f9f	cac0b3d8-cc98-4ac3-8b5b-7b571cb53ebc	t	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-04-27 18:09:14.71-05	2026-04-27 18:11:54.732-05	160.022	0	1351	8946	2298233	214899	60	0.544528	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\cac0b3d8-cc98-4ac3-8b5b-7b571cb53ebc\\subagents\\agent-a9c20e2df1fef9f9f.jsonl
cc99d47c-762c-43df-8de0-3563f1b9a58f	\N	f	\N	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 02:30:34.078-05	2026-05-17 02:31:17.485-05	43.407	0	0	0	0	0	0	0	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\cc99d47c-762c-43df-8de0-3563f1b9a58f.jsonl
cceefb07-b757-4e08-9853-dce9ca281796	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-12 01:45:13.606-05	2026-05-12 13:00:50.789-05	40537.183	0	1999	1673697	871510704	6425256	1755	1553.296866	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\cceefb07-b757-4e08-9853-dce9ca281796.jsonl
d025390c-ec16-4624-b628-3f6e346eca92	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-17 12:56:57.453-05	2026-05-17 13:36:29.82-05	2372.367	0	40736	313062	41224290	2247936	220	128.075925	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\d025390c-ec16-4624-b628-3f6e346eca92.jsonl
d9c90c0d-d579-4d3d-83a5-398545f4d37a	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-13 09:57:45.424-05	2026-05-13 23:51:24.515-05	50019.091	0	3154	3600142	1318156711	11960179	2675	2471.546383	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\d9c90c0d-d579-4d3d-83a5-398545f4d37a.jsonl
db4d86c3-eb9d-48ae-b424-6e799323bfb8	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\self-decomp-2	2026-05-17 01:26:28.884-05	2026-05-17 02:49:30.295-05	4981.411	0	562	350249	101615472	1012801	439	197.690332	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\db4d86c3-eb9d-48ae-b424-6e799323bfb8.jsonl
ea65e4aa-5d66-49f2-8f7d-14bd0dc65cb9	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-18 02:04:00.781-05	2026-05-20 22:12:33.869-05	245313.088	0	5620	3180006	2378726986	19708942	4656	4176.217892	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\ea65e4aa-5d66-49f2-8f7d-14bd0dc65cb9.jsonl
ed36cf34-dc86-430a-99e9-6eb65998a8fc	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-10 22:18:42.566-05	2026-05-11 01:01:39.373-05	9776.807	0	832	602419	188050892	912442	587	344.378531	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\ed36cf34-dc86-430a-99e9-6eb65998a8fc.jsonl
f134ece2-499f-4d33-b9af-e5dd8f2e9b75	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 14:19:08.608-05	2026-05-16 16:55:15.043-05	9366.435	0	14757	1446813	355617306	2817273	747	694.982158	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f134ece2-499f-4d33-b9af-e5dd8f2e9b75.jsonl
f161c896-bb9c-47a3-82e7-ce1edb9d0660	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-03 19:25:02.226-05	2026-05-05 23:01:16.75-05	185774.524	0	4431	2606342	917542785	25239507	1909	2045.097049	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660.jsonl
agent-a04966ec7dc78b260	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a893ae0a4024eae79	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a04966ec7dc78b260	2026-05-04 03:39:37.198-05	2026-05-04 04:29:25.243-05	2988.045	0	417	128683	73634673	640044	402	132.110314	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a04966ec7dc78b260.jsonl
agent-a07537e7158f17a5e	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-aaba90cb096bcc725	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a07537e7158f17a5e	2026-05-04 19:06:45.928-05	2026-05-04 19:35:05.856-05	1699.928	0	226	40054	19912157	416303	206	40.681357	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a07537e7158f17a5e.jsonl
agent-a1167ed6c7a84f31e	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a1167ed6c7a84f31e	2026-05-04 14:39:29.441-05	2026-05-04 16:19:07.132-05	5977.691	0	468	117987	77667522	920409	376	142.614997	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a1167ed6c7a84f31e.jsonl
agent-a1256375685a70e83	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a17410603a0cbf784	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a1256375685a70e83	2026-05-03 23:13:11.763-05	2026-05-03 23:38:03.907-05	1492.144	0	151	30317	10513155	445021	131	26.389916	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a1256375685a70e83.jsonl
agent-a13b8bcf1eef3ee9e	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a13b8bcf1eef3ee9e	2026-05-03 21:23:35.89-05	2026-05-03 22:20:24.762-05	3408.872	0	265	70871	30194138	838076	245	66.324432	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a13b8bcf1eef3ee9e.jsonl
agent-a17410603a0cbf784	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a17410603a0cbf784	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a17410603a0cbf784	2026-05-03 22:38:19.463-05	2026-05-03 23:24:27.657-05	2768.194	0	232	61256	26200747	299196	222	49.508725	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a17410603a0cbf784.jsonl
agent-a1b4757524c382267	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a50bcdd0c4fefb2d8	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a1b4757524c382267	2026-05-05 07:05:24.408-05	2026-05-05 07:33:43.99-05	1699.582	0	136	35628	12776409	426360	121	29.833004	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a1b4757524c382267.jsonl
agent-a1b4cd599cb3575e6	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a1b4cd599cb3575e6	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a1b4cd599cb3575e6	2026-05-03 23:47:11.322-05	2026-05-04 00:28:07.055-05	2455.733	0	183	40066	14422533	637879	163	36.601726	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a1b4cd599cb3575e6.jsonl
agent-a1d50d54d855a6bc6	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-aff2eee49a3bca3ae	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a1d50d54d855a6bc6	2026-05-03 22:34:22.436-05	2026-05-03 23:00:13.201-05	1550.765	0	158	37527	13420745	205083	148	26.793319	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a1d50d54d855a6bc6.jsonl
agent-a1f83f94e4111fa98	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a1f83f94e4111fa98	2026-05-04 15:18:38.673-05	2026-05-04 15:59:13.239-05	2434.566	0	280	84006	46010765	710850	270	88.649235	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a1f83f94e4111fa98.jsonl
agent-a224a85a1bcec26eb	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a224a85a1bcec26eb	2026-05-04 08:43:35.997-05	2026-05-04 09:29:25.486-05	2749.489	0	236	76927	33141697	410980	226	63.191485	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a224a85a1bcec26eb.jsonl
agent-a273fe8ec03271e70	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a273fe8ec03271e70	2026-05-04 16:22:38.665-05	2026-05-04 18:50:27.362-05	8868.697	0	224	40310	18511534	1335287	195	55.830542	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a273fe8ec03271e70.jsonl
agent-a2800bafb8877d8ff	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a50bcdd0c4fefb2d8	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a2800bafb8877d8ff	2026-05-05 06:15:22.394-05	2026-05-05 06:16:34.467-05	72.073	0	27	2421	480220	97771	17	2.735516	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a2800bafb8877d8ff.jsonl
agent-a298cf9945d227e90	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a93fc04fa636825dd	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a298cf9945d227e90	2026-05-04 01:33:09.768-05	2026-05-04 01:34:29.992-05	80.224	0	8819	3376	851194	144529	23	4.372195	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a298cf9945d227e90.jsonl
agent-a2ba5c67bae0b936d	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a2ba5c67bae0b936d	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a2ba5c67bae0b936d	2026-05-03 22:38:34.905-05	2026-05-03 23:15:16.53-05	2201.625	0	294	64928	35027444	659540	274	69.781551	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a2ba5c67bae0b936d.jsonl
agent-a30f96da22fb4dc83	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a1167ed6c7a84f31e	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a30f96da22fb4dc83	2026-05-04 15:17:14.544-05	2026-05-04 15:18:21.493-05	66.949	0	22	1348	327808	36464	12	1.276842	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a30f96da22fb4dc83.jsonl
agent-a320f4566584996f1	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a5697667cff09b93e	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a320f4566584996f1	2026-05-04 06:15:20.805-05	2026-05-04 07:06:38.514-05	3077.709	0	222	53916	26910799	881594	198	60.943116	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a320f4566584996f1.jsonl
agent-a33d3b4d813bae9e4	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a893ae0a4024eae79	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a33d3b4d813bae9e4	2026-05-04 03:39:01.744-05	2026-05-04 04:32:50.399-05	3228.655	0	482	123668	91359262	731556	452	160.037898	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a33d3b4d813bae9e4.jsonl
agent-a3608e486b9ad6964	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a1b4cd599cb3575e6	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a3608e486b9ad6964	2026-05-03 23:47:32.698-05	2026-05-04 00:27:59.671-05	2426.973	0	293	71323	36689714	530736	278	70.339491	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a3608e486b9ad6964.jsonl
agent-a3649bdd9394458be	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a93fc04fa636825dd	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a3649bdd9394458be	2026-05-04 00:29:58.075-05	2026-05-04 00:49:36.518-05	1178.443	0	128	33031	10194733	238283	118	22.239151	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a3649bdd9394458be.jsonl
agent-a36c7cb4b75a77575	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a5697667cff09b93e	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a36c7cb4b75a77575	2026-05-04 06:45:09.051-05	2026-05-04 06:57:39.149-05	750.098	0	84	34753	5738546	213380	75	15.216429	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a36c7cb4b75a77575.jsonl
agent-a37336621adaa378d	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a5df5c593fdc359db	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a37336621adaa378d	2026-05-05 01:13:10.392-05	2026-05-05 02:27:49.76-05	4479.368	0	584	158464	126833066	996946	554	220.835896	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a37336621adaa378d.jsonl
agent-a3b106928564d1d6d	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a3b106928564d1d6d	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a3b106928564d1d6d	2026-05-04 04:42:39.004-05	2026-05-04 05:49:42.301-05	4023.297	0	456	106269	78517008	534954	431	135.782915	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a3b106928564d1d6d.jsonl
agent-a3d45429dd686412a	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a3d45429dd686412a	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a3d45429dd686412a	2026-05-04 11:00:03.479-05	2026-05-04 11:38:23.69-05	2300.211	0	229	71309	27737264	551019	215	57.289112	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a3d45429dd686412a.jsonl
agent-a3f943b9bfe2b3bda	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a3f943b9bfe2b3bda	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a3f943b9bfe2b3bda	2026-05-04 19:36:11.154-05	2026-05-04 22:01:58.025-05	8746.871	0	663	185670	152494460	2401723	653	287.709191	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a3f943b9bfe2b3bda.jsonl
agent-a4415b1f43d8f9acf	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a91ec71f7ed9ba81e	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a4415b1f43d8f9acf	2026-05-04 00:41:13.763-05	2026-05-04 01:23:24.76-05	2530.997	0	257	86848	31769495	656072	237	66.473048	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a4415b1f43d8f9acf.jsonl
agent-a48427dad19298804	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a48427dad19298804	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a48427dad19298804	2026-05-04 23:11:02.923-05	2026-05-05 01:09:17.225-05	7094.302	0	863	192181	240091653	1764342	853	407.645412	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a48427dad19298804.jsonl
agent-a4a7c15da11821195	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a4a7c15da11821195	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a4a7c15da11821195	2026-05-04 13:47:51.711-05	2026-05-04 15:16:25.803-05	5314.092	0	331	105960	51908525	2094904	316	125.094202	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a4a7c15da11821195.jsonl
agent-a4ab58864c7af8e27	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a4ab58864c7af8e27	2026-05-04 03:39:10.556-05	2026-05-04 04:08:28.987-05	1758.431	0	177	47453	15619498	462238	157	35.65784	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a4ab58864c7af8e27.jsonl
agent-a50bcdd0c4fefb2d8	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a50bcdd0c4fefb2d8	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a50bcdd0c4fefb2d8	2026-05-05 03:45:08.051-05	2026-05-05 06:04:04.052-05	8336.001	0	4556	127404	94689994	972265	478	169.8886	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a50bcdd0c4fefb2d8.jsonl
agent-a5697667cff09b93e	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a5697667cff09b93e	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a5697667cff09b93e	2026-05-04 05:50:53.048-05	2026-05-04 06:57:40.345-05	4007.297	0	381	103188	54881862	684882	352	102.909145	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a5697667cff09b93e.jsonl
agent-a5df5c593fdc359db	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a5df5c593fdc359db	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a5df5c593fdc359db	2026-05-05 01:09:41.291-05	2026-05-05 03:44:02.4-05	9261.109	0	546	144425	109288168	3320330	526	237.028504	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a5df5c593fdc359db.jsonl
agent-a674ec951e8aa97ae	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a8a79ce70a3a9fb3a	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a674ec951e8aa97ae	2026-05-04 04:42:32.088-05	2026-05-04 06:14:36.248-05	5524.16	0	751	192531	185982710	856669	726	309.487699	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a674ec951e8aa97ae.jsonl
agent-a6cc36c3f76f56566	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-afcbf42269029714a	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a6cc36c3f76f56566	2026-05-04 08:43:19.953-05	2026-05-04 09:53:46.623-05	4226.67	0	319	91037	57205484	920913	299	109.907905	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a6cc36c3f76f56566.jsonl
agent-a6d446097c7835811	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-ac6edaafff0c74ed1	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a6d446097c7835811	2026-05-04 13:48:00.988-05	2026-05-04 14:28:31.813-05	2430.825	0	213	65016	26719530	544562	203	55.169227	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a6d446097c7835811.jsonl
agent-a72b8f4a60360556c	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a72b8f4a60360556c	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a72b8f4a60360556c	2026-05-05 02:55:59.841-05	2026-05-05 05:50:55.321-05	10495.48	0	7000	104155	73614242	3072697	383	175.951057	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a72b8f4a60360556c.jsonl
agent-a7aa225c1edb2807a	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a50bcdd0c4fefb2d8	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a7aa225c1edb2807a	2026-05-05 06:15:18.01-05	2026-05-05 06:17:04.079-05	106.069	0	33	2698	984365	126478	23	4.050855	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a7aa225c1edb2807a.jsonl
agent-a7d5ed266527aaec5	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a3f943b9bfe2b3bda	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a7d5ed266527aaec5	2026-05-04 21:08:37.308-05	2026-05-04 22:23:08.303-05	4470.995	0	562	167740	126532145	1022673	542	221.562266	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a7d5ed266527aaec5.jsonl
agent-a7d68df27c67a8722	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a7d68df27c67a8722	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a7d68df27c67a8722	2026-05-05 03:33:32.509-05	2026-05-05 08:57:14.76-05	19422.251	0	905	244364	283499037	6392785	873	563.454149	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a7d68df27c67a8722.jsonl
agent-a7df5816f649ef465	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a7df5816f649ef465	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a7df5816f649ef465	2026-05-03 21:23:13.329-05	2026-05-03 22:23:08.445-05	3595.116	0	449	148583	61837811	539919	383	114.030658	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a7df5816f649ef465.jsonl
agent-a7ef250bc33d5fc3d	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-afcbf42269029714a	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a7ef250bc33d5fc3d	2026-05-04 08:43:30.573-05	2026-05-04 09:48:23.771-05	3893.198	0	366	122930	76823875	574179	356	135.226909	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a7ef250bc33d5fc3d.jsonl
agent-a8408302fa4b091f0	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a50bcdd0c4fefb2d8	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a8408302fa4b091f0	2026-05-05 06:17:35.429-05	2026-05-05 07:05:03.533-05	2848.104	0	428	108696	74218835	908215	408	136.515904	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a8408302fa4b091f0.jsonl
agent-a893ae0a4024eae79	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a893ae0a4024eae79	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a893ae0a4024eae79	2026-05-04 03:39:28.195-05	2026-05-04 04:39:25.261-05	3597.066	0	392	116512	68208521	844304	382	126.887761	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a893ae0a4024eae79.jsonl
agent-a8a79ce70a3a9fb3a	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a8a79ce70a3a9fb3a	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a8a79ce70a3a9fb3a	2026-05-04 05:12:53.85-05	2026-05-04 06:15:44.649-05	3770.799	0	283	68493	35376545	814435	263	73.476694	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a8a79ce70a3a9fb3a.jsonl
agent-a8f1dde4a81e8d0d1	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a8f1dde4a81e8d0d1	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a8f1dde4a81e8d0d1	2026-05-04 05:13:27.851-05	2026-05-04 06:31:29.471-05	4681.62	0	537	150908	119116308	611022	527	201.457279	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a8f1dde4a81e8d0d1.jsonl
f6e6b474-a735-4fd5-b5a1-f6522cbeb4cb	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-02 13:34:57.197-05	2026-05-02 22:03:45.695-05	30528.498	0	38101	1482022	677127053	21119416	1242	1523.402795	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f6e6b474-a735-4fd5-b5a1-f6522cbeb4cb.jsonl
agent-a8fb32b658fae1e05	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a3f943b9bfe2b3bda	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a8fb32b658fae1e05	2026-05-04 22:02:25.197-05	2026-05-04 22:23:04.483-05	1239.286	0	98	22505	6585373	166913	88	14.697023	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a8fb32b658fae1e05.jsonl
agent-a918fa85b6bb94868	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a918fa85b6bb94868	2026-05-04 23:56:15.667-05	2026-05-05 00:34:32.013-05	2296.346	0	219	61382	22927801	813673	199	54.255005	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a918fa85b6bb94868.jsonl
agent-a91ec71f7ed9ba81e	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a93fc04fa636825dd	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a91ec71f7ed9ba81e	2026-05-04 01:03:31.83-05	2026-05-04 01:31:44.156-05	1692.326	0	137	27325	10605104	429906	127	26.019824	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a91ec71f7ed9ba81e.jsonl
agent-a9315e1babb851f46	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a93fc04fa636825dd	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a9315e1babb851f46	2026-05-04 01:24:42.808-05	2026-05-04 01:34:35.213-05	592.405	0	98	22588	6887402	216914	89	16.093811	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a9315e1babb851f46.jsonl
agent-a93fc04fa636825dd	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a93fc04fa636825dd	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a93fc04fa636825dd	2026-05-04 00:29:47.831-05	2026-05-04 01:36:27.601-05	3999.77	0	320	99777	47873582	475404	306	88.212273	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a93fc04fa636825dd.jsonl
agent-a94678623392fe9b4	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a7d68df27c67a8722	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a94678623392fe9b4	2026-05-05 06:17:14.554-05	2026-05-05 08:46:04.354-05	8929.8	0	384	95773	59385627	2894875	369	150.546082	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a94678623392fe9b4.jsonl
agent-a952b5687479c531d	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a93fc04fa636825dd	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a952b5687479c531d	2026-05-04 01:01:45.209-05	2026-05-04 01:38:01.009-05	2175.8	0	239	63941	25801370	696464	220	56.559915	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a952b5687479c531d.jsonl
agent-a9807e0af174aea9c	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a8a79ce70a3a9fb3a	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-04 05:12:59.552-05	2026-05-04 06:17:14.495-05	3854.943	0	7957	112961	68545676	903044	379	128.342019	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a9807e0af174aea9c.jsonl
agent-a990ff5dfbd11ae20	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-aa292b3cc5a8eda11	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a990ff5dfbd11ae20	2026-05-04 04:42:25.955-05	2026-05-04 05:05:14.901-05	1368.946	0	133	21254	8921483	246293	118	19.596263	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a990ff5dfbd11ae20.jsonl
agent-a9a1e29f459c6f102	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a5697667cff09b93e	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a9a1e29f459c6f102	2026-05-04 06:45:19.877-05	2026-05-04 06:57:12.152-05	712.275	0	91	29469	5962624	195796	82	14.826651	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-a9a1e29f459c6f102.jsonl
agent-aa0848797fb5fb8a7	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a5df5c593fdc359db	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aa0848797fb5fb8a7	2026-05-05 00:35:28.547-05	2026-05-05 01:25:00.989-05	2972.442	0	193	35279	15733635	600591	173	37.510354	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aa0848797fb5fb8a7.jsonl
agent-aa12f97e3612d8266	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a893ae0a4024eae79	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aa12f97e3612d8266	2026-05-04 04:30:44.652-05	2026-05-04 04:32:06.42-05	81.768	0	29	2223	556021	43021	19	1.807835	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aa12f97e3612d8266.jsonl
agent-aa292b3cc5a8eda11	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-aa292b3cc5a8eda11	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aa292b3cc5a8eda11	2026-05-04 03:39:21.572-05	2026-05-04 04:46:59.355-05	4057.783	0	496	107982	83987005	753648	481	148.217498	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aa292b3cc5a8eda11.jsonl
agent-aa3500a4305a044d6	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aa3500a4305a044d6	2026-05-04 21:42:39.071-05	2026-05-05 00:01:48.346-05	8349.275	0	1214	234983	399014643	1055758	1199	635.959362	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aa3500a4305a044d6.jsonl
agent-aa427956be342a8f2	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-aaf02039f5f672192	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aa427956be342a8f2	2026-05-04 11:00:13.589-05	2026-05-04 11:41:26.619-05	2473.03	0	213	66378	26585980	327761	204	51.006034	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aa427956be342a8f2.jsonl
agent-aa47fdac2aa4ba683	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aa47fdac2aa4ba683	2026-05-03 20:48:24.132-05	2026-05-03 21:10:12.44-05	1308.308	0	134	30356	9100812	293175	114	21.426959	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aa47fdac2aa4ba683.jsonl
agent-aaaa1c1f6cfb3186b	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aaaa1c1f6cfb3186b	2026-05-04 13:47:56.322-05	2026-05-04 15:51:59.379-05	7443.057	0	654	180049	166085646	2113429	644	302.268748	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aaaa1c1f6cfb3186b.jsonl
agent-aaba90cb096bcc725	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a3f943b9bfe2b3bda	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aaba90cb096bcc725	2026-05-04 19:06:51.916-05	2026-05-04 21:42:09.925-05	9318.009	0	823	210762	224672325	3071271	808	410.414314	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aaba90cb096bcc725.jsonl
agent-aabd8c366a9b60e80	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-aa3500a4305a044d6	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aabd8c366a9b60e80	2026-05-04 23:11:13.186-05	2026-05-04 23:55:22.62-05	2649.434	0	169	30392	12654912	391610	154	28.60699	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aabd8c366a9b60e80.jsonl
agent-aaf02039f5f672192	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-aaf02039f5f672192	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aaf02039f5f672192	2026-05-04 11:02:02.108-05	2026-05-04 11:38:26.172-05	2184.064	0	178	41911	19575021	305654	169	38.239539	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aaf02039f5f672192.jsonl
agent-ab0067a4b1842fe28	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a93fc04fa636825dd	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ab0067a4b1842fe28	2026-05-04 01:16:10.627-05	2026-05-04 01:34:33.698-05	1103.071	0	167	42330	14865644	232510	158	29.835283	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ab0067a4b1842fe28.jsonl
agent-ab34b6cae2fe39cea	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a93fc04fa636825dd	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ab34b6cae2fe39cea	2026-05-03 23:23:43.837-05	2026-05-04 00:39:03.327-05	4519.49	0	259	57718	28327378	412510	244	54.558365	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ab34b6cae2fe39cea.jsonl
agent-ab34e5dad77cb32dc	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-ab34e5dad77cb32dc	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ab34e5dad77cb32dc	2026-05-04 16:32:44.855-05	2026-05-04 18:51:46.987-05	8342.132	0	163	31401	10280407	727615	144	31.420912	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ab34e5dad77cb32dc.jsonl
agent-ac0ca789b097d7aef	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-aaba90cb096bcc725	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ac0ca789b097d7aef	2026-05-04 19:36:04.012-05	2026-05-04 21:07:57.333-05	5513.321	0	1175	200591	156409509	917855	614	266.885995	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ac0ca789b097d7aef.jsonl
agent-ac345e48614530d0f	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a4415b1f43d8f9acf	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ac345e48614530d0f	2026-05-03 23:13:01.462-05	2026-05-04 01:05:26.978-05	6745.516	0	572	166113	121742999	2497714	562	241.913691	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ac345e48614530d0f.jsonl
agent-ac43c5f022522822e	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ac43c5f022522822e	2026-05-04 14:39:34.072-05	2026-05-04 15:24:02.937-05	2668.865	0	348	91115	51961687	911798	328	101.877588	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ac43c5f022522822e.jsonl
agent-ac6edaafff0c74ed1	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ac6edaafff0c74ed1	2026-05-04 13:47:47.182-05	2026-05-04 16:32:17.961-05	9870.779	0	1168	320415	434485816	1445669	1148	702.883663	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ac6edaafff0c74ed1.jsonl
agent-acc945e72c72c4ba4	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a7d68df27c67a8722	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-acc945e72c72c4ba4	2026-05-05 07:34:27.88-05	2026-05-05 08:53:43.601-05	4755.721	0	417	102629	78914449	2062771	398	164.75206	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-acc945e72c72c4ba4.jsonl
agent-ad04c16a27de53ccc	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-ac6edaafff0c74ed1	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ad04c16a27de53ccc	2026-05-04 13:48:05.515-05	2026-05-04 14:38:12.019-05	3006.504	0	348	102442	59676171	549918	328	107.513589	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ad04c16a27de53ccc.jsonl
agent-ad4f9aa87e9ba6681	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-aa292b3cc5a8eda11	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ad4f9aa87e9ba6681	2026-05-04 04:09:39.994-05	2026-05-04 05:11:19.404-05	3699.41	0	1704	105539	66166251	1335318	382	132.227574	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ad4f9aa87e9ba6681.jsonl
agent-ad622601aa846790b	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ad622601aa846790b	2026-05-03 20:29:33.219-05	2026-05-03 21:10:12.15-05	2438.931	0	327	97047	48950339	630068	307	92.522713	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ad622601aa846790b.jsonl
agent-ad7d821f3f99b4112	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-afcbf42269029714a	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ad7d821f3f99b4112	2026-05-04 08:43:25.269-05	2026-05-04 10:29:04.569-05	6339.3	0	506	114429	100620894	1116721	486	180.459625	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ad7d821f3f99b4112.jsonl
agent-ae02c5c9f900bceda	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a48427dad19298804	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ae02c5c9f900bceda	2026-05-05 00:17:24.52-05	2026-05-05 01:12:01.051-05	3276.531	0	540	124832	99085831	856717	520	174.06269	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ae02c5c9f900bceda.jsonl
agent-ae1f4fc313a51e558	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ae1f4fc313a51e558	2026-05-04 15:24:39.998-05	2026-05-04 15:26:11.292-05	91.294	0	25	2250	427640	44908	15	1.65261	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ae1f4fc313a51e558.jsonl
agent-ae208c78bad7dab4f	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-ab34e5dad77cb32dc	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ae208c78bad7dab4f	2026-05-04 16:20:17.045-05	2026-05-04 19:05:23.24-05	9906.195	0	436	105498	65891949	1290321	402	130.950332	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ae208c78bad7dab4f.jsonl
agent-ae25492782ab57df5	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ae25492782ab57df5	2026-05-04 19:06:36.965-05	2026-05-04 19:26:57.677-05	1220.712	0	110	17098	5937190	329147	85	16.361291	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ae25492782ab57df5.jsonl
agent-ae2e93649f27b6096	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-a5697667cff09b93e	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ae2e93649f27b6096	2026-05-04 06:45:15.927-05	2026-05-04 06:57:10.6-05	714.673	0	108	26309	7727210	212508	99	17.550135	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ae2e93649f27b6096.jsonl
agent-ae81e467d8c219c94	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a93fc04fa636825dd	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ae81e467d8c219c94	2026-05-03 23:47:22.387-05	2026-05-04 00:43:22.908-05	3360.521	0	270	60927	31751646	313018	260	58.070132	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ae81e467d8c219c94.jsonl
agent-ae8a815614c57e2f5	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a5df5c593fdc359db	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ae8a815614c57e2f5	2026-05-05 01:26:01.52-05	2026-05-05 02:55:27.84-05	5366.32	0	558	115666	105544728	1635038	538	197.657375	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ae8a815614c57e2f5.jsonl
agent-ae966044af8160d31	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-ae966044af8160d31	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ae966044af8160d31	2026-05-03 23:13:21.896-05	2026-05-03 23:45:04.556-05	1902.66	0	208	54158	20788784	553867	188	45.633152	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-ae966044af8160d31.jsonl
agent-aecbf2c4b6cc33e45	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aecbf2c4b6cc33e45	2026-05-03 20:29:17.472-05	2026-05-03 21:10:12.122-05	2454.65	0	328	80779	40333538	767648	303	80.957052	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aecbf2c4b6cc33e45.jsonl
agent-af0c4fb03c4ecfe87	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-af0c4fb03c4ecfe87	2026-05-04 11:00:08.822-05	2026-05-04 11:01:16.257-05	67.435	0	24	1433	391767	41705	14	1.477454	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-af0c4fb03c4ecfe87.jsonl
agent-af1d66d678f5bfb27	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	<synthetic>	worktree-agent-aaf02039f5f672192	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-af1d66d678f5bfb27	2026-05-04 10:59:58.063-05	2026-05-04 11:38:27.713-05	2309.65	0	286	107094	46008204	475018	277	85.955234	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-af1d66d678f5bfb27.jsonl
agent-af24774e1c45c4111	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a17410603a0cbf784	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-af24774e1c45c4111	2026-05-03 22:34:01.066-05	2026-05-03 23:03:42.961-05	1781.895	0	154	40840	12849186	512833	134	31.954708	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-af24774e1c45c4111.jsonl
agent-af3c8d0580280fc67	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a5df5c593fdc359db	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-af3c8d0580280fc67	2026-05-05 02:29:05.624-05	2026-05-05 03:32:59.456-05	3833.832	0	386	141089	72892832	1503529	376	148.117882	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-af3c8d0580280fc67.jsonl
agent-af536b01f1b260656	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-af536b01f1b260656	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-af536b01f1b260656	2026-05-04 15:26:52.195-05	2026-05-04 16:21:49.14-05	3296.945	0	422	130794	80171483	559965	412	140.572448	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-af536b01f1b260656.jsonl
agent-af7b137ea83cad514	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-afcbf42269029714a	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-af7b137ea83cad514	2026-05-04 09:30:17.265-05	2026-05-04 10:54:10.906-05	5033.641	0	825	198615	218733277	1077491	805	363.211372	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-af7b137ea83cad514.jsonl
agent-afb076d324c9a9f08	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-afb076d324c9a9f08	2026-05-03 20:29:48.814-05	2026-05-03 20:45:07.417-05	918.603	0	150	8096	2898261	409160	62	12.628592	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-afb076d324c9a9f08.jsonl
agent-afcbf42269029714a	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-afcbf42269029714a	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-afcbf42269029714a	2026-05-04 08:43:40.973-05	2026-05-04 10:30:08.78-05	6387.807	0	709	194250	181425254	967905	694	304.865485	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-afcbf42269029714a.jsonl
agent-aff2eee49a3bca3ae	f161c896-bb9c-47a3-82e7-ce1edb9d0660	t	claude-opus-4-7	worktree-agent-a2ba5c67bae0b936d	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-aff2eee49a3bca3ae	2026-05-03 21:22:51.38-05	2026-05-03 23:09:26.237-05	6394.857	0	783	269189	221704498	1214983	758	375.538598	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f161c896-bb9c-47a3-82e7-ce1edb9d0660\\subagents\\agent-aff2eee49a3bca3ae.jsonl
f39d11a2-6324-4b12-9e3e-2b0f9f77fa2a	\N	f	claude-opus-4-6	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-04-26 00:52:58.706-05	2026-04-27 15:45:01.088-05	139922.382	0	10128	12955051	790660755	22101958	7662	2572.18359	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f39d11a2-6324-4b12-9e3e-2b0f9f77fa2a.jsonl
f7c35c10-571c-4d35-98a5-434efccc2038	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\decomp-3	2026-05-21 19:57:37.552-05	2026-05-22 13:26:08.548-05	62910.996	0	2506	4128534	570870239	16223308	1284	1470.170024	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f7c35c10-571c-4d35-98a5-434efccc2038.jsonl
f8b20d25-b625-4d10-8c0e-d27739934f1d	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 11:05:08.387-05	2026-05-16 13:25:28.076-05	8419.689	0	78038	812679	164388779	1541603	550	337.60972	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f8b20d25-b625-4d10-8c0e-d27739934f1d.jsonl
agent-aa739ec6f86be6ee0	f8b20d25-b625-4d10-8c0e-d27739934f1d	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 11:22:15.763-05	2026-05-16 11:24:22.156-05	126.393	0	7420	7236	1724522	374197	38	10.256977	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\f8b20d25-b625-4d10-8c0e-d27739934f1d\\subagents\\agent-aa739ec6f86be6ee0.jsonl
fe84210f-b43b-4d45-bb21-dfae2fc0f5e1	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-25 12:42:34.057-05	2026-05-26 00:07:44.864-05	41110.807	0	113658	3626881	427555803	17258645	1042	1238.654243	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\fe84210f-b43b-4d45-bb21-dfae2fc0f5e1.jsonl
fed82e75-9429-4372-8d42-dd5e02bc7eed	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-16 14:44:44.054-05	2026-05-16 22:27:30.958-05	27766.904	0	1716	1602226	627492263	7670587	1163	1205.254591	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\fed82e75-9429-4372-8d42-dd5e02bc7eed.jsonl
agent-a4b57ad4734cc32f8	fed82e75-9429-4372-8d42-dd5e02bc7eed	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a4b57ad4734cc32f8	2026-05-16 15:13:42.306-05	2026-05-16 15:28:51.947-05	909.641	0	153	89014	23953426	437452	143	50.810709	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\fed82e75-9429-4372-8d42-dd5e02bc7eed\\subagents\\agent-a4b57ad4734cc32f8.jsonl
agent-a72dc0754b6ea5e0f	fed82e75-9429-4372-8d42-dd5e02bc7eed	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-a72dc0754b6ea5e0f	2026-05-16 15:14:31.821-05	2026-05-16 15:33:26.251-05	1134.43	0	229	63966	39070539	415847	219	71.203825	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\fed82e75-9429-4372-8d42-dd5e02bc7eed\\subagents\\agent-a72dc0754b6ea5e0f.jsonl
agent-adf11c114b660730d	fed82e75-9429-4372-8d42-dd5e02bc7eed	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-adf11c114b660730d	2026-05-16 15:13:57.677-05	2026-05-16 15:41:04.164-05	1626.487	0	303	117374	49880584	577205	288	94.451065	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\fed82e75-9429-4372-8d42-dd5e02bc7eed\\subagents\\agent-adf11c114b660730d.jsonl
agent-ae999cd66172c7076	fed82e75-9429-4372-8d42-dd5e02bc7eed	t	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile\\.claude\\worktrees\\agent-ae999cd66172c7076	2026-05-16 15:14:15.231-05	2026-05-16 15:44:04.955-05	1789.724	0	385	107074	72171802	501785	375	125.702497	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\fed82e75-9429-4372-8d42-dd5e02bc7eed\\subagents\\agent-ae999cd66172c7076.jsonl
09f3b693-a308-4c74-b64d-c038fbba3c32	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 02:24:22.417-05	2026-05-26 02:27:16.368-05	173.951	0	93504	15763	1332843	125224	27	0.941403	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\09f3b693-a308-4c74-b64d-c038fbba3c32.jsonl
36e4f862-72da-41dd-9128-cef918728314	\N	f	claude-opus-4-6	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 02:54:06.653-05	2026-05-26 02:59:34.827-05	328.174	0	32	35478	1755369	234372	30	1.0412445	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\36e4f862-72da-41dd-9128-cef918728314.jsonl
652390e7-752c-4f8b-adc9-65a6de137113	\N	f	claude-sonnet-4-6	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 02:40:04.128-05	2026-05-26 02:47:25.094-05	440.966	0	48	40586	2222094	233952	44	0.92072505	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\652390e7-752c-4f8b-adc9-65a6de137113.jsonl
74fe95e0-815f-4d9a-9576-d4776e437153	\N	f	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 02:59:50.475-05	2026-05-26 03:05:58.551-05	368.076	0	531	53689	6394566	292025	75	0.4911202	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\74fe95e0-815f-4d9a-9576-d4776e437153.jsonl
93a5db69-e547-44ff-ad61-17ea83481f86	\N	f	claude-haiku-4-5-20251001	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 05:09:13.228-05	2026-05-26 05:17:05.407-05	472.179	0	848	86503	10433003	380961	120	0.71740415	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\93a5db69-e547-44ff-ad61-17ea83481f86.jsonl
9b900aeb-0e88-4cd4-848f-4025580bcffd	\N	f	claude-sonnet-4-6	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 02:28:21.335-05	2026-05-26 02:36:09.187-05	467.852	0	48	45911	2309039	242008	45	0.97326885	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\9b900aeb-0e88-4cd4-848f-4025580bcffd.jsonl
a9c5145c-51ac-4426-b1e4-927bb766e691	\N	f	claude-opus-4-6	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 02:50:18.767-05	2026-05-26 02:53:51.536-05	212.769	0	40	9934	1754729	237934	36	0.9921765	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\a9c5145c-51ac-4426-b1e4-927bb766e691.jsonl
ccd83fe9-1285-43f9-8dfb-d26f34a1064c	\N	f	claude-opus-4-7	main	C:\\Users\\Trenton\\Desktop\\Bushido Blade 2 Decompile	2026-05-26 02:47:47.808-05	2026-05-26 02:50:03.292-05	135.484	0	50	7671	987852	138100	26	0.86748625	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\ccd83fe9-1285-43f9-8dfb-d26f34a1064c.jsonl
\.


--
-- Data for Name: attempt_logs; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.attempt_logs (log_id, attempt_id, kind, transcript_path, line_start, line_end) FROM stdin;
1	1	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6ab25f66-1b4c-4bcf-9991-66a6c23a97c9.jsonl	\N	\N
2	2	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\4e96e912-ecb4-40f0-b936-5fe6b82f3d56.jsonl	\N	\N
3	3	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\25d978c8-b2a1-4918-88ac-2c0b6fa0783c.jsonl	\N	\N
4	4	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\a5d50fee-8ab8-496b-890b-9c463980be5b.jsonl	\N	\N
5	5	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\379f5971-7080-46f4-8832-b43ce4f7b468.jsonl	\N	\N
6	6	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\c36ba981-33ec-481e-a0d3-dd668fbd777b.jsonl	\N	\N
7	7	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\88a2f7a8-fb43-44e7-9f22-0da9cefd180a.jsonl	\N	\N
8	8	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\93a5db69-e547-44ff-ad61-17ea83481f86.jsonl	\N	\N
9	9	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\09f3b693-a308-4c74-b64d-c038fbba3c32.jsonl	\N	\N
10	10	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\9b900aeb-0e88-4cd4-848f-4025580bcffd.jsonl	\N	\N
11	11	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\652390e7-752c-4f8b-adc9-65a6de137113.jsonl	\N	\N
12	12	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\ccd83fe9-1285-43f9-8dfb-d26f34a1064c.jsonl	\N	\N
13	13	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\a9c5145c-51ac-4426-b1e4-927bb766e691.jsonl	\N	\N
14	14	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\36e4f862-72da-41dd-9128-cef918728314.jsonl	\N	\N
15	15	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\74fe95e0-815f-4d9a-9576-d4776e437153.jsonl	\N	\N
16	16	session-slice	C:\\Users\\Trenton\\.claude\\projects\\C--Users-Trenton-Desktop-Bushido-Blade-2-Decompile\\6ab25f66-1b4c-4bcf-9991-66a6c23a97c9.jsonl	\N	\N
\.


--
-- Data for Name: attempt_techniques; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.attempt_techniques (attempt_id, technique_id, relation) FROM stdin;
1	1	applied
2	1	applied
3	1	applied
4	1	applied
5	1	applied
6	1	applied
7	1	applied
8	1	applied
9	1	applied
10	1	applied
11	1	applied
12	1	applied
13	1	applied
14	1	applied
15	1	applied
16	1	applied
1	2	applied
2	2	applied
3	2	applied
4	2	applied
5	2	applied
6	2	applied
7	2	applied
8	2	applied
9	2	applied
10	2	applied
11	2	applied
12	2	applied
13	2	applied
14	2	applied
15	2	applied
16	2	applied
1	3	applied
2	3	applied
3	3	applied
4	3	applied
5	3	applied
6	3	applied
7	3	applied
8	3	applied
9	3	applied
10	3	applied
11	3	applied
12	3	applied
13	3	applied
14	3	applied
15	3	applied
16	3	applied
1	4	applied
2	4	applied
3	4	applied
4	4	applied
5	4	applied
6	4	applied
7	4	applied
8	4	applied
9	4	applied
10	4	applied
11	4	applied
12	4	applied
13	4	applied
14	4	applied
15	4	applied
16	4	applied
1	5	applied
2	5	applied
3	5	applied
4	5	applied
5	5	applied
6	5	applied
7	5	applied
8	5	applied
9	5	applied
10	5	applied
11	5	applied
12	5	applied
13	5	applied
14	5	applied
15	5	applied
16	5	applied
1	6	applied
2	6	applied
3	6	applied
4	6	applied
5	6	applied
6	6	applied
7	6	applied
8	6	applied
9	6	applied
10	6	applied
11	6	applied
12	6	applied
13	6	applied
14	6	applied
15	6	applied
16	6	applied
1	7	applied
2	7	applied
3	7	applied
4	7	applied
5	7	applied
6	7	applied
7	7	applied
8	7	applied
9	7	applied
10	7	applied
11	7	applied
12	7	applied
13	7	applied
14	7	applied
15	7	applied
16	7	applied
1	8	applied
2	8	applied
3	8	applied
4	8	applied
5	8	applied
6	8	applied
7	8	applied
8	8	applied
9	8	applied
10	8	applied
11	8	applied
12	8	applied
13	8	applied
14	8	applied
15	8	applied
16	8	applied
1	9	applied
2	9	applied
3	9	applied
4	9	applied
5	9	applied
6	9	applied
7	9	applied
8	9	applied
9	9	applied
10	9	applied
11	9	applied
12	9	applied
13	9	applied
14	9	applied
15	9	applied
16	9	applied
1	10	applied
2	10	applied
3	10	applied
4	10	applied
5	10	applied
6	10	applied
7	10	applied
8	10	applied
9	10	applied
10	10	applied
11	10	applied
12	10	applied
13	10	applied
14	10	applied
15	10	applied
16	10	applied
1	11	applied
2	11	applied
3	11	applied
4	11	applied
5	11	applied
6	11	applied
7	11	applied
8	11	applied
9	11	applied
10	11	applied
11	11	applied
12	11	applied
13	11	applied
14	11	applied
15	11	applied
16	11	applied
1	12	applied
2	12	applied
3	12	applied
4	12	applied
5	12	applied
6	12	applied
7	12	applied
8	12	applied
9	12	applied
10	12	applied
11	12	applied
12	12	applied
13	12	applied
14	12	applied
15	12	applied
16	12	applied
1	13	applied
2	13	applied
3	13	applied
4	13	applied
5	13	applied
6	13	applied
7	13	applied
8	13	applied
9	13	applied
10	13	applied
11	13	applied
12	13	applied
13	13	applied
14	13	applied
15	13	applied
16	13	applied
1	14	applied
2	14	applied
3	14	applied
4	14	applied
5	14	applied
6	14	applied
7	14	applied
8	14	applied
9	14	applied
10	14	applied
11	14	applied
12	14	applied
13	14	applied
14	14	applied
15	14	applied
16	14	applied
1	15	applied
2	15	applied
3	15	applied
4	15	applied
5	15	applied
6	15	applied
7	15	applied
8	15	applied
9	15	applied
10	15	applied
11	15	applied
12	15	applied
13	15	applied
14	15	applied
15	15	applied
16	15	applied
\.


--
-- Data for Name: attempts; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.attempts (attempt_id, func, file, run_id, model, started_at, ended_at, wall_clock_s, start_score, final_score, outcome, matched, commit_sha, input_tokens, output_tokens, est_cost_usd, n_events, notes) FROM stdin;
1	func_8003F420	config	6ab25f66-1b4c-4bcf-9991-66a6c23a97c9	claude-opus-4-7	2026-05-26 02:37:48.462-05	2026-05-26 02:37:48.462-05	0	0	0	matched	t	218a41ea6bda28b50c18b4a2cff638be2472098d	417	32126	2.415705	1	\N
2	func_8003F420	config	4e96e912-ecb4-40f0-b936-5fe6b82f3d56	claude-opus-4-7	2026-05-26 03:07:17.662-05	2026-05-26 03:25:19.54-05	1081.878	65	0	matched	t	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	199	185799	13.93791	11	\N
3	func_8003F420	config	25d978c8-b2a1-4918-88ac-2c0b6fa0783c	claude-opus-4-7	2026-05-26 03:26:49.531-05	2026-05-26 03:45:29.516-05	1119.985	34	7	canonical-asm	f	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	67211	148360	12.135165	14	\N
4	func_8003F420	config	a5d50fee-8ab8-496b-890b-9c463980be5b	claude-sonnet-4-6	2026-05-26 03:47:24.592-05	2026-05-26 04:07:01.066-05	1176.474	65	3	canonical-asm	f	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	41	199399	2.991108	4	\N
5	func_8003F420	config	379f5971-7080-46f4-8832-b43ce4f7b468	claude-sonnet-4-6	2026-05-26 04:07:37.751-05	2026-05-26 04:27:08.909-05	1171.158	57	11	canonical-asm	f	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	56	151987	2.279973	5	\N
6	func_8003F420	config	c36ba981-33ec-481e-a0d3-dd668fbd777b	claude-opus-4-6	2026-05-26 04:28:13.548-05	2026-05-26 04:44:56.457-05	1002.909	22	3	canonical-asm	f	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	72	101526	7.61553	10	\N
7	func_8003F420	config	88a2f7a8-fb43-44e7-9f22-0da9cefd180a	claude-opus-4-6	2026-05-26 04:49:30.043-05	2026-05-26 05:07:25.171-05	1075.128	65	3	canonical-asm	f	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	70	146213	10.967025	11	\N
8	func_8003F420	config	93a5db69-e547-44ff-ad61-17ea83481f86	claude-haiku-4-5-20251001	2026-05-26 05:10:38.143-05	2026-05-26 05:16:56.179-05	378.036	65	11	canonical-asm	f	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	848	86503	0.433363	16	\N
9	game_SetPlayerCount	config	09f3b693-a308-4c74-b64d-c038fbba3c32	claude-opus-4-7	2026-05-26 02:24:34.612-05	2026-05-26 02:27:06.944-05	152.332	17	0	matched	t	218a41ea6bda28b50c18b4a2cff638be2472098d	93504	15763	2.584785	3	\N
10	game_SetPlayerCount	config	9b900aeb-0e88-4cd4-848f-4025580bcffd	claude-sonnet-4-6	2026-05-26 02:28:33.326-05	2026-05-26 02:35:57.461-05	444.135	18	0	matched	t	218a41ea6bda28b50c18b4a2cff638be2472098d	48	45911	0.688809	3	\N
11	game_SetPlayerCount	config	652390e7-752c-4f8b-adc9-65a6de137113	claude-sonnet-4-6	2026-05-26 02:40:14.724-05	2026-05-26 02:47:18.005-05	423.281	18	0	matched	t	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	48	40586	0.608934	4	\N
12	game_SetPlayerCount	config	ccd83fe9-1285-43f9-8dfb-d26f34a1064c	claude-opus-4-7	2026-05-26 02:48:33.448-05	2026-05-26 02:49:55.858-05	82.41	18	0	matched	t	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	50	7671	0.576075	3	\N
13	game_SetPlayerCount	config	a9c5145c-51ac-4426-b1e4-927bb766e691	claude-opus-4-6	2026-05-26 02:50:32.013-05	2026-05-26 02:53:44.553-05	192.54	18	0	matched	t	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	40	9934	0.74565	4	\N
14	game_SetPlayerCount	config	36e4f862-72da-41dd-9128-cef918728314	claude-opus-4-6	2026-05-26 02:55:01.968-05	2026-05-26 02:59:29.335-05	267.367	18	0	matched	t	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	32	35478	2.66133	3	\N
15	game_SetPlayerCount	config	74fe95e0-815f-4d9a-9576-d4776e437153	claude-haiku-4-5-20251001	2026-05-26 03:00:32.03-05	2026-05-26 03:05:41.403-05	309.373	18	0	matched	t	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	531	53689	0.268976	9	\N
16	stage_InitCollision	config	6ab25f66-1b4c-4bcf-9991-66a6c23a97c9	claude-opus-4-7	2026-05-26 02:37:47.598-05	2026-05-26 02:37:47.598-05	0	7	7	stalled	f	218a41ea6bda28b50c18b4a2cff638be2472098d	417	32126	2.415705	1	\N
\.


--
-- Data for Name: engine_events; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.engine_events (event_id, ts, command, func, file, session_id, git_commit, branch, cwd, pid, ppid, exit_code, score, verdict, sha1, ok, asm_insns, target_insns, build_insns, total_dropped, extra, payload, line_hash) FROM stdin;
1	2026-05-26 02:24:34.612-05	canonical	game_SetPlayerCount	\N	\N	218a41ea6bda28b50c18b4a2cff638be2472098d	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	804	799	\N	\N	C	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "game_SetPlayerCount", "total": 20, "reason": "pure-C distance 18 <= 50 — pure-C target", "verdict": "C", "distance": 18, "asm_insns": 0}	8c661451e6a06bdc4e63928417ca4257a743b68d
2	2026-05-26 02:25:19.692-05	sandbox	game_SetPlayerCount	config	\N	218a41ea6bda28b50c18b4a2cff638be2472098d	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	414	278	\N	17	\N	\N	\N	\N	20	3	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 17, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 3, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	5546539c00cfd232a4cba3ab513b8325fda3752d
3	2026-05-26 02:27:06.944-05	sandbox	game_SetPlayerCount	config	\N	218a41ea6bda28b50c18b4a2cff638be2472098d	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	274	\N	0	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 0, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	92f72d450aa5c9886924ae6babf4e682f587801c
7	2026-05-26 02:28:33.326-05	canonical	game_SetPlayerCount	\N	\N	218a41ea6bda28b50c18b4a2cff638be2472098d	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	502	498	\N	\N	C	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "game_SetPlayerCount", "total": 20, "reason": "pure-C distance 18 <= 50 — pure-C target", "verdict": "C", "distance": 18, "asm_insns": 0}	9c443d982fcfc779e51401137abf946d57f42372
8	2026-05-26 02:28:46.717-05	sandbox	game_SetPlayerCount	config	\N	218a41ea6bda28b50c18b4a2cff638be2472098d	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	585	581	\N	18	\N	\N	\N	\N	20	2	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 18, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 2, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	bc5b298231533bf331f9c172f67e1204733725d6
9	2026-05-26 02:35:57.461-05	sandbox	game_SetPlayerCount	config	\N	218a41ea6bda28b50c18b4a2cff638be2472098d	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	275	\N	0	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 0, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	5b71e03d865469a6119e9cfe461cc37ef21857a4
16	2026-05-26 02:37:47.598-05	sandbox	stage_InitCollision	config	\N	218a41ea6bda28b50c18b4a2cff638be2472098d	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	406	279	\N	7	\N	\N	\N	\N	69	69	\N	{"disable": "all"}	{"file": "config", "func": "stage_InitCollision", "score": 7, "disable": "all", "disabled_o": "tmp/sandbox/stage_InitCollision/config.o", "build_insns": 69, "strip_tier3": false, "target_insns": 69, "rules_dropped": 2, "tier3_stripped": 0}	fb6f750ea32e6b748437a26906121808c942e74b
17	2026-05-26 02:37:48.462-05	sandbox	func_8003F420	config	\N	218a41ea6bda28b50c18b4a2cff638be2472098d	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	279	275	\N	0	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 0, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	9c0b585c3a9cfa05aa44c1d010e2a3d4e625a779
18	2026-05-26 02:40:14.724-05	canonical	game_SetPlayerCount	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	508	507	\N	\N	C	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "game_SetPlayerCount", "total": 20, "reason": "pure-C distance 18 <= 50 — pure-C target", "verdict": "C", "distance": 18, "asm_insns": 0}	7c7367afc92a8271578af6aa1c5cb1da4740178f
19	2026-05-26 02:40:50.055-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	746	745	\N	18	\N	\N	\N	\N	20	2	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 18, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 2, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	7a6452146c725baae93a1856487959653e9911ac
20	2026-05-26 02:45:35.52-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	415	279	\N	1	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 1, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	56d9087bff2e1aa46323f6a1863f96033690df37
21	2026-05-26 02:47:18.005-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	416	280	\N	0	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 0, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	2e2ec5255dea2ebb60c40eab07e7ffd17ee0acef
34	2026-05-26 02:48:33.448-05	canonical	game_SetPlayerCount	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	446	445	\N	\N	C	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "game_SetPlayerCount", "total": 20, "reason": "pure-C distance 18 <= 50 — pure-C target", "verdict": "C", "distance": 18, "asm_insns": 0}	51153d9a056a32f6b7d7bd2e0e2e058f57fd68dc
35	2026-05-26 02:48:34.287-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	445	441	\N	18	\N	\N	\N	\N	20	2	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 18, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 2, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	3d89865f90716ea7a568a670f71a315ccd332104
36	2026-05-26 02:49:55.858-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	280	273	\N	0	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 0, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	0caa9927aaede4af0118a6a3361fa24803831662
52	2026-05-26 02:50:32.013-05	canonical	game_SetPlayerCount	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	1026	1025	\N	\N	C	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "game_SetPlayerCount", "total": 20, "reason": "pure-C distance 18 <= 50 — pure-C target", "verdict": "C", "distance": 18, "asm_insns": 0}	29aaea2a9d01009aa975ec0e9f2a47af9d136fb3
53	2026-05-26 02:50:43.86-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	1110	1104	\N	18	\N	\N	\N	\N	20	2	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 18, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 2, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	5c93e3b2d7fc7bb2208bd55e4d974c81458d00c9
54	2026-05-26 02:51:23.314-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	1191	1190	\N	18	\N	\N	\N	\N	20	2	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 18, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 2, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	986f972a2edce19d9bcf405723eb2bf30d07c74e
55	2026-05-26 02:53:44.553-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	536	532	\N	0	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 0, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	d3cc2ddc09850cd3c4c94865cb9f6781086661c2
75	2026-05-26 02:55:01.968-05	canonical	game_SetPlayerCount	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	274	\N	\N	C	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "game_SetPlayerCount", "total": 20, "reason": "pure-C distance 18 <= 50 — pure-C target", "verdict": "C", "distance": 18, "asm_insns": 0}	4ac3a85a20229c8d0adf515f0dc93e3fa115bbe2
76	2026-05-26 02:55:13.666-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	495	491	\N	18	\N	\N	\N	\N	20	2	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 18, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 2, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	79ea46dd09741f28d4b6aa2e3c27dcee103672f9
77	2026-05-26 02:59:29.335-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	283	278	\N	0	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 0, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	2b0da26af08bc5a5e63e7e3ce533683df0dd2649
100	2026-05-26 03:00:32.03-05	canonical	game_SetPlayerCount	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	415	279	\N	\N	C	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "game_SetPlayerCount", "total": 20, "reason": "pure-C distance 18 <= 50 — pure-C target", "verdict": "C", "distance": 18, "asm_insns": 0}	8e72cdc08cbf41aacdc84add9d9a45a596730d34
101	2026-05-26 03:00:44.435-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	503	502	\N	18	\N	\N	\N	\N	20	2	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 18, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 2, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	f90c1d8380fcb7a7b421bb4f5ad4b27da3d7592b
102	2026-05-26 03:02:17.841-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	405	278	\N	5	\N	\N	\N	\N	20	17	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 5, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 17, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	df1d686d9119c0503a8dcef65110915febdbb257
103	2026-05-26 03:02:43.047-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	508	507	\N	3	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 3, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	86c0a868a8808e9553e19d95f3e1ecfc08060062
104	2026-05-26 03:03:38.098-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	412	276	\N	2	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 2, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	83578aeb706304195336b7260770241dd4a38c9d
105	2026-05-26 03:04:29.059-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	406	279	\N	2	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 2, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	54668ba8f65c1080d1b668e2f544c628efd659ac
106	2026-05-26 03:05:24.125-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	419	283	\N	6	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 6, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	a6439d215ef2e757ce34abc534c0d30d9455e80c
107	2026-05-26 03:05:32.938-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	469	468	\N	6	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 6, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	e42bf3b4be66702e4be23686015d4d306b5d9506
108	2026-05-26 03:05:41.403-05	sandbox	game_SetPlayerCount	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	516	515	\N	0	\N	\N	\N	\N	20	20	\N	{"disable": "all"}	{"file": "config", "func": "game_SetPlayerCount", "score": 0, "disable": "all", "disabled_o": "tmp/sandbox/game_SetPlayerCount/config.o", "build_insns": 20, "strip_tier3": false, "target_insns": 20, "rules_dropped": 0, "tier3_stripped": 0}	6a2aff3c42abdc0d17438cb75ecb40e641f0b9c1
140	2026-05-26 03:07:17.662-05	canonical	func_8003F420	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	426	425	\N	\N	ASM-SUSPECT	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "func_8003F420", "total": 67, "reason": "pure-C distance 65 > 50 — structural-asm suspect (bounded attempt, then PARK)", "verdict": "ASM-SUSPECT", "distance": 65, "asm_insns": 0}	da269f7ea8a05a995829d6e5b8f29e0d9b54c8fa
141	2026-05-26 03:07:18.523-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	425	421	\N	65	\N	\N	\N	\N	67	2	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 65, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 2, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	ac05e09242d0a6670c261f03f4917d531c41632b
142	2026-05-26 03:09:32.387-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	283	279	\N	34	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 34, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	5a509edc61a3e321fea838ccdaf194928416b017
143	2026-05-26 03:11:19.242-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	274	\N	41	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 41, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	8ced9b152f700472e3e4e7c6bfd29874f44691df
144	2026-05-26 03:13:09.969-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	285	282	\N	27	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 27, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	edaa3b7b5be93d4f553b5a8518d101fccd99040c
145	2026-05-26 03:15:21.766-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	281	278	\N	15	\N	\N	\N	\N	67	68	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 15, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 68, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	2a5546845450c5459482e0d2d89508b1e5a4f115
146	2026-05-26 03:17:10.206-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	282	279	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	63f38cd2d430b8b522dd0a0bf16d2fd367dfe0e9
147	2026-05-26 03:19:01.384-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	283	280	\N	14	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 14, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	43014fd985848daa9a08ad6cff500183e2671b7e
148	2026-05-26 03:22:15.852-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	285	281	\N	7	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 7, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	7432996d8da99d68b4cbaa8d99e99255b838b0b0
149	2026-05-26 03:23:51.491-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	275	\N	7	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 7, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	082bd2b878bcdd2b868b4aea0dbebb1e2b01a97e
150	2026-05-26 03:25:19.54-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	277	274	\N	0	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 0, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	b39331005414227a99a8fc44b9fb3c0572ce8425
193	2026-05-26 03:26:49.531-05	canonical	func_8003F420	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	860	856	\N	\N	ASM-SUSPECT	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "func_8003F420", "total": 67, "reason": "pure-C distance 65 > 50 — structural-asm suspect (bounded attempt, then PARK)", "verdict": "ASM-SUSPECT", "distance": 65, "asm_insns": 0}	b7312f4548f2e6d29e07dd6c3ef95e81e160c947
194	2026-05-26 03:28:22.769-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	276	\N	34	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 34, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	fb0972dd4864817d7cfab7114054fcf3cfad8386
195	2026-05-26 03:29:37.046-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	409	275	\N	41	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 41, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	b0c33e21e2601c82e25a9aed3a970966a6c92cb0
196	2026-05-26 03:31:53.757-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	417	281	\N	15	\N	\N	\N	\N	67	68	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 15, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 68, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	d7e7da822769f82690e58f67cba4421bc1b609cd
197	2026-05-26 03:32:59.225-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	414	278	\N	35	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 35, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	2583609187bfaf9d133cbf5435b7a3e5c2b65935
198	2026-05-26 03:34:22.645-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	416	280	\N	15	\N	\N	\N	\N	67	68	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 15, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 68, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	c9b094f081331ade5a0db05968b14b643f5aa369
199	2026-05-26 03:36:47.173-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	416	280	\N	17	\N	\N	\N	\N	67	68	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 17, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 68, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	107e8dfbe1e58a44a2bf08168b31af23162a4bba
200	2026-05-26 03:37:48.979-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	416	280	\N	34	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 34, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	b8d09b91befc802056912585ab76094e797a2f31
201	2026-05-26 03:38:54.054-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	414	278	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	fee905b366bf17e5f91f11858b72222676e8df95
202	2026-05-26 03:40:29.707-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	413	277	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	64bc2e09d120ef1fdbcc813d9771c0a07a37cb8e
203	2026-05-26 03:41:23.283-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	405	278	\N	7	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 7, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	cc6e9d7bc3302e9f13dff31aa713de7f357e8638
204	2026-05-26 03:42:25.335-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	413	277	\N	7	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 7, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	69182fb2cb22e53f5ae7decb2478113d25d4e5ea
205	2026-05-26 03:44:22.582-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	414	278	\N	7	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 7, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	503c5915ccafe766194e1e76e05af8453bceb72e
206	2026-05-26 03:45:29.516-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	413	277	\N	7	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 7, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	d08ad23ddc835d25df18e62bf9af6480b2003628
263	2026-05-26 03:47:24.592-05	canonical	func_8003F420	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	846	845	\N	\N	ASM-SUSPECT	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "func_8003F420", "total": 67, "reason": "pure-C distance 65 > 50 — structural-asm suspect (bounded attempt, then PARK)", "verdict": "ASM-SUSPECT", "distance": 65, "asm_insns": 0}	28cd89efe1f643becce4b0e50a8b1b0b0164e10e
264	2026-05-26 03:47:37.774-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	1062	1061	\N	65	\N	\N	\N	\N	67	2	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 65, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 2, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	8269e6162a26f9f9f0bf498db7eea27a1ab54ae3
265	2026-05-26 03:56:07.642-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	416	280	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	210a752f0cb90e8954fdc22195dae76cbe18f5d5
266	2026-05-26 04:07:01.066-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	418	282	\N	3	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 3, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	6a02a6d041c858cacf5eceec6068429c672419f9
327	2026-05-26 04:07:37.751-05	canonical	func_8003F420	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	1033	1029	\N	\N	ASM-SUSPECT	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "func_8003F420", "total": 67, "reason": "pure-C distance 65 > 50 — structural-asm suspect (bounded attempt, then PARK)", "verdict": "ASM-SUSPECT", "distance": 65, "asm_insns": 0}	e788289ebbfd82f4ca91ac2b1ce1e2719a9fea2b
328	2026-05-26 04:11:57.074-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	417	281	\N	57	\N	\N	\N	\N	67	22	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 57, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 22, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	e9c6ed8731bdf9bd61b4b2e1a7ee2869718606eb
329	2026-05-26 04:16:55.821-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	410	274	\N	27	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 27, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	b4fdb9a9f18bbab125111605685be9ff606b4a81
330	2026-05-26 04:23:08.936-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	419	283	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	0e0ce92a7eedd07789504ad3261572c0f0c5403e
331	2026-05-26 04:27:08.909-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	419	283	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	5555b129902eaac811f166850b30d0bb19d236d5
397	2026-05-26 04:28:13.548-05	canonical	func_8003F420	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	833	832	\N	\N	ASM-SUSPECT	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "func_8003F420", "total": 67, "reason": "pure-C distance 65 > 50 — structural-asm suspect (bounded attempt, then PARK)", "verdict": "ASM-SUSPECT", "distance": 65, "asm_insns": 0}	6e345516e9c44b36b700311fe05d377c186a069e
398	2026-05-26 04:30:17.826-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	283	279	\N	22	\N	\N	\N	\N	67	68	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 22, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 68, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	2b96098ef54011d08708dac637a5c1f9ad6a4703
399	2026-05-26 04:32:52.568-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	282	279	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	c5f1467cb472070429c0ca9fa073309adc696ce9
400	2026-05-26 04:34:21.468-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	274	\N	9	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 9, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	253c3cec3941540127124d3babe10d4f6bb2b91c
401	2026-05-26 04:34:28.525-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	461	458	\N	9	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 9, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	e17488e913ae2f2371ba9a9ddf6beac508bdc602
402	2026-05-26 04:36:36.577-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	284	279	\N	31	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 31, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	be17340b992cad7b50d2df24fa7d88590bea28f3
403	2026-05-26 04:37:14.503-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	275	\N	3	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 3, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	445419830243e18b0c06b6e95095b49364b91c51
404	2026-05-26 04:39:02.188-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	283	278	\N	3	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 3, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	1d3be6920ea2f314affd43dbc95f6bac54baea0c
405	2026-05-26 04:41:33.191-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	282	279	\N	3	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 3, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	d2e54940eaedf351e13d6f33de3d7e84d34b7b54
406	2026-05-26 04:44:56.457-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	274	\N	3	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 3, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	dbe4d0abc9dae62c17ee126d9a03e1284d45d514
482	2026-05-26 04:49:30.043-05	canonical	func_8003F420	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	275	\N	\N	ASM-SUSPECT	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "func_8003F420", "total": 67, "reason": "pure-C distance 65 > 50 — structural-asm suspect (bounded attempt, then PARK)", "verdict": "ASM-SUSPECT", "distance": 65, "asm_insns": 0}	66cdc93145566d40fdabbe1930a0378888c725d5
483	2026-05-26 04:49:45.269-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	502	501	\N	65	\N	\N	\N	\N	67	2	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 65, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 2, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	5ef5fd6b797c9d708b0fa0ecf39fbc588767c1d4
484	2026-05-26 04:53:20.884-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	404	400	\N	15	\N	\N	\N	\N	67	68	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 15, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 68, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	bc7b8443aee0bf1d1f3c85e78c222be1cac091d6
485	2026-05-26 04:56:12.838-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	283	279	\N	9	\N	\N	\N	\N	67	68	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 9, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 68, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	173c4274a7707778ae523f2b12b68bda4215fc25
486	2026-05-26 04:58:27.81-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	284	281	\N	9	\N	\N	\N	\N	67	68	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 9, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 68, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	37bcb107469fec981846dd847b6c04a80c7c5fa6
487	2026-05-26 05:01:19.634-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	284	281	\N	9	\N	\N	\N	\N	67	68	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 9, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 68, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	53ea945a9d18d2b456dca2bdc3831bdaae39db3c
488	2026-05-26 05:02:01.45-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	275	\N	9	\N	\N	\N	\N	67	68	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 9, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 68, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	dd835236185a9c044ee06564a9dc9c317126c3c1
489	2026-05-26 05:03:30.249-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	278	274	\N	16	\N	\N	\N	\N	67	68	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 16, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 68, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	b18d57b8e2d5f7fa565acd02db9c9c047f7a1867
490	2026-05-26 05:04:18.058-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	279	275	\N	5	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 5, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	86ba51b3e6184de0070bf4f354c299f85590b403
491	2026-05-26 05:05:18.439-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	279	274	\N	3	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 3, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	f492a17df215837e4553153ceeee191e3548fab3
492	2026-05-26 05:07:25.171-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	281	278	\N	3	\N	\N	\N	\N	67	67	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 3, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 67, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	b066700b8fa6651ea908bc313c56110aa343f82e
579	2026-05-26 05:10:38.143-05	canonical	func_8003F420	\N	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	417	281	\N	\N	ASM-SUSPECT	\N	\N	0	\N	\N	\N	{"fast": false}	{"func": "func_8003F420", "total": 67, "reason": "pure-C distance 65 > 50 — structural-asm suspect (bounded attempt, then PARK)", "verdict": "ASM-SUSPECT", "distance": 65, "asm_insns": 0}	f1836d93a1356dec5ac3c74b84616e8223e99133
580	2026-05-26 05:10:42.632-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	469	468	\N	65	\N	\N	\N	\N	67	2	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 65, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 2, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	a5cab1ddeb8b887b7b123c42f5f1b56617332c45
581	2026-05-26 05:11:52.921-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	414	278	\N	37	\N	\N	\N	\N	67	76	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 37, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 76, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	835aad43612b5c01c7358d41c65820b0c7966f1c
582	2026-05-26 05:12:39.342-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	405	278	\N	38	\N	\N	\N	\N	67	74	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 38, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 74, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	006162f530436b6933f7f99fd8abea479b9fe087
583	2026-05-26 05:13:16.897-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	415	279	\N	39	\N	\N	\N	\N	67	76	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 39, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 76, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	da76f004467f2884f5797e5ffffcc9dcdf9b51e6
584	2026-05-26 05:13:28.53-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	464	463	\N	38	\N	\N	\N	\N	67	74	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 38, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 74, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	2d68de66b348d5e18ca18d2f4a43d032833ecc7d
585	2026-05-26 05:14:07.481-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	405	278	\N	45	\N	\N	\N	\N	67	74	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 45, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 74, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	8ab191c255fc2e1a0c55edf6d2921e0b35b1470c
586	2026-05-26 05:14:44.945-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	414	278	\N	35	\N	\N	\N	\N	67	74	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 35, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 74, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	a73b18690a5b55b7b900421cdd36b6b76463db58
587	2026-05-26 05:14:55.002-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	464	463	\N	35	\N	\N	\N	\N	67	74	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 35, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 74, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	e569702ea0a3ccd233c9ca8df4149840efebd24d
588	2026-05-26 05:15:09.552-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	510	509	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	b31479002b72793593ddc16ab50f89a290c0ddfe
589	2026-05-26 05:15:28.212-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	716	715	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	edb92beb96a0f65d447130072fd83a182e5ec454
590	2026-05-26 05:16:05.773-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	411	275	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	b79c6a0e46577c389e58ac105c8cc218af44b142
591	2026-05-26 05:16:18.746-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	461	460	\N	35	\N	\N	\N	\N	67	74	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 35, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 74, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	be6fb107ef52ffd6189b6f5f83592f9c0c8474b9
592	2026-05-26 05:16:29.919-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	508	507	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	7d5b7407ddc2567f0c5352d8243ed7be2903ce7b
593	2026-05-26 05:16:45.666-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	684	683	\N	35	\N	\N	\N	\N	67	74	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 35, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 74, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	3bcd3285acfc994fed45200b244fd938148276c1
594	2026-05-26 05:16:56.179-05	sandbox	func_8003F420	config	\N	ab2fa368f0fbc765ee9f3b4f108d3d48d590cf04	main	/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile	732	731	\N	11	\N	\N	\N	\N	67	66	\N	{"disable": "all"}	{"file": "config", "func": "func_8003F420", "score": 11, "disable": "all", "disabled_o": "tmp/sandbox/func_8003F420/config.o", "build_insns": 66, "strip_tier3": false, "target_insns": 67, "rules_dropped": 0, "tier3_stripped": 0}	9f549fe2161c5e7b2a6a098dce871ed2be60c619
\.


--
-- Data for Name: experiments; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.experiments (experiment_id, ts, func, file, model, effort, session_id, budget_usd, minutes, matched, start_score, final_score, full_sha1_match, total_cost_usd, input_tokens, output_tokens, cache_read_tokens, cache_creation_tokens, num_turns, duration_s, wall_s, terminal_reason, git_commit, notes, run_key) FROM stdin;
1	2026-05-26 02:27:25-05	game_SetPlayerCount	config	opus	high	09f3b693-a308-4c74-b64d-c038fbba3c32	8	20	t	18	0	t	0.941403	30734	5839	576666	56548	13	174.291	175	completed	218a41ea6bda	Matched on the first real attempt. **Score: 0**, 20/20 instructions.\n\n## Result\n\n- **Final score:** `0` (cheat-free, pure C â€” `--disable all`)\n- **Sandbox iterations:** 2 (one throwaway `g_game_player_count = a0;` to get the function into the `.o` for disassembly, then the real reconstruction)\n- **Final C body:**\n\n```c\nvoid game_SetPlayerCount(s32 a0) {\n    if ((u32)a0 < 2) {\n        if (a0 != g_game_player_count) {\n            g_game_player_count = a0;\n            if (a0 == 0) {\n             	fdc75145db18df2b0817b4c6c265c661767c0233
3	2026-05-26 02:36:18-05	game_SetPlayerCount	config	sonnet	high	9b900aeb-0e88-4cd4-848f-4025580bcffd	8	20	t	18	0	t	0.97326885	25	17177	1268867	89301	25	468.176	470	completed	218a41ea6bda	Score: **0** on the first attempt.\n\n**Summary:**\n- **Final score**: 0 (byte-identical match)\n- **Sandbox iterations**: 1\n- **Final C body**:\n  ```c\n  void game_SetPlayerCount(s32 a0) {\n      if ((u32)a0 >= 2u) return;\n      if (a0 == g_game_player_count) return;\n      g_game_player_count = a0;\n      if (!a0) game_SetControllerPorts(0);\n      g_game_mirror_mode = (s16)g_game_player_count;\n  }\n  ```\n- **Tricky part**: The `jal` at the core of the function had a relocation to `game_SetControllerPor	8bf80162587cf7881f61612acd32542183bb4c86
6	2026-05-26 02:47:41-05	game_SetPlayerCount	config	sonnet	low	652390e7-752c-4f8b-adc9-65a6de137113	8	20	t	18	0	f	0.92072505	26	15344	1346436	76415	28	441.287	442	completed	ab2fa368f0fb	**Score: 0** â€” byte-identical match.\n\n---\n\n**Summary:**\n- **Final score:** 0\n- **Sandbox iterations:** 3 (blank â†’ 1 â†’ 0)\n- **Final C body:**\n\n```c\nvoid game_SetPlayerCount(s32 a0) {\n    if ((u32)a0 >= 2) {\n        return;\n    }\n    if (a0 == g_game_player_count) {\n        return;\n    }\n    g_game_player_count = a0;\n    if (!a0) {\n        void stage_ExecInitFunc(s32);\n        stage_ExecInitFunc(0);\n    }\n    g_game_mirror_mode = (s16)g_game_player_count;\n}\n```\n\n**Tricky part:** The `jal sta	3eaee8c7ee4dc8c475cf7696708d6cfba9782e8c
10	2026-05-26 02:50:12-05	game_SetPlayerCount	config	opus	low	ccd83fe9-1285-43f9-8dfb-d26f34a1064c	8	20	t	18	0	t	0.86748625	31	4475	632250	70293	16	135.811	136.5	completed	ab2fa368f0fb	Score 0 â€” matched on the first edit.\n\n**Final score:** `0` (target_insns 20 = build_insns 20)\n**Sandbox iterations:** 1 (after the initial baseline read)\n\n**Final C body:**\n```c\nvoid game_SetPlayerCount(s32 a0) {\n    if ((u32)a0 < 2) {\n        if (a0 != g_game_player_count) {\n            g_game_player_count = a0;\n            if (a0 == 0) {\n                game_SetControllerPorts(0);\n            }\n            g_game_mirror_mode = g_game_player_count;\n        }\n    }\n}\n```\n(plus one added extern	bb9f8298a8cf8845f7b106fe05b3be29be863f71
15	2026-05-26 02:54:00-05	game_SetPlayerCount	config	claude-opus-4-6	low	a9c5145c-51ac-4426-b1e4-927bb766e691	8	20	t	18	0	t	0.9921765	19	4429	967713	63600	23	213.085	213.8	completed	ab2fa368f0fb	Score **0** on the first attempt. Done.\n\n**Result:**\n- Final score: **0** (byte-identical match)\n- Sandbox iterations: **1**\n- Final C body:\n```c\nvoid game_SetPlayerCount(s32 a0) {\n    if ((u32)a0 >= 2) {\n        return;\n    }\n    if (a0 == g_game_player_count) {\n        return;\n    }\n    g_game_player_count = a0;\n    if (a0 == 0) {\n        game_SetControllerPorts(0);\n    }\n    g_game_mirror_mode = g_game_player_count;\n}\n```\n- Nothing tricky â€” straightforward setter with bounds check (`0 or 1`	4fbfa453d2b0b8f4188128da352b9f8b5abf9041
21	2026-05-26 02:59:44-05	game_SetPlayerCount	config	claude-opus-4-6	high	36e4f862-72da-41dd-9128-cef918728314	8	20	t	18	0	t	1.0412445	11	10215	590429	78496	15	328.496	329.2	completed	ab2fa368f0fb	Score **0** on the first attempt.\n\n**Result:**\n- Final score: **0** (byte-identical match)\n- Sandbox iterations: **1**\n- Final C body:\n```c\nvoid game_SetPlayerCount(s32 a0) {\n    if ((u32)a0 >= 2) return;\n    if (a0 == g_game_player_count) return;\n    g_game_player_count = a0;\n    if (a0 == 0) {\n        game_SetControllerPorts(0);\n    }\n    g_game_mirror_mode = g_game_player_count;\n}\n```\n- Nothing tricky â€” straightforward setter with a bounds check (`a0` must be 0 or 1), an early-out if unchan	d083a80077990e1a6e428fdf3527ba7a64da7d18
28	2026-05-26 03:06:14-05	game_SetPlayerCount	config	haiku	high	74fe95e0-815f-4d9a-9576-d4776e437153	8	20	t	18	0	f	0.4911202	219	18993	2666412	103436	31	368.422	369.1	completed	ab2fa368f0fb	Perfect! The function now matches the target byte-identically with a score of **0**.\n\n## Summary\n\n**Final C Body (lines 62-82):**\n```c\nvoid game_SetPlayerCount(s32 a0) {\n    s32 v0;\n\n    if ((u32)a0 >= 2) {\n        return;\n    }\n\n    v0 = g_game_player_count;\n    if (a0 == v0) {\n        return;\n    }\n\n    g_game_player_count = a0;\n\n    if (a0 == 0) {\n        ((void (*)(s32))stage_ExecInitFunc)(0);\n    }\n\n    v0 = g_game_player_count;\n    *((u16 *)((s32)&func_8004153C)) = v0;\n}\n```\n\n**Sandbox Ite	6ee61a02d137667400f2976921f8197d9afff732
36	2026-05-26 03:26:31-05	func_8003F420	config	opus	high	\N	8	20	t	65	0	t	\N	\N	\N	\N	\N	\N	\N	1200	timeout-killed	ab2fa368f0fb		b4ce886555220dcce85224db2418365c3c11341e
45	2026-05-26 03:47:06-05	func_8003F420	config	opus	low	\N	8	20	f	65	7	\N	\N	\N	\N	\N	\N	\N	\N	1200	timeout-killed	ab2fa368f0fb		c23b90f94b05d0edbb7b8df4ec9e1ffc11ad84c3
55	2026-05-26 04:07:20-05	func_8003F420	config	sonnet	high	\N	8	20	f	65	3	\N	\N	\N	\N	\N	\N	\N	\N	1200	timeout-killed	ab2fa368f0fb		ccb3abd963585b7c5eb037c3b8f663ed374636d7
66	2026-05-26 04:27:55-05	func_8003F420	config	sonnet	low	\N	8	20	f	65	11	\N	\N	\N	\N	\N	\N	\N	\N	1200	timeout-killed	ab2fa368f0fb		3d343ceac207562cb01c1cc5444bf8cb1c26383f
78	2026-05-26 04:48:31-05	func_8003F420	config	claude-opus-4-6	low	\N	8	20	f	65	3	\N	\N	\N	\N	\N	\N	\N	\N	1200	timeout-killed	ab2fa368f0fb		a6c9925af30a8ae628d762b1c85623bdd013ff3a
91	2026-05-26 05:09:06-05	func_8003F420	config	claude-opus-4-6	high	\N	8	20	f	65	3	\N	\N	\N	\N	\N	\N	\N	\N	1200	timeout-killed	ab2fa368f0fb		0534f939ab1502eefddc0d33dae0b3b634302846
105	2026-05-26 05:17:13-05	func_8003F420	config	haiku	high	93a5db69-e547-44ff-ad61-17ea83481f86	8	20	f	65	11	\N	0.71740415	338	30771	4236299	111665	50	472.478	473.2	completed	ab2fa368f0fb	## Summary\n\n**Final Score:** 11 (down from initial 65)  \n**Sandbox Iterations:** 3 main attempts + refinements  \n**Final C Body:**\n\n```c\nvoid func_8003F420(s32 a0, s32 a1) {\n    s32 s3, s2, s1, s0;\n\n    a0 = a0 + 0x7D00;\n    a1 = a1 + 0x7D00;\n\n    s3 = a0 / 2000;\n    s1 = a0 % 2000;\n    s2 = a1 / 2000;\n    s0 = a1 % 2000;\n\n    if (s1 < 1000) {\n        s1 = -1;\n    } else {\n        s1 = 1;\n    }\n    if (s0 < 1000) {\n        s0 = -1;\n    } else {\n        s0 = 1;\n    }\n\n    stage_SetCollision(s3, s	1deac28d8a32d729fb391680cecc7ec8c1c38bfa
\.


--
-- Data for Name: sync_meta; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.sync_meta (key, value, ts) FROM stdin;
last_sync	2026-05-26T10:17:17.494842+00:00	2026-05-26 05:17:17.495327-05
\.


--
-- Data for Name: techniques; Type: TABLE DATA; Schema: public; Owner: postgres
--

COPY public.techniques (technique_id, slug, content_hash, kind, title, rule_path, first_seen_ts, first_seen_commit, description) FROM stdin;
16	split-read-defeats-hoist	\N	rule	split read defeats hoist	.claude/rules/split-read-defeats-hoist.md	2026-05-22 19:08:10-05	95d637085c7d56d63ef49570860bfd08855eb0df	\N
17	\N	c136c8d230530514	pattern	Match coli_HitPauseKatana_2 -- retire asmfix in pure C	\N	2026-05-22 16:11:00-05	b233af879aa140118c0c94742d5d96374555635d	\N
19	\N	90e157ed1e2ba434	pattern	cheat-cleanup: func_80068F70 â€” retire 35-rule regfix block via 100% pure C (no regfix, no pins, no asm)	\N	2026-05-22 15:05:07-05	6c9dc45ffd8541392402f5d9b878e93d63d2a82c	\N
20	packed-multiply-cluster	\N	rule	packed multiply cluster	.claude/rules/packed-multiply-cluster.md	2026-05-21 11:31:29-05	19243efc739f946cc98682afa013f62cb004d08c	\N
21	\N	06c8212624386611	pattern	cheat-cleanup: func_8007EDBC -- retire via canonical asm (hand-coded, no C form)	\N	2026-05-21 10:58:25-05	2db22e18b8749bd3661fe2f698b8d6210862678c	\N
30	register-alloc-pure-c	\N	rule	register alloc pure c	.claude/rules/register-alloc-pure-c.md	2026-05-21 00:50:11-05	2ceff65156a0dde463b147530127ff353642cc6e	\N
32	\N	c32772eb56946d71	pattern	saTan0Main: retire all 5 register pins + andi regfix rule -> 100% pure C	\N	2026-05-21 00:39:17-05	6dba0501d8e715fd2ff606af941f42c9c1abf24d	\N
34	\N	722b5574eb000a96	pattern	cheat-cleanup: InitHiraRmd_80047FBC â€” retire via 100% pure C (no regfix, no pins, no asm)	\N	2026-05-20 23:21:58-05	10becc3b1a319122c7058fd1400a9c26464940b2	\N
35	\N	e8875c0a540419bc	pattern	maspsx: nop for lw;.L-label;jalr; retire 2 saTan0Main regfix rules	\N	2026-05-20 13:13:51-05	2f96f501b114d541c83b7a9bd84aac0d84469698	\N
36	compiler-flags-canonical	\N	rule	compiler flags canonical	.claude/rules/compiler-flags-canonical.md	2026-05-20 05:03:20-05	81e174e687b7e3c63e2dc3baf9c02a53f66693dd	\N
37	cross-jump-call-merge	\N	rule	cross jump call merge	.claude/rules/cross-jump-call-merge.md	2026-05-20 05:03:20-05	81e174e687b7e3c63e2dc3baf9c02a53f66693dd	\N
38	\N	9c8eedf4a190d03f	pattern	Backlog: retire func_8005509C -- 3 regfix rules	\N	2026-05-19 14:55:30-05	e3c69ab272d872329cee35cfb42b9e58fbedec7f	\N
39	\N	a8e15e7e15b080eb	pattern	Backlog: retire func_8004954C -- 3 regfix rules via targeted permuter	\N	2026-05-19 14:49:51-05	d5d9b65d99db5624f3a4937744be2ee6062b7fde	\N
40	\N	5098dfa9aaf3f560	pattern	Backlog: retire func_80060CB8 -- NEW match via typedef-closure unlock	\N	2026-05-19 11:50:57-05	c1f3465c0cf133176d02df1c38f50a2e51882abc	\N
41	\N	ebe9af4e165edf49	pattern	Backlog: retire func_8007D9C4 -- 2 regfix substs via extern type change	\N	2026-05-19 06:03:48-05	04d5f89102a26e58890949c162a618ee44c7573b	\N
42	\N	951a2dd4988765af	pattern	Backlog: retire func_80086014 -- 2 regfix inserts via single keyword change	\N	2026-05-19 05:37:17-05	32f4fe11f91fdf4bc2b3d5bfa3abe7081d990351	\N
43	\N	748313b1b8191b19	pattern	Backlog: retire mario_getMarioVoiceData_8005BE84 regfix reorder	\N	2026-05-19 05:31:16-05	f8706d7dcadc72d3ceb8c8474371f987043be488	\N
44	\N	ebf1219b09ad9bed	pattern	Backlog: retire func_80077894 regfix reorder via targeted permuter	\N	2026-05-19 04:29:49-05	2d5330de183f6b2dfdccc63b5087f3f44c0b96df	\N
45	\N	29630ea2253779fd	pattern	Backlog: retire func_80047BE0 regfix via targeted permuter	\N	2026-05-19 03:57:22-05	56625b5d427193bdeb00890057fa3ffa8a1e4f13	\N
46	\N	ea56fa6fea4264a0	pattern	Backlog: retire func_80062FEC regfix via targeted permuter	\N	2026-05-19 03:54:04-05	efa039e27f8f91b7a079df811e6130273c2b7522	\N
47	\N	952926e7677077b8	pattern	Backlog: retire InitFadePanel regfix swap via targeted permuter	\N	2026-05-19 03:51:44-05	84571c78927268442100344bfe1d0b8eb6294704	\N
24	inline-asm-tiers	\N	rule	inline asm tiers	.claude/rules/inline-asm-tiers.md	2026-05-21 10:25:22-05	f2592d0729f38ae60e41768f67469f30dc516ef0	\N
50	\N	5ccde00178209662	pattern	Revert "Match: coli_HitPauseKatana_2 (asmfix retirement, main.c, 178 insns)"	\N	2026-05-18 21:41:03-05	d9e5b4faaa5056ff4f286e88999384f5fdc8bf3c	\N
51	\N	ca4e6c573f26202a	pattern	Revert "Match: coli_HitPauseKatana_2 (pure-C improved â€” 6 rules, 2 pins removed)"	\N	2026-05-18 21:40:16-05	09fb36146ac3a0c22f5bf567e12a32413817e7db	\N
52	\N	2b55bade8f53b88d	pattern	func_800483DC: enrich INLINE_MOVE_ALIASING justification with full attempts log	\N	2026-05-18 18:10:41-05	7cfad8cec982473bdeb6d242112a3c751e80133f	\N
3	attempts-log-gate	\N	rule	attempts log gate	.claude/rules/attempts-log-gate.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
54	\N	161e53c4c410f4b5	pattern	cheat-cleanup: func_800483DC â€” legitimate inline_move_aliasing escape valve	\N	2026-05-18 16:42:02-05	c0c8ca584c188ab423c9d5b56e55cd55f760068e	\N
55	\N	543bc7b4bf7ba81e	pattern	Match: coli_HitPauseKatana_2 (pure-C improved â€” 6 rules, 2 pins removed)	\N	2026-05-18 14:53:20-05	f7c73793ce408b06364723b55e91661af2d5ee48	\N
56	\N	a09378e5fe4d77fa	pattern	hooks: bump audit timeout 180â†’900s, budget $0.50â†’$1.00	\N	2026-05-18 13:13:10-05	a1d6a0070eae2b319b247733db784824a162cc57	\N
57	\N	d5f2d5158943dacf	pattern	Match: coli_HitPauseKatana_2 (asmfix retirement, main.c, 178 insns)	\N	2026-05-18 13:00:03-05	66ccae2b3c1befa7a98d4ed5d568f063f1d56f48	\N
58	\N	10cf33d6153c7a7e	pattern	SetPacketData: defer + volatile improvement; bulk pre-emptive defers	\N	2026-05-18 04:09:12-05	122d8f1341812a3d549d076f2312b1d57c7f80bd	\N
59	\N	a1b9a7539f27f048	pattern	asmfix_defer: add coli_HitPauseKatana_2 + auto-cleanups	\N	2026-05-18 03:54:19-05	f1668bd14fb1409bc812c583debe4b2e693eaa32	\N
60	\N	c17d3cca1a4d3ad2	pattern	queue: add asmfix defer mechanism + small analyzer	\N	2026-05-18 03:39:18-05	382c734e8f576ee1f478a69b65533b14eec18d0c	\N
61	\N	0303d395d6bc4f8e	pattern	Match func_80075830 (104 insns, pure C)	\N	2026-05-18 03:23:28-05	2c3c146b39634d568ce6be8d2f5a348f3c87b200	\N
4	bridge-signature	\N	rule	bridge signature	.claude/rules/bridge-signature.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
5	cheat-cleanup-techniques	\N	rule	cheat cleanup techniques	.claude/rules/cheat-cleanup-techniques.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
6	cu-split-asmfix-drift	\N	rule	cu split asmfix drift	.claude/rules/cu-split-asmfix-drift.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
65	dead-branch-scheduling	\N	rule	dead branch scheduling	.claude/rules/dead-branch-scheduling.md	2026-05-18 00:55:31-05	806003d05eba30642e7779588bfa9c9234a28255	\N
33	dead-vars-local-array	\N	rule	dead vars local array	.claude/rules/dead-vars-local-array.md	2026-05-20 23:23:33-05	9a790585ba79935cdd3d3ebcba546d36b3be1536	\N
7	exec-game-plateau	\N	rule	exec game plateau	.claude/rules/exec-game-plateau.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
68	gte-3x3	\N	rule	gte 3x3	.claude/rules/gte-3x3.md	2026-05-18 00:55:31-05	806003d05eba30642e7779588bfa9c9234a28255	\N
1	inline-asm-injection	\N	rule	inline asm injection	.claude/rules/inline-asm-injection.md	2026-05-26 00:07:14-05	7524f26aa2ec64ac4ec1a019375766faeee62f62	\N
25	inline-move-aliasing	\N	rule	inline move aliasing	.claude/rules/inline-move-aliasing.md	2026-05-21 10:25:22-05	f2592d0729f38ae60e41768f67469f30dc516ef0	\N
8	kengo-cross-reference	\N	rule	kengo cross reference	.claude/rules/kengo-cross-reference.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
9	kengo-name-unreliable	\N	rule	kengo name unreliable	.claude/rules/kengo-name-unreliable.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
2	lost-codegen-insert-cheat	\N	rule	lost codegen insert cheat	.claude/rules/lost-codegen-insert-cheat.md	2026-05-26 00:07:14-05	7524f26aa2ec64ac4ec1a019375766faeee62f62	\N
74	maspsx-noreorder-stripping	\N	rule	maspsx noreorder stripping	.claude/rules/maspsx-noreorder-stripping.md	2026-05-18 00:55:31-05	806003d05eba30642e7779588bfa9c9234a28255	\N
10	raw-dx-unsafe	\N	rule	raw dx unsafe	.claude/rules/raw-dx-unsafe.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
11	regfix-reference	\N	rule	regfix reference	.claude/rules/regfix-reference.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
12	regfix-subst-multi-splice	\N	rule	regfix subst multi splice	.claude/rules/regfix-subst-multi-splice.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
28	register-asm-pins	\N	rule	register asm pins	.claude/rules/register-asm-pins.md	2026-05-21 10:25:22-05	f2592d0729f38ae60e41768f67469f30dc516ef0	\N
13	retirement-recipes	\N	rule	retirement recipes	.claude/rules/retirement-recipes.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
14	satan2-kabuto-pin	\N	rule	satan2 kabuto pin	.claude/rules/satan2-kabuto-pin.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
81	scratchpad-gte	\N	rule	scratchpad gte	.claude/rules/scratchpad-gte.md	2026-05-18 00:55:31-05	806003d05eba30642e7779588bfa9c9234a28255	\N
82	shared-end-label	\N	rule	shared end label	.claude/rules/shared-end-label.md	2026-05-18 00:55:31-05	806003d05eba30642e7779588bfa9c9234a28255	\N
83	store-before-jal	\N	rule	store before jal	.claude/rules/store-before-jal.md	2026-05-18 00:55:31-05	806003d05eba30642e7779588bfa9c9234a28255	\N
84	strength-reduce-defeat	\N	rule	strength reduce defeat	.claude/rules/strength-reduce-defeat.md	2026-05-18 00:55:31-05	806003d05eba30642e7779588bfa9c9234a28255	\N
15	voice-control-playbook	\N	rule	voice control playbook	.claude/rules/voice-control-playbook.md	2026-05-25 19:09:42-05	c8d6a381692ca15f3d752e0436324a549037da8e	\N
86	\N	99a914dc3d078217	pattern	code6cac.c: rewrite single_game_VoiceContorol â€” pure-C body + 5 specific regfix rules	\N	2026-05-17 23:30:22-05	d33ea6b8b6173091fad85d8243b78f173917223b	\N
87	\N	9f0a38159004b940	pattern	cleanup: saTan2KabutoWareMove â€” strip 45 stale regfix rules (bridged)	\N	2026-05-17 23:17:56-05	5b86aafcf987521d3e6b205a5094a6a81f629a49	\N
88	\N	62ed0290a3e7f79f	pattern	cleanup: coli_HitPauseKatana_2 â€” strip 41 stale regfix rules (bridged)	\N	2026-05-17 23:17:56-05	39737070aabfa2417aa06367a049ee4d400091e0	\N
89	\N	bdacf0b216699647	pattern	cleanup: DispPracticeMenuTex_A â€” strip 32 stale regfix rules (bridged)	\N	2026-05-17 23:17:56-05	7db4ead861dfff7d6d3c5d2b1843b9e8a4793bc4	\N
90	\N	e3b77484e3579b05	pattern	cleanup: saTan0KiWareMoveB â€” strip 7 stale regfix rules (bridged)	\N	2026-05-17 23:17:56-05	07425188b501930e97ea86df046b4ac94f4e1c47	\N
91	\N	1427f896a5d7c3cf	pattern	cheat-cleanup: cpu_check_run_attack â€” split multi-insn __asm__ + strip stale regfix	\N	2026-05-17 23:17:56-05	7ede479990288374c77330ae7f3ccf80a66aeed5	\N
92	\N	fe0ed75b5fddf112	pattern	cheat-cleanup: func_80084500 â€” strip 146 stale regfix rules (bridged)	\N	2026-05-17 23:17:56-05	b51fd4e7f90c02d3d9fa03f21424c8b3e8de9ed3	\N
93	\N	449acd7f925ef6ed	pattern	cheat-cleanup: func_800841E0 â€” strip 121 stale regfix rules (bridged)	\N	2026-05-17 23:17:56-05	e3ea0ed9cdcd468439b8cf2c46640320820cc4b2	\N
94	\N	f5e7b6099eb37fdf	pattern	cheat-cleanup: SetPacketData â€” retire vestigial lost_codegen + stale regfix rules	\N	2026-05-17 23:17:56-05	f1edd1ff806821ab6db2f570cd510806322d8af0	\N
95	\N	75b31cde4a4a6082	pattern	naming: promote D_800A26D4 -> g_alarm_secondary_cb_ptr (3 sites in main.c)	\N	2026-05-17 22:41:38-05	ff7501bc9f1bf2a33f3b401e077db1d4993e207d	\N
96	\N	d4f75f741dcff106	pattern	cheat-cleanup: func_800826CC -- retire 2 lost_codegen via volatile aliasing + switch	\N	2026-05-17 20:27:20-05	d8ec667a1e6a40fea8a2172a048b379f472fcd86	\N
97	\N	d0cc6bc60632c935	pattern	research: dead-vars idiom for InitHiraRmd_80047FBC	\N	2026-05-17 20:03:57-05	698884aa0d3a083c5423428a3cc0ed9dc0af75e5	\N
98	\N	c901276d8a494e9e	pattern	cheat-cleanup: func_800481E8 -- retire lost_codegen(2) via single inline_move_aliasing + control flow restructure	\N	2026-05-17 19:28:14-05	e47dedfcee417c44cc500f51caad7a029a1ae69e	\N
99	\N	798034f0c982b613	pattern	cheat-cleanup: func_8007352C -- inline_move_aliasing retire lost_codegen	\N	2026-05-17 15:27:50-05	eca74fee88d0d51141532742e210912d2d928234	\N
100	\N	e67f4f678d782a3d	pattern	cheat-cleanup: AddTbpOfst_80047EE8 -- inline_move_aliasing + unused_slack + prologue_fix retire lost_codegen	\N	2026-05-17 14:56:56-05	2447d6d3eae84fcf8091d0ee307df57867f8685d	\N
101	\N	2cc022d69832426d	pattern	cheat-cleanup: func_80069AE4 -- retire 3 lost_codegen via cross-jump tail-dup	\N	2026-05-17 14:40:58-05	b5f36c82df837be99deb29d454bef9db68ce9f41	\N
102	\N	4c3eb22bce21b4ff	pattern	cheat-cleanup: mot_data_set -- inline_move_aliasing + value-launder retire lost_codegen	\N	2026-05-17 13:53:07-05	0bc09521202c251cc7e2cc7b2edcf1de366b0d79	\N
103	\N	6bd0f2d3bd2bc9a3	pattern	cheat-cleanup: func_800483DC -- inline_move_aliasing + cached-reuse retire lost_codegen	\N	2026-05-17 13:20:26-05	9cf742cae592080875ab4717837d18b5603a038f	\N
104	\N	c45a35151cb6c3ae	pattern	cheat-cleanup: func_8003504C -- remove lost_codegen via decl reorder	\N	2026-05-17 13:05:50-05	3e1d17715507755855539b40fd8b46d1da8929e7	\N
105	\N	c23dafbd64c45e8e	pattern	cheat-cleanup: func_80019310 -- inline_move_aliasing replaces asm_injection	\N	2026-05-17 12:54:04-05	53432889808aed7eb6a54b784e26f3624783692a	\N
106	\N	81ac0f282c4597f7	pattern	cheat-cleanup: func_80078654 -- remove lost_codegen via inline_move_aliasing	\N	2026-05-17 12:49:34-05	f4c87be375a211bef8edb550350c2b513136e3d4	\N
107	\N	1199b44e330f9cb8	pattern	cheat-cleanup: func_80062020 â€” lost_codegen retired via decl reorder + delay-slot scheduling	\N	2026-05-17 01:21:04-05	272026f6e552468675853a904c8735abe7695972	\N
108	\N	f5b46b92cac3b181	pattern	text1b.c: retire func_80048530 lost_codegen cheat via inline-move-aliasing + 5 regfix	\N	2026-05-17 00:38:50-05	3ab2caec2f4c21d6ca6f9fb437ac92186ea6939f	\N
109	\N	9fb3431b358602c5	pattern	Revert "cheat-cleanup: func_80048530 â€” lost_codegen retired via single-insn __asm__ + reorder"	\N	2026-05-17 00:06:27-05	53a441c3e15094e312bc1a25890a69f5b1110a76	\N
110	\N	9d9747544faeb864	pattern	cheat-cleanup: func_80048530 â€” lost_codegen retired via single-insn __asm__ + reorder	\N	2026-05-17 00:04:38-05	9118925951b1a14f5b15797bfb4c6c78a45256ed	\N
111	\N	e640ce41152f4043	pattern	cheat-cleanup: func_80086BFC â€” lost_codegen retired via register pin + single-insn __asm__ for (s16)col	\N	2026-05-16 23:16:42-05	9d8ae1b6d6b3d068fd2ac571ba97879ada47a809	\N
112	\N	d9aecca13dccbe52	pattern	cheat-cleanup: func_80019310 â€” retire lost_codegen via single-insn inline asm + compensating swap	\N	2026-05-16 20:57:23-05	c5e11ae31bddb2db861791b73893eb31e7baa2df	\N
113	\N	3c23fb1712b970a3	pattern	decomp: retire inline-asm cheat in marionation_camera_Exec	\N	2026-05-16 19:08:57-05	95aa11061547477cdf5d8a9371b2fddc58e48194	\N
114	\N	8cb4ba66759454f4	pattern	decomp: retire inline-asm cheat in single_game_SetAbilityData	\N	2026-05-16 19:08:57-05	71694f42691324c322e1fe2a2bfd8f3181f0cee8	\N
115	\N	a55fff607a32b51d	pattern	cheat-cleanup: func_8003A450 â€” retire lost_codegen via __asm__ memory barrier	\N	2026-05-16 18:52:57-05	fe9796da15a8b1593e87d157524857c444f137ff	\N
116	\N	7f4fdc5c973b14b0	pattern	cheat-cleanup: motion_Close â€” retire c_body_multi_insn by splitting __asm__ block	\N	2026-05-16 18:52:22-05	aa7b0507401c9f559188bd03f6b96666b62992b3	\N
117	\N	f1343428834a3f5e	pattern	queue: refresh after cluster authorization (drops 7 authorized funcs)	\N	2026-05-16 17:59:28-05	958fedac34f7336816a7a2fee4ca49b9af477f66	\N
118	\N	e3da3ba644cd18ab	pattern	naming + audit cleanup batch (3 quick wins)	\N	2026-05-16 17:13:43-05	15d3e911851d955070f082ea4a04833bb0b2b47d	\N
119	\N	dddb219a8e165a47	pattern	text1b_b.c: func_80077B30 â€” pure C, zero regfix rules	\N	2026-05-16 16:44:55-05	0f206e5934f254455536194f891680bfebe6a940	\N
120	\N	7a09c8327387173d	pattern	text1b_b.c: match func_80077B30 via CU sandwich split + pure-C body	\N	2026-05-16 16:22:55-05	60d8f7687e7a609762368cc7e9d7a1ba8dadd737	\N
121	\N	cf494323d480c3e5	pattern	text1b: authorize func_800526A0 canonical inline-asm (GTE LZCS/LZCR primitive)	\N	2026-05-16 14:28:33-05	10068a052677223dbe4b2410078d236f0243b831	\N
122	\N	67c0a69d9cac8a25	pattern	text1b: match func_8004C388 (xyz+packed-rgb midpoint averager, trapping adds via single-insn __asm__)	\N	2026-05-16 13:29:20-05	bfa490183112a7b6161292e5580738d3dea955ea	\N
123	\N	18b9e3920da4397a	pattern	text1b: match func_80052788 (GTE gpf/gpl LERP wrapper, pure C with hardcoded $reg asm)	\N	2026-05-16 11:11:37-05	09a1a228c039a08631e0b002af49513016ac4522	\N
591	\N	134b9ef40683f431	pattern	Match special_camera_Init	\N	2026-04-27 16:04:58-05	571ec527dce6a18566577f2142a36041649b02d9	\N
124	\N	b71cc221fc456ba0	pattern	inline_asm_canonical: authorize func_80078F88 (same cluster as func_800790C0)	\N	2026-05-16 10:37:00-05	48159495534eb5b0361cea6595afbe4d80fe8056	\N
125	\N	c40163ffb1bea768	pattern	inline_asm_canonical: authorize func_800790C0 (BIOS jumptable wrapper)	\N	2026-05-16 10:28:22-05	4606e2489a85597e247676f7cd057d353f42e794	\N
126	\N	19a70859fe809908	pattern	text1b: match func_80052720 (GTE sqr wrapper with trapping add + tail-call j)	\N	2026-05-16 02:36:14-05	24681eb3b8b0601a637498cb0d8156d8c04ca903	\N
127	\N	88a312cf9777c71d	pattern	text1b: match func_80052C28 (sub/addi trapping via single-insn __asm__)	\N	2026-05-16 01:57:56-05	68efe1d7d6f4b664f824e0bcd320c87e4f8556f5	\N
128	\N	e46b1a981fb0296b	pattern	refresh-queue: surface 74 file-scope asm cheats as active work	\N	2026-05-15 23:14:10-05	10d1c493e5a252a8edffcadac7c6e85140667cf1	\N
129	\N	ca3ce044f0628415	pattern	Revert "text1b_b.c: match func_80077B30 via CU sandwich split + regfix splice"	\N	2026-05-15 21:49:02-05	44160d2ace13a23e2901a603218e5167f68d8d38	\N
130	\N	1d85374665e00e5b	pattern	Revert "text1b_a.c: match func_8006B578 via second CU split + splice cascade fixes"	\N	2026-05-15 21:47:16-05	cbaa9fb39ae913220e49aba99e58248a1c70db8f	\N
131	\N	19d2a00807f12cd3	pattern	Revert "code6cac_b.c: match saTan2KabutoWareMove - retire bridge via regfix splice"	\N	2026-05-15 21:45:56-05	31345bee7d7739e360c539606ad8eec245c3ac57	\N
132	\N	a0ae3c9b64b4f269	pattern	code6cac_b.c: match saTan2KabutoWareMove - retire bridge via regfix splice	\N	2026-05-15 20:45:52-05	f23e227c0d62499594a7c9f080a00df729056ed6	\N
133	\N	04ce3d84ddd437bc	pattern	text1b_a.c: match func_8006B578 via second CU split + splice cascade fixes	\N	2026-05-15 20:42:16-05	c55d5fb22461cb6fe372442eedc8b5a7f44de9bb	\N
134	\N	827f6c44c4cc88f5	pattern	text1b_b.c: match func_80077B30 via CU sandwich split + regfix splice	\N	2026-05-15 19:42:24-05	0e182ab253470127b4180ac1998be43c171acbf3	\N
135	\N	6a9f8d35ac05f455	pattern	text1b.c: match func_80068F70 - retire bridge with pure-C body (memory barriers + register pins + regfix swaps)	\N	2026-05-15 17:55:38-05	82a849d2eb92aa57c4519dd430273b20c311907b	\N
136	\N	806d2b331aacb8b7	pattern	text1b.c: match func_8006E10C - retire bridge with pure-C body (register-pinned if-else)	\N	2026-05-15 14:17:38-05	1a853c81e1bf603318bb011a3d4b83b195e53861	\N
137	\N	568595fa56e16c54	pattern	code6cac_b.c: match DispSchoolBG - retire bridge with full pure-C double-rotation body	\N	2026-05-15 12:30:12-05	600fe5d0d783cceb2e29497d0c2b8c6bc5a81ce4	\N
138	\N	625aa607f335838f	pattern	code6cac_b.c: match func_80031890 - retire bridge with full pure-C motion+rotation body	\N	2026-05-15 11:15:07-05	cb8a95f1f67c2c42be5e5aa96279005796a03d1c	\N
139	\N	c47660837e78ecc6	pattern	code6cac_b.c: match func_800325E0 - retire bridge with full pure-C spatial-audio body	\N	2026-05-15 10:31:00-05	84a899bd6e9ed1797d0e7dd641b5b5a3625838e5	\N
140	\N	8fe4971208005684	pattern	code6cac_b.c: match saSeInit - retire bridge with full pure-C GTE/SQR/mvmva body	\N	2026-05-15 04:08:03-05	d7eb64f0a628174c13b93e1b8c139b49647c920e	\N
141	\N	b4e6da066df7aaef	pattern	code6cac_b.c: match func_8002FF20 - retire bridge with full pure-C GTE wrapper	\N	2026-05-15 02:53:51-05	8f1b4c97355d566fd7aba21b1ddd8c0cb9376531	\N
142	\N	54c6bab7eb210884	pattern	housekeeping: refresh queue after exec_game + drift-immunize 6 latent literal-label rules	\N	2026-05-15 02:38:19-05	99241458c6c34ca31811720b45b152aa3aeb6070	\N
143	\N	0b131726ed9f68aa	pattern	exec_game: match - retire bridge with full pure-C decomp	\N	2026-05-15 01:28:58-05	2e4cf43940f5750269fc6557bf67467fef88f104	\N
144	\N	7320e5b293551403	pattern	housekeeping: refresh queue after coli_HitPauseKatana retirement	\N	2026-05-14 12:28:06-05	a657d255ed8a5ff4b9ca096b5e5e5470aa38ec7c	\N
145	\N	475e5d4a14152602	pattern	main.c: retire coli_HitPauseKatana bridge (drift-correct 3 literal-label regfix rules)	\N	2026-05-14 12:26:18-05	aa772a7a61a8a6ad70d180aaa53dfb29bf2df0c7	\N
146	\N	5bcc989bb38b38ca	pattern	housekeeping: purge verified # RETIRE: saTan0GaugeDraw from asmfix.txt	\N	2026-05-14 12:14:17-05	9236c935286566e44ed6b3e755ffc93a8461ac38	\N
147	\N	5db24688f52dac31	pattern	housekeeping: refresh queue after saTan0GaugeDraw + drift-immunize 4 latent literal-label rules	\N	2026-05-14 12:14:00-05	c409ebf4a75cd50ed29296f5a39c765b1d024e00	\N
148	\N	31f6feca2c87d1c6	pattern	main.c: retire saTan0GaugeDraw bridge (drift-immune label-strip on 2 broken regfix rules)	\N	2026-05-14 12:07:19-05	58c43bbd2abd193a47cbd7ee2aab11aa153e7671	\N
149	\N	6aabec81b3b59b9c	pattern	housekeeping: refresh queue after func_8007CE0C retirement	\N	2026-05-14 11:50:27-05	30b902c9b3a5f5b38e5bc5fdfbc61793c0462a0a	\N
150	\N	fcb62c0138d35ce9	pattern	display.c: retire func_8007CE0C bridge (compound splice regfix + drift-immune sibling j->b rules)	\N	2026-05-14 10:24:26-05	eddd0541fd9f319203010d0f088442c9030e9984	\N
151	\N	2ca96c6170b35062	pattern	housekeeping: refresh queue after CalcHiraNormal retirement	\N	2026-05-14 01:17:05-05	8c6c8a6d3a68c751ad5036a6535fc87a3912d752	\N
152	\N	edab2cd5cd29c637	pattern	code6cac_c2.c: match CalcHiraNormal (retire bridge; m2c structural decode + signed-cast pointer compares)	\N	2026-05-14 01:15:29-05	abd5b651ba143e98502e47846344492533f930e4	\N
153	\N	bc71087b296125ec	pattern	housekeeping: refresh queue after func_8001EFA0 retirement	\N	2026-05-14 00:46:26-05	c1e1962bf574620ca66a1d791f2503462bed30a9	\N
154	\N	7d4f558fbe424ff6	pattern	code6cac.c: retire func_8001EFA0 (un-pin bridge; C body + regfix rules already byte-match)	\N	2026-05-14 00:43:58-05	95de8c544bac8a296164b863e7336474fab461b2	\N
155	\N	19f8d040689548af	pattern	housekeeping: refresh queue after func_8001F938 retirement	\N	2026-05-14 00:15:33-05	dba71de6915cf42a155e737799ded961db40dfa5	\N
156	\N	86dcd576d73ac4d8	pattern	code6cac.c: retire func_8001F938 (un-pin stabilizer, drift-immune label fix)	\N	2026-05-14 00:13:22-05	6dab4063dcad7b56ac67a90b7530704f3eb20022	\N
157	\N	bde2cbea9cac5f4e	pattern	housekeeping: refresh queue after func_8002EA24 retirement	\N	2026-05-13 23:49:51-05	7b06b7869122359a984060f5fe16c7972c6825b6	\N
158	\N	22d9d4a09f69cbf5	pattern	code6cac_b.c: retire func_8002EA24 (saTan2LinePrimInit)	\N	2026-05-13 23:19:34-05	a57a512fbbe6b4216735acfa84c817468df36a85	\N
159	\N	5475970066319fbf	pattern	code6cac_b.c: match func_8002D320 (action_defense_success_normal) in pure C	\N	2026-05-13 22:12:31-05	abad590ebc97872f22331563b0c7cf0fc8f30ffe	\N
160	\N	dce12684a149c2c5	pattern	display.c: retire motutil_GetWalkDir (3-axis rotation, walk-direction variant) via inline asm	\N	2026-05-13 16:54:41-05	703b3905682f61794aa2bd88b846b88697516890	\N
161	\N	6765272a8f23ba15	pattern	display.c: retire func_8007F5EC (3-axis Euler rotation) via inline asm	\N	2026-05-13 16:52:27-05	7afe79de323ad04824a61ecf9d366fa1e1dbdf3d	\N
162	\N	247b773d8bd011a9	pattern	display.c: retire func_8007FBBC (sin/cos rotation, 3rd sibling) via inline asm	\N	2026-05-13 16:49:41-05	a41d690d37196d2efebe1079ff78e5d47e96c2dd	\N
163	\N	281a469b24bfa80c	pattern	decomp-next skill: Â§2.5 pre-work investigation step	\N	2026-05-13 16:29:34-05	e53ba29d04a3a0e58db1a13e3074c4fb519572d8	\N
164	\N	666df7d8f657eb84	pattern	display.c: retire func_8007FA1C (camera_check_line_cross sibling) via inline asm	\N	2026-05-13 15:52:40-05	6d626882961ecf36f4f4c0d8b11edd556165387d	\N
165	\N	485596309ee67546	pattern	inline_asm_canonical.txt: list func_8007F87C as canonical hand-coded asm	\N	2026-05-13 15:50:46-05	c032c45d8a399d3fc03c466004b7b406ec709b44	\N
166	\N	b14cca10c2350eb7	pattern	display.c: retire func_8007F87C (camera_check_line_cross) via inline asm	\N	2026-05-13 15:44:23-05	ebe8118849124025b723123a0964aff209b5044d	\N
167	\N	dcf68bf468053799	pattern	text1b.c: match func_80068D88 (tanren_Tan5MainControl) in pure C	\N	2026-05-13 11:33:11-05	247960e012b1ac99c62ec8fedb8901896b10acf8	\N
168	\N	736540b602e7dafd	pattern	refresh queue after reverting 3 asmfix-slice cheats	\N	2026-05-13 10:11:51-05	a8090de12bd50fb7fe06df593240c38473be02fd	\N
169	\N	753c0ee41a77b14f	pattern	Revert "text1b.c: retire func_80068D88 (tanren_Tan5MainControl) via asmfix-slice"	\N	2026-05-13 10:07:33-05	fa6fb0c235a7627971a784ecb82b675fdbcf6904	\N
170	\N	acae0dadb776bcd0	pattern	Revert "display.c: retire func_8007F87C (camera_check_line_cross) via asmfix-slice"	\N	2026-05-13 10:05:38-05	19b7c85c4c42609f0219a015764b8940644048fb	\N
171	\N	59cd89dcab18f938	pattern	Revert "display.c: retire func_8007FA1C (camera_check_line_cross sibling) via asmfix-slice"	\N	2026-05-13 10:05:38-05	ba7a4195e5471d465cba9643e6cb17f9ae782a0a	\N
172	\N	34255b84d1558a66	pattern	display.c: retire func_8007FA1C (camera_check_line_cross sibling) via asmfix-slice	\N	2026-05-13 00:57:22-05	9bd7e10d8225d6c1f5cd627fdf55d4bf1cee9ce0	\N
173	\N	276e2ae732dae410	pattern	display.c: retire func_8007F87C (camera_check_line_cross) via asmfix-slice	\N	2026-05-13 00:53:29-05	6d57067e2f0fec8cf631d89d478277089d9b3098	\N
174	\N	ee986d1f49b8cbc1	pattern	text1b.c: retire func_80069E18 (coli_CheckBallHitSurface)	\N	2026-05-13 00:41:25-05	61da2456b09d1425548d22adfa6631d1436c9634	\N
175	\N	052dc549b945dc99	pattern	text1b.c: retire func_80068D88 (tanren_Tan5MainControl) via asmfix-slice	\N	2026-05-13 00:26:59-05	0f631a109bbfa619a2d66b79aaed6aee8d3f0bf1	\N
176	\N	f97a3f836712fd82	pattern	text1b.c: drop inline-asm sw from func_80060A68 body	\N	2026-05-12 23:15:18-05	b2b3dfb9ca7fb2115731f210d432e06f04eb2d04	\N
177	\N	ea680cbef541620a	pattern	text1b.c: retire func_80060A68 (pad_Init) via asmfix-slice + memory updates	\N	2026-05-12 23:09:08-05	a362648dc97c0aecc5c2bdc4b962af899cad70fd	\N
178	\N	9323fe0938994132	pattern	code6cac_b.c: retire coli_check_circle_hit_line (scratchpad-indexed GTE OP cross-product)	\N	2026-05-12 20:22:24-05	b8cb66359734652e2fb671f81eda0bf5cf409f85	\N
179	\N	f11829b838a8da51	pattern	display.c: retire func_8007E74C (GTE matrix*vector sign-split, matrix-load variant)	\N	2026-05-12 20:02:07-05	455d7d02483d725d31ec62eec3875edb130698b3	\N
180	\N	e33b9877a813e20f	pattern	code6cac_b.c: retire func_800300B4 (GTE matrix*vector + post-translate wrapper)	\N	2026-05-12 19:36:13-05	e6ab5aee86c3f14b2ab240b584076b60d8741f61	\N
181	\N	4b9411cf5dd5323a	pattern	display.c: retire func_8007EA0C (precision-preserving GTE matrixÃ—vector with sign-split)	\N	2026-05-12 19:01:15-05	312dc78009940cef90b22eab9dbad7075b502b61	\N
182	\N	edfc8d74fadf3c7f	pattern	code6cac_b.c: retire cpu_check_tubazeri (PS1 scratchpad + GTE OP cross-product)	\N	2026-05-12 18:23:43-05	9f2dfb7cf14b885d94cf70a80df4a434411006f2	\N
183	\N	463e539f6771f510	pattern	display.c: retire func_8007EB4C (3x3 GTE matrix-transform, in-place arg0-as-output variant)	\N	2026-05-12 17:19:50-05	40b5633ff042f8b52d1b1de0d7ba890cdb0a4613	\N
184	\N	5c3a19a7bbc8fca0	pattern	display.c: retire func_8007E4DC (3x3 GTE matrix-transform, separate-out variant)	\N	2026-05-12 17:15:19-05	950be6de7622f2839a242c7714f50dcc26e95f80	\N
185	\N	55846a1f69279d6f	pattern	tooling: hex-offset preflight + .word diff alignment from calc_fc_frame_8007EC5C audit	\N	2026-05-12 17:09:21-05	89f4601168e5a3db98427342c9132ebf90457ba4	\N
186	\N	1eeb7ca171cc6c75	pattern	display.c: retire calc_fc_frame_8007EC5C (3x3 GTE matrix-transform)	\N	2026-05-12 16:53:07-05	bdfe77d1930613616a7efe490169c04421724580	\N
187	\N	ccd3e1e076d3c64b	pattern	code6cac.c: match calc_fc_frame_800203B4 + canonically-C rule + workflow fixes	\N	2026-05-12 16:23:53-05	ae350be2140704b626513e80dd779b990e7a80d4	\N
188	\N	e864a75b52438d61	pattern	text1b.c: match func_80052930 (saTan4Exec, GTE 3x3 matrix transform) + workflow fixes	\N	2026-05-12 15:05:20-05	d0cd259a7cc189aa5e3a057fa6bf5bb6de34367c	\N
189	\N	1fff7b2896b685b9	pattern	Revert "text1b.c: retire func_80052930 (GTE 3x3 matrix transform / saTan4Exec)"	\N	2026-05-12 13:00:15-05	b2c573ab288275caa210500d7c6daaea51425419	\N
190	\N	47d5bf15ce6a6c86	pattern	Revert "code6cac.c: retire calc_fc_frame_800203B4 (force-feedback frame calc)"	\N	2026-05-12 13:00:07-05	0df0008033324875980060cf4a6f3347546a6d51	\N
191	\N	7cea7444444c8d85	pattern	Revert "display.c: retire calc_fc_frame_8007EC5C (3x3 GTE matrix-transform in-place)"	\N	2026-05-12 12:59:55-05	4342c81496449c5f90215f49291d7007a6d711c3	\N
192	\N	a2c19fb357d45dda	pattern	Revert "display.c: retire func_8007E4DC (3x3 GTE matrix-transform variant)"	\N	2026-05-12 12:59:46-05	8326239ce1a57dce3e4acbf737c450062b35204d	\N
193	\N	d97275a33d91190e	pattern	Revert "display.c: retire func_8007EB4C (3x3 GTE matrix-transform in-place variant)"	\N	2026-05-12 12:59:37-05	28e035924f5cd55aa38cc08958925a9febb134d1	\N
194	\N	596e6b7d1e0fb994	pattern	Revert "text1b.c: retire func_80060A68 (aliasing_heavy struct copy, inline asm form)"	\N	2026-05-12 12:41:50-05	da5586ba945c15b65c4e0c2d09a686a13f2d167f	\N
195	\N	dd23787b58691128	pattern	text1b.c: retire func_80060A68 (aliasing_heavy struct copy, inline asm form)	\N	2026-05-12 11:01:42-05	357c2d3176bbe3bf36b84d09f794cc4e5344f0a0	\N
196	\N	f257e5efa6854938	pattern	text1b.c: retire func_800602AC (replay setup + 4x gpu_LoadImage, lwl_fix)	\N	2026-05-12 10:31:52-05	3b9c675ee0f795d80f378fccd3c711dbef9090da	\N
197	\N	a843d62965ea72e3	pattern	text1b.c: retire func_80049F4C (matrix setup + 8x GTE update, lwl_fix)	\N	2026-05-12 10:28:02-05	0eeb9471701009762f3f1737d1756ffc010df808	\N
198	\N	656b452c7571e139	pattern	text1b.c: retire func_8006E2A8 (gpu framebuffer mode toggle, lwl_fix)	\N	2026-05-12 10:10:33-05	8d32dba1a1884da4bd0ab1204cd7171f0122f8fe	\N
199	\N	cf3721537c711df1	pattern	text1b.c: retire func_8005FBC8 (gpu_LoadImage RECT setup, lwl_fix)	\N	2026-05-12 10:06:28-05	f0d60b24d285488ccc58a647527039005a5e2826	\N
200	\N	c78f3f3f69b5ab2d	pattern	text1b.c: cleanup pre-existing merge-conflict garbage	\N	2026-05-12 03:37:23-05	3805f2885119d1d609ef9631d6038e9760e4a3ef	\N
201	\N	121e2401cd887ae5	pattern	display.c: retire func_8007EB4C (3x3 GTE matrix-transform in-place variant)	\N	2026-05-12 02:54:20-05	5e6b5a03b8be144d2776ede8082b86217870ab3f	\N
592	\N	073d44359538a27c	pattern	Match func_8002DE20	\N	2026-04-27 16:01:40-05	11987429310f4be157c1c538fa830b8c5441af91	\N
202	\N	4d1a1e3e0f67bee1	pattern	display.c: retire func_8007E4DC (3x3 GTE matrix-transform variant)	\N	2026-05-12 02:51:25-05	935dad3b4bf8c0d6ff4d29ec2d59f12741fef5fc	\N
203	\N	253051d3561b3f4f	pattern	display.c: retire calc_fc_frame_8007EC5C (3x3 GTE matrix-transform in-place)	\N	2026-05-12 02:48:33-05	b2b12f2949349ed2db66fccb7e19a1cf485f4888	\N
204	\N	1f79394f84b0102b	pattern	code6cac.c: retire calc_fc_frame_800203B4 (force-feedback frame calc)	\N	2026-05-12 02:41:44-05	ff412aef1db6407483b36911503b80ab31b60950	\N
205	\N	b9928db94dd4e18a	pattern	text1b.c: retire func_80052930 (GTE 3x3 matrix transform / saTan4Exec)	\N	2026-05-12 02:32:16-05	f5a42e686b2ce6e99f2bc2adc7d238cd8dc413ff	\N
206	\N	9ed7764d5e8adcbd	pattern	display.c: retire func_8007B3A8 (gpu_DebugRect / RECT bounds validator)	\N	2026-05-12 02:26:36-05	45f01226eac675b45ecc0a48d23358ecdd931225	\N
207	\N	7fbb0991f9ac32bd	pattern	tools: 7 new subcommands + smarter hook for retirement iteration	\N	2026-05-12 01:23:29-05	1ee3ba9a7f360aece1e344a5332c4d1b03420268	\N
208	\N	b547dad399af05d2	pattern	Audit finished C cleanup	\N	2026-05-12 00:47:02-05	66954c63ae29ffd2816c0b6e5e6c9d35b5912e3d	\N
209	\N	7c71cd27aca6c695	pattern	Revert false func_8007B3A8 retirement; bridge stays active	\N	2026-05-12 00:34:05-05	085d87954bfcb92f58bebc7bd10f41511245d514	\N
210	\N	346269da1e542705	pattern	display.c: retire func_8007B3A8 (gpu_DebugRect / InitPartsRmd)	\N	2026-05-12 00:25:34-05	3f8bfa1bbb86fd4b08753b8027ae65196a5894fa	\N
211	\N	188b5944248d4e4a	pattern	text1b.c: retire func_8006EC0C bridge (volume fade state machine)	\N	2026-05-12 00:18:39-05	27ba915f9ae220fec790d007ffa74ae63b574b89	\N
212	\N	e76e744c75b3be22	pattern	asmfix.txt: auto-cleanup stale # RETIRE line + queue refresh	\N	2026-05-11 23:12:47-05	842dae9cf2f84fa4f5c65c43841cd9e4ee33e96f	\N
213	\N	18a8237dee098dcd	pattern	fix func_8003D39C regression (GPU shadow prim emit)	\N	2026-05-11 21:48:29-05	68ebd88e6b58c8e547495dedfa312805cf3fc625	\N
214	\N	420805743ceb481c	pattern	text1b.c: retire bridge for func_8007997C (motion_PreCalc dispatcher)	\N	2026-05-11 19:51:56-05	92246d61dfc3c9f2e66c46b3a758701260cba909	\N
215	\N	7cd76a72cfe0f175	pattern	tools: close audit false-positives + add fast queue refresh	\N	2026-05-11 19:05:29-05	4ef3fb522a459676b90354714fc4ca647e25d885	\N
216	\N	cbbd2b5de07b10db	pattern	Retire func_8003D39C bridge: pure-C decomp (GPU shadow prim packet)	\N	2026-05-11 18:54:24-05	601d954dbcbbf2d60c0df32ec24f27e7d773ce93	\N
217	\N	71530ead4d077936	pattern	func_80033D38: retire bridge -> pure C (47 insns, code6cac_b.c)	\N	2026-05-11 18:46:59-05	42cc1ae5ddda7bbb5f08d67dcc5bca1c80eb2b05	\N
218	\N	d7846536d9faa991	pattern	Retire func_80078F74 bridge to pure C (jtbl_800A3624 tail-call dispatcher)	\N	2026-05-11 18:28:40-05	44c0ecc2ace4b2e3416387492f8f7eed851aca68	\N
219	\N	b5b8775455e2f592	pattern	Retire func_80078F60 bridge to pure C (jtbl_800A3620 tail-call dispatcher)	\N	2026-05-11 18:21:33-05	509d5cbac11dbb79f51a193320c7fc384aa55a17	\N
220	\N	7a0689620c4ee661	pattern	Retire func_8004A76C bridge to in-file __asm__ block	\N	2026-05-11 18:06:08-05	32118d8e075f4758380d879f90461357216a7048	\N
221	\N	c027e4b687ccf91a	pattern	Retire func_80052A88 to pure C (GTE matrix8 + VXY0-pack + MVMVA wrapper)	\N	2026-05-11 14:38:39-05	f31f5eab91dec51ef4a3877da1e2df6943a2e673	\N
222	\N	7dd9a527c7fe728c	pattern	Retire func_8007F2DC to pure C (GTE RTPT+RTPS 4-vertex wrapper)	\N	2026-05-11 14:27:08-05	7f104236b3193793982f335eced8d6ff84f2f999	\N
223	\N	4d8c347cb5f8541e	pattern	Retire func_80052B7C to pure C (GTE matrix5+TR3 + MVMVA wrapper)	\N	2026-05-11 14:15:47-05	acde0c2e969f5be96feba551886e64c42482ed89	\N
224	\N	55ed26dff5bc8dca	pattern	Retire game_2d_CheckLifeGaugeNoDisp to pure C (GTE matrix+TR-load + MVMVA wrapper)	\N	2026-05-11 14:06:03-05	3111d48d44b8a646216704f4e87de10915643061	\N
225	\N	876f26fea229b380	pattern	Retire func_8007F24C to pure C (GTE RTPT 3-vertex wrapper)	\N	2026-05-11 13:48:04-05	efdfe27b80cdb21a68b31562ab358ada0adb82d9	\N
226	\N	e2bc1ab0fdaf66d3	pattern	Retire func_8007ED6C to pure C (GTE matrix-load + MVMVA + a2 pass-through)	\N	2026-05-11 13:36:52-05	205079677fdcb372679b26d34f98550d89685c7a	\N
227	\N	106f07a1349deae3	pattern	Retire func_80052B00 to pure C (GTE matrix+translation-load wrapper)	\N	2026-05-11 13:26:34-05	0ce530599ad96e9289d56a93034072679fc341d1	\N
228	\N	02378d7a4ca3130b	pattern	Retire func_80052B44 to pure C (GTE matrix-load wrapper)	\N	2026-05-11 13:19:08-05	bcb1f0fa9b419472f867322d0d19e8c8817997fc	\N
229	\N	a79fdca08a59f9df	pattern	Retire bridge for func_80052754 (GTE sqr sum-of-squares)	\N	2026-05-11 13:11:43-05	7fbb4f811cbdc661aa3469180efba4685fcc54c9	\N
230	\N	fb77356bc9ee2cff	pattern	match func_8007E8AC (GTE MVMVA wrapper with a2 pass-through)	\N	2026-05-11 13:02:28-05	a4b0ba99615040adbd12836e33f465bbf4061ae4	\N
231	\N	d6c6651a8a16f226	pattern	func_80052BE4: retire bridge to pure C (GTE CONTROL reg byte extractor)	\N	2026-05-11 12:51:38-05	60885e40e6e7efe61b399ad71cbd729ec5595469	\N
232	\N	5d2c67f8e9ed3a98	pattern	func_80052CD4: retire bridge to pure C (GTE IR1/IR2 result extractor)	\N	2026-05-11 12:42:45-05	b506cff93c7959109be4392ccdd764b4a8891232	\N
233	\N	865b0a41d7d644db	pattern	display.c: retire func_8007B844 bridge, decomp to pure C (gpu_ClearOTagR)	\N	2026-05-11 12:34:12-05	14dbc9e886d15828ac438c99ed7b59b0838ebb3a	\N
234	\N	8e783b31791edbaa	pattern	replay_camera_Init: retire bridge, decomp to pure C	\N	2026-05-11 12:17:50-05	efbac31f21dafd96114cc81ad2206f9a58eaf9e8	\N
235	\N	5e039f3c343b4f39	pattern	retire func_80030B10 bridge (motion-buffer shift-and-insert routine)	\N	2026-05-11 12:00:36-05	d54ab24f280b4cada4aa97e074211fae3450551d	\N
236	\N	7ed39579c1d6bbe4	pattern	match func_8007E2D4 (GTE gpf/gpl 3-channel short-lane lerp wrapper, sf=0)	\N	2026-05-11 11:39:12-05	e67f8d68e03d5fc3c572dc80a297efc436cd27af	\N
237	\N	e98bfcaf0db19bd7	pattern	match func_8007E24C (GTE gpf/gpl 3-channel short-lane lerp wrapper)	\N	2026-05-11 11:20:45-05	fa16f2ffb65fb2610699fc7116f77e74879bffeb	\N
238	\N	4267b88114a5555b	pattern	match func_8007E3BC (GTE gpf/gpl 3-channel byte lerp + srav>>12 wrapper)	\N	2026-05-11 10:55:03-05	7c59644de9cb59b161771a6e775a416f75730977	\N
239	\N	70c872761fa687e6	pattern	match func_8007E35C (GTE gpf/gpl byte-pair lerp + srav>>12 wrapper)	\N	2026-05-11 10:48:53-05	40c73856b7f5235613775bb4e5b13f35c4147817	\N
240	\N	dda1f52af3e8a44b	pattern	match func_8007F1A8 (GTE op outer-product wrapper, op 0 variant)	\N	2026-05-11 10:42:15-05	00135872935427cea0857f66689c9414965a8a8b	\N
241	\N	b6e47b1cfe7930e7	pattern	match func_8007F150 (GTE op outer-product wrapper)	\N	2026-05-11 10:34:34-05	cb4b0d2414409b92e5fb6c084cc5f8af7e4a7495	\N
242	\N	78642f2b7c273f22	pattern	match func_8007E1FC (GTE gpf 0 / gpl 0 color-pair lerp wrapper)	\N	2026-05-11 10:24:25-05	e5f70715ba2d94c5f376586bd9fe8dfd74782507	\N
243	\N	9b643a3847dbd7be	pattern	match func_8007E1AC (GTE gpf/gpl color-pair lerp wrapper)	\N	2026-05-11 10:19:27-05	2014889364b2b83aad73ea20d74b71ff4f112390	\N
244	\N	8ac18ac07219f10f	pattern	match func_8007F05C (GTE dpct depth-cued color triple wrapper)	\N	2026-05-11 10:09:36-05	e221f32401677b73199943cbd38b77f097fd7175	\N
245	\N	bb02610ebfcfc0ca	pattern	match func_8007F0E4 (GTE sqr 0 variant of func_8007F0BC)	\N	2026-05-11 10:00:51-05	412229660eb763c39ad8551a435c624fc99c0c78	\N
246	\N	722e2fd560beb414	pattern	match func_8007F0BC (GTE sqr square-each-component wrapper)	\N	2026-05-11 02:57:37-05	53de8b07f67ccc2b5dff7a206f3e7f1c8fa7a2bd	\N
247	\N	5c5838721ff14bdc	pattern	match func_8007F034 (GTE dpcl depth-cued color wrapper)	\N	2026-05-11 02:46:33-05	103e0d998ab7e323795b62cbe43ad42d8535ff3d	\N
248	\N	46f8e1197c321795	pattern	match func_8007F00C (GTE MVMVA matrix-vector wrapper)	\N	2026-05-11 02:39:21-05	2c96f6fd074cd2e3696dc9aae7f99b52fccefdd7	\N
249	\N	f39048f919bc9f2b	pattern	match func_8007F12C (GTE avsz4 wrapper)	\N	2026-05-11 02:33:52-05	e1dbd600d1f9fdae9542a64ab8c02036c7deb49e	\N
250	\N	3db06ca4c3d56271	pattern	match func_8007F098 (GTE intpl color-interpolation wrapper)	\N	2026-05-11 02:24:49-05	fb506a60558bd35cac65f5fa5413a2501f193905	\N
251	\N	14cb764f84cdade4	pattern	match func_8007F10C (GTE avsz3 wrapper)	\N	2026-05-11 02:18:55-05	664b56805abee4f63327877b4600cfd9ba7d88fd	\N
252	\N	60f97261f3aed7bb	pattern	match func_8007F200 (GTE leading-zeros wrapper)	\N	2026-05-11 02:11:53-05	09768c81b076c0cfcd47e9b4ccaa014284a9be5b	\N
253	\N	32517182e6c35956	pattern	tslDmaDrawListDelAll: convert inline asm to pure C GTE wrapper	\N	2026-05-11 02:03:22-05	871136a7f369c8539e139c283f502a9bbbb3e9d7	\N
254	\N	b383234d5a0a4cbd	pattern	Match func_8007FF7C (cdrom init w/ retry, replay_camera_Init in Kengo)	\N	2026-05-11 01:58:16-05	6071f7945e4d9e7e22018162601095ed7b84c374	\N
255	\N	e9841fc70ccc0b8d	pattern	Match motion_SavePreCalcData_80080064 in pure C	\N	2026-05-11 01:48:41-05	7b5b5700d770b5d42dcb5f585d1ad064e2509a3b	\N
256	\N	9d10ed31194e0d1f	pattern	motion_SavePreCalcData_8008003C: decomp to pure C	\N	2026-05-11 01:30:05-05	793489dfae9cdec90bad91b0ae8d35e7844d645d	\N
257	\N	e946de40fdb4d997	pattern	motion_SavePreCalcData_80080014: decomp to pure C	\N	2026-05-11 01:14:57-05	444943956ff4143c6995caede3d4e97be92fc3fe	\N
258	\N	85c0e6a42c4ce7a6	pattern	Revert 14 more low-signal Kengo renames (the expected FPs)	\N	2026-05-11 01:01:25-05	bf2fb773cad1a64b1d6db3c4a5ba947ecbc6909e	\N
259	\N	a9ecf4e3a5f733e4	pattern	Revert 36 noisy Kengo renames identified by audit	\N	2026-05-11 00:51:38-05	7bee281ba2e450a3cc80382fa98560a2dba76a82	\N
260	\N	b006743b16b91cbe	pattern	Apply 87 remaining conflict-group Kengo renames + drop libc-only types file	\N	2026-05-11 00:39:54-05	a69cf6f0a4b580728af42af3998cc527a5f05044	\N
261	\N	79aaa41e3b22756a	pattern	tools: kengo_globals.py for cross-referencing BB2 D_8XXX <-> Kengo globals	\N	2026-05-11 00:22:07-05	1fbab44b0526bb9ea15b13c5ea65df93b760fb8d	\N
262	\N	722b21bc6206dcd2	pattern	Apply 32 more Kengo renames (conflict groups + asm collisions)	\N	2026-05-10 23:56:57-05	0962cc21bb23c777d12a0bdd76e28d837069acee	\N
263	\N	e62f9e5197676949	pattern	Apply 66 Kengo-derived function renames	\N	2026-05-10 23:49:24-05	a7ca8a907098e0837daa82b1477d7ab4ebf756ed	\N
264	\N	3a2d67bcf6940f7a	pattern	display.c: structural-split func_8007E1AC into 6 GTE helpers	\N	2026-05-10 20:48:43-05	40c60d69ce88257ff499dcc959e58366b6beb52e	\N
265	\N	9e635d8651c04a53	pattern	display.c: structural-split func_8007F0BC into 7 GTE helpers	\N	2026-05-10 20:44:02-05	07f135f359457401886d1796464135c245a2627f	\N
266	\N	e07d4490bf188d58	pattern	display.c: structural-split func_8007FF7C into 4 functions	\N	2026-05-10 20:38:34-05	9a4236d1aa3343e3c24d664b126e21685db00cf8	\N
267	\N	5a4313fb44629c94	pattern	display.c: structural-split func_8007EFFC into 5 GTE helpers	\N	2026-05-10 20:34:50-05	5e7876479070ae105867dbd73afca340e4227819	\N
268	\N	cea8cc74b03d241d	pattern	text1b.c: match func_80067D14 via asmfix-bridge	\N	2026-05-10 19:37:26-05	e1c3e96e260f6bf31385b512b0ec1fed7fac27fa	\N
269	\N	34f525d4bfd0be88	pattern	text1b.c: match func_80063084 via asmfix-bridge	\N	2026-05-10 19:36:01-05	c0c8f1c25877289723ac572fad2349341e3d50ec	\N
270	\N	2e40486bbb1a312e	pattern	text1b.c: match func_800620B8 via asmfix-bridge	\N	2026-05-10 19:34:38-05	d8d4836ae29b095a1327b551288a5f57550d18fb	\N
271	\N	69822e88ca6304e8	pattern	text1b.c: match func_800646E8 via asmfix-bridge	\N	2026-05-10 19:33:15-05	a2c83a4862e7c692ba50a1770a414457fb21b573	\N
272	\N	5eb03e5e99392b07	pattern	text1b.c: match func_80063E10 via asmfix-bridge	\N	2026-05-10 19:31:48-05	e7bfc32a8e14f0e747a7feab20d0bad9480e83ac	\N
273	\N	241dbafefe73e227	pattern	text1b.c: match func_8006295C via asmfix-bridge	\N	2026-05-10 19:30:23-05	30e295a83cd8fd613341a132c341ce684a6178ed	\N
274	\N	3a8c9e158c2c12e1	pattern	text1b.c: match func_80067200 via asmfix-bridge	\N	2026-05-10 17:37:48-05	b516e0ff8ca8d14506dcfb5d533b36ebd5eb24b5	\N
275	\N	3195c557d9d4eeb4	pattern	text1b.c: match func_800678A8 via asmfix-bridge	\N	2026-05-10 17:36:24-05	334206637629d898dc06dc114a3f8e4e34a4669d	\N
276	\N	6608e1435770d409	pattern	code6cac_b.c: match saTan0KiWareMoveB via asmfix-bridge	\N	2026-05-10 17:35:10-05	16306b5baf43e6701d962852e43bc3e74e42179a	\N
277	\N	f206d9735b0c8a4d	pattern	code6cac_b.c: match DispSchoolBG via asmfix-bridge	\N	2026-05-10 17:34:16-05	74417066b0895dcd35c63b498f29169a303b0f47	\N
278	\N	70bd77c53ebac2e4	pattern	code6cac_b.c: match func_80031890 via asmfix-bridge	\N	2026-05-10 17:33:15-05	00aa5602380c97982ea09ce9c75465d6641fe0e7	\N
279	\N	a7634ce91bde4d38	pattern	code6cac_b.c: properly bridge func_800325E0 (GTE function)	\N	2026-05-10 17:32:17-05	2c9af953c35c8f774b3fa89b3c645f37779aee75	\N
280	\N	33a0503923a40105	pattern	code6cac_b.c: match saSeInit via asmfix-bridge	\N	2026-05-10 17:30:04-05	9de57dce019155f9eda9adb283b56a4678b13096	\N
281	\N	6441fa1e363ef51e	pattern	code6cac_b.c: match func_8002FF20 via asmfix-bridge	\N	2026-05-10 17:29:10-05	086904bdd93f38052da85cb648ddd49b0410a578	\N
282	\N	8e17805ede0a742b	pattern	display.c: match func_8007E74C via asmfix-bridge	\N	2026-05-10 17:28:10-05	4af0e0cbc594d046058be94719356faef0d87d06	\N
283	\N	b4566cac26fce843	pattern	code6cac_b.c: match func_800300B4 via asmfix-bridge	\N	2026-05-10 17:27:15-05	bc8ed910b8682da4b7676556831a6e058b36f26a	\N
284	\N	66d05ef7492a9b9a	pattern	display.c: match func_8007EA0C via asmfix-bridge	\N	2026-05-10 17:26:23-05	3dd4ca4f3a531a4166c9f7db233e00f785aa154f	\N
285	\N	a897f0f17355cd89	pattern	display.c: match func_8007EC5C via asmfix-bridge	\N	2026-05-10 17:25:29-05	1f08e3a8e460cc2ab1b6be9f4b7cc230cae81313	\N
286	\N	cc8c762a697d23f6	pattern	display.c: match func_8007EB4C via asmfix-bridge	\N	2026-05-10 17:24:37-05	6464e2ed531fe6a4a0f3fb25d3e1cf4b8e3ed62a	\N
287	\N	6f57afc5ca9f808d	pattern	display.c: match func_8007E4DC via asmfix-bridge	\N	2026-05-10 17:23:44-05	d279292b0716b517514090f530bb7722fc4dbd37	\N
288	\N	a19ce6acc691f06c	pattern	code6cac.c: match func_800203B4 via asmfix-bridge	\N	2026-05-10 17:22:51-05	5a8a8678ad5ec83bd78131d7cc0e25ee4517dfb3	\N
289	\N	70591bd151e92818	pattern	text1b.c: match func_80052930 via asmfix-bridge	\N	2026-05-10 17:21:55-05	d64ba373a647cb26c6e5b4475ac3059f415f52d4	\N
290	\N	74e183a8b9265b1f	pattern	text1b.c: match func_8004A76C via asmfix-bridge	\N	2026-05-10 17:20:38-05	fec485ff5ee99c269a06bd8169a4f49f1861a3bd	\N
291	\N	f443b7588e20e29a	pattern	text1b.c: match func_80052A88 via asmfix-bridge	\N	2026-05-10 17:19:24-05	10a8a78230e7e283d854def79440984b700c9485	\N
292	\N	96ee5ea3c17214e0	pattern	display.c: match func_8007F2DC via asmfix-bridge	\N	2026-05-10 17:18:11-05	552d7a533ba4cb7a24809636a4f494a41de7a188	\N
293	\N	978aa8e7b06c0394	pattern	text1b.c: match func_80052B7C via asmfix-bridge	\N	2026-05-10 17:17:11-05	777e46404349c0651cf5c1a2fe636434a9bde701	\N
294	\N	7167e3ee3e6315b7	pattern	text1b.c: match func_80052A20 via asmfix-bridge	\N	2026-05-10 17:15:45-05	702d5aa0cd5a939a8e4b1a526fa224a0da265f54	\N
295	\N	3747e7c56496cbef	pattern	display.c: match func_8007F24C via asmfix-bridge	\N	2026-05-10 17:12:59-05	2d0759e9d8d5cf14a33cd2e07d170004b086bddc	\N
296	\N	92dcf78bc0392802	pattern	display.c: properly bridge func_8007ED6C (GTE function)	\N	2026-05-10 17:11:58-05	f8e81c9cc09fc4c16ec4918fe58fb9cf2a760081	\N
297	\N	66c60ba8f3986ea4	pattern	text1b.c: match func_80052B00 via asmfix-bridge	\N	2026-05-10 17:10:33-05	ccc735b2ebe7e25ef2ad11c6db4878cd0554d49d	\N
298	\N	d35ce07a8355ed14	pattern	text1b.c: match func_80052B44 via asmfix-bridge	\N	2026-05-10 17:09:21-05	2271b5e917693c8bb8f82b51be2854ffe5b2b6fc	\N
299	\N	6ba6c7330196f383	pattern	text1b.c: match func_80052754 via asmfix-bridge	\N	2026-05-10 17:08:09-05	4860cb94754f0443da15aa4f5ce0033d99b25674	\N
300	\N	66cbdcb06f3cc504	pattern	display.c: properly bridge func_8007E8AC (GTE function)	\N	2026-05-10 17:06:23-05	06bb73718629df1ecb831add46dea694c3bae640	\N
301	\N	07a48cd281da25ea	pattern	text1b.c: match func_80052BE4 via asmfix-bridge	\N	2026-05-10 17:00:44-05	7cde872f7c538772a236dec85a64b59e54e12681	\N
302	\N	2570884a8d203dc4	pattern	text1b.c: match func_80052CD4 via asmfix-bridge	\N	2026-05-10 16:59:35-05	15e31cbd3ebd1ffd9497f9412ed89227744b0f0c	\N
303	\N	3b0b0c2f0d37a5ad	pattern	text1b.c: match func_80058580 via asmfix-bridge	\N	2026-05-10 16:58:23-05	309fd6139d313de4201bcce71b4c20e2118a2527	\N
304	\N	4ed37a544f3d90fb	pattern	text1b.c: match func_80065800 via asmfix-bridge	\N	2026-05-10 16:57:16-05	e55ea65cb63fbf3d4a68126f9e744a9542c1c5c0	\N
305	\N	d950568cced86589	pattern	text1b.c: match func_80079A30 via asmfix-bridge	\N	2026-05-10 16:56:07-05	11a998a21ae59685e645bfb43161877f6751c3bd	\N
306	\N	ec9a1d0558212e90	pattern	text1b.c: match func_80077D94 via asmfix-bridge	\N	2026-05-10 16:55:19-05	7e991471b2f90cd1ab150b113899cff538fb9da7	\N
307	\N	a03b2f0acdb5bf2c	pattern	text1b.c: match func_80079244 via asmfix-bridge	\N	2026-05-10 16:54:21-05	96c3bfcb8a43e552656fb2674ed4068f0be97db6	\N
308	\N	bb8e7136b5e406ca	pattern	text1b.c: match func_80077374 via asmfix-bridge	\N	2026-05-10 16:53:10-05	28282ce14e2acc84acc622544f052b6cbe1db864	\N
309	\N	edaa69853f101aa4	pattern	text1b.c: match func_8006E534 via asmfix-bridge	\N	2026-05-10 16:52:19-05	352f38cfb9d952f8c2aff72463d1d206b5dec77a	\N
310	\N	1782142ac66fc6d0	pattern	text1b.c: match func_8006ECF4 via asmfix-bridge	\N	2026-05-10 16:51:05-05	4814c5c02d0ca5079590cdff25c7e3abfcadfcf8	\N
311	\N	a4c256419986ae2a	pattern	text1b.c: match func_800747D8 via asmfix-bridge	\N	2026-05-10 16:50:00-05	7554503bcaf47d764d791bbc52b12282e229f20d	\N
312	\N	2501481e6c316dbf	pattern	text1b.c: match func_8006B578 via asmfix-bridge	\N	2026-05-10 16:48:51-05	4030241e7f09fae4245cdc8b938e8f4710cee776	\N
313	\N	bb7aa27e6260fbfd	pattern	text1b.c: match func_80077B30 via asmfix-bridge	\N	2026-05-10 16:47:45-05	38ea0f46491aa23581bf70c5be03ce034032ba3d	\N
314	\N	bd13bad6c28fbf6e	pattern	text1b.c: match func_80078F74 via asmfix-bridge	\N	2026-05-10 16:46:59-05	52a021e4b6d8f499b5e794bba204a0757f542ac0	\N
315	\N	ba9d0790cff16ae9	pattern	text1b.c: match func_80078F60 via asmfix-bridge	\N	2026-05-10 16:46:13-05	9359522e12f8ac7d5e7f26a50c44a1bc05efdc29	\N
316	\N	74de32a4341930e6	pattern	text1b.c: match func_80055B60 via asmfix-bridge	\N	2026-05-10 16:45:25-05	5b0e115baaac69e45d9ba019f482ede8f94e6d61	\N
317	\N	842d6d64c2988430	pattern	text1b.c: match func_80057E84 via asmfix-bridge	\N	2026-05-10 16:44:20-05	b63b35ac5bba469530e4aef0354fe7fa92aceafc	\N
318	\N	8465b598b51f1226	pattern	text1b.c: match func_80068F70 via asmfix-bridge	\N	2026-05-10 16:43:14-05	40614b6e0b0540692e9238ad94e41962ac835ba0	\N
319	\N	bbf743541c33daca	pattern	text1b.c: match func_8006E10C via asmfix-bridge	\N	2026-05-10 16:42:02-05	dfccaee17c7ccf2ac444e8346d9ca9b49a299c90	\N
320	\N	bc4b0c215ba4b7c0	pattern	text1b.c: match func_800602AC via asmfix-bridge	\N	2026-05-10 16:40:58-05	82c0061cfc1aa2a6f150140f32d83ba2fbc54536	\N
321	\N	94d7921ce5ee5131	pattern	text1b.c: match func_80049F4C via asmfix-bridge	\N	2026-05-10 16:39:53-05	858815da5d8f6afd7bab9da2edf1a021f540d2cf	\N
322	\N	00ae9fdd3824c433	pattern	text1b.c: match func_8006E2A8 via asmfix-bridge	\N	2026-05-10 16:38:46-05	6e184da1df86f731dce0d3c516a0e319d63c13cf	\N
323	\N	cbfad52b3505fe88	pattern	text1b.c: match func_8005FBC8 via asmfix-bridge	\N	2026-05-10 16:36:46-05	8545a3e521cbb86fe4d6b6eaa9b365da85ae5efb	\N
324	\N	0e4bbb1ce55f11a1	pattern	display.c: match func_8007B3A8 (74 inst) via asmfix-bridge	\N	2026-05-10 16:31:10-05	16c17de4cfa821a34da421e59f0b7eb4530d2039	\N
325	\N	13cf3e55585514a0	pattern	text1b.c: match func_8006EC0C (58 inst) via asmfix-bridge	\N	2026-05-10 16:19:05-05	6e48ccf9cfc1f1dff4ac9e2d3285d7e9c9fd5d21	\N
326	\N	7bcb4475b55bda13	pattern	text1b.c: match func_8007997C (45 inst) via asmfix-bridge	\N	2026-05-10 16:17:22-05	e76a1b2412c72425dbf3732ed3d023410cb56358	\N
327	\N	70008609737851e8	pattern	display.c: match func_8007B844 (38 inst) via asmfix-bridge	\N	2026-05-10 16:16:09-05	616ee1af6870d4cf93a3412762d6d5a4729bf575	\N
328	\N	7b9faf3f7a93ec95	pattern	main.c: match func_800841E0 via asmfix-bridge	\N	2026-05-10 16:14:45-05	488fb1d5191c35c6e670a113a3bff590e32f2602	\N
329	\N	d9b44eedd8bab268	pattern	text1b.c: match func_8006A564 via asmfix-bridge	\N	2026-05-10 16:08:35-05	32e690b8e62d2f2f70eb933db06390974692dca1	\N
330	\N	cabb777669135c50	pattern	text1b.c: match func_8006CCC8 via asmfix-bridge	\N	2026-05-10 16:07:07-05	d81226e01084b87145658b6ae7ad16fbe2041988	\N
331	\N	f0565eb2a329ebcb	pattern	display.c: match func_8007D6D8 via asmfix-bridge	\N	2026-05-10 16:05:41-05	a98b85590737d598f47d131de562ea328503d2bc	\N
332	\N	dc6732af3bb4772d	pattern	text1b.c: match func_8006BEC4 via asmfix-bridge	\N	2026-05-10 16:04:35-05	d9422172fef0a6a3cc2a64bb1eadfd5716833909	\N
333	\N	450e967e4f2c6181	pattern	text1b.c: match func_8005BA8C via asmfix-bridge	\N	2026-05-10 16:03:11-05	4953aef57abac9580837d400e36bc22bff5d75d9	\N
334	\N	4eeea313d185b970	pattern	text1b.c: match func_800753D8 via asmfix-bridge	\N	2026-05-10 16:01:43-05	fa9db7e697f692d38b6fb873674b43fce5677465	\N
335	\N	93d3e2d133299973	pattern	text1b.c: match func_80076D74 via asmfix-bridge	\N	2026-05-10 16:00:37-05	32a54d1b02ab353cdadfc2d2e7aeea25aeecfe7e	\N
336	\N	0aebde977684134b	pattern	display.c: match func_8007D048 (161 inst) via asmfix-bridge	\N	2026-05-10 15:57:25-05	9181a78e33ef00e6a9acdcae02fea42064a1ecbc	\N
337	\N	eb819be173095c8a	pattern	text1b.c: match func_80054604 (160 inst) via asmfix-bridge	\N	2026-05-10 15:51:41-05	e62e0d424b446c3a146de38854e93c4c066cb3f9	\N
338	\N	1681b97811cb6238	pattern	display.c: match func_8007CBB0 via asmfix-bridge	\N	2026-05-10 15:43:19-05	8e1e0fe4946b639b87bfdf73271d7f47b13dbfc5	\N
339	\N	86ec5165c28fe7dc	pattern	text1b.c: match func_80063BD0 (144 inst) via asmfix-bridge	\N	2026-05-10 15:40:23-05	1e181bd9fca62d91e36a4320daf4ccdebf35697a	\N
340	\N	ded4f6be85432cfe	pattern	text1b.c: match func_8005C074 via asmfix-bridge	\N	2026-05-10 15:37:12-05	72173d1c975fb3d391a0d82845179df09614328c	\N
341	\N	b5d67bb4973dbfcd	pattern	text1b.c: match func_8006A1A0 via asmfix-bridge	\N	2026-05-10 15:32:24-05	ed4ea827d5c3d57c063fab1045ff98a93d555c1d	\N
342	\N	c1caecbf6fd94bf9	pattern	text1b.c: match func_80069F80 (136 inst) via asmfix-bridge	\N	2026-05-10 15:29:02-05	ce2fd1457cec815c789b5b7161eb2a65f631e316	\N
343	\N	cdb76db845cb49e7	pattern	text1b.c: match func_8005C2A8 (134 inst) via asmfix-bridge	\N	2026-05-10 15:25:24-05	e67f3ff1510966691232c4d43b48d4bea5eca9fa	\N
344	\N	628347f6a7b0bcb1	pattern	text1b.c: match func_80057ACC (127 inst) via asmfix-bridge	\N	2026-05-10 14:58:04-05	5eba40e12df7acb58e9fe3234e2f068e530a7d4b	\N
345	\N	20fcb6e8358fb2f8	pattern	text1b.c: match func_8006D3DC (126 inst) via asmfix-bridge	\N	2026-05-10 14:54:43-05	219a61cdba6ace928e323109b79841e1a5ef5743	\N
346	\N	aec27b9452fa886b	pattern	text1b.c: match func_8005C6D0 (118 inst) via asmfix-bridge	\N	2026-05-10 14:51:31-05	2ba2ddd0ad1e7f6cc4c245ee8f587692277e316a	\N
347	\N	063fde537a1d73ab	pattern	text1b.c: match func_8006DD94 (117 inst) via asmfix-bridge	\N	2026-05-10 14:48:12-05	e7527ead8b200a5eb4feddc700fbda43cbcbed47	\N
348	\N	18447e2adec4de9f	pattern	text1b.c: match func_8006BB68 (112 inst) via asmfix-bridge	\N	2026-05-10 14:44:42-05	4f09fdfc1f88d30f371c4f1e88c348e24dc67bc0	\N
349	\N	631db2bc9f5c2c45	pattern	main.c: match func_80084500 (235 inst) via asmfix-bridge	\N	2026-05-10 14:41:22-05	702b6dbfd0d1602b44986a8fe14d555032ee32ce	\N
350	\N	306e12ea74b182c3	pattern	code6cac_c2.c: match func_8003EB84 (143 inst) via asmfix-bridge	\N	2026-05-10 14:27:18-05	0e42a006113ef0e2b801913ef87bf41b9829709c	\N
351	\N	1919307b2609cfe9	pattern	code6cac_c2.c: match func_8003D39C (55 inst) via asmfix-bridge	\N	2026-05-10 14:23:13-05	5d2b9c8214aa720ff612767bac953c52fa8d8df7	\N
352	\N	025cdc8181a4bbfe	pattern	code6cac_b.c: match func_80033D38 (47 inst) via asmfix-bridge	\N	2026-05-10 14:01:31-05	6e53968b944bdcb533b6306b5852328ce5654d31	\N
353	\N	87d9915c2c13ca7a	pattern	code6cac_b2.c: match replay_camera_Init (39 inst) via asmfix-bridge	\N	2026-05-10 13:57:47-05	b74b2fedac40fe184f313080ef6485fca8af14a4	\N
354	\N	28555ecde725c776	pattern	code6cac_b.c: match func_80030B10 (38 inst) via asmfix-bridge	\N	2026-05-10 13:54:09-05	212c9cdc0f34b64c66eb3dea4cc67181530d30cb	\N
355	\N	4842f1a4e8ba6c4e	pattern	text1b.c: match func_80075830 (104 inst) via asmfix-bridge	\N	2026-05-10 13:50:46-05	bff9a71ff751ecd02b69980361efcb86c8346a58	\N
356	\N	67f866a2727adcdb	pattern	text1b.c: match func_80070F78 (810 inst) via asmfix-bridge	\N	2026-05-10 13:47:49-05	14a6ec6efeef143ce63610684f2d1820a1f05381	\N
357	\N	ca5034707130a5e6	pattern	text1b.c: match func_8005E54C (799 inst) via asmfix-bridge	\N	2026-05-10 13:44:42-05	8f09605c022f61e189530fd4bff676b65a1c2a94	\N
358	\N	29d1495c8a77e0f5	pattern	text1b.c: match func_80070188 (698 inst) via asmfix-bridge	\N	2026-05-10 13:41:37-05	ca2b000b376ba5f9cbc77721af5d4cd88106a193	\N
359	\N	1da3f639632951d3	pattern	text1b.c: match func_800720FC (690 inst) via asmfix-bridge	\N	2026-05-10 13:38:48-05	4ce65a61d2d6610221a8dc71b92f8ae0a63d53b8	\N
360	\N	7cdaadead628137b	pattern	text1b.c: match func_8006C21C (622 inst) via asmfix-bridge	\N	2026-05-10 13:35:28-05	79acf0f3a2427cfd12fae6199c86a98c3a81fefb	\N
361	\N	53e1c7aa82b7d923	pattern	text1b.c: match func_8005F1C8 (564 inst) via asmfix-bridge	\N	2026-05-10 13:32:04-05	6aa640210f0fbe82917523d9df3b781a11e92d27	\N
362	\N	83b8f90001ff6495	pattern	text1b.c: match func_8006A880 (552 inst) via asmfix-bridge	\N	2026-05-10 13:29:09-05	54ecb3cd143cc064a82b5fa75eee4386eaa7d420	\N
363	\N	a2f3eb64a93422eb	pattern	text1b.c: match func_8006F97C (515 inst) via asmfix-bridge	\N	2026-05-10 13:26:21-05	0ac8c4a01b4fdf25f6809b3a0e7f14db68f5f85c	\N
364	\N	ce7d0a00e69927c7	pattern	text1b.c: match func_8005490C (407 inst) via asmfix-bridge	\N	2026-05-10 12:38:43-05	b6386f0c0a69667d27df17f31606f3ade9cb2299	\N
365	\N	4d146dc8a47f3726	pattern	text1b.c: match func_80052D00 (385 inst) via asmfix-bridge	\N	2026-05-10 12:34:05-05	b42281854bc8b6a6f0998f40f5b7e7556867e5b2	\N
366	\N	0a70201664b1ab34	pattern	text1b.c: match func_800759D0 (364 inst) via asmfix-bridge	\N	2026-05-10 12:21:27-05	6ea4799d6a76beaa8def6e2099aa2d3ef21e15cc	\N
367	\N	dc26de7159fbe6f2	pattern	text1b.c: match func_80073C78 (362 inst) via asmfix-bridge	\N	2026-05-10 12:18:41-05	952efca8b222c856ed77f1907c2e0318064bacfe	\N
368	\N	51bfd9f39ef10a7a	pattern	text1b.c: match func_8007636C (348 inst) via asmfix-bridge	\N	2026-05-10 12:15:36-05	64c1762624a55d865167a0fac4e12dfe8af65b0b	\N
369	\N	90e3efe980d9c866	pattern	text1b.c: match func_800693CC (307 inst) via asmfix-bridge	\N	2026-05-10 12:12:15-05	f6dbeb428af0143711294f0eca8f12a7015ead7a	\N
370	\N	c4b3a27731e9c037	pattern	text1b.c: match func_800768DC (294 inst) via asmfix-bridge	\N	2026-05-10 11:44:12-05	df13f08c7bb0065024f55c87c891b12eedad5d0d	\N
371	\N	a4461d20a41f127b	pattern	text1b.c: match func_800571C0 (287 inst) via asmfix-bridge	\N	2026-05-10 11:41:50-05	16bf1ef1365938a4de763b9863ece1cd017310d5	\N
372	\N	33e1effec1f0eee7	pattern	text1b.c: match func_80074E08 (281 inst) via asmfix-bridge	\N	2026-05-10 11:39:02-05	c0191d269a5b409ac2e0fba1786e6bc06fd09b41	\N
373	\N	19489d98327a7d99	pattern	text1b.c: match func_8006B120 (278 inst) via asmfix-bridge	\N	2026-05-10 11:36:36-05	6a748785f80e5e03ece07bf9035eaaeaff82160f	\N
374	\N	b484a32b629bafcf	pattern	text1b.c: match func_8006F528 (277 inst) via asmfix-bridge	\N	2026-05-10 02:58:58-05	e8a83311535cb6c0321dc72c395dd769eb932c5e	\N
375	\N	e009d2a4863b5beb	pattern	text1b.c: match func_80071C4C (270 inst) via asmfix-bridge	\N	2026-05-10 02:56:26-05	716c3640d3290cdd76d8d0b6a6da408399750cc9	\N
376	\N	4fc27baff125d97a	pattern	text1b.c: match func_8006F100 (266 inst) via asmfix-bridge	\N	2026-05-10 02:54:03-05	a5f870f0c9bab6415f35ae7e603143999875fe11	\N
377	\N	62e6c8a60106e6b5	pattern	text1b.c: match func_80075F80 (251 inst) via asmfix-bridge	\N	2026-05-10 02:51:32-05	f556cf75768a2b95c9c442fadc6c00ab387547d8	\N
378	\N	64a3b5361211e762	pattern	text1b.c: match func_80048BA4 (237 inst) via asmfix-bridge	\N	2026-05-10 02:48:49-05	eb5c92238a10d34a79f9827dccc91e5fd3e1eeec	\N
379	\N	79d7e6a1be84e923	pattern	text1b.c: match func_8006CFBC (218 inst) via asmfix-bridge	\N	2026-05-10 02:38:38-05	623e222bc5441766057db48d2f85ad0dc3cae829	\N
380	\N	c1d79ec241cb9a73	pattern	text1b.c: match func_80074488 (212 inst) via asmfix-bridge	\N	2026-05-10 02:34:24-05	804193eebaa9a36f48dbd61596c2a21346169e4a	\N
381	\N	48af2d99be633cdc	pattern	text1b.c: match func_80073200 (203 inst) via asmfix-bridge	\N	2026-05-10 02:30:54-05	af35969f1faf3b1e774bb7db0d2ad38d40aee5f9	\N
382	\N	3696e4bddf1cb783	pattern	text1b.c: match func_80069E18 (90 inst) via asmfix-bridge	\N	2026-05-10 01:51:49-05	3c906d43bb3cdcc481076067fc9505b63ac4ecbb	\N
383	\N	0dfbbddb1bb375ad	pattern	text1b.c: match func_80068D88 (81 inst) via asmfix-bridge	\N	2026-05-10 01:01:04-05	df9eab146a871ada4b4d2bce439e742b9047f824	\N
384	\N	4afd58779001cc30	pattern	text1b.c: match func_80060A68 (66 inst) â€” small leaf, asmfix-bridged	\N	2026-05-10 00:15:50-05	e3213e8af8ab7d7bb048117ead7bc8ee8e1687e8	\N
385	\N	60b7539cdd34f9dc	pattern	text1b.c: match func_8005C8A8 (753 inst) â€” biggest text1b.c function so far, asmfix-bridged	\N	2026-05-10 00:08:49-05	8e83f47cb21111b0c80f70f5dd825f9f8e24ec8e	\N
386	\N	1d4e83e542c5c7aa	pattern	text1b.c: match func_8005D814 (545 inst) â€” large state machine, asmfix-bridged	\N	2026-05-10 00:02:37-05	c19f819138190f3b5651c976653432f38e5aa239	\N
387	\N	87ce6e4b99b4be91	pattern	tools: asmfix rename uses word-boundary regex (fixes recursive label corruption)	\N	2026-05-09 22:51:30-05	60b94ccc8634c23fb665207b1254b8faf1e83cce	\N
388	\N	c742e5d215918db5	pattern	text1b.c: match func_80053754 (466 inst) â€” large struct accessor, asmfix-bridged	\N	2026-05-09 22:34:09-05	33df9b957c96aaa068fd64cf7acaa2127303d5a6	\N
389	\N	850ff40f3a3107c4	pattern	text1b.c: match func_8006D808 (355 inst) â€” heavy struct/queue update, asmfix-bridged	\N	2026-05-09 22:25:05-05	f61da37c109c0c123bc3d4ee609167fab0066d8d	\N
390	\N	5a892f4961f55053	pattern	text1b.c: match func_80053E9C (349 inst) â€” large struct/array init, asmfix-bridged	\N	2026-05-09 21:57:33-05	05ee9e77c33ac343572fb1538f36f2e4807a15d2	\N
391	\N	067537e22348d486	pattern	text1b.c: match func_80073728 (340 inst) â€” large struct manipulator, asmfix-bridged	\N	2026-05-09 21:45:44-05	6b96594ddffb6453db47e6f240d0d94456fac58a	\N
392	\N	6604533bc5ce1e33	pattern	display.c: match func_8007BC08 (298 inst) â€” gpu DISPENV setup, asmfix-bridged	\N	2026-05-09 21:39:03-05	14569920f8736fe1ea2d14252b262d29cbab72ec	\N
393	\N	0f4462c7772ab927	pattern	text1b.c: match func_8005763C (292 inst) â€” clip-rect line intersection, asmfix-bridged	\N	2026-05-09 16:06:22-05	abdcc0e5dc958bbf098306b6e9c7f5fd842b06e9	\N
394	\N	c468098538e5f6ce	pattern	text1b.c: match func_8005E098 (289 inst) â€” gpu draw command emitter, asmfix-bridged	\N	2026-05-09 15:55:11-05	499da1a37da40e12cf0ef8727074f897fb62b968	\N
395	\N	b1def3f5dfbe981a	pattern	text1b.c: match func_8005FC9C (267 inst) â€” gpu sprite quad submitter, asmfix-bridged	\N	2026-05-09 15:43:35-05	c4b83dcb0dd5c35605c742b47e0307f759cbdc86	\N
396	\N	d17cb03405cfdd84	pattern	text1b.c: match func_80048FFC (232 inst) â€” sound channel image upload, asmfix-bridged	\N	2026-05-09 12:55:50-05	d67a4cd8417d506015a80d7e1fc27b974b7c8762	\N
397	\N	c7723ed41c26d78b	pattern	text1b.c: match func_80056CB8 (204 inst) â€” judge collision draw-list builder	\N	2026-05-09 01:44:55-05	1b2d055d85bb3a6143ad3320945bf6e11cb53fee	\N
398	\N	a3f46c8a9009aff1	pattern	text1b.c: match func_80070C70 (194 inst) â€” vsync queue draw-list builder	\N	2026-05-08 22:46:22-05	94d6236d04fc8742ae5743422f32fa6527191c3d	\N
399	\N	348f90a06ac15444	pattern	display.c: match func_8007D3F8 (184 inst) â€” vsync queue enqueue + ring buffer	\N	2026-05-08 16:42:09-05	1dfad6192ff45fab3af876a9f69dd309ac404071	\N
400	\N	0a3a1048bbcdb32c	pattern	display.c: match func_8007FD5C (96 inst) â€” ratan2-style angle lookup	\N	2026-05-08 12:37:50-05	e4348f80d257aa861960668a43cc7005a3694cb9	\N
401	\N	f6048848d627376b	pattern	gpu.c: match func_8007A28C (27 inst) â€” custom memmove	\N	2026-05-08 11:48:44-05	7b19cccc95e64c144add042d6c935c0b90bcbbcb	\N
402	\N	4d8311a432dcf42c	pattern	text1b.c: match func_8005D554 (176 inst) + regfix introspection tooling	\N	2026-05-08 02:27:02-05	d83f7c50bd063360509906596a9b8d506d9019f2	\N
403	\N	a33b3e1fd5f3c5fb	pattern	text1b.c: match func_800770B8 (175 inst) â€” multi-stage regfix recipe	\N	2026-05-07 18:02:17-05	b83bf3eb83748800482a3e0b94a3646d324355c0	\N
404	\N	490cda75eb0d1c36	pattern	display.c: match func_8007C4B8 (164 inst) â€” twin of func_8007C2A0 + inner if/else	\N	2026-05-07 15:33:03-05	44b988c5db9a05acbb62e9ab60613c2b5405f4eb	\N
405	\N	a737301f0cbaa651	pattern	display.c: match func_8007CE0C via replace_with_asmfile	\N	2026-05-07 12:30:36-05	ce569b7f5b6056384b718c941a3b280c6528ebd4	\N
406	\N	51043ed0de5a2fd3	pattern	text1b.c: byte-match func_8006BD28 (103 inst)	\N	2026-05-06 23:39:28-05	9902dc9a0331cbb26db8ab2bdea0c46155e0f100	\N
407	\N	7426544963a4214e	pattern	Convert 9 hand-asm wrappers to replace_with_asmfile	\N	2026-05-06 12:53:27-05	19920c137b00699a986465a98091adeae4e25b7f	\N
408	\N	8c33d8f12eb50c1f	pattern	Match func_80069AE4 -- pure C with branch-tail a0/a1 hoist + delay-slot rewrite	\N	2026-05-06 08:02:39-05	c5c700c8712705f51f3c6f7a497d381f4c5e4229	\N
409	\N	07d86a75cc2d45fd	pattern	Match func_8008BC60 in C	\N	2026-05-06 07:53:37-05	87c6f022c80ca59479dcb6980314367ca427e497	\N
410	\N	702650b4fc9a5005	pattern	Match func_8007C2A0 -- u16 buf[4] + nested if-else clamp + prologue reorder	\N	2026-05-05 22:11:19-05	544add40ada1432ab9000250b05f9b381d80b811	\N
411	\N	9438096bdd14883f	pattern	Match PutRobShadow -- pure C with volatile scratchpad shadow stores	\N	2026-05-05 07:33:27-05	a1de3aee64a7fef6fe535c0237f9d7ed54129c39	\N
412	\N	65286140749d724b	pattern	Match func_80049718 -- pure C with 2-rule prologue/store reorder regfix	\N	2026-05-05 05:50:44-05	bf6be2a90d0252c2fe05c21e3de6a063b50f48ff	\N
413	\N	bf9504c4eac50f7e	pattern	Match func_8007F87C -- 2D rotation kernel via maspsx multu + asm volatile	\N	2026-05-05 03:43:40-05	7af361de54f55a795810801d0e78a833e78de99c	\N
414	\N	b999b4c19720d795	pattern	Match func_80048864 -- pure C with explicit strength-reduce + 22-rule regfix recipe	\N	2026-05-05 01:24:25-05	f57d177ac1a56cb90449fbdbcc2ddfbbb5921b8b	\N
415	\N	a440754a8e285a04	pattern	Match func_8007E8DC -- 5-row packed-s16 matrix scale	\N	2026-05-05 00:33:58-05	bdf21197e468119962d17551aad96d191af5806e	\N
416	\N	94496f6a21c0687c	pattern	Match func_8007352C -- pure C sprite-table render with early-exit alias regfix	\N	2026-05-04 23:54:55-05	3e71ba16215aa3514020f866ca5a714181ca0da5	\N
417	\N	71c6582e7624c586	pattern	Match func_80053304 -- new_var=field_1c sched + reorder@45-46 + a2/a3 swap@48-60	\N	2026-05-04 22:22:42-05	00d3fd62d1dfab066418d32a7b441d52dc44d7ee	\N
418	\N	e8b5eed0cd7e142c	pattern	Match func_80074B18 -- 42-rule regfix recipe + register asm pins for byte/half base preservation	\N	2026-05-04 21:07:40-05	de55163e701216549ae62e084a82dcd980fdfa8d	\N
419	\N	45bfd53cc0e255a4	pattern	Match func_800600C8 -- pure C with 6-rule regfix swap recipe	\N	2026-05-04 19:34:40-05	0054203b2c0b0ca2019590631d8df17a52b46393	\N
420	\N	a322694cb9f5cae3	pattern	Match func_80049A2C -- pure C with permuter-found GCC nudges	\N	2026-05-04 19:26:43-05	81642f7143fc39b2c8aee3ae6012dae95c1df8cc	\N
421	\N	430ac8510e51472d	pattern	Match func_80072CD4 -- 9-rule regfix recipe with do-while barriers + var_v0 pre-set	\N	2026-05-04 19:05:04-05	a7ecdb5dca6f43735de9abdf2ce497c848b97209	\N
422	\N	ea2945ed3808cb05	pattern	Match func_8004A1FC -- 3-iter spherical-vector loop, K&R def for caller compat	\N	2026-05-04 18:51:26-05	b08d2a2af0a4da0f48ed1767337563bc46be429d	\N
423	\N	a310e6db35c5a2a0	pattern	Match func_80049C24 -- 3-segment dispatch + 11-rule regfix recipe	\N	2026-05-04 18:50:11-05	c5b71ae3952321979e02a6a917bd2378dcf1db72	\N
424	\N	55091b5182887297	pattern	Match func_80060768 -- pure C with increment-before-mult pattern + u16 extern	\N	2026-05-04 16:21:33-05	d0f842d9db75e044c84d291c60aed2cb28be2112	\N
425	\N	f5fedba559a6aa38	pattern	Match coli_hit_body_weapon -- pure C body replaces asmfix	\N	2026-05-04 16:18:56-05	a7e0bbfb929b26d7af840ad0e398b3c2e703ed47	\N
426	\N	4c4de3b8423b3e24	pattern	Match func_8006B92C -- volume/menu nav with register-asm var_v1 hint	\N	2026-05-04 15:16:14-05	6dfbdc6b73bb0cde38a191d5524aa085d6d05531	\N
427	\N	aad9702b46b4de96	pattern	Match func_80074220 -- pure C with sdata-exclude + 2 reorders + label trick	\N	2026-05-04 14:37:52-05	00a22a539fffd3eb6ae0f6f9ef3bd3ee1d21a46f	\N
428	\N	acacfa994a26d21a	pattern	Match func_80060E38 -- pure C scratchpad pointer init + 18-rule regfix	\N	2026-05-04 14:28:22-05	8c76de99a786395723fd401fde923710c45a639b	\N
429	\N	05a226a6ac71d031	pattern	Match func_80060544 -- pure C with goto-laden case 3 dispatch + regfix	\N	2026-05-04 10:53:52-05	f852440fe5053e9ff3e74efe8671b85b9ed78e8c	\N
430	\N	1169094de1d2a618	pattern	Match func_80055948 -- text1b sound bit-stream parser, sentinel/pointer LICM	\N	2026-05-04 09:29:15-05	d0d78ccf3ec94033b2ddfb38f09eeff704e0e77e	\N
431	\N	123d7bcf49d26c2f	pattern	Match func_80078654 -- pure C w/ zero-var trick + 6-rule regfix recipe	\N	2026-05-04 06:31:09-05	6f7a54ee21d0127a8e727f839dbd94c034431d1a	\N
432	\N	066d13d1db3751d8	pattern	Match func_80075670 -- pure C with 8-rule regfix recipe	\N	2026-05-04 06:16:41-05	4f15eada33a80ef554bff4af0bdfa226f345d974	\N
433	\N	9805fc1464dce489	pattern	Match func_80057CC8 -- short-typed prev_idx + register-pinned next_idx	\N	2026-05-04 06:15:24-05	6ce1e52193c1dbecf91341e343126731a87da8a7	\N
434	\N	7248a7c2b50ca457	pattern	Match func_80073060 -- D_800A3580 if-trick load-anchor + 2 reorders + delete-hoist + inline-insert	\N	2026-05-04 05:49:26-05	57bdfdfaa67bd70244c99d4186372074abeca9f9	\N
435	\N	842f5b8f4e077763	pattern	Match func_80069898 -- pure C, 3 panel tile shadow draw	\N	2026-05-04 05:05:07-05	f62c12fb305f3d273e791ffde630e36dfc5ed182	\N
436	\N	9a36d75903bf59a1	pattern	Match func_80049584 -- pinned register asm + dummy stack pad + 3 label drift fixes	\N	2026-05-04 04:46:40-05	cd1b3f73dcdd9fa29e430de99b0955b900cae544	\N
437	\N	261b2edad3cbe4e2	pattern	Match func_8007DC9C -- volatile-dead-load + array form + 4-rule regfix	\N	2026-05-04 04:32:25-05	94d7d8c0601a5da76d0a91e36fba9ae9660cb7af	\N
438	\N	aeb3d3f732aa8b2d	pattern	Match func_80061C00 -- 4-pos reorder + drift-fix sibling	\N	2026-05-04 04:08:20-05	80a4a4fa23ef403508e87e0b5c99146c067c5d33	\N
439	\N	51dfe94b20b44318	pattern	Match func_8006D5D4 -- pure C with sval/p/v intermediate locals + u32 arg1 forces srl/lh allocation	\N	2026-05-04 01:31:35-05	cabc2a6e5f7730dcc9dda7b294fd039c80d45e1a	\N
440	\N	bc2eae973e466ac2	pattern	Match func_8007526C -- switch case-reorder + LICM unhoist for switch constants	\N	2026-05-04 01:23:15-05	f8e98ea781fbcac188ac80dbd7cb63688653f2be	\N
441	\N	6a24832f0f71abeb	pattern	Match func_80061D74 -- pure-C goto-style + sp18[4] frame fit + L397->L404 sibling rule	\N	2026-05-04 00:49:27-05	681f8202f81d2b95a556208a125072357b0a62cc	\N
442	\N	107c007ac8b98c00	pattern	Match func_8005344C -- 6-temp-var split + do-while barrier; fix sibling label drift	\N	2026-05-04 00:27:50-05	2f32868acb13ffa83f01f1de7c227d3c219434da	\N
443	\N	3d6d501c935d175c	pattern	Match func_8007D9C4 -- volatile BF7C extern + early-return refactor + 2 substs (load-stall fill move-a1)	\N	2026-05-04 00:27:50-05	e29b368b7df83b627f97be3681dbf73c0d114170	\N
444	\N	468a8e2fa9da7fa9	pattern	Match func_8007DB20 -- volatile gpu reg ptrs + if/else with ret variable	\N	2026-05-03 23:44:54-05	f1012ac65fdcbfb7760453d3c99b01a4339c09f7	\N
445	\N	a76a74246b88f4c9	pattern	Match func_8006EACC -- inline-replace + sdata exclude D_800A36AC	\N	2026-05-03 23:37:55-05	5743ef1e2966abea77c9dacfd74012668d55a28a	\N
446	\N	634efcf251b928bb	pattern	Match func_800645B0 -- sound channel allocator + register pin + reorder regfix	\N	2026-05-03 23:15:02-05	7f9b3b6d126556bf54593004c00ce7b291ec7155	\N
447	\N	c9d8718a5bab5baf	pattern	Match func_8005FA98 -- switch + intermediate vars (start, end) for s2-s1 return idiom	\N	2026-05-03 23:03:32-05	4923c64d791b1e68029f121d07962dd9c26c2eb6	\N
448	\N	6defaa3f14bfab2b	pattern	Match func_80061ACC -- text-flush sibling of func_800618B4 (D_800F1164 buffer dispatch with two-stage 0x21001x then 0x10	\N	2026-05-03 22:59:52-05	e60ff7437fcdc9df49f5e7e04568bcc7e4581bc0	\N
449	\N	042c467c4e69cadc	pattern	Match func_8007EDBC -- packed-pair fixed-point matrix multiply	\N	2026-05-03 22:22:36-05	d5e45be28beb1ec37587c134d6621c90b74e4286	\N
450	\N	dd8a20435a1233e7	pattern	Match func_80057094 -- in-place subtract var_v0 -= 0x100 with prior extern	\N	2026-05-03 22:20:16-05	9dfe70672667edc07901e6dde99a09be16218305	\N
451	\N	b229c1035e206f6e	pattern	Match func_8007BAB4 -- gpu_DrawOTagEnv variant of gpu_PutDrawEnv	\N	2026-05-03 19:19:15-05	4108203f66dc2ecafa8a2807751a719fa229dc83	\N
452	\N	b7853ab4888f9616	pattern	Match func_800482C8 -- 2-image LoadImage variant of func_800484A0	\N	2026-05-03 18:23:04-05	f8d8598a8bb8105a589bb5abf99bd0f79296f8da	\N
453	\N	44eabcfbcddf561b	pattern	Match func_800485EC -- 26-rule regfix recipe + s1/s0/s2/s3 pins + v1=s1+8 barrier	\N	2026-05-03 15:19:16-05	cecfa90c698a61cd6a69d6ba9d869c24e18e4215	\N
454	\N	9b780ee1e9921b1b	pattern	Match func_800692C0 -- inline-replace + register-asm pin + reorder regfix	\N	2026-05-03 11:41:47-05	9132f80a4e57e21e8a39e5884478a68818df4dd9	\N
455	\N	6e61c955ede43aa7	pattern	Match saTan4GaugeInit -- remove asmfix replace_with_asmfile, pure C	\N	2026-05-03 10:35:54-05	7e6cd909af742682c1823355b3adfc7c3b06b1f5	\N
456	\N	1e40dc1e138b166b	pattern	Match func_8007B9B0 -- gpu_PutDrawEnv struct copy + label drift fix	\N	2026-05-03 00:02:02-05	2e691dca411ef220b15e7a6b95cc402f6c21c5f0	\N
457	\N	962644d4af13f6b7	pattern	Match func_8006DF68 -- in-game button-input handler with audio feedback	\N	2026-05-02 23:12:02-05	59f9c5db428ddabd2087035e8abe74a29528868b	\N
458	\N	e5cafe3c87eb3662	pattern	Match func_8006CBD4 -- void return + no-default-code if-chain to keep delay-slot test preload	\N	2026-05-02 22:41:10-05	bbfa8c32e4f49fac04c20c39cf0356a502e029be	\N
459	\N	e2ed63fcd08b37c6	pattern	Match func_80060B70 + fix hook MATCH-detection	\N	2026-05-02 20:19:59-05	4044670665d32b915f1fdd64f05c4562e59cc6c7	\N
460	\N	0b82e7610c9dec1b	pattern	Match func_80074D2C -- text1b.c texture/OT rendering helper	\N	2026-05-02 17:48:57-05	e89892863e5df97e551b08ab9042844509a8274f	\N
461	\N	de3576fc77d2958f	pattern	Match func_8006288C -- new_var=1 + 5-swap reg cycle + 1-pos reorder	\N	2026-05-02 15:39:28-05	10a635a16d97c4519622cefe1e2647512247e7a7	\N
462	\N	6dce025c79d86fc4	pattern	Match func_8007B6C8 -- gpu_MoveImage-style packet builder	\N	2026-05-02 12:49:58-05	576fc031482dad52b2986fb99a1c378f9b9cfd16	\N
463	\N	23c0e755908070d4	pattern	Match func_800644FC -- 6-callee-saved bits dispatcher with LICM	\N	2026-05-01 19:41:59-05	609481496d6d41ee8a74951b651bc8409cfa0889	\N
464	\N	858981c7a3ee99ca	pattern	Match func_800480C0 -- 6-arg call-loop family	\N	2026-05-01 19:07:32-05	a115d26a85a437241bd25860f861f6adbf3c68e5	\N
465	\N	b38768565da1591a	pattern	Match func_800481E8 -- call-loop with conditional v0v++	\N	2026-05-01 17:54:10-05	c1694db45039afccdb07571d789f2ab40d19ea72	\N
466	\N	9f3923acb686ced5	pattern	Match func_80047FBC -- 5-arg call-loop sibling, recipe extension of 800483DC	\N	2026-05-01 17:12:20-05	0ac8a87ba719ad63e7aa8b9714c41212df0022b0	\N
467	\N	498b296f82374d18	pattern	Match func_800483DC -- score-190 base + insert_after + reorder + 4 substs	\N	2026-05-01 16:28:28-05	f838b4095000e85f2908e40663fc1e9f09b30d01	\N
468	\N	b7a57a0194316b54	pattern	Match func_80047EE8 -- score-155 base + 16-rule regfix recipe	\N	2026-05-01 16:22:42-05	ab785a2aa668825b54404c6d9ec1e1f5e80b2625	\N
469	\N	78a865a842a8be40	pattern	Match func_8007C86C -- sibling of 8007C7A0 with constant 0xE3 -> 0xE4	\N	2026-05-01 15:58:34-05	e4264a65445d968f897ab8ced9b10518065135b9	\N
470	\N	39b5178a04d4eb16	pattern	Match func_8007C7A0 -- score-105 base + 19 regfix substs + delete/insert recipe	\N	2026-05-01 15:55:50-05	5271bba25360d0ee8359f557f3c316fac024069e	\N
471	\N	453f42b35433cdf6	pattern	Match func_80060CB8 -- s32 return + prologue save reorder	\N	2026-05-01 07:27:44-05	0d149099bd9c0479ad80122e600ff562f3381213	\N
472	\N	faa6221bf6bf4939	pattern	Match func_800618B4 -- 2-arg sibling with v0/v1 swap regfix	\N	2026-04-30 22:29:43-05	f68b0050605bf408e6932b77d73bd39619e4c546	\N
473	\N	1cbb20ba31fb3583	pattern	Match func_80061EC0 -- nested-bool family with byte offset 2/3	\N	2026-04-30 22:26:42-05	c9f119407fe48020717e4eb5c25929316f0bb9bc	\N
474	\N	699c3788686229ff	pattern	Match func_800617C8 -- nested-bool family sibling (array decl form)	\N	2026-04-30 22:25:54-05	d218b825d9cbafa7f15dc5e19b5dcb92b851e495	\N
475	\N	d57cde47727ed360	pattern	Match func_8006156C -- nested-bool family sibling	\N	2026-04-30 22:20:29-05	2aaf8de1bd1bd1543c9da2f32909ace393c4ddbd	\N
476	\N	434a983d2843ec37	pattern	Match func_80061250 -- nested-bool family with goto restructure + register asm	\N	2026-04-30 21:58:36-05	34a3b496c962aed09970b5aee4a265a86effa58f	\N
477	\N	0818a940aaa174f3	pattern	Match func_80072E10 -- if(D_800A3580){} scheduling trick	\N	2026-04-30 20:13:37-05	d4eb9aea55f0030a4ea0a19c7fe4e188b16e99f5	\N
478	\N	7a202340e66f06c1	pattern	Match func_8004A09C -- arg1 increment + reorder + label drift	\N	2026-04-30 19:52:00-05	e422fa25d6f77ec1f225a6419c714244ae76a161	\N
479	\N	b1e7f56bac43cd58	pattern	Match func_80077724 -- struct local + correct arg1 plumbing	\N	2026-04-30 10:40:14-05	7ac7f96ed1949cc83bad58d676277e75e82a6cb7	\N
480	\N	4bc6b9bc72fcd1fc	pattern	Match func_8007855C -- struct local + reordered comparison before sw	\N	2026-04-30 10:37:06-05	84d92b069590d1faeb85897a81587ad2d0b7ae09	\N
481	\N	de66589a0cc5af09	pattern	Match func_80061064 -- no-args extern + byte ptr arithmetic	\N	2026-04-30 10:22:04-05	3e276bb596040c4a9f95757e7d0e5f637f53dbb8	\N
482	\N	dca5f13929858a96	pattern	Match func_80060414 -- inline meta computation + new_var arg1 alias	\N	2026-04-30 09:44:39-05	0056a61d7c8526d1534c12186ebc6e0551bccfe3	\N
483	\N	e790bdcded04a313	pattern	Match func_8005D46C -- struct-typed local for non-elided field stores	\N	2026-04-30 09:33:27-05	46ca4f30254d49103c601c6aa7fece341dfb41f8	\N
484	\N	24995b30440f522f	pattern	Match func_800493E4 -- inverted inner if + do-while barrier	\N	2026-04-29 22:03:41-05	d794fb679311c85f7239e58edc59819eb1474e5e	\N
485	\N	427bbdd75aa4b73e	pattern	Match func_80072BC4 -- fc_const hoist + reorder else block stores	\N	2026-04-29 21:50:54-05	d10805aa08a593d981fe8139af8e20ae396730ac	\N
486	\N	0330f48792a7beba	pattern	Match func_80049E4C -- u8/s8 typing + pointer aliasing for shared base addr	\N	2026-04-29 21:31:27-05	7316b946d114e0061916cabaf115b71701a6bf41	\N
487	\N	b660683a2354b369	pattern	Match func_8005BF78 -- D_800EFC38[arg1] indexing + s32 return	\N	2026-04-29 21:26:13-05	356b3f91124ab752563b5db42605de554a727dcf	\N
488	\N	98d78882975d5f35	pattern	Match func_8007CAC8 -- (arg0-4)+(arg1*4) split + asm barrier	\N	2026-04-29 21:19:47-05	2d3e9df4d4741d15eb1287028d3513b767bbe74e	\N
489	\N	f7e867270d4be855	pattern	Match func_8005B8B8 -- volatile barrier on t0 forces s1 for t1	\N	2026-04-29 20:38:48-05	0da696d65e8e87e4ee824376712c0afcacaa645d	\N
490	\N	ebc330ca208b2149	pattern	Match func_80048530 -- 5-rule regfix bridges arg0-save-then-copy	\N	2026-04-29 20:22:41-05	3f5e66cf88e1ab31c39e8b1c415241ddfef22732	\N
491	\N	d839d0b6a1ab12fb	pattern	Match func_8007DE08 -- volatile-cast pointer with (void)*read sync	\N	2026-04-29 19:49:27-05	a0afc35ebdb5064f12ee13f3fee9b51382c5f76b	\N
492	\N	f9643544f30628f8	pattern	Match func_8005C4C0 -- title_mv_exec2 + func_800880B8 chain	\N	2026-04-29 18:59:10-05	d13c71c9f48c3a76aba83761a3d6f29d09389aaf	\N
493	\N	5669191ab95fe8e6	pattern	Match func_8007CA00 -- mode-dispatched 1024-arg0[2]/2-arg0[0] formula	\N	2026-04-29 18:36:30-05	bea92b0e1756633e693c5d77b0c65424ae12f546	\N
494	\N	d1cd7b94decc1440	pattern	Match func_8005BD30 -- title_mv_exec2 + memcard initial setup	\N	2026-04-29 18:17:14-05	ac751dbe480ee8e14d0bba85888667748ee4e011	\N
495	\N	a6248afa6c2be83a	pattern	Match func_8006F038 -- gpu_SetSemiTransp + initTexPage tile setup	\N	2026-04-29 17:57:01-05	4cb7e2fdb6870172343fee5a412242d63555d79e	\N
496	\N	87d8a4d41ea42fbc	pattern	Match func_8005B43C -- title_mv_exec2 + memcard_SetSlot setup	\N	2026-04-29 15:10:42-05	8010d858a6426b5c1baf470901a806420f4b39f5	\N
497	\N	fec4126cd56593f3	pattern	Match func_8006A494 -- ot_Link + initTexPage setup (sibling of 6A3CC)	\N	2026-04-29 14:49:37-05	a7bae8bf2063feb9806168d5aa9685ffd672b73b	\N
498	\N	e499de9943f1761f	pattern	Match func_8006A3CC -- ot_Link + initTexPage setup	\N	2026-04-29 14:40:29-05	6a145730daa03873f2d2811ae8b09b5cdc2717c7	\N
499	\N	830ea83469ff52f3	pattern	Match func_80053694 -- D_800A33F4 read+transform with regfix	\N	2026-04-29 14:08:44-05	d3c46265c4a026442055e9784ccfa11f94dd4e3d	\N
500	\N	fb740b98b4bd4593	pattern	Match func_80048AD0 -- snd_LoadBgm/snd_PlayBgm setup	\N	2026-04-29 13:33:30-05	c13184671e4318b783beb31c50b5be031df7a764	\N
501	\N	c457b8998f885934	pattern	Match func_80068ECC -- bit-permute D_8009BC04 from arg0	\N	2026-04-29 13:22:13-05	35271a8d9f32b59d70b52b248e8a457a8bd9b6bb	\N
502	\N	8fdeedd5de800ab7	pattern	Match func_80065484 -- mode-dispatched D_800F0BB2 update	\N	2026-04-29 13:10:43-05	efb306144ae03f8908fdc16c53d9bcc71d18bebb	\N
503	\N	e24a34c53108cca2	pattern	Match func_8004876C -- 16bpp BGR555 color modify loop	\N	2026-04-29 12:25:16-05	16cf7348fa3fd1c3842553ee6c3f0e527f717521	\N
504	\N	0c94fdcf504937bb	pattern	Match func_80056FE8 -- conditional weapon-position lookup	\N	2026-04-29 10:43:56-05	19ec2a030753214ed33a734e076a3eaa3c139525	\N
505	\N	545d5ec5ade68c8a	pattern	Match func_8005BE84 -- title intro / mode change handler	\N	2026-04-29 10:19:52-05	b13bbab57882bea789713e987f7c68877783db34	\N
506	\N	5fa9b252c46f8678	pattern	Match func_80061710 -- sibling of func_80061658	\N	2026-04-29 05:27:53-05	9a3bdf2086b0408e38e04a4cfb6ec1a10cdf0cdd	\N
507	\N	c585a77580381e1e	pattern	Match func_80061658 -- 2-arg variant of func_8006133C/func_800611A4 family	\N	2026-04-29 05:26:42-05	7f9d8c32ffe31421ba32c79b07234cc7a54bd983	\N
508	\N	56a5f12824258b81	pattern	Match func_8006D74C -- 5-call init wrapper	\N	2026-04-29 04:54:36-05	57a496f7a9d9383cbbfcd5d1c9cde21f86ebada4	\N
509	\N	a4bfecbf5e62043a	pattern	Match func_800611A4 -- pre-call setup with stack-buffered halfwords	\N	2026-04-29 04:27:13-05	d9a257ebd590feab7a5f62f5b695428301722ce1	\N
510	\N	62c5ebded773c596	pattern	Match func_80078A68 -- item slot init with packed flag bits	\N	2026-04-29 03:18:35-05	c58b5fa7c93a8c0f8f968f8eb2465bf35e8146f6	\N
511	\N	91f6f6dacd1b9870	pattern	Match func_8006E49C -- buffer pointer table builder	\N	2026-04-29 03:12:39-05	79480bbb48eec0f2b1354947d80e34b4e0be311f	\N
512	\N	84a3cec650f30736	pattern	Match func_80062FEC -- bit-allocation table builder	\N	2026-04-29 03:01:46-05	67f2e06ae3299c816d4192698eff5f0cf34a764c	\N
513	\N	df5103e28632cbc3	pattern	Match func_8007B564 -- sibling of func_8007B4D0 with 0x80000000 OR	\N	2026-04-29 01:33:54-05	0af0de6c98b062480cb7e45649d4eceb79fb407a	\N
514	\N	5251b35fdee2d446	pattern	Match func_8007B4D0 -- packed-arg call with 4-rule register subst	\N	2026-04-29 01:27:51-05	65c6ecf9d261a1af03ab66d330d8736bdfeb8f64	\N
515	\N	615bb5747a4308f2	pattern	Match func_8006E068 -- 4-call wrapper sibling, args saved in s1/s2	\N	2026-04-29 01:14:35-05	593daaefd0b78420ada2d823ea6f6bbb1bc659e2	\N
516	\N	120b8dd5e7697603	pattern	Match func_8006D338 -- 5-call wrapper sibling of func_8006C168	\N	2026-04-29 01:07:30-05	ea32e6ff1d1a75070e6944aadf56f836558d4909	\N
517	\N	a36fe00d72e0c4b1	pattern	Match func_8006C168 -- sibling of func_8006B898 with bigger frame	\N	2026-04-29 00:38:34-05	143a1c06ec89a1efd7da777995b33c1bdaa7aa8f	\N
518	\N	1718208243d2e0de	pattern	Match func_8006B898 -- 4-call wrapper with arg-spill addressing	\N	2026-04-29 00:37:03-05	41339ed0aea77ba37f13d505124a7a3a428fe6ee	\N
519	\N	2ee2b72b78acb7b4	pattern	Match func_80053614 -- sibling of func_80053584 with arg4=base ptr	\N	2026-04-29 00:16:04-05	4dab722e4fdbf69fe1d94158a73d6a4adb6e03b9	\N
520	\N	77ac632ff44a0d57	pattern	Match func_800494D4 -- limit-check write with anti-stomp temp	\N	2026-04-29 00:12:52-05	093488103520aa60f628b8d21c809bb174eb3dc8	\N
521	\N	b13ffb8a42a765ad	pattern	Match func_80069250 -- 2-call wrapper + conditional 3rd call	\N	2026-04-29 00:04:56-05	ed16d336e37aa573b4844199ad0bed72bec3d059	\N
522	\N	5e57925de6e00f91	pattern	Match func_80078E58 -- volatile-counter timer with frame-size pad	\N	2026-04-29 00:00:05-05	05f26df3171e24fd844e48c111c43e9317abc332	\N
523	\N	d8e5d2ffc72b3298	pattern	Match func_8007992C -- memchr-style with reorder regfix	\N	2026-04-28 23:54:17-05	354a3fbd6b018a356c2fc073c2b6c5994a6a9307	\N
524	\N	2d1cdbd7d3d1ece3	pattern	Match func_8007C938 + harden func_8007C97C against label drift	\N	2026-04-28 23:35:55-05	23195f679a7f6137a511eb7bdfd1ec8603db71cf	\N
525	\N	6d12b13f38de52a8	pattern	Implement 4 workflow improvements from regfix-trap session	\N	2026-04-28 23:01:29-05	7b2724955d2c804720553c06750360dbf659d3d6	\N
526	\N	285b2af2e2cb01a3	pattern	Match func_80053584 -- struct copy + tail call (no regfix needed)	\N	2026-04-28 21:56:35-05	2b8635ed1c8ecbc6d8e392b08fb9a0b46f82f893	\N
527	\N	484bad64107c0fa9	pattern	Match func_80062020 -- struct array copy with sentinel + 6 regfix rules	\N	2026-04-28 21:51:46-05	d053ee601b867f6c7d0d7e87a6034ca9e31c9a36	\N
528	\N	abaf589e1a16f2c3	pattern	Match func_8007C97C -- early-exit restructure via 8 regfix rules	\N	2026-04-28 21:22:59-05	794dd948f4830d72709d6bb61147569226db5fca	\N
529	\N	a6470ebc5fd915b7	pattern	Match debug_printf -- varargs prologue reorder via regfix	\N	2026-04-28 20:46:58-05	fc50dc7a5ac79263c470c5293429123a38233416	\N
530	\N	5e6acaadc563eb4d	pattern	Match func_8006133C / 800613C8 / 800614E0 - sibling family	\N	2026-04-28 19:55:32-05	e73c345df9df50e3ecc66438b4afda5a0e9113b8	\N
531	\N	225df9b79a5dcdca	pattern	Match func_80061454 - register pin t/mask only	\N	2026-04-28 19:50:13-05	2de917579b301d2637263c45ecfcff9e1029a593	\N
532	\N	a0fd42f7b9abce31	pattern	Match func_80078EC0 - register volatile + ret=1; ret=0 trick + frame-removal regfix	\N	2026-04-28 19:48:48-05	9b9c2019a56dc531053aafe164ef4d49303e95ac	\N
533	\N	dbcf07334c82711a	pattern	Match func_80077894 - heavy register pinning + reorder regfix	\N	2026-04-28 19:41:34-05	8abe5b0e6aedafce0e482949ea86fbfd53a2c349	\N
534	\N	89241b71ca7a44c0	pattern	Match func_8006517C and func_800651F0 - dedup defeat + reorder regfix	\N	2026-04-28 19:02:41-05	a8a9d5b31381b93bca881f66e6101bd8834ea2f3	\N
535	\N	c012c6e293051ad1	pattern	Match func_80061A3C - inverted if/else for branch direction	\N	2026-04-28 18:21:31-05	11b9669f534168c1157a5707c695029eb289576a	\N
536	\N	05ee2d6bf57fceb3	pattern	Match func_80078824 - sibling pattern of func_800784E4	\N	2026-04-28 18:17:19-05	47d9c9e15f4ee7d115caf89a4a0d60c3dde5c759	\N
537	\N	74103791b4fade8d	pattern	Match func_800484A0 - TIM-loader with 5 regfix rules	\N	2026-04-28 18:05:43-05	07a87df8b249d35b02abd0e344cf9e5867d2d9c5	\N
538	\N	f423acceed080fa8	pattern	Match func_80072FCC - sibling pattern of func_80072F30	\N	2026-04-28 13:32:26-05	ad7a9306d2fd529c3fd38802e617665a1a11e21d	\N
539	\N	05660a8a52a2073d	pattern	Match func_80065000 - early pointer decl fills load-delay slot	\N	2026-04-28 11:55:54-05	d369fc709afe7b566ed6550df038edfa530c8e02	\N
540	\N	8be4d0166c15b127	pattern	Match func_8004881C - constant-mul fixed-point via swap + substs + reorder	\N	2026-04-28 11:46:21-05	93deab84f5cd7dff6700f8a7bf43fbcf112fc3cd	\N
541	\N	3d5a40ee2ca305b7	pattern	Match func_8005509C - byte_indexing_with_la_fold_undo recipe	\N	2026-04-28 11:36:51-05	9bfc89bc927e4225904abe67a5158529cffba58b	\N
542	\N	90e2951c18e55df3	pattern	Match func_8004954C - frame elision via register asm + drain_delay regfix	\N	2026-04-28 11:19:22-05	d511d1fb69e0bdb77a8ba37a529fc3b42c1c0a86	\N
543	\N	93877b5f9dff177b	pattern	Match 3 inline-asm functions in text1b.c via attempt-first pipeline	\N	2026-04-28 11:11:18-05	2e93d12b71f7322dbdf0085426ea5465b2dabb8b	\N
544	\N	1944c8f88766a868	pattern	Match func_80064FB4 -- sibling of func_80064F68 (D_800F0CD0/D4/D8 set)	\N	2026-04-28 02:32:52-05	526ac6200a420faf074c2e0cbfc9ebc4c28726d3	\N
545	\N	0b328c8690deb34a	pattern	Match func_80077904 -- byte-table indexing + forward extern	\N	2026-04-28 02:32:11-05	6c67ac82f9b124db0ac7f207cdeb16380abcb8b3	\N
546	\N	f18d7628a16836d7	pattern	Match func_80064F68 -- integrate permuter match (Tier 0)	\N	2026-04-28 02:21:39-05	673782cb1766e9b803092156b9b67c9daf874a27	\N
547	\N	26e5a1d07623b48b	pattern	Match func_80048A7C -- integrate auto_matches/ candidate (Tier 0)	\N	2026-04-28 02:18:46-05	937cf7f4d30c468b6b9f42d49c526381df3e4ded	\N
548	\N	aef28282a6bd7b28	pattern	Match func_80077940 -- integrate auto_matches/ candidate (Tier 0)	\N	2026-04-28 02:17:28-05	ac081981c909915776499a613a1d3b91e2ff18e9	\N
549	\N	ed99609fea516d4d	pattern	Match func_800784E4 -- m2c return-chain fix + 4 missing externs	\N	2026-04-28 02:00:08-05	28d245c43eafb8a6adab5e368833ffe3e12d86ba	\N
550	\N	4662fe2e30b1e6f0	pattern	Match func_8007C748 -- 3-register cycle via full-insn subst	\N	2026-04-28 00:52:20-05	de6b41a3780cbe8a2c2db6db716eb162e8675ede	\N
551	\N	2e6af5e57233b3a3	pattern	Match func_80078B70 -- clear-flag sibling of func_80078B3C	\N	2026-04-28 00:42:00-05	63e5876b29ddac24a152d6c8212bb584b1102a90	\N
552	\N	ecc4274a532b34ac	pattern	Match func_80078B3C -- byte indexing + drain_delay	\N	2026-04-28 00:40:44-05	03bb3aeed1bbf2e4d7e0470671adf11276c94dd4	\N
553	\N	644911c31a3c48cf	pattern	Match func_8005B7C4 -- single $v0/$v1 register swap	\N	2026-04-28 00:34:04-05	161dc5ab15c2f6050a4307cdc6a815d99b858f59	\N
554	\N	0917dc932022374a	pattern	Match func_800550E8 -- byte-array indexing with offset preservation	\N	2026-04-28 00:29:32-05	b8b918ed6783faffdd4664a7a396acdceb25db4f	\N
555	\N	8281ace7df9190f1	pattern	Match func_80052C10 and repair 30 drifted .L labels in regfix.txt	\N	2026-04-28 00:21:10-05	0070618d7962544b00b6273d12045344f6edd2e6	\N
556	\N	3087a617ec242fb3	pattern	Match func_8008C464	\N	2026-04-27 17:11:24-05	90d7f11eb9a46b67d6ef034194c4a0eeeec749ba	\N
557	\N	c2d59e142f609460	pattern	Match exec_game	\N	2026-04-27 17:10:41-05	d32eb0cc8b920161c1168aa4fa92be272bd64893	\N
558	\N	8242e65f74c7ae5b	pattern	Match saTan2Main	\N	2026-04-27 17:09:56-05	8e15fc3190621951669dfe8a77d32d48e44b9628	\N
559	\N	e6c1b48461fc146b	pattern	Match AllocBukiRmd	\N	2026-04-27 17:09:09-05	f7b423ee3813928041c143ce53e5d270748d1736	\N
560	\N	7e48334d3b1bff79	pattern	Match DispStuff	\N	2026-04-27 17:08:20-05	3bb78f99b9ee52764da856a8e1e05a42f81a3bb1	\N
561	\N	5fd20f56d96809cb	pattern	Match DispUpdateStatusMessage	\N	2026-04-27 17:06:49-05	bb01935ffd1bd8d9d1fa0265e0b1d9307ab79a70	\N
562	\N	82cb26d8ba4f22ca	pattern	Match action_CheckHitZangeki	\N	2026-04-27 17:06:04-05	c58cfc5123181acaefc7e3efc545a0bd45723b29	\N
563	\N	576524e7ba141c96	pattern	Match md_game_end	\N	2026-04-27 17:05:17-05	d26d20638b98698d2793d874a97e15b32a8534d4	\N
564	\N	103c9e39e57ea573	pattern	Match func_80086CF8	\N	2026-04-27 17:04:38-05	610e2f7bf744fcbdccf8372f4fdce4760412b51a	\N
565	\N	04f7406da077cdb8	pattern	Match func_80087770	\N	2026-04-27 17:03:57-05	8916f0be3407ce0131606aced0971d2a54c8003b	\N
566	\N	d1e27c0480ca76c0	pattern	Match func_800872A4	\N	2026-04-27 17:03:18-05	e678df98733922cc706c1e1cd4a01a81e7ece7d6	\N
567	\N	c484c8f2c84bd26e	pattern	Match single_game_setModeRequest	\N	2026-04-27 17:01:33-05	076ced0d8e51a2a28e2679ed97bccc477270f73f	\N
568	\N	7b762ef9a1b7983f	pattern	Match func_800198D0	\N	2026-04-27 16:53:07-05	00de540ca0c92ef243d927ad528dd5da6822d0cf	\N
569	\N	ad30e3137ca5db44	pattern	Match func_8001A820	\N	2026-04-27 16:52:26-05	8c8e8d03fc17a77f5d2487f1bbdc115c4d53b0e5	\N
570	\N	c5d9aad9e86d554d	pattern	Match func_8001BE20	\N	2026-04-27 16:51:00-05	5f128c134c55abb6a9ec723f423ee0c8796638d2	\N
571	\N	63f2e440b351ced9	pattern	Match DispPracticeMenuTex_B	\N	2026-04-27 16:50:15-05	613dfaef4721879da47665fc309cfebed0f641b4	\N
572	\N	e95ecbadb440fae3	pattern	Match mario_test_Exec	\N	2026-04-27 16:49:38-05	c597ba61b409d30ce862ceb2dd42bf46ea7adb97	\N
573	\N	a9482faaf44fbe78	pattern	Match md_game_rob_data_init	\N	2026-04-27 16:49:00-05	8b70888b9a56bd234820dac027b7ae3b756fc5e8	\N
574	\N	0a2946dd4604d8f0	pattern	Match single_game_CheckStatusUpDataTotalOver	\N	2026-04-27 16:48:19-05	20f5f863cac2fc0801b1bc3a0f05857fc164f2c9	\N
575	\N	f78840a3d90a2365	pattern	Match func_800207C8	\N	2026-04-27 16:47:40-05	051d8548fd9465cd19013bedf234c253c6f08c7c	\N
576	\N	255048d2d93e42d0	pattern	Match func_80021DB0	\N	2026-04-27 16:46:59-05	6c66dc69d164399821119f9c60866c8c21a8405f	\N
577	\N	ba3469cbfb12cb8e	pattern	Match func_8001C8DC	\N	2026-04-27 16:46:13-05	b1746287c8074d35c1bd1f02729d31cad33d3104	\N
578	\N	d5a8ae3f9df39cbc	pattern	Match func_80026DA4	\N	2026-04-27 16:44:46-05	fb9f60b54103932ef50d1741f99e9814b9b9ad3c	\N
579	\N	e70d80808be26168	pattern	Match camera_set_target_zoom	\N	2026-04-27 16:41:46-05	2be240b96e878d97ffd5a7438c864d5d294b2b1c	\N
580	\N	0f1ea1ef0a2de094	pattern	Match func_80021424	\N	2026-04-27 16:31:32-05	c79b2140c35d343a1cb98786b5394484786b69d7	\N
581	\N	d05052fb76ca9bc1	pattern	Match func_80022580	\N	2026-04-27 16:28:18-05	5b5a33ffe3e704e94f9faef2aad17a793459f5e3	\N
582	\N	4fb4e34da1fedbd8	pattern	Match func_80023F08	\N	2026-04-27 16:25:06-05	091f043610f71f400c3c44b86ad488809416690e	\N
583	\N	de0347160df4c6f5	pattern	Match func_8003993C	\N	2026-04-27 16:24:01-05	be6b327c7c351acb15b881ae250942a84142f017	\N
584	\N	26d5d7271925ca25	pattern	Match func_80027AD8	\N	2026-04-27 16:22:44-05	be3dcc28a397286ae9f04f5bc4877bdba9351616	\N
585	\N	68b8b5df12eae640	pattern	Match saTan3MainJump	\N	2026-04-27 16:09:11-05	a8e14304b1381a6a59f90f6cbe9cb15feef175f9	\N
586	\N	8cb2e2e39f74b654	pattern	Match DispPracticeMenuTex_C	\N	2026-04-27 16:08:30-05	c74d3d19b5b561bef433f10b0a82c76937d03c09	\N
587	\N	5220ddbbe90243b3	pattern	Match func_80029454	\N	2026-04-27 16:07:47-05	83dec8105e73f859ac39729801b6ca5267c1d134	\N
588	\N	dbc3b12b29aa297e	pattern	Match func_8002A458	\N	2026-04-27 16:06:59-05	2275be1fa58183a104974a360e39ebf264671c73	\N
589	\N	73bc255e3383b38a	pattern	Match calc_loc_mat_fw	\N	2026-04-27 16:06:21-05	214e09a7ca381bfea37900ed09359062ecf6a3f6	\N
590	\N	e6b773a86fb5c403	pattern	Match PutRobShadow	\N	2026-04-27 16:05:41-05	41dadfe22b533e589a5414f1886249f35218d28b	\N
593	\N	b85e5c43d094e84d	pattern	Match func_8002F2D0	\N	2026-04-27 16:00:59-05	8be2993216537c5aaca419ce8acf36ce7bb89e2e	\N
594	\N	f97e24d27861946b	pattern	Match func_8002F770	\N	2026-04-27 16:00:21-05	d1a089dc940217595dc702bd70a87ef426899974	\N
595	\N	da9605e29e33a48b	pattern	Match coli_hit_body_weapon	\N	2026-04-27 15:59:35-05	8eb2b7f3d69c85b0db0898f5aad1ace21d5c0a93	\N
596	\N	50914a2fa7075fb4	pattern	Match func_80030D7C	\N	2026-04-27 15:58:50-05	088b4b0a2bddf22abb4a3ccb73a0852a00890b70	\N
597	\N	a9ba0fb2ff879847	pattern	Match func_80031B24	\N	2026-04-27 15:57:41-05	0fe0bf09bc0357bb1462bbd36a78490b11f8bebc	\N
598	\N	560927ce308e6e4f	pattern	Match func_80032C50	\N	2026-04-27 15:51:22-05	1e3e9490970a9d2c0b189a40582f19987191b57b	\N
599	\N	e0d7356e6971c5d2	pattern	Match func_80034708	\N	2026-04-27 13:38:44-05	31e5c9b7c99fc20b5fba18881811e16e9bea58ba	\N
600	\N	9fa6703097f86d76	pattern	Match replay_camera_get_attack_number	\N	2026-04-27 13:38:00-05	4e01731d728f19a1d2f1a04e028c5cba790f5512	\N
601	\N	915733e8244f66bb	pattern	Match DispHira	\N	2026-04-27 13:37:24-05	ca48f8c7309f7dbe8a1a8dae85007428f94bf27d	\N
602	\N	923d6fdf668d2b24	pattern	Match ang_hosei	\N	2026-04-27 13:36:45-05	cc27185d4b6ea7f6b5288be75bbc767c7c1a4072	\N
603	\N	abb8472f24e313f2	pattern	Match md_game_check_mode	\N	2026-04-27 13:36:08-05	fbf67b6bc37a04e355a05d7d0da402bfe85a6c9a	\N
604	\N	dd5406807cb98fc1	pattern	Match saTan1MainJump	\N	2026-04-27 13:29:07-05	c463637c22b3908ee8d72bab49b4a75ead3c7a6f	\N
605	\N	4986b83978e70d4b	pattern	Match special_camera_Exec	\N	2026-04-27 13:26:51-05	8666a2ed93be8da2987fa96c9a56e10d86d27e6e	\N
606	\N	24cbd48ac5816425	pattern	Match special_camera_set_win_cam	\N	2026-04-27 13:26:06-05	a3cd17daa75d21cb2cfd7a2756b8b656df9ddf7b	\N
607	\N	4c3d228e6a0966f7	pattern	Match func_80045B68	\N	2026-04-27 13:25:21-05	4585da8d376ca0bab58b9b64b55372e5defacbd2	\N
608	\N	3f843d66e5115014	pattern	Match func_80043454	\N	2026-04-27 13:24:38-05	10b1b1c4a58e753820fce711f158c48947ced952	\N
609	\N	a5ba98bddd4ada3a	pattern	Match tslTm2LoadImage	\N	2026-04-27 13:23:53-05	50c26827d4f08d6f3010133b5a0e1a9f8bdb50cd	\N
610	\N	199d10e9656520eb	pattern	Match tslTm2LoadImage_2	\N	2026-04-27 13:22:25-05	cc7c47aea0d0bc1c3e6078e7f324a96cdd45b000	\N
611	\N	241898685bd6832a	pattern	Match func_80080828	\N	2026-04-27 13:17:46-05	a947517a802f7b65b5d8c6e51864de9e9f3baea4	\N
612	\N	f68e4b64a9b5076f	pattern	Match func_80035828	\N	2026-04-27 13:10:37-05	eb44851a1bc15cb4955ecd89aac1bbb8eafc9efe	\N
613	\N	a738dce7bcd24b6c	pattern	Match func_8003FA24	\N	2026-04-27 13:09:58-05	a925e9fc37a6c45fe946ba7b5e969b6489c509d3	\N
614	\N	4743a6a6a83b41ec	pattern	Match func_80082D34	\N	2026-04-27 13:09:10-05	6977b72ea7032d3ebe212e0708af77a1adc2766d	\N
615	\N	67762b3c387377a5	pattern	Match func_80017A44	\N	2026-04-27 13:07:13-05	e169033130557d16c7b4f650fc56059c23bfe335	\N
616	\N	b04984670e76ae28	pattern	Match func_80089F3C	\N	2026-04-27 13:04:25-05	2f46891bc62a08270e17ed7b4ccd9348438926de	\N
617	\N	6054ab3de5885dc1	pattern	Match func_8008AF9C	\N	2026-04-27 12:58:34-05	49f74fe6cbc0df3a838c47b4f39f4354ac021399	\N
618	\N	da9601e080cf827c	pattern	Match func_8002CA8C	\N	2026-04-27 12:48:34-05	bbce2bad1ac10f1b3a764d0abb97b8197a3353bd	\N
619	\N	0c144112ed79290f	pattern	Match func_8003EB84 and update handoff	\N	2026-04-27 11:58:28-05	0829e53d24f9dc287a344a8a39ed8646a259c770	\N
620	\N	583a7d99f4abb815	pattern	Match cpu_check_run_attack	\N	2026-04-27 10:50:28-05	d5ba1902235309b53505d7e2641437f8fff2fa5a	\N
621	\N	467c8407e229fe72	pattern	Match func_8002304C (tanren_CameraControl) â€” 216-insn camera with scratchpad + 16-rule compound regfix; fix +16 byte d	\N	2026-04-27 06:03:20-05	dab4eb89f485244f51e2af39ba7c9c9a0b2e3556	\N
622	\N	36b2b5b887414e75	pattern	Match saTan0KiWareMoveB â€” 212-insn GTE collision/rotation with outer product + sqrt + 2x MVP transform + 13-rule compo	\N	2026-04-26 23:13:11-05	8cb90559496b111eee43bda0385d58c038259af5	\N
623	\N	b1b737a327dd361a	pattern	Match saTan0KiWareMoveA â€” 212-insn collision/triangle test with GTE, div-by-3, sqrt, 3x saTan5TakeAnim2 + 66-rule comp	\N	2026-04-26 21:56:48-05	c709ef1170d9cc6e6ad67ba13df16b96e1e69c39	\N
624	\N	b6f533881ad13413	pattern	Match func_8002C61C â€” 284-insn game main loop with Vec3i struct copy + 74-rule compound regfix (68 stubs remain)	\N	2026-04-26 20:13:54-05	fd55819f97c2c84b9ed590aa5fa17fd402a1bbdf	\N
625	\N	a68814242aa616a2	pattern	Match func_80032854 â€” 304-insn animation dispatch switch with 50-case jump table + label renumber regfix cascade (69 s	\N	2026-04-26 17:42:53-05	bd1879e9fce75ff4160b50c00e573b10239c3199	\N
626	\N	0b932659bb98c102	pattern	Match motion_SetMotion â€” 396-insn motion state machine with anti-cross-jump + inline rodata padding + goto block reord	\N	2026-04-26 15:42:14-05	4bda8b35e320ed7f6bfd444e4ffd0a08a4c11499	\N
627	\N	e2e61d50be99b321	pattern	Match func_80038988 â€” 205-insn quad-switch selection state machine with 4 GCC jump tables + rodata padding fix (77 stu	\N	2026-04-26 11:50:36-05	34f5ab7ea2c237a37f8e1d51be0fb22a6ab6a1c0	\N
628	\N	db67290e948aba8c	pattern	Match DispSchoolBG â€” 189-insn GTE school bg renderer with scratchpad + 18-rule compound regfix (86 stubs remain)	\N	2026-04-26 10:31:17-05	7b6c1da02f5186932681f4531ff17fcff43e64d2	\N
629	\N	bbaaca0720c3c87a	pattern	Match func_8008BB24 â€” 79-insn fixed-point attenuation with delete+insert label detachment + 41-rule compound regfix (7	\N	2026-04-26 08:57:51-05	802ad6f891f41e93c619dee83118dc1abb39fb78	\N
630	\N	5766544b5f62cacc	pattern	Match saTan2KabutoWareMove â€” 215-insn kabuto weapon AI with tail duplication + 45-rule compound regfix (88 stubs remai	\N	2026-04-26 07:13:36-05	33e85b6d5be43332983f6586a6d55af47f00f6f0	\N
631	\N	24271c03ea9b69f9	pattern	Match cpu_set_move_command_and_dir_for_no_action â€” 189-insn CPU AI move shuffler with s32[2] array stack-force + 6-rul	\N	2026-04-26 05:12:45-05	aa0704164b9c8d720337198d824ee143c018a1ff	\N
632	\N	dc5d934978fde0f7	pattern	Match func_800233AC â€” 169-insn collision ray-trace with packed struct lwl/lwr + 4-rule regfix (76 stubs remain)	\N	2026-04-26 04:12:27-05	71466484b71b5978353891d5d04652bf7a57bf6c	\N
633	\N	eb0bb1c01765cb37	pattern	Match func_80040D48 â€” 272-insn switch/linked-list initializer with rodata empty + 39-rule compound regfix (77 stubs re	\N	2026-04-26 01:52:33-05	9d52cb3cea86f8477adf762703bf63e3ec4b152f	\N
634	\N	aed22b884b6529b2	pattern	Match marionation_camera_Exec â€” 155-insn GTE camera with scratchpad + 74-rule compound regfix (78 stubs remain)	\N	2026-04-26 00:46:50-05	d014ca1f0f4032ff64fb9e6b1884fa46496db543	\N
635	\N	b6c3d7905ed52150	pattern	Match mk_leaf_newpos â€” 176-insn leaf spawner with 41-rule compound regfix (79 stubs remain)	\N	2026-04-25 22:25:53-05	900ddb47096320301f6538f248f5d7468e78cabe	\N
674	\N	48185c7aa480b091	pattern	Match cpu_side_move_dir_2 via C + 2-rule insert_after regfix	\N	2026-04-18 14:26:48-05	7fc5a1e0b38e22f13045da0441115c458ab0c27b	\N
636	\N	251d92bfafc6d873	pattern	Match func_80031890 â€” 167-insn GTE rotation/decay with s32 load widening + 3-rule regfix (80 stubs remain)	\N	2026-04-25 20:20:08-05	ba5adf7ce61fb0a7c3655962706b8095f751e806	\N
637	\N	4ae2f19f4d5b0e29	pattern	Match func_800325E0 â€” 157-insn camera distance/angle brightness with GTE sqrt + 23-rule regfix (81 stubs remain)	\N	2026-04-25 18:35:38-05	543dcbb1a8700ad55df98c76cef4b6cf4820c1cb	\N
638	\N	c66074b66890c967	pattern	Match saTan5TakeAnim2 â€” 154-insn line-sphere intersection with 36-rule compound regfix (82 stubs remain)	\N	2026-04-25 16:25:26-05	4284cdaee6a67244acfb86f6b0491f27a61839ef	\N
639	\N	567fb6fe62d36ab1	pattern	Match DispSamnailWindow â€” 149-insn 7-case switch with rodata sandwich split (83 stubs remain)	\N	2026-04-25 13:47:58-05	84c802a95c10eef4a54acd5d5d0e244a2f7cbddc	\N
640	\N	7ddc6f55392649c4	pattern	Match func_800460E4 â€” 249-insn stage loader with CU split + 18-rule compound regfix (84 stubs remain)	\N	2026-04-25 02:30:15-05	567afaf19ab18b4d5985339b0c7dc9c3955b8157	\N
641	\N	21400671de86a2b1	pattern	Match efc_rob_set_type_flash â€” 204-insn particle effect renderer with 21-rule compound regfix (85 stubs remain)	\N	2026-04-25 00:06:25-05	bc57f3eb7f03bae8591078577e8e458ce82f6642	\N
642	\N	25f9a1c9927f9ad4	pattern	fix: restore broken regfix rules from conv_matrix_rotation commit	\N	2026-04-24 19:43:58-05	7b7221b1cc60e3195aaadf200c78b4a93a9450c6	\N
643	\N	97d47938cf282825	pattern	Match conv_matrix_rotation â€” 3-function IRQ handler stub with 33-rule compound regfix (86 stubs remain)	\N	2026-04-24 15:03:01-05	0840580c63bd0524936a43bac91f000a39e4d35b	\N
644	\N	cf4ce2570a0ee558	pattern	Match cpu_set_move_command_and_dir_for_no_action_2 â€” 189-insn game main loop with 25-rule compound regfix (87 stubs re	\N	2026-04-24 13:05:46-05	01abc5e9e2c2f7e3368651c32fc04a03a156c3c7	\N
645	\N	486e4ba950b20792	pattern	Match func_8003DE14 â€” 179-insn VRAM pixel fade/blend with 36-rule compound regfix (88 stubs remain)	\N	2026-04-24 08:45:36-05	6c51b01b2cdc0576fa5ca251860ed0bcdfe7539d	\N
646	\N	35f331b091512c43	pattern	Match DispSleepMenuTex â€” 146-insn variadic printf-like text renderer (88 stubs remain)	\N	2026-04-24 06:33:49-05	b86e82efba1d8d20e184056b43b45f9fb38e9780	\N
647	\N	00ca36b613558b37	pattern	Match func_8008BC60 â€” 75-insn SPU pitch-to-note converter (89 stubs remain)	\N	2026-04-24 05:16:03-05	0c885040def02ecc2f366f89e78545bfe88fcfa0	\N
648	\N	5d40b52c2263e777	pattern	Match func_8002BEA0 (rob_Scale) â€” 145-insn perspective scaling with GTE sqrt + asm barrier (90 stubs remain)	\N	2026-04-24 04:18:38-05	bb60a3dc91a9538c6da55797881e018e89660404	\N
649	\N	a2c2f714af1871cd	pattern	Match saTan5TakeAnim2_2 â€” 154-insn sound/IRQ dispatcher with switch, signed div (91 stubs remain)	\N	2026-04-24 02:04:36-05	5293b15612a77867e6dc50db3c13da7c22abe54a	\N
650	\N	fdd3724ba9938e78	pattern	Match camera_set_zoom â€” 219-insn camera zoom/angle dispatcher (92 stubs remain)	\N	2026-04-23 22:24:57-05	c97da3e496cc113d44b6e06fe0846f9f4535f399	\N
651	\N	e853b87afe5804f1	pattern	Match func_800841E0, func_80084500, saTan0Main + fix broken build (93 stubs remain)	\N	2026-04-23 18:37:43-05	2837d67be83e2366c6b927627c7cd3e452d3fead	\N
652	\N	e49091f3a0a705f9	pattern	Decompile func_80016E60	\N	2026-04-22 23:58:44-05	6ac9a6b8ac84a921d498817e508d7a74ebf45894	\N
653	\N	1978536fc9b3a921	pattern	Decompile single_game_VoiceContorol	\N	2026-04-22 23:45:51-05	aff014907eef08a43926d3242ae6cf148247e2bd	\N
654	\N	a4e12be5dfb5ef12	pattern	Match coli_HitPauseKatana â€” 191-insn SPU heap allocator (111 stubs remain)	\N	2026-04-22 15:22:26-05	67b601834c6de651122ff137aed42b8d367f5990	\N
655	\N	eb64694854c3a391	pattern	Match suDispMentalBar â€” 179-insn switch command dispatcher (112 stubs remain)	\N	2026-04-22 12:01:20-05	f2d6c199517d51b5702c3bfca76fa7947507e193	\N
656	\N	241a7ea2106c4d82	pattern	Match func_8003AE5C â€” 70-insn command byte stream interpreter (113 stubs remain)	\N	2026-04-22 11:12:51-05	265a9b8db39b1ceddf1c9bb7d9bb35e5d3fe867f	\N
657	\N	9bc0489633503b8a	pattern	Match marionation_Exec â€” 179-insn Marionation engine poll loop (114 stubs remain)	\N	2026-04-22 05:19:58-05	7c40b1254dfba489b1cb21199b3f5c729f58957c	\N
658	\N	97c15db6e4f630a8	pattern	Match cpu_side_move_dir_4 and coli_HitPauseKatana_2 (115 stubs remain)	\N	2026-04-21 23:37:33-05	d970392f44c7145ba15a1982a35ebba64295b6ef	\N
659	\N	e19288d6adaa8690	pattern	Match func_8002BC68 via GTE inline asm + 6-rule regfix	\N	2026-04-20 12:23:23-05	7179070d08309460a6f1afcd1409c00093904e4d	\N
660	\N	4f6b91cff2aaa7e6	pattern	Match func_8003CF84 via volatile pad + label shift in regfix	\N	2026-04-20 11:25:47-05	99462945df5298fc55257b06d89303e1c462863b	\N
661	\N	60c59164db83a7ed	pattern	Match cpu_get_move_pattern_table_number via C + buf[4] + ret-temp idx	\N	2026-04-20 04:42:10-05	8d342343276736c240394885e2e8a77af055fdda	\N
662	\N	98a4bad1909cb82d	pattern	Match DispPracticeMenuTex_A via C + 28-rule compound regfix	\N	2026-04-20 03:41:09-05	fc12aeb4b74ca1788ac29031ee76fad1c20e408c	\N
663	\N	0ae358d5de953bfe	pattern	Match saTan0GaugeDraw via C + 8-stage compound regfix (~45 rules)	\N	2026-04-20 00:05:57-05	0d7e5d2b960745b75ce59935a0cb04a96b263d25	\N
664	\N	7a25824aa962b574	pattern	Match SetPacketData via C + 13-rule compound regfix	\N	2026-04-19 21:42:17-05	aeaf1a290948f8af0891f882a9d6040f87ef607a	\N
665	\N	9095d7ca54ea6311	pattern	Match func_8003A728 via C + 11-rule regfix (4 clusters)	\N	2026-04-19 13:31:43-05	cb184a2a53a780a98b1552ecc89dbd0e61e3995f	\N
666	\N	16c18d45c641fe22	pattern	Match func_80038170 via C + register asm + 1-rule regfix	\N	2026-04-19 02:24:02-05	c6fef2f6c0ead713c1790a7a1a3f16eecb5ffbdb	\N
667	\N	da95ec59ea572de2	pattern	Remove 3 dead label-drift regfix rules surfaced by validate-regfix	\N	2026-04-19 01:15:43-05	d7316d64969f34d6ddf6973df54b37ff9a494d87	\N
668	\N	5d348f1cfc569165	pattern	Match func_8003B9D0 via C + opaque ptr asm + 1-rule fill_delay regfix	\N	2026-04-19 00:36:29-05	26296210ea7e219e547036b8ee5ddb46107fbd6e	\N
669	\N	08d2055cfb9f958b	pattern	Match cpu_side_move_dir_3 via C + 6-rule regfix + fill_delay fix	\N	2026-04-19 00:07:05-05	7ff85ddbb94d40258ab8254f110ffd6670b4ef5e	\N
670	\N	4d5c9dca7afff1ca	pattern	Match pad_FuncAnalog via C-only state machine + register asm v0/v1	\N	2026-04-18 23:01:35-05	b4b5b95727af9144cbac64a62606352e58890f00	\N
671	\N	aef31ad7fdb877c7	pattern	Match func_80088740 via C + drain_delay/fill_delay primitives + 14-rule compound regfix	\N	2026-04-18 21:40:04-05	c6214461a483deaea7aab3685142c4a6301e9f84	\N
672	\N	bac6fc9807ff964a	pattern	Match func_8003C9A4 via C-only + .L193->.L218 label rename in regfix	\N	2026-04-18 19:15:13-05	41378a383a67302b928a9ccf6d7dd15bd7ff9b79	\N
673	\N	debf599ab2e145a3	pattern	Match cpu_side_move_dir via C restructure + 18-rule compound regfix	\N	2026-04-18 17:04:06-05	71b3c2a7b03f1d97fae16d3a7fb1b79254722d32	\N
675	\N	efa86600cc5960f9	pattern	Match func_800856B0 via C + 66-rule compound regfix	\N	2026-04-18 11:34:45-05	32c8f2e8723d142159dfa17fc7f4435e0d655a0a	\N
676	\N	06302c7265b985cf	pattern	Match hirahira_w_ctrl_2 via C + volatile lhu + 63-rule compound regfix	\N	2026-04-18 10:30:47-05	5871a955697c154511f4c2412b17fb1d97d28e0e	\N
677	\N	baea99e10eb0769c	pattern	Match mot_data_set via C + register asm + 15-rule regfix	\N	2026-04-18 08:31:10-05	f3b9dfb5918d87b44fcde7cbd5ee1692eaf5b6bc	\N
678	\N	2e4a8e3ab79f1466	pattern	Match _SelectSection via C + volatile lhu + 11-rule regfix	\N	2026-04-18 07:49:53-05	03cc37ed3257b4bab6b629ff7a31a711d8b82079	\N
679	\N	c055e2d93bfdbb2a	pattern	Match replay_camera_rob_back_loose3 via C + volatile lhu + 8-rule regfix	\N	2026-04-18 07:21:30-05	4aa737c3710e0feae10b5260b49d1e4b0eb1e66a	\N
680	\N	6d27d7a758ca48e6	pattern	Match AllocRobRmd via C + regfix nop insert + texture reorder + asmfix label fix	\N	2026-04-18 06:35:30-05	ccf51be77eac35bdc1c72f0fbd7f37ca7fcca92d	\N
681	\N	c806269dbb377eb6	pattern	Match hirahira_w_ctrl via C + 18-rule regfix	\N	2026-04-18 01:54:07-05	5cc54a051b198be58088b543e53f794e4c019e97	\N
682	\N	8268baf12669126a	pattern	Match func_80084A7C via C + 11-rule compound regfix	\N	2026-04-18 00:33:30-05	58de07d94b09cafdc7ee44eb95ec4aa4b19835b4	\N
683	\N	9a5d6f21b193a573	pattern	Match func_800861BC via C + 30-rule compound regfix	\N	2026-04-18 00:10:59-05	31bbacad905fafdd8db89a986048de2d789fb828	\N
684	\N	dc9625dd3c0d9375	pattern	Match func_8003504C via C + 11-rule compound regfix	\N	2026-04-17 23:01:10-05	989afa7bc5c76ede044e4be19b488075f8a667ca	\N
685	\N	181d9b36b5023696	pattern	Match func_8003DBE4 via C + 36-rule compound regfix	\N	2026-04-17 22:01:06-05	030a187d18a2350ffcac50af8f3845d39da5c3d8	\N
686	\N	90a6185afa8eaafa	pattern	Match func_80017848 with asmfix	\N	2026-04-17 21:27:06-05	7381923bc475aaca469022f8266a14795cd80fe7	\N
687	\N	c62e6dee82c0f986	pattern	Match func_800200DC via C + 14 regfix subst rules	\N	2026-04-17 18:33:50-05	d226be693e7cffbc375b6be0bd104ea60a7a134b	\N
688	\N	fd21761dd7f28a13	pattern	Match func_80022F34 with C+regfix; revert coli_hit_body_weapon (GCC crash)	\N	2026-04-17 17:44:13-05	7347f9cff2717346fbc8791592f7f9a1fc6373ef	\N
689	\N	4ab0c1bfd6b2c821	pattern	Match coli_hit_body_weapon with asmfix	\N	2026-04-17 16:57:26-05	38754381828a85384f1ae617822f813d064ed652	\N
690	\N	49537f5fe179047a	pattern	Match func_80022F34 with asmfix	\N	2026-04-17 16:46:37-05	b4aa7e9253588f917801eca9613d9983b696b43b	\N
691	\N	f195d9447ce209d9	pattern	Match pad_main_control with asmfix	\N	2026-04-17 16:39:05-05	6a49a1c34bea786e2d75e70d3e27ed72e468d6c9	\N
692	\N	9c78267c76b6e55b	pattern	Match func_80023648 (replay camera frame rate / speed)	\N	2026-04-17 15:58:09-05	36a7ca336cdf7ea108213ce632389a6b4979ca5e	\N
693	\N	5ba2c183e88b2b91	pattern	Match func_80033DF4 with localized asmfix	\N	2026-04-17 15:25:37-05	ad013c8ee275d49ba8a97c2a28735590f2d31a6c	\N
694	\N	5891385a46c6e84d	pattern	Match func_80021A98 (special camera animation setup)	\N	2026-04-17 15:15:37-05	537c516ce3ac9fe8302853d50c87e77229fe771e	\N
695	\N	fc11aba4b44c6ea4	pattern	Match func_8001E404 (saTan3GaugeMain camera setup)	\N	2026-04-17 13:16:19-05	bd80f80e6e574f0f65e238766074ae7633b9e8fa	\N
696	\N	d3446667a5e2bd13	pattern	Match saSeInit_2 with localized asmfix	\N	2026-04-17 12:27:11-05	a5933af914a7f21df9648c0ab8016cec5cfb4281	\N
697	\N	c99ba7fb23a0a663	pattern	Match func_8001EFA0 (tanren camera set target position)	\N	2026-04-17 11:56:07-05	fe8833c1495f41816a7c79483f34fac13edfdb04	\N
698	\N	8d91468f4056de46	pattern	Match saSeInit with localized asmfix	\N	2026-04-17 11:41:05-05	8d6a369b7ad6e15b1e33129455d591a9975772fe	\N
699	\N	9a86df72b0a109d8	pattern	Match func_80016A8C with asmfix	\N	2026-04-17 10:28:49-05	d8f7ddd78cfa7ee46c912b19311b750f5cc52483	\N
700	\N	1f0a0bb8d2f2b648	pattern	Match myRobGeneiMove (134 instructions)	\N	2026-04-17 03:33:06-05	7a60002df583215a10963c7b04af91f6050c2d62	\N
701	\N	9a469a77e6177f32	pattern	Match func_800826CC with regfix	\N	2026-04-17 02:25:32-05	27724b4bb2b340eb8d11cea9c9d1afc1cc218686	\N
702	\N	8b86625f34a980fc	pattern	Restore matching around func_800372F4	\N	2026-04-16 19:07:45-05	a4ba94749c299f30584691682bfceaf0bad5da02	\N
703	\N	a4ed7cac37e0f214	pattern	Revert "Match conv_matrix_rotation via split asm helpers"	\N	2026-04-16 18:48:54-05	18df57f71781788c227a37a49c50e203e8701bd3	\N
704	\N	4c536b0d79403b97	pattern	Match conv_matrix_rotation via split asm helpers	\N	2026-04-16 18:47:20-05	01cc02b67484e97f76618a448e5ac2e3d05558a6	\N
705	\N	b172122b03ce29ac	pattern	Add per-function asmfix tooling and match replay_camera_rob_back_loose2	\N	2026-04-16 18:12:06-05	d5c68b03b50c426caa9dc148979f74526dbec4e1	\N
706	\N	d49a2499034ad333	pattern	Match tslPrintScreen with two-stage regfix	\N	2026-04-16 13:01:32-05	0900079b001392885840e12f4da736a6da3ce792	\N
707	\N	1eaca467625a425d	pattern	match: func_8001F938 â€” lhu+andi split + 0x270 block regfix patch	\N	2026-04-14 19:21:30-05	79a45cc53b4595b7d582d1590cd3dcdda8f08d11	\N
708	\N	836f5a31384fc7ab	pattern	match: func_80041AC8 - rect builder loop w/ regfix reorder	\N	2026-04-14 12:28:25-05	32016d40d757474d081e784b8a0eb2aae5fa8dfc	\N
709	\N	4f2346e4d757d2d7	pattern	match: func_8001C624 â€” permuter-discovered do-while(0) block scoping	\N	2026-04-14 10:29:41-05	6514a1296e38243e79bfb272243f1cc0cb027a3b	\N
710	\N	ba85c2c0852b9976	pattern	match: func_80037D14 â€” CU split + rodata split for switch jtbl	\N	2026-04-14 09:13:26-05	14545f80b1caff50c81a731557b1501608b42a14	\N
711	\N	25ea89b8fcb18b0e	pattern	match: func_80045878 â€” v0=s1 base + v1 scratch tail block via regfix	\N	2026-04-14 00:15:33-05	4530643bcede10f352d5d3a2bce040cde439590f	\N
712	\N	d9d19b188e3f5035	pattern	match: func_80086014 â€” register asm + memory barrier + regfix frame insert	\N	2026-04-13 23:10:56-05	579715332982036dabc21b7b554c3f0291df2ab2	\N
713	\N	5857a838600c213e	pattern	match: func_8002D320 â€” inline asm port of func_8002EA24 pattern	\N	2026-04-13 23:01:39-05	32c8bc8777e83965b506e65b951d4f3b3801a8a6	\N
714	\N	6449a62960bd2b2f	pattern	match: func_8001A67C â€” GTE CLZ log2 path with regfix reorder	\N	2026-04-13 22:05:25-05	d4ab5f45c31613941bf85afde3d5be7fcaaf7b3a	\N
715	\N	1c8ce6d88d28df94	pattern	match: func_800274BC â€” log2 dist + GTE LZCS with regfix srl reorder	\N	2026-04-13 21:49:52-05	da02fdfa4f25fa75ee95ef8acbea9f526c6190a8	\N
716	\N	993a98530e761bab	pattern	match: func_8001BAE4 â€” Judge cast + regfix srl->sra subst	\N	2026-04-13 18:06:10-05	e3c284bbc3a412a36cb5cc17b6bb78a122c75ba1	\N
717	\N	d2cfeadf88fd2721	pattern	match: Pad_Prs â€” log2 dist check for 4-entry pad table	\N	2026-04-13 13:51:48-05	83dd77525de6fd7dd516e38e2ce10b6d63d0ccc2	\N
718	\N	e726baf9ac38c84f	pattern	decomp: func_8002EA24 (inline asm) â€” GTE rotation + distance check	\N	2026-04-13 10:02:15-05	76ec25370c84078a3324ac87f216cb8a42faaeb9	\N
719	\N	b7efedb7e09f8a75	pattern	match: func_8003C560 â€” code6cac_c2 state dispatcher	\N	2026-04-13 09:53:00-05	1238398d6a492197f7882f7c7427927b785692a2	\N
720	\N	0909bfee33dff8ff	pattern	match: single_game_SetAbilityData via inline asm GTE + regasm v1	\N	2026-04-13 09:37:25-05	1e73d649253a804f726bb5cd0abf3492f55aaa21	\N
721	\N	47dc541f71ad6bd8	pattern	decomp: match gnd_land_hit_char_die_main (text1a.c)	\N	2026-04-13 09:17:36-05	af3d9829bcd729a11724927c50a36dd902cb721e	\N
722	\N	ca1d481f7e10cb60	pattern	decomp: func_8002FF20 â€” 3D motion init w/ GTE matrix transform	\N	2026-04-13 09:11:15-05	4d50c69c4836ca2be9e108d3f34f142a9a34cfec	\N
723	\N	2771b0596d891653	pattern	match: func_80022224 (tanren_calc_rob_pos) â€” rob spawn position picker	\N	2026-04-13 01:20:25-05	fea5adc0ca6b747633dab46c3fe1394982cf784a	\N
724	\N	5353294912db5780	pattern	match: func_80085270 + 2 wrappers (spu motion init) via regfix reorder	\N	2026-04-13 01:15:20-05	5bce99405a88b97e02f497d0be916465b0407425	\N
725	\N	4e79050faaa57e0b	pattern	fix: func_8003AB44 regfix labels and dedupe comment block	\N	2026-04-13 00:40:30-05	d808eab4b95e524445e63b4f540eb83329414a39	\N
726	\N	f7e4e23c30d8d715	pattern	match: single_game_SetWazaData via layered regfix (code6cac_b.c)	\N	2026-04-13 00:30:54-05	1b1762a49e1b9454bc3f81616f44794aabbf54f1	\N
727	\N	4f9e2e01e8b05e6a	pattern	match: func_8003AB44 via CU split (code6cac_c_ab.c)	\N	2026-04-12 22:00:53-05	9cb130a8ef17086b4be5f3bf1c704b72f6abc493	\N
728	\N	77292a6b24318da4	pattern	match: func_8008AAD4 (main.c) â€” 26 substs + 15 deletes + multi-line insert	\N	2026-04-12 19:40:28-05	0ff43ab1f1f027bf1bce2dac58a5512afddccec6	\N
729	\N	37f73423ddedaafb	pattern	tabled: single_game_SetWazaData â€” permuter plateaued at 1705	\N	2026-04-12 18:23:15-05	5da4c6f3c130343142481240bf4a4ef39fcf003f	\N
730	\N	cb26b18e901c0e24	pattern	match: SetCurrentCursor (code6cac_c2.c) â€” LICM unhoist breakthrough	\N	2026-04-12 16:21:37-05	e59fe99a629f52d5bbe2a63733c0e8435c053d86	\N
731	\N	8346028211ab8443	pattern	match: AddTbpOfst (main.c) via permuter + 3-rule regfix	\N	2026-04-12 15:52:19-05	8f37e12d4d7b139b6514e3b3262bcb537d1adb06	\N
732	\N	60871b8b91ce192e	pattern	match: mottest_disp (code6cac_b.c)	\N	2026-04-12 03:08:28-05	35ac5cf0b6640ee49c9b0682517c7d30679c9871	\N
733	\N	bf01640c6eea6a23	pattern	match: rob_life_ctrl_2 via text1a.c split + rodata split + regfix	\N	2026-04-12 02:17:38-05	435cad7cd6eeeb582331e941a2f85780569a1785	\N
734	\N	19494fd771e3df55	pattern	match: SetBloodSpot (main.c) + main.c rodata split	\N	2026-04-12 01:34:47-05	fe9fd80f6eb8f16db695f19a44f38557c42704bf	\N
735	\N	dc9fbd11a8664d9e	pattern	match: saTan1SyuryoDraw + func_80089178 + func_800891B4 (main.c)	\N	2026-04-12 00:35:07-05	88bc4f0d1c94f3e177ea68786f9e924820887047	\N
736	\N	c6776851681a4c4d	pattern	decomp: func_80041688 in text1a.c	\N	2026-04-12 00:02:52-05	a52de9db801dbd13fa76137d9cdc344ac307f078	\N
737	\N	9bfd4dc0c13e172b	pattern	match: coli_check_circle_hit_line (code6cac_b.c)	\N	2026-04-11 23:32:57-05	e3ae627703e97ca0399bc6c99cdd2da2b3a7a89a	\N
738	\N	88f5fe2e76b94c69	pattern	match: rob_calc_2d_position (text1a.c)	\N	2026-04-11 22:56:53-05	f574640e49f96cab30bbd6444f115fd2aaf250f9	\N
739	\N	516c4cbaf1f23265	pattern	decomp: match func_800430E4 in text1a.c (90 insns)	\N	2026-04-11 22:44:28-05	4ad38e89566603794afa8f875fd350b5beb2ef3b	\N
740	\N	d731aedbfd8b4f59	pattern	decomp: match func_80041988 (text1a.c) + partial func_80085FB8 (main.c)	\N	2026-04-11 22:10:59-05	bdb9fbda5b81b59d594ef2ca86c12547f07e06d2	\N
741	\N	f5af41ad1cd411e3	pattern	decomp: Wave 18 â€” match func_800300B4 (code6cac_b.c)	\N	2026-04-11 20:50:25-05	97d1bca1af590f7761bc096d90a9764eac138a45	\N
742	\N	e4d20ca4323cfd04	pattern	decomp: Wave 17 â€” match func_80019310, saTan0Init + regfix.py insert_after	\N	2026-04-11 18:18:12-05	233f6e7cf6db9aa08819841c03a8f8ab7a07aed3	\N
743	\N	4a4b34f66db0b901	pattern	decomp: Wave 16 â€” match func_800203B4, rob_life_ctrl, func_80047BE0	\N	2026-04-11 16:53:56-05	83dc0e5dcc7baf101cee910be2058541cc2f33cb	\N
744	\N	532de36e7bed1446	pattern	decomp: Wave 15 â€” match func_8001DA8C, myRobGeneiDraw2, md_game_check_change_sub_mode	\N	2026-04-11 15:41:04-05	76464f74355322339c2975a90bb75e4b41ab815f	\N
745	\N	cb0f80327e5b2907	pattern	decomp: match md_game_check_change_sub_mode in code6cac_c2.c (91 insns)	\N	2026-04-11 14:55:12-05	e1b7e416a409c21d7ed3899cf7bfda8ca906c6e4	\N
746	\N	0142bf14acec3212	pattern	decomp: match func_8001979C (Agent 37, salvaged) + Wave 15 rodata scaffolding	\N	2026-04-11 14:33:28-05	5c7cd4afdd30e0cb9bb0170b2fae41bbf5b076c1	\N
747	\N	43664523bbe82bc6	pattern	decomp: match func_80030BA8 in code6cac_b.c (96 insns)	\N	2026-04-11 13:56:03-05	33108d58fb25a1621bbe66ae9d47f6b228713931	\N
748	\N	3a77f319eb9a0e9f	pattern	decomp: match func_80042FA0 in src/text1a.c	\N	2026-04-11 13:51:14-05	3c4acedfdb27d896fa6e816ec22c4246f547403c	\N
749	\N	2880dcb6c8154541	pattern	decomp: match func_800466C0 (Agent 35, text1a switch + rodata split)	\N	2026-04-11 10:46:17-05	8cedcff489db89e50ea6afadf015555761f6ffc9	\N
750	\N	e2087aa6223fbf39	pattern	decomp: match cpu_check_tubazeri in code6cac_b.c (Agent 36, inline asm + GTE)	\N	2026-04-11 10:27:11-05	16b9cedbaa2c11e59dbb42d75071786a2c2f05b8	\N
751	\N	cca818bb18b16d4f	pattern	decomp: match func_80089E30 in src/main.c (goto + regfix)	\N	2026-04-11 10:14:16-05	d8fb69a406bc75bd4cc7cb8300c3812e73bf59d9	\N
752	\N	cadc7ed6dcaf2496	pattern	decomp: match videoDecCreate (func_800432A0) in text1a.c (Agent 33, goto + regfix)	\N	2026-04-11 09:47:05-05	fff3d3f5a567c2ad5f092de00b6b785105d1b8dc	\N
753	\N	08d98ddff26f5eb8	pattern	decomp: match cpu_check_tubazeri_2 in code6cac_b.c (Agent 32, goto + regfix)	\N	2026-04-11 09:43:43-05	dd47bd2efa493e5945f11b8f9573c9c265589abd	\N
754	\N	28625d1efac5404d	pattern	decomp: match saEft03Start2 in src/main.c	\N	2026-04-11 09:36:57-05	9a21c9e716c7253ef3d436ec2e7888fdbaec16a6	\N
755	\N	7b2f380ab1a89ebf	pattern	decomp: match func_800453E0 in src/text1a.c	\N	2026-04-11 03:29:16-05	9aa5e43f7c9213d305b58670d6074a635eb770cb	\N
756	\N	605420b47f1f9fe0	pattern	decomp: match func_8002C0DC in src/code6cac_b.c	\N	2026-04-11 03:17:37-05	9a141f78abd4ded7dcfd499713117654e4c7e540	\N
757	\N	196963aa7e74451d	pattern	match: title_mv_exec2 â€” decompile + regfix (18 rules)	\N	2026-04-11 01:14:25-05	6993b00f2f02d8f29420de420dfc6ea0775498e7	\N
758	\N	7c7fdaf8a22032bb	pattern	decomp: match func_80030208 (ang_near_dif, code6cac_b.c, 80 insns)	\N	2026-04-11 00:53:58-05	faddd527c373abb08ad053010a60707e7b0cdbfd	\N
759	\N	0a9d0801ca3c0870	pattern	decomp: match hirahira_w_frie in src/text1a.c	\N	2026-04-11 00:46:32-05	4c4e0737f56a3b5bc004d80cf66c98e34747c62f	\N
760	\N	e95a18a379625c2b	pattern	decomp: match calc_fc_frame in src/text1a.c	\N	2026-04-10 23:25:59-05	5a4f91c4022751fceca9a2251738f07d65358b63	\N
761	\N	034d72bf1d674ab4	pattern	match: func_800278C0 (camera_SetMatrix) in code6cac_b.c	\N	2026-04-10 22:51:48-05	690016a70fb63c72abd32b260355a9afa2f7c946	\N
762	\N	8458c3d13b33dc6d	pattern	match: func_8003A450 (ang_near_dif) â€” vsync polling with timer	\N	2026-04-10 22:50:41-05	d9cb9231d7f454fb61526a53c98cfb6e4c4f0ec3	\N
763	\N	581b1635a95a5084	pattern	match: decompile func_80034F88 (code6cac_b.c)	\N	2026-04-10 22:14:04-05	a748987a2db3a326be3443b5072faeac1ce4afdd	\N
764	\N	39fe7bcd898c8d0a	pattern	decomp: match func_80044170 in text1a.c	\N	2026-04-10 21:56:52-05	e8a0b19c51380d363f61dbfd28d6a2f1678d2586	\N
765	\N	a89778dc55de098d	pattern	decomp: match func_80087CAC (3 functions) in src/main.c	\N	2026-04-10 21:54:29-05	e7fda08b7cf2445ef8193f5790db5334ee9ee5f2	\N
766	\N	e77c22d7b57a4c4f	pattern	match: saTan4GaugeInit decompiled and matched	\N	2026-04-10 21:20:02-05	af903d9550dee92bc814bab44a563da6d8a89b15	\N
767	\N	84cc65b95b2741af	pattern	decomp: match tslLineG5Init in src/code6cac_c2.c	\N	2026-04-10 20:56:15-05	5f95cbb2b72fd437eacff671bdd787c9a1559e07	\N
768	\N	4125d22524e897f1	pattern	decomp: match func_80040304 in text1a.c (switch + rodata split)	\N	2026-04-10 20:44:38-05	98434be7156e7ec474608f907da216819f21ab3c	\N
769	\N	ce4a1f44e2d77644	pattern	readability: apply 66 Kengo function renames at asm level with linker aliases	\N	2026-04-10 20:24:21-05	bce02c53a0a730d15b5591d7389e3a18b62c7034	\N
770	\N	938b1d561ff833c8	pattern	decomp: match pad_ClearAppliBuffer in src/code6cac_c.c	\N	2026-04-10 19:58:29-05	8c13c613d908ac3a8d9157358695b028a674e542	\N
771	\N	0e5f8363a82dc5d0	pattern	decomp: match md_game_check_change_main_mode_katinuki in src/main.c	\N	2026-04-10 19:01:33-05	f8c04ad489338bb73784cf9fd05a6b9bd9fdb9a0	\N
772	\N	3042aec3acee753b	pattern	decomp: match func_80044B30 in src/text1a.c	\N	2026-04-10 18:58:24-05	a486817dd20d729829a2d6cc9c87a3e0bae0c6cc	\N
773	\N	258b38d8b818c016	pattern	fix: remove obsolete regfix prologue reorders after prologue_fix li/lui matching	\N	2026-04-10 15:15:51-05	2bd97ca0b9cb6415fb2d73a9772a0a23c1947e81	\N
774	\N	08d455e84c55cbaa	pattern	match: decompile damage_DebugDisp (79 insns, 3 phases)	\N	2026-04-10 14:28:04-05	ce5dbae1e362b83dc1430c2de3185393afbc80e6	\N
775	\N	1791fe882106ca69	pattern	readability: 30 config.c/system.c function renames with regfix	\N	2026-04-10 14:06:08-05	2971df5ec9a232d68ae6154564efe2b833a1ee94	\N
776	\N	99b8cb5b1f410101	pattern	decomp: match motion_Open + motion_Close (ings2.c, ~60 insns, 2 sub-functions)	\N	2026-04-10 13:26:53-05	3010a0c0ee8f71881e24c3400e1d9c3a10cc0ab1	\N
777	\N	b5b0db3c8125cad2	pattern	decomp: match se_data_set (code6cac.c, ~98 insns)	\N	2026-04-10 13:10:12-05	4598a7cef47fd1994e954fdb8cd9e289f902ffc0	\N
778	\N	7d59aed286b79e3f	pattern	readability: 76 function renames across gpu.c, display.c, main.c	\N	2026-04-10 13:10:07-05	eef127ee2f69a9a9e2e2f680daa55ec0bc0ff5fc	\N
779	\N	ebd9139fc2cbd027	pattern	fix: use sys_VSync name instead of func_800828CC in saEft00Add	\N	2026-04-10 12:44:31-05	b8e42965b0426a13316a6888f20baf4e5eef85ab	\N
780	\N	ae46a92023153ca3	pattern	decomp: match saEft00Add + saEft00Add_sub (system.c, 133+39 insns)	\N	2026-04-10 12:19:20-05	624ff03e6d836cc8a68338b5453064fe7510e098	\N
781	\N	91a5ca1cb96f461f	pattern	readability: add named constants for PS1 hardware and game magic numbers	\N	2026-04-10 12:04:08-05	775dc68a508214bc34479e0a4f1131e948841e4f	\N
782	\N	a03fef582e8a8f51	pattern	decomp: match func_800477E8 (sound.c, ~181 insns)	\N	2026-04-10 11:44:51-05	803b2ee5f71a61046ab7f550b020e6f39bd4440b	\N
783	\N	ab229592198279cc	pattern	readability: rename 29 func_ functions to semantic names	\N	2026-04-10 11:44:04-05	4c7390cc3bb3cde05085266d3a9724c8b9f51284	\N
784	\N	d0fe787fba1a09d7	pattern	decomp: match saEft01Init (system.c, 91 insns) + comprehensive regfix	\N	2026-04-10 02:24:27-05	be225276a6d62601764ff790bd35a81ae4d0b8ca	\N
785	\N	9ad307cdc9f37b15	pattern	decomp: match func_80036FD4 (code6cac_b2.c, 85 insns) + regfix reorder/subst	\N	2026-04-10 01:54:07-05	2ad31afd2fa82302fe9f9fa28a8c5414f4829831	\N
786	\N	e38b1ab7657cd1d5	pattern	refactor: create subsystem headers to deduplicate extern declarations	\N	2026-04-10 01:36:41-05	f75aea0976c898b152a4e6c1415676d13f718c2a	\N
787	\N	d6b28e54f5725ee6	pattern	decomp: match func_800832A0 (ings2.c, 70 insns) - 3-function multi-stub	\N	2026-04-10 01:31:12-05	acd12b83eff458ebb54407c69f77648cba65e018	\N
788	\N	2a4611c0ef826e47	pattern	refactor: rename D_ symbols in ings.c, ings2.c, system.c	\N	2026-04-10 01:04:47-05	7e182728ed69b5f120b4f020f9088806ae1538d1	\N
789	\N	836161fa21b1087d	pattern	decomp: match func_80080258 (system.c, 85 insns) + regfix swap/reorder	\N	2026-04-10 00:41:57-05	7e97e1f7557a0e1430b0267054fd63a5ac9cc74e	\N
790	\N	f5660c9b26dcc4fb	pattern	decomp: match tslPolyF4Init (system.c, 81 insns) + regfix subst	\N	2026-04-10 00:29:03-05	62d7e1cd1e14b8f35e48d65cd44dffa1bbda3f88	\N
791	\N	90830624dad88ebc	pattern	refactor: rename D_XXXXXXXX symbols to semantic names in C source	\N	2026-04-10 00:28:39-05	91c26f44c4d4f10836e1e3db4ef1c0980dc8b47b	\N
792	\N	a6881832e3853594	pattern	decomp: match motion_make_table + func_80082C58 (ings2.c, 66 insns)	\N	2026-04-09 23:56:33-05	cf0c687e1823c649e3a638b428d7779dca219a88	\N
793	\N	583d01eb9366ed60	pattern	decomp: match func_80044CCC (text1a.c, 70 insns) + 4-way register rotation regfix	\N	2026-04-09 19:35:43-05	ee094ca4d793fedbc3a56b3ef00af0a2c2cd9c92	\N
794	\N	dbd2241d6008f862	pattern	decomp: match func_8003AFFC (code6cac_c.c, 68 insns) + range-restricted regfix swap	\N	2026-04-09 19:05:32-05	82f0d451762adfe9a12f1f6eb594660026aca910	\N
795	\N	90781da37ad6cee4	pattern	decomp: match func_8003B10C (code6cac_c.c, 64 insns) + improve regfix insert/reorder	\N	2026-04-09 18:40:07-05	0d195fbf0ab03303948d5042411d985ff77ec106	\N
796	\N	c87bddffbd59d93a	pattern	decomp: match func_80021280 (code6cac.c, 74 insns)	\N	2026-04-09 17:22:30-05	d5f76a53aef4f90b91730df26ce34b6b85d3902e	\N
797	\N	b73642a07060d0d1	pattern	decomp: match func_80017FA0 + func_8001E6E4 (code6cac.c) + add regfix delete support	\N	2026-04-09 15:48:45-05	1e9d4ffc537e653c09ec50abc2b99e7dc9eeada1	\N
798	\N	61cddf38d70305cd	pattern	decomp: match func_80086BFC (main.c, 63 insns) + add regfix subst support	\N	2026-04-09 12:13:21-05	3e81d115e1a435f1e9f4665cb4c59c40fa920003	\N
799	\N	9a4a29dd28fb3e8a	pattern	decomp: match func_80037F40 (code6cac_c.c, 51 insns) + struct copy technique	\N	2026-04-09 11:41:07-05	344d71cd5bef1bce8d2a5d4940dce3a441781fae	\N
800	\N	5fe76dd5d38f137f	pattern	decomp: match func_800828CC (ings2.c, ~45 insns) + docs improvements	\N	2026-04-09 10:35:09-05	d7c91eba8e50df188b12c74c2b8e4bb91ece9725	\N
801	\N	72cac0c555ef7157	pattern	decomp: match func_8003F274 (config.c, 45 insns) + regfix for first-loop regs	\N	2026-04-09 00:15:09-05	634a8a15c46e272217ed33d6705e81bb37dbad54	\N
802	\N	4f01aaa8fc5eafed	pattern	decomp: match func_8001EEB4 (code6cac.c, 59 insns) + add regfix insert support	\N	2026-04-08 20:12:58-05	1a15ae8cd3c4e4bbfc448ded1048e708db6dbcd0	\N
803	\N	f91528feb964dee3	pattern	decomp: match func_8001BCF0 (code6cac.c, 74 insns)	\N	2026-04-08 17:54:05-05	1f27990c4f8e7b6063c028f31a02eaa4fe91310c	\N
804	\N	553394e4c1e141e9	pattern	decomp: match func_8003D9A0 (hirahira_w_frie, 64 insns) + add frame_fix tool	\N	2026-04-08 11:54:17-05	2ab0b3357cf632889fe93c8aabaeeb01315f63db	\N
805	\N	d44367f7e93eb0a6	pattern	decomp: TABLE func_80034F88 (-4 bytes, alternating pointer register allocation)	\N	2026-04-08 11:13:03-05	6c243337365c5c0cb287616b1ff86f400b56c70d	\N
806	\N	e9bbd8c95031fddd	pattern	decomp: TABLE func_80036FD4 (score 185, register allocation mismatch)	\N	2026-04-08 10:51:03-05	ef379369fb37fe78996aad30261acf8cf49eed39	\N
807	\N	739c9a1c7a7ed7da	pattern	tabled: func_800321E8 + cpu_check_tubazeri_2, fix func_8005344C decl	\N	2026-04-08 10:07:42-05	5d512cbeecb67f00945a2df3161ddbc5a80125c8	\N
808	\N	45f967e313ff07ef	pattern	tabled: cpu_check_tubazeri_2 (code6cac_b.c, -4 bytes)	\N	2026-04-08 02:25:12-05	d25b6d1f637f979a2108380c3965c8e197f3a4fd	\N
809	\N	d8e0744bce71c67b	pattern	decomp: TABLE func_80033BC0 (code6cac_b.c, score 235, 94/94 insns)	\N	2026-04-08 02:05:24-05	5319ee73c6307c173e7230b0b3c7f73d91924949	\N
810	\N	d1d2da8036a45739	pattern	decomp: match cpu_get_dist_2 + TABLE func_800278C0 (code6cac_b.c, 68 insns)	\N	2026-04-08 01:38:59-05	228857b09275ac756f54037c1d8ca5ca0dc7a7af	\N
811	\N	b7d425b213cce03b	pattern	decomp: match func_8002738C (code6cac_b.c, 50 insns)	\N	2026-04-08 01:05:43-05	9b9e31ae54e3feceebeb895464a7520f0e02e275	\N
812	\N	c786d01e6c686d70	pattern	decomp: match func_800340A0 (code6cac_b.c, 88 insns)	\N	2026-04-07 23:36:55-05	e3da9452210ff689f05ef0555980761fc8c79018	\N
813	\N	7a47c90037e64047	pattern	decomp: match func_80032064 (code6cac_b.c, 97 insns)	\N	2026-04-07 23:05:36-05	9791cb1b75215772abbbceca5227f3752917f0f4	\N
814	\N	d7fe09de231b3f09	pattern	decomp: match func_80030900 (code6cac_b.c, 75 insns)	\N	2026-04-07 22:17:00-05	965a9d24695a1fdb4d31fd6eae1c68ff6f788ec7	\N
815	\N	e4c42a94f2f206bb	pattern	decomp: match cpu_set_move_command_and_dir (code6cac_b.c, 57 insns)	\N	2026-04-07 18:43:52-05	03a8127903d650cd6156a2dfe042ad56da09ac2d	\N
816	\N	f0140424fc65801c	pattern	decomp: match gnd_disp_loop_ctrl (ings.c, 140 insns)	\N	2026-04-07 13:51:18-05	50ee8db86f733c8161bb404329f4308496d49a28	\N
817	\N	61eb2e2f156ec5b2	pattern	decomp: match cpu_get_dist + add reorder support to regfix.py	\N	2026-04-07 12:52:34-05	fb578a987cb930760f4c4a966bb993b48f0d7e62	\N
818	\N	cd53dd0aab53a75e	pattern	decomp: add regfix.py + match func_800806A4 & PutShadowRmd	\N	2026-04-07 12:42:00-05	7640e8f9385f8f89f49b13525148d43e065c404f	\N
819	\N	1b4a9bf20cbea2dd	pattern	decomp: match func_80082A14 + add delay_slot_ra tool	\N	2026-04-07 01:16:07-05	3fdcf5bd6d4e3c016750e96ebfecaa5774e4f57c	\N
820	\N	36c9654a17043605	pattern	decomp: match special_camera_get_rot_dir (code6cac_b2.c, 72 insns)	\N	2026-04-06 19:27:54-05	a6b005d2b37def6d85a4cc5f4866f55d0e66084d	\N
821	\N	7444742f77bb481b	pattern	decomp: match func_80046BF4 (stage/camera init, 112 insns)	\N	2026-04-06 15:03:45-05	d8724bee045889c26acd18de18a7c37dc92ae484	\N
822	\N	ed0967956806deb8	pattern	expand prologue_fix.py: handle interleaved init insns + match func_80080390	\N	2026-04-06 00:58:29-05	7504c2fe750a7eec57ac964eca4915d58098457d	\N
823	\N	234410d15a419014	pattern	match cpu_check_same_dir_timer: =r/0 asm trick for jal delay-slot fill + asm volatile barrier for branch polarity	\N	2026-04-05 17:00:43-05	34401c645dbe97158ce08b6aae8d71c64d1df9a7	\N
824	\N	278db72669be3af0	pattern	match func_8002872C: 17-way comparison chain with register asm pins + lifetime extension	\N	2026-04-05 15:49:34-05	1d16c0d35b68b93f06a23b424d1d429b8c784a0a	\N
825	\N	fde6f55f9dffc98b	pattern	match func_80083A48: SPU register init + voice/control copy + table clear	\N	2026-04-05 06:20:23-05	d8519f52f31bfbf7e9cf0291dda7db8193cb8c72	\N
826	\N	34c0c1a60478b3b1	pattern	match func_800475A4: camera bone setup with trig lookup + transform chain	\N	2026-04-05 03:03:36-05	41e9b9f14335471fdbcfb40c975575f2a235faa6	\N
827	\N	5ad22f9183eeef0e	pattern	match func_80034200: volatile pad + count-dependent a0 zero for blez delay slot	\N	2026-04-05 00:15:20-05	4febd3a4c44ad46565d4d6d8c77cc1e2721fbf41	\N
828	\N	3429d076f8be121f	pattern	match func_8003553C: register asm v1/v0 load order + inline asm + a0/a1 reg pins + pre-increment trick	\N	2026-04-04 23:55:43-05	a6e768625ae3d0dd2aa8856e124739ff48898db3	\N
829	\N	b245107b59cfba36	pattern	match func_80033D38: register asm pins + volatile asm for sll/addu encoding + memory barrier for t0 scheduling	\N	2026-04-04 23:40:33-05	e3990182333310a5f333821f8a27ae661e138547	\N
830	\N	90b1799beba71e71	pattern	match replay_camera_Init: register asm pins + inline lh + volatile barriers	\N	2026-04-04 22:33:45-05	94b779b0faf591cd95f947913209f60accada94a	\N
831	\N	24bd5512de9140f0	pattern	match func_8003880C: switch(func_80038734()-4) with 12 explicit cases + rodata split for jtbl_80010AC4	\N	2026-04-04 22:25:31-05	425e3bf2b9c12dba0f1d10618a14157181109370	\N
832	\N	efa11f8f83a265bd	pattern	match func_80030B10: register asm pins a0/a1/a2/v0/v1 + inline addu for pointer arithmetic	\N	2026-04-04 21:59:38-05	175272e89846e0e40a45a04598aa0f70f7e752aa	\N
833	\N	0d1516a6b8ccd83e	pattern	match func_8003F6D8: volatile spill + unpinned fp + register asm pins	\N	2026-04-04 20:17:24-05	729fc3f5fbac347522632ec85c4e14c1cb29bbbc	\N
834	\N	de02008bcd694af5	pattern	match func_80047A90: goto loops prevent LICM hoisting of div-by-10 magic constant	\N	2026-04-04 16:24:45-05	e43ae7f8ceecf44101eb7978a5d777cf8450f55c	\N
835	\N	76f756f16cf9627c	pattern	match func_800470B0: s16 array struct + register asm pins + volatile frame pad	\N	2026-04-04 04:36:15-05	47cfe7c9a633f0ca0a1bf3e030f0d0fcce2a9f1e	\N
836	\N	74747edd51afc1f7	pattern	match func_80037A20: opacify s1 before increment to block constant propagation	\N	2026-04-04 03:36:21-05	7fe6faa415465db21e2368c0779132d72ded3056	\N
837	\N	c21cf61669e57ed1	pattern	match func_800871D4, func_8004473C: register asm + frame padding	\N	2026-04-04 03:11:01-05	d6c87d304c1e3a59f0bb289c4c97f355ac03cc09	\N
838	\N	eea32d6c370cee8c	pattern	match func_80017D84: move+sh in asm body to fix instruction ordering	\N	2026-04-04 01:49:29-05	527a66e2845b84257b43f385061acf1d9d5e7fd8	\N
839	\N	1f5408db120ef3e1	pattern	split rodata + match func_80027438, func_80033498 (switch tables)	\N	2026-04-01 21:59:33-05	ad5a08b96a1189bf729df48689d9b9b572550484	\N
840	\N	23ab1c570fda7418	pattern	match 3 functions: func_80040CB8, func_80089EB0, func_8008ACD0	\N	2026-04-01 21:04:42-05	e450e069724f5602721bbde750d041b1d67a5c39	\N
841	\N	0e809ac941942cef	pattern	match func_8003D39C: GPU sprite OT primitive setup	\N	2026-04-01 15:39:50-05	37e3850ba69982f25ee42871613c9ec3b7e8d179	\N
842	\N	47b87058a2be7a87	pattern	match func_8003D330: GPU primitive OT link (addPrim pattern)	\N	2026-04-01 01:29:47-05	4fcdb544e345484a1e0775029e32a71197650fc6	\N
843	\N	ca8840c865454dfd	pattern	match 6 functions via register asm technique	\N	2026-04-01 00:15:51-05	1f5ea4c9dac96a7239fb4e95d3473f4369cc2c01	\N
844	\N	ae6695ea3b094fcf	pattern	Name 14 data globals from Kengo symbol cross-reference	\N	2026-03-30 23:56:47-05	0ee0e5060a9a3f9b87e7ac36ed0e900998734a73	\N
845	\N	3fe21548693b20e9	pattern	Apply 2 final Kengo renames: marionation_Exec, replay_camera_rob_back_loose2	\N	2026-03-30 22:57:50-05	257c65fb6dfdd18d92c84af7aeb785e85e157c07	\N
846	\N	ea35b16e85d1f2de	pattern	Apply calc_loc_mat_fw rename + expand se_fc affinity for code6cac_b.c	\N	2026-03-30 22:50:02-05	90787370b835405a3496fd13785ab6b1d1047cd7	\N
847	\N	65e96eb157050fe3	pattern	Apply 19 additional Kengo renames: Â±10% near-misses + motion functions	\N	2026-03-30 22:45:59-05	0457e27f80bc4906578cd2f5a904ed51fff45949	\N
848	\N	69af2088d03d965e	pattern	Apply 13 Kengo near-miss function renames (Â±5% size tolerance)	\N	2026-03-30 22:33:23-05	d01e87091c0616210b0a981a72da1b6417616125	\N
849	\N	995c8e9097e07a0a	pattern	Apply 38 Kengo affinity-batch function renames atomically across all files	\N	2026-03-30 22:07:45-05	c402f4577b40e5f919d04c4616e707cc7cf98560	\N
850	\N	036089b399b6ab77	pattern	Revert 9 low-confidence PS2 UI renames; add kengo confidence comments	\N	2026-03-30 21:10:09-05	dfedbe7f7e39ee5dc1c6b06592b41f9c50033626	\N
851	\N	32f5586804dab2b7	pattern	Apply 38 Kengo size-match renames atomically across all files	\N	2026-03-30 20:54:35-05	295a7a259bc985d40b3cd7e507884589fa745df7	\N
852	\N	d0c5c90ac30e274c	pattern	Apply 110 Kengo-derived function renames atomically across all files	\N	2026-03-30 19:38:23-05	a65aa16a196f4f74cc3ed0baa11a934e0cf06c08	\N
853	\N	aaaba55e3a2c7aca	pattern	main.c: rename 21 functions using Kengo debug symbol cross-reference	\N	2026-03-30 00:32:12-05	b1adb2544035219dfecedb227b5f2ccab21e83ec	\N
854	\N	06829b84746feb15	pattern	code6cac_c.c: decompile func_800395B4 (sound slot allocator)	\N	2026-03-29 23:05:21-05	b8ac81a3ea0b9ade500674d4a744f4b3931e1fae	\N
855	\N	b5c26ce1cc4e4fe3	pattern	code6cac.c: decompile func_800206B0 (vertex scale with fixed-point multiply)	\N	2026-03-28 23:30:01-05	bc6c2974b581329a85a95e7a92bda266881d8dca	\N
856	\N	0878c72c3f295534	pattern	Revert "Merge code6cac agent: decompile func_800206B0"	\N	2026-03-28 23:27:08-05	f35bb184d89a3be7060728a842d1afaba142a0e1	\N
857	\N	fd02cb3619dc4f0a	pattern	code6cac_c.c: decompile func_8003984C (3D collision box check)	\N	2026-03-28 18:34:48-05	865e89c78a1a002b0c650ae423a8444448f127ff	\N
858	\N	98f2e30d99b5e381	pattern	code6cac.c: decompile func_800206B0	\N	2026-03-28 18:34:27-05	e1551ee9b840c1f7fef9faf970bdf12a661b5f8a	\N
859	\N	20a53a1d60a412bb	pattern	Per-function --expand-lb + decompile func_800343F0 + stub classifier	\N	2026-03-28 18:11:47-05	38101f3138a235f8fa04a2c318bf223ec9c85478	\N
860	\N	b4a41dc64c27cd1c	pattern	text1a.c: decompile func_80044504 (stage init with scratchpad setup)	\N	2026-03-28 16:06:54-05	fe91375e96a72ffeb1c5c9837cbc4a82cd299dab	\N
861	\N	7269ef0cbbb8241c	pattern	text1a.c: decompile func_8004659C (pointer table load + dispatch)	\N	2026-03-28 16:01:47-05	2ac7faf5cd479ea1c9e2bf9b347b4e20083718ac	\N
862	\N	82c299664981ca7b	pattern	main.c: decompile func_80087DA0	\N	2026-03-28 15:44:09-05	53521d49653682f2b9daa7c3134567fda73f5866	\N
863	\N	06e1097c64f92d1d	pattern	maspsx: split --expand-lb/--expand-lh + decompile func_8003047C	\N	2026-03-28 13:30:43-05	b7f7059522d481718ea020c036aa3054e39da8b6	\N
864	\N	329d77a9172b7354	pattern	Apply maspsx li fix + decompile func_80033550	\N	2026-03-27 22:47:48-05	eee69e8ed6439a56c4d9e2a8b9e9a20c0cf1902a	\N
865	\N	046477b0ad49d48f	pattern	code6cac_b.c: decompile func_8002EECC (3x3 matrix inverse) + add triage tools	\N	2026-03-27 02:34:14-05	2eaeba95ae1ae7c20d56821daeff2583fb64341d	\N
866	\N	2ed74a47f1a8874a	pattern	sound.c: decompile func_80047384 (3D sound angle calculator)	\N	2026-03-27 00:34:57-05	1c62b5880a313d2af48175b263fca071c3fa6d9c	\N
867	\N	0e8441811746a98f	pattern	config.c: decompile func_8003FECC (config data loader)	\N	2026-03-27 00:17:38-05	410c5105560af49958066d3f0b01baed9ae6898e	\N
868	\N	703991bebfed5584	pattern	Revert "Merge Wave 14 agent-a3a61072: config.c all 5 functions decompiled"	\N	2026-03-26 19:30:31-05	fca3870c04556108c12090246f06bc037d69ff32	\N
869	\N	295ba868d13b1083	pattern	config.c: decompile all 5 stubs (func_8003F274, func_8003F6D8, func_8003F824, func_8003FA24, func_8003FECC)	\N	2026-03-26 18:58:37-05	714f3667db7a0fdd3d9fcbfe9be90699ca9736f9	\N
870	\N	61b8f0772f23704e	pattern	code6cac_c2.c: decompile func_8003C42C (pad input vote counter)	\N	2026-03-26 11:40:57-05	751025d4fc4c2316548f75905242729f809e586a	\N
871	\N	ef02a4b42c8878f1	pattern	main.c: decompile func_8008908C	\N	2026-03-26 11:40:44-05	c6d988bd46ac15e8359b924889dafbd48b10cc9e	\N
872	\N	ff9b1f609ef23f4a	pattern	code6cac_c.c: decompile func_80038658	\N	2026-03-26 11:39:03-05	030fe23ac06e67f53f1722beb03ab3e0825a2c32	\N
873	\N	80e4c89f2d59b4bd	pattern	code6cac_c.c: decompile func_80037B00	\N	2026-03-26 11:25:45-05	75fbab8c08551b8acd997b4a839459b2e9cee337	\N
874	\N	37eb778513a26324	pattern	code6cac_c.c: decompile func_80037AA4	\N	2026-03-26 11:23:18-05	2e6cf867090b363d4dadc9a46eb313cbb4bf26b6	\N
875	\N	91188f2c14470bc9	pattern	text1a.c: decompile func_800433E4	\N	2026-03-26 11:21:42-05	8c85b2d44828d5bf9decd508ab53b16b21d3a910	\N
876	\N	e17479903fb80f7c	pattern	code6cac.c: decompile func_8001CD68 (time decomposition)	\N	2026-03-26 11:21:13-05	9eec01da5ba3d366cab9336e4a822703b935361d	\N
877	\N	dd71ff7674c83ec3	pattern	code6cac.c: decompile func_800224E0	\N	2026-03-26 10:44:27-05	4300eb43a906550bf81cb80c07263284672a2ace	\N
878	\N	9c9fe7643e314102	pattern	text1a.c: decompile func_800417D0	\N	2026-03-26 10:41:14-05	54fb4a4348a11f3443ce73d38fa98eddfcf2a77c	\N
879	\N	704077ef2290ad48	pattern	text1a.c: decompile func_80044FA0	\N	2026-03-26 10:37:52-05	00c8e5a5143eb783e8208045a8493741c1b2c22b	\N
880	\N	af444b9a483823fa	pattern	text1a.c: decompile func_800464C4	\N	2026-03-26 10:36:52-05	965d056f25ee8a165b65e05de25201048763dd95	\N
881	\N	bf0a63d4219f2928	pattern	text1a.c: decompile func_80044010 and func_800455AC	\N	2026-03-26 04:16:35-05	a5423f386e84f9720d662d069cda2a9859670614	\N
882	\N	065796832bdcec9a	pattern	main.c: decompile func_80085448, func_80085EE4, func_80085F88	\N	2026-03-26 04:12:22-05	9c48ab4f18ecfa630ee529932724f1c41e4446c1	\N
883	\N	47fba1243f6268d3	pattern	sound.c: decompile func_80047210; system.c: decompile func_80081E1C/F0C/F1C	\N	2026-03-26 04:00:49-05	014898077924994596c2ae11e60d3c4c79645abc	\N
884	\N	60fa5847004a35ee	pattern	code6cac_c2.c: decompile func_8003D7B4 (delta animation decompression)	\N	2026-03-26 03:25:20-05	cfbe4af8361214f1fc77a4081ff12fb68d16e4de	\N
885	\N	edeb1ad2e54abaf7	pattern	code6cac_c.c: decompile func_80037B90 and func_80037C34	\N	2026-03-26 03:07:46-05	5f1f3f03f31c2d9343f2e3c61205aa288a576e7a	\N
886	\N	5f229cb5dba56845	pattern	code6cac.c: decompile func_8001FAE4 (linked-list walker)	\N	2026-03-26 03:04:31-05	923b14f46ddab34db29995a94ae909adade52b86	\N
887	\N	311f4f0a602bbb88	pattern	text1a.c: decompile 3 functions (func_80044098, func_80041E10, func_80041398)	\N	2026-03-26 03:03:31-05	ce6459ee9c5ddcbf3075092fb1f6e080d289dcd4	\N
888	\N	090185cc4451c90b	pattern	Decompile 3 functions from m2c catalog (score 0-2)	\N	2026-03-26 02:04:50-05	5036ebcf4bb27540a15add5a3d31c8431cf912eb	\N
889	\N	c52bf761e09d8ab0	pattern	text1a.c: decompile 4 functions (func_80045600, func_80045510, func_80044670, func_80045AA4)	\N	2026-03-26 02:01:15-05	103575a338a9cd0341e0cad0fd1f0d81c5ad8650	\N
890	\N	1b59c6a25de0aba8	pattern	code6cac_c.c: decompile func_8003B20C and func_8003B3A4	\N	2026-03-26 02:00:49-05	5a4dba615782a3158b33de5e56eb2fd091f23802	\N
891	\N	be036ce1496a6d52	pattern	main.c: decompile func_800899A8	\N	2026-03-26 01:47:11-05	f9d6c8aadc13cd8007acfa9d81eb16a030ba2dd3	\N
892	\N	03cf339a7e42c579	pattern	main.c: decompile func_800848AC	\N	2026-03-26 01:15:24-05	96e163a590a9c2502a04e6198284fec6fbb39cae	\N
893	\N	b9fa8d93210434ce	pattern	system.c: decompile func_800819C4 (CD subsystem init)	\N	2026-03-26 01:14:37-05	766ee01acecb94b1840f514c8eec74babecb7b5a	\N
894	\N	38e879daebbcdbd9	pattern	code6cac_c.c: decompile func_8003B328 and func_80039680	\N	2026-03-26 01:08:22-05	cd2a2093afc50fc757691b04d75029208d3deb52	\N
895	\N	9307bb8d9ef070db	pattern	text1a.c: decompile func_80046048 + fix D_800963EC type conflict	\N	2026-03-26 01:07:34-05	fc10a0917257639f85345c7e54ddad705ccc25a0	\N
896	\N	2c3a7d15a6d08022	pattern	code6cac.c: decompile 4 functions (func_8001C820, func_80023E40, func_8001E878, func_8001BC70)	\N	2026-03-26 01:06:51-05	6d255e7920bcbfc2101229bc3d5d932897b7a5a2	\N
897	\N	9b1071658aa422a5	pattern	code6cac_c2.c: decompile func_8003BEA8 and func_8003C2C0	\N	2026-03-26 01:02:47-05	7dcd2f54fc4d173153378a1a03916963a110aa1c	\N
898	\N	bdf7962c861a0f4c	pattern	sound.c: decompile func_800467B8	\N	2026-03-26 01:02:19-05	ad8db9787ae103d645f929d820da97264fdd10b8	\N
899	\N	8efe42830223f257	pattern	config.c: decompile func_8003FE40 (palette/index data processing)	\N	2026-03-26 00:23:22-05	9f7e3a277bcaabc7cb3a0cb7ef6687c680e731fe	\N
900	\N	0d43e2676f0c659d	pattern	main.c: decompile 5 functions (func_80085210, func_8008C184, func_8008BEA4, func_80085114, func_80085E4C)	\N	2026-03-25 23:51:52-05	e8fc5f34c9a1c089732d47a47adafeca96340578	\N
901	\N	03bf9fca5ef61b52	pattern	text1a.c: decompile func_800450F4 and func_80044100	\N	2026-03-25 23:26:04-05	7be02d0f13dc13d1e12b90c1ad1d8d6bd0fd7f1f	\N
902	\N	e19c06984857098b	pattern	sound.c: decompile func_800472C0	\N	2026-03-25 23:02:49-05	b05c4314ba541c9a781962d8ae39e2af4f015aa0	\N
903	\N	51e86228460524e7	pattern	code6cac.c: decompile 4 recovered functions	\N	2026-03-25 22:53:53-05	45b7ae9ebf6015d505df6481a1f10118379c0b1e	\N
904	\N	d286ce71939a9223	pattern	code6cac_c2.c: decompile func_8003CE18 (fight result screen init)	\N	2026-03-25 22:33:15-05	65a646fa1c308aff62143bb4c10fb5eabe19c1c2	\N
905	\N	e15d04dd4fba7079	pattern	sound.c: decompile func_80046B44	\N	2026-03-25 22:31:32-05	0128117723b10d6f1f89e3bff0c8bb3da66aadb7	\N
906	\N	7394404c692fa7f6	pattern	text1a.c: fix corrupted file, maintain 3 matched functions	\N	2026-03-25 21:55:54-05	6229ba7979a7dfd79f0ec8a061abd1d8deea1b35	\N
907	\N	1664d7654504439f	pattern	text1a.c: decompile 3 functions (func_800414FC, func_80040400, func_80041430)	\N	2026-03-25 21:52:57-05	5a3e9706cbd114dd89ae6e972654df36ea62c64e	\N
908	\N	37c215c7c37fe1d9	pattern	code6cac.c: decompile 21 functions	\N	2026-03-25 21:48:21-05	8100a99b6373df0e0f7b6a8511a0222b12bb340c	\N
909	\N	0b5cf0106921ca9e	pattern	Revert "sound.c: decompile func_80046B44 (sound init)"	\N	2026-03-25 20:44:46-05	4b289fec62872ce58b727cff6d675ab03dbc1fb7	\N
910	\N	6e41ce65a7ec55f6	pattern	sound.c: decompile func_80046B44 (sound init)	\N	2026-03-25 20:44:34-05	e66e9e68ae7b167f743f378894edfc42ab24bd74	\N
911	\N	0efcb679f9c52714	pattern	Revert "Merge sound.c: decompile func_80046B44"	\N	2026-03-25 20:43:59-05	acf0bfefd796283c411c2b5b505c75111fbd6e32	\N
912	\N	885a3da5858ec99b	pattern	gpu.c: decompile func_8007AE7C (GPU config initialization)	\N	2026-03-25 20:41:51-05	626a9a65fb838ec3ba07bf8bf643f08a34942935	\N
913	\N	466fceb3b7500b49	pattern	code6cac_c.c: decompile func_800388A8	\N	2026-03-25 20:37:40-05	3053129f4220cd6642acf131bd30874cf93d1f70	\N
914	\N	4497f590591eeb3e	pattern	code6cac_c.c: decompile 4 functions	\N	2026-03-25 20:33:50-05	762a7215955c34c063d6fb6ecdf3291b9a5f1906	\N
915	\N	0b45d5a6e8d2caa8	pattern	main.c: decompile 3 functions (func_8008B400, func_800892F8, func_80089384)	\N	2026-03-25 20:30:57-05	b46c91ebc14d788a91a22f2fb8a45ba9bed19698	\N
917	\N	352cae0942f15ad5	pattern	code6cac_b2.c: decompile func_80037110	\N	2026-03-25 20:22:30-05	a6aeecd0687876460ae2cf3a707783efb6fea2aa	\N
918	\N	384de2663d563c0a	pattern	code6cac_c2.c: decompile 3 functions (func_8003CD10, func_8003E164, func_8003E22C)	\N	2026-03-25 20:22:03-05	62f2d2fd6d193b4ee71899ab7bce5725e8bf0439	\N
919	\N	6bab834e788c62c9	pattern	system.c: decompile func_800827D0 (CD read polling loop)	\N	2026-03-25 20:21:28-05	d30ba70c955a66237032271411edd9fdf385d36e	\N
920	\N	15f69d51c11d3e40	pattern	code6cac_b.c: decompile func_8002798C	\N	2026-03-25 19:14:01-05	e9ad19b1fbd5b96d524e54aba8a39e13f214d802	\N
921	\N	58484a0a8c6d4707	pattern	text1a.c: decompile func_80040A78 and func_8004574C	\N	2026-03-25 19:10:57-05	9d5c1a249efdb24dfdff1d01d456c56e0d043373	\N
922	\N	7aed270115c65ac2	pattern	main.c: decompile 3 functions via permuter refinement	\N	2026-03-25 18:59:28-05	be829a4597b8e6e318d5ee1f50fea4c3a9a213d6	\N
923	\N	1ce0ab562e604a8d	pattern	Decompile 5 functions in code6cac_c.c	\N	2026-03-25 18:59:20-05	5a79b7e58080b80e413df9be79cd9d9789bc642f	\N
924	\N	3ec0b76d8bded45f	pattern	gpu.c: convert func_8007A28C and func_8007A458 to inline asm	\N	2026-03-25 15:26:48-05	7472435b87131fbeb5602b97b4048a822491e45e	\N
925	\N	900b8f26750b2c1d	pattern	Revert "Merge main.c: 2 functions decompiled via permuter"	\N	2026-03-25 15:23:16-05	feb034906b66420c6e4e8b1f8616c5bb5479f27d	\N
926	\N	b772d2bee2bd714b	pattern	Decompile func_80089384 and func_8008BD88 in main.c	\N	2026-03-25 15:17:53-05	0369c7560be189404dea5d973bea31d67b24c5a6	\N
927	\N	0cd6bdc71db8e9a5	pattern	Decompile 5 functions in code6cac.c (65â†’60 INCLUDE_ASM stubs)	\N	2026-03-25 15:03:01-05	32b54159d432e268296cb63b9e79206a5e24109e	\N
928	\N	7e0f3e872ba6bfc9	pattern	text1b.c: convert all 206 remaining INCLUDE_ASM stubs to inline __asm__	\N	2026-03-25 14:50:44-05	cceedd8a2017bc7e929d3f2d86e32abafac77baf	\N
929	\N	f16e0091e16f182a	pattern	text1b.c: convert 28 handwritten asm functions to inline __asm__	\N	2026-03-25 14:48:49-05	8f5b36b436cb98d30a0803209498e0799e1c03f8	\N
930	\N	fd35524fdb186d8c	pattern	Decompile 4 functions in code6cac_c.c (42 -> 38 stubs)	\N	2026-03-25 14:41:04-05	145a978f80ddbd3e60c4cdc798e439d7753f2910	\N
931	\N	1d9b4e0600baa6b5	pattern	Decompile func_800400F8 in config.c	\N	2026-03-25 14:02:53-05	7f57042d96b9d5eac76bd5106f7c51d0a9db8b62	\N
932	\N	a71bbfdae9a88531	pattern	Decompile func_80083220 (setjmp/longjmp) in ings2.c	\N	2026-03-25 13:55:29-05	a89cea4c08dc4473a550fbcbef4d9143363215fa	\N
933	\N	9bc48b9e914a2c92	pattern	Decompile all 26 INCLUDE_ASM stubs in display.c	\N	2026-03-25 13:53:13-05	b44bc594b7747f14831281ca4d33b24fba15549a	\N
934	\N	834346b4025ac7c8	pattern	Decompile func_80047D94 in sound.c	\N	2026-03-25 13:50:26-05	17b66efa9221d8869b59c9dc44d8797098f4cca0	\N
935	\N	aba22799a704dce3	pattern	Decompile func_80035480 and func_80036064 in code6cac_b2.c	\N	2026-03-25 13:48:40-05	c58dba1220b90e3a722851133b435cefe981f2b1	\N
936	\N	2bd5040535f8f7b7	pattern	Decompile func_80037540 in code6cac_b2.c	\N	2026-03-25 03:59:30-05	1d7533c0ffb32b6918e39abc16c86c4499e971cb	\N
937	\N	1a264595a7f09006	pattern	Restore decompiled functions lost in accidental revert	\N	2026-03-25 03:56:26-05	068a28ec98afbfe335d42f2b592d4a927f7443d1	\N
938	\N	035aea9f35980692	pattern	Revert func_800400F8 decompilation in config.c	\N	2026-03-25 03:54:24-05	b72b8f8b8ac4264d9241bb63ebc4dd49c389d458	\N
939	\N	e0b03da75616f618	pattern	Decompile func_8008926C in main.c	\N	2026-03-25 03:51:16-05	100741ad9ad9270aef720dc7991467318674fa41	\N
940	\N	2075ae0ca8562566	pattern	Decompile 11 functions in text1b.c	\N	2026-03-25 03:50:38-05	ed08151842ecb6dd65f0fccdee1dbc6ab698e915	\N
941	\N	db374a0000c16b54	pattern	Decompile func_800375EC (code6cac_c.c) and func_80033FE4 (code6cac_b.c)	\N	2026-03-25 03:43:19-05	c8a74ecf966aa985d602d67876b6de7f1955c7de	\N
943	\N	d171c98bd47fcf27	pattern	Decompile 2 functions in ings.c	\N	2026-03-25 03:10:48-05	43bbe5635df279eb7b333d99ed8d448cfb7d6a4f	\N
944	\N	18f5dee88efce842	pattern	Decompile func_800884C4 in main.c	\N	2026-03-25 01:32:07-05	ec0bbd8e7e7d5351b8f7e6666ab39bedad2c4a4b	\N
945	\N	301e3616ad91189c	pattern	Decompile 7 functions in text1a.c	\N	2026-03-25 01:17:28-05	66d6b076958d7d08e0381cea66da13dcffa30518	\N
946	\N	62a4e44f9635dd59	pattern	Add extern declarations for code6cac_c2.c decompilation prep	\N	2026-03-25 01:17:17-05	8dff7eac045043cd3d84df9b61011cd39b2a364c	\N
947	\N	4d1ff092b185b20c	pattern	Decompile func_800342A0 in code6cac_b.c (goto workaround)	\N	2026-03-25 01:15:24-05	138b3b200f708e1c72245f8711bbd9aa428bc78f	\N
949	\N	f45fe129138c796f	pattern	Convert 19 INCLUDE_ASM stubs to __asm__ blocks in display.c	\N	2026-03-25 01:07:19-05	d140fe489bf7377576436b60d74b8e26c0f952ad	\N
950	\N	b8a3878bbabd9ce2	pattern	Decompile 16 functions in code6cac_c.c	\N	2026-03-25 01:06:08-05	b1e0f141700549665c7f1bd0e32f924544d33bc2	\N
951	\N	d0fa62cb0a14e7f8	pattern	Decompile 4 functions in code6cac.c	\N	2026-03-25 01:04:24-05	783d4d21a3b10a2e7bc94608e35f723fa34bea51	\N
952	\N	66d4d0301027830b	pattern	Decompile func_8008AE7C in main.c (permuter zero-score match)	\N	2026-03-25 00:41:08-05	1260708be5961baad11d899e63dd0e6d6301fc4d	\N
953	\N	6889d5a7c307ae83	pattern	Decompile 4 functions in sound.c and config.c	\N	2026-03-25 00:10:52-05	514b3cd6603b45a05348111a1ff28980652ab251	\N
954	\N	1c8e72057d91b8c2	pattern	Convert 64 GTE functions in text1b.c to __asm__ blocks	\N	2026-03-25 00:08:48-05	d2df9a15cd93bcb22f87ede3a5d0b7113c7761bf	\N
955	\N	233ed3317b639e53	pattern	Decompile 6 functions in code6cac_b2/c2	\N	2026-03-25 00:00:19-05	f5a387772aeb3359e28f9c574af27c8e42a12a30	\N
956	\N	72dabba590c5fba5	pattern	Decompile func_800469C4 in sound.c (sound channel setup)	\N	2026-03-24 22:27:55-05	587dc76acc10e25c809fffb074b92e00f306490e	\N
957	\N	d3bd9096afd05874	pattern	Decompile 27 functions in text1b.c (leaf + medium-tier)	\N	2026-03-24 22:27:20-05	6139f28051be2e9b523403fb8517e2fb4b63d73c	\N
958	\N	140f22c301dda8a9	pattern	Decompile 3 functions in code6cac/code6cac_c2	\N	2026-03-24 22:26:23-05	46bdd8603b3e44fb23d3b19dc3b686f9c4d58e57	\N
959	\N	d4a313a596566b82	pattern	Decompile 6 GTE/COP2 functions in display.c	\N	2026-03-24 22:22:56-05	856ce63afc8881da141cdd7b76696c04b01e0979	\N
960	\N	816763a336a84b26	pattern	Decompile 8 functions in text1a.c (high-score candidates)	\N	2026-03-24 22:16:06-05	a2c795061f8be8d49be7e21ae44e216621504e0f	\N
961	\N	73984405019e4172	pattern	Convert BIOS trampoline func_8008D060 in main.c to __asm__ block	\N	2026-03-24 21:37:36-05	c3b98c095ff88f6e0b358d5aa0969fb7fb1307f3	\N
962	\N	fdd7c6b13026da89	pattern	Convert BIOS trampoline func_80078FF0 in text1b.c to __asm__ block	\N	2026-03-24 21:37:14-05	2fc632a669ab09603a03acd70ae7c1f0d40f10ea	\N
963	\N	a357fc39f21dcdec	pattern	Decompile 2 functions in main.c + fix type conflicts	\N	2026-03-24 20:58:55-05	7d58c18ad7514b9c13854f7c2fdc8e420913902d	\N
964	\N	2fa9a0b7d1f988e8	pattern	Decompile 1 function in text1a.c (non-GP easy tier)	\N	2026-03-24 20:54:46-05	97b92793742db92a515cb0a41387f0dc201da721	\N
965	\N	c0e7e3437f694fdc	pattern	Convert func_8007DF10 BIOS trampoline in display.c to __asm__ block	\N	2026-03-24 20:54:10-05	408feae72766d70d8530c94841b949dc700adf59	\N
966	\N	bdeec5bbc411b493	pattern	Decompile func_8007B2A0 in display.c (display mode setter)	\N	2026-03-24 20:49:17-05	8311ef6f3e6e01ed5c0b2a6eb8850338f3207a37	\N
967	\N	751c47b75e716d1b	pattern	Further split code6cac_b/c and fix extern conflicts	\N	2026-03-24 20:37:20-05	97eb39533534e666b483db29c14bcdd708e9f968	\N
968	\N	cacdfaf83ef9d862	pattern	Decompile 5 functions in sound.c (GP-rel + non-GP)	\N	2026-03-24 20:17:32-05	71306da32fdb9c9abbbb25823f9c6dcb221efd84	\N
969	\N	12adddbae349b64e	pattern	Decompile 6 functions in config.c (5 GP-rel + 1 non-GP)	\N	2026-03-24 20:13:24-05	f4faba4de7840c3e6e8e04926066108e191e446d	\N
970	\N	aa08cd5c819c7893	pattern	Decompile 8 functions in ings.c (GP-rel + non-GP)	\N	2026-03-24 20:12:59-05	7ba202e388fffbe783ab06c86bdd7769769e6094	\N
971	\N	15a65ae351c9c3b8	pattern	Decompile 3 functions in code6cac.c (range check, entity lookup, animation)	\N	2026-03-24 20:08:21-05	97131ec30885c3411117e45748104de1a7ae967e	\N
972	\N	22ab4515eabead7d	pattern	Split code6cac.c into 3 files to work around GCC 2.7.2 __asm__ limit	\N	2026-03-24 20:03:33-05	88c6a6f1766532ed2e7bc875e6c85ddfc8074367	\N
973	\N	f31f87b511fc545a	pattern	Convert 21 BIOS trampolines in text1b.c to __asm__ blocks	\N	2026-03-24 20:02:20-05	8c511f59b4695cd9a9076b504cde036f93e387d4	\N
974	\N	a3753d52e1424b13	pattern	Fix code6cac.c and text1b.c after botched merge/revert	\N	2026-03-24 19:42:36-05	99a09c450dc4e2b44b4df7ce64a2a2e08bf777b5	\N
975	\N	73a9a239bcfa13a0	pattern	Revert "Merge code6cac.c: decompile 4 functions (range check, entity lookup, array slot, animation)"	\N	2026-03-24 19:39:40-05	1c4d7e68910b51b1f23ab7528abc5ce895e8181b	\N
976	\N	b052ad7f6514e766	pattern	Decompile 3 BIOS trampolines in main.c + add data symbol addresses	\N	2026-03-24 19:20:29-05	694572bf9d254ae92840f6faedda2ab621ede5dc	\N
977	\N	b1131ae2897b5037	pattern	Decompile 2 functions in text1a.c (non-GP bitfield packers)	\N	2026-03-24 19:18:28-05	2c838595eb4e57d31603cb80316632fc608f5612	\N
978	\N	cf85169af7e51108	pattern	Decompile func_80052C10 in text1b.c (scratchpad RAM clear)	\N	2026-03-24 19:18:04-05	0ac00b0fb1ea035df5ea5780f7eb5fd742e8b868	\N
979	\N	af01e4946d2a0d30	pattern	Decompile 4 functions in code6cac.c (non-GP tiny+small+medium tier)	\N	2026-03-24 19:16:54-05	764cb45e45dcc23bd7442f47a66efba9d167b3dc	\N
980	\N	406bcca552d13a5f	pattern	Decompile 12 functions in gpu.c (GPU primitive/display interface)	\N	2026-03-24 19:14:14-05	821cd6ba9e89b652ca0432c91822f5247201198e	\N
981	\N	461fb07472be21a4	pattern	Decompile 4 functions in display.c (GTE, trig tables, GPU linked list)	\N	2026-03-24 19:12:51-05	0b6f4ec38e4cdffb013c6fde9b94dee27097bf76	\N
982	\N	3aa79255cb5c5e28	pattern	Decompile 2 functions in system.c (BIOS trampoline + BCD conversion)	\N	2026-03-24 19:09:26-05	a2ac1b2aad1883e9df08ea672ecdb59d84c79eb9	\N
983	\N	48a62719eab85f12	pattern	Decompile 2 functions in ings2.c (interrupt/IO handlers)	\N	2026-03-24 19:07:25-05	b7505b318dbd94cf9cdc417dd1a65f64c7840596	\N
984	\N	d48eac1a026543f9	pattern	Fix binary mismatch: system.c nop + text1a.c decompile/revert pass	\N	2026-03-24 17:41:56-05	afb1d4289e2309530c0f459c1688298b99893f71	\N
985	\N	cd8bf2f4887477c0	pattern	Fix system.c to achieve byte-identical match	\N	2026-03-24 16:44:40-05	638e417c03af0f69c6fc460c6de3a76d77f33fd3	\N
986	\N	e17061e5648eea91	pattern	Decompile 10 functions in system.c (system hardware interface)	\N	2026-03-24 16:27:40-05	317042e3f77e168163dd2ff21e79d2865e417e55	\N
987	\N	a10758ecb5e24f9d	pattern	Decompile func_8006E950 in text1b.c	\N	2026-03-24 12:27:47-05	68c265d14f1b0cf237f9e6068fe985bf0cae795d	\N
988	\N	e15a94c65810b9a9	pattern	Decompile func_8006E480 in text1b.c	\N	2026-03-24 12:12:30-05	a736e78de82504a3109a461c3b85b64631ac53c1	\N
989	\N	e23c9e724bce44a1	pattern	Decompile 11 GTE functions in display.c using inline asm	\N	2026-03-24 11:41:50-05	f011bb92722be5a340f5bd50ae04855e4b9da332	\N
990	\N	39db102d641382d1	pattern	Decompile func_8003B56C in code6cac.c	\N	2026-03-24 11:36:02-05	1f72e29da1c3fb0d643d5912869c5f88d78638f1	\N
991	\N	73c77e13bfc4c03b	pattern	Decompile 7 BIOS trampolines in gpu.c (A0/B0 vector calls)	\N	2026-03-24 11:35:37-05	052464907047e6202dd1ee8bf0e45353dd1c2640	\N
992	\N	172039b8a9c95ef2	pattern	Decompile 10 functions in ings2.c (BIOS syscalls and file I/O stubs)	\N	2026-03-24 11:24:32-05	2db3c3a5087b2ed9c2520169781e3a5e3167eaa7	\N
993	\N	7ecda3abd6964f19	pattern	Decompile func_80043278 in text1a.c	\N	2026-03-24 11:20:16-05	7d2b9ae0a173f1b9409cee81a06e8f951b310197	\N
994	\N	7e0f25f985d2ca60	pattern	Decompile 2 more functions in ings.c (round 8)	\N	2026-03-24 07:56:10-05	e74d56cde169c6848e78b81b6723eb3e65492264	\N
995	\N	30b64178c7b2b452	pattern	Decompile 4 functions, add 120+ data symbols and extern declarations	\N	2026-03-24 07:55:11-05	5b621868fdb5ab3c0f09c7c6624db35c6da2dc13	\N
996	\N	0acba138f49b2406	pattern	Decompile 6 more functions (round 7)	\N	2026-03-24 07:24:33-05	f158f77d8b741498fea7ff9adf7f407d899f9d34	\N
997	\N	fedb82e101b4034c	pattern	Decompile 25 more functions (round 6, large tier)	\N	2026-03-24 06:39:56-05	cf2e9ca3dc9ab945e16db689624afc44b0cec000	\N
998	\N	b9c6216dcf3758bf	pattern	Decompile 11 more functions (round 5)	\N	2026-03-24 05:59:07-05	cde89307ac2fe44a6a1b1dce3d520610ecad7fd6	\N
999	\N	b8bc5ba5d72cfd86	pattern	Decompile 30 more functions (round 4, medium+large tier)	\N	2026-03-24 05:28:34-05	f7b445f40697e48626be78b515d27b347420a9a7	\N
1000	\N	3acf7dfb8e0520f4	pattern	Decompile 36 more functions across all files (round 3, medium+large tier)	\N	2026-03-24 04:55:07-05	5814cfbeae06b17000affb85206ef40ce2d77921	\N
1001	\N	7266b2d594e5a434	pattern	Decompile 39 more functions across text1b, code6cac, main, system (round 2)	\N	2026-03-24 04:13:40-05	413053800ea16e74cdfc62ca407837f1338ae116	\N
1002	\N	82cd0f11fcc9c00c	pattern	Decompile 19 functions in text1a.c and main.c (tiny+small tier)	\N	2026-03-24 03:45:23-05	7d1713e0319c42dd806a7c9566fc7e32cfd7047c	\N
1003	\N	887e54cfb31cd8dd	pattern	Decompile 62 functions in text1b.c (409 INCLUDE_ASM stubs remaining)	\N	2026-03-24 03:39:44-05	1b241dc05b0dc30ba0eb111647dd1a9fe111c270	\N
1004	\N	d6eb9c5c82145e8d	pattern	Decompile 12 functions in display.c (text2 segment)	\N	2026-03-24 03:38:37-05	5b8d6aadb4aa3fd1effb476e22f5b5566e08404f	\N
1005	\N	b688a56a379f3f83	pattern	Decompile 28 functions in code6cac.c (6CAC segment)	\N	2026-03-24 03:35:31-05	3939f0defda26666aa24c60bfab87264b378cd45	\N
1006	\N	33c560507abf28fb	pattern	Scaffold text1b (471 funcs) and 6CAC (349 funcs) segments to C files	\N	2026-03-24 03:16:06-05	621f738e7ad07585a0ffb6312ad3111433c459d0	\N
1007	\N	ae29445d2facd73a	pattern	Link display.c (text2 segment) into build, replacing asm/text2.o	\N	2026-03-24 03:05:12-05	f9343686fd50436ae784fd451403be0f81d7aa4b	\N
1008	\N	bc42f40b33d974ee	pattern	Scaffold text1a+text3 segments to C, decompile 19 functions (196 total, 13.9%)	\N	2026-03-24 02:44:40-05	394c939e92bdb6c9a7f06d9200ae681ce80b6718	\N
1009	\N	ee1c76e1caf5697b	pattern	Decompile 4 functions in ings.c and ings2.c (medium-tier)	\N	2026-03-24 02:34:26-05	a78a93f1c8539be2ae253d4072233d6a38dbeb0c	\N
1010	\N	be8d283b9851decb	pattern	Decompile 31 functions across config, gpu, sound, and ings	\N	2026-03-24 00:52:50-05	882bdb5b48d15a7c87ad09d2c08b1832640af3cb	\N
1011	\N	3d713b95b62b1717	pattern	Migrate text4 segment to C, scaffold text2/display, add asm-differ	\N	2026-03-24 00:34:28-05	1e3e3a15597b5373993f17688a8dbc20c95b2250	\N
1012	\N	82032431c6647066	pattern	Add config.c: 6 new decompiled functions (112 total, 7.9%)	\N	2026-03-23 22:52:48-05	e579fa49d8509fcfc5dd22a6e267fc6652b4c877	\N
1013	\N	0dca4e1942253ede	pattern	Add sound.c: 33 new decompiled functions (106 total, 7.5%)	\N	2026-03-23 22:41:54-05	8879093e1c0b48666dcfc208fd4afffb802ef6f1	\N
1014	\N	3bb9cca68abaa31d	pattern	Initial commit: Bushido Blade 2 matching decompilation	\N	2026-03-23 22:09:57-05	913757ac5bbc6eedfd8526c7db986d08a301e56b	\N
\.


--
-- Name: attempt_logs_log_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.attempt_logs_log_id_seq', 16, true);


--
-- Name: attempts_attempt_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.attempts_attempt_id_seq', 16, true);


--
-- Name: engine_events_event_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.engine_events_event_id_seq', 594, true);


--
-- Name: experiments_experiment_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.experiments_experiment_id_seq', 105, true);


--
-- Name: techniques_technique_id_seq; Type: SEQUENCE SET; Schema: public; Owner: postgres
--

SELECT pg_catalog.setval('public.techniques_technique_id_seq', 1014, true);


--
-- Name: agent_runs agent_runs_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.agent_runs
    ADD CONSTRAINT agent_runs_pkey PRIMARY KEY (run_id);


--
-- Name: attempt_logs attempt_logs_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.attempt_logs
    ADD CONSTRAINT attempt_logs_pkey PRIMARY KEY (log_id);


--
-- Name: attempt_techniques attempt_techniques_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.attempt_techniques
    ADD CONSTRAINT attempt_techniques_pkey PRIMARY KEY (attempt_id, technique_id);


--
-- Name: attempts attempts_func_run_id_started_at_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.attempts
    ADD CONSTRAINT attempts_func_run_id_started_at_key UNIQUE (func, run_id, started_at);


--
-- Name: attempts attempts_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.attempts
    ADD CONSTRAINT attempts_pkey PRIMARY KEY (attempt_id);


--
-- Name: engine_events engine_events_line_hash_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.engine_events
    ADD CONSTRAINT engine_events_line_hash_key UNIQUE (line_hash);


--
-- Name: engine_events engine_events_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.engine_events
    ADD CONSTRAINT engine_events_pkey PRIMARY KEY (event_id);


--
-- Name: experiments experiments_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.experiments
    ADD CONSTRAINT experiments_pkey PRIMARY KEY (experiment_id);


--
-- Name: experiments experiments_run_key_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.experiments
    ADD CONSTRAINT experiments_run_key_key UNIQUE (run_key);


--
-- Name: sync_meta sync_meta_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.sync_meta
    ADD CONSTRAINT sync_meta_pkey PRIMARY KEY (key);


--
-- Name: techniques techniques_content_hash_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.techniques
    ADD CONSTRAINT techniques_content_hash_key UNIQUE (content_hash);


--
-- Name: techniques techniques_pkey; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.techniques
    ADD CONSTRAINT techniques_pkey PRIMARY KEY (technique_id);


--
-- Name: techniques techniques_slug_key; Type: CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.techniques
    ADD CONSTRAINT techniques_slug_key UNIQUE (slug);


--
-- Name: idx_attempts_func; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_attempts_func ON public.attempts USING btree (func);


--
-- Name: idx_attempts_model; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_attempts_model ON public.attempts USING btree (model);


--
-- Name: idx_attempts_outcome; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_attempts_outcome ON public.attempts USING btree (outcome);


--
-- Name: idx_events_cmd; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_events_cmd ON public.engine_events USING btree (command);


--
-- Name: idx_events_func; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_events_func ON public.engine_events USING btree (func);


--
-- Name: idx_events_ts; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_events_ts ON public.engine_events USING btree (ts);


--
-- Name: idx_exp_func; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_exp_func ON public.experiments USING btree (func);


--
-- Name: idx_exp_model; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_exp_model ON public.experiments USING btree (model, effort);


--
-- Name: idx_runs_model; Type: INDEX; Schema: public; Owner: postgres
--

CREATE INDEX idx_runs_model ON public.agent_runs USING btree (model);


--
-- Name: attempt_logs attempt_logs_attempt_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.attempt_logs
    ADD CONSTRAINT attempt_logs_attempt_id_fkey FOREIGN KEY (attempt_id) REFERENCES public.attempts(attempt_id) ON DELETE CASCADE;


--
-- Name: attempt_techniques attempt_techniques_attempt_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.attempt_techniques
    ADD CONSTRAINT attempt_techniques_attempt_id_fkey FOREIGN KEY (attempt_id) REFERENCES public.attempts(attempt_id) ON DELETE CASCADE;


--
-- Name: attempt_techniques attempt_techniques_technique_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.attempt_techniques
    ADD CONSTRAINT attempt_techniques_technique_id_fkey FOREIGN KEY (technique_id) REFERENCES public.techniques(technique_id) ON DELETE CASCADE;


--
-- Name: attempts attempts_run_id_fkey; Type: FK CONSTRAINT; Schema: public; Owner: postgres
--

ALTER TABLE ONLY public.attempts
    ADD CONSTRAINT attempts_run_id_fkey FOREIGN KEY (run_id) REFERENCES public.agent_runs(run_id) ON DELETE SET NULL;


--
-- PostgreSQL database dump complete
--

\unrestrict EGlKCkTiDtQ8XjRKDc2MiaEyVmmkIwTBscls7N0I7AwCY9hJLh5flmKSotDbPG6

