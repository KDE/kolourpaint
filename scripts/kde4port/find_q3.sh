find -name \*.cpp -o -name \*.h|xargs egrep '[kKqQ]3[^ ]+' -o | cut -d: -f2- | sort | uniq -c | sort -n
