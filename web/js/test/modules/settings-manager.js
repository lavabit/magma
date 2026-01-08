/**
 * Settings Manager Tests
 *
 * Tests for settings panel, field types, validation, and API
 */

module('settings-manager');

test('settingsManager exists', function() {
    ok(magma.settingsManager, 'settingsManager exists');
    ok(typeof magma.settingsManager.create === 'function', 'create exists');
    ok(typeof magma.settingsManager.createField === 'function', 'createField exists');
    ok(typeof magma.settingsManager.createSection === 'function', 'createSection exists');
    ok(magma.settingsManager.fieldTypes, 'fieldTypes exists');
});

// Field type tests
test('fieldTypes includes all expected types', function() {
    var types = magma.settingsManager.fieldTypes;
    ok(types.toggle, 'Has toggle type');
    ok(types.checkbox, 'Has checkbox type');
    ok(types.dropdown, 'Has dropdown type');
    ok(types.text, 'Has text type');
    ok(types.number, 'Has number type');
    ok(types.textarea, 'Has textarea type');
    ok(types.email, 'Has email type');
    ok(types.password, 'Has password type');
    ok(types.radio, 'Has radio type');
    ok(types.color, 'Has color type');
});

test('each field type has required methods', function() {
    var types = magma.settingsManager.fieldTypes;

    for (var name in types) {
        ok(typeof types[name].render === 'function', name + ' has render');
        ok(typeof types[name].getValue === 'function', name + ' has getValue');
        ok(typeof types[name].setValue === 'function', name + ' has setValue');
    }
});

// Create field tests
test('createField returns field API', function() {
    var field = magma.settingsManager.createField({
        name: 'test',
        type: 'text',
        label: 'Test Field'
    }, 'initial value');

    ok(field.element.length, 'Has element');
    ok(typeof field.getValue === 'function', 'Has getValue');
    ok(typeof field.setValue === 'function', 'Has setValue');
    ok(typeof field.reset === 'function', 'Has reset');
    ok(typeof field.hasChanges === 'function', 'Has hasChanges');
    ok(typeof field.validate === 'function', 'Has validate');
    ok(typeof field.showError === 'function', 'Has showError');
    ok(typeof field.clearError === 'function', 'Has clearError');
});

test('createField text type works correctly', function() {
    var field = magma.settingsManager.createField({
        name: 'username',
        type: 'text',
        label: 'Username'
    }, 'testuser');

    equal(field.getValue(), 'testuser', 'Gets initial value');

    field.setValue('newuser');
    equal(field.getValue(), 'newuser', 'Sets new value');
});

test('createField toggle type works correctly', function() {
    var field = magma.settingsManager.createField({
        name: 'enabled',
        type: 'toggle',
        label: 'Enable Feature'
    }, true);

    equal(field.getValue(), true, 'Gets initial value');

    field.setValue(false);
    equal(field.getValue(), false, 'Sets false value');

    field.setValue(true);
    equal(field.getValue(), true, 'Sets true value');
});

test('createField checkbox type works correctly', function() {
    var field = magma.settingsManager.createField({
        name: 'agree',
        type: 'checkbox',
        checkboxLabel: 'I agree'
    }, false);

    equal(field.getValue(), false, 'Gets initial false');

    field.setValue(true);
    equal(field.getValue(), true, 'Sets true value');
});

test('createField dropdown type works correctly', function() {
    var field = magma.settingsManager.createField({
        name: 'theme',
        type: 'dropdown',
        options: [
            { value: 'light', label: 'Light' },
            { value: 'dark', label: 'Dark' },
            { value: 'auto', label: 'Auto' }
        ]
    }, 'dark');

    equal(field.getValue(), 'dark', 'Gets initial value');

    field.setValue('light');
    equal(field.getValue(), 'light', 'Sets new value');
});

test('createField number type works correctly', function() {
    var field = magma.settingsManager.createField({
        name: 'count',
        type: 'number',
        min: 0,
        max: 100
    }, 50);

    equal(field.getValue(), 50, 'Gets initial value');

    field.setValue(75);
    equal(field.getValue(), 75, 'Sets new value');
});

test('createField textarea type works correctly', function() {
    var field = magma.settingsManager.createField({
        name: 'signature',
        type: 'textarea',
        rows: 4
    }, 'My signature');

    equal(field.getValue(), 'My signature', 'Gets initial value');

    field.setValue('New signature\nWith multiple lines');
    equal(field.getValue(), 'New signature\nWith multiple lines', 'Sets multiline value');
});

test('createField radio type works correctly', function() {
    var field = magma.settingsManager.createField({
        name: 'format',
        type: 'radio',
        options: [
            { value: 'html', label: 'HTML' },
            { value: 'plain', label: 'Plain Text' }
        ]
    }, 'html');

    equal(field.getValue(), 'html', 'Gets initial value');

    field.setValue('plain');
    equal(field.getValue(), 'plain', 'Sets new value');
});

test('createField color type works correctly', function() {
    var field = magma.settingsManager.createField({
        name: 'accentColor',
        type: 'color'
    }, '#ff0000');

    equal(field.getValue(), '#ff0000', 'Gets initial value');

    field.setValue('#00ff00');
    equal(field.getValue(), '#00ff00', 'Sets new value');
});

// Validation tests
test('createField validates required fields', function() {
    var field = magma.settingsManager.createField({
        name: 'email',
        type: 'text',
        required: true
    }, '');

    ok(!field.validate(), 'Empty required field is invalid');
    ok(field.element.hasClass('has-error'), 'Has error class');

    field.setValue('test@example.com');
    ok(field.validate(), 'Filled required field is valid');
    ok(!field.element.hasClass('has-error'), 'Error class removed');
});

test('createField runs custom validation', function() {
    var field = magma.settingsManager.createField({
        name: 'age',
        type: 'number',
        validate: function(value) {
            if (value < 18) {
                return 'Must be at least 18';
            }
            return true;
        }
    }, 15);

    ok(!field.validate(), 'Invalid value fails validation');

    field.setValue(21);
    ok(field.validate(), 'Valid value passes validation');
});

// Reset tests
test('createField reset restores default value', function() {
    var field = magma.settingsManager.createField({
        name: 'name',
        type: 'text',
        defaultValue: 'Default Name'
    }, 'Default Name');

    field.setValue('Changed Name');
    equal(field.getValue(), 'Changed Name', 'Value changed');

    field.reset();
    equal(field.getValue(), 'Default Name', 'Value reset to default');
});

// Create section tests
test('createSection returns section API', function() {
    var section = magma.settingsManager.createSection({
        title: 'General Settings',
        description: 'Configure basic settings'
    });

    ok(section.element.length, 'Has element');
    ok(section.fieldsContainer.length, 'Has fields container');
    ok(section.element.find('.mgm-setting-section-title').length, 'Has title');
    ok(section.element.find('.mgm-setting-section-description').length, 'Has description');
});

// Create panel tests
test('create returns settings panel API', function() {
    var $container = $('<div>');
    var settings = magma.settingsManager.create({
        container: $container,
        fields: [
            { name: 'test', type: 'text', defaultValue: '' }
        ]
    });

    ok(settings.element.length, 'Has element');
    ok(typeof settings.getValue === 'function', 'Has getValue');
    ok(typeof settings.setValue === 'function', 'Has setValue');
    ok(typeof settings.getValues === 'function', 'Has getValues');
    ok(typeof settings.setValues === 'function', 'Has setValues');
    ok(typeof settings.validate === 'function', 'Has validate');
    ok(typeof settings.hasChanges === 'function', 'Has hasChanges');
    ok(typeof settings.resetToDefaults === 'function', 'Has resetToDefaults');
    ok(typeof settings.getField === 'function', 'Has getField');
    ok(typeof settings.destroy === 'function', 'Has destroy');

    settings.destroy();
});

test('create panel gets and sets values', function() {
    var $container = $('<div>');
    var settings = magma.settingsManager.create({
        container: $container,
        fields: [
            { name: 'name', type: 'text' },
            { name: 'enabled', type: 'toggle' }
        ],
        values: {
            name: 'Test User',
            enabled: true
        }
    });

    equal(settings.getValue('name'), 'Test User', 'Gets name value');
    equal(settings.getValue('enabled'), true, 'Gets enabled value');

    settings.setValue('name', 'New Name');
    equal(settings.getValue('name'), 'New Name', 'Sets name value');

    var values = settings.getValues();
    equal(values.name, 'New Name', 'getValues returns name');
    equal(values.enabled, true, 'getValues returns enabled');

    settings.destroy();
});

test('create panel handles sections', function() {
    var $container = $('<div>');
    var settings = magma.settingsManager.create({
        container: $container,
        sections: [
            {
                title: 'Section 1',
                fields: [
                    { name: 'field1', type: 'text' }
                ]
            },
            {
                title: 'Section 2',
                fields: [
                    { name: 'field2', type: 'toggle' }
                ]
            }
        ],
        values: {
            field1: 'value1',
            field2: true
        }
    });

    equal($container.find('.mgm-setting-section').length, 2, 'Has two sections');
    equal(settings.getValue('field1'), 'value1', 'Gets section 1 field');
    equal(settings.getValue('field2'), true, 'Gets section 2 field');

    settings.destroy();
});

test('create panel validates all fields', function() {
    var $container = $('<div>');
    var settings = magma.settingsManager.create({
        container: $container,
        fields: [
            { name: 'required1', type: 'text', required: true },
            { name: 'required2', type: 'text', required: true }
        ],
        values: {
            required1: '',
            required2: ''
        }
    });

    ok(!settings.validate(), 'Validation fails with empty required fields');

    settings.setValue('required1', 'value1');
    settings.setValue('required2', 'value2');
    ok(settings.validate(), 'Validation passes with values');

    settings.destroy();
});

test('create panel tracks changes', function() {
    var $container = $('<div>');
    var settings = magma.settingsManager.create({
        container: $container,
        fields: [
            { name: 'test', type: 'text' }
        ],
        values: {
            test: 'initial'
        }
    });

    // Note: hasChanges tracks input events, which we simulate manually
    ok(!settings.hasChanges(), 'No changes initially');

    // Get field and trigger change
    var field = settings.getField('test');
    field.element.find('input').val('changed').trigger('input');

    ok(settings.hasChanges(), 'Has changes after input');

    settings.destroy();
});

test('create panel resets to defaults', function() {
    var $container = $('<div>');
    var settings = magma.settingsManager.create({
        container: $container,
        fields: [
            { name: 'name', type: 'text', defaultValue: 'Default' },
            { name: 'count', type: 'number', defaultValue: 10 }
        ],
        values: {
            name: 'Custom',
            count: 50
        }
    });

    equal(settings.getValue('name'), 'Custom', 'Has custom value');

    settings.resetToDefaults();
    equal(settings.getValue('name'), 'Default', 'Reset to default');
    equal(settings.getValue('count'), 10, 'Reset count to default');

    settings.destroy();
});

test('create panel calls onSave callback', function() {
    expect(3);

    var $container = $('<div>');
    var savedValues = null;
    var settings = magma.settingsManager.create({
        container: $container,
        fields: [
            { name: 'test', type: 'text' }
        ],
        values: { test: 'initial' },
        onSave: function(values) {
            savedValues = values;
        }
    });

    // Simulate change and save
    var field = settings.getField('test');
    field.element.find('input').val('saved value').trigger('input');

    // Click save
    $container.find('.mgm-setting-btn-save').click();

    ok(savedValues !== null, 'onSave called');
    equal(savedValues.test, 'saved value', 'Saved correct value');
    ok(!settings.hasChanges(), 'Changes cleared after save');

    settings.destroy();
});

test('create panel calls onCancel callback', function() {
    expect(3);

    var $container = $('<div>');
    var cancelCalled = false;
    var settings = magma.settingsManager.create({
        container: $container,
        fields: [
            { name: 'test', type: 'text' }
        ],
        values: { test: 'initial' },
        onCancel: function() {
            cancelCalled = true;
        }
    });

    // Simulate change
    var field = settings.getField('test');
    field.element.find('input').val('changed').trigger('input');

    // Click cancel
    $container.find('.mgm-setting-btn-cancel').click();

    ok(cancelCalled, 'onCancel called');
    equal(settings.getValue('test'), 'initial', 'Value restored');
    ok(!settings.hasChanges(), 'Changes cleared after cancel');

    settings.destroy();
});

test('create panel shows and clears field errors', function() {
    var $container = $('<div>');
    var settings = magma.settingsManager.create({
        container: $container,
        fields: [
            { name: 'email', type: 'email' }
        ],
        values: { email: '' }
    });

    settings.showFieldError('email', 'Invalid email address');

    var field = settings.getField('email');
    ok(field.element.hasClass('has-error'), 'Has error class');
    ok(field.element.find('.mgm-setting-error').text().indexOf('Invalid email') >= 0, 'Shows error message');

    settings.clearFieldError('email');
    ok(!field.element.hasClass('has-error'), 'Error class cleared');
    equal(field.element.find('.mgm-setting-error').text(), '', 'Error message cleared');

    settings.destroy();
});

// Action button visibility tests
test('create panel respects showActions options', function() {
    var $container = $('<div>');

    var settings1 = magma.settingsManager.create({
        container: $container,
        fields: [{ name: 'test', type: 'text' }],
        showActions: false
    });
    equal($container.find('.mgm-setting-actions').length, 0, 'No actions when showActions false');
    settings1.destroy();

    var settings2 = magma.settingsManager.create({
        container: $container,
        fields: [{ name: 'test', type: 'text' }],
        showSave: false,
        showCancel: false,
        showReset: true
    });
    equal($container.find('.mgm-setting-btn-save').length, 0, 'No save button when showSave false');
    equal($container.find('.mgm-setting-btn-cancel').length, 0, 'No cancel button when showCancel false');
    equal($container.find('.mgm-setting-btn-reset').length, 1, 'Has reset button when showReset true');
    settings2.destroy();
});

// Destroy tests
test('destroy removes panel from DOM', function() {
    var $container = $('<div>');
    var settings = magma.settingsManager.create({
        container: $container,
        fields: [{ name: 'test', type: 'text' }]
    });

    equal($container.find('.mgm-setting-panel').length, 1, 'Panel in DOM');

    settings.destroy();
    equal($container.find('.mgm-setting-panel').length, 0, 'Panel removed from DOM');
});
