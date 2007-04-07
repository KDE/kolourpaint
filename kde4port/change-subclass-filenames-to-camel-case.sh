# Changes e.g. kptoolrectangle.(h|cpp) to kpToolRectangle.(h|cpp) by looking
# inside the .h file for the CamelCase classname.
#
# Changes only those .h/.cpp pairs where the .h file contains a single
# subclass (must be derived from something).

HEADERS_WITH_ONE_SUBCLASS=$((for f in `find -name \*.h | egrep -v '[A-Z]'`; do echo -n $f: ; egrep '^class kp[^:]+ :[^:]+$' $f | wc -l; done) | fgrep  :1 | cut -d: -f1 )

echo $HEADERS_WITH_ONE_SUBCLASS

for f in $HEADERS_WITH_ONE_SUBCLASS
do
	# Look inside header file for classname e.g. "kpToolRectangle".
	CLASS_NAME=$(egrep '^class kp[^:]+ :[^:]+$' $f | sed -re 's/^class (kp[^:]+) :[^:]+$/\1/')

	# Look at existing filename e.g. "kptoolrectangle".
	FILE_CLASS_NAME=`basename $f | cut -d. -f1`

	if [ `echo $CLASS_NAME | tr A-Z a-z` != $FILE_CLASS_NAME ]
	then
		echo
		echo SKIPPING $FILE_CLASS_NAME $CLASS_NAME as classname does not match filename.
		echo
		continue
	fi

	H_FILE=$f
	C_FILE=`echo $f | sed -e 's/\.h$/.cpp/'`

	if [ ! -f $C_FILE ]
	then
		echo
		echo SKIPPING $FILE_CLASS_NAME $CLASS_NAME as .cpp does not exist.
		echo
		continue
	fi

	DIR_NAME=`dirname $f`

	echo $CLASS_NAME $FILE_CLASS_NAME $H_FILE $C_FILE
	svn move $H_FILE $DIR_NAME/${CLASS_NAME}.h
	svn move $C_FILE $DIR_NAME/${CLASS_NAME}.cpp
	echo
done
