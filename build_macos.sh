#!/bin/sh
#
# Example on how to build Mini vMac on Macintosh
#

# we need to build the setup tool first
if [ ! -x ./setup_t ]; then
	gcc -o setup_t setup/tool.c
fi

# run setup tool to generate makefile generator
./setup_t -maintainer "egon.rath@gmail.com" \
        -homepage "https://github.com/egrath" \
        -n "minivmac-3.7-test" \
        -e xcd \
        -t mcar \
        -m SE \
        -d d \
        -log 1 \
        -dis 1 \
        > setup.sh

# generate makefile and build
. ./setup.sh
xcodebuild

