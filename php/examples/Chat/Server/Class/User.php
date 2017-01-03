<?php
namespace WS;

class User
{
    /**
     * WebSocket connexion
     * @var \WebSocket\Connection
     */
    private $connection;

    /**
     * Username
     * @var string
     */
    private $username;

    const USERNAME_MAX_LENGTH = 50;

    /**
     * Currently used usernames
     * @var string[]
     */
    private static $usernames = [];

    /**
     * Constructor
     *
     * @param \WebSocket\Connection $connection
     * @param string $username
     */
    public function __construct(\WebSocket\Connection $connection, $username)
    {
        $this->connection = $connection;
        $this->setUsername($username);
    }

    /**
     * Return username
     *
     * @return string
     */
    public function getUsername()
    {
        return $this->username;
    }

    /**
     * Return username
     *
     * @param string $username
     * @return string
     */
    public function setUsername($username)
    {
        $this->unsetUsername();

        if (0 === strlen($username)) {
            $username = 'Nobody';
        }

        $username = preg_replace('/[\s]+/u', ' ', $username);
        $username = trim($username);
        $username = htmlspecialchars($username);
        if (mb_strlen($username) > self::USERNAME_MAX_LENGTH) {
            $username = mb_substr($username, 0, self::USERNAME_MAX_LENGTH);
            $username = rtrim($username);
        }

        $this->username = $username;

        $suffix = ' ';
        $length = 1;
        while (isset(self::$usernames[$this->username])) {
            $suffix  .= rand(0, 9);
            $this->username = mb_substr($username, 0, self::USERNAME_MAX_LENGTH - ++$length) . $suffix;
        }
        self::$usernames[$this->username] = true;

        return $this->username;
    }

    /**
     * Free an username
     */
    public function unsetUsername() {
        if (isset(self::$usernames[$this->username])) {
            unset(self::$usernames[$this->username]);
        }
    }
}