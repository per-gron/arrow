redo-ifchange compile

DIR=../src
. ./object-files.sh $DIR | xargs redo-ifchange
. ./object-files.sh $DIR | xargs ar -rv $3
