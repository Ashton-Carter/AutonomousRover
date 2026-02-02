#!/bin/bash

SESSION="rover"

if tmux has-session -t $SESSION 2>/dev/null; then
    echo "Stopping rover..."
    tmux kill-session -t $SESSION
else
    echo "No rover session running."
fi
