/**
 * Displays static html file selector with backtick and fills in templates using template comments in html blocks
 */
$(document).ready(function() {
    /*** load tmpl ***/
    /**
     * Loads the template if it's not cached and handles composite template calls
     *
     * @param template string   template name
     * @param callback function callback to be called on load
     */
    var loadTmpl = function(template, callback) {
        if(!template) {
            throw new Error('loadTmpl must be provided a template to load!');
        }

        // check if template is cached
        if($.template[template]) {
            if(callback) {
                callback();
            }

        // load the template
        } else {
            // make sure the template exists
            if(!magma.tmpl[template]) {
                throw new Error('loadTmpl: no tmpl named ' + template);
            }

            // compile the template
            $.template(template, magma.tmpl[template]);

            // look for references to other templates
            var matches = magma.tmpl[template].match(/{{tmpl[\(]?[^\)]*[\)]?[^"]+"[^"]+"}}/g);
            
            // may need to load before issuing callback
            if(matches) {
                var i = 0;
                while(i < matches.length) {
                    // grab last string which is the template reference
                    matches[i] = matches[i].match(/"[^"]+"/g).pop().replace(/"/g,'');

                    // can have recursive templates so check if already loaded
                    if(!$.template[matches[i]]) {
                        loadTmpl(matches[i++]);
                    } else {
                        // remove element since self reference
                        matches.splice(i,1);
                    }
                }

                // delay callback until dependancies loaded
                if(matches.length && callback) {
                    $.each(matches, function(i, tmplname) {
                        // pull for existance of template
                        var interval = setInterval(function() {
                            if($.template[tmplname]) {
                                clearInterval(interval); // stop pulling
                                matches.splice(i,1); // remove template
                                if(!matches.length) {
                                    callback();
                                }
                            }
                        }, 10);
                    });

                // all matches were recursive and removed
                } else if(callback) {
                    callback();
                }

            // no matches, go ahead and fill template if callback
            } else if(callback) {
                callback();
            }

            /*

            used to load templates from server

            $.ajax({
                url: "/js/templates/" + template + ".html",
                success: function(data) {
                    // code above here
                },
                error: function() {
                    throw new Error('No template named ' + template + '.');
                }
            });
            */
        }
    };

    /*** bytes to size ***/
    /**
     * Convert number of bytes into human readable format
     *
     * @source https://github.com/codeaid/snippets/blob/master/js/functions/bytesToSize.js
     *
     * @param integer bytes Number of bytes to convert
     * @param integer precision Number of digits after the decimal separator
     * @return string
     */
    var bytesToSize = function(bytes, precision) {
        var kilobyte = 1024;
        var megabyte = kilobyte * 1024;
        var gigabyte = megabyte * 1024;
        var terabyte = gigabyte * 1024;

        if ((bytes >= 0) && (bytes < kilobyte)) {
            return bytes + ' B';
        } else if ((bytes >= kilobyte) && (bytes < megabyte)) {
            return (bytes / kilobyte).toFixed(precision) + ' KB';
        } else if ((bytes >= megabyte) && (bytes < gigabyte)) {
            return (bytes / megabyte).toFixed(precision) + ' MB';
        } else if ((bytes >= gigabyte) && (bytes < terabyte)) {
            return (bytes / gigabyte).toFixed(precision) + ' GB';
        } else if (bytes >= terabyte) {
            return (bytes / terabyte).toFixed(precision) + ' TB';
        } else {
            return bytes + ' B';
        }
    };

    /*** fill tmpl ***/
    /**
     * Grabs the template from the server if it's not cached then fills it
     * Also massages data from server into form suitable for templates
     *
     * @param template string   template name
     * @param data object       object containing fields for template
     * @param callback function callback function to be called after template filled
     */
    var fill_tmpl = function(template, data, callback) {

        // transform data for certain templates
        switch(template) {
            case "folderList":
                // create a folder tree
                // loop with while to deal with splicing during loop
                var i = 0;

                while(data[i]) {
                    // folder href is lower case name with hyphens in place of spaces
                    data[i].link = data[i].name.toLowerCase().replace(/ /g, '-');

                    // child folder
                    if(typeof data[i].parentID == "number") {
                        // find child's parent
                        if(!(function(folders) {
                            for(var j in folders) {
                                if(folders[j].folderID === data[i].parentID) {
                                    // init if no folders yet
                                    folders[j].subfolders = folders[j].subfolders || [];
                                    folders[j].subfolders.push(data[i]);

                                    // remove folder from root
                                    data.splice(i, 1);

                                    return true; // success!

                                // scan subfolders
                                } else if(folders[j].subfolders) {
                                    if(arguments.callee(folders[j].subfolders)) {
                                        // found parent in subfolder
                                        return true;
                                    }
                                }
                            }

                        // no parent found!
                        })(data)) {
                            //console.log("Orphaned folder!", data[i]);

                            // goodbye little orphan...
                            data.splice(i, 1);
                        }

                    // increment since nothing removed
                    } else {
                        i++;
                    }
                }
            break;

            case "messageList":
                //convert dates and sizes to pretty strings
                for(var i in data) {
                    data[i].date = new Date(data[i].utc).toLocaleString();
                    data[i].size = bytesToSize(data[i].bytes, 2);
                    data[i].flag = {};
                }
            break;

            case "messageHeader":
                data.date = new Date(data.utc).toLocaleString();
            break;

            case "messageInfo":
                data.server.date = new Date(data.server.utc).toLocaleString();
            break;

            case "meta":
                if($.browser.webkit) {
                    data.browser = "Webkit";
                } else if($.browser.mozilla) {
                    data.browser = "Firefox";
                } else if($.browser.opera) {
                    data.browser = "Opera";
                } else if($.browser.msie) {
                    data.browser = "IE";
                }
            break;

            case "options":
                // check if listing aliases in composing
                if(data[0].email && data[0].name) {
                    // sort alphabetically by name
                    data.sort(function(a, b) {
                        if(a.name.toLowerCase() == b.name.toLowerCase()) {
                            return 0;
                        }
                        return a.name.toLowerCase() > b.name.toLowerCase() ? 1 : -1;
                    });

                    for(var i in data) {
                        data[i].value = data[i].email;
                        data[i].text = data[i].name + ' <' + data[i].email + '>';
                    }
                }

                // check if listing folders in search options
                if(typeof data[0].folderID === "number") {
                    data.sort(function(a, b) {
                        if(a.name.toLowerCase() == b.name.toLowerCase()) {
                            return 0;
                        }
                        return a.name.toLowerCase() > b.name.toLowerCase() ? 1 : -1;
                    });

                    for(var i in data) {
                        data[i].value = data[i].folderID;
                        data[i].text = data[i].name;
                    }
                }
            break;
        }

        // compile the template if needed
        loadTmpl(template, function() {
            var jquery = $.tmpl(template, data);

            // call the callback if given
            if(callback) {
                callback.call(jquery);
            }
        });
    };

    /*** fill state ***/
    var fill_state = function(state) {
        $.ajax({
            method: "get",
            url: "/" + state + ".html",
            success: function(data) {
                // cannot grab body tag so assumes outer dives and gets them
                // strips new lines and extra spaces
                var content = $(data.replace(/\n| {2,}/g,'')).filter('div');

                // replace body with content
                $('body').html(content);

                // watermarks
                switch(state) {
                    case "index":
                        $('#username, #password').fillWatermarks($('#login').find('label'));
                    break;

                    case "index.error":
                        $('#username, #password').fillWatermarks($('#login').find('label'));
                    break;

                    case "main.composing":
                        $('.composing-body textarea').fillWatermarks($('.composing-body label'));
                    break;

                    default:
                        // look for seach box
                        if($('.search').length) {
                            $('#search-input').fillWatermarks($('.search label'));
                        }
                    break;
                }

                // fill template comments with data
                content.comments(true).each(function() {
                    var stub = $(this).text(),
                        template,
                        options,
                        method,
                        params,
                        keys,
                        insert,
                        selector,
                        get_template = function(stub) {
                            // check for template options
                            if(stub.indexOf('(') > 0) {
                                template = stub.substr(0,stub.indexOf('('));
                                options = stub.substring(stub.indexOf('(') + 1, stub.indexOf(')')) || '{}';
                                // eval JSON - never do in production!!!
                                options = eval('(' + options + ')');
                            } else {
                                template = stub;
                                options = {};
                            }
                        },
                        get_destination = function(stub) {
                            insert = stub.split(':')[0];

                            if(insert === 'appendTo' || insert === 'prependTo' || insert === 'insertAfter' || insert === 'insertBefore') {
                                selector = stub.split(':')[1].replace(/'/g,'');
                            } else {
                                insert = '';
                            }
                        },
                        fill = function(data) {
                            switch(template) {
                                case "auth.failed":
                                    $('<div id="login-error"><p>' + data.failed + '</p></div>')[insert](selector);
                                break;

                                case "paragraph":
                                    if(data.locked) {
                                        $('<p>' + data.locked + '</p>')[insert](selector);
                                    } else if(data.text) {
                                        $('<p>' + data.text + '</p>')[insert](selector);
                                    }
                                break;

                                case "link":
                                    if(data.id === "continue") {
                                        $('<a id="continue" class="button" target="_blank" href="' + data.href + '">Continue</a>')[insert](selector);
                                    } else if(data instanceof Array) {
                                        if(data[0].attachmentID) {
                                            for(var i in data) {
                                                $('<a/>')
                                                    .attr({
                                                        id: "attachment-" + data[i].attachmentID,
                                                        "class": "attachment",
                                                        href: "attachment/" + data[i].name.toLowerCase().replace(/ /g, '-')
                                                    })
                                                    .text(data[i].name + " (" + bytesToSize(data[i].bytes, 2) + ")")
                                                    [insert](selector);
                                            }
                                        }
                                    }
                                break;

                                case "messageList":
                                    for(var i in data) {
                                        data[i].date = new Date(data[i].utc).toLocaleString();
                                        data[i].size = bytesToSize(data[i].bytes, 2);
                                        for(var j in data[i].tags) {
                                            data[i].tags[j] = [data[i].tags[j].toLowerCase().replace(/ /,'-'), data[i].tags[j]];
                                        }
                                    }

                                    fill_tmpl(template, data, function() {
                                        this[insert](selector);
                                    });
                                break;

                                case "contacts.list":
                                    if(state === 'main.contacts.info') {
                                        fill_tmpl(template, data, function() {
                                            this.each(function() {
                                                for(var i=1; i<$(this).children().length; i++) {
                                                    $(this).children().eq(i).remove();
                                                }
                                            });
                                            this[insert](selector);
                                        });
                                    } else {
                                        fill_tmpl(template, data, function() {
                                            this[insert](selector);
                                        });
                                    }
                                break;

                                case "tags":
                                    for(var i in data.tags) {
                                        $('<li/>')
                                            .append($('<a/>')
                                                .text(data.tags[i])
                                            )
                                            .appendTo('.tool-options ul');

                                    }
                                break;

                                default:
                                    fill_tmpl(template, data, function() {
                                        this[insert](selector);
                                    });
                                break;
                            }
                        };

                    // match template[({options})] method([{params}])[.keys] insert:'selector'
                    stub = stub.match(/[^ ]+(\([^\)]*\)[^ ]*|'[^']*')|[^ ]+/g);

                    // template method-call and destination
                    if(stub.length === 3) {
                        get_template(stub[0]);

                        method = stub[1].substr(0,stub[1].indexOf('('));

                        if(method) {
                            params = stub[1].substring(stub[1].indexOf('(') + 1, stub[1].indexOf(')')) || '{}';
                            params = eval('(' + params + ')');
                            keys = stub[1].substr(stub[1].indexOf(')') + 1, stub[1].length).split('.');

                            get_destination(stub[2]);

                            if(insert) {
                                $.get('/portal/mockiface', {method: method, params: params}, function(data) {
                                    data = data["error"] || data["result"];

                                    // get to the template data from the given key
                                    for(var i=0; i<keys.length; i++) {
                                        // ignore empty strings
                                        if(keys[i]) {
                                            data = data[keys[i]];
                                        }
                                    }

                                    // see if options were set
                                    if((function(obj) {
                                        for(var prop in obj) {
                                            if(obj.hasOwnProperty(prop)) { return true; }
                                        }
                                        return false;
                                    })(options)) {
                                        if(data instanceof Array && typeof data[0] == "object") {
                                            // add options to beginning
                                            data.unshift(options);

                                        } else if(typeof data == "object") {
                                            $.extend(data, options);

                                        // see if options were set
                                        } else {
                                            throw new Error(data, "is not an object and cannot be extended with:", options);
                                        }
                                    }

                                    fill(data);
                                });
                            } else {
                                throw new Error("Bad insert method: " + stub[2]);
                            }
                        // probably forgot method call parans
                        } else {
                            throw new Error("Bad method call: " + stub[1]);
                        }
                    // template and destination
                    } else if(stub.length === 2) {
                        get_template(stub[0]);
                        get_destination(stub[1]);

                        if(insert) {
                            fill(options);
                        } else {
                            throw new Error("Bad insert method: " + stub[1]);
                        }
                    }
                });
            },
            error: function(data) {
                throw new Error('State ' + url_args.state + ' was not found.');
            }
        });
    };

    /*** List static templates ***/
    var states = [],
        state,
        dialog_box,
        state_dialog = function() {

        // list with numberings to figure out folder structure
        list = false;

        if(list) {
            var template = '<input type="radio" name="templates" value="${state}" />${index}. ${state}<br />',
                dialog_box;

            dialog_box = $('<div id="state-selector"><form></form></div>').appendTo('body').css({height: "auto", backgroundColor: "white"});
            $.tmpl(template, states)
                .appendTo(dialog_box.children('form'))
                .siblings('input').css({margin: "0 5px 5px 0"})
                .filter(':first').attr('checked', true);
        } else {
            var treeTmpl = '<li id="folder-${folderID}" class="{{if subfolders}}expandable{{/if}}{{if opened}} open{{/if}}">{{if subfolders}}<span>+</span>{{/if}}<a href="${state}">${name}</a></li> {{if expanded}} <li> <ul> {{tmpl($item.data.subfolders) "tree"}} </ul> </li> {{/if}}';

            // order of folder list maps to state
            // set list to true above to see a numbered list with backtick to make changes below easier
            var tree = [{
                "name": "Advertisement"
            }, {
                "parentID": 4,
                "name": "Error"
            }, {
                "parentID": 4,
                "name": "Locked"
            }, {
                "name": "Logout"
            }, {
                "name": "Login"
            }, {
                "parentID": 8,
                "name": "Ad"
            }, {
                "parentID": 8,
                "name": "Finished Ad"
            }, {
                "parentID": 8,
                "name": "Finished"
            }, {
                "name": "Loading"
            }, {
                "parentID": 8,
                "name": "Warning"
            }, {
                "name": "Alert"
            }, {
                "name": "Composing"
            }, {
                "parentID": 13,
                "name": "Info"
            }, {
                "name": "Contacts"
            }, {
                "parentID": 17,
                "name": "Fields"
            }, {
                "parentID": 17,
                "name": "Reading Pane"
            }, {
                "parentID": 17,
                "name": "Sort"
            }, {
                "parentID": 27,
                "name": "Display Options"
            }, {
                "parentID": 27,
                "name": "Editing Folders"
            }, {
                "name": "Fatal Error"
            }, {
                "name": "Meta Info"
            }, {
                "parentID": 23,
                "name": "Display Attachments"
            }, {
                "parentID": 23,
                "name": "Extra Info"
            }, {
                "name": "Reading"
            }, {
                "name": "Scrape Contacts"
            }, {
                "name": "Search"
            }, {
                "name": "Identity Settings"
            }, {
                "name": "Mail"
            }, {
                "parentID": 27,
                "name": "Tags"
            }];

            for(var index in tree) {
                tree[index].folderID = parseInt(index);
                tree[index].state = states[index].state;
            }

            // create a folder tree
            // loop with while to deal with splicing during loop
            var i = 0;

            while(tree[i]) {
                // child folder
                if(typeof tree[i].parentID === "number") {
                    // find child's parent
                    if(!(function(folders) {
                        for(var j in folders) {
                            if(folders[j].folderID === tree[i].parentID) {
                                // init if no folders yet
                                folders[j].subfolders = folders[j].subfolders || [];
                                folders[j].subfolders.push(tree[i]);

                                // remove folder from root
                                tree.splice(i, 1);

                                return true; // success!

                            // scan subfolders
                            } else if(folders[j].subfolders) {
                                if(arguments.callee(folders[j].subfolders)) {
                                    // found parent in subfolder
                                    return true;
                                }
                            }
                        }

                    // no parent found!
                    })(tree)) {
                        console.log(tree[i]);

                        // goodbye little orphan...
                        tree.splice(i, 1);
                    }

                // increment since nothing removed
                } else {
                    i++;
                }
            }

            dialog_box = $('<div id="state-selector"><ul></ul></div>');

            $.template('tree', treeTmpl);
            $.tmpl('tree', tree)
                .appendTo(dialog_box.find('ul'));
        }

        dialog_box.appendTo('body')
            .css({height: "auto", backgroundColor: "white"})
            .delegate('span', 'click', function(event) {
                event.preventDefault();

                var tmplItem = $(this).parent().tmplItem();
                tmplItem.data.opened = !tmplItem.data.opened;
                if($(this).parent().hasClass("expandable")) {
                    tmplItem.data.expanded = !tmplItem.data.expanded;
                }
                tmplItem.update();
            }).delegate('a', 'click', function(event) {
                event.preventDefault();
                fill_state($(this).attr('href'));
            });
    
        // prevent overlay from overflowing
        $('body').css("overflow", "hidden");

        dialog_box.dialog({
            resizable: false,
            draggable: true,
            modal: true,
            title: "Choose a state",
            close: function() {
                $(this).remove();
                $('body').css("overflow", "auto");
            }
        });
    };

    $(window).keypress(function(event) {
        // type back tick '`'
        if(event.which === 96 && !$('#state-selector').length) {
            if(!states.length) {
                $.getJSON("dev/static/statenames.json", function(data) {
                    for(var index in data) {
                        var state = data[index];
                        states.push({state: state.substring(0, state.indexOf('.state')), index: index});
                    }
                    state_dialog();
                });
            } else {
                state_dialog();
            }
        }
    });
});
