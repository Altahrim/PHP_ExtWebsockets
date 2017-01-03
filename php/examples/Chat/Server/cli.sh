#!/usr/bin/php
<?php
// Console specific settings
ini_set('html_errors', 0);
ini_set('error_log', 'stderr');
ini_set('error_prepend_string', "\e[31m");
ini_set('error_append_string', "\033[0m");

include __DIR__ . DIRECTORY_SEPARATOR . 'index.php';
