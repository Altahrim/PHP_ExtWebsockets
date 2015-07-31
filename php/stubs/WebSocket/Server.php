<?php
namespace WebSocket;

class Server {
    /**
     * Constructor
     *
     * @param int $port
     * @access public
     */
    public function __construct($port = 8080) {}

    /**
     *
     * @param callable $callback
     */
    public function onClientAccept(callable $callback) {}

    /**
     *
     * @param callable $callback
     */
    public function onClose(callable $callback) {}

    /**
     *
     * @param callable $callback
     */
    public function onClientError(callable $callback) {}

    /**
     *
     * @param callable $callback
     */
    public function onClientMessage(callable $callback) {}

    /**
     *
     * @param callable $callback
     */
    public function onTick(callable $callback) {}

    /**
     *
     * @param callable $callback
     */
    public function onFilterHeaders(callable $callback) {}

    /**
     *
     * @param callable $callback
     */
    public function onUserSignal(callable $callback) {}

    /**
     *
     * @return string|null
     */
    //public function getLastUrl() {}

    /**
     *
     * @return array|null
     */
    //public function getLastHeaders() {}

    /**
     * Launch websocket server
     */
    public function run() {}

    /**
     * Ask server to quit on next tick
     */
    public function stop() {}

    /**
     * Broadcast a text to all connected clients
     *
     * @param string $text
     * @access public
     */
    public function broadcast($text) {}
}
