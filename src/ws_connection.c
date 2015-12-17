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
#include "ws_connection.h"
#include <ext/json/php_json.h>
#include "ws_libwebsockets.h"
#include <zend_smart_str.h>

/***** Class \WebSocket\Connection *****/

/*--- Definitions ---*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_connection_sendText, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_connection_sendJson, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, payload, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_connection_is_connected, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_connection_get_uid, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_connection_disconnect, 0, 0, 0)
ZEND_END_ARG_INFO()

const zend_function_entry obj_ws_connection_funcs[] = {
	PHP_ME(WS_Connection, sendText, arginfo_ws_connection_sendText, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Connection, sendJson, arginfo_ws_connection_sendJson, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Connection, isConnected, arginfo_ws_connection_is_connected, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Connection, getUid, arginfo_ws_connection_get_uid, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Connection, disconnect, arginfo_ws_connection_disconnect, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/*--- Registration ---*/

void register_ws_connection_class(TSRMLS_DC)
{
	zend_class_entry tmp_ws_connection_ce;

	INIT_NS_CLASS_ENTRY(tmp_ws_connection_ce, "WebSocket", "Connection", obj_ws_connection_funcs);
	ws_connection_ce = zend_register_internal_class(&tmp_ws_connection_ce TSRMLS_CC);
	ws_connection_ce->create_object = ws_connection_create_object_handler;
	memcpy(&ws_connection_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	ws_connection_object_handlers.free_obj = (zend_object_free_obj_t) ws_connection_free_object_storage_handler;
}

/*--- Handlers ---*/

zend_object* ws_connection_create_object_handler(zend_class_entry *ce TSRMLS_DC)
{
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

void ws_connection_free_object_storage_handler(ws_connection_obj *intern TSRMLS_DC)
{
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

/*--- Methods ---*/

/* {{{ proto int|false WebSocket\Connection::sendText(string text)
	Send a text to the client */
PHP_METHOD(WS_Connection, sendText)
{
	ws_connection_obj *intern;
	zend_string *text;
	int n;

	ZEND_PARSE_PARAMETERS_START(1, 1);
		Z_PARAM_STR(text)
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_connection_obj *) Z_OBJ_P(getThis());
	n = php_ws_conn_write(intern, text);
	efree(text);
	if (-1 == n) {
		RETURN_FALSE;
	}
	RETURN_LONG(n);
}
/* }}} */

/* {{{ proto int|false WebSocket\Connection::sendJson(mixed payload)
	Send a JSON to the client */
PHP_METHOD(WS_Connection, sendJson)
{
	ws_connection_obj *intern;
	smart_str text = {0};
	zval *val;
	int n;

	ZEND_PARSE_PARAMETERS_START(1, 1);
		Z_PARAM_ZVAL(val);
	ZEND_PARSE_PARAMETERS_END();

	printf("Conn at %p send JSON\n", getThis());
	intern = (ws_connection_obj *) Z_OBJ_P(getThis());
	php_json_encode(&text, val, PHP_JSON_UNESCAPED_UNICODE|PHP_JSON_UNESCAPED_SLASHES);
	n = php_ws_conn_write(intern, text.s);
	smart_str_free(&text);
	if (-1 == n) {
		RETURN_FALSE;
	}
	RETURN_LONG(n);
}
/* }}} */

/* {{{ proto bool WebSocket\Connection::isConnected()
	Check is connection is established */
PHP_METHOD(WS_Connection, isConnected)
{
	ws_connection_obj *intern;

	ZEND_PARSE_PARAMETERS_START(0, 0);
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_connection_obj *) Z_OBJ_P(getThis());

	RETURN_BOOL(intern->connected);
}
/* }}} */

/* {{{ proto int WebSocket\Connection::getUid()
	Get connection unique ID */
PHP_METHOD(WS_Connection, getUid)
{
	ws_connection_obj *intern;

	ZEND_PARSE_PARAMETERS_START(0, 0);
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_connection_obj *) Z_OBJ_P(getThis());

	RETURN_LONG(intern->id);
}
/* }}} */

/* {{{ proto void WebSocket\Connection::disconnect()
	Close connection to the client */
PHP_METHOD(WS_Connection, disconnect)
{
	ws_connection_obj *intern;

	ZEND_PARSE_PARAMETERS_START(0, 0);
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_connection_obj *) Z_OBJ_P(getThis());
	php_ws_conn_close(intern);
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
