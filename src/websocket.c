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
  | Author: Benjamin Gaussorgues <bengauss+ws@gmail.com                  |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_websocket.h"
#include "ws_constants.h"
#include "zend_smart_str.h"
#include "ext/standard/info.h"
#include "ext/json/php_json.h"

/* If you declare any globals in php_websocket.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(websocket)
*/

/* True global resources - no need for thread safety here */
// static int le_websocket;

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("websocket.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_websocket_globals, websocket_globals)
    STD_PHP_INI_ENTRY("websocket.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_websocket_globals, websocket_globals)
PHP_INI_END()
*/
/* }}} */


/* {{{ php_websocket_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_websocket_init_globals(zend_websocket_globals *websocket_globals)
{
	websocket_globals->global_value = 0;
	websocket_globals->global_string = NULL;
}
*/
/* }}} */

struct ws_connection_data {
	zval connection;
};

static void get_token_as_array(zval *arr, struct libwebsocket *wsi)
{
	const unsigned char *key;
	char buf[256];
	unsigned int n = 0, keylen, buflen, i;

	ZVAL_NEW_ARR(arr);
	array_init_size(arr, 20);

	do {
		key = lws_token_to_string(n);
		if (!key || !lws_hdr_total_length(wsi, n)) {
			++n;
			continue;
		}

		keylen = strlen(key);
		--keylen;
		if (key[keylen] == ':') {
			buflen = lws_hdr_copy(wsi, buf, sizeof buf, n);
			add_assoc_stringl_ex(arr, key, keylen, buf, buflen);
		}
		++n;
	} while (key);
}

static int callback_php(struct libwebsocket_context *context, struct libwebsocket *wsi, enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
	zval *server = libwebsocket_context_user(context);
	ws_server_obj *intern = (ws_server_obj *) Z_OBJ_P(server);
	ws_connection_obj * wsconn;
	struct ws_connection_data *session_data = user;
	zval retval, tmp;
	int n, return_code = 0;
	zend_string *text;

	switch (reason) {
		case LWS_CALLBACK_WSI_CREATE:

			break;

		case LWS_CALLBACK_WSI_DESTROY:
			printf("Destroy connection\n");
			break;

		case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
			/* TODO
			sockfd = (int) in;
			struct sockaddr_storage addr;
			int len = sizeof(addr);
			int port; char ipstr[64] = "";
			getpeername(sockfd, (struct sockaddr*) &addr, &len);
			if (addr.ss_family == AF_INET) {
			    struct sockaddr_in *s = (struct sockaddr_in *)&addr;
			    port = ntohs(s->sin_port);
			    lws_plat_inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
			} else { // AF_INET6
			    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
			    port = ntohs(s->sin6_port);
			    lws_plat_inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
			}
			*/
			break;

		case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
			printf("Filter Protocol Connection\n");
			if (Z_TYPE(intern->cb_filter_headers) != IS_UNDEF) {
				get_token_as_array(&tmp, wsi);

				ZVAL_FALSE(&retval);
				zval params[2] = { tmp, *server };
				if (FAILURE == call_user_function(CG(function_table), NULL, &intern->cb_filter_headers, &retval, 2, params TSRMLS_CC)) {
					php_error_docref(NULL, E_WARNING, "Unable to call filter headers callback");
				}

				if (!zval_is_true(&retval)) {
					return_code = -1;
				}

				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_ESTABLISHED:
			object_init_ex(&session_data->connection, ws_connection_ce);
			wsconn = (ws_connection_obj *) Z_OBJ(session_data->connection);
			wsconn->id = ++((ws_server_obj *) Z_OBJ(*server))->next_id;
			wsconn->context = context;
			printf("Accept connection %lu. Callback is %p\n", wsconn->id, intern->cb_accept);
			wsconn->wsi = wsi;

			zval_addref_p(&session_data->connection);
			add_index_zval(&intern->connections, wsconn->id, &session_data->connection);

			wsconn->connected = 1;
			if (Z_TYPE(intern->cb_accept) != IS_UNDEF) {
				ZVAL_NULL(&retval);
				zval params[2] = { *server, session_data->connection };
				if (FAILURE == call_user_function(CG(function_table), NULL, &intern->cb_accept, &retval, 2, params TSRMLS_CC)) {
					php_error_docref(NULL, E_WARNING, "Unable to call accept callback");
				}
				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_RECEIVE:
			printf("Receive data. Callback is %p\n", intern->cb_data);
			if (Z_TYPE(intern->cb_data) != IS_UNDEF) {
				ZVAL_NULL(&retval);
				ZVAL_STRINGL(&tmp, in, len);
				zval params[3] = { *server, session_data->connection, tmp };
				if (FAILURE == call_user_function(CG(function_table), NULL, &intern->cb_data, &retval, 3, params TSRMLS_CC)) {
					php_error_docref(NULL, E_WARNING, "Unable to call data callback");
				}

				// Test if different from true (or assimilated), close connection
				if (!zval_is_true(&retval)) {
					wsconn = (ws_connection_obj *) Z_OBJ(session_data->connection);
					wsconn->connected = 0;
					return_code = -1;
				}

				zval_dtor(&retval);
				zval_dtor(&tmp);
			}
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:
			wsconn = (ws_connection_obj *) Z_OBJ(session_data->connection);
			while (wsconn->read_ptr != wsconn->write_ptr) {
				text = wsconn->buf[wsconn->read_ptr];
				n = libwebsocket_write(wsi, text->val, text->len, LWS_WRITE_TEXT);

				if (n < 0) {
					lwsl_err("Write to socket %lu failed with code %d\n", wsconn->id, n);
					return 1;
				}
				if (n < (int) text->len) {
					lwsl_err("Partial write\n");
					return -1;
				}

				// Cleanup
				wsconn->buf[wsconn->read_ptr] = NULL;
				wsconn->read_ptr = (wsconn->read_ptr + 1) % WEBSOCKET_CONNECTION_BUFFER_SIZE;
				zend_string_delref(text);
				if (zend_string_refcount(text) < 1) {
					zend_string_free(text);
				}
			}

			break;

		case LWS_CALLBACK_CLOSED:
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			wsconn = (ws_connection_obj *) Z_OBJ(session_data->connection);
			wsconn->connected = 0;
			printf("Close %lu. Callback is %p\n", wsconn->id, intern->cb_close);

			// Drop from active connections
			zend_hash_index_del(Z_ARRVAL(intern->connections), wsconn->id);
			zval_delref_p(&session_data->connection);

			if (Z_TYPE(intern->cb_close) != IS_UNDEF) {
				ZVAL_NULL(&retval);
				zval params[2] = { *server, session_data->connection };
				if (FAILURE == call_user_function(CG(function_table), NULL, &intern->cb_close, &retval, 2, params TSRMLS_CC)) {
					php_error_docref(NULL, E_WARNING, "Unable to call close callback");
				}
				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
		case LWS_CALLBACK_PROTOCOL_INIT:
		case LWS_CALLBACK_PROTOCOL_DESTROY:
		case LWS_CALLBACK_GET_THREAD_ID:
		case LWS_CALLBACK_ADD_POLL_FD:
		case LWS_CALLBACK_DEL_POLL_FD:
		case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
		case LWS_CALLBACK_LOCK_POLL:
		case LWS_CALLBACK_UNLOCK_POLL:
			// Ignore
			break;

		default:
			printf(" – Non-handled action (reason: %d)\n", reason);
	}

	return intern->exit_request == 1 ? -1 : return_code;
}

static struct libwebsocket_protocols protocols[] = {
	{
		"default",							/* name */
		callback_php,						/* callback */
		sizeof(struct ws_connection_data),	/* per_session_data_size */
		0									/* max frame size / rx buffer */
	},
	{ NULL, NULL, 0, 0 }
};

static void php_ws_conn_close(ws_connection_obj *conn) {
	unsigned char payload[4] = "Bye";

	conn->connected = 0;
	printf("Send close to %lu\n", conn->id);
	libwebsocket_write(conn->wsi, payload, strlen(payload), LWS_WRITE_CLOSE);
}

static int php_ws_conn_write(ws_connection_obj *conn, zend_string *text) {
	if (!conn->connected) {
		php_error_docref(NULL, E_WARNING, "Client is disconnected\n");
		return -1;
	}
	if (((conn->write_ptr + 1) % WEBSOCKET_CONNECTION_BUFFER_SIZE) == conn->read_ptr) {
		php_error_docref(NULL, E_WARNING, "Write buffer is full\n");
		return -1;
	}

	zend_string_addref(text);
	conn->buf[conn->write_ptr] = text;
	conn->write_ptr = (conn->write_ptr + 1) % WEBSOCKET_CONNECTION_BUFFER_SIZE;

	libwebsocket_callback_on_writable(conn->context, conn->wsi);

	return text->len;
}


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

static void ws_server_free_object_storage_handler(ws_server_obj *intern TSRMLS_DC)
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

zend_object* ws_connection_create_object_handler(zend_class_entry *ce TSRMLS_DC)
{
	printf("Create connection object\n");
	ws_connection_obj *intern = emalloc(sizeof(ws_connection_obj));
	memset(intern, 0, sizeof(ws_connection_obj));

	zend_object_std_init(&intern->std, ce TSRMLS_CC);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &ws_connection_object_handlers;

	intern->connected = 0;
	intern->wsi = NULL;
	intern->read_ptr = 0;
	intern->write_ptr = 0;

	return &intern->std;
}

static void ws_connection_free_object_storage_handler(ws_connection_obj *intern TSRMLS_DC)
{
	printf("Free connection object\n");

	while (intern->read_ptr != intern->write_ptr) {
		zend_string_delref(intern->buf[intern->read_ptr]);
		intern->read_ptr = (intern->read_ptr + 1) % WEBSOCKET_CONNECTION_BUFFER_SIZE;
	}
	intern->context = NULL;

	zend_object_std_dtor(&intern->std TSRMLS_CC);

	// FIXME Following error when uncomment this line:
	//       0x00007ffff6756a98 in __GI_raise (sig=sig@entry=0x6) at ../sysdeps/unix/sysv/linux/raise.c:55
	//       return INLINE_SYSCALL (tgkill, 3, pid, selftid, sig);
	// efree(intern);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_run, 0, 0, 0)
ZEND_END_ARG_INFO()

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

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

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

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

/* {{{ proto void WebSocket\Server::__construct()
	Constructor */
PHP_METHOD(WS_Server, __construct)
{
	ws_server_obj *intern;
	long port = 8080;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &port) != SUCCESS) {
		return;
	}

	intern = (ws_server_obj *) Z_OBJ_P(getThis());
	intern->info.port = port;
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_stop, 0, 0, 0)
ZEND_END_ARG_INFO()

/* {{{ proto void WebSocket\Server::stop()
	Stop websocket server */
PHP_METHOD(WS_Server, stop)
{
	ws_server_obj *intern;
	ws_connection_obj *conn;

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	intern = (ws_server_obj *) Z_OBJ_P(getThis());

	// Mark as stopped
	intern->exit_request = 1;
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_onClientAccept, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

/* {{{ proto void WebSocket\Server::onClientAccept(callback callback)
	Register a callback which will be called when a new connection is established */
PHP_METHOD_WS_CALLBACK(WS_Server, onClientAccept, cb_accept)
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_onClientData, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

/* {{{ proto void WebSocket\Server::onClientData(callback callback)
	Register a callback which will be called when data is received from client */
PHP_METHOD_WS_CALLBACK(WS_Server, onClientData, cb_data)
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_onTick, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

/* {{{ proto void WebSocket\Server::onTick(callback callback)
	Register a callback which will be called on each tick */
PHP_METHOD_WS_CALLBACK(WS_Server, onTick, cb_tick)
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_onClose, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

/* {{{ proto void WebSocket\Server::onClose(callback callback)
	Register a callback which will be called when a connection is closed */
PHP_METHOD_WS_CALLBACK(WS_Server, onClose, cb_close)
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_onFilterHeaders, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 0)
ZEND_END_ARG_INFO()

/* {{{ proto boolean WebSocket\Server::onFilterHeaders(callback callback)
	Register a callback which will be called to check headers and accept or not the new connection */
PHP_METHOD_WS_CALLBACK(WS_Server, onFilterHeaders, cb_filter_headers)
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_server_broadcast, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, ignored, IS_LONG, 1)
ZEND_END_ARG_INFO()

/* {{{ proto void WebSocket\Server::broadcast(string text)
	Broadcast a message to all connected clients */
PHP_METHOD(WS_Server, broadcast)
{
	ws_server_obj *intern;
	zend_string *str;
	ws_connection_obj *conn;
	long ignoredId = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|l", &str, &ignoredId) != SUCCESS) {
		return;
	}

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

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_connection_sendText, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

/* {{{ proto int|false WebSocket\Connection::sendText(string text)
	Send a text to the client */
PHP_METHOD(WS_Connection, sendText)
{
	ws_connection_obj *intern;
	zend_string *text;
	int n;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &text) == FAILURE) {
		return;
	}

	intern = (ws_connection_obj *) Z_OBJ_P(getThis());
	n = php_ws_conn_write(intern, text);
	efree(text);
	if (-1 == n) {
		RETURN_FALSE;
	}
	RETURN_LONG(n);
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_connection_sendJson, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, payload, 0, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(WS_Connection, sendJson)
{
	ws_connection_obj *intern;
	smart_str text = {0};
	zval *val;
	int n;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &val) == FAILURE) {
		return;
	}

	intern = (ws_connection_obj *) Z_OBJ_P(getThis());
	php_json_encode(&text, val, PHP_JSON_UNESCAPED_UNICODE|PHP_JSON_UNESCAPED_SLASHES);
	n = php_ws_conn_write(intern, text.s);
	smart_str_free(&text);
	if (-1 == n) {
		RETURN_FALSE;
	}
	RETURN_LONG(n);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_connection_is_connected, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(WS_Connection, isConnected)
{
	ws_connection_obj *intern;

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	intern = (ws_connection_obj *) Z_OBJ_P(getThis());

	RETURN_BOOL(intern->connected);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_connection_get_uid, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(WS_Connection, getUid)
{
	ws_connection_obj *intern;

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	intern = (ws_connection_obj *) Z_OBJ_P(getThis());

	RETURN_LONG(intern->id);
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_connection_disconnect, 0, 0, 0)
ZEND_END_ARG_INFO()

/* {{{ proto void WebSocket\Connection::disconnect()
	Close connection to the client */
PHP_METHOD(WS_Connection, disconnect)
{
	ws_connection_obj *intern;

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	intern = (ws_connection_obj *) Z_OBJ_P(getThis());
	php_ws_conn_close(intern);
}
/* }}} */

const zend_function_entry obj_ws_connection_funcs[] = {
	PHP_ME(WS_Connection, sendText, arginfo_ws_connection_sendText, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Connection, sendJson, arginfo_ws_connection_sendJson, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Connection, isConnected, arginfo_ws_connection_is_connected, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Connection, getUid, arginfo_ws_connection_get_uid, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Connection, disconnect, arginfo_ws_connection_disconnect, ZEND_ACC_PUBLIC)
	PHP_FE_END
};


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(websocket)
{
	zend_class_entry tmp_ws_server_ce, tmp_ws_connection_ce;

	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/

	// Class WebSocket\Server
	INIT_NS_CLASS_ENTRY(tmp_ws_server_ce, "WebSocket", "Server", obj_ws_server_funcs);
	ws_server_ce = zend_register_internal_class(&tmp_ws_server_ce TSRMLS_CC);
	ws_server_ce->create_object = ws_server_create_object_handler;
	memcpy(&ws_server_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	ws_server_object_handlers.free_obj = (zend_object_free_obj_t) ws_server_free_object_storage_handler;

	// Class WebSocket\Connection
	INIT_NS_CLASS_ENTRY(tmp_ws_connection_ce, "WebSocket", "Connection", obj_ws_connection_funcs);
	ws_connection_ce = zend_register_internal_class(&tmp_ws_connection_ce TSRMLS_CC);
	ws_connection_ce->create_object = ws_connection_create_object_handler;
	memcpy(&ws_connection_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	ws_connection_object_handlers.free_obj = (zend_object_free_obj_t) ws_connection_free_object_storage_handler;

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(websocket)
{
#if defined(COMPILE_DL_WEBSOCKET) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(websocket)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "WebSocket support", "enabled");
	php_info_print_table_header(2, "WebSocket version", PHP_WEBSOCKET_VERSION);
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ websocket_functions[]
 * Every user visible function must have an entry in websocket_functions[].
 */
const zend_function_entry websocket_functions[] = {
	PHP_FE_END
};
/* }}} */

/* {{{ websocket_module_entry
 */
zend_module_entry websocket_module_entry = {
	STANDARD_MODULE_HEADER,
	"WebSocket",
	websocket_functions,
	PHP_MINIT(websocket),
	NULL,
	PHP_RINIT(websocket),
	NULL,
	PHP_MINFO(websocket),
	PHP_WEBSOCKET_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_WEBSOCKET
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(websocket)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
