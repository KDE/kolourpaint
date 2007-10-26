#!/bin/bash

# Regenerates the "MimeType=" line in kolourpaint4.desktop.
#
# Read kolourpaint4.desktop for Read usage.
# Read kpDocumentSaveOptions.cpp for Write usage.

MODE="Read"
[ "$1" ] && MODE="Write"

echo Mode = $MODE > /dev/stderr

echo -n MimeType=
firstTime=1
for g in $(for f in `grep -l X-KDE-${MODE}=true *.desktop`; do echo `grep X-KDE-MimeType= $f`; done | cut -d= -f2 | sort | sed -e '/^$/d')
do
    echo -n $g
    echo -n ';'
done
echo