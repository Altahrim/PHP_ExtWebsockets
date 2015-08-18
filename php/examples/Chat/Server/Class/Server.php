<?php
namespace WS;

class Server
{
    /**
     * WebSocket server
     * @var \WebSocket\Server
     */
    private $server;

    /**
     * Connections
     * @var \WebSocket\Connection[]
     */
    private $connections;

    /**
     * Topic
     * @var string
     */
    private $topic = 'Ceci n\'est pas un chat';

    /**
     * History
     * @var Array
     */
    private $history = [];

    /**
     * Smileys (builded at construct)
     * @var array
     */
    private $smileys = [
        '☺' => [':)', ':-)', '=)'],
        '☹' => [':(', ':-(', '=(']
    ];

    /**
     * Constructor
     */
    public function __construct()
    {
        $this->server      = new \WebSocket\Server(8081);
        $this->connections = [];
        $this->server->onClientAccept([$this, 'onAccept']);
        $this->server->onClose([$this, 'onClose']);
        $this->server->onClientData([$this, 'onData']);

        // Build smileys array
        $smileys = [];
        foreach ($this->smileys as $smiley => $strings) {
            foreach ($strings as $str) {
                $smiley[$str] = $smiley;
            }
        }
        $this->smileys = $smileys;
    }

    public function onAccept(\WebSocket\Server $serv, \WebSocket\Connection $conn) {
        $time = time();
        echo "New client accepted\n";

        $newUser = new \WS\User($conn, 'User ' . $conn->getUid());

        $list = [];
        foreach ($this->connections as $uid => $user) {
            $list[] = [
                'uid'      => $uid,
                'username' => $user->getUsername()
            ];
        }

        // Send walcome message
        $msg = $this->buildMessage('welcome', [
            'uid'      => $conn->getUid(),
            'topic'    => $this->topic,
            'username' => $newUser->getUsername(),
            'users'    => $list,
            'history'  => $this->history
        ], $time, false);
        $conn->sendJson($msg);

        // Introduce to others
        $msg = $this->buildMessage('connect', [
            'uid'      => $conn->getUid(),
            'username' => 'User ' . $conn->getUid(),
        ], $time);
        $serv->broadcast($msg, $conn->getUid());

        $this->connections[$conn->getUid()] = $newUser;
    }

    public function onClose(\WebSocket\Server $serv, \WebSocket\Connection $conn) {
        $time = time();

        $this->connections[$conn->getUid()]->unsetUsername();
        unset($this->connections[$conn->getUid()]);
        $msg = $this->buildMessage('disconnect', [
            'uid' => $conn->getUid()
        ], $time);
        $serv->broadcast($msg);
    }

    public function onData(\WebSocket\Server $serv, \WebSocket\Connection $conn, $message) {
        $time = time();
        echo "Got message! \"$message\"\n";

        $continue = true;

        $handled = false;
        if ($message[0] === '/') {
            $handled = true;
            $cmd = explode (' ', $message, 2);
            switch ($cmd[0]) {
                case '/shutdown':
                    $serv->stop();
                    break;
                case '/quit':
                    $continue = false;
                    break;
                case '/topic':
                    if (!empty($cmd[1])) {
                        $this->topic = trim(htmlspecialchars($cmd[1]));
                        $msg = $this->buildMessage('topic', [
                            'uid'   => $conn->getUid(),
                            'topic' => $this->topic
                        ], $time, true);
                        $serv->broadcast($msg);
                    }
                    break;
                case '/help':
                    $msg = $this->buildMessage('event', [
                        'text' => 'Visiblement, ' . $this->connections[$conn->getUid()]->getUsername() . ' voudrait de l\aide… Quelqu\'un peut faire quelque chose ?'
                    ], $time, true);
                    $serv->broadcast($msg);
                    break;
                case '/nick':
                    $user = $this->connections[$conn->getUid()];
                    $user->setUsername($cmd[1]);
                    $msg = $this->buildMessage('nick', [
                        'uid' => $conn->getUid(),
                        'username' => $user->getUsername()
                    ], $time, true);
                    $serv->broadcast($msg);
                    break;
                case '/kick':
                    $msg = $this->buildMessage('alert', [
                        'result' => 'D\'où tu tentes de kicker les gens toi ? Tu te crois admin ?'
                    ], $time, false);
                    $conn->sendJson($msg);
                    $conn->disconnect();
                    break;
                case '/me':
                    $message = $this->connections[$conn->getUid()]->getUsername() . ' ' . htmlspecialchars($cmd[1]);
                    $msg = $this->buildMessage('event', [
                        'text' => $message,
                    ], $time, true);
                    $serv->broadcast($msg);
                    $this->history[] = [$time, $message];
                    break;
                case '/quote':
                case '/code':
                    $handled = false;
                    break;
                default:
                    $msg = $this->buildMessage('alert', [
                        'result' => 'Commande inconnue'
                    ], $time, false);
                    $conn->sendJson($msg);
            }
        }

        // Seems to be a normal message
        if (true !== $handled) {
            $message = strip_tags($message);

            if (!empty(trim($message))) {
                // Special /quote
                $parts   = explode('/quote', $message);
                $message = array_shift($parts);
                foreach ($parts as $part) {
                    $message .= '<quote>' . trim($part) . '</quote>';
                }

                // Special /code
                $parts   = explode('/code', $message);
                $message = array_shift($parts);
                foreach ($parts as $part) {
                    $class = '';
                    if (preg_match('/-([0-9a-z])/i', $part, $matches)) {
                        $language = ' class="language-' . strtolower($matches[1]) . '"';
                        $part     = substr($part, strlen($language) + 1);
                        if ($language === 'php') {
                            $part = highlight_string($part, true);
                        }
                    }
                    $message .= '<code' . $class . '>' . trim($part) . '</code>';
                }

                // Smileys
                $message = strtr($message, $this->smileys);

                $message = [
                    'uid' => $conn->getUid(),
                    'msg' => nl2br($message)
                ];
                $this->history[] = [$time, $message, $this->connections[$conn->getUid()]->getUsername()];
                $msg = $this->buildMessage('msg', $message, $time);
                $serv->broadcast($msg);
            }
        }

        return $continue;
    }

    public function run()
    {
        $this->server->run();
    }

    public function stop()
    {
        $this->server->stop();
    }

    /**
     * Build a message
     *
     * @param string $operation
     * @param array $data
     * @param string $time
     * @return string JSON string
     */
    protected function buildMessage($operation, array $data = [], $time = null, $jsonEncode = true) {
        $time = null === $time ? microtime(true) : (int) $time;
        $msg  = [
            'op'   => trim($operation),
            'time' => $time,
            'data' => $data
        ];
        return $jsonEncode ? json_encode($msg, \JSON_UNESCAPED_SLASHES|\JSON_UNESCAPED_UNICODE) : $msg;
    }
}
