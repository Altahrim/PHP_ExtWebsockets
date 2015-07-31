dnl $Id$
dnl config.m4 for extension WebSocket

dnl Enable WebSocket support
PHP_ARG_ENABLE(websocket, whether to enable websocket support,
[  --enable-websocket           Enable websocket support])

dnl Include libwebsockets library from custom directory
PHP_ARG_WITH([libwebsockets-dir], [],
[  --with-libwebsockets-dir[=DIR]     WebSocket: where to find libwebsockets], $PHP_LIBWEBSOCKETS_DIR, "")


if test "$PHP_WEBSOCKET" != "no"; then
	SEARCH_FOR="/include/libwebsockets.h"
	if test -r $PHP_LIBWEBSOCKETS_DIR/$SEARCH_FOR; then
		LIBWEBSOCKETS_DIR=$PHP_LIBWEBSOCKETS_DIR
	else
		SEARCH_PATH="/usr/local /usr ../deps/libwebsockets"
		AC_MSG_CHECKING([for libwebsockets files in default path])
		for i in $SEARCH_PATH; do
			if test -r $i/$SEARCH_FOR; then
				LIBWEBSOCKETS_DIR=$i
			fi
		done
	fi

	if test -z "$LIBWEBSOCKETS_DIR"; then
		AC_MSG_RESULT([not found])
		AC_MSG_ERROR([Please reinstall the libwebsockets distribution])
	fi

	PHP_ADD_INCLUDE($LIBWEBSOCKETS_DIR/include)

	LIBNAME=websockets
	LIBSYMBOL=libwebsocket_callback_on_writable

	PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
	[
		PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $LIBWEBSOCKETS_DIR/$PHP_LIBDIR, WEBSOCKET_SHARED_LIBADD)
		AC_DEFINE(HAVE_WEBSOCKET, 1, [Have WebSocket support])
	],[
		AC_MSG_ERROR([wrong $LIBNAME version or library not found])
	],[
		-L$LIBWEBSOCKETS_DIR/$PHP_LIBDIR -lm
	])

	PHP_SUBST(WEBSOCKET_SHARED_LIBADD)

	PHP_NEW_EXTENSION(websocket, websocket.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi

