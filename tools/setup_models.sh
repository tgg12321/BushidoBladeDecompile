#!/bin/bash
# Pull and create all BB2 decompilation models for Ollama.
# Run from project root: bash tools/setup_models.sh
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "=== BB2 Local Model Setup ==="
echo ""

# 1. Pull base models
echo "[1/4] Pulling base models (this may take a while)..."
echo ""

echo "--- DeepSeek Coder V2 16B (~9 GB) ---"
ollama pull deepseek-coder-v2:16b
echo ""

echo "--- Codestral 22B (~13 GB) ---"
ollama pull codestral:latest
echo ""

# 2. Create custom models with BB2 system prompt
echo "[2/4] Creating bb2-decomp (Qwen - should already exist)..."
ollama create bb2-decomp -f "$SCRIPT_DIR/bb2-decomp.Modelfile" 2>/dev/null || echo "  (already exists or base not pulled)"
echo ""

echo "[3/4] Creating bb2-deepseek..."
ollama create bb2-deepseek -f "$SCRIPT_DIR/bb2-deepseek.Modelfile"
echo ""

echo "[4/4] Creating bb2-codestral..."
ollama create bb2-codestral -f "$SCRIPT_DIR/bb2-codestral.Modelfile"
echo ""

# 3. Verify
echo "=== Installed models ==="
ollama list | grep -E "bb2-|deepseek-coder|codestral"
echo ""
echo "Done! Run A/B test with:"
echo "  python tools/batch_local_decomp.py --ab-test"
