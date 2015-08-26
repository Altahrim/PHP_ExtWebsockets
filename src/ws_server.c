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

/* $Id$ */

#include <php.h>
#include "ws_server.h"
#include "ws_libwebsockets.h"

/***** Class \Websocket\Server *****/

/*--- Helpers ---*/

#define PHP_METHOD_WS_CALLBACK(wsObj, callbackName, callbackStorage) 						\
PHP_METHOD(wsObj, callbackName)																\
{																							\
	printf("Add callback %s (%s)\n", #callbackName, #callbackStorage);						\
	ws_server_obj *intern;																	\
	zval *cb;																				\
																							\
	ZEND_PARSE_PARAMETERS_START(1, 1)														\
		Z_PARAM_ZVAL(cb)																	\
	ZEND_PARSE_PARAMETERS_END();															\
																							\
	if (zend_is_callable(cb, 0, NULL) == FAILURE) {											\
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "$callback is not a valid callback");	\
		RETURN_FALSE;																		\
	}																						\
																							\
	intern = (ws_server_obj *) Z_OBJ_P(getThis());											\
																							\
	zval_dtor(&intern->callbackStorage);													\
	ZVAL_DUP(&intern->callbackStorage, cb);													\
																							\
	RETURN_TRUE;																			\
}

/*--- Definitions ---*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_run, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_stop, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_broadcast, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, ignored, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_onClientAccept, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_onClientData, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_onTick, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_onClose, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_onFilterHeaders, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

const zend_function_entry obj_ws_server_funcs[] = {
	PHP_ME(WS_Server, __construct, arginfo_ws_server___construct, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, run, arginfo_ws_server_run, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, stop, arginfo_ws_server_stop, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, onClientAccept, arginfo_ws_server_onClientAccept, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, onClientData, arginfo_ws_server_onClientData, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, onTick, arginfo_ws_server_onTick, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, onClose, arginfo_ws_server_onClose, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, onFilterHeaders, arginfo_ws_server_onFilterHeaders, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, broadcast, arginfo_ws_server_broadcast, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/*--- Registration ---*/

void register_ws_server_class(TSRMLS_DC)
{
	zend_class_entry tmp_ws_server_ce;

	INIT_NS_CLASS_ENTRY(tmp_ws_server_ce, "WebSocket", "Server", obj_ws_server_funcs);
	ws_server_ce = zend_register_internal_class(&tmp_ws_server_ce TSRMLS_CC);
	ws_server_ce->create_object = ws_server_create_object_handler;
	memcpy(&ws_server_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	ws_server_object_handlers.free_obj = (zend_object_free_obj_t) ws_server_free_object_storage_handler;
}

/*--- Handlers ---*/

zend_object* ws_server_create_object_handler(zend_class_entry *ce TSRMLS_DC)
{
	printf("Create server object\n");
	ws_server_obj *intern = emalloc(sizeof(ws_server_obj));
	memset(intern, 0, sizeof(ws_server_obj));

	zend_object_std_init(&intern->std, ce TSRMLS_CC);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &ws_server_object_handlers;

	// Set Libwebsockets default options
	intern->info.uid = -1;
	intern->info.gid = -1;
	intern->info.port = 8080;
	intern->info.iface = NULL;
	intern->info.options = 0;
	intern->info.ssl_cert_filepath = NULL;
	intern->info.ssl_private_key_filepath = NULL;
	intern->info.user = NULL;

	intern->info.protocols = protocols;

	intern->exit_request = 0;
	ZVAL_UNDEF(&intern->cb_accept);
	ZVAL_UNDEF(&intern->cb_close);
	ZVAL_UNDEF(&intern->cb_data);
	ZVAL_UNDEF(&intern->cb_tick);
	ZVAL_UNDEF(&intern->cb_filter_headers);
	intern->next_id = 0;
	array_init_size(&intern->connections, 20);

	return &intern->std;
}

void ws_server_free_object_storage_handler(ws_server_obj *intern TSRMLS_DC)
{
	printf("Free server object\n");
	zend_object_std_dtor(&intern->std TSRMLS_CC);
	zval_dtor(&intern->cb_accept);
	zval_dtor(&intern->cb_close);
	zval_dtor(&intern->cb_data);
	zval_dtor(&intern->cb_filter_headers);
	zval_dtor(&intern->cb_tick);
	zval_dtor(&intern->connections);
	efree(intern);
	printf("Server destroyed\n");
}

/*--- Methods ---*/

/* {{{ proto void WebSocket\Server::__construct()
	Constructor */
PHP_METHOD(WS_Server, __construct)
{
	ws_server_obj *intern;
	long port = 8080;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_server_obj *) Z_OBJ_P(getThis());
	intern->info.port = port;
}
/* }}} */

/* {{{ proto void WebSocket\Server::run()
	Launch websocket server */
PHP_METHOD(WS_Server, run)
{
	ws_server_obj *intern;
	ws_connection_obj *conn;
	struct libwebsocket_context *context;
	int n = 0;
	unsigned int ms = 0, oldMs = 0, tickInterval = 1000 / PHP_WEBSOCKET_FREQUENCY;
	int nextTick = 0;
	struct timeval tv;

	ZEND_PARSE_PARAMETERS_START(0, 0);
	ZEND_PARSE_PARAMETERS_END();

	// Start websocket
	intern = (ws_server_obj *) Z_OBJ_P(getThis());
	intern->info.user = getThis();
	context = libwebsocket_create_context(&intern->info);

	gettimeofday(&tv, NULL);
	oldMs = ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	while (n >= 0 && !intern->exit_request) {
		if (nextTick <= 0) {
			// printf("Tick. Callback is %p\n", intern->cb_tick);
			if (Z_TYPE(intern->cb_tick) != IS_UNDEF) {
				zval retval;
				ZVAL_NULL(&retval);
				zval params[1] = { *getThis() };
				if (FAILURE == call_user_function(CG(function_table), NULL, &intern->cb_tick, &retval, 1, params TSRMLS_CC)) {
					intern->exit_request = 1;
					php_error_docref(NULL, E_WARNING, "Unable to call tick callback");
				}
			}
			nextTick = tickInterval;
		}

		n = libwebsocket_service(context, nextTick);

		gettimeofday(&tv, NULL);
		ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
		nextTick -= ms - oldMs;
		oldMs = ms;
	}

	// Disconnect users
	ZEND_HASH_FOREACH(Z_ARR(intern->connections), 0);
		conn = (ws_connection_obj *) Z_OBJ_P(_z);
		php_ws_conn_close(conn);
		zval_delref_p(_z);
		zend_hash_index_del(Z_ARR(intern->connections), _p->h);
	ZEND_HASH_FOREACH_END();
}
/* }}} */

/* {{{ proto void WebSocket\Server::stop()
	Stop websocket server */
PHP_METHOD(WS_Server, stop)
{
	ws_server_obj *intern;
	ws_connection_obj *conn;

	ZEND_PARSE_PARAMETERS_START(0, 0);
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_server_obj *) Z_OBJ_P(getThis());

	// Mark as stopped
	intern->exit_request = 1;
}
/* }}} */

/* {{{ proto void WebSocket\Server::broadcast(string text)
	Broadcast a message to all connected clients */
PHP_METHOD(WS_Server, broadcast)
{
	ws_server_obj *intern;
	zend_string *str;
	ws_connection_obj *conn;
	long ignoredId = -1;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_STR(str)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(ignoredId)
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_server_obj *) Z_OBJ_P(getThis());
	ZEND_HASH_FOREACH(Z_ARR(intern->connections), 0);
		// TODO Test return? Interrupt if a write fail?
		conn = (ws_connection_obj *) Z_OBJ_P(_z);
		if (conn->id == ignoredId) {
			continue;
		}
		php_ws_conn_write(conn, str);
	ZEND_HASH_FOREACH_END();
	efree(str);
}
/* }}} */

/* {{{ proto void WebSocket\Server::onClientAccept(callback callback)
	Register a callback which will be called when a new connection is established */
PHP_METHOD_WS_CALLBACK(WS_Server, onClientAccept, cb_accept)
/* }}} */

/* {{{ proto void WebSocket\Server::onClientData(callback callback)
	Register a callback which will be called when data is received from client */
PHP_METHOD_WS_CALLBACK(WS_Server, onClientData, cb_data)
/* }}} */

/* {{{ proto void WebSocket\Server::onTick(callback callback)
	Register a callback which will be called on each tick */
PHP_METHOD_WS_CALLBACK(WS_Server, onTick, cb_tick)
/* }}} */

/* {{{ proto void WebSocket\Server::onClose(callback callback)
	Register a callback which will be called when a connection is closed */
PHP_METHOD_WS_CALLBACK(WS_Server, onClose, cb_close)
/* }}} */

/* {{{ proto boolean WebSocket\Server::onFilterHeaders(callback callback)
	Register a callback which will be called to check headers and accept or not the new connection */
PHP_METHOD_WS_CALLBACK(WS_Server, onFilterHeaders, cb_filter_headers)
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
