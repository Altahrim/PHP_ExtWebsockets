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

    private static $usernames = [];

    /**
     * Constructor
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
     * @return string
     */
    public function setUsername($username)
    {
        $this->unsetUsername();

        $username = preg_replace('/[\s]+/u', ' ', $username);
        $username = trim($username);
        $username = htmlspecialchars($username);
        if (mb_strlen($username) > self::USERNAME_MAX_LENGTH) {
            $username = mb_substr($username, 0, self::USERNAME_MAX_LENGTH);
            $username = rtrim($username);
        }

        $this->username = $username;

        $suffix = '';
        while (isset(self::$usernames[$this->username])) {
            $username = mb_substr($username, 0, -1);
            $suffix  .= rand(0, 9);
            $this->username = $username . $suffix;
        }
        self::$usernames[$this->username] = true;
    }

    public function unsetUsername() {
        if (isset(self::$usernames[$this->username])) {
            unset(self::$usernames[$this->username]);
        }
    }
}