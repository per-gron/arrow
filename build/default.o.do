redo-ifchange compile ../${2#out/}.cc
./compile $1 ../${2#out/} $3
read DEPS < ${3%.o}.d
redo-ifchange ${DEPS#*:}
