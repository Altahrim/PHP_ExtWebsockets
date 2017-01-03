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
#include "ws_client.h"
#include <ext/json/php_json.h>
#include <zend_smart_str.h>

ZEND_EXTERN_MODULE_GLOBALS(websocket)

/***** Class \WebSocket\Client *****/

/*--- Definitions ---*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_client___construct, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_ws_client_on, 0, 2, IS_TRUE|IS_FALSE, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, event, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, callback, IS_CALLABLE, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_client_connect, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_client_send, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, text, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_client_sendAsJson, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, payload, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_client_is_connected, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_client_disconnect, 0, 0, 0)
ZEND_END_ARG_INFO()

const zend_function_entry obj_ws_client_funcs[] = {
	PHP_ME(WS_Client, __construct, arginfo_ws_client___construct, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Client, on, arginfo_ws_client_on, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Client, connect, arginfo_ws_client_connect, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Client, send, arginfo_ws_client_send, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Client, sendAsJson, arginfo_ws_client_sendAsJson, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Client, isConnected, arginfo_ws_client_is_connected, ZEND_ACC_PUBLIC)
	PHP_ME(WS_Client, disconnect, arginfo_ws_client_disconnect, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/*--- Registration ---*/

void register_ws_client_class(TSRMLS_DC)
{
	zend_class_entry tmp_ws_client_ce;

	INIT_NS_CLASS_ENTRY(tmp_ws_client_ce, "WebSocket", "Client", obj_ws_client_funcs);
	ws_client_ce = zend_register_internal_class(&tmp_ws_client_ce TSRMLS_CC);
	ws_client_ce->create_object = ws_client_create_object_handler;
	memcpy(&ws_client_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	ws_client_object_handlers.free_obj = (zend_object_free_obj_t) ws_client_free_object_storage_handler;
}

/*--- Handlers ---*/

zend_object* ws_client_create_object_handler(zend_class_entry *ce TSRMLS_DC)
{
	int i;
	ws_client_obj *intern = emalloc(sizeof(ws_client_obj));
	memset(intern, 0, sizeof(ws_client_obj));

	zend_object_std_init(&intern->std, ce TSRMLS_CC);
	object_properties_init(&intern->std, ce);
	intern->std.handlers = &ws_client_object_handlers;

	return &intern->std;
}

void ws_client_free_object_storage_handler(ws_client_obj *intern TSRMLS_DC)
{
	int i;

	for (i = 0; i < PHP_CB_CLIENT_COUNT; ++i) {
		if (NULL != intern->callbacks[i]) {
		    Z_DELREF(intern->callbacks[i]->fci->function_name);
			efree(intern->callbacks[i]->fci);
			efree(intern->callbacks[i]->fcc);
			efree(intern->callbacks[i]);
		}
	}

	zend_object_std_dtor(&intern->std TSRMLS_CC);
	efree(intern);
}

/*--- Methods ---*/

/* {{{ proto void WebSocket\Client::__construct()
	Constructor */
PHP_METHOD(WS_Client, __construct)
{
	ws_client_obj *intern;
	zend_string *host;
	long port = 8080;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_STR(host)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(port)
	ZEND_PARSE_PARAMETERS_END();
}
/* }}} */

/* {{{ proto bool WebSocket\Client::on(int event, callback callback)
	Register a callback for specified event */
PHP_METHOD(WS_Client, on)
{
	ws_client_obj *intern;
    zend_fcall_info fci = empty_fcall_info;
    zend_fcall_info_cache fcc = empty_fcall_info_cache;
	long event;

	ZEND_PARSE_PARAMETERS_START(2, 2)
		Z_PARAM_LONG(event)
		Z_PARAM_FUNC_EX(fci, fcc, 1, 0)
	ZEND_PARSE_PARAMETERS_END();

	if (event < 0 || event >= PHP_CB_CLIENT_COUNT || !ZEND_FCI_INITIALIZED(fci)) {
		php_error_docref(NULL, E_WARNING, "Try to add an invalid callback");
		RETURN_FALSE;
	}

	intern = (ws_client_obj *) Z_OBJ_P(getThis());
	intern->callbacks[event] = emalloc(sizeof(ws_callback));
	intern->callbacks[event]->fci = emalloc(sizeof(zend_fcall_info));
	intern->callbacks[event]->fcc = emalloc(sizeof(zend_fcall_info_cache));

	memcpy(intern->callbacks[event]->fci, &fci, sizeof(zend_fcall_info));
	memcpy(intern->callbacks[event]->fcc, &fcc, sizeof(zend_fcall_info_cache));

    Z_ADDREF(intern->callbacks[event]->fci->function_name);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto void WebSocket\Connection::connect()
	Establish connection with remote server */
PHP_METHOD(WS_Client, connect)
{
	ws_client_obj *intern;

	ZEND_PARSE_PARAMETERS_START(0, 0);
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_client_obj *) Z_OBJ_P(getThis());

	// FIXME Implement
	RETURN_FALSE
}
/* }}} */

/* {{{ proto int|false WebSocket\Connection::send(string text)
	Send data to the client */
PHP_METHOD(WS_Client, send)
{
	ws_client_obj *intern;
	zend_string *text;
	int n;

	ZEND_PARSE_PARAMETERS_START(1, 1);
		Z_PARAM_STR(text)
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_client_obj *) Z_OBJ_P(getThis());
	// FIXME Implement
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto int|false WebSocket\Connection::sendAsJson(mixed payload)
	Send data to the client as JSON string */
PHP_METHOD(WS_Client, sendAsJson)
{
	ws_client_obj *intern;
	smart_str text = {0};
	zval *val;
	int n;

	ZEND_PARSE_PARAMETERS_START(1, 1);
		Z_PARAM_ZVAL(val);
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_client_obj *) Z_OBJ_P(getThis());
	php_json_encode(&text, val, PHP_JSON_UNESCAPED_UNICODE|PHP_JSON_UNESCAPED_SLASHES);
	// FIXME Implement
	smart_str_free(&text);
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool WebSocket\Connection::isConnected()
	Check is connection is established */
PHP_METHOD(WS_Client, isConnected)
{
	ws_client_obj *intern;

	ZEND_PARSE_PARAMETERS_START(0, 0);
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_client_obj *) Z_OBJ_P(getThis());

	// FIXME Implement
	RETURN_FALSE
}
/* }}} */

/* {{{ proto void WebSocket\Connection::disconnect()
	Close connection to the client */
PHP_METHOD(WS_Client, disconnect)
{
	ws_client_obj *intern;
	zend_string *reason;

	ZEND_PARSE_PARAMETERS_START(0, 1);
		Z_PARAM_OPTIONAL
		Z_PARAM_STR(reason)
	ZEND_PARSE_PARAMETERS_END();

	intern = (ws_client_obj *) Z_OBJ_P(getThis());
	// FIXME Implement
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
