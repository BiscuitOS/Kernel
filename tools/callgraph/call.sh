#!/bin/bash

ROOT=$1
FUNC=$2
BROWSER="firefox"
#BROWSER="chromium-browser"


PWD=${ROOT}/tools/callgraph

${PWD}/callgraph -f ${FUNC} -d ${ROOT} -b ${BROWSER} -D 999
