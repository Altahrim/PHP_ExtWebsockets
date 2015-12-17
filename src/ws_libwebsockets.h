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

#ifndef WS_LIBWEBSOCKETS_H
#define WS_LIBWEBSOCKETS_H

#include <php.h>
#include <libwebsockets.h>
#include "ws_constants.h"
#include "ws_server.h"
#include "ws_connection.h"


/*--- LibWebsockets protocols and callbacks ---*/

/**
 * Available protocols
 */
enum ws_protocols {
	PROTOCOL_EXTPHP = 0,

	PROTOCOLS_COUNT
};

int callback_ext_php(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

struct _ws_protocol_storage {
	zval* php_obj;
	ws_server_obj *obj;
};

static struct lws_protocols protocols[] = {
	{
		"websockets",								/* name */
		callback_ext_php,							/* callback */
		sizeof(struct _ws_protocol_storage),	/* per_session_data_size */
		0											/* max frame size / rx buffer */
	},
	{ NULL, NULL, 0, 0 }
};

/*--- Helpers ---*/

//void get_token_as_array(zval *arr, struct lws *wsi);
void php_ws_conn_close(ws_connection_obj *conn);
int php_ws_conn_write(ws_connection_obj *conn, zend_string *text);


#endif /* WS_LIBWEBSOCKETS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
