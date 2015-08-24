#! /bin/sh
#
#   * Build PHP extension 'PHP_ExtWebsockets'
#   * Test it
#
# Authors:
#   * Timandes White <timands@gmail.com>
#

LIBWEBSOCKETS_VERSION=$1
LIBWEBSOCKETS_NAME=libwebsockets
PACKAGE_NAME=${LIBWEBSOCKETS_NAME}-${LIBWEBSOCKETS_VERSION}
LIBWEBSOCKETS_PREFIX=${HOME}/${PACKAGE_NAME}

cd src
    phpize || exit 1
    CFLAGS="--coverage -fprofile-arcs -ftest-coverage" \
        LDFLAGS="--coverage" \
        ./configure --with-libwebsockets-dir=${LIBWEBSOCKETS_PREFIX} || exit 1
    make || exit 1

    lcov --directory . --zerocounters &&
        lcov --directory . --capture --initial --output-file coverage.info
    export NO_INTERACTION=1 && make test
    lcov --no-checksum --directory . --capture --output-file coverage.info

    OUT_FILES=`find tests/ |grep "out$"`
    if [ "${OUT_FILES}" != "" ]; then
        for f in `find tests/ |grep "out$"`; do cmd="cat $f" && echo $cmd: && $cmd && echo; done
        exit 1
    fi
cd ..

exit 0
