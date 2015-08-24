'use strict';

////////////////////////////////////////////////////////////////////
function ws_chat_list(targetId, nicknames) {
    this.list = document.getElementById(targetId);

    this.me = 0;
    this.users = {};
    this.nicknames = nicknames;
}

ws_chat_list.prototype.setUserMe = function (userId, userName) {
    this.setUser(userId, userName);
    this.me = userId;
    this._refresh();
}

ws_chat_list.prototype.setUser = function (userId, userName) {
    this.nicknames[userId] = userName;
    if (this.users[userId]) {
        this.users[userId].nick = userName;
    } else {
        this.users[userId] = {
                nick: userName,
                elem: null
        }
    }
    this._refresh();
}

ws_chat_list.prototype.unsetUser = function (userId) {
    if (this.users[userId]) {
        if (this.users[userId].elem) {
            this.users[userId].elem.parentNode.removeChild(this.users[userId].elem);
        }
        delete this.users[userId];
    }

    this._refresh();
}

ws_chat_list.prototype._refresh = function () {
    for (var i in this.users) {
        var user = this.users[i];
        if (null === user.elem) {
            user.elem = document.createElement('li');
            user.elem.textContent = user.nick;
            user.elem.dataset.id = i;
            this.list.appendChild(user.elem);
        }
        user.elem.textContent = user.nick;
        if (i == this.me) {
            user.elem.classList.add('me');
        }
    }
}

////////////////////////////////////////////////////////////////////
function ws_chat_room(targetId, chat) {
    this.room = document.getElementById(targetId);
    this.chat = chat;
}

ws_chat_room.prototype.displayMessage = function(time, data, username, noNotification) {
    var msg = document.createElement('div');
    var span = document.createElement('span');
    span.className = 'time';
    var tmp = new Date();
    tmp.setTime(time * 1000);
    span.textContent = tmp.toLocaleString();
    msg.appendChild(span);

    span = document.createElement('span');
    span.className = 'user';
    if (undefined === username) {
        username = this.chat.nicknames[data.uid];
    }
    span.textContent = username + ' :';
    msg.appendChild(span);

    span = document.createElement('span');
    span.className = 'msg';
    span.innerHTML = data.msg;
    msg.appendChild(span);

    this.room.appendChild(msg);
    this._scroll();

    if (noNotification) {
        this.chat._notify('New message from ' + username, data.msg);
    }

    return msg;
}

ws_chat_room.prototype.displayEvent = function(time, message) {
    var msg = document.createElement('div');
    msg.classList.add('event');
    var span = document.createElement('span');
    span.className = 'time';
    var tmp = new Date();
    tmp.setTime(time * 1000);
    span.textContent = tmp.toLocaleString();
    msg.appendChild(span);
    
    span = document.createElement('span');
    span.className = 'msg';
    span.innerHTML = message;
    msg.appendChild(span);
    
    this.room.appendChild(msg);
    this._scroll();
    
    return msg;
}

ws_chat_room.prototype._scroll = function () {
    this.room.scrollTop = this.room.scrollHeight;
}

////////////////////////////////////////////////////////////////////
/**
 * Constructor
 * 
 * @param host
 * @param port
 * @param uri
 * @param secure
 * @constructor
 */
function ws_chat(host, port, uri, secure) {
    this.host = undefined === host ? '127.0.0.1' : host;
    this.port = undefined === port ? '8081' : parseInt(port);
    this.uri = undefined === uri ? 'websocket' : uri;
    this.secure = secure ? true : false;

    var url = 'ws' + (this.secure ? 's' : '') + '://';
    url += this.host + ':' + this.port + '/' + this.uri;

    this.log('Trying to connect to ' + url + '…');

    this.ws = new WebSocket(url);
    this.ws.onopen = function() {
        var username = localStorage.getItem('username');
        if (username) {
            this.ws.send('/nick ' + username);
        }
        this.onConnect.call(this);
    }.bind(this);
    this.ws.onmessage = function(msg) {
        this.onMessage.call(this, msg);
    }.bind(this);
    this.ws.onclose = function() {
        this.onClose.call(this);
    }.bind(this);
    window.addEventListener('beforeunload', function() {
        this.ws.close();
        return 'Bye !';
    }.bind(this));

    Notification.requestPermission();
    this.nicknames = {};
    this.input = document.getElementById('input');
    this.contactList = new ws_chat_list('people', this.nicknames);
    this.room = new ws_chat_room('room', this);
    this.topic = document.getElementById('topic');

    this.input.addEventListener('keydown', this._inputKeydown.bind(this));
}

ws_chat.prototype.onConnect = function() {
    this.log('Connected');
}

ws_chat.prototype.onMessage = function(payload) {
    var msg = JSON.parse(payload.data);

    switch (msg.op) {
        case 'connect':
            this.contactList.setUser(msg.data.uid, msg.data.username);
            var text = msg.data.username + ' has joined the conversation.';
            this.room.displayEvent(msg.time, text);
            this._notify(text);
            break;
        case 'disconnect':
            this.contactList.unsetUser(msg.data.uid);
            var text = this.nicknames[msg.data.uid] + ' has left the conversation.';
            this.room.displayEvent(msg.time, text);
            this._notify(text);
            break;
        case 'msg':
            this.room.displayMessage(msg.time, msg.data);
            break;
        case 'nick':
            this.room.displayEvent(msg.time, this.nicknames[msg.data.uid] + ' changes his nickname to ' + msg.data.username);
            this.contactList.setUser(msg.data.uid, msg.data.username);
            if (msg.data.uid == this.contactList.me) {
                localStorage.setItem('username', msg.data.username);
            }
            break;
        case 'event':
            this.room.displayEvent(msg.time, msg.data.text);
            break;
        case 'topic':
            this.topic.textContent = msg.data.topic;
            this.room.displayEvent(msg.time, this.nicknames[msg.data.uid] + ' changed the topic to ' + this.topic.textContent);
            break;
        case 'welcome':
            this.contactList.setUserMe(msg.data.uid, msg.data.username);
            for (var i in msg.data.users) {
                var user = msg.data.users[i];
                this.contactList.setUser(user.uid, user.username);
            }
            for (var i in msg.data.history) {
                var data = msg.data.history[i];
                if (undefined === data[2]) {
                    this.room.displayEvent(data[0], data[1]);
                } else {
                    this.room.displayMessage(data[0], data[1], data[2], true);
                }
            }
            this.room.displayEvent(msg.time, 'You joined the conversation');

            this.topic.textContent = msg.data.topic;
            break;
        case 'alert':
            alert(msg.data.result);
            break;
        default:
            this.log("Unknown operation");
    }
    
}

ws_chat.prototype.onClose = function() {
    this.log('Disconnected');
    document.getElementsByTagName('html')[0].innerHTML = 'End of conversation (server closes connection)';
}

ws_chat.prototype.send = function(text) {
    this.log('Send message: ' + text);
    this.ws.send(text);
}

ws_chat.prototype._inputKeydown = function(event) {
    if (event.keyCode === 13 && event.shiftKey === false) {
        this.ws.send(this.input.value);
        this.input.value = '';
        event.preventDefault();
    }
}

ws_chat.prototype._notify = function(title, content) {
    if (document.hasFocus()) {
        console.debug('Notification ignored (cause: focus)');
        return;
    }
    if (!("Notification" in window)) {
        console.warning('Browser does not implement Notifications');
        return;
    }

    if (Notification.permission === "granted") {
        this._createNotification(title, content);
    } else if (Notification.permission !== 'denied') {
        Notification.requestPermission(function (permission) {
            if (permission === "granted") {
                this._createNotification(title, content);
            }
        }.bind(this));
    }
}

ws_chat.prototype._createNotification = function(title, content) {
    var options = {
        body: content,
        icon: '/notification.png',
        lang: 'fr'
    };
    var n = new Notification(title, options);
    setTimeout(n.close.bind(n), 2500);
    return n;
}

/** 
 * Log
 * 
 * @param {String} msg
 * @returns {void}
 */
ws_chat.prototype.log = function(msg) {
    console.log(new Date() + ' ' + msg)
}

var ws_chat;
window.addEventListener('load', function() {
    ws_chat = new ws_chat(window.location.hostname);
});