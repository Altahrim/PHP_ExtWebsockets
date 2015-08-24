#! /bin/sh
#
#   * Install Libwebsockets
#
# Authors:
#   * Timandes White <timands@gmail.com>
#

LIBWEBSOCKETS_VERSION=$1
LIBWEBSOCKETS_NAME=libwebsockets
PACKAGE_NAME=${LIBWEBSOCKETS_NAME}-${LIBWEBSOCKETS_VERSION}
LIBWEBSOCKETS_PREFIX=${HOME}/${PACKAGE_NAME}

if [ ! -e ${PACKAGE_NAME}.tar.gz ]; then
    wget -O ${PACKAGE_NAME}.tar.gz https://github.com/warmcat/${LIBWEBSOCKETS_NAME}/archive/v${LIBWEBSOCKETS_VERSION}.tar.gz || exit 1
fi
tar xvf ${PACKAGE_NAME}.tar.gz || exit 1

cd ${PACKAGE_NAME}
    mkdir build
    cd build
        cmake -DCMAKE_INSTALL_PREFIX:PATH=${LIBWEBSOCKETS_PREFIX} .. || exit 1
        make || exit 1
        make install || exit 1
    cd ..
cd ..

exit 0
