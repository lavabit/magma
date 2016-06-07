# Workflow Examples

## Adding HTML Blocks

HTML blocks are located in `dev/blocks`. Block hierarchy is indicated by the filename. The hierarchy shows which files will be embedded into each other.

Blocks can indicate where other blocks can be embedded with the `<!-- [BLOCK] -->` comment. Only one embed statement is allowed per block.

## Compiling Blocks Into States

State files located in `static/states` and `js/states` are used to compile blocks into sets of HTML code. Embedding is indicated with white space (4 spaces, not tabs). For example, with the blocks named:

    container
    container.list
    container.list.item

A state file that embeds the item into the list and list into the container would be:

    container
        list
            item

Also, if you wanted a block with just the list and embedded item (usefull for javascript blocks which only need snippets of html, not complete html files), you would do the following:

    container.list
        item

Running `dev/build/build-html.php` compiles all the states into blocks. (You will need to run `dev/build/build-js.php` following compiling html to update the blocks stored in `dev/js/scripts/blocks.js` for javascript access).

## Views

Views are responsible for creating html containers and managing user interactions. The `getBlock` method lets you grab an html block by name (the name of the state file). For example, if you create a state called list.state:

    container.list
        item

Then run `build-html.php` then `build-js.php` to complile the blocks into an html string and insert them `dev/js/scripts/blocks.js`, you can access the block with the following:

    var list = getBlock('list');

Which returns the jQuery object of the list.html block.

Views will also grab data from models and fill templates. Templates are located in `dev/js/templates/`. See [jQuery templates API](http://api.jquery.com/category/plugins/templates/) for syntax details. Running `dev/build/build-js.php` inserts templates into `dev/js/scripts/tmpl.js`. Use the tmpl model to fill templates. 

Model data is accessed by observing the events that provide it. For example, with template named `listItems`, the following snippet fills the template on the `loaded` event:

    listModel.addObserver('loaded', function(data) {
        var tmplModel = magma.model.tmpl();

        tmplModel.observeOnce('completed', function(list) {
            // do stuff with list jquery object
        });

        tmplModel.fillTmpl('listItems', data);
    });

Views select specific elements to bind actions to or delegate events to handle the user interactions:

    list.delegate('.item', 'click', function() {
        alert($(this).text());
    });

Finally, the root element is returned to interface with the bootstrap explained below:

    return {
        root: list
    };

Putting it together:

    magma.view = {
        list: function(listModel) {
            var list = getBlock('list');

            list.delegate('.item', 'click', function() {
                alert($(this).text());
            });

            listModel.addObserver('loaded', function(data) {
                var tmplModel = magma.model.tmpl();

                tmplModel.observeOnce('completed', function(items) {
                    list.append(items);
                });

                tmplModel.fillTmpl('listItems', data);
            });

            listModel.loadItems(10);

            return {
                root: list
            };
        }
    };

## Models

Models can communicate with views with events passed through the observable object. Events are typically listed when the observerable is created:

    var observable = newObservable(['loaded']);

This creates an observable with two events that can be subscribed to. In some cases, the `listEvents` helper is used to create a set of events:

    var observable = newObservable(listEvents(['loaded']));

`listEvents` expands the array to `['loaded', 'loadedError', 'loadedFailed']` for convenience since server methods will either be successfull, return an error, or fail.

Models also access server data via the JSON interface. `getData` is used to make an AJAX request to server:

    var load = function(amount) {
        getData('list.items', {items: amount}, {
            success: function(data) {
                observable.notifyObservers('loaded', data);
            },
            error: function(error) {
                observable.notifyObservers('loadedError', error);
            },
            failure: function() {
                observable.notifyObservers('loadedFailed');
            }
        });
    };

Any views that subscribe to the `loaded` event will be notified when the data is finished loading and have access to the data object.

All models return on object containing interface methods and subscription methods:

    return {
        loadItems: load,
        addObserver: observable.addObserver,
        observeOnce: observable.observeOnce,
        removeObserver: observable.removeObserver
    };

Putting things together:

    magma.model = {
        list: functions() {
            var observable = newObservable(listEvents(['loaded']));

            var load = function(amount) {
                getData('list.items', {items: amount}, {
                    success: function(data) {
                        observable.notifyObservers('loaded', data);
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
                loadItems: load,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        }
    };

## Bootstrap

The bootstrap methods located in `dev/js/scripts/bootstrap.js` are used to launch tabs or workspaces. These functions glue together specific models and views and add the views to the page.

`login`, `locked`, `logout`, and the `loading` bootstraps are for startup

`main` sets up the chrome and workspace container

`inbox`, `read`, `compose`, `search`, and `scrape` are examples of creating a tabbed workspace

`mail`, `contacts`, `options`, and `help` are global navigation that hide or show the tabs.

For example, a bootstrap method for the list view and model above would look like the following:

    magma.bootstrap = {
        list: function() {
            var listModel = magma.model.list();
            magma.view.list(listModel).root.appendTo(document.body);
        }
    };
