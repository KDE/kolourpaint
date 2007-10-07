#!/bin/bash

# Regenerates the "MimeType=" line in kolourpaint.desktop.
#
# Read kolourpaint.desktop for Read usage.
# Read kpdocumentsaveoptions.cpp for Write usage.

MODE="Read"
[ "$1" ] && MODE="Write"

echo Mode = $MODE > /dev/stderr

echo -n MimeType=
firstTime=1
for g in $(for f in `grep -l ${MODE}=true *.kimgio`; do echo `grep Mimetype= $f`; done | cut -d= -f2 | sort | sed -e '/^$/d')
do
    echo -n $g
    echo -n ';'
done
echo
