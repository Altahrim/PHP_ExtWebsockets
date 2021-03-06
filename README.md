# PHP_ExtWebsockets

[![Build Status](https://img.shields.io/travis/Timandes/PHP_ExtWebsockets/master.svg?style=flat-square)](https://travis-ci.org/Timandes/PHP_ExtWebsockets)
[![Coverage Status](https://coveralls.io/repos/Timandes/PHP_ExtWebsockets/badge.svg?branch=master&service=github)](https://coveralls.io/github/Timandes/PHP_ExtWebsockets?branch=master)

PHP extension providing support for websockets.

This extension rely on [libwebsockets](https://github.com/warmcat/libwebsockets) by [warmcat](https://github.com/warmcat/) to provide object-oriented WebSockets support in PHP 7.

Warning: this extension is only compatible with PHP >= 7.0 and only tested on linux.

## Quickstart

Install [libwebsockets](https://github.com/warmcat/libwebsockets). For Fedora you can use `sudo dnf install libwebsockets-devel`.

```sh
git clone https://github.com/Altahrim/PHP_ExtWebsockets
cd PHP_ExtWebsockets/src
phpize
./configure
make
sudo make install
## Check if extension is available
php -dextension=websocket.so -m | grep WebSocket
```
