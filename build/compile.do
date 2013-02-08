redo-ifchange ../build/config.sh
. ../build/config.sh

echo "clang++ -I../src -I../vendor/utf8/source -I../vendor/gtest/include -DGTEST_USE_OWN_TR1_TUPLE=1 -std=c++11 -stdlib=libc++ -MD -MF \${3%.o}.d -c \$2.cc -o \$3 $CFLAGS \$CFLAGS" > $3
chmod a+x $3
