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

#ifndef WEBSOCKET_CONSTANTS_H
#define WEBSOCKET_CONSTANTS_H

/**
 * Frequency in Hertz
 */
#define PHP_WEBSOCKET_FREQUENCY 0.2

/**
 * Minimal WebSocket version supported by this extension
 */
#define WEBSOCKET_MIN_VERSION 13

/**
 * Magic number used by WebSockets to accept incoming connections
 */
#define WEBSOCKET_MAGIC_NUMBER "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

/**
 * Number of unsent messages to keep in connection object
 */
#define WEBSOCKET_CONNECTION_BUFFER_SIZE 30

#endif /* WEBSOCKET_CONSTANTS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
