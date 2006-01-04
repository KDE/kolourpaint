#!/bin/bash

# Clarence Dang
# 2005-02-03

# OBSOLETE - port to SVN, or use my "svn-commit-changes" script

# Note: Only accepts patches made with -p1 + description must be between "BEGIN" and "END" lines

if [ $# -ne 1 ]
then
    echo Syntax: $0 '<patch>'
    exit 1
fi


echo "$1"

patch -p1 < "$1"

# add dirs
for f in $(for f in $(egrep '^---.+1970' "$1"  | cut -d' ' -f2 | cut -d/ -f2- | cut -d"`echo -en "\x9"`" -f1 | grep /); do dirname $f; done | sort | uniq)
do
    cvs add $f
done
cvs add $(egrep '^\+\+\+' "$1"  | cut -d' ' -f2 | cut -d/ -f2- | cut -d"`echo -en "\x9"`" -f1)

files=$(egrep '^\+\+\+.+1970' "$1"  | cut -d' ' -f2 | cut -d/ -f2- | cut -d"`echo -en "\x9"`" -f1)
if [ "$files" ]
then
    # The patch should have removed it but just in case...
    rm $files

    cvs remove $files
fi

sed -ne '/BEGIN/,/END/p' "$1" | sed -e '/^BEGIN$/d' -e '/^END$/d' > ../apply_patch_temp
echo '~~~~'
cat ../apply_patch_temp
echo '~~~~'

echo 'OK? '
read

cvs commit -F ../apply_patch_temp
cvscheck

echo 'OK? '
read

rm ../apply_patch_temp
