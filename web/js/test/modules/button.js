/**
 * Button Component Tests
 *
 * Tests for the unified button system
 */

module('button');

test('button.create returns API object', function() {
    var btn = magma.button.create({
        text: 'Test Button'
    });

    ok(btn, 'Button was created');
    ok(btn.element, 'Button has element property');
    ok(typeof btn.appendTo === 'function', 'Has appendTo method');
    ok(typeof btn.disable === 'function', 'Has disable method');
    ok(typeof btn.enable === 'function', 'Has enable method');
    ok(typeof btn.setPending === 'function', 'Has setPending method');
    ok(typeof btn.setActive === 'function', 'Has setActive method');
    ok(typeof btn.setText === 'function', 'Has setText method');
    ok(typeof btn.setIcon === 'function', 'Has setIcon method');
    ok(typeof btn.remove === 'function', 'Has remove method');
    ok(typeof btn.click === 'function', 'Has click method');

    btn.remove();
});

test('button creates standard button by default', function() {
    var btn = magma.button.create({
        text: 'Standard'
    });

    ok(btn.element.is('a'), 'Standard button is an anchor element');
    ok(btn.element.hasClass('mgm-btn'), 'Has base class');
    ok(btn.element.hasClass('mgm-btn-standard'), 'Has standard class');

    btn.remove();
});

test('button creates toggle button', function() {
    var btn = magma.button.create({
        text: 'Toggle',
        type: 'toggle'
    });

    ok(btn.element.is('span'), 'Toggle button wrapper is span');
    ok(btn.element.hasClass('mgm-btn-toggle'), 'Has toggle class');
    ok(btn.element.find('input[type="checkbox"]').length, 'Contains checkbox input');
    ok(btn.element.find('label').length, 'Contains label');

    btn.remove();
});

test('button creates dropdown button', function() {
    var btn = magma.button.create({
        text: 'Dropdown',
        type: 'dropdown'
    });

    ok(btn.element.hasClass('mgm-btn-dropdown'), 'Has dropdown class');
    ok(btn.element.find('.mgm-btn-dropdown-arrow').length, 'Contains dropdown arrow');

    btn.remove();
});

test('button layouts work correctly', function() {
    var iconTextBtn = magma.button.create({
        icon: 'send',
        text: 'Send',
        layout: 'icon-text'
    });
    ok(iconTextBtn.element.hasClass('mgm-btn-layout-icon-text'), 'icon-text layout class');
    ok(iconTextBtn.element.find('.mgm-btn-icon').length, 'Has icon element');
    ok(iconTextBtn.element.find('.mgm-btn-text').length, 'Has text element');
    iconTextBtn.remove();

    var iconOnlyBtn = magma.button.create({
        icon: 'refresh',
        text: 'Refresh',
        layout: 'icon'
    });
    ok(iconOnlyBtn.element.hasClass('mgm-btn-layout-icon'), 'icon layout class');
    ok(iconOnlyBtn.element.find('.sr-only').length, 'Has screen reader text');
    iconOnlyBtn.remove();

    var textOnlyBtn = magma.button.create({
        text: 'Text Only',
        layout: 'text'
    });
    ok(textOnlyBtn.element.hasClass('mgm-btn-layout-text'), 'text layout class');
    textOnlyBtn.remove();

    var shortcutBtn = magma.button.create({
        icon: 'send',
        text: 'Send',
        shortcut: 'Ctrl+Enter',
        layout: 'shortcut'
    });
    ok(shortcutBtn.element.hasClass('mgm-btn-layout-shortcut'), 'shortcut layout class');
    ok(shortcutBtn.element.find('.mgm-btn-shortcut').length, 'Has shortcut element');
    shortcutBtn.remove();
});

test('button disable/enable works', function() {
    var btn = magma.button.create({
        text: 'Test'
    });

    ok(!btn.isDisabled(), 'Button starts enabled');

    btn.disable();
    ok(btn.isDisabled(), 'Button is disabled after disable()');
    ok(btn.element.hasClass('mgm-btn-disabled'), 'Has disabled class');

    btn.enable();
    ok(!btn.isDisabled(), 'Button is enabled after enable()');
    ok(!btn.element.hasClass('mgm-btn-disabled'), 'Disabled class removed');

    btn.remove();
});

test('button pending state works', function() {
    var btn = magma.button.create({
        text: 'Test'
    });

    ok(!btn.isPending(), 'Button starts not pending');

    btn.setPending(true);
    ok(btn.isPending(), 'Button is pending after setPending(true)');
    ok(btn.element.hasClass('mgm-btn-pending'), 'Has pending class');
    ok(btn.element.hasClass('mgm-btn-disabled'), 'Pending adds disabled class');

    btn.setPending(false);
    ok(!btn.isPending(), 'Button not pending after setPending(false)');
    ok(!btn.element.hasClass('mgm-btn-pending'), 'Pending class removed');
    ok(!btn.element.hasClass('mgm-btn-disabled'), 'Disabled class removed when not originally disabled');

    btn.remove();
});

test('button active state works for toggle', function() {
    var btn = magma.button.create({
        text: 'Toggle',
        type: 'toggle'
    });

    ok(!btn.isActive(), 'Toggle starts inactive');

    btn.setActive(true);
    ok(btn.isActive(), 'Toggle is active after setActive(true)');
    ok(btn.element.hasClass('mgm-btn-active'), 'Has active class');
    ok(btn.element.find('input').prop('checked'), 'Checkbox is checked');

    btn.setActive(false);
    ok(!btn.isActive(), 'Toggle inactive after setActive(false)');
    ok(!btn.element.hasClass('mgm-btn-active'), 'Active class removed');
    ok(!btn.element.find('input').prop('checked'), 'Checkbox unchecked');

    btn.remove();
});

test('button setText updates text', function() {
    var btn = magma.button.create({
        text: 'Original'
    });

    equal(btn.element.find('.mgm-btn-text').text(), 'Original', 'Initial text set');

    btn.setText('Updated');
    equal(btn.element.find('.mgm-btn-text').text(), 'Updated', 'Text updated');

    btn.remove();
});

test('button setIcon updates icon', function() {
    var btn = magma.button.create({
        icon: 'send',
        text: 'Test'
    });

    ok(btn.element.hasClass('mgm-btn-icon-send'), 'Has initial icon class');

    btn.setIcon('reply');
    ok(!btn.element.hasClass('mgm-btn-icon-send'), 'Old icon class removed');
    ok(btn.element.hasClass('mgm-btn-icon-reply'), 'New icon class added');

    btn.remove();
});

test('button onClick callback fires', function() {
    var clicked = false;

    var btn = magma.button.create({
        text: 'Click Me',
        onClick: function() {
            clicked = true;
        }
    });

    var $container = $('<div>').appendTo('body');
    btn.appendTo($container);

    btn.click();
    ok(clicked, 'onClick callback was called');

    btn.remove();
    $container.remove();
});

test('button onToggle callback fires', function() {
    var toggleState = null;

    var btn = magma.button.create({
        text: 'Toggle',
        type: 'toggle',
        onToggle: function(active) {
            toggleState = active;
        }
    });

    var $container = $('<div>').appendTo('body');
    btn.appendTo($container);

    btn.click();
    equal(toggleState, true, 'onToggle called with true');

    btn.click();
    equal(toggleState, false, 'onToggle called with false');

    btn.remove();
    $container.remove();
});

test('button disabled prevents clicks', function() {
    var clicked = false;

    var btn = magma.button.create({
        text: 'Test',
        onClick: function() {
            clicked = true;
        }
    });

    var $container = $('<div>').appendTo('body');
    btn.appendTo($container);

    btn.disable();
    btn.click();
    ok(!clicked, 'Click not fired when disabled');

    btn.remove();
    $container.remove();
});

test('button pending prevents clicks', function() {
    var clicked = false;

    var btn = magma.button.create({
        text: 'Test',
        onClick: function() {
            clicked = true;
        }
    });

    var $container = $('<div>').appendTo('body');
    btn.appendTo($container);

    btn.setPending(true);
    btn.click();
    ok(!clicked, 'Click not fired when pending');

    btn.remove();
    $container.remove();
});

test('button escapes HTML in text', function() {
    var xssPayload = '<script>alert("xss")</script>';

    var btn = magma.button.create({
        text: xssPayload
    });

    var textContent = btn.element.find('.mgm-btn-text').text();
    equal(textContent, xssPayload, 'XSS payload displayed as text');

    var htmlContent = btn.element.find('.mgm-btn-text').html();
    ok(htmlContent.indexOf('<script>') === -1, 'Script tag is escaped in HTML');

    btn.remove();
});

test('button escapes HTML in shortcut', function() {
    var xssPayload = '<img src=x onerror=alert(1)>';

    var btn = magma.button.create({
        text: 'Test',
        shortcut: xssPayload,
        layout: 'shortcut'
    });

    var shortcutContent = btn.element.find('.mgm-btn-shortcut').text();
    equal(shortcutContent, xssPayload, 'XSS payload in shortcut displayed as text');

    btn.remove();
});

test('button.createGroup creates button group', function() {
    var group = magma.button.createGroup([
        { text: 'One' },
        { text: 'Two' },
        { text: 'Three' }
    ]);

    ok(group.element.hasClass('mgm-btn-group'), 'Has group class');
    equal(group.buttons.length, 3, 'Contains 3 buttons');
    ok(group.buttons[0].element.length, 'First button exists');
    ok(group.buttons[1].element.length, 'Second button exists');
    ok(group.buttons[2].element.length, 'Third button exists');

    group.element.remove();
});

test('button.createGroup disableAll/enableAll works', function() {
    var group = magma.button.createGroup([
        { text: 'One' },
        { text: 'Two' }
    ]);

    group.disableAll();
    ok(group.buttons[0].isDisabled(), 'First button disabled');
    ok(group.buttons[1].isDisabled(), 'Second button disabled');

    group.enableAll();
    ok(!group.buttons[0].isDisabled(), 'First button enabled');
    ok(!group.buttons[1].isDisabled(), 'Second button enabled');

    group.element.remove();
});

test('button initial disabled state', function() {
    var btn = magma.button.create({
        text: 'Test',
        disabled: true
    });

    ok(btn.isDisabled(), 'Button starts disabled');
    ok(btn.element.hasClass('mgm-btn-disabled'), 'Has disabled class initially');

    btn.remove();
});

test('button initial active state', function() {
    var btn = magma.button.create({
        text: 'Test',
        type: 'toggle',
        active: true
    });

    ok(btn.isActive(), 'Button starts active');
    ok(btn.element.hasClass('mgm-btn-active'), 'Has active class initially');
    ok(btn.element.find('input').prop('checked'), 'Checkbox starts checked');

    btn.remove();
});

test('button appendTo/prependTo/insertAfter/insertBefore work', function() {
    var $container = $('<div>').appendTo('body');
    var $marker = $('<span class="marker">').appendTo($container);

    var btn1 = magma.button.create({ text: 'Append' });
    btn1.appendTo($container);
    ok($container.children().last().is(btn1.element), 'appendTo adds to end');

    var btn2 = magma.button.create({ text: 'Prepend' });
    btn2.prependTo($container);
    ok($container.children().first().is(btn2.element), 'prependTo adds to start');

    var btn3 = magma.button.create({ text: 'After' });
    btn3.insertAfter($marker);
    ok($marker.next().is(btn3.element), 'insertAfter works');

    var btn4 = magma.button.create({ text: 'Before' });
    btn4.insertBefore($marker);
    ok($marker.prev().is(btn4.element), 'insertBefore works');

    btn1.remove();
    btn2.remove();
    btn3.remove();
    btn4.remove();
    $container.remove();
});

test('button getId returns unique IDs', function() {
    var btn1 = magma.button.create({ text: 'One' });
    var btn2 = magma.button.create({ text: 'Two' });

    ok(btn1.getId(), 'First button has ID');
    ok(btn2.getId(), 'Second button has ID');
    notEqual(btn1.getId(), btn2.getId(), 'IDs are unique');

    btn1.remove();
    btn2.remove();
});

test('button getElement returns jQuery element', function() {
    var btn = magma.button.create({ text: 'Test' });

    var el = btn.getElement();
    ok(el instanceof jQuery || el.jquery, 'getElement returns jQuery object');
    ok(el.is(btn.element), 'Returns same element');

    btn.remove();
});
