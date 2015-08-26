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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php_websocket.h"
#include "ws_server.h"
#include "ws_connection.h"
#include "ws_libwebsockets.h"

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

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(websocket)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/

	register_ws_server_class(TSRMLS_CC);
	register_ws_connection_class(TSRMLS_CC);

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
