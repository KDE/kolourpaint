find -name \*.cpp -o -name \*.h |xargs egrep '^#define *DEBUG.+ *1|\|\| *1'
# Use for ViM: \v^#define *DEBUG.+ *1|\|\| *1
