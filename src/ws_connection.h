/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Altahrïm <me@altahrim.net>                                   |
  +----------------------------------------------------------------------+
*/

#ifndef WEBSOCKET_CONNECTION_H
#define WEBSOCKET_CONNECTION_H

#include <php.h>
#include "ws_constants.h"
#include <libwebsockets.h>

/***** Class \WebSocket\Connection *****/

/*--- Storage ---*/

typedef struct _ws_connection_obj {
	zend_object std;

	// ID (unique on server)
	zend_ulong id;

	// Connection state (Connected/Disconnected)
	zend_bool connected;

	// Write buffer
	zend_string *buf[WEBSOCKET_CONNECTION_BUFFER_SIZE];
	unsigned int read_ptr;
	unsigned int write_ptr;

	// LibWebSockets context
	struct lws *wsi;
} ws_connection_obj;

/*--- Definition ---*/

zend_class_entry *ws_connection_ce;
zend_object_handlers ws_connection_object_handlers;

PHP_METHOD(WS_Connection, sendText);
PHP_METHOD(WS_Connection, sendJson);
PHP_METHOD(WS_Connection, isConnected);
PHP_METHOD(WS_Connection, getUid);
PHP_METHOD(WS_Connection, disconnect);

/*--- Handlers ---*/

zend_object* ws_connection_create_object_handler(zend_class_entry *ce TSRMLS_DC);
void ws_connection_free_object_storage_handler(ws_connection_obj *intern TSRMLS_DC);
void register_ws_connection_class(TSRMLS_DC);

#endif /* WEBSOCKET_CONNECTION_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
