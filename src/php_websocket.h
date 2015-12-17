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

#ifndef PHP_WEBSOCKET_H
#define PHP_WEBSOCKET_H

#include <php.h>
#include <libwebsockets.h>
#include "ws_constants.h"
#include "ws_server.h"

extern zend_module_entry websocket_module_entry;
#define phpext_websocket_ptr &websocket_module_entry

#define PHP_WEBSOCKET_VERSION "0.1.0"

#ifdef PHP_WIN32
#	define PHP_WEBSOCKET_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_WEBSOCKET_API __attribute__ ((visibility("default")))
#else
#	define PHP_WEBSOCKET_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(websocket)
	zval* php_obj;
	ws_server_obj *intern;
	struct lws_context * context;
ZEND_END_MODULE_GLOBALS(websocket)

#ifdef ZTS
#define WEBSOCKET_G(v) ZEND_TSRMG(websocket_globals_id, zend_websocket_globals *, v)
#ifdef COMPILE_DL_WEBSOCKET
ZEND_TSRMLS_CACHE_EXTERN();
#endif
#else
#define WEBSOCKET_G(v) (websocket_globals.v)
#endif

#endif /* PHP_WEBSOCKET_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
