for d in $(find $1 -name *.cc); do
  . ./object-file.sh $d
done
