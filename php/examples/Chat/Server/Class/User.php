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

    const USERNAME_MAX_LENGTH = 120;

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

        $this->username = htmlspecialchars($username);
        $this->username = trim($username);
        if (strlen($this->username > self::USERNAME_MAX_LENGTH)) {
            $this->username = substr($this->username, 0, self::USERNAME_MAX_LENGTH);
            $this->username = rtrim($this->username);
        }

        while (isset(self::$usernames[$this->username])) {
            $this->username .= rand(0, 9);
        }
        self::$usernames[$this->username] = true;
    }

    public function unsetUsername() {
        if (isset(self::$usernames[$this->username])) {
            unset(self::$usernames[$this->username]);
        }
    }
}
