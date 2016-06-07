module('message list');

asyncTest('empty list', function() {
    var list = magma.model.messageList();

    list.addObserver('loaded', function(messages) {
        ok(!messages.length, 'Empty message list does not break messageList model');
        auth().logout();
        start();
    });

    list.addObserver('loadedFailed', function() {
        ok(false, 'Failed to load empty message list.');
        auth().logout();
        start();
    });

    list.load(0);
});

asyncTest('load inbox', function() {
    var list = magma.model.messageList(),
        folders = magma.model.folders();

    folders.observeOnce('loaded', function() {
        list.observeOnce('loaded', function(messages) {
            ok(messages.length, 'Loaded inbox');
            start();
        });
       list.load('inbox');
    });

    list.addObserver('loadedFailed', function() {
        ok(false, 'Failed to load messages');
        start();
    });

    folders.addObserver('loadedFailed', function() {
        ok(false, 'Failed to load folders');
        start();
    });

    folders.loadFolders('mail');
});
