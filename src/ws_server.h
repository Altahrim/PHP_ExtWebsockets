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

#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <php.h>
#include <libwebsockets.h>

/***** Class \WebSocket\Connection *****/

/*--- Storage ---*/

typedef struct _ws_server_obj {
	zend_object std;

	// LibWebsockets parameters
	struct lws_context_creation_info info;

	// Available PHP callbacks
	zval cb_accept;
	zval cb_tick;
	zval cb_close;
	zval cb_data;
	zval cb_filter_headers;

	// Current connections
	zend_ulong next_id;
	zval connections;

	zend_bool exit_request;
} ws_server_obj;

/*--- Storage ---*/

zend_class_entry *ws_server_ce;
zend_object_handlers ws_server_object_handlers;

PHP_METHOD(WS_Server, __construct);
PHP_METHOD(WS_Server, run);
PHP_METHOD(WS_Server, stop);
PHP_METHOD(WS_Server, broadcast);
PHP_METHOD(WS_Server, onClientAccept);
PHP_METHOD(WS_Server, onClientData);
PHP_METHOD(WS_Server, onTick);
PHP_METHOD(WS_Server, onClose);
PHP_METHOD(WS_Server, onFilterHeaders);

/*--- Handlers ---*/

zend_object* ws_server_create_object_handler(zend_class_entry *ce TSRMLS_DC);
void ws_server_free_object_storage_handler(ws_server_obj *intern TSRMLS_DC);
void register_ws_server_class(TSRMLS_DC);


#endif /* WEBSOCKET_SERVER_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
