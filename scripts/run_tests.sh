set -e

make -f Makefile.test rebuild
mkdir -p build/tmp

sh4objtest suite -s tests.php "$@"
