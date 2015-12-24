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

#ifndef WEBSOCKET_eventloop_H
#define WEBSOCKET_eventloop_H

#include <php.h>
#include "ws_constants.h"

/***** Class \WebSocket\EventLoopInterface *****/

/*--- Definition ---*/

zend_class_entry *ws_eventloop_ce;
zend_object_handlers ws_eventloop_object_handlers;

PHP_METHOD(WS_EventLoop, add);
PHP_METHOD(WS_EventLoop, delete);
PHP_METHOD(WS_EventLoop, setMode);
PHP_METHOD(WS_EventLoop, lock);
PHP_METHOD(WS_EventLoop, unlock);

/*--- Handlers ---*/

void register_ws_eventloop_class(TSRMLS_DC);

#endif /* WEBSOCKET_eventloop_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
