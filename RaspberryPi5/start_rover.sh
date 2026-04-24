#!/bin/bash
set -e

SESSION="rover"

ROOT="$HOME/Documents/RaspberryPi5"
PY_DIR="$ROOT/PythonVideo"
C_DIR="$ROOT"

PY="$PY_DIR/venv/bin/python"
C_BIN="$C_DIR/rovermake"

if tmux has-session -t $SESSION 2>/dev/null; then
    echo "Session '$SESSION' already running."
    tmux attach -t $SESSION
    exit 0
fi

tmux new-session -d -s $SESSION -c $ROOT

tmux send-keys -t $SESSION \
    "$C_BIN" C-m

sleep 1

tmux split-window -h -t $SESSION
tmux send-keys -t $SESSION \
    "cd $PY_DIR && python videoCapture.py" C-m

tmux split-window -v -t $SESSION:0.1
tmux send-keys -t $SESSION \
    "cd $PY_DIR && $PY videoClassification.py" C-m

tmux split-window -v -t $SESSION:0.0
tmux send-keys -t $SESSION \
    "cd $PY_DIR && $PY externalConnection.py" C-m

tmux select-layout -t $SESSION tiled

tmux attach -t $SESSION
