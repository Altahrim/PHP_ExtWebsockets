<?php
namespace WS;

/**
 * External event loop
 * A connection will be terminated if returning false in one of these functions
 */
class EventLoop implements \WebSocket\EventLoopInterface
{
    /**
     * WebSocket server
     * @var \WS\Server
     */
    private $server;

    /**
     * Watched sockets
     * @var \EvIo[]
     */
    private $socketsHandlers = [];

    /**
     * Constructor
     * @param \WS\Server $server
     */
    public function __construct(\WS\Server $server)
    {
        $this->server = $server;
    }


    /**
     * Convert event flags from extension to \Ev
     * @param int $flags
     * @return int
     */
    public static function flagsToEv(int $flags) : int
    {
        $res = 0;
        if ($flags & self::EVENT_READ) {
            $res |= \Ev::READ;
        }
        if ($flags & self::EVENT_WRITE) {
            $res |= \Ev::WRITE;
        }

        return $res;
    }

    /**
     * Convert event flags from \Ev to extension
     * @param int $flags
     * @return int
     */
    public static function flagsToExt(int $flags) : int
    {
        $res = 0;
        if ($flags & \Ev::READ) {
            $res |= self::EVENT_READ;
        }
        if ($flags & \Ev::WRITE) {
            $res |= self::EVENT_WRITE;
        }

        return $res;
    }


    /**
     * Add a new socket to watch
     * @param resource $fd
     * @param int $flags
     * @return bool
     */
    public function add($fd, int $flags)
    {
        $server = $this->server->getWsServer();
        $flags = self::flagsToEv($flags);
        $this->socketsHandlers[(int) $fd] = new \EvIo($fd, $flags, function(\EvIo $evIo, $flags) use ($server) {
            $flags = \WS\EventLoop::flagsToExt($flags);
            $server->serviceFd($evIo->fd, $flags);
        });
        return true;
    }

    /**
     * Remove a socket from watch list
     * @param resource $fd
     * @return bool
     */
    public function delete($fd)
    {
        $this->socketsHandlers[(int) $fd]->stop();
        unset($this->socketsHandlers[(int) $fd]);
        return true;
    }

    /**
     * Set new events for a socket
     * @param resource $fd
     * @param int $flags
     * @return bool
     */
    public function setMode($fd, int $flags)
    {
        $flags = self::flagsToEv($flags);
        $this->socketsHandlers[(int) $fd]->set($fd, $flags);
        return true;
    }

    /**
     * Lock pool
     * @return bool
     */
    public function lock()
    {
        // Lock pool
        return true;
    }

    /**
     * Unlock pool
     * @return bool
     */
    public function unlock()
    {
        // Unlock pool
        return true;
    }
}
