<?php
namespace WebSocket;

interface EventLoopInterface {
    /**
     * Read event
     * @var int
     */
    const EVENT_READ = 0;

    /**
     * Write event
     * @var int
     */
    const EVENT_WRITE = 0;

    /**
     * Add a new socket to watchlist
     * @param resource $fd
     * @param int $flags
     * @return bool
     */
    public function add($fd, int $flags);

    /**
     * Removes a socket from watchlist
     * @param resource $fd
     * @return bool
     */
    public function delete($fd);

    /**
     * Changes mode for a specified socket
     * @param resource $fd
     * @param int $flags
     * @return bool
     */
    public function setMode($fd, int $flags);

    /**
     * Lock pool
     * @return bool
     */
    public function lock();

    /**
     * Unlock pool
     * @return bool
     */
    public function unlock();
}
