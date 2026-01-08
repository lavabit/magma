/**
 * Folders Model Tests
 */

module('folders');

asyncTest('load mail folders', function() {
    var folders = magma.model.folders();

    folders.addObserver('loaded', function(data) {
        ok(true, 'Folders loaded successfully');
        ok(data.length > 0, 'Got folders data');
        start();
    });

    folders.addObserver('loadedError', function() {
        ok(false, 'Folders load returned error');
        start();
    });

    folders.addObserver('loadedFailed', function() {
        ok(false, 'Folders load failed');
        start();
    });

    folders.loadFolders('mail');
});

asyncTest('load contacts folders', function() {
    var folders = magma.model.folders();

    folders.addObserver('loaded', function(data) {
        ok(true, 'Contact folders loaded successfully');
        start();
    });

    folders.addObserver('loadedFailed', function() {
        ok(false, 'Contact folders load failed');
        start();
    });

    folders.loadFolders('contacts');
});

asyncTest('load settings folders', function() {
    var folders = magma.model.folders();

    folders.addObserver('loaded', function(data) {
        ok(true, 'Settings folders loaded successfully');
        start();
    });

    folders.addObserver('loadedFailed', function() {
        ok(false, 'Settings folders load failed');
        start();
    });

    folders.loadFolders('settings');
});

asyncTest('add folder', function() {
    var folders = magma.model.folders();

    folders.addObserver('loaded', function() {
        folders.addObserver('added', function(data) {
            ok(true, 'Folder added successfully');
            ok(data.folderID, 'Got new folder ID');
            start();
        });

        folders.addObserver('addedError', function() {
            ok(false, 'Add folder returned error');
            start();
        });

        folders.addObserver('addedFailed', function() {
            ok(false, 'Add folder failed');
            start();
        });

        folders.addFolder('Test Folder', 1);
    });

    folders.loadFolders('mail');
});

asyncTest('rename folder', function() {
    var folders = magma.model.folders();

    folders.addObserver('loaded', function(data) {
        if (data.length > 0) {
            var firstFolder = data[0];

            folders.addObserver('renamed', function() {
                ok(true, 'Folder renamed successfully');
                start();
            });

            folders.addObserver('renamedError', function() {
                ok(false, 'Rename folder returned error');
                start();
            });

            folders.addObserver('renamedFailed', function() {
                ok(false, 'Rename folder failed');
                start();
            });

            folders.rename(firstFolder.folderID, 'Renamed Folder');
        } else {
            ok(true, 'No folders to rename');
            start();
        }
    });

    folders.loadFolders('mail');
});

test('folder model methods exist', function() {
    var folders = magma.model.folders();

    ok(typeof folders.loadFolders === 'function', 'loadFolders exists');
    ok(typeof folders.addFolder === 'function', 'addFolder exists');
    ok(typeof folders.rename === 'function', 'rename exists');
    ok(typeof folders.remove === 'function', 'remove exists');
    ok(typeof folders.addObserver === 'function', 'addObserver exists');
    ok(typeof folders.removeObserver === 'function', 'removeObserver exists');
});
