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

#ifndef PHP_WEBSOCKET_H
#define PHP_WEBSOCKET_H

extern zend_module_entry websocket_module_entry;
#define phpext_websocket_ptr &websocket_module_entry

#define PHP_WEBSOCKET_VERSION "0.1.0"
#define PHP_WEBSOCKET_FREQUENCY 0.2 // Frequency in Hertz

#ifdef PHP_WIN32
#	define PHP_WEBSOCKET_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_WEBSOCKET_API __attribute__ ((visibility("default")))
#else
#	define PHP_WEBSOCKET_API
#endif


#include "libwebsockets.h"
#include "ws_constants.h"

#ifdef ZTS
#include "TSRM.h"
#endif

#define PHP_METHOD_WS_CALLBACK(wsObj, callbackName, callbackStorage) 						\
PHP_METHOD(wsObj, callbackName)																\
{																							\
	printf("Add callback %s (%s)\n", #callbackName, #callbackStorage);						\
	ws_server_obj *intern;																	\
	zval *cb;																				\
																							\
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &cb) == FAILURE) {			\
		return;																				\
	}																						\
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

/*
	Declare any global variables you may need between the BEGIN
	and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(websocket)
	zend_long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(websocket)
*/

#ifdef ZTS
#define WEBSOCKET_G(v) ZEND_TSRMG(websocket_globals_id, zend_websocket_globals *, v)
#ifdef COMPILE_DL_WEBSOCKET
ZEND_TSRMLS_CACHE_EXTERN();
#endif
#else
#define WEBSOCKET_G(v) (websocket_globals.v)
#endif

// WebSocket\Server
zend_class_entry *ws_server_ce;
static zend_object_handlers ws_server_object_handlers;

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


// WebSocket\Connection
zend_class_entry *ws_connection_ce;
static zend_object_handlers ws_connection_object_handlers;

typedef struct _ws_connection_obj {
	zend_object std;

	// ID (unique on server)
	zend_ulong id;

	// Connection state (Connected/Disconnected)
	zend_bool connected;

	// Write buffer
	struct libwebsocket_context *context;
	zend_string *buf[WEBSOCKET_CONNECTION_BUFFER_SIZE];
	unsigned int read_ptr;
	unsigned int write_ptr;

	// LibWebSockets context
	struct libwebsocket *wsi;
} ws_connection_obj;


#endif /* PHP_WEBSOCKET_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
