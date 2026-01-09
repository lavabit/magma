/**
 * Alerts Manager Tests
 *
 * Tests for alerts, toasts, badges, and panels
 */

module('alerts-manager');

test('alertsManager exists', function() {
    ok(magma.alertsManager, 'alertsManager exists');
    ok(typeof magma.alertsManager.create === 'function', 'create exists');
    ok(typeof magma.alertsManager.createBadge === 'function', 'createBadge exists');
    ok(typeof magma.alertsManager.createPanel === 'function', 'createPanel exists');
    ok(typeof magma.alertsManager.createToast === 'function', 'createToast exists');
    ok(typeof magma.alertsManager.createInlineAlert === 'function', 'createInlineAlert exists');
    ok(typeof magma.alertsManager.createAlertElement === 'function', 'createAlertElement exists');
    ok(magma.alertsManager.alertTypes, 'alertTypes exists');
});

// Alert types tests
test('alertTypes includes all expected types', function() {
    var types = magma.alertsManager.alertTypes;

    ok(types.system, 'Has system type');
    ok(types.warning, 'Has warning type');
    ok(types.error, 'Has error type');
    ok(types.info, 'Has info type');
    ok(types.success, 'Has success type');
});

test('alertTypes have required properties', function() {
    var types = magma.alertsManager.alertTypes;

    for (var name in types) {
        ok(types[name].icon, name + ' has icon');
        ok(types[name].title, name + ' has title');
        ok(types[name].className, name + ' has className');
        ok(typeof types[name].persistent === 'boolean', name + ' has persistent flag');
        ok(typeof types[name].requiresAcknowledge === 'boolean', name + ' has requiresAcknowledge flag');
    }
});

// Format time tests
test('formatTime formats timestamps correctly', function() {
    var formatTime = magma.alertsManager.formatTime;
    var now = new Date();

    // Just now
    equal(formatTime(now), 'Just now', 'Current time shows Just now');

    // Minutes ago
    var fiveMinAgo = new Date(now - 5 * 60000);
    equal(formatTime(fiveMinAgo), '5 minutes ago', '5 minutes ago');

    // Hours ago
    var twoHoursAgo = new Date(now - 2 * 3600000);
    equal(formatTime(twoHoursAgo), '2 hours ago', '2 hours ago');

    // Empty value
    equal(formatTime(null), '', 'Null returns empty string');
    equal(formatTime(''), '', 'Empty string returns empty string');
});

// Create alert element tests
test('createAlertElement returns alert element', function() {
    var $alert = magma.alertsManager.createAlertElement({
        id: 'test-1',
        type: 'info',
        message: 'Test message'
    });

    ok($alert.length, 'Creates element');
    ok($alert.hasClass('mgm-alert'), 'Has base class');
    ok($alert.hasClass('mgm-alert-info'), 'Has type class');
    ok($alert.attr('data-alert-id') === 'test-1', 'Has alert ID');
});

test('createAlertElement displays message', function() {
    var $alert = magma.alertsManager.createAlertElement({
        id: 'test-2',
        type: 'warning',
        message: 'Warning message here'
    });

    ok($alert.find('.mgm-alert-message').text().indexOf('Warning message here') >= 0, 'Shows message');
});

test('createAlertElement escapes HTML', function() {
    var $alert = magma.alertsManager.createAlertElement({
        id: 'test-3',
        type: 'info',
        message: '<script>alert("xss")</script>'
    });

    ok($alert.find('.mgm-alert-message').html().indexOf('<script>') === -1, 'Script tags escaped');
});

test('createAlertElement shows acknowledge button for system alerts', function() {
    var $alert = magma.alertsManager.createAlertElement({
        id: 'test-4',
        type: 'system',
        message: 'System alert'
    });

    ok($alert.find('.mgm-alert-btn-acknowledge').length, 'Has acknowledge button');
});

test('createAlertElement calls onDismiss callback', function() {
    expect(1);

    var dismissed = false;
    var $alert = magma.alertsManager.createAlertElement({
        id: 'test-5',
        type: 'info',
        message: 'Dismissible alert'
    }, {
        onDismiss: function(id) {
            dismissed = true;
        }
    });

    $alert.find('.mgm-alert-btn-dismiss').click();
    ok(dismissed, 'onDismiss called');
});

// Badge tests
test('createBadge returns badge API', function() {
    var badge = magma.alertsManager.createBadge();

    ok(badge.element.length, 'Has element');
    ok(typeof badge.setCount === 'function', 'Has setCount');
    ok(typeof badge.pulse === 'function', 'Has pulse');
    ok(typeof badge.show === 'function', 'Has show');
    ok(typeof badge.hide === 'function', 'Has hide');
});

test('createBadge updates count correctly', function() {
    var badge = magma.alertsManager.createBadge();

    badge.setCount(0);
    ok(!badge.element.hasClass('has-alerts'), 'No class with 0 count');

    badge.setCount(5);
    ok(badge.element.hasClass('has-alerts'), 'Has class with count');
    equal(badge.element.find('.mgm-alert-badge-count').text(), '5', 'Shows count');

    badge.setCount(100);
    equal(badge.element.find('.mgm-alert-badge-count').text(), '99+', 'Shows 99+ for large counts');
});

test('createBadge calls onClick callback', function() {
    expect(1);

    var clicked = false;
    var badge = magma.alertsManager.createBadge({
        onClick: function() {
            clicked = true;
        }
    });

    badge.element.click();
    ok(clicked, 'onClick called');
});

// Panel tests
test('createPanel returns panel API', function() {
    var panel = magma.alertsManager.createPanel();

    ok(panel.element.length, 'Has element');
    ok(typeof panel.addAlert === 'function', 'Has addAlert');
    ok(typeof panel.removeAlert === 'function', 'Has removeAlert');
    ok(typeof panel.clear === 'function', 'Has clear');
    ok(typeof panel.setAlerts === 'function', 'Has setAlerts');
    ok(typeof panel.getCount === 'function', 'Has getCount');
    ok(typeof panel.show === 'function', 'Has show');
    ok(typeof panel.hide === 'function', 'Has hide');
    ok(typeof panel.toggle === 'function', 'Has toggle');
    ok(typeof panel.isOpen === 'function', 'Has isOpen');
});

test('createPanel manages alerts', function() {
    var panel = magma.alertsManager.createPanel();

    equal(panel.getCount(), 0, 'Starts empty');

    panel.addAlert({ id: '1', type: 'info', message: 'First' });
    equal(panel.getCount(), 1, 'Count increased');

    panel.addAlert({ id: '2', type: 'warning', message: 'Second' });
    equal(panel.getCount(), 2, 'Count is 2');

    panel.removeAlert('1');
    equal(panel.getCount(), 1, 'Count decreased after remove');

    panel.clear();
    equal(panel.getCount(), 0, 'Count is 0 after clear');
});

test('createPanel setAlerts replaces all', function() {
    var panel = magma.alertsManager.createPanel();

    panel.addAlert({ id: '1', type: 'info', message: 'First' });

    panel.setAlerts([
        { id: '2', type: 'info', message: 'Second' },
        { id: '3', type: 'info', message: 'Third' }
    ]);

    equal(panel.getCount(), 2, 'Replaced with 2 alerts');
});

test('createPanel toggle visibility', function() {
    var panel = magma.alertsManager.createPanel();

    ok(!panel.isOpen(), 'Starts closed');

    panel.show();
    ok(panel.isOpen(), 'Opens after show');

    panel.hide();
    ok(!panel.isOpen(), 'Closes after hide');

    panel.toggle();
    ok(panel.isOpen(), 'Opens after toggle');

    panel.toggle();
    ok(!panel.isOpen(), 'Closes after second toggle');
});

// Toast tests
test('createToast returns toast API', function() {
    var toast = magma.alertsManager.createToast({
        id: 'toast-1',
        type: 'info',
        message: 'Toast message'
    });

    ok(toast.element.length, 'Has element');
    ok(typeof toast.dismiss === 'function', 'Has dismiss');
    ok(toast.element.hasClass('mgm-alert-toast'), 'Has toast class');
});

test('createToast has correct type class', function() {
    var toast = magma.alertsManager.createToast({
        id: 'toast-2',
        type: 'success',
        message: 'Success toast'
    });

    ok(toast.element.hasClass('mgm-alert-toast-success'), 'Has success class');
});

// Inline alert tests
test('createInlineAlert returns inline API', function() {
    var inline = magma.alertsManager.createInlineAlert({
        id: 'inline-1',
        type: 'warning',
        message: 'Inline warning'
    });

    ok(inline.element.length, 'Has element');
    ok(typeof inline.dismiss === 'function', 'Has dismiss');
    ok(inline.element.hasClass('mgm-alert-inline'), 'Has inline class');
    ok(inline.element.hasClass('mgm-alert-inline-warning'), 'Has type class');
});

test('createInlineAlert dismissible by default', function() {
    var inline = magma.alertsManager.createInlineAlert({
        id: 'inline-2',
        type: 'info',
        message: 'Dismissible'
    });

    ok(inline.element.find('.mgm-alert-inline-dismiss').length, 'Has dismiss button');
});

test('createInlineAlert can be non-dismissible', function() {
    var inline = magma.alertsManager.createInlineAlert({
        id: 'inline-3',
        type: 'error',
        message: 'Non-dismissible'
    }, {
        dismissible: false
    });

    ok(!inline.element.find('.mgm-alert-inline-dismiss').length, 'No dismiss button');
});

// Main manager tests
test('create returns manager API', function() {
    var $container = $('<div>');
    var manager = magma.alertsManager.create({
        container: $container
    });

    ok(typeof manager.show === 'function', 'Has show');
    ok(typeof manager.system === 'function', 'Has system');
    ok(typeof manager.warning === 'function', 'Has warning');
    ok(typeof manager.error === 'function', 'Has error');
    ok(typeof manager.info === 'function', 'Has info');
    ok(typeof manager.success === 'function', 'Has success');
    ok(typeof manager.toast === 'function', 'Has toast');
    ok(typeof manager.acknowledge === 'function', 'Has acknowledge');
    ok(typeof manager.dismiss === 'function', 'Has dismiss');
    ok(typeof manager.clearAll === 'function', 'Has clearAll');
    ok(typeof manager.getAlerts === 'function', 'Has getAlerts');
    ok(typeof manager.getUnreadCount === 'function', 'Has getUnreadCount');
    ok(typeof manager.destroy === 'function', 'Has destroy');

    manager.destroy();
});

test('create manager shows alerts', function() {
    var $container = $('<div>');
    var manager = magma.alertsManager.create({
        container: $container
    });

    var id = manager.show({
        type: 'info',
        message: 'Test alert'
    });

    ok(id, 'Returns alert ID');
    equal(manager.getAlerts().length, 1, 'Alert added');

    manager.destroy();
});

test('create manager convenience methods work', function() {
    var $container = $('<div>');
    var manager = magma.alertsManager.create({
        container: $container
    });

    manager.info('Info message');
    manager.warning('Warning message');
    manager.error('Error message');
    manager.success('Success message');

    equal(manager.getAlerts().length, 4, '4 alerts added');

    manager.destroy();
});

test('create manager dismiss removes alert', function() {
    var $container = $('<div>');
    var manager = magma.alertsManager.create({
        container: $container
    });

    var id = manager.info('Test alert');
    equal(manager.getAlerts().length, 1, 'Alert added');

    manager.dismiss(id);
    equal(manager.getAlerts().length, 0, 'Alert removed');

    manager.destroy();
});

test('create manager clearAll removes all alerts', function() {
    var $container = $('<div>');
    var manager = magma.alertsManager.create({
        container: $container
    });

    manager.info('First');
    manager.warning('Second');
    manager.error('Third');
    equal(manager.getAlerts().length, 3, '3 alerts added');

    manager.clearAll();
    equal(manager.getAlerts().length, 0, 'All alerts removed');

    manager.destroy();
});

test('create manager setAlerts loads alerts', function() {
    var $container = $('<div>');
    var manager = magma.alertsManager.create({
        container: $container
    });

    manager.setAlerts([
        { type: 'info', message: 'Alert 1' },
        { type: 'warning', message: 'Alert 2' }
    ]);

    equal(manager.getAlerts().length, 2, 'Loaded 2 alerts');

    manager.destroy();
});

test('create manager tracks unread count', function() {
    var $container = $('<div>');
    var manager = magma.alertsManager.create({
        container: $container
    });

    manager.info('Alert 1');
    manager.warning('Alert 2');
    equal(manager.getUnreadCount(), 2, 'Unread count is 2');

    manager.acknowledge(manager.getAlerts()[0].id);
    equal(manager.getUnreadCount(), 1, 'Unread count is 1 after acknowledge');

    manager.destroy();
});

test('create manager panel controls work', function() {
    var $container = $('<div>');
    var manager = magma.alertsManager.create({
        container: $container
    });

    ok(!manager.isPanelOpen(), 'Panel starts closed');

    manager.showPanel();
    ok(manager.isPanelOpen(), 'Panel opens');

    manager.hidePanel();
    ok(!manager.isPanelOpen(), 'Panel closes');

    manager.togglePanel();
    ok(manager.isPanelOpen(), 'Panel toggles open');

    manager.destroy();
});

test('create manager with badge updates count', function() {
    var $container = $('<div>');
    var $badgeContainer = $('<div>');
    var manager = magma.alertsManager.create({
        container: $container,
        badgeContainer: $badgeContainer
    });

    var badge = manager.getBadge();
    ok(badge, 'Badge created');

    manager.info('Test alert');
    equal($badgeContainer.find('.mgm-alert-badge-count').text(), '1', 'Badge shows count');

    manager.destroy();
});

test('create manager calls callbacks', function() {
    expect(2);

    var $container = $('<div>');
    var acknowledged = false;
    var dismissed = false;

    var manager = magma.alertsManager.create({
        container: $container,
        onAcknowledge: function(id) {
            acknowledged = true;
        },
        onDismiss: function(id) {
            dismissed = true;
        }
    });

    var id = manager.show({ type: 'system', message: 'Test' });

    manager.acknowledge(id);
    ok(acknowledged, 'onAcknowledge called');

    var id2 = manager.info('Another');
    manager.dismiss(id2);
    ok(dismissed, 'onDismiss called');

    manager.destroy();
});

test('destroy cleans up manager', function() {
    var $container = $('<div>');
    var $badgeContainer = $('<div>');
    var manager = magma.alertsManager.create({
        container: $container,
        badgeContainer: $badgeContainer
    });

    manager.info('Test');
    ok($container.find('.mgm-alert-panel').length, 'Panel in DOM');
    ok($badgeContainer.find('.mgm-alert-badge').length, 'Badge in DOM');

    manager.destroy();
    equal($container.find('.mgm-alert-panel').length, 0, 'Panel removed');
    equal($badgeContainer.find('.mgm-alert-badge').length, 0, 'Badge removed');
});
