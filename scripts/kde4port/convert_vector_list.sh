#!/bin/bash
for f in `find -name \*.h -o -name \*.cpp`
do
	echo $f
	sed -e 's/Q3ValueVector/QList/g' -e 's/q3valuevector/qlist/g'	\
		-e 's/Q3ValueList/QLinkedList/g' -e 's/q3valuelist/qlinkedlist/g' $f > k
	mv k $f
done
