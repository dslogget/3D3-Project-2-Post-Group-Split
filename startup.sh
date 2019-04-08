#!/bin/bash
tmux new-session -d;

tmux splitw -h -p 66;
tmux splitw -h -p 50;
tmux splitw -v -p 50;
tmux selectp -L
tmux splitw -v -p 50;
tmux selectp -L
tmux splitw -v -p 50;

tmux selectp -U;

servers=("A" "B" "C" "D" "E" "F");
servers=( $(shuf -e "${servers[@]}") );
index=0
for s in "${servers[@]}"
do
	tmux selectp -t $index;
	index=$((index+1));
	tmux send-keys "./my-router " $s C-m;

done


tmux attach;
