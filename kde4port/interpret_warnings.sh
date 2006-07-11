#!/bin/sh

echo Given a file \'warnings\' consisting of gcc build warnings,
echo prints all the deprecated warning types in a nice format sorted by
echo frequency.  You can specify a filename as an argument else
echo it defaults to \'warnings\'.
echo Version 2006-05-04

GCC_WARNINGS="$1"
if [ ! "$GCC_WARNINGS" ]
then
	GCC_WARNINGS=warnings
fi

fgrep ': warning:' $GCC_WARNINGS  | cut -d: -f4- | sort | uniq -c | sort -n | sed -e 's/is deprecated //' -e 's/declared at //' | sed -re 's/:([0-9]+)\)$/ \1/' | sed -e 's/[‘’]//g'
