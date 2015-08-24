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
    private $topic = 'Change the topic with /topic <your_topic>';

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
                $smileys[$str] = $smiley;
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
                        'text' => $this->connections[$conn->getUid()]->getUsername() . ' would like some help but as I am too lazy to implement it, can someone help him?'
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
                        'result' => 'Hey! Why do you try to kick people?'
                    ], $time, false);
                    $conn->sendJson($msg);
                    break;
                case '/me':
                    $message = $this->connections[$conn->getUid()]->getUsername() . ' ' . htmlspecialchars($cmd[1]);
                    $msg = $this->buildMessage('event', [
                        'text' => $message,
                    ], $time, true);
                    $serv->broadcast($msg);
                    $this->history[] = [$time, $message];
                    break;
                default:
                    $handled = false;
                    break;
            }
        }

        // Seems to be a normal message
        if (true !== $handled) {
            if (strlen(trim($message)) >= 1) {
                $message = [
                    'uid' => $conn->getUid(),
                    'msg' => $this->formatMessage($message)
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

    /**
     * Handle basic formatting for message
     *
     * @param string $message
     * @return string
     */
    protected function formatMessage($message) {
        $parts = preg_split('#(?:(/quote|/code-?)|(/code-)([a-z0-9]+))\s+#ui', $message, null, PREG_SPLIT_DELIM_CAPTURE|PREG_SPLIT_NO_EMPTY);
        $context  = null;
        $message  = '';
        $language = '';
        while (null !== $part = array_shift($parts)) {
            switch ($part) {
                case '/code':
                case '/code-':
                    if (!empty($context)) {
                        $message .= '</' . $context . '>';
                    }
                    $context  = 'code';
                    if ('/code-' === $part) {
                        $language = mb_strtolower($this->escape(array_shift($parts)));
                        $message .= '<code class="language-' . $language . '">';
                    } else {
                        $message .= '<code>';
                    }
                    break;
                case '/quote':
                    if (!empty($context)) {
                        $message .= '</' . $context . '>';
                    }
                    $message .= '<blockquote>';
                    $context = 'blockquote';
                    break;
                default:
                    switch ($context) {
                        case 'code':
                            $part = $this->highlight($part, $language);
                            break;
                        default:
                            $part = strip_tags($part);
                            $part = strtr($part, $this->smileys);
                            $part = $this->replaceColors($part, true);
                            $part = $this->replaceLinks($part, true);
                    }
                    $message .= $part;
                    break;
            }
        }

        if (!empty($context)) {
            $message .= '</' . $context . '>';
        }

        return $message;
    }

    /**
     * Escape a string in oprder to safely display it
     *
     * @param string $str
     * @return string
     */
    protected function escape($str) {
        return htmlspecialchars($str, ENT_HTML5, 'UTF-8');
    }

    /**
     * Highlight source for <code>
     * Only handle PHP syntax (for now)
     *
     * @param string $str
     * @param string $language
     * @return string
     */
    protected function highlight($str, $language) {
        if ('php' === $language) {
            if (false === strpos($str, '<?php') && false === strpos($str, '<?=')) {
                $str = "<?php\n" . $str;
            }
            return str_replace(['<code>', '</code>'], '', highlight_string($str, true));
        }

        return $this->escape(nl2br($str));
    }

    /**
     * Detect and replace link in $str
     *
     * @param string $str
     * @param bool $displayImages If true, images will be displayed
     */
    protected function replaceLinks($str) {
        $cb = function ($matches) {
            $url = $matches[0];

            // Detect Youtube
            if (preg_match('#^https?://www\.youtube\.com/watch\?(?:.*?&)?v=([^&]+)#ui', $url, $matches)) {
                return '<div class="embed"><iframe width="560" height="315" src="https://www.youtube.com/embed/' . $this->escape($matches[1]) . '" frameborder="0" allowfullscreen></iframe></div>';
            }

            $url = $this->escape($url);
            $isImage = null;
            // Detect image from extention
            $ext = explode('?', $url, 2)[0];
            $ext = explode('.', $ext);
            $ext = array_pop($ext);
            if (in_array($ext, ['jpg', 'jpeg', 'gif', 'png'], true)) {
                $isImage = true;
            }

            // Detect image from Content-Type
            if (null === $isImage) {
                $ch = curl_init($url);
                curl_setopt_array($ch, [
                    CURLOPT_NOBODY         => true,
                    CURLOPT_FOLLOWLOCATION => true,
                    CURLOPT_HEADER         => true,
                    CURLOPT_RETURNTRANSFER => true
                ]);
                $res = curl_exec($ch);
                $content = curl_getinfo($ch, CURLINFO_CONTENT_TYPE);
                if (in_array($ext, ['image/jpg', 'image/gif', 'image/png'], true)) {
                    $isImage = true;
                }
            }

            return $isImage
                ? '<div class="embed"><a href="' . $url . '"><img src="' . $url . '" alt="' . $url . '"></a></div>'
                : '<a href="' . $url . '">' . $url . '</a>';
        };

        return preg_replace_callback('#https?://[^\s]+#ui', $cb, $str);
    }

    /**
     * Show a color when hexadecimal code is detected
     *
     * @param string $str
     * @return string
     */
    protected function replaceColors($str) {
        return preg_replace('@#[A-F0-9]{6}|#[A-F0-9]{3}@ui', '\\0<span class="color" style="background:\\0"></span>', $str);
    }
}
