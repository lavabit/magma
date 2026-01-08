/**
 * Contacts Manager Tests
 *
 * Tests for contact forms, views, selection, and operations
 */

module('contacts-manager');

test('contactsManager exists', function() {
    ok(magma.contactsManager, 'contactsManager exists');
    ok(typeof magma.contactsManager.create === 'function', 'create exists');
    ok(typeof magma.contactsManager.createEditForm === 'function', 'createEditForm exists');
    ok(typeof magma.contactsManager.createContactView === 'function', 'createContactView exists');
    ok(typeof magma.contactsManager.createMoveDialog === 'function', 'createMoveDialog exists');
    ok(typeof magma.contactsManager.createSelectionManager === 'function', 'createSelectionManager exists');
    ok(typeof magma.contactsManager.createSortControls === 'function', 'createSortControls exists');
    ok(magma.contactsManager.contactFields, 'contactFields exists');
});

// Contact fields tests
test('contactFields includes required fields', function() {
    var fields = magma.contactsManager.contactFields;

    ok(fields.name, 'Has name field');
    ok(fields.email, 'Has email field');
    ok(fields.company, 'Has company field');
    ok(fields.phoneHome, 'Has phoneHome field');
    ok(fields.phoneMobile, 'Has phoneMobile field');
    ok(fields.address, 'Has address field');
});

test('contactFields has proper definitions', function() {
    var fields = magma.contactsManager.contactFields;

    equal(fields.name.type, 'text', 'Name is text type');
    ok(fields.name.required, 'Name is required');

    equal(fields.email.type, 'email', 'Email is email type');
    ok(fields.email.required, 'Email is required');
    ok(typeof fields.email.validate === 'function', 'Email has validation');

    equal(fields.address.type, 'textarea', 'Address is textarea type');
});

// Edit form tests
test('createEditForm returns form API', function() {
    var form = magma.contactsManager.createEditForm({
        name: 'Test User',
        email: 'test@example.com'
    });

    ok(form.element.length, 'Has element');
    ok(typeof form.validate === 'function', 'Has validate');
    ok(typeof form.getValues === 'function', 'Has getValues');
    ok(typeof form.hasChanges === 'function', 'Has hasChanges');
    ok(typeof form.getField === 'function', 'Has getField');
    ok(typeof form.focus === 'function', 'Has focus');
});

test('createEditForm populates initial values', function() {
    var contact = {
        name: 'John Doe',
        email: 'john@example.com',
        company: 'Acme Corp'
    };

    var form = magma.contactsManager.createEditForm(contact);
    var values = form.getValues();

    equal(values.name, 'John Doe', 'Name populated');
    equal(values.email, 'john@example.com', 'Email populated');
    equal(values.company, 'Acme Corp', 'Company populated');
});

test('createEditForm validates required fields', function() {
    var form = magma.contactsManager.createEditForm({
        name: '',
        email: ''
    });

    ok(!form.validate(), 'Empty required fields fail validation');

    // Fill required fields
    form.getField('name').element.find('input').val('Test Name');
    form.getField('email').element.find('input').val('test@example.com');

    ok(form.validate(), 'Filled required fields pass validation');
});

test('createEditForm validates email format', function() {
    var form = magma.contactsManager.createEditForm({
        name: 'Test',
        email: 'invalid-email'
    });

    ok(!form.validate(), 'Invalid email fails validation');

    form.getField('email').element.find('input').val('valid@email.com');
    ok(form.validate(), 'Valid email passes validation');
});

test('createEditForm detects changes', function() {
    var form = magma.contactsManager.createEditForm({
        name: 'Original Name',
        email: 'original@example.com'
    });

    ok(!form.hasChanges(), 'No changes initially');

    form.getField('name').element.find('input').val('New Name');
    ok(form.hasChanges(), 'Detects changes');
});

test('createEditForm calls onSave callback', function() {
    expect(2);

    var savedValues = null;
    var form = magma.contactsManager.createEditForm({
        name: 'Test',
        email: 'test@example.com'
    }, {
        onSave: function(values) {
            savedValues = values;
        }
    });

    form.element.trigger('submit');

    ok(savedValues !== null, 'onSave called');
    equal(savedValues.name, 'Test', 'Values passed to onSave');
});

// Contact view tests
test('createContactView returns view API', function() {
    var view = magma.contactsManager.createContactView({
        name: 'Test User',
        email: 'test@example.com'
    });

    ok(view.element.length, 'Has element');
    ok(view.element.hasClass('mgm-contact-view'), 'Has correct class');
});

test('createContactView displays contact info', function() {
    var view = magma.contactsManager.createContactView({
        name: 'John Doe',
        email: 'john@example.com',
        company: 'Acme Corp',
        phoneWork: '555-1234'
    });

    ok(view.element.find('.mgm-contact-name').text().indexOf('John Doe') >= 0, 'Shows name');
    ok(view.element.find('.mgm-contact-company').text().indexOf('Acme Corp') >= 0, 'Shows company');
    ok(view.element.html().indexOf('john@example.com') >= 0, 'Shows email');
    ok(view.element.html().indexOf('555-1234') >= 0, 'Shows phone');
});

test('createContactView escapes HTML', function() {
    var view = magma.contactsManager.createContactView({
        name: '<script>alert("xss")</script>',
        email: 'test@example.com'
    });

    ok(view.element.find('.mgm-contact-name').html().indexOf('<script>') === -1, 'Script tags escaped');
    ok(view.element.find('.mgm-contact-name').html().indexOf('&lt;script&gt;') >= 0, 'Script tags encoded');
});

// Move dialog tests
test('createMoveDialog returns dialog API', function() {
    var dialog = magma.contactsManager.createMoveDialog({
        folders: [
            { id: 1, name: 'Folder 1' },
            { id: 2, name: 'Folder 2' }
        ]
    });

    ok(dialog.element.length, 'Has element');
    ok(typeof dialog.getSelectedFolder === 'function', 'Has getSelectedFolder');
});

test('createMoveDialog shows folder list', function() {
    var dialog = magma.contactsManager.createMoveDialog({
        folders: [
            { id: 1, name: 'People' },
            { id: 2, name: 'Business' },
            { id: 3, name: 'Collected' }
        ],
        currentFolder: 1
    });

    var folderItems = dialog.element.find('.mgm-contact-folder-list li');

    // Should show 2 folders (excluding current)
    equal(folderItems.length, 2, 'Shows folders excluding current');
});

test('createMoveDialog enables move button on selection', function() {
    var dialog = magma.contactsManager.createMoveDialog({
        folders: [
            { id: 1, name: 'Folder 1' },
            { id: 2, name: 'Folder 2' }
        ]
    });

    var $moveBtn = dialog.element.find('.mgm-contact-btn-primary');
    ok($moveBtn.prop('disabled'), 'Move button disabled initially');

    dialog.element.find('.mgm-contact-folder-list li').first().click();
    ok(!$moveBtn.prop('disabled'), 'Move button enabled after selection');
});

test('createMoveDialog calls onMove callback', function() {
    expect(2);

    var movedTo = null;
    var dialog = magma.contactsManager.createMoveDialog({
        folders: [
            { id: 1, name: 'Folder 1' },
            { id: 2, name: 'Folder 2' }
        ],
        onMove: function(folderId) {
            movedTo = folderId;
        }
    });

    dialog.element.find('.mgm-contact-folder-list li').first().click();
    dialog.element.find('.mgm-contact-btn-primary').click();

    ok(movedTo !== null, 'onMove called');
    equal(movedTo, '1', 'Correct folder ID passed');
});

// Selection manager tests
test('createSelectionManager returns selection API', function() {
    var $container = $('<div>');
    var selection = magma.contactsManager.createSelectionManager({
        container: $container
    });

    ok(typeof selection.getSelected === 'function', 'Has getSelected');
    ok(typeof selection.select === 'function', 'Has select');
    ok(typeof selection.deselect === 'function', 'Has deselect');
    ok(typeof selection.toggle === 'function', 'Has toggle');
    ok(typeof selection.selectAll === 'function', 'Has selectAll');
    ok(typeof selection.deselectAll === 'function', 'Has deselectAll');
    ok(typeof selection.hasSelection === 'function', 'Has hasSelection');
    ok(typeof selection.count === 'function', 'Has count');
});

test('createSelectionManager manages selection state', function() {
    var $container = $('<div>' +
        '<div data-contact-id="1">Contact 1</div>' +
        '<div data-contact-id="2">Contact 2</div>' +
        '<div data-contact-id="3">Contact 3</div>' +
        '</div>');

    var selection = magma.contactsManager.createSelectionManager({
        container: $container
    });

    ok(!selection.hasSelection(), 'No selection initially');
    equal(selection.count(), 0, 'Count is 0');

    selection.select('1');
    ok(selection.hasSelection(), 'Has selection after select');
    equal(selection.count(), 1, 'Count is 1');
    deepEqual(selection.getSelected(), ['1'], 'Returns selected IDs');

    selection.select('2');
    equal(selection.count(), 2, 'Count is 2');

    selection.deselect('1');
    equal(selection.count(), 1, 'Count reduced after deselect');
    deepEqual(selection.getSelected(), ['2'], 'Correct ID remains');
});

test('createSelectionManager selectAll and deselectAll', function() {
    var $container = $('<div>' +
        '<div data-contact-id="1">Contact 1</div>' +
        '<div data-contact-id="2">Contact 2</div>' +
        '<div data-contact-id="3">Contact 3</div>' +
        '</div>');

    var selection = magma.contactsManager.createSelectionManager({
        container: $container
    });

    selection.selectAll();
    equal(selection.count(), 3, 'All selected');

    selection.deselectAll();
    equal(selection.count(), 0, 'All deselected');
});

test('createSelectionManager toggle', function() {
    var $container = $('<div>' +
        '<div data-contact-id="1">Contact 1</div>' +
        '</div>');

    var selection = magma.contactsManager.createSelectionManager({
        container: $container
    });

    selection.toggle('1');
    ok(selection.hasSelection(), 'Selected after first toggle');

    selection.toggle('1');
    ok(!selection.hasSelection(), 'Deselected after second toggle');
});

test('createSelectionManager calls onSelectionChange', function() {
    expect(2);

    var $container = $('<div>' +
        '<div data-contact-id="1">Contact 1</div>' +
        '</div>');

    var changes = [];
    var selection = magma.contactsManager.createSelectionManager({
        container: $container,
        onSelectionChange: function(ids) {
            changes.push(ids.slice());
        }
    });

    selection.select('1');
    ok(changes.length > 0, 'Callback called');
    deepEqual(changes[changes.length - 1], ['1'], 'Correct IDs passed');
});

// Sort controls tests
test('createSortControls returns controls API', function() {
    var controls = magma.contactsManager.createSortControls();

    ok(controls.element.length, 'Has element');
    ok(typeof controls.getCurrentSort === 'function', 'Has getCurrentSort');
    ok(typeof controls.setSort === 'function', 'Has setSort');
});

test('createSortControls returns current sort state', function() {
    var controls = magma.contactsManager.createSortControls({
        currentField: 'email',
        currentDirection: 'desc'
    });

    var sort = controls.getCurrentSort();
    equal(sort.field, 'email', 'Returns current field');
    equal(sort.direction, 'desc', 'Returns current direction');
});

test('createSortControls calls onSort callback', function() {
    expect(2);

    var sortCalls = [];
    var controls = magma.contactsManager.createSortControls({
        onSort: function(field, direction) {
            sortCalls.push({ field: field, direction: direction });
        }
    });

    controls.element.find('.mgm-contact-sort-direction').click();

    ok(sortCalls.length > 0, 'onSort called');
    equal(sortCalls[sortCalls.length - 1].direction, 'desc', 'Direction changed');
});

// Create manager tests
test('create returns manager API', function() {
    var $container = $('<div>');
    var manager = magma.contactsManager.create({
        container: $container
    });

    ok(manager.element.length, 'Has element');
    ok(typeof manager.newContact === 'function', 'Has newContact');
    ok(typeof manager.editContact === 'function', 'Has editContact');
    ok(typeof manager.viewContact === 'function', 'Has viewContact');
    ok(typeof manager.deleteContacts === 'function', 'Has deleteContacts');
    ok(typeof manager.moveContacts === 'function', 'Has moveContacts');
    ok(typeof manager.copyContacts === 'function', 'Has copyContacts');
    ok(typeof manager.getSelection === 'function', 'Has getSelection');
    ok(typeof manager.isEditMode === 'function', 'Has isEditMode');
    ok(typeof manager.cancelEdit === 'function', 'Has cancelEdit');
    ok(typeof manager.destroy === 'function', 'Has destroy');

    manager.destroy();
});

test('create manager newContact enters edit mode', function() {
    var $container = $('<div>');
    var manager = magma.contactsManager.create({
        container: $container
    });

    ok(!manager.isEditMode(), 'Not in edit mode initially');

    manager.newContact();
    ok(manager.isEditMode(), 'In edit mode after newContact');

    manager.destroy();
});

test('create manager calls onNew callback', function() {
    expect(1);

    var $container = $('<div>');
    var newCalled = false;
    var manager = magma.contactsManager.create({
        container: $container,
        onNew: function() {
            newCalled = true;
        }
    });

    manager.newContact();
    ok(newCalled, 'onNew callback called');

    manager.destroy();
});

test('create manager editContact enters edit mode with data', function() {
    var $container = $('<div>');
    var manager = magma.contactsManager.create({
        container: $container
    });

    manager.editContact({
        name: 'Edit Test',
        email: 'edit@example.com'
    });

    ok(manager.isEditMode(), 'In edit mode');
    ok($container.find('input[name="name"]').val() === 'Edit Test', 'Form populated');

    manager.destroy();
});

test('create manager viewContact shows contact', function() {
    var $container = $('<div>');
    var manager = magma.contactsManager.create({
        container: $container
    });

    manager.viewContact({
        name: 'View Test',
        email: 'view@example.com'
    });

    ok(!manager.isEditMode(), 'Not in edit mode');
    ok($container.find('.mgm-contact-view').length, 'View element created');
    ok($container.html().indexOf('View Test') >= 0, 'Contact name shown');

    manager.destroy();
});

test('create manager createSortControls creates controls', function() {
    var $container = $('<div>');
    var manager = magma.contactsManager.create({
        container: $container
    });

    var controls = manager.createSortControls();
    ok(controls.element.length, 'Sort controls created');

    manager.destroy();
});

test('destroy removes manager from DOM', function() {
    var $container = $('<div>');
    var manager = magma.contactsManager.create({
        container: $container
    });

    manager.newContact();
    ok($container.find('.mgm-contact-panel').length, 'Panel in DOM');

    manager.destroy();
    equal($container.find('.mgm-contact-panel').length, 0, 'Panel removed');
});
