<?php
namespace WS;

class Client
{
    /**
     * WebSocket client
     * @var \WebSocket\Client
     */
    private $client;

    /**
     * Constructor
     */
    public function __construct()
    {
        $this->client = new \WebSocket\Client('127.0.0.1', 8081);
        $this->client->on(\WebSocket\Server::ON_ACCEPT, [$this, 'onAccept']);
        $this->client->on(\WebSocket\Server::ON_CLOSE, [$this, 'onClose']);
        $this->client->on(\WebSocket\Server::ON_DATA, [$this, 'onData']);
    }

    /**
     * Handle new connection
     */
    public function onAccept() {
        echo "Connection established\n";
    }

    /**
     * Handle disconnect
     */
    public function onClose() {
        echo "Connection closed\n";
    }

    /**
     * Handle new messages
     *
     * @param string $message
     * @return bool
     */
    public function onData($message): bool {
        echo "Got message! \"$message\"\n";
        return true;
    }

    /**
     * Main loop
     *
     * @param bool $useExternalEventLoop
     */
    public function connect($useExternalEventLoop = false)
    {
        echo "Establish connection to remote server\n";
        $this->client->connect();
    }
}
