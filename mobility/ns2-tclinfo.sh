#!/bin/bash
# Author: Sérgio Vieira, sergio.vieira@ifce.edu.br - Instituto Federal de Educação, Ciência e Tecnologia do Ceará
echo -e "NS2 Mobility Trace Info - sergio.vieira@ifce.edu.br"
echo    "==================================================="
awk '$1 ~ /\$node_/ && !seen[$1]++ {count+=1} END{print "Nodes: " count}' $1
awk -F '[()]' '$1 ~ /\$node_/ { for (i=2; i<=NF; i+=2) max = ($i > max) ? $i : max } END { print "Last Node Id: " max }' $1
awk '$1 ~ /\$ns_/ NR==1 {start=$3} {end=$3} END { print "End time: " end}' $1