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
#include "ws_libwebsockets.h"
#include "ws_eventloop.h"
#include "zend_interfaces.h"

ZEND_EXTERN_MODULE_GLOBALS(websocket)

static void get_token_as_array(zval *arr, struct lws *wsi)
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

int callback_ext_php(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	ws_server_obj *intern = WEBSOCKET_G(intern);
	zval *this = WEBSOCKET_G(php_obj);
	ws_connection_obj * wsconn;
	zval *connection = user;
	zval retval;
	int n, return_code = 0;
	zend_string *text;
	struct lws_pollargs *pa = in;
	ws_callback *user_cb;

	switch (reason) {
		/** External event loop **/
		case LWS_CALLBACK_ADD_POLL_FD:
			if (intern->eventloop) {
				return_code = invoke_eventloop_cb(intern, "add", pa->fd, pa->events);
			}
			break;

		case LWS_CALLBACK_DEL_POLL_FD:
			if (intern->eventloop) {
				return_code = invoke_eventloop_cb(intern, "delete", pa->fd, -1);
			}
			break;

		case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
			if (intern->eventloop) {
				return_code = invoke_eventloop_cb(intern, "setmode", pa->fd, pa->events);
			}
			break;

		case LWS_CALLBACK_LOCK_POLL:
			if (intern->eventloop) {
				return_code = invoke_eventloop_cb(intern, "lock", 0, -1);
			}
			break;

		case LWS_CALLBACK_UNLOCK_POLL:
			if (intern->eventloop) {
				return_code = invoke_eventloop_cb(intern, "unlock", 0, -1);
			}
			break;
		/** End external event loop **/

		case LWS_CALLBACK_WSI_CREATE:
			printf("WSI create\n");
			break;

		case LWS_CALLBACK_WSI_DESTROY:
			printf("Destroy WSI\n");
			break;

		case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
			/*
			printf("Filter Protocol Connection\n");
			if (intern->cb_filter_headers->fci) {

				ZVAL_FALSE(&retval);
				zval params[2];
				get_token_as_array(&params[0], wsi);
				ZVAL_COPY_VALUE(&params[1], this);
				intern->cb_filter_headers->fci.param_count = 2;
				intern->cb_filter_headers->fci.params = params;
				intern->cb_filter_headers->fci.retval = &retval;
				if (FAILURE == zend_call_function(&intern->cb_filter_headers->fci, &intern->cb_filter_headers->fcc)) {
					php_error_docref(NULL, E_WARNING, "Unable to call filter headers callback");
				}

				if (!zval_is_true(&retval)) {
					return_code = -1;
				}

				zval_dtor(&retval);
			}
			*/
			break;

		case LWS_CALLBACK_ESTABLISHED:
			object_init_ex(connection, ws_connection_ce);
			wsconn = (ws_connection_obj *) Z_OBJ_P(connection);
			wsconn->id = ++intern->next_id;
			wsconn->context = WEBSOCKET_G(context);
			printf("Accept connection %lu\n", wsconn->id);
			wsconn->wsi = wsi;

			zval_addref_p(connection);
			add_index_zval(&intern->connections, wsconn->id, connection);

			wsconn->connected = 1;
			if (intern->callbacks[PHP_CB_ACCEPT]) {
				user_cb = intern->callbacks[PHP_CB_ACCEPT];
				ZVAL_NULL(&retval);
				zval params[2];
				ZVAL_COPY_VALUE(&params[0], this);
				ZVAL_COPY_VALUE(&params[1], connection);
				user_cb->fci.param_count = 2;
				user_cb->fci.params = params;
				user_cb->fci.retval = &retval;

				printf("Ready to call accept handler\n");
				if (FAILURE == zend_call_function(&user_cb->fci, &user_cb->fcc)) {
					printf("Accept handler fail\n");
					php_error_docref(NULL, E_WARNING, "Unable to call accept callback");
				}
				printf("Accept handler OK\n");

				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_RECEIVE:
			printf("Receive data.\n");
			if (intern->callbacks[PHP_CB_DATA]) {
				user_cb = intern->callbacks[PHP_CB_DATA];
				ZVAL_NULL(&retval);
				zval params[3];
				ZVAL_COPY_VALUE(&params[0], this);
				ZVAL_COPY_VALUE(&params[1], connection);
				ZVAL_STRINGL(&params[2], in, len);
				user_cb->fci.param_count = 3;
				user_cb->fci.params = params;
				user_cb->fci.retval = &retval;
				if (FAILURE == zend_call_function(&user_cb->fci, &user_cb->fcc)) {
					php_error_docref(NULL, E_WARNING, "Unable to call data callback");
				}

				// Test if different from true (or assimilated), close connection
				if (!zval_is_true(&retval)) {
					wsconn = (ws_connection_obj *) Z_OBJ_P(connection);
					wsconn->connected = 0;
					return_code = -1;
				}

				zval_dtor(&retval);
			}
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:
			wsconn = (ws_connection_obj *) Z_OBJ_P(connection);
			while (wsconn->read_ptr != wsconn->write_ptr) {
				text = wsconn->buf[wsconn->read_ptr];
				n = lws_write(wsi, text->val, text->len, LWS_WRITE_TEXT);

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
			wsconn = (ws_connection_obj *) Z_OBJ_P(connection);
			wsconn->connected = 0;
			printf("Close %lu.\n", wsconn->id);

			// Drop from active connections
			zend_hash_index_del(Z_ARRVAL(intern->connections), wsconn->id);
			zval_delref_p(connection);

			if (intern->callbacks[PHP_CB_CLOSE]) {
				user_cb = intern->callbacks[PHP_CB_CLOSE];
				ZVAL_NULL(&retval);
				zval params[2];
				ZVAL_COPY_VALUE(&params[0], this);
				ZVAL_COPY_VALUE(&params[1], connection);
				user_cb->fci.param_count = 2;
				user_cb->fci.params = params;
				user_cb->fci.retval = &retval;
				if (FAILURE == zend_call_function(&user_cb->fci, &user_cb->fcc)) {
					php_error_docref(NULL, E_WARNING, "Unable to call close callback");
				}
				zval_dtor(&retval);
			}
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

		case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
		case LWS_CALLBACK_PROTOCOL_INIT:
		case LWS_CALLBACK_PROTOCOL_DESTROY:
		case LWS_CALLBACK_GET_THREAD_ID:
			// Ignore
			break;

		default:
			printf(" – CB PHP Non-handled action (reason: %d)\n", reason);
	}

	return intern->exit_request == 1 ? -1 : return_code;
}

void php_ws_conn_close(ws_connection_obj *conn) {
	unsigned char payload[4] = "Bye";

	conn->connected = 0;
	printf("Send close to %lu\n", conn->id);
	lws_write(conn->wsi, payload, strlen(payload), LWS_WRITE_CLOSE);
}

int php_ws_conn_write(ws_connection_obj *conn, zend_string *text) {
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

	lws_callback_on_writable(conn->wsi);

	return text->len;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
