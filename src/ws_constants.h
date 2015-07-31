/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Gaussorgues Benjamin                                         |
  +----------------------------------------------------------------------+
*/

#ifndef PHP_WEBSOCKET_CONSTANTS_H
#define PHP_WEBSOCKET_CONSTANTS_H

#define OPCODE_CONTINUATION 0x0
#define OPCODE_TEXT         0x1
#define OPCODE_BINARY       0x2
#define OPCODE_CLOSE        0x8
#define OPCODE_PING         0x9
#define OPCODE_PONG         0xA

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

#endif /* PHP_WEBSOCKET_CONSTANTS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
