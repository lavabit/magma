/**
 * model.js
 *
 * Contains all models for magma
 */
var magma = magma || {};

magma.model = (function() {
    // private variables and methods

    // share permanent folder IDs
    var permaFolders = (function() {
        var folders = {},
            names = {
                mail: [
                    'inbox',
                    'drafts',
                    'sent',
                    'junk',
                    'trash'
                ],
                contacts: [
                    'all',
                    'people',
                    'business',
                    'collected'
                ],
                settings: [
                    'identity',
                    'mail-settings',
                    'portal-settings',
                    'account-upgrades',
                    'password'
                ],
                logs: [
                    'statistics',
                    'security',
                    'contacts',
                    'mail'
                ]
            };

        return {
            getNames: function(context) {
                if(!names[context]) {
                    throw new Error('permaFolders.getNames: no context ' + context);
                }
                return names[context];
            },
            getName: function(id) {
                for(var name in folders) {
                    if(folders[name] === id) {
                        return name;
                    }
                }
                throw new Error('permaFolders.getName: no folder with id ' + id);
            },
            getID: function(name) {
                if(typeof folders[name] === 'number') {
                    return folders[name];
                }
                throw new Error('getFolderID: no folder named ' + name);
            },
            setID: function(name, folderID) {
                folders[name] = folderID;
            }
        };
    }());

    /*
     * Loads data from server
     *
     * @param method            JSON method name
     * @param params            method parameters
     * @param callbacks         object containing callbacks
     */
    var getData = (function() {
        var ID = 0;

        return function(method, params, callbacks) {
            ID += 1;

            var session = magma.session.get(),
                data = $.extend({method: method, id: ID}, params ? {params: params} : {});

            $.ajax({
                type: 'POST',
                url: magma.portalUrl + (session ? '/' + session + '/' : ''),
                dataType: 'json',
                contentType: 'application/json',
                cache: false,
                processDate: false,
                data: JSON.stringify(data),
                success: function(data) {
                    if(data.result) {
                        if(callbacks.success) {
                            callbacks.success(data.result);
                        }
                    } else if(data.error) {
                        if(callbacks.error) {
                            callbacks.error(data.error);
                        }
                    }
                },
                error: function(xhr) {
                    if(xhr.status !== 0 && callbacks.error) {
                        callbacks.error(xhr.responseText);
                    } else if(callbacks.fatal) {
                        callbacks.fatal();
                    } else {
                        magma.dialog.die("Could not connect to server!");
                    }
                }
            });
        };
    }());

    /**
     * Loads the jquery template if it's not cached and handles composite template calls
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
                throw new Error('loadTmpl: no tmpl named ' + template + '. this may have occured from an embedded tmpl call.');
            }

            // compile the template
            $.template(template, magma.tmpl[template]);

            // look for references to other templates
            var matches = magma.tmpl[template].match(/\{\{tmpl[\(]?[^\)]*[\)]?[^"]+"[^"]+"\}\}/g);
            
            // may need to load before issuing callback
            if(matches) {
                var i = 0;
                while(i < matches.length) {
                    // grab last string which is the template reference
                    matches[i] = matches[i].match(/"[^"]+"/g).pop().replace(/"/g,'');

                    // can have recursive templates so check if already loaded
                    if(!$.template[matches[i]]) {
                        loadTmpl(matches[i]);
                        i += 1;
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
                                matches.splice(i,1); // remove match
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
        }
    };

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

    /**
     * Transform to lower case and replace spaces with hyphens
     *
     * @param str   string to slugify
     */
    var slugify = function(str) {
        return str.toLowerCase().replace(/ /,'-');
    };

    /**
     * Transform to camel case
     *
     * @param str   string to convert to camel case
     */
    var camelCasify = function(str) {
        return str.toLowerCase().replace(/[ -_]+([a-z])/, function(s, p1) {
            return p1.toUpperCase();
        });
    };

    /**
     * Sort an array of strings ignoring case
     * Pass this to the sort method
     */
    var ignoreCaseSort = function(a, b) {
        if(a.toLowerCase() === b.toLowerCase()) {
            return 0;
        }
        return a.toLowerCase() > b.toLowerCase() ? 1 : -1;
    };

    /**
     * Sort an array of strings ignoring case
     * Pass this to the sort method
     */
    var ignoreCaseSortName = function(a, b) {
        if(a.name.toLowerCase() === b.name.toLowerCase()) {
            return 0;
        }
        return a.name.toLowerCase() > b.name.toLowerCase() ? 1 : -1;
    };

    /**
     * Create success, error, and failed events from a list of events
     *
     * @param eventList     array of events
     */
    var listEvents = function(eventList) {
        var events = [];

        for(var i in eventList) {
            events.push(eventList[i]);
            events.push(eventList[i] + 'Error');
            events.push(eventList[i] + 'Failed');
        }

        return events;
    };
    
    /**
     * Provides a way for model to notify subscribers of changes
     *
     * @param events    event names that subscribers will subscribe to
     *
     * @source http://michaux.ca/articles/mvc-to-do-application
     */
    var newObservable = function(events) {
        var groups = {},
            addEvent,
            addObserver,
            observeOnce,
            removeObserver,
            notifyObservers;

        events = events || [];

        addEvent = function(event) {
            if (groups.hasOwnProperty(event)) {
                throw new Error('addEvent: Already an event named "' + event + '"');
            }
            groups[event] = [];
        };

        addObserver = function(event, observer) {
            if (!groups.hasOwnProperty(event)) {
                throw new Error('addObserver: No event "' + event + '".');
            }
            var group = groups[event];
            for (var i=0, ilen=group.length; i<ilen; i += 1) {
                if (group[i] === observer) {
                    throw new Error('Cannot add the same listener more than once.');
                }
            }
            group.push(observer);
        };

        observeOnce = function(event, observe) {
            var observer = function(data) {
                observe(data);
                removeObserver(event, observer);
            };

            addObserver(event, observer);
        };
        
        removeObserver = function(event, observer) {
            if (!groups.hasOwnProperty(event)) {
                throw new Error('removeObserver: No event "' + event + '".');
            }
            var group = groups[event];
            for (var i=0, ilen=group.length; i<ilen; i += 1) {
                if (group[i] === observer) {
                    group.splice(i, 1);
                    return;
                }
            }
            throw new Error('removeObserver: Did not find the observer and so could not remove it.');
        };
        
        notifyObservers = function(event, data) {
            if (!groups.hasOwnProperty(event)) {
                throw new Error('notifyObservers: No event "' + event + '".');
            }
            var group = groups[event];
            for (var i=0, ilen=group.length; i<ilen; i += 1) {
                group[i](data);
            }
        };
        
        // initialize
        for (var i=0; i<events.length; i += 1) {
            var event = events[i];
            addEvent(event);
        }
                    
        return {
            addEvent: addEvent,
            addObserver: addObserver,
            observeOnce: observeOnce,
            removeObserver: removeObserver,
            notifyObservers: notifyObservers
        };
    };

    var addModel = function(params) {
        // make sure all the necessary params are added
        (function(properties) {
            for(var i in properties) {
                if(!params[properties[i]]) {
                    throw new Error('addModel: missing parameter ' + properties[i]);
                }
            }
        }(['method', 'models', 'observable', 'event']));

        return function(model) {
            if(!model) {
                throw new Error(params.method + ': model must be provided');
            }

            for(var i in params.models) {
                if(model === params.models[i]) {
                    throw new Error(params.method + ': cannot add the same model twice');
                }
            }

            params.models.push(model);
            params.observable.notifyObservers(params.event, model);
        };
    };

    var removeModel = function(params) {
        // make sure all the necessary params are present
        (function(properties) {
            for(var i in properties) {
                if(!params[properties[i]]) {
                    throw new Error('removeModel: missing parameter ' + properties[i]);
                }
            }
        }(['method', 'models', 'observable', 'event']));

        return function(model) {
            if(!model) {
                throw new Error(params.method + ': no model given.');
            }

            for(var i in params.models) {
                if(model === params.models[i]) {
                    params.models.splice(i, 1);
                    params.observable.notifyObservers(params.event, model);
                    return;
                }
            }

            throw new Error(params.method + ': model not in list saved models.');
        };
    };

    // models
    return {
        /*** ad ***/
        ad: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['clicked'])),
                ad,
                fact;

            var load = function(context) {
                context = context || 'message';

                getData('ad', {context: context}, {
                    success: function(data) {
                        ad = data.ad;
                        fact = data.fact;

                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        ad = magma.fallback.ad[context];
                        observable.notifyObservers('loadedError');
                    },
                    fatal: function() {
                        ad = magma.fallback.ad[context];
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            var getProperty = function(obj, property) {
                if(obj !== ad && obj !== fact) {
                    throw new Error('getProperty: must provide ad or fact object.');
                }

                if(obj) {
                    if(obj.hasOwnProperty(property)) {
                        return obj[property];
                    } else {
                        throw new Error('getProperty: no property ' + property + '.');
                    }
                }
            };

            return {
                loadAd: load,
                hasAd: function() {
                    return ad ? true : false;
                },
                hasFact: function() {
                    return fact ? true : false;
                },
                getAd: function() {
                    if(!ad) {
                        throw new Error('getAd: no ad to get!');
                    }
                    return ad;
                },
                getFact: function() {
                    if(!fact) {
                        throw new Error('getFact: no fact to get!');
                    }
                    return fact;
                },
                getAdHref: function() {
                    return getProperty(ad, 'href');
                },
                clicked: function() {
                    observable.notifyObservers('clicked');
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** alrt ***/
        alrt: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['open'])),
                open = false;

            var get = function() {
                getData('alert.list', null, {
                    success: function(data) {
                        // group alerts by type
                        var groups = {};

                        $.each(data, function() {
                            if(!groups[this.type]) {
                                groups[this.type] = [];
                            }

                            groups[this.type].push({
                                alertID: this.alertID,
                                message: this.message,
                                date: this.date
                            });
                        });

                        observable.notifyObservers('loaded', groups);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failed: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                getAlerts: get,
                open: function() {
                    observable.notifyObservers('open');
                },
                isOpen: function() {
                    return open;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** aliases ***/
        aliases: function() {
            var observable = newObservable(listEvents(['loaded']));

            var aliasesToOptions = function(data) {
                data.sort(function(a, b) {
                    if(a.name.toLowerCase() === b.name.toLowerCase()) {
                        return 0;
                    }
                    return a.name.toLowerCase() > b.name.toLowerCase() ? 1 : -1;
                });

                for(var i in data) {
                    data[i].value = data[i].email;
                    data[i].text = data[i].name + ' <' + data[i].email + '>';
                }

                return data;
            };

            var load = function(transform) {
                transform = transform || {};

                getData('aliases', {}, {
                    success: function(data) {
                        if(transform['options']) {
                            data = aliasesToOptions(data);
                        }
                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                loadAliases: load,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** auth ***/
        auth: function() {
            var observable = newObservable([
                    'success',
                    'failed',
                    'incorrect',
                    'noUsername',
                    'noPassword',
                    'locked'
                ].concat(listEvents([
                    'logout'
                ]))),
                message;

            var login = function(username, password) {
                if(!username || !password) {
                    if(!username && !password) {
                        message = 'Username and password are required.';
                        observable.notifyObservers('failed', message);
                        observable.notifyObservers('noUsername');
                    } else if(!username) {
                        message = 'Username is required.';
                        observable.notifyObservers('failed', message);
                        observable.notifyObservers('noUsername');
                    } else if(!password) {
                        message = 'Password is required.';
                        observable.notifyObservers('failed', message);
                        observable.notifyObservers('noPassword');
                    }
                } else {
                    getData('auth', {username: username, password: password}, {
                        success: function(data) {
                            switch(data.auth) {
                                case 'success':
                                    magma.session.set(data.session);
                                    observable.notifyObservers('success');
                                break;

                                case 'failed':
                                    observable.notifyObservers('failed', data.message);
                                    observable.notifyObservers('incorrect');
                                break;

                                case 'locked':
                                    observable.notifyObservers('locked', data.message);
                                break;
                            }
                        },
                        error: function(data) {
                            observable.notifyObservers('failed', 'Could not connect to the server');
                        },
                        fatal: function() {
                            // display error in login dialog
                            observable.notifyObservers('failed', 'Could not connect to the server');
                        }
                    });
                }
            };

            var logout = function() {
                getData('logout', null, {
                    success: function() {
                        // clear session
                        magma.session.set('');

                        observable.notifyObservers('logout');
                    },
                    error: function() {
                        observable.notifyObservers('logoutError');
                    },
                    failed: function() {
                        observable.notifyObservers('logoutFailed');
                    }
                });
            };

            return {
                login: login,
                logout: logout,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** compose ***/
        compose: function() {
            var observable = newObservable(listEvents(['ready']).concat(['send'])),
                id;

            // gets an id to keep track of drafts / added attachments / etc
            var compose = function() {
                getData('messages.compose', null, {
                    success: function(data) {
                        id = data.composeID;
                        observable.notifyObservers('ready');
                    },
                    error: function() {
                        observable.notifyObservers('readyError');
                    },
                    failure: function() {
                        observable.notifyObservers('readyFailed');
                    }
                });
            };

            return {
                composeMessage: compose,
                sendMessage: function() {
                    observable.notifyObservers('send');
                },
                getComposeID: function() {
                    return id;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** contact info ***/
        contactInfo: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['new', 'edit', 'show', 'hide'])),
                open = false;

            var load = function(contactID) {
                if(typeof contactID !== 'number') {
                    throw new Error('contactInfo.load: must provide a contactID');
                }

                getData('contacts.load', {contactID: contactID}, {
                    success: function(data) {
                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };
                
            return {
                loadContact: load,
                newContact: function() {
                    observable.notifyObservers('new');
                },
                editContact: function() {
                    observable.notifyObservers('edit');
                },
                isOpen: function() {
                    return open;
                },
                showInfo: function(type) {
                    open = true;
                    observable.notifyObservers('show', type);
                },
                hideInfo: function() {
                    open = false;
                    observable.notifyObservers('hide');
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** contact list ***/
        contactList: function() {
            var observable = newObservable(listEvents(['loaded', 'deleted']));

            var load = function(folder) {
                var folderID = folder;

                if(typeof folder === 'string') {
                    folderID = permaFolders.getID(folder);
                }

                if(typeof folderID !== 'number') {
                    throw new Error('model.contactList: requires folderID.');
                }

                getData('contacts.list', {folderID: folderID}, {
                    success: function(data) {
                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            var del = function(ids) {
                getData('contacts.remove', {contactIDs: ids}, {
                    success: function(data) {
                        observable.notifyObservers('deleted', ids);
                    },
                    error: function(error) {
                        observable.notifyObservers('deletedError', error);
                    },
                    failure: function() {
                        observable.notifyObservers('deletedFailed');
                    }
                });
            };

            return {
                load: load,
                deleteContacts: del,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** folder list ***/
        folders: function(context) {
            var observable = newObservable(listEvents(['loaded', 'added', 'removed', 'renamed', 'moved'])),
                folders = (function() {
                    var permanent = {}, // holds permanent named folders
                        custom = {}; // holds user defined folders

                    return {
                        add: function(name, folderID, parentID) {
                            // check unique
                            if(this.hasFolder(folderID)) {
                                throw new Error('model.folders folders.add: folderID ' + folderID + ' already set');
                            }

                            var folder = $.extend({name: name}, parentID ? {parentID: parentID} : {}),
                                slug = slugify(name);

                            // look for permanent folder names
                            if(permaFolders.getNames(context).join(' ').match(slug)) {
                                permaFolders.setID(slug, folderID);
                                permanent[folderID] = folder;
                            } else {
                                custom[folderID] = folder;
                            }

                            // use method to handle permant/custom discrepancy
                            return this.getFolder(folderID);
                        },
                        remove: function(folderID) {
                            if(this.hasFolder(folderID, 'permanent')) {
                                throw new Error('model.folders folders.remove: cannot remove permanent folder');
                            }

                            if(this.hasFolder(folderID, 'custom')) {
                                delete custom[folderID];

                                // look for children
                                for(var id in custom) {
                                    if(custom[id].parentID === folderID) {
                                        delete custom[id];
                                    }
                                }
                                return;
                            }

                            throw new Error('model.folders folders.remove: folder ' + folderID + ' not found');
                        },
                        move: function(folderID, parentID) {
                            // TODO: folder location data structure
                        },
                        rename: function(name, folderID) {
                            if(this.hasFolder(folderID, 'permanent')) {
                                throw new Error('model.folders folders.remove: cannot rename permanent folder');
                            }

                            if(this.hasFolder(folderID, 'custom')) {
                                custom[folderID].name = name;
                                return this.getFolder(folderID);
                            }

                            throw new Error('model.folders folders.remove: folder ' + folderID + ' not found');
                        },
                        hasFolder: function(folderID, type) {
                            if(type === 'permanent') {
                                return permanent[folderID] ? true : false;
                            } else if(type === 'custom') {
                                return custom[folderID] ? true : false;
                            } else {
                                return permanent[folderID] || custom[folderID] ? true : false;
                            }
                        },
                        getFolder: function(folderID) {
                            if(this.hasFolder(folderID, 'permanent')) {
                                var slug = permanent[folderID].name,
                                    folder = {};

                                folder[slug] = $.extend({folderID: folderID}, permanent[folderID]);
                                return folder;
                            } else if(this.hasFolder(folderID, 'custom')) {
                                return $.extend({folderID: folderID}, custom[folderID]);
                            }
                        },
                        getFolderList: function() {
                            var perm = {},
                                cust = [],
                                id;

                            for(id in permanent) {
                                var camel = camelCasify(permanent[id].name);

                                // not parsing int leaves id as a string and ruins subfolders
                                perm[camel] = $.extend({folderID: parseInt(id, 10)}, permanent[id]);
                            }

                            for(id in custom) {
                                // not parsing int leaves id as a string and ruins subfolders
                                cust.push($.extend({folderID: parseInt(id, 10)}, custom[id]));
                            }

                            return {permanent: perm, custom: cust};
                        },
                        empty: function() {
                            return (function(objs) {
                                for(var i in objs) {
                                    for(var key in objs[i]) {
                                        if(objs[i].hasOwnProperty(key)) {
                                            return false;
                                        }
                                    }
                                }
                                return true;
                            }([permanent, custom]));
                        },
                        reset: function() {
                            permanent = {};
                            custom = {};
                        }
                    };
                }());

            // default context
            context = context || 'mail';

            if(typeof context !== 'string') {
                throw new Error('view.folders: must provide folder context as a string');
            }

            if(typeof context !== 'string') {
                throw new Error('view.folders: must provide folder context as a string');
            }

            var cleanName = function(method, name) {
                if(!name || typeof name !== 'string') {
                    throw new Error('model.folders.' + method + ': must provide a name');
                }
            };

            var cleanID = function(method, id) {
                if(typeof id !== 'number') {
                    throw new Error('model.folders.' + method + ': must provide a folderID');
                }
            };

            var listToTree = function(list) {
                // create a folder tree
                // loop with while to deal with splicing during loop
                var i = 0,
                    scan = function(folders) {
                        for(var j in folders) {
                            if(folders[j].folderID === list[i].parentID) {
                                // init if no folders yet
                                folders[j].subfolders = folders[j].subfolders || [];
                                folders[j].subfolders.push(list[i]);

                                // remove folder from root
                                list.splice(i, 1);

                                return true; // success!

                            // scan subfolders
                            } else if(folders[j].subfolders) {
                                if(scan(folders[j].subfolders)) {
                                    // found parent in subfolder
                                    return true;
                                }
                            }
                        }
                    };

                while(list[i]) {
                    // child folder
                    if(typeof list[i].parentID === "number") {
                        // find child's parent
                        if(!scan(list)) {
                            // goodbye little orphan...
                            list.splice(i, 1);
                        }

                    // increment since nothing removed
                    } else {
                        i += 1;
                    }
                }

                return list;
            };

            var listToAlphaOrder = function(list) {
                list.sort(function(a, b) {
                    if(a.name.toLowerCase() === b.name.toLowerCase()) {
                        return 0;
                    }
                    return a.name.toLowerCase() > b.name.toLowerCase() ? 1 : -1;
                });

                return list;
            };

            var listToOptions = function(list) {
                list = listToAlphaOrder(list);

                for(var i in list) {
                    list[i].value = 'folder-' + list[i].folderID;
                }

                return list;
            };

            var transform = function(folders, transformOption) {
                transformOption = transformOption || {};

                if(transformOption['options']) {
                    folders.custom = listToOptions(folders.custom);
                } else if(transformOption['alphabetical']) {
                    folders.custom = listToAlphaOrder(folders.custom);
                } else if(!transformOption['none']) {
                    folders.custom = listToTree(folders.custom);
                }

                return folders;
            };

            var load = function(transformOption) {
                if(folders.empty()) {
                    getData('folders.list', {context: context}, {
                        success: function(data) {
                            $.each(data, function(i, folder) {
                                folders.add(folder.name, folder.folderID, folder.parentID);
                            });

                            observable.notifyObservers('loaded', transform(folders.getFolderList(), transformOption));
                        },
                        error: function() {
                            observable.notifyObservers('loadedError');
                        },
                        failure: function() {
                            observable.notifyObservers('loadedFailed');
                        }
                    });
                } else {
                    observable.notifyObservers('loaded', transform(folders.getFolderList(), transformOption));
                }
            };

            var add = function(name, parentID) {
                cleanName('add', name);

                if(permaFolders.getNames(context).join(' ').match(name)) {
                    observable.notifyObservers('addedError', name + ' is a reserved folder name');
                } else {
                    getData('folders.add', $.extend({name: name}, parentID ? {parentID: parentID} : {}), {
                        success: function(data) {
                            if(data['folders.add'] !== 'failed') {
                                var folder = folders.add(name, data.folderID, parentID);
                                observable.notifyObservers('added', folder);
                            } else {
                                observable.notifyObservers('addedError', data.message);
                            }
                        },
                        error: function(data) {
                            observable.notifyObservers('addedError', data.error);
                        }
                    });
                }
            };

            var remove = function(folderID) {
                cleanID('remove', folderID);

                // make sure it exists
                var folder = folders.getFolder(folderID);

                getData('folders.remove', {folderID: folderID}, {
                    success: function(data) {
                        try {
                            folders.remove(folderID);
                            observable.notifyObservers('removed', {folderID: folder.folderID, parentID: folder.parentID});
                        } catch(e) {
                            observable.notifyObservers('removedError', e.message);
                        }
                    },
                    error: function(data) {
                        observable.notifyObservers('removedError', data.error);
                    }
                });
            };

            var rename = function(name, folderID) {
                cleanName('rename', name);
                cleanID('remove', folderID);

                getData('folders.rename', {name: name, folderID: folderID}, {
                    success: function(data) {
                        try {
                            var folder = folders.rename(name, folderID);
                            observable.notifyObservers('renamed', folder);
                        } catch(e) {
                            observable.notifyObservers('renamedError', e.message);
                        }
                    },
                    error: function(data) {
                        observable.notifyObservers('renamedError', data.error);
                    }
                });
            };

            var move = function(sourceID, targetID) {
                cleanID('move', sourceID);
                cleanID('move', targetID);

                var params = {sourceFolderID: sourceID, targetFolderID: targetID};

                getData('folders.move', params, {
                    success: function(data) {
                        observable.notifyObservers('moved', params);
                    },
                    error: function(error) {
                        observable.notifyObservers('movedError', {folderID: sourceID, errer: error});
                    },
                    failure: function() {
                        observable.notifyObservers('movedFailed');
                    }
                });
            };

            var change = function(newContext, transformOption) {
                if(typeof newContext !== 'string') {
                    throw new Error('model.folders.change: must provide new context as string');
                }

                context = newContext;
                folders.reset();
                load(transformOption);
            };

            return {
                loadFolders: load,
                changeContext: change,
                getContext: function() {
                    return context;
                },
                addFolder: add,
                removeFolder: remove,
                renameFolder: rename,
                moveFolder: move,
                listFolders: function(transformOption) {
                    return transform(folders.getFolderList(), transformOption);
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** global nav ***/
        globalNav: function() {
            var observable = newObservable(['open']),
                openStates = {
                    mail: true,
                    contacts: false,
                    options: false,
                    logs: false,
                    help: false
                };

            var open = function(loc, init) {
                if(typeof openStates[loc] === 'undefined') {
                    throw new Error('model.globalNav: undefined location');
                }

                observable.notifyObservers('open', {loc: loc, init: init});

                for(var prop in openStates) {
                    if(prop === loc) {
                        openStates[prop] = true;
                    } else {
                        openStates[prop] = false;
                    }
                }
            };

            return {
                isOpen: function(loc) {
                    if(typeof openStates[loc] === 'undefined') {
                        throw new Error('model.globalNav.isOpen: undefined location');
                    }
                    return openStates[loc];
                },
                openLocation: open,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** loading ***/
        loading: function() {
            var observable = newObservable(['finished', 'continue']),
                callbacks = [],
                items = 0;

            var complete = function() {
                items -= 1;

                // call the finished function if all the items are complete
                if(!items) {
                    observable.notifyObservers('finished');
                }
            };

            // items are ananymous functions
            var register = function(callback) {
                // add an function to the list of callbacks
                callbacks.push(callback);
                items += 1;
            };

            var begin = function() {
                // call each item
                $.each(callbacks, function(i, callback) {
                    callback(complete);
                });
            };

            var cont = function() {
                observable.notifyObservers('continue');
            };

            return {
                registerItem: register,
                begin: begin,
                cont: cont,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },


        /*** help ***/
        help: function() {
            var observable = newObservable(listEvents([
                    'updateCategories',
                    'updateTopics',
                    'updatePage'
                ])),
                currentCategory,
                currentTopic,
                self;

            var updateCategories = function() {
                getData('folders.list', {context: 'help'}, {
                    success: function(data) {
                        // convert to helpList template context
                        $.each(data, function() {
                            this.id = this.folderID;
                        });

                        observable.notifyObservers('updateCategories', data);
                    },
                    error: function(error) {
                        observable.notifyObservers('updateCategoriesError', error);
                    },
                    failed: function() {
                        observable.notifyObservers('updateCategoriesFailed');
                    }
                });
            };

            var updateTopics = function(id) {
                if(typeof id !== 'number') {
                    throw new Error('model.help.updateTopics: must privide an id');
                }

                // don't update if same category clicked
                if(id !== currentCategory) {
                    getData('help.topics', {categoryID: id}, {
                        success: function(data) {
                            currentCategory = id;

                            // convert to helpList template context
                            $.each(data, function() {
                                this.id = this.topicID;
                            });

                            observable.notifyObservers('updateTopics', data);
                        },
                        error: function(error) {
                            observable.notifyObservers('updateTopicsError', error);
                        },
                        failed: function() {
                            observable.notifyObservers('updateTopicsFailed');
                        }
                    });
                }
            };

            var updatePage = function(id) {
                if(typeof id !== 'number') {
                    throw new Error('model.help.updatePage: must privide an id');
                }

                // don't update if same category clicked
                if(id !== currentTopic) {
                    getData('help.page', {topicID: id}, {
                        success: function(data) {
                            currentTopic = id;
                            observable.notifyObservers('updatePage', data);
                        },
                        error: function(error) {
                            observable.notifyObservers('updatePageError', error);
                        },
                        failed: function() {
                            observable.notifyObservers('updatePageFailed');
                        }
                    });
                }
            };

            var init = function() {
                self.observeOnce('updateCategories', function(categories) {
                    self.observeOnce('updateTopics', function(topics) {
                        // load first page
                        self.updatePage(topics[0].topicID);
                    });

                    // load the first category
                    self.updateTopics(categories[0].folderID);
                });

                // load the categories
                self.updateCategories();
            };

            self = {
                updateCategories: updateCategories,
                updateTopics: updateTopics,
                updatePage: updatePage,
                initHelp: init,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
            return self;
        },

        /*** logs ***/
        logs: function() {
            var observable = newObservable(listEvents(['loaded']));

            var load = function(display) {
                var context;

                if(typeof display === 'string') {
                    context = display;
                } else if(typeof display === 'number') {
                    context = permaFolders.getName(display);
                } else {
                    throw new Error('model.logs.load: must either provide an id or settings context ' + display);
                }

                // multiple word context use lowercase with underscores
                getData('logs.' + context.replace(/-/, '_'), null, {
                    success: function(data) {
                        $.each(data, function(i) {
                            data[i].time = new Date(data[i].utc).toLocaleString();
                            data[i].size = bytesToSize(data[i].bytes, 2);
                        });

                        observable.notifyObservers('loaded', {context: camelCasify(context), data: data});
                    },
                    error: function(error) {
                        observable.notifyObservers('loadedError', error);
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                load: load,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** message ***/
        message: function(messageID) {
            if(typeof messageID !== 'number') {
                throw new Error('model.messages.load: must provide a messageID');
            }

            var observable = newObservable(listEvents([
                    'headerLoaded',
                    'bodyLoaded',
                    'infoLoaded',
                    'attachmentsLoaded'
                ]).concat([
                    'messageLoaded',
                    'showInfo',
                    'closeInfo',
                    'showAttachments',
                    'closeAttachments'
                ])),
                message = {},
                infoOpen = false,
                attachmentsOpen = false;

            var load = function(section) {
                if(!(section instanceof Array)) {
                    throw new Error('model.messages.load: section list must be an array');
                }

                for(var i in section) {
                    if(typeof section[i] !== 'string') {
                        throw new Error('model.messages.load: section list must contain strings');
                    }
                }

                getData('messages.load', {messageID: messageID, section: section}, {
                    success: function(data) {
                        for(var prop in data) {
                            message[prop] = data[prop];
                        }
                        for(var i in section) {
                            observable.notifyObservers(section[i] + 'Loaded');
                        }
                        observable.notifyObservers('messageLoaded');
                    },
                    error: function() {
                        observable.notifyObservers(section + 'LoadedError');
                    },
                    failure: function() {
                        observable.notifyObservers(section + 'LoadedFailed');
                    }
                });
            };

            return {
                getID: function() {
                    return messageID;
                },
                loadSections: load,
                hasSection: function(section) {
                    return message[section] ? true : false;
                },
                getSection: function(section) {
                    if(!message[section]) {
                        throw new Error('model.message: does not contain section ' + section);
                    }
                    return message[section];
                },
                hasField: function(section, field) {
                    if(message[section]) {
                        return message[section][field] ? true : false;
                    } else {
                        return false;
                    }
                },
                getField: function(section, field) {
                    if(!message[section]) {
                        throw new Error('model.message: does not contain section ' + section);
                    } else if(!message[section][field]) {
                        throw new Error('model.message: does not contain field ' + field);
                    }
                    return message[section][field];
                },
                showInfo: function() {
                    infoOpen = true;
                    observable.notifyObservers('showInfo');
                },
                closeInfo: function() {
                    infoOpen = false;
                    observable.notifyObservers('closeInfo');
                },
                infoIsOpen: function() {
                    return infoOpen;
                },
                showAttachments: function() {
                    attachmentsOpen = true;
                    observable.notifyObservers('showAttachments');
                },
                closeAttachments: function() {
                    attachmentsOpen = false;
                    observable.notifyObservers('closeAttachments');
                },
                attachmentsIsOpen: function() {
                    return attachmentsOpen;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** message list ***/
        messageList: function() {
            var observable = newObservable(listEvents([
                    'loaded',
                    'deleted',
                    'flagged',
                    'copied',
                    'moved',
                    'tagged'
                ])),
                messages = (function() {
                    var self,
                        _messages = {}, // {{message: message, refCount: num}, ...}
                        _folders = {}, // {folderID: messageList, folderID: messsageList, ...}
                        _flags = ['seen', 'flagged', 'answered'],
                        _indexOf = function(messageID, folderID) {
                            if(self.hasMessage(messageID, folderID)) {
                                return _folders[folderID].indexOf(_messages[messageID]);
                            }
                        };

                    self = {
                        add: function(messageID, folderID, message) {
                            if(this.hasMessage(messageID)) {
                                throw new Error('messsageList messages.add: message id ' + messageID + ' already exists');
                            }

                            if(!this.hasFolder(folderID)) {
                                this.initFolder(folderID);
                            }

                            _messages[messageID] = message;
                            this.updateFlags(messageID);
                            _folders[folderID].push(_messages[messageID]);
                        },
                        remove: function(messageID, folderID) {
                            var index;
                            
                            if(this.hasMessage(messageID, folderID)) {
                                index = _indexOf(messageID, folderID);
                            }

                            if(index === -1) {
                                throw new Error('messsageList messages.remove: id ' + messageID + ' does not exists in ' + folderID);
                            }

                            _folders[folderID].splice(index, 1);
                            delete _messages[messageID];
                        },
                        move: function(messageID, oldFolderID, newFolderID) {
                            var index;

                            if(this.hasMessage(messageID, oldFolderID)) {
                                index = _indexOf(messageID, oldFolderID);
                            }

                            if(index === -1) {
                                throw new Error('messsageList messages.move: id ' + messageID + ' does not exists in ' + oldFolderID);
                            }

                            // temp copy of message
                            var message = _messages[messageID];
                            this.remove(messageID, oldFolderID);
                            this.add(messageID, newFolderID, message);
                        },
                        update: function(id, folderID, message) {
                            if(this.hasMessage(id, folderID)) {
                                // copy message instead of asigning to replicate changes to folder
                                for(var prop in _messages[id]) {
                                    _messages[id][prop] = message[prop];
                                }
                                this.updateFlags(id);
                                return true;
                            } else {
                                return false;
                            }
                        },
                        hasMessage: function(messageID, folderID) {
                            if(typeof folderID === 'number') {
                                if(this.hasMessage(messageID)) {
                                    return _folders[folderID].indexOf(_messages[messageID]) >= 0 ? true : false;
                                } else {
                                    return false;
                                }
                            } else {
                                return _messages[messageID] ? true : false;
                            }
                        },
                        updateFlags: function(id) {
                            if(this.hasMessage(id)) {
                                $.each(_flags, function(i, flag) {
                                    _messages[id].flag[flag] = false;
                                });

                                $.each(_messages[id].flags, function(i, flag) {
                                    _messages[id].flag[flag] = true;
                                });
                            }
                        },
                        getMessage: function(id) {
                            if(!this.hasMessage(id)) {
                                throw new Error('messsageList messages.getMessage: no message ' + id);
                            }
                            return $.extend(true, {}, _messages[id]);
                        },
                        getMessages: function(ids) {
                            var messages = [];

                            for(var i in ids) {
                                if(!this.hasMessage(ids[i])) {
                                    throw new Error('messsageList messages.getMessages: no message ' + ids[i]);
                                }
                                messages.push(this.getMessage(ids[i]));
                            }

                            return messages;
                        },
                        initFolder: function(id) {
                            if(this.hasFolder(id)) {
                                throw new Error('messsageList messages.initFolder: folder ' + id + ' already exists');
                            }
                            _folders[id] = [];
                        },
                        hasFolder: function(id) {
                            return _folders[id] ? true : false;
                        },
                        getFolder: function(id) {
                            return _folders[id].slice(0);
                        }
                    };

                    return self;
                }()),
                currentFolder;

            var load = function(folder) {
                var folderID = folder;

                if(typeof folder === 'string') {
                    folderID = permaFolders.getID(folder);
                }

                if(typeof folderID !== 'number') {
                    throw new Error('model.messages.list: requires folderID.');
                }

                getData('messages.list', {folderID: folderID}, {
                    success: function(data) {
                        // must be called before filling folder
                        // this creates an empty folder
                        // avoids error when valid request for empty folder is made
                        if(!messages.hasFolder(folderID)) {
                            messages.initFolder(folderID);
                        }

                        // transformations
                        $.each(data, function(i, message) {
                            message.flags = message.flags || [];
                            message.flag = {};
                            message.tags = message.tags || [];
                            message.attachment = message.attachment || false;

                            // convert unix time and bytes to human readable strings
                            message.date = new Date(message.utc).toLocaleString();
                            message.arrivalDate = new Date(message.arrivalUtc).toLocaleString();
                            message.size = bytesToSize(message.bytes, 2);

                            $.each(message.tags.sort(ignoreCaseSort), function(j, tag) {
                                message.tags[j] = {name: tag, slug: slugify(tag)};
                            });

                            if(!messages.update(message.messageID, folderID, message)) {
                                messages.add(message.messageID, folderID, message);
                            }
                        });

                        // save current folder for operations
                        currentFolder = folderID;

                        observable.notifyObservers('loaded', messages.getFolder(folderID));
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            var del = function(messageIDs) {
                if(!(messageIDs instanceof Array)) {
                    throw new Error('model.messsageList.deleteMessage: must provide messageIDs in an array.');
                }

                if(!messageIDs.length) {
                    observable.notifyObservers('deletedError', 'Please check messages to delete.');
                }

                // ignore request if emtpy
                if(messageIDs.length) {
                    getData('messages.remove', {messageIDs: messageIDs, folderID: currentFolder}, {
                        success: function() {
                            $.each(messageIDs, function(i, id) {
                                messages.remove(id, currentFolder);
                            });

                            observable.notifyObservers('deleted', messageIDs);
                        },
                        error: function(error) {
                            observable.notifyObservers('deletedError', error);
                        },
                        failure: function() {
                            observable.notifyObservers('deletedFailed', messageIDs);
                        }
                    });
                }
            };

            // handles junk and mark
            var flag = function(messageIDs, action) {
                if(!(messageIDs instanceof Array)) {
                    throw new Error('model.messsageList.flagMessages: must provide messageIDs in an array.');
                }

                if(messageIDs.length) {                
                    var flag,
                        ids = [],
                        match,
                        params;

                    // use IMAP terms
                    if(action.match(/read|unread/i)) {
                        flag = 'seen';
                        match = /seen/i;
                    } else if(action.match(/star|unstar/i)) {
                        flag = 'flagged';
                        match = /flagged/i;
                    }

                    // look for removal types
                    if(action.match(/^unread$|^unstar$/i)) {

                        // narrow down request to applicable messageIDs
                        $.each(messageIDs, function(i, id) {
                            if(messages.hasMessage(id) && messages.getMessage(id).flags.length) {

                                // send only ids that have tag to be removed
                                $.each(messages.getMessage(id).flags, function(i, _flag) {
                                    if(_flag.match(match)) {
                                        ids.push(id);
                                    }
                                });
                            }
                        });

                        if(ids.length) {
                            params = {messageIDs: ids, flags: [flag], action: 'remove', folderID: currentFolder};

                            getData('messages.flag', params, {
                                success: function() {
                                    // update messages
                                    $.each(ids, function(i, id) {
                                        $.each(messages.getMessage(id).flags, function(i, _flag) {
                                            if(_flag.match(match)) {
                                                var message = messages.getMessage(id);
                                                message.flags.splice(i, 1);
                                                messages.update(id, currentFolder, message);
                                            }
                                        });
                                    });

                                    observable.notifyObservers('flagged', {messageIDs: ids, data: messages.getMessages(ids)});
                                },
                                error: function() {
                                    observable.notifyObservers('flaggedError', ids);
                                },
                                failure: function() {
                                    observable.notifyObservers('flaggedFailed', ids);
                                }
                            });
                        }

                    // else flags will be added
                    } else {
                        // only apply tag if not already present
                        $.each(messageIDs, function(i, id) {
                            var _flags = [];

                            if(messages.hasMessage(id) && messages.getMessage(id).flags.length) {
                                // build matchable flag string
                                $.each(messages.getMessage(id).flags, function(i, _flag) {
                                    _flags.push(_flag);
                                });
                            }

                            if(!_flags.join(' ').match(match)) {
                                ids.push(id);
                            }
                        });

                        if(ids.length) {
                            params = {messageIDs: ids, flags: [flag], action: 'add', folderID: currentFolder};

                            getData('messages.flag', params, {
                                success: function() {
                                    // update messages
                                    $.each(ids, function(i, id) {
                                        var message = messages.getMessage(id);
                                        message.flags.push(flag);
                                        messages.update(id, currentFolder, message);
                                    });

                                    observable.notifyObservers('flagged', {messageIDs: ids, data: messages.getMessages(ids)});
                                },
                                error: function() {
                                    observable.notifyObservers('flaggedError', ids);
                                },
                                failure: function() {
                                    observable.notifyObservers('flaggedFailed', ids);
                                }
                            });
                        }
                    }

                // empty messageIDs is invalid
                } else {
                    throw new Error('model.messageList messages.flag: Empty messageIDs');
                }
            };

            var copy = function(messageIDs, folderID) {
                if(!(messageIDs instanceof Array)) {
                    throw new Error('model.messsageList.copyMessages: must provide messageIDs in an array.');
                }

                if(typeof folderID !== 'number') {
                    throw new Error('model.messageList.copyMessages: must provide a folderID');
                }

                if(currentFolder === folderID) {
                    throw new Error('model.messsageList copyMessages: cannot copy to the same folder');
                }

                if(messageIDs.length) {
                    var params = {messageIDs: messageIDs, sourceFolderID: currentFolder, targetFolderID: folderID};

                    getData('messages.copy', params, {
                        // contains old and new messageIDs
                        success: function(messageIDs) {
                            var msgs = [];

                            $.each(messageIDs, function(i, id) {
                                if(messages.hasMessage(id.sourceMessageID)) {
                                    var newMessage = messages.getMessage(id.sourceMessageID);
                                    newMessage.messageID = id.targetMessageID;
                                    messages.add(id.targetMessageID, folderID, newMessage);

                                    msgs.push(newMessage);
                                } else {
                                    throw new Error('copyMessages: source message id ' + id.sourceMessageID + ' does not exist');
                                }
                            });
                        },
                        error: function(errorMessage) {
                            observable.notifyObservers('copiedError', [errorMessage, params]);
                        },
                        failure: function() {
                            observable.notifyObservers('copiedFailed', params);
                        }
                    });
                }
            };

            var move = function(messageIDs, folderID) {
                if(!(messageIDs instanceof Array)) {
                    throw new Error('model.messsageList.moveMessages: must provide messageIDs in an array.');
                }

                if(typeof folderID !== 'number') {
                    throw new Error('model.messageList.moveMessages: must provide a folderID');
                }

                if(currentFolder === folderID) {
                    throw new Error('model.messsageList moveMessages: cannot move to the same folder');
                }

                var params = {messageIDs: messageIDs, sourceFolderID: currentFolder, targetFolderID: folderID};

                getData('messages.move', params, {
                    success: function(data) {
                        $.each(messageIDs, function(i, id) {
                            // remove message from folder
                            messages.move(id, currentFolder, folderID);
                        });

                        observable.notifyObservers('moved', messageIDs);
                    },
                    error: function(data) {
                        observable.notifyObservers('movedError', data);
                    },
                    failure: function() {
                        observable.notifyObservers('movedFailed', params);
                    }
                });
            };

            var tag = function(messageIDs, tags) {
                // convert tags to array if just a single tag passed
                if(typeof tags === 'string') {
                    tags = [tags];
                }

                if(!(tags instanceof Array)) {
                    throw new Error('model.messsageList.tagMessages: must provide tags as an array of objects.');
                }

                var params = {messageIDs: messageIDs, tag: tags};

                getData('messages.tag', params, {
                    success: function() {
                        // update messages
                        $.each(messages.getMessages(messageIDs), function(i, message) {
                            $.each(tags, function(i, tag) {
                                message.tags.push({slug: slugify(tag), name: tag});
                            });

                            messages.update(message.messageID, currentFolder, message);
                        });

                        observable.notifyObservers('tagged', {messageIDs: messageIDs, data: messages.getMessages(messageIDs)});
                    },
                    error: function() {
                        observable.notifyObservers('taggedError', tags);
                    },
                    failure: function() {
                        observable.notifyObservers('taggedFailed', tags);
                    }
                });
            };

            return {
                load: load,
                del: del,
                flag: flag,
                copy: copy,
                move: move,
                tag: tag,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** meta ***/
        meta: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['open'])),
                open = false;

            var get = function() {
                getData('meta', null, {
                    success: function(data) {
                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failed: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                getMeta: get,
                open: function() {
                    observable.notifyObservers('open');
                },
                isOpen: function() {
                    return open;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** model ***/
        model: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['open'])),
                open = false;

            var get = function() {
                getData('meta', null, {
                    success: function(data) {
                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failed: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                getMeta: get,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },


        /*** tab ***/
        tab: function(title, id, permatab) {
            if(!title) {
                throw new Error('model.tab: Must provide a title.');
            } else if(typeof title !== 'string') {
                throw new Error('model.tab: Tab title must be given as a string.');
            } else if(typeof id !== 'number') {
                throw new Error('model.tab: Tab id not provided');
            }

            // false if empty
            permatab = permatab || false;
            
            if(typeof permatab !== 'boolean') {
                throw new Error('model.tab: Permatab option must be given as a boolean');
            }

            var observable = newObservable(['focus', 'blur', 'closed', 'remove', 'titleModified']),
                active = false;

            return {
                focus: function() {
                    active = true;
                    observable.notifyObservers('focus', this);
                    return this;
                },
                blur: function() {
                    active = false;
                    observable.notifyObservers('blur', this);
                    return this;
                },
                close: function() {
                    observable.notifyObservers('closed', this);
                },
                remove: function() {
                    observable.notifyObservers('remove', this);
                },
                setTitle: function(t) {
                    if(typeof title !== 'string') {
                        throw new Error('Tab title must be given as a string');
                    }
                    
                    title = t;
                    observable.notifyObservers('titleModified', t);
                },
                getTitle: function() {
                    return title;
                },
                getID: function() {
                    return id;
                },
                isActive: function() {
                    return active;
                },
                isPermatab: function() {
                    return permatab;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** tabs ***/
        tabs: function() {
            var observable = newObservable(['tabAdded', 'tabRemoved', 'show', 'hide']),
                tabs = [],
                open = true,
                params = {
                    observable: observable,
                    models: tabs
                };

            var add = addModel($.extend({
                method: 'addTab',
                event: 'tabAdded'
            }, params));

            var remove = removeModel($.extend({
                method: 'removeTab',
                event: 'tabRemoved'
            }, params));

            return {
                forEach: function(f) {
                    if(!f) {
                        throw new Error('tabs.forEach: must provide a callback function');
                    } else if(typeof f !== 'function') {
                        throw new Error('tabs.forEach: callback must be a function');
                    }
                    for(var i in tabs) {
                        f(tabs[i]);
                    }
                },
                tabCount: function() {
                    return tabs.length;
                },
                hasTabs: function() {
                    return tabs.length ? true : false;
                },
                lastTab: function() {
                    // check for tabs first
                    if(tabs.length === 0) {
                        throw new Error('model.tabs.last: no tabs to return');
                    }
                    return tabs[tabs.length - 1];
                },
                isOpen: function() {
                    return open;
                },
                showTabs: function() {
                    open = true;
                    observable.notifyObservers('show');
                },
                hideTabs: function() {
                    open = false;
                    observable.notifyObservers('hide');
                },
                addTab: add,
                removeTab: remove,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** tags ***/
        tags: function() {
            var observable = newObservable(listEvents(['loaded', 'added']));

            // takes a list of tag names and returns a list of objects with slug/href/name fields
            var toTagList = function(tags) {
                var list = [];

                $.each(tags, function(i, tag) {
                    var slug = slugify(tag);

                    list.push({
                        slug: slug,
                        href: '#' + slug,
                        name: tag
                    });
                });

                return list;
            };

            var get = function() {
                getData('messages.tags', null, {
                    success: function(data) {
                        observable.notifyObservers('loaded', toTagList(data));
                    },
                    error: function(error) {
                        observable.notifyObservers('loadedError');
                    },
                    failed: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            var add = function(name) {
                if(typeof tag !== 'string') {
                    throw new Error('tags.add: tag name must be of type string');
                }

                getData('tags.add', null, {
                    success: function(data) {
                        observable.notifyObservers('added', data);
                    },
                    error: function(error) {
                        observable.notifyObservers('addedError');
                    },
                    failed: function() {
                        observable.notifyObservers('addedFailed');
                    }
                });
            };

            return {
                loadTags: get,
                addTag: add,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** tmpl ***/
        tmpl: function() {
            var observable = newObservable(['completed']);

            var fill = function(template, data) {
                // compile the template if needed
                loadTmpl(template, function() {
                    var jquery = $.tmpl(template, data);
                    observable.notifyObservers('completed', jquery);
                });
            };

            return {
                fillTmpl: fill,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** tools ***/
        tools: function() {
            var observable = newObservable(['enabled', 'disabled', 'showDropdown', 'hideDropdown']);

            return {
                addTool: function(toolname, toggle) {
                    observable.addEvent(toolname + 'Clicked');
                },
                toolClicked: function(toolname, data) {
                    observable.notifyObservers(toolname + 'Clicked', data);
                },
                enableTool: function(toolname) {
                    observable.notifyObservers('enabled', toolname);
                },
                disableTool: function(toolname) {
                    observable.notifyObservers('disabled', toolname);
                },
                showDropdown: function() {
                    observable.notifyObservers('showDropdown');
                },
                hideDropdown: function() {
                    observable.notifyObservers('hideDropdown');
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** scrape contacts ***/
        scrape: function(messageID) {
            var observable = newObservable(['update'].concat(listEvents(['loaded', 'added'])));

            var load = function() {
                getData('scrape', {messageID: messageID}, {
                    success: function(data) {
                        observable.notifyObservers('loaded', data);
                    },
                    error: function(error) {
                        observable.notifyObservers('loadedError', error);
                    },
                    failed: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            var add = function(values) {
                getData('scrape.add', $.extend(values, {messageID: messageID}), {
                    success: function(data) {
                        observable.notifyObservers('added', values.id);
                    },
                    error: function(error) {
                        observable.notifyObservers('addedError', error);
                    },
                    failed: function() {
                        observable.notifyObservers('addedFailed');
                    }
                });
            };

            return {
                load: load,
                add: add,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** search results ***/
        searchResults: function() {
            var observable = newObservable(['loading'].concat(listEvents(['loaded']))),
                loading;

            var search = function(query) {
                // start loading
                loading = true;
                getData('search', query, {
                    success: function(results) {
                        // TODO: remove artificial search results delay
                        setTimeout(function() {
                        loading = false;

                        // transformations
                        $.each(results, function(i, message) {
                            message.flags = message.flags || [];
                            message.flag = {};
                            message.tags = message.tags || [];
                            message.attachment = message.attachment || false;

                            // convert unix time and bytes to human readable strings
                            message.date = new Date(message.utc).toLocaleString();
                            message.arrivalDate = new Date(message.arrivalUtc).toLocaleString();
                            message.size = bytesToSize(message.bytes, 2);

                            $.each(message.tags.sort(ignoreCaseSort), function(j, tag) {
                                message.tags[j] = {name: tag, slug: slugify(tag)};
                            });
                        });

                        observable.notifyObservers('loaded', results);
                        }, 6000);
                    },
                    error: function(error) {
                        loading = false;
                        observable.notifyObservers('loadedError', error);
                    },
                    failed: function() {
                        loading = false;
                        observable.notifyObservers('loadedFailed');
                    }
                });

                // wait to start loading bar
                setTimeout(function() {
                    // TODO: remove progress bar test
                    var progress = 10;

                    var i = setInterval(function() {
                        // get progress
                        // TODO: to get search result loading progress, need to identify search - i.e. return id on initial search
                        if(loading) {
                            if(progress < 100) {
                                progress += 10;
                                observable.notifyObservers('loading', progress);
                            }
                        } else {
                            clearInterval(i);
                        }
                    }, 500);
                }, 500);
            };

            return {
                search: search,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** search option ***/
        searchOption: function(id, row) {
            var observable = newObservable(['remove', 'update', 'rowUpdated', 'controlsUpdated']),
                controls = {add: false, remove: false};

            var remove = function() {
                observable.notifyObservers('remove');
            };

            var update = function(params) {
                observable.notifyObservers('update', params);
            };

            return {
                removeOption: remove,
                updateOption: update,
                getID: function() {
                    return id;
                },
                getRow: function() {
                    return row;
                },
                setRow: function(num) {
                    if(typeof num !== 'number') {
                        throw new Error('model.searchOption.setRow: row must be an integer');
                    }
                    row = num;
                    observable.notifyObservers('rowUpdated', row);
                },
                getControls: function() {
                    return controls;
                },
                setControls: function(ctrls) {
                    ctrls = ctrls || {};
                    if(typeof ctrls.add !== 'boolean') {
                        ctrls.add = false;
                    }
                    if(typeof ctrls.remove !== 'boolean') {
                        ctrls.remove = false;
                    }
                    controls = ctrls;
                    observable.notifyObservers('controlsUpdated', controls);
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** search options ***/
        searchOptions: function() {
            var observable = newObservable(['optionAdded', 'optionRemoved']),
                options = [],
                limit = 5,
                params = {
                    observable: observable,
                    models: options
                };

            var add = addModel($.extend({
                method: 'addOption',
                event: 'optionAdded'
            }, params));

            var remove = removeModel($.extend({
                method: 'removeOption',
                event: 'optionRemoved'
            }, params));

            var updateRows = function() {
                for(var i=0; i<options.length; i += 1) {
                    options[i].updateOption({
                        row: i,
                        singleRow: options.length === 1,
                        atLimit: options.length === limit
                    });
                }
            };

            return {
                addOption: function(searchOptionModel) {
                    if(options.length < limit) {
                        add(searchOptionModel);
                        updateRows();
                    }
                },
                removeOption: function(searchOptionModel) {
                    remove(searchOptionModel);
                    updateRows();
                },
                getCurrRow: function() {
                    return options.length;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** settings ***/
        settings: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['advanced', 'default', 'edit']));

            // handles sending in context or folder id
            var load = function(display) {
                var context;

                if(typeof display === 'string') {
                    context = display;
                } else if(typeof display === 'number') {
                    context = permaFolders.getName(display);
                } else {
                    throw new Error('model.settings.load: must either provide an id or settings context ' + display);
                }

                // multiple word context use lowercase with underscores
                getData('settings.' + context.replace(/-/, '_'), null, {
                    success: function(data) {
                        $.each(data, function(i) {
                            data[i].time = new Date(data[i].utc).toLocaleString();
                            data[i].size = bytesToSize(data[i].bytes, 2);
                        });

                        observable.notifyObservers('loaded', {context: camelCasify(context), data: data});
                    },
                    error: function(error) {
                        observable.notifyObservers('loadedError', error);
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                load: load,
                advancedSettings: function() {
                    observable.notifyObservers('advanced');
                },
                defaultSettings: function() {
                    observable.notifyObservers('defaults');
                },
                editSettings: function() {
                    observable.notifyObservers('edit');
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** workspace ***/
        workspace: (function() {
            // close over id counter
            var ID = 0;

            return function() {
                ID += 1;

                var observable = newObservable(['focus', 'blur', 'closed', 'remove']),
                    id = ID,
                    active = false;

                return {
                    focus: function() {
                        active = true;
                        observable.notifyObservers('focus', this);
                        return this;
                    },
                    blur: function() {
                        active = false;
                        observable.notifyObservers('blur', this);
                        return this;
                    },
                    close: function() {
                        observable.notifyObservers('closed', this);
                    },
                    remove: function() {
                        observable.notifyObservers('remove', this);
                    },
                    getID: function() {
                        return id;
                    },
                    isActive: function() {
                        return active;
                    },
                    addObserver: observable.addObserver,
                    observeOnce: observable.observeOnce,
                    removeObserver: observable.removeObserver
                };
            };
        }()),

        /*** workspaces ***/
        workspaces: function() {
            var observable = newObservable(['workspaceAdded', 'workspaceRemoved', 'dedicatedWorkspaceAdded']),
                workspaces = [],
                lastActive,
                params = {
                    observable: observable,
                    models: workspaces
                };

            var add = addModel($.extend({
                method: 'addWorkspace',
                event: 'workspaceAdded'
            }, params));

            var remove = removeModel($.extend({
                method: 'addWorkspace',
                event: 'workspaceRemoved'
            }, params));

            return {
                forEach: function(f) {
                    if(!f) {
                        throw new Error('workspaces.forEach: must provide a callback function');
                    } else if(typeof f !== 'function') {
                        throw new Error('workspaces.forEach: callback must be a function');
                    }
                    for(var i in workspaces) {
                        f(workspaces[i]);
                    }
                },
                workspaceCount: function() {
                    return workspaces.length;
                },
                hasWorkspace: function() {
                    return workspaces.length ? true : false;
                },
                lastWorkspace: function() {
                    if(workspaces.length === 0) {
                        throw new Error('model.workspaces.last: no workspace to return');
                    }
                    return workspaces[workspaces.length - 1];
                },
                addWorkspace: add,
                removeWorkspace: remove,
                addDedicatedWorkspace: function(workspaceModel) {
                    add(workspaceModel);
                    observable.notifyObservers('dedicatedWorkspaceAdded', workspaceModel);
                },
                saveLastActive: function(workspace) {
                    for(var i in workspaces) {
                        if(workspace === workspaces[i]) {
                            lastActive = i;
                            return;
                        }
                    }
                    throw new Error('saveLastActive: no workspace found.');
                },
                focusLastActive: function() {
                    if(!lastActive) {
                        throw new Error('focusLastActive: lastActive not set.');
                    }
                    workspaces[lastActive].focus();
                    lastActive = undefined;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        }
    };
}());
