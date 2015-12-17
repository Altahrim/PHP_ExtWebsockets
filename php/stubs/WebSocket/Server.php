<?php
namespace WebSocket;

class Server {
    const ON_ACCEPT = null;
    const ON_DATA = null;
    const ON_CLOSE = null;
    const ON_TICK = null;

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
