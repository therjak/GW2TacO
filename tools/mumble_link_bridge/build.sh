#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT_DIR="${1:-$SCRIPT_DIR}"
OUTPUT_BIN="${OUTPUT_DIR}/mumble_link_bridge.exe"

echo "Building MumbleLink Bridge for Windows (amd64)..."
cd "$SCRIPT_DIR"

GOOS=windows GOARCH=amd64 go build -ldflags="-s -w" -o "$OUTPUT_BIN" .

echo "Successfully built: ${OUTPUT_BIN}"
ls -lh "$OUTPUT_BIN"
