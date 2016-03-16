<?php
namespace WebSocket;

class Server {
    /**
     * Event fired when a new connection is established
     */
    const ON_ACCEPT = null;

    /**
     * Event fired when new data are received
     */
    const ON_DATA = null;

    /**
     * Event fired when a connection is closed
     */
    const ON_CLOSE = null;

    /**
     * Event fired on periodic tick
     */
    const ON_TICK = null;

    /**
     * Event fired before a connection is accepted. It allows to filter clients by headers
     */
    const ON_FILTER_HEADERS = null;

    /**
     * Event fired before a connection is accepted. It allows to filter clients by IP and port
     */
    const ON_FILTER_CONNECTION = null;

    /**
     * Constructor
     *
     * @param int $port
     * @access public
     */
    public function __construct($port = 8080) {}

    /**
     *
     * @param int $event self::ON_*
     * @param callable $callback
     */
    public function on(int $event, callable $callback) {}

    /**
     * Use external event loop
     *
     * @param \WebSocket\InterfaceEventLoop $ev
     * @return bool True if event handled by library
     */
    public function setEventLoop(\WebSocket\EventLoopInterface $ev) {}

    /**
     * Register callbacks for event-loop
     *
     * @param socket $fd
     * @param int $revents
     */
    public function serviceFd($fd, $revents) {}

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
