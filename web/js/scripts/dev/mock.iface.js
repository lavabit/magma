/**
 * Gets json method data on calls to mockiface using mockjax plugin
 */
$(document).ready(function() {
    /*
     * helper ajax call
     *
     * @param method    interface mathod name
     */
    var get_response = function(method) {
        var response;

        $.ajax({
            type: 'GET',
            url: '/json/responses/' + method + '.json',
            async: false,
            processData: true,
            dataType: 'json',
            success: function(data) {
                response = data;
            },
            error: function(data) {
                if(data.statusText === "parsererror") {
                    console.log('Parse error in response ' + method + '.json');
                } else if(data.status === 404) {
                    console.log('json/responses/' + method + '.json does not exist.');
                }
            }
        });

        return response;
    };
    
    /*
     * Ajax Mockups
     *
     * All Ajax requests will go to a single url during development.
     * Request and response data based on JSON-RPC spec: http://json-rpc.org/wiki/specification
     *
     * Request data
     * method: string name of method to be invoked
     * params: array containing arguments to pass to the function
     *
     * Response data
     * result: true or message if success, undefined if error
     * error: undefined if success, message string if error
     */
    $.mockjax({
        url: '/portal/mockiface',
        status: 200,    // change to 500 to test errors
        responseTime: 50,
        response: function(settings) {
            var data;

            // data could be string or json object
            if(typeof settings.data === 'string') {
                data = JSON.parse(settings.data);
            } else {
                data = settings.data;
            }

            var method = data.method,
                params = data.params,
                response = {},
                respond = function(result, num) {
                    // use first result if no index provided
                    num = num || 0;

                    var full_response = get_response(method);

                    // returns undefined if error
                    if(!full_response) {
                        response = {error: true};
                        return;
                    }

                    // get rid of results we don't need
                    var i = 0;
                    while(i < full_response.length) {
                        if(!full_response[i][result]) {
                            full_response.splice(i, 1);
                        } else {
                            i++;
                        }
                    }

                    response[result] = full_response[num][result];
                };

            // select specific results depending on method params
            // result number is array index of hard coded json response
            switch(method) {

                case "auth":
                    if(params && params.username === "magma" && params.password === "password") {
                        // authenticated!
                        respond("result");
                    } else if(params && params.username === "locked") {
                        // account locked
                        respond("result", 2);
                    } else {
                        // wrong username/password
                        respond("result", 1);
                    }
                break;

                case "aliases":
                    if(params && params.def) {
                        // only return default
                        respond("result", 1);
                    } else {
                        // return full list of aliases
                        respond("result");
                    }
                break;

                case "contacts.list":
                    // no gravatars
                    if(params && params.folderID === 999) {
                        respond("result", 1);
                    } else {
                        respond("result");
                    }
                break;

                case "folders.list":
                    if(params && params.context === "permanent") {
                        respond("result", 2);
                    } else if(params && params.context === "contacts") {
                        respond("result", 1);
                    } else if(params && params.context === "settings") {
                        respond("result", 2);
                    // TODO: change to logs
                    } else if(params && params.context === "logs") {
                        respond("result", 3);
                    } else if(params && params.context === "help") {
                        respond("result", 4);
                    } else {
                        // inbox
                        respond("result");
                    }
                break;

                case "folders.add":
                    respond("result");
                    response.result.folderID = Math.floor(Math.random()*1000);
                break;

                case "messages.list":
                    if(params && params.folderID === 0) {
                        // empty folder
                        respond("result");
                    } else {
                        respond("result", 1);
                        $.each(response.result, function(i, message) {
                            message.messageID = Math.floor(Math.random()*10000000);
                        });
                    }
                break;

                case "messages.load":
                    if(params && params.section[0] === "info") {
                        respond("result", 1);
                    } else {
                        respond("result");
                    }
                break;

                case "ad":
                    if(params && params.context === "loading") {
                        // loading context
                        respond("result");
                    } else {
                        // messages context
                        respond("result", 1);
                    }
                break;

                default:
                    respond("result");
                break;
            }

            this.responseText = response;
        }
    });
});
