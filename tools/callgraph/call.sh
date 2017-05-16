#!/bin/bash

ROOT=$1
FUNC=$2
BROWSER="firefox"
#BROWSER="chromium-browser"


PWD=${ROOT}/tools/callgraph

${PWD}/callgraph -f ${FUNC} -d ${ROOT} -b ${BROWSER} -D 4
clear
figlet "BiscuitOS"
echo ""
echo -e "\e[1;31m Draw ${FUNC} call-graph on ${BROWSER} \e[0m"
echo ""
sleep 5
rm ${PWD}/*.svg

