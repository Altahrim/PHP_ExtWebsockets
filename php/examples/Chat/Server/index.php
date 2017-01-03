<?php
if (!extension_loaded('websocket')){
    die ('WebSocket not loadable.');
}

define('HAS_EV', extension_loaded('ev'));

error_reporting(-1);
ini_set('display_errors', true);

require_once __DIR__ . DIRECTORY_SEPARATOR . 'Class' . DIRECTORY_SEPARATOR . 'Server.php';
require_once __DIR__ . DIRECTORY_SEPARATOR . 'Class' . DIRECTORY_SEPARATOR . 'EventLoop.php';
require_once __DIR__ . DIRECTORY_SEPARATOR . 'Class' . DIRECTORY_SEPARATOR . 'User.php';

$server = new \WS\Server();
$server->run(HAS_EV);
