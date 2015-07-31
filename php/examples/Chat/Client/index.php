<?php header('Content-Type: text/html; charset=UTF-8') ?>
<html>
    <head>
        <title>WS Chat</title>
        <script src="main.js"></script>
        <link rel="stylesheet" type="text/css" href="main.css"/>
    </head>
    <body>
        <header>
            <h1>WS_Chat</h1>
            <h2 id="topic"></h2>
        </header>
        <section>
            <div id="room">

            </div>
            <aside>
                <h3>People:</h3>
                <ul id="people">

                </ul>
            </aside>
        </section>
        <footer>
            <textarea id="input" autofocus placeholder="Write your text here and hit “Enter”"></textarea>
        </footer>
    </body>
</html>
