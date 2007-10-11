echo You must be in the KolourPaint source directory.

for f in `fgrep 2005 list.txt | cut -d' ' -f1`
do
	w=`echo $f | cut -c2-`
	v=$((w - 1))

	echo '******************' $f '********************';
	svn log -$f
	echo
	echo
	echo

	svn diff --diff-cmd=/usr/bin/diff -x -bBwupd -r$v:$w

	echo
	echo
	echo
done
