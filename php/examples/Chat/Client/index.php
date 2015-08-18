<?php
header('Content-Type: text/html; charset=UTF-8');
$title = 'WebSocket example: Chat';
$title = htmlspecialchars($title, ENT_NOQUOTES, 'UTF-8');
?>
<html>
    <head>
        <title><?=$title?></title>
        <script src="js/main.js"></script>
        <link rel="stylesheet" type="text/css" href="css/main.css"/>
    </head>
    <body>
        <header>
            <h1><?=$title?></h1>
            <h2 id="topic"></h2>
        </header>
        <section>
            <div id="room"></div>
            <aside>
                <h3>People:</h3>
                <ul id="people"></ul>
            </aside>
        </section>
        <footer>
            <textarea id="input" autofocus placeholder="Write your text here and hit “Enter”"></textarea>
            <a href="#send">Send</a>
        </footer>
    </body>
</html>
