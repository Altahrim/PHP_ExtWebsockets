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

#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include <php.h>
#include <libwebsockets.h>
#include "ws_structures.h"

/***** Class \WebSocket\Client *****/

/*--- Definitions ---*/

enum php_client_callbacks {
	PHP_CB_CLIENT_ACCEPT,
	PHP_CB_CLIENT_CLOSE,

	PHP_CB_CLIENT_DATA,

	PHP_CB_CLIENT_COUNT
};

/*--- Storage ---*/

typedef struct _ws_client_obj {
	zend_object std;

	// Available PHP callbacks
	ws_callback *callbacks[PHP_CB_CLIENT_COUNT];
} ws_client_obj;

/*--- Methods ---*/

zend_class_entry *ws_client_ce;
zend_object_handlers ws_client_object_handlers;

PHP_METHOD(WS_Client, __construct);
PHP_METHOD(WS_Client, on);
PHP_METHOD(WS_Client, connect);
PHP_METHOD(WS_Client, send);
PHP_METHOD(WS_Client, sendAsJson);
PHP_METHOD(WS_Client, isConnected);
PHP_METHOD(WS_Client, disconnect);

/*--- Handlers ---*/

zend_object* ws_client_create_object_handler(zend_class_entry *ce TSRMLS_DC);
void ws_client_free_object_storage_handler(ws_client_obj *intern TSRMLS_DC);
void register_ws_client_class(TSRMLS_DC);


#endif /* WEBSOCKET_CLIENT_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
