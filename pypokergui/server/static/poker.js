/*
 *  Register callback functions on buttons.
 */
$(document).ready(function() {
    if (!window.console) window.console = {};
    if (!window.console.log) window.console.log = function() {};
    $("#registration_form").on("submit", function() {
        registerPlayer($(this));
        return false;
    });
    $("#start_game_form").on("submit", function() {
        $("#message").val(generateMixed(10));
        $("#registration_form").submit();
        startGame();
        $(".btn.btn-success").disabled = true;
        return false;
    });



    updater.start();

    window.unload = function () {
        close_socket();
    };
});



function close_socket() {
    updater.socket.close();
}

function generateMixed(n) {
    var chars = ['0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'];
    var res = "";
    for(var i = 0; i < n ; i ++) {
     var id = Math.ceil(Math.random()*35);
    res += chars[id];
    }
    return res;
}
function autoregisterPlayer() {
    var message = { _xsrf: "2|a5abd3cc|f3122250d0b4e0a31525066f19089faa|1591582469", type: "action_new_member", name: "ffff"};
    console.log(message)
    message['name'] = generateMixed(10)
    console.warn('warn')
    console.log('send')
    updater.socket.send(JSON.stringify(message));
}
/*
 *  Callback function invoked when
 *  human player is registered.
 */
function registerPlayer(form) {
    var message = form.formToDict();
    message['type'] = "action_new_member"
    message['name'] = 'Human'//message['body']
    console.log(message)
    delete message.body
    console.warn('warn')
    updater.socket.send(JSON.stringify(message));
}

function onClickNext()
{
    message = {}
    message['type'] = "next_action"
    updater.socket.send(JSON.stringify(message));
    console.log("点击执行")
}

/*
 * Callback function invoked when
 * game is starged.
 */
function startGame() {
    message = {}
    message['type'] = "action_start_game"
    updater.socket.send(JSON.stringify(message));

}

/*
 * Callback function invoked when
 * human player declared his action in the game.
 */
function declareAction(form) {
  var message = form.formToDict();
  message['type'] = "action_declare_action"
  updater.socket.send(JSON.stringify(message))
}

/*
 * Helper function to get form information as hash.
 */
jQuery.fn.formToDict = function() {
    var fields = this.serializeArray();
    var json = {}
    for (var i = 0; i < fields.length; i++) {
        json[fields[i].name] = fields[i].value;
    }
    if (json.next) delete json.next;
    return json;
};
function CheckIsNullOrEmpty(value) {
    //正则表达式用于判斷字符串是否全部由空格或换行符组成
    var reg = /^\s*$/
    //返回值为true表示不是空字符串
    return (value != null && value != undefined && !reg.test(value))
}
/*
 *  This object setups and holds websocket.
 */
var updater = {
    socket: null,

    /*
     *  This method is invoked when index page is opened.
     *  Setup websocket and register callback method on it.
     *  URL would be "ws://localhost/pokersocket:8888".
     */
    start: function() {
        var url = "ws://" + location.host + "/pokersocket";
        updater.socket = new WebSocket(url);
        updater.socket.onmessage = function(event) {
            // window.console.log("received new message: " + event.data)
            message = JSON.parse(event.data)
            if ('config_update' == message['message_type']) {
              updater.updateConfig(message)
            } else if ('start_game' == message['message_type']) {
              updater.startGame(message)
            } else if ('update_game' == message['message_type']) {
              updater.updateGame(message)
            } else if ('alert_restart_server' == message['message_type']) {
              updater.alert_restart_server(message)
            } else if ('pause_game' == message['message_type']) {
                updater.pauseGame()
            }
            else {
              window.console.error("received unexpected message: " + message)
            }
        }
    },

    /*
     * Invoked when received the new message
     * about update of config like new member is registered.
     */
    updateConfig: function(message) {
        var node = $(message.html);
        $("#config_box").html(node)
        if (message.registered) {
          $("#registration_form input[type=submit]").prop("disabled", true);
        }
    },

    /*
     * Invoked when received the message
     * about start of the game.
     */
    startGame: function(message) {
      var node = $(message.html)
      $("#container").html(node)
      $("#declare_action_form").hide()
      $("#declare_action_form").on("submit", function() {
        if($(this).serializeArray()[0]['value'] == 'raise'){
            var validaction = eval("(" + $("#raise_amount").text() + ")");
            window.console.log(validaction);
            if (CheckIsNullOrEmpty($(this).serializeArray()[1]['value']) == false){
                $("#errorRaise").show();
                return false;
            }
            amountc = parseInt($(this).serializeArray()[1]['value']);
            window.console.log(amountc);
            // window.console.log(amountc < validaction['min'] || amountc > validaction['max']);
            if (amountc < validaction['min'] || amountc > validaction['max']){
                $("#errorRaise").show();
                return false;
            }
            else{
                $("#errorRaise").hide();
                declareAction($(this));
                return false;
            }
        }
        else{
            $("#errorRaise").hide();
            declareAction($(this));
            return false;
        }
      });
    },

    /*
     * Invoked when received the message about
     * new event of the game like "new round will start".
     */
    updateGame: function(message) {
        $("#declare_action_form").hide()
        content = message['content']
        // window.console.log("updateGame: " + JSON.stringify(content))
        message_type = content['update_type']
        if ('round_start_message' == message_type) {
          updater.roundStart(content.event_html)
        } else if ('street_start_message' == message_type) {
          updater.newStreet(content.table_html, content.event_html)
        } else if ('game_update_message' == message_type) {
          updater.newAction(content.table_html, content.event_html)
       } else if ('round_result_message' == message_type) {
         updater.roundResult(content.table_html, content.event_html)
       } else if ('game_result_message' == message_type) {
         updater.gameResult(content.event_html)
       } else if ('ask_message' == message_type) {
         $("#declare_action_form").show()
         updater.askAction(content.table_html, content.event_html)
       } else {
          window.console.error("unexpected message in updateGame: " + content)
       }
    },

    roundStart: function(event_html) {
      $("#event_box").html($(event_html))
    },

    newStreet: function(table_html, event_html) {
      $("#table").html($(table_html))
      $("#event_box").html($(event_html))
    },

    newAction: function(table_html, event_html) {
      $("#table").html($(table_html))
      $("#event_box").html($(event_html))
    },

    roundResult: function(table_html, event_html) {
      $("#table").html($(table_html))
      $("#event_box").html($(event_html))
    },

    gameResult: function(event_html) {
      $("#event_box").html($(event_html))
    },

    askAction: function(table_html, event_html) {
      $("#table").html($(table_html))
      $("#event_box").html($(event_html))
    },

    alert_restart_server: function(message) {
      alert(message.message)
    },
};

