function find_source {
  find $1 -name *.h
  find $1 -name *.cc
}

case $1 in
  clean) redo build/clean ;;
  all) redo-ifchange build/arw ;;
  lint) ./vendor/cpplint.py $(find_source src) $(find_source test/cc) ;;
  test) redo  build/test ;;
  *) echo "No rule to build '$1'" >&2; exit 1 ;;
esac
