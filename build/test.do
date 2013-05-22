redo-ifchange libarw.a

DIR=../test/cc
. ./object-files.sh $DIR | xargs redo-ifchange libgtest.a
. ./object-files.sh $DIR | xargs clang++ -stdlib=libc++ -o $3 libgtest.a libarw.a >&2
exec ./$3 >&2
