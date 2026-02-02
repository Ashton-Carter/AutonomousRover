#!/bin/bash
set -e

SESSION="rover"

ROOT="$HOME/Documents/RaspberryPi5"
PY_DIR="$ROOT/PythonVideo"
C_DIR="$ROOT"

PY="$PY_DIR/venv/bin/python"
C_BIN="$C_DIR/rovermake"

# If session exists, attach
if tmux has-session -t $SESSION 2>/dev/null; then
    echo "Session '$SESSION' already running."
    tmux attach -t $SESSION
    exit 0
fi

echo "Starting rover system..."

# Create session
tmux new-session -d -s $SESSION -c $ROOT

# ─────────────────────────────────────
# Pane 0: C driver (must start first)
# ─────────────────────────────────────
tmux send-keys -t $SESSION \
    "$C_BIN" C-m

# Give C time to create sockets
sleep 1

# ─────────────────────────────────────
# Pane 1: Python video capture
# ─────────────────────────────────────
tmux split-window -h -t $SESSION
tmux send-keys -t $SESSION \
    "cd $PY_DIR && python videoCapture.py" C-m

# ─────────────────────────────────────
# Pane 2: YOLO inference
# ─────────────────────────────────────
tmux split-window -v -t $SESSION:0.1
tmux send-keys -t $SESSION \
    "cd $PY_DIR && $PY videoClassification.py" C-m

# ─────────────────────────────────────
# Pane 3: system monitor
# ─────────────────────────────────────
tmux split-window -v -t $SESSION:0.0
tmux send-keys -t $SESSION "htop" C-m

# Layout
tmux select-layout -t $SESSION tiled

# Attach
tmux attach -t $SESSION
