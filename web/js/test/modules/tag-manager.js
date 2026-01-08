/**
 * Tag Manager Tests
 *
 * Tests for the tag management system
 */

module('tag-manager');

test('tagManager exists', function() {
    ok(magma.tagManager, 'tagManager exists');
    ok(typeof magma.tagManager.init === 'function', 'init method exists');
    ok(typeof magma.tagManager.createTagElement === 'function', 'createTagElement exists');
    ok(typeof magma.tagManager.createTagInput === 'function', 'createTagInput exists');
    ok(typeof magma.tagManager.createColorPicker === 'function', 'createColorPicker exists');
    ok(typeof magma.tagManager.createTagList === 'function', 'createTagList exists');
    ok(typeof magma.tagManager.getColor === 'function', 'getColor exists');
    ok(typeof magma.tagManager.setColor === 'function', 'setColor exists');
    ok(typeof magma.tagManager.getCommonTags === 'function', 'getCommonTags exists');
    ok(typeof magma.tagManager.getAllTags === 'function', 'getAllTags exists');
    ok(typeof magma.tagManager.slugify === 'function', 'slugify exists');
});

test('slugify converts names correctly', function() {
    equal(magma.tagManager.slugify('Work'), 'work', 'Lowercase conversion');
    equal(magma.tagManager.slugify('My Tag'), 'my-tag', 'Space to hyphen');
    equal(magma.tagManager.slugify('Tag With  Spaces'), 'tag-with-spaces', 'Multiple spaces');
    equal(magma.tagManager.slugify('Tag!@#$%'), 'tag', 'Special characters removed');
    equal(magma.tagManager.slugify(''), '', 'Empty string');
    equal(magma.tagManager.slugify(null), '', 'Null value');
});

test('createTagElement creates valid element', function() {
    var $tag = magma.tagManager.createTagElement({ name: 'Work', slug: 'work' });

    ok($tag.hasClass('tag'), 'Has tag class');
    ok($tag.hasClass('tag-work'), 'Has tag-specific class');
    equal($tag.attr('data-tag'), 'work', 'Has data-tag attribute');
    equal($tag.attr('data-tag-name'), 'Work', 'Has data-tag-name attribute');
    equal($tag.find('.tag-name').text(), 'Work', 'Contains tag name');
});

test('createTagElement with removable option', function() {
    var $tag = magma.tagManager.createTagElement('Test', { removable: true });

    ok($tag.find('.tag-remove').length, 'Has remove button');
});

test('createTagElement without removable option', function() {
    var $tag = magma.tagManager.createTagElement('Test', { removable: false });

    equal($tag.find('.tag-remove').length, 0, 'No remove button');
});

test('createTagElement handles string input', function() {
    var $tag = magma.tagManager.createTagElement('Simple Tag');

    equal($tag.find('.tag-name').text(), 'Simple Tag', 'Handles string input');
    equal($tag.attr('data-tag'), 'simple-tag', 'Generates slug from string');
});

test('createTagList creates list of tags', function() {
    var tags = [
        { name: 'Work', slug: 'work' },
        { name: 'Personal', slug: 'personal' },
        { name: 'Urgent', slug: 'urgent' }
    ];

    var $list = magma.tagManager.createTagList(tags);

    ok($list.hasClass('tag-list'), 'Has tag-list class');
    equal($list.find('.tag').length, 3, 'Contains 3 tags');
});

test('createTagList with removable and callbacks', function() {
    var removed = null;
    var clicked = null;

    var tags = [{ name: 'Test', slug: 'test' }];

    var $list = magma.tagManager.createTagList(tags, {
        removable: true,
        onRemove: function(tag) { removed = tag; },
        onClick: function(tag) { clicked = tag; }
    });

    // Test click callback
    $list.find('.tag').click();
    deepEqual(clicked, tags[0], 'Click callback fired');

    // Test remove callback
    $list.find('.tag-remove').click();
    deepEqual(removed, tags[0], 'Remove callback fired');
});

test('createTagInput creates input elements', function() {
    var $container = $('<div>');
    var created = null;

    var input = magma.tagManager.createTagInput($container, function(tag) {
        created = tag;
    });

    ok(input.element.hasClass('tag-input-wrapper'), 'Has wrapper');
    ok(input.element.find('.tag-input').length, 'Has input');
    ok(input.element.find('.tag-input-add').length, 'Has add button');
    ok(typeof input.focus === 'function', 'Has focus method');
    ok(typeof input.clear === 'function', 'Has clear method');
});

test('createTagInput focus and clear work', function() {
    var $container = $('<div>').appendTo('body');
    var input = magma.tagManager.createTagInput($container, function() {});

    input.element.find('.tag-input').val('test');
    input.clear();
    equal(input.element.find('.tag-input').val(), '', 'Clear empties input');

    $container.remove();
});

test('getCommonTags returns intersection', function() {
    var messages = [
        { tags: [{ name: 'Work' }, { name: 'Urgent' }] },
        { tags: [{ name: 'Work' }, { name: 'Personal' }] },
        { tags: [{ name: 'Work' }] }
    ];

    var common = magma.tagManager.getCommonTags(messages);

    deepEqual(common, ['Work'], 'Returns only common tags');
});

test('getCommonTags handles empty messages', function() {
    var common = magma.tagManager.getCommonTags([]);
    deepEqual(common, [], 'Returns empty for no messages');

    common = magma.tagManager.getCommonTags(null);
    deepEqual(common, [], 'Returns empty for null');
});

test('getCommonTags handles messages without tags', function() {
    var messages = [
        { tags: [{ name: 'Work' }] },
        { },
        { tags: [] }
    ];

    var common = magma.tagManager.getCommonTags(messages);
    deepEqual(common, [], 'Returns empty when some messages have no tags');
});

test('getAllTags returns union', function() {
    var messages = [
        { tags: [{ name: 'Work' }, { name: 'Urgent' }] },
        { tags: [{ name: 'Work' }, { name: 'Personal' }] },
        { tags: [{ name: 'Family' }] }
    ];

    var all = magma.tagManager.getAllTags(messages);

    ok(all.indexOf('Work') !== -1, 'Contains Work');
    ok(all.indexOf('Urgent') !== -1, 'Contains Urgent');
    ok(all.indexOf('Personal') !== -1, 'Contains Personal');
    ok(all.indexOf('Family') !== -1, 'Contains Family');
    equal(all.length, 4, 'Contains 4 unique tags');
});

test('getContrastColor returns appropriate contrast', function() {
    // Light colors should get black text
    equal(magma.tagManager.getContrastColor('#ffffff'), '#000', 'White bg gets black text');
    equal(magma.tagManager.getContrastColor('#ffff00'), '#000', 'Yellow bg gets black text');

    // Dark colors should get white text
    equal(magma.tagManager.getContrastColor('#000000'), '#fff', 'Black bg gets white text');
    equal(magma.tagManager.getContrastColor('#0000ff'), '#fff', 'Blue bg gets white text');
});

test('getContrastColor handles edge cases', function() {
    equal(magma.tagManager.getContrastColor(null), '#000', 'Null returns black');
    equal(magma.tagManager.getContrastColor(''), '#000', 'Empty returns black');
    equal(magma.tagManager.getContrastColor('fff'), '#000', 'Handles without #');
});

test('defaultColors array exists', function() {
    ok(Array.isArray(magma.tagManager.defaultColors), 'defaultColors is array');
    ok(magma.tagManager.defaultColors.length > 0, 'Has colors');

    // Check format
    var firstColor = magma.tagManager.defaultColors[0];
    ok(firstColor.match(/^#[0-9a-f]{6}$/i), 'Colors are hex format');
});

test('setColor and getColor work', function() {
    // Clear any existing
    magma.tagManager.removeColor('test-color-tag');

    equal(magma.tagManager.getColor('test-color-tag'), null, 'No color initially');

    magma.tagManager.setColor('test-color-tag', '#ff5722');
    equal(magma.tagManager.getColor('test-color-tag'), '#ff5722', 'Color is set');

    magma.tagManager.removeColor('test-color-tag');
    equal(magma.tagManager.getColor('test-color-tag'), null, 'Color is removed');
});

test('createColorPicker creates picker elements', function() {
    var $container = $('<div>');
    var changed = null;

    var picker = magma.tagManager.createColorPicker('test', $container, function(color) {
        changed = color;
    });

    ok(picker.element.hasClass('tag-color-picker'), 'Has picker class');
    ok(picker.element.find('.tag-color-preview').length, 'Has preview');
    ok(picker.element.find('.tag-color-options').length, 'Has options');
    ok(picker.element.find('.tag-color-swatch').length > 0, 'Has swatches');
    ok(typeof picker.getColor === 'function', 'Has getColor');
    ok(typeof picker.setColor === 'function', 'Has setColor');
});

test('init returns manager API', function() {
    var manager = magma.tagManager.init(null, {});

    ok(typeof manager.createTagElement === 'function', 'Has createTagElement');
    ok(typeof manager.createTagInput === 'function', 'Has createTagInput');
    ok(typeof manager.createColorPicker === 'function', 'Has createColorPicker');
    ok(typeof manager.createTagList === 'function', 'Has createTagList');
    ok(typeof manager.getColor === 'function', 'Has getColor');
    ok(typeof manager.setColor === 'function', 'Has setColor');
});

test('tag element escapes HTML', function() {
    var xssPayload = '<script>alert("xss")</script>';
    var $tag = magma.tagManager.createTagElement(xssPayload);

    var textContent = $tag.find('.tag-name').text();
    equal(textContent, xssPayload, 'XSS payload displayed as text');

    var htmlContent = $tag.find('.tag-name').html();
    ok(htmlContent.indexOf('<script>') === -1, 'Script tag is escaped');
});
