--TEST--
Check for WebSocket presence
--SKIPIF--
<?php if (!extension_loaded("websocket")) print "skip"; ?>
--FILE--
<?php 
echo "WebSocket extension is available";
?>
--EXPECT--
WebSocket extension is available
