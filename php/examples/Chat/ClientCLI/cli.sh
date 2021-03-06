#!/usr/bin/php
<?php
// Console specific settings
ini_set('html_errors', 0);
ini_set('error_log', 'stderr');
ini_set('error_prepend_string', "\e[31m");
ini_set('error_append_string', "\033[0m");

if (!extension_loaded('websocket')){
    die ('WebSocket not loadable.');
}

define('HAS_EV', extension_loaded('ev'));

error_reporting(-1);
ini_set('display_errors', true);

require_once __DIR__ . DIRECTORY_SEPARATOR . 'Class' . DIRECTORY_SEPARATOR . 'Client.php';

$server = new \WS\Client();
$server->connect();
