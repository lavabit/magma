/**
 * Compose Utilities Tests
 *
 * Tests for email validation, form validation, and compose enhancements
 */

module('compose-utils');

test('composeUtils exists', function() {
    ok(magma.composeUtils, 'composeUtils exists');
    ok(typeof magma.composeUtils.init === 'function', 'init exists');
    ok(typeof magma.composeUtils.isValidEmail === 'function', 'isValidEmail exists');
    ok(typeof magma.composeUtils.validateEmails === 'function', 'validateEmails exists');
    ok(typeof magma.composeUtils.suggestEmailCorrection === 'function', 'suggestEmailCorrection exists');
    ok(typeof magma.composeUtils.createFormValidator === 'function', 'createFormValidator exists');
    ok(typeof magma.composeUtils.createDraftStatus === 'function', 'createDraftStatus exists');
    ok(typeof magma.composeUtils.createCharCounter === 'function', 'createCharCounter exists');
    ok(typeof magma.composeUtils.createRecipientChips === 'function', 'createRecipientChips exists');
});

// Email validation tests
test('isValidEmail validates correct emails', function() {
    ok(magma.composeUtils.isValidEmail('test@example.com'), 'Simple email');
    ok(magma.composeUtils.isValidEmail('user.name@example.com'), 'Email with dot');
    ok(magma.composeUtils.isValidEmail('user+tag@example.com'), 'Email with plus');
    ok(magma.composeUtils.isValidEmail('user@sub.example.com'), 'Email with subdomain');
    ok(magma.composeUtils.isValidEmail('test@example.co.uk'), 'Email with country TLD');
});

test('isValidEmail rejects invalid emails', function() {
    ok(!magma.composeUtils.isValidEmail(''), 'Empty string');
    ok(!magma.composeUtils.isValidEmail('test'), 'No @ symbol');
    ok(!magma.composeUtils.isValidEmail('test@'), 'No domain');
    ok(!magma.composeUtils.isValidEmail('@example.com'), 'No local part');
    ok(!magma.composeUtils.isValidEmail('test @example.com'), 'Space in email');
    ok(!magma.composeUtils.isValidEmail(null), 'Null value');
    ok(!magma.composeUtils.isValidEmail(undefined), 'Undefined value');
});

test('validateEmails handles multiple emails', function() {
    var result = magma.composeUtils.validateEmails('a@b.com, c@d.com');
    ok(result.isValid, 'Valid multiple emails');
    equal(result.validEmails.length, 2, 'Two valid emails');

    result = magma.composeUtils.validateEmails('a@b.com, invalid');
    ok(!result.isValid, 'Invalid when one email is bad');
    equal(result.invalid.length, 1, 'One invalid email');
    equal(result.invalid[0], 'invalid', 'Captures invalid email');
});

test('validateEmails handles semicolon separator', function() {
    var result = magma.composeUtils.validateEmails('a@b.com; c@d.com');
    ok(result.isValid, 'Valid with semicolons');
    equal(result.validEmails.length, 2, 'Two valid emails');
});

test('validateEmails handles empty input', function() {
    var result = magma.composeUtils.validateEmails('');
    ok(result.isValid, 'Empty is valid');
    equal(result.validEmails.length, 0, 'No emails');

    result = magma.composeUtils.validateEmails(null);
    ok(result.isValid, 'Null is valid');
});

// Email suggestion tests
test('suggestEmailCorrection fixes common typos', function() {
    var suggestion = magma.composeUtils.suggestEmailCorrection('test@gmial.com');
    equal(suggestion, 'test@gmail.com', 'Fixes gmial.com');

    suggestion = magma.composeUtils.suggestEmailCorrection('test@yaho.com');
    equal(suggestion, 'test@yahoo.com', 'Fixes yaho.com');

    suggestion = magma.composeUtils.suggestEmailCorrection('test@hotmal.com');
    equal(suggestion, 'test@hotmail.com', 'Fixes hotmal.com');
});

test('suggestEmailCorrection returns null for correct emails', function() {
    var suggestion = magma.composeUtils.suggestEmailCorrection('test@gmail.com');
    equal(suggestion, null, 'No suggestion for correct email');

    suggestion = magma.composeUtils.suggestEmailCorrection('test@customdomain.com');
    equal(suggestion, null, 'No suggestion for unknown domain');
});

test('suggestEmailCorrection handles edge cases', function() {
    var suggestion = magma.composeUtils.suggestEmailCorrection('');
    equal(suggestion, null, 'Empty string');

    suggestion = magma.composeUtils.suggestEmailCorrection('invalid');
    equal(suggestion, null, 'No @ symbol');
});

// Form validator tests
test('createFormValidator returns API', function() {
    var $form = $('<form>');
    var validator = magma.composeUtils.createFormValidator($form);

    ok(typeof validator.validateField === 'function', 'Has validateField');
    ok(typeof validator.validateForm === 'function', 'Has validateForm');
    ok(typeof validator.showError === 'function', 'Has showError');
    ok(typeof validator.clearError === 'function', 'Has clearError');
    ok(typeof validator.getErrors === 'function', 'Has getErrors');
    ok(typeof validator.hasErrors === 'function', 'Has hasErrors');
});

test('createFormValidator validates required fields', function() {
    var $form = $('<form>' +
        '<div class="field-wrapper">' +
        '<input type="text" id="to" value="">' +
        '</div>' +
        '</form>');

    var validator = magma.composeUtils.createFormValidator($form, {
        requiredFields: ['to']
    });

    var isValid = validator.validateField('#to');
    ok(!isValid, 'Empty required field is invalid');
    ok(validator.hasErrors(), 'Has errors');

    $form.find('#to').val('test@example.com');
    isValid = validator.validateField('#to');
    ok(isValid, 'Filled required field is valid');
});

test('createFormValidator validates email fields', function() {
    var $form = $('<form>' +
        '<div class="field-wrapper">' +
        '<input type="text" id="to" value="invalid-email">' +
        '</div>' +
        '</form>');

    var validator = magma.composeUtils.createFormValidator($form);

    var isValid = validator.validateField('#to');
    ok(!isValid, 'Invalid email fails validation');

    $form.find('#to').val('valid@email.com');
    isValid = validator.validateField('#to');
    ok(isValid, 'Valid email passes validation');
});

// Draft status tests
test('createDraftStatus returns API', function() {
    var $container = $('<div>');
    var status = magma.composeUtils.createDraftStatus($container, null);

    ok(status.element.length, 'Has element');
    ok(typeof status.setState === 'function', 'Has setState');
    ok(typeof status.getState === 'function', 'Has getState');
    ok(typeof status.saving === 'function', 'Has saving');
    ok(typeof status.saved === 'function', 'Has saved');
    ok(typeof status.error === 'function', 'Has error');
});

test('createDraftStatus changes states correctly', function() {
    var $container = $('<div>');
    var status = magma.composeUtils.createDraftStatus($container, null);

    equal(status.getState(), 'idle', 'Starts idle');

    status.saving();
    equal(status.getState(), 'saving', 'Changes to saving');
    ok(status.element.hasClass('saving'), 'Has saving class');

    status.saved();
    equal(status.getState(), 'saved', 'Changes to saved');
    ok(status.element.hasClass('saved'), 'Has saved class');

    status.error();
    equal(status.getState(), 'error', 'Changes to error');
    ok(status.element.hasClass('error'), 'Has error class');
});

// Character counter tests
test('createCharCounter returns API', function() {
    var $field = $('<input type="text" value="">');
    var $wrapper = $('<div class="field-wrapper">').append($field);

    var counter = magma.composeUtils.createCharCounter($field, 100);

    ok(counter.element.length, 'Has element');
    ok(typeof counter.update === 'function', 'Has update');
    ok(typeof counter.getCount === 'function', 'Has getCount');
    ok(typeof counter.getRemaining === 'function', 'Has getRemaining');
});

test('createCharCounter tracks character count', function() {
    var $field = $('<input type="text" value="">');
    var $wrapper = $('<div class="field-wrapper">').append($field);

    var counter = magma.composeUtils.createCharCounter($field, 100);

    equal(counter.getCount(), 0, 'Empty count is 0');
    equal(counter.getRemaining(), 100, 'Remaining is max');

    $field.val('Hello');
    counter.update();

    equal(counter.getCount(), 5, 'Count is 5');
    equal(counter.getRemaining(), 95, 'Remaining is 95');
});

test('createCharCounter adds warning/danger classes', function() {
    var $field = $('<input type="text" value="">');
    var $wrapper = $('<div class="field-wrapper">').append($field);

    var counter = magma.composeUtils.createCharCounter($field, 10);

    // Fill to 90% (9 chars)
    $field.val('123456789');
    counter.update();
    ok(counter.element.hasClass('warning'), 'Has warning class at 90%');

    // Fill to 100% (10 chars)
    $field.val('1234567890');
    counter.update();
    ok(counter.element.hasClass('danger'), 'Has danger class at 100%');

    // Exceed limit
    $field.val('12345678901');
    counter.update();
    ok(counter.element.hasClass('danger'), 'Has danger class over limit');
});

// Recipient chips tests
test('createRecipientChips returns API', function() {
    var $input = $('<input type="text">');
    var $container = $('<div>').append($input);

    var chips = magma.composeUtils.createRecipientChips($input);

    ok(chips.element.length, 'Has element');
    ok(typeof chips.addChip === 'function', 'Has addChip');
    ok(typeof chips.removeChip === 'function', 'Has removeChip');
    ok(typeof chips.getRecipients === 'function', 'Has getRecipients');
    ok(typeof chips.clear === 'function', 'Has clear');
});

test('createRecipientChips adds and removes chips', function() {
    var $input = $('<input type="text">');
    var $container = $('<div>').append($input);

    var chips = magma.composeUtils.createRecipientChips($input, { validate: false });

    chips.addChip('test@example.com');
    deepEqual(chips.getRecipients(), ['test@example.com'], 'Chip added');

    chips.addChip('another@example.com');
    equal(chips.getRecipients().length, 2, 'Second chip added');

    chips.removeChip('test@example.com');
    deepEqual(chips.getRecipients(), ['another@example.com'], 'Chip removed');

    chips.clear();
    equal(chips.getRecipients().length, 0, 'All chips cleared');
});

test('createRecipientChips validates emails', function() {
    var $input = $('<input type="text">');
    var $container = $('<div>').append($input);

    var chips = magma.composeUtils.createRecipientChips($input, { validate: true });

    var added = chips.addChip('valid@example.com');
    ok(added, 'Valid email added');

    added = chips.addChip('invalid-email');
    ok(!added, 'Invalid email rejected');

    equal(chips.getRecipients().length, 1, 'Only valid email in list');
});

test('createRecipientChips prevents duplicates', function() {
    var $input = $('<input type="text">');
    var $container = $('<div>').append($input);

    var chips = magma.composeUtils.createRecipientChips($input, { validate: false });

    chips.addChip('test@example.com');
    var added = chips.addChip('test@example.com');

    ok(!added, 'Duplicate rejected');
    equal(chips.getRecipients().length, 1, 'Only one chip');
});

// Init tests
test('init returns utilities API', function() {
    var $form = $('<form>');
    var utils = magma.composeUtils.init($form, null, {
        validation: false,
        draftStatus: false,
        charCounters: false,
        shortcuts: false
    });

    ok(utils, 'Returns object');
    ok(typeof utils.validate === 'function', 'Has validate');
    ok(typeof utils.showDraftSaving === 'function', 'Has showDraftSaving');
    ok(typeof utils.showDraftSaved === 'function', 'Has showDraftSaved');
    ok(typeof utils.showDraftError === 'function', 'Has showDraftError');
});
