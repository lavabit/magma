/**
 * Folder Manager Tests
 *
 * Tests for the enhanced folder management system
 */

module('folder-manager');

test('folderManager exists', function() {
    ok(magma.folderManager, 'folderManager exists');
    ok(typeof magma.folderManager.init === 'function', 'init method exists');
    ok(typeof magma.folderManager.extendModel === 'function', 'extendModel method exists');
    ok(typeof magma.folderManager.confirmDelete === 'function', 'confirmDelete method exists');
    ok(typeof magma.folderManager.setSortable === 'function', 'setSortable method exists');
    ok(typeof magma.folderManager.setEditMode === 'function', 'setEditMode method exists');
});

test('extendModel adds reorder methods', function() {
    // Create a mock folder model
    var mockModel = {
        observers: {},
        addObserver: function(event, callback) {
            this.observers[event] = this.observers[event] || [];
            this.observers[event].push(callback);
        }
    };

    // Extend the model
    var extended = magma.folderManager.extendModel(mockModel);

    ok(typeof extended.reorderFolder === 'function', 'reorderFolder method added');
    ok(typeof extended.getFolderOrder === 'function', 'getFolderOrder method added');
    ok(typeof extended.setFolderOrder === 'function', 'setFolderOrder method added');
    ok(extended._folderManagerExtended, 'Model marked as extended');
});

test('extendModel does not double-extend', function() {
    var mockModel = {
        observers: {},
        addObserver: function(event, callback) {
            this.observers[event] = this.observers[event] || [];
            this.observers[event].push(callback);
        }
    };

    var extended1 = magma.folderManager.extendModel(mockModel);
    var reorderFn = extended1.reorderFolder;

    var extended2 = magma.folderManager.extendModel(mockModel);
    strictEqual(extended2.reorderFolder, reorderFn, 'Same function after second extend');
});

test('reorderFolder updates order correctly', function() {
    var mockModel = {
        observers: {},
        addObserver: function(event, callback) {
            this.observers[event] = this.observers[event] || [];
            this.observers[event].push(callback);
        }
    };

    var extended = magma.folderManager.extendModel(mockModel);

    // Set initial order
    extended.setFolderOrder([1, 2, 3, 4, 5]);

    // Move folder 3 to the beginning
    var newOrder = extended.reorderFolder(3, null);
    deepEqual(newOrder, [3, 1, 2, 4, 5], 'Folder moved to beginning');

    // Move folder 5 after folder 1
    newOrder = extended.reorderFolder(5, 1);
    deepEqual(newOrder, [3, 1, 5, 2, 4], 'Folder moved after specified folder');
});

test('getFolderOrder returns copy', function() {
    var mockModel = {
        observers: {},
        addObserver: function(event, callback) {
            this.observers[event] = this.observers[event] || [];
            this.observers[event].push(callback);
        }
    };

    var extended = magma.folderManager.extendModel(mockModel);
    extended.setFolderOrder([1, 2, 3]);

    var order = extended.getFolderOrder();
    order.push(4);

    var originalOrder = extended.getFolderOrder();
    equal(originalOrder.length, 3, 'Original order unchanged');
});

test('init returns manager API', function() {
    // Create mock elements
    var $container = $('<div class="folder-menu">')
        .append('<div class="folder-options"><input type="checkbox" class="edit"></div>')
        .append('<div class="folder-scroll-wrapper"><ul class="folder-list"></ul></div>');

    // Create mock folder model
    var mockModel = {
        addObserver: function() {}
    };

    var manager = magma.folderManager.init(mockModel, $container, {sortable: false});

    ok(manager, 'Manager returned');
    ok(typeof manager.setEditMode === 'function', 'Has setEditMode');
    ok(typeof manager.isEditMode === 'function', 'Has isEditMode');
    ok(typeof manager.getOrder === 'function', 'Has getOrder');
    ok(typeof manager.refresh === 'function', 'Has refresh');

    $container.remove();
});

test('init tracks edit mode state', function() {
    var $container = $('<div class="folder-menu">')
        .append('<div class="folder-options"><input type="checkbox" class="edit"></div>')
        .append('<div class="folder-scroll-wrapper"><ul class="folder-list"></ul></div>');

    var mockModel = {
        addObserver: function() {}
    };

    var manager = magma.folderManager.init(mockModel, $container, {sortable: false});

    ok(!manager.isEditMode(), 'Starts not in edit mode');

    manager.setEditMode(true);
    ok(manager.isEditMode(), 'Edit mode enabled');
    ok($container.hasClass('folder-edit-mode'), 'Container has edit class');

    manager.setEditMode(false);
    ok(!manager.isEditMode(), 'Edit mode disabled');
    ok(!$container.hasClass('folder-edit-mode'), 'Container edit class removed');

    $container.remove();
});

test('getOrder returns folder IDs', function() {
    var $container = $('<div class="folder-menu">')
        .append('<div class="folder-options"><input type="checkbox" class="edit"></div>')
        .append('<div class="folder-scroll-wrapper"><ul class="folder-list">' +
            '<li id="folder-10"><a class="folder">Folder A</a></li>' +
            '<li id="folder-20"><a class="folder">Folder B</a></li>' +
            '<li id="folder-30"><a class="folder">Folder C</a></li>' +
            '</ul></div>');

    var mockModel = {
        addObserver: function() {}
    };

    var manager = magma.folderManager.init(mockModel, $container, {sortable: false});
    var order = manager.getOrder();

    deepEqual(order, [10, 20, 30], 'Returns folder IDs in order');

    $container.remove();
});

test('confirm delete uses dialog.confirm', function() {
    var confirmCalled = false;
    var capturedMessage = null;

    // Mock dialog.confirm
    var originalConfirm = magma.dialog.confirm;
    magma.dialog.confirm = function(message, onConfirm, onCancel) {
        confirmCalled = true;
        capturedMessage = message;
        // Simulate user clicking confirm
        if (onConfirm) onConfirm();
    };

    var deleteConfirmed = false;
    magma.folderManager.confirmDelete('Test Folder', function() {
        deleteConfirmed = true;
    });

    ok(confirmCalled, 'dialog.confirm was called');
    ok(capturedMessage.indexOf('Test Folder') !== -1, 'Message contains folder name');
    ok(deleteConfirmed, 'Confirm callback was called');

    // Restore
    magma.dialog.confirm = originalConfirm;
});

test('confirm delete escapes HTML in folder name', function() {
    var capturedMessage = null;

    var originalConfirm = magma.dialog.confirm;
    magma.dialog.confirm = function(message) {
        capturedMessage = message;
    };

    magma.folderManager.confirmDelete('<script>alert(1)</script>', function() {});

    ok(capturedMessage.indexOf('<script>') === -1, 'Script tag is escaped');
    ok(capturedMessage.indexOf('&lt;') !== -1 || capturedMessage.indexOf('\\u003c') !== -1 ||
       capturedMessage.indexOf('<script>') === -1, 'HTML was escaped');

    magma.dialog.confirm = originalConfirm;
});

// Test dialog.confirm function itself
module('dialog');

test('dialog.confirm exists', function() {
    ok(magma.dialog.confirm, 'confirm method exists');
    ok(typeof magma.dialog.confirm === 'function', 'confirm is a function');
});

test('dialog.confirm creates dialog', function() {
    // Note: This test checks that the dialog is created but doesn't test
    // the full UI interaction which requires jQuery UI

    var dialogCreated = false;
    var originalDialog = $.fn.dialog;

    $.fn.dialog = function() {
        dialogCreated = true;
        this.remove(); // Clean up
        return this;
    };

    magma.dialog.confirm('Test message', function() {}, function() {});

    ok(dialogCreated, 'Dialog was created');

    $.fn.dialog = originalDialog;
    $('#confirm-dialog').remove();
});

test('dialog.confirm escapes HTML', function() {
    var capturedContent = null;
    var originalDialog = $.fn.dialog;

    $.fn.dialog = function() {
        capturedContent = this.find('p').text();
        this.remove();
        return this;
    };

    var xssPayload = '<script>alert("xss")</script>';
    magma.dialog.confirm(xssPayload, function() {});

    equal(capturedContent, xssPayload, 'XSS payload displayed as text');

    $.fn.dialog = originalDialog;
    $('#confirm-dialog').remove();
});
