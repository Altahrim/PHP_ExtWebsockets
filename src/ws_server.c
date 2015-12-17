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

#include "php_websocket.h"
#include "ws_eventloop.h"
#include "ws_server.h"
#include "ws_libwebsockets.h"
#include <php_network.h>
#include <zend_interfaces.h>

ZEND_EXTERN_MODULE_GLOBALS(websocket)

/***** Class \WebSocket\Server *****/

/*--- Definitions ---*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_run, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_setEventLoop, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_OBJECT, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_serviceFd, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_RESOURCE, 0)
	ZEND_ARG_TYPE_INFO(0, events, IS_LONG, 0)
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

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ws_server_on, 0, 2, IS_TRUE|IS_FALSE, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 1)
ZEND_END_ARG_INFO()

const zend_function_entry obj_ws_server_funcs[] = {
	PHP_ME(WS_Server, __construct, arginfo_ws_server___construct, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, run, arginfo_ws_server_run, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, setEventLoop, arginfo_ws_server_setEventLoop, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, serviceFd, arginfo_ws_server_serviceFd, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, stop, arginfo_ws_server_stop, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Server, on, arginfo_ws_server_on, ZEND_ACC_PUBLIC)
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

	zend_declare_class_constant_long(ws_server_ce, "ON_ACCEPT", sizeof("ON_ACCEPT") - 1, PHP_CB_ACCEPT);
	zend_declare_class_constant_long(ws_server_ce, "ON_CLOSE", sizeof("ON_CLOSE") - 1, PHP_CB_CLOSE);
	zend_declare_class_constant_long(ws_server_ce, "ON_DATA", sizeof("ON_DATA") - 1, PHP_CB_DATA);
	zend_declare_class_constant_long(ws_server_ce, "ON_TICK", sizeof("ON_TICK") - 1, PHP_CB_TICK);
}

/*--- Handlers ---*/

zend_object* ws_server_create_object_handler(zend_class_entry *ce TSRMLS_DC)
{
	int i;
	ws_server_obj *intern = emalloc(sizeof(ws_server_obj));
	memset(intern, 0, sizeof(ws_server_obj));

	zend_object_std_init(&intern->std, ce TSRMLS_CC);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &ws_server_object_handlers;

	// Set LibWebsockets default options
	memset(&intern->info, 0, sizeof(intern->info));
	intern->info.uid = -1;
	intern->info.gid = -1;
	intern->info.port = 8080;
	intern->info.ssl_private_key_filepath = intern->info.ssl_cert_filepath = NULL;	//FIXME HTTPS
	intern->info.protocols = protocols;
	intern->info.extensions = NULL;
	intern->info.options = 0;

	intern->exit_request = 0;
	for (i = 0; i < PHP_CB_COUNT; ++i) {
		intern->callbacks[i] = NULL;
	}

	intern->eventloop = NULL;
	intern->eventloop_sockets = NULL;

	intern->next_id = 0;
	array_init_size(&intern->connections, 20);

	return &intern->std;
}

void ws_server_free_object_storage_handler(ws_server_obj *intern TSRMLS_DC)
{
	zend_object_std_dtor(&intern->std TSRMLS_CC);
	zval_dtor(&intern->connections);
	efree(intern);
}

/*--- Methods ---*/

zend_bool invoke_eventloop_cb(ws_server_obj *intern, const char *func, php_socket_t sock, int flags)
{
	zval retval, el;
	zval php_flags;
	zval *php_sock;
	zval params[2];
	unsigned int param_count = 0;
	php_stream *stream;

	if (sock) {
		php_sock = zend_hash_index_find(intern->eventloop_sockets, sock);
		if (NULL == php_sock) {
			stream = php_stream_fopen_from_fd(sock, "rb", NULL);
			php_sock = emalloc(sizeof(zval));
			ZVAL_RES(php_sock, stream->res);
			Z_ADDREF_P(php_sock);
			zend_hash_index_add(intern->eventloop_sockets, sock, php_sock);
		}
		ZVAL_COPY_VALUE(&params[param_count++], php_sock);
	}
	if (flags >= 0) {
		ZVAL_LONG(&params[param_count++], flags);
	}
	ZVAL_NULL(&retval);
	zend_call_method(intern->eventloop, Z_OBJCE_P(intern->eventloop), NULL, func, strlen(func), &retval, param_count, &params[0], &params[1]);

	if (zval_is_true(&retval)) {
		return 0;
	}
	return -1;
}

/* {{{ proto void WebSocket\Server::__construct()
	Constructor */
PHP_METHOD(WS_Server, __construct)
{
	ws_server_obj *intern;
	long port = 8080;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();

	if (NULL != WEBSOCKET_G(php_obj)) {
		// TODO Warning, server already built
		RETURN_FALSE;
	}

	WEBSOCKET_G(php_obj) = emalloc(sizeof(zval *));
	ZVAL_ZVAL(WEBSOCKET_G(php_obj), getThis(), 0, ZVAL_PTR_DTOR);
	intern = (ws_server_obj *) Z_OBJ_P(getThis());
	intern->info.port = port;
}
/* }}} */

/* {{{ proto void WebSocket\Server::setEventLoop()
	Set external event loop */
PHP_METHOD(WS_Server, setEventLoop)
{
	zval *el;
	ws_server_obj *intern;

	ZEND_PARSE_PARAMETERS_START(1, 1);
		Z_PARAM_OBJECT_OF_CLASS(el, ws_eventloop_ce)
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_server_obj *) Z_OBJ_P(getThis());
	intern->eventloop = emalloc(sizeof(zval *));
	ZVAL_ZVAL(intern->eventloop, el, 0, ZVAL_PTR_DTOR);
	intern->eventloop_sockets = emalloc(sizeof(HashTable));
	zend_hash_init(intern->eventloop_sockets, 20, NULL, ZVAL_PTR_DTOR, 0);
}
/* }}} */

/* {{{ proto void WebSocket\Server::serviceFd()
	Service a socket (used with external event loop) */
PHP_METHOD(WS_Server, serviceFd)
{
	ws_server_obj *intern;
	struct lws_pollfd pollfd;
	zval* res;
	php_stream *stream;
	php_socket_t fd;
	zend_long revents;

	ZEND_PARSE_PARAMETERS_START(2, 2);
		Z_PARAM_RESOURCE(res)
		Z_PARAM_LONG(revents);
	ZEND_PARSE_PARAMETERS_END();

	php_stream_from_zval(stream, res);
	if (FAILURE == php_stream_cast(stream, PHP_STREAM_AS_FD, (void **)&pollfd.fd, REPORT_ERRORS)) {
		// TODO Warning, error ?
		RETURN_FALSE;
	}

	pollfd.events = 1;
	pollfd.revents = revents;
	intern = (ws_server_obj *) Z_OBJ_P(getThis());
	lws_service_fd(WEBSOCKET_G(context), &pollfd);

	if (pollfd.revents) {
		RETURN_FALSE;
	} else {
		RETURN_TRUE;
	}
}
/* }}} */

/* {{{ proto void WebSocket\Server::run()
	Launch WebSocket server */
PHP_METHOD(WS_Server, run)
{
	ws_server_obj *intern;
	ws_connection_obj *conn;
	struct lws *context;
	int n = 0;
	unsigned int ms = 0, oldMs = 0, tickInterval = 1000 / PHP_WEBSOCKET_FREQUENCY;
	int nextTick = 0;
	struct timeval tv;
	struct _ws_protocol_storage * context_data;

	ZEND_PARSE_PARAMETERS_START(0, 0);
	ZEND_PARSE_PARAMETERS_END();

	// Start WebSocket
	intern = (ws_server_obj *) Z_OBJ_P(getThis());
	intern->info.user = emalloc(sizeof(struct _ws_protocol_storage));
	context_data = intern->info.user;

	if (NULL != WEBSOCKET_G(intern)) {
		// TODO Warning, server already running
		RETURN_FALSE;
	}
	WEBSOCKET_G(intern) = intern;
	WEBSOCKET_G(context) = lws_create_context(&intern->info);

	// If an external event loop is used, nothing more to do
	if (intern->eventloop) {
		return;
	}

	gettimeofday(&tv, NULL);
	oldMs = ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	while (n >= 0 && !intern->exit_request) {
		if (nextTick <= 0) {
			printf("Tick from ext.\n");
			if (intern->callbacks[PHP_CB_TICK]) {
				zval retval;
				ZVAL_NULL(&retval);
				zval params[1] = { *getThis() };
				intern->callbacks[PHP_CB_TICK]->fci.param_count = 1;
				intern->callbacks[PHP_CB_TICK]->fci.params = params;
				intern->callbacks[PHP_CB_TICK]->fci.retval = &retval;
				if (FAILURE == zend_call_function(&intern->callbacks[PHP_CB_TICK]->fci, &intern->callbacks[PHP_CB_TICK]->fcc)) {
					php_error_docref(NULL, E_WARNING, "Unable to call tick callback");
				}
			}
			nextTick = tickInterval;
		}

		n = lws_service(WEBSOCKET_G(context), nextTick);

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
	Stop WebSocket server */
PHP_METHOD(WS_Server, stop)
{
	ws_server_obj *intern;
	ws_connection_obj *conn;

	ZEND_PARSE_PARAMETERS_START(0, 0);
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_server_obj *) Z_OBJ_P(getThis());

	// Mark as stopped
	intern->exit_request = 1;

	if (intern->eventloop) {
		// TODO In case of EventLoop, disconnect users here
	}
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

/* {{{ proto bool WebSocket\Server::on(string event, callback callback)
	Register a callback for specified event */
PHP_METHOD(WS_Server, on)
{
	ws_server_obj *intern;
	ws_callback *cb = emalloc(sizeof(ws_callback));
	long event;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_LONG(event)
		Z_PARAM_FUNC(cb->fci, cb->fcc)
	ZEND_PARSE_PARAMETERS_END();

	if (event < 0 || event >= PHP_CB_COUNT) {
		php_error_docref(NULL, E_WARNING, "Try to add an invalid callback");
		RETURN_FALSE;
	}

	intern = (ws_server_obj *) Z_OBJ_P(getThis());
	intern->callbacks[event] = cb;

	RETURN_TRUE;
}
/* }}} */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
