--TEST--
Check for basic run
--SKIPIF--
<?php if (!extension_loaded("websocket")) print "skip"; ?>
--FILE--
<?php 
$port = 31330 + rand (1, 999);
$ws = new \Websocket\Server($port);
?>
--EXPECT--
Create server object
Free server object
Server destroyed
