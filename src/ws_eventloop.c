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

#include <zend_API.h>
#include <zend_types.h>
#include "ws_eventloop.h"
#include <libwebsockets.h>

/***** Class \WebSocket\EventLoopInterface *****/

/*--- Definitions ---*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_eventloop_add, 0, 0, 2)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_eventloop_delete, 0, 0, 1)
	ZEND_ARG_INFO(0, fd)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_eventloop_setMode, 0, 0, 2)
	ZEND_ARG_INFO(0, fd)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_eventloop_lock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ws_eventloop_unlock, 0, 0, 0)
ZEND_END_ARG_INFO()

const zend_function_entry obj_ws_eventloop_funcs[] = {
	PHP_ABSTRACT_ME(WS_EventLoop, add, arginfo_ws_eventloop_add)
	PHP_ABSTRACT_ME(WS_EventLoop, delete, arginfo_ws_eventloop_delete)
	PHP_ABSTRACT_ME(WS_EventLoop, setMode, arginfo_ws_eventloop_setMode)
	PHP_ABSTRACT_ME(WS_EventLoop, lock, arginfo_ws_eventloop_lock)
	PHP_ABSTRACT_ME(WS_EventLoop, unlock, arginfo_ws_eventloop_unlock)
	PHP_FE_END
};

/*--- Registration ---*/

void register_ws_eventloop_class(TSRMLS_DC)
{
	zend_class_entry tmp_ws_eventloop_ce;
	INIT_NS_CLASS_ENTRY(tmp_ws_eventloop_ce, "WebSocket", "EventLoopInterface", obj_ws_eventloop_funcs);
	ws_eventloop_ce = zend_register_internal_interface(&tmp_ws_eventloop_ce TSRMLS_CC);

	zend_declare_class_constant_long(ws_eventloop_ce, "EVENT_READ", sizeof("EVENT_READ") - 1, 1<<0);
	zend_declare_class_constant_long(ws_eventloop_ce, "EVENT_WRITE", sizeof("EVENT_WRITE") - 1, 1<<2);
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
