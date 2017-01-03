<?php
namespace WebSocket;

class Connection {
    /**
     * No direct construction
     */
    final private function __construct() {}

    /**
     * Close current connection
     */
    public function disconnect() {}

    /**
     * Send data to remote client
     *
     * @param string $str Text
     */
    public function send($str) {}

    /**
     * Send data to remote client as JSON string
     *
     * @param mixed $payload Raw payload
     */
    public function sendAsJson($payload) {}

    /**
     * Return connection UID
     * @return string
     */
    public function getUid():string {
        return '';
    }
}
