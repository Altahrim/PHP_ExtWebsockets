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

wget https://github.com/warmcat/${LIBWEBSOCKETS_NAME}/archive/${LIBWEBSOCKETS_VERSION}.tar.gz ${PACKAGE_NAME}.tar.gz || exit 1
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
