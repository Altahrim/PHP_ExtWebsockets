<?php
namespace WebSocket;

class Client {
    const ON_CONNECT = null;
    const ON_DATA = null;
    const ON_DISCONNECT = null;

    /**
     * Constructor
     *
     * @param string $host
     * @param int $port
     * @param bool $useSSL
     */
    public function __construct($host = '127.0.0.1', $port = 8080, $useSSL = false) {}

    /**
     * Register events
     *
     * @param int $event self::ON_*
     * @param callable $callback
     */
    public function on(int $event, callable $callback) {}

    /**
     * Init connection to the WebSocket server
     */
    public function connect() {}

    /**
     * Send text to remote server
     *
     * @param string $str Text
     */
    public function send($str) {}

    /**
     * Send JSON to remote server
     *
     * @param mixed $payload Raw payload
     */
    public function sendAsJson($payload) {}

    public function isConnected() {}
    public function disconnect() {}
}
