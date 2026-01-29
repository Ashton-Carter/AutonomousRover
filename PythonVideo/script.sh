#!/bin/bash

SESSION="rover"
PROJECT_DIR="$HOME/Documents/RaspberryPi5/PythonVideo"
PY="$PROJECT_DIR/venv/bin/python"

# If session already exists, just attach
tmux has-session -t $SESSION 2>/dev/null
if [ $? -eq 0 ]; then
    echo "Session '$SESSION' already running."
    tmux attach -t $SESSION
    exit 0
fi

# Create new session (detached)
tmux new-session -d -s $SESSION -c $PROJECT_DIR

# Pane 0: video capture
tmux send-keys -t $SESSION "python videoCapture.py" C-m

# Split right
tmux split-window -h -t $SESSION

# Pane 1: YOLO (low priority)
tmux send-keys -t $SESSION "$PY videoClassification.py" C-m

# Split bottom left
tmux split-window -v -t $SESSION:0.0

# Pane 2: system monitor
tmux send-keys -t $SESSION "htop" C-m

# Arrange nicely
tmux select-layout -t $SESSION tiled

# Attach
tmux attach -t $SESSION
