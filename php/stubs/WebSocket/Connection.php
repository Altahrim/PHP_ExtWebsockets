<?php
namespace WebSocket;

class Connection {
    /**
     * No direct construction
     */
    final private function __construct() {}

    /**
     * Close current connection
     *
     * @return void
     */
    public function disconnect() {}

    /**
     * Send text to remote client
     *
     * @param string $str Text
     * @return void
     */
    public function sendText($str) {}

    /**
     * Send JSON to remote client
     *
     * @param mixed $payload Unencoded payload
     * @return void
     */
    public function sendJson($payload) {}



    public function sendBinary($payload) {}
    public function sendFile($file) {}
}
