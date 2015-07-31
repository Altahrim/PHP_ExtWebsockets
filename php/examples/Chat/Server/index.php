<?php
if (!extension_loaded('websocket')){
    if (!dl('websocket.so')){
        die ('WebSocket not loadable.');
    }
}
error_reporting(-1);
ini_set('display_errors', true);

require_once __DIR__ . DIRECTORY_SEPARATOR . 'Class' . DIRECTORY_SEPARATOR . 'Server.php';
require_once __DIR__ . DIRECTORY_SEPARATOR . 'Class' . DIRECTORY_SEPARATOR . 'User.php';

$serv = new \WS\Server();
$serv->run();
