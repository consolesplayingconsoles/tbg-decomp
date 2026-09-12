set -e

mkdir -p build/tmp

# `rebuild` wipes build/output_test, so two concurrent runs delete each other's
# objects mid-build. Serialize instead of racing.
exec 9>build/.test.lock
flock 9

make -f Makefile.test rebuild

sh4objtest suite -s tests.php "$@"
