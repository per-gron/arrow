redo-ifchange compile

GTEST_DIR=../vendor/gtest
GTEST_FILE=$GTEST_DIR/src/gtest-all.cc
GTEST_OBJ=${3%.a}.o

CFLAGS="-I${GTEST_DIR}/include -I${GTEST_DIR} -DGTEST_USE_OWN_TR1_TUPLE=1" ./compile $GTEST_FILE ${GTEST_FILE%.cc} $GTEST_OBJ

ar -rv $3 $GTEST_OBJ
