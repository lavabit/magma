/**
 * Logs Viewer Tests
 *
 * Tests for log tables, filters, pagination, and statistics
 */

module('logs-viewer');

test('logsViewer exists', function() {
    ok(magma.logsViewer, 'logsViewer exists');
    ok(typeof magma.logsViewer.create === 'function', 'create exists');
    ok(typeof magma.logsViewer.createTable === 'function', 'createTable exists');
    ok(typeof magma.logsViewer.createFilters === 'function', 'createFilters exists');
    ok(typeof magma.logsViewer.createToolbar === 'function', 'createToolbar exists');
    ok(typeof magma.logsViewer.createPagination === 'function', 'createPagination exists');
    ok(typeof magma.logsViewer.createTypeTabs === 'function', 'createTypeTabs exists');
    ok(typeof magma.logsViewer.createStatisticsView === 'function', 'createStatisticsView exists');
    ok(magma.logsViewer.logTypes, 'logTypes exists');
    ok(magma.logsViewer.severityLevels, 'severityLevels exists');
});

// Log types tests
test('logTypes includes all expected types', function() {
    var types = magma.logsViewer.logTypes;

    ok(types.statistics, 'Has statistics type');
    ok(types.security, 'Has security type');
    ok(types.contacts, 'Has contacts type');
    ok(types.mail, 'Has mail type');
});

test('logTypes have required properties', function() {
    var types = magma.logsViewer.logTypes;

    for (var name in types) {
        ok(types[name].label, name + ' has label');
        ok(types[name].icon, name + ' has icon');
        ok(typeof types[name].hasTable === 'boolean', name + ' has hasTable flag');
    }
});

// Severity levels tests
test('severityLevels includes all expected levels', function() {
    var levels = magma.logsViewer.severityLevels;

    ok(levels.critical, 'Has critical level');
    ok(levels.error, 'Has error level');
    ok(levels.warning, 'Has warning level');
    ok(levels.info, 'Has info level');
    ok(levels.debug, 'Has debug level');
});

// Utility function tests
test('formatBytes formats correctly', function() {
    var formatBytes = magma.logsViewer.formatBytes;

    equal(formatBytes(0), '0 B', 'Zero bytes');
    equal(formatBytes(1024), '1.00 KB', 'Kilobytes');
    equal(formatBytes(1048576), '1.00 MB', 'Megabytes');
    equal(formatBytes(1073741824), '1.00 GB', 'Gigabytes');
});

test('formatDate formats correctly', function() {
    var formatDate = magma.logsViewer.formatDate;

    equal(formatDate(null), '', 'Null date');
    equal(formatDate(''), '', 'Empty date');

    var date = new Date('2024-01-15T10:30:00');
    ok(formatDate(date).length > 0, 'Formats date string');
});

// Filters tests
test('createFilters returns filters API', function() {
    var filters = magma.logsViewer.createFilters();

    ok(filters.element.length, 'Has element');
    ok(typeof filters.getFilters === 'function', 'Has getFilters');
    ok(typeof filters.clear === 'function', 'Has clear');
});

test('createFilters has search input', function() {
    var filters = magma.logsViewer.createFilters({
        showSearch: true
    });

    ok(filters.element.find('.mgm-logs-search').length, 'Has search input');
});

test('createFilters has severity dropdown', function() {
    var filters = magma.logsViewer.createFilters({
        showSeverity: true
    });

    ok(filters.element.find('.mgm-logs-severity').length, 'Has severity dropdown');
});

test('createFilters getFilters returns filter values', function() {
    var filters = magma.logsViewer.createFilters();

    filters.element.find('.mgm-logs-search').val('test');
    var values = filters.getFilters();

    equal(values.search, 'test', 'Returns search value');
    ok('severity' in values, 'Has severity key');
    ok('dateRange' in values, 'Has dateRange key');
});

test('createFilters clear resets values', function() {
    var filters = magma.logsViewer.createFilters();

    filters.element.find('.mgm-logs-search').val('test');
    filters.clear();

    equal(filters.element.find('.mgm-logs-search').val(), '', 'Search cleared');
});

// Toolbar tests
test('createToolbar returns toolbar API', function() {
    var toolbar = magma.logsViewer.createToolbar();

    ok(toolbar.element.length, 'Has element');
    ok(typeof toolbar.startAutoRefresh === 'function', 'Has startAutoRefresh');
    ok(typeof toolbar.stopAutoRefresh === 'function', 'Has stopAutoRefresh');
    ok(typeof toolbar.setRefreshing === 'function', 'Has setRefreshing');
});

test('createToolbar has refresh button', function() {
    var toolbar = magma.logsViewer.createToolbar({
        showRefresh: true
    });

    ok(toolbar.element.find('.mgm-logs-btn-refresh').length, 'Has refresh button');
});

test('createToolbar has export button', function() {
    var toolbar = magma.logsViewer.createToolbar({
        showExport: true
    });

    ok(toolbar.element.find('.mgm-logs-btn-export').length, 'Has export button');
});

test('createToolbar calls onRefresh callback', function() {
    expect(1);

    var refreshed = false;
    var toolbar = magma.logsViewer.createToolbar({
        onRefresh: function() {
            refreshed = true;
        }
    });

    toolbar.element.find('.mgm-logs-btn-refresh').click();
    ok(refreshed, 'onRefresh called');
});

// Pagination tests
test('createPagination returns pagination API', function() {
    var pagination = magma.logsViewer.createPagination();

    ok(pagination.element.length, 'Has element');
    ok(typeof pagination.setPage === 'function', 'Has setPage');
    ok(typeof pagination.getPage === 'function', 'Has getPage');
});

test('createPagination displays page info', function() {
    var pagination = magma.logsViewer.createPagination({
        page: 1,
        totalPages: 5,
        pageSize: 50,
        totalItems: 240
    });

    ok(pagination.element.find('.mgm-logs-page-info').text().indexOf('240') >= 0, 'Shows total items');
});

test('createPagination updates page', function() {
    var pagination = magma.logsViewer.createPagination({
        page: 1,
        totalPages: 5
    });

    equal(pagination.getPage(), 1, 'Starts at page 1');

    pagination.setPage(3, 5, 100);
    equal(pagination.getPage(), 3, 'Updates to page 3');
});

test('createPagination calls onPageChange callback', function() {
    expect(1);

    var newPage = null;
    var pagination = magma.logsViewer.createPagination({
        page: 1,
        totalPages: 5,
        onPageChange: function(page) {
            newPage = page;
        }
    });

    pagination.element.find('.mgm-logs-page-btn').eq(3).click(); // Last button
    equal(newPage, 5, 'onPageChange called with page 5');
});

// Type tabs tests
test('createTypeTabs returns tabs API', function() {
    var tabs = magma.logsViewer.createTypeTabs();

    ok(tabs.element.length, 'Has element');
    ok(typeof tabs.setActiveType === 'function', 'Has setActiveType');
    ok(typeof tabs.getActiveType === 'function', 'Has getActiveType');
});

test('createTypeTabs shows all log types', function() {
    var tabs = magma.logsViewer.createTypeTabs();

    var tabCount = tabs.element.find('.mgm-logs-tab').length;
    equal(tabCount, 4, 'Has 4 tabs (statistics, security, contacts, mail)');
});

test('createTypeTabs sets active type', function() {
    var tabs = magma.logsViewer.createTypeTabs({
        activeType: 'mail'
    });

    equal(tabs.getActiveType(), 'mail', 'Gets active type');

    tabs.setActiveType('security');
    equal(tabs.getActiveType(), 'security', 'Sets active type');
});

test('createTypeTabs calls onTypeChange callback', function() {
    expect(1);

    var newType = null;
    var tabs = magma.logsViewer.createTypeTabs({
        onTypeChange: function(type) {
            newType = type;
        }
    });

    tabs.element.find('[data-type="mail"]').click();
    equal(newType, 'mail', 'onTypeChange called');
});

// Table tests
test('createTable returns table API', function() {
    var table = magma.logsViewer.createTable('security');

    ok(table.element.length, 'Has element');
    ok(typeof table.setEntries === 'function', 'Has setEntries');
    ok(typeof table.getTable === 'function', 'Has getTable');
});

test('createTable creates correct columns for security', function() {
    var table = magma.logsViewer.createTable('security');
    var headers = table.element.find('th');

    equal(headers.length, 6, 'Has 6 columns');
    equal(headers.eq(0).text(), 'UTC', 'First column is UTC');
});

test('createTable creates correct columns for mail', function() {
    var table = magma.logsViewer.createTable('mail');
    var headers = table.element.find('th');

    equal(headers.length, 7, 'Has 7 columns');
    equal(headers.eq(0).text(), 'Queue', 'First column is Queue');
});

test('createTable displays entries', function() {
    var table = magma.logsViewer.createTable('security', {
        entries: [
            { utc: '2024-01-15', type: 'login', severity: 'info', ip: '192.168.1.1', protocol: 'HTTPS' },
            { utc: '2024-01-14', type: 'login', severity: 'warning', ip: '192.168.1.2', protocol: 'HTTP' }
        ]
    });

    var rows = table.element.find('tbody tr');
    equal(rows.length, 2, 'Has 2 rows');
});

test('createTable shows empty state', function() {
    var table = magma.logsViewer.createTable('security', {
        entries: []
    });

    var emptyRow = table.element.find('.mgm-logs-empty');
    ok(emptyRow.length, 'Shows empty state');
});

// Statistics view tests
test('createStatisticsView returns view API', function() {
    var view = magma.logsViewer.createStatisticsView({
        account: { username: 'test' }
    });

    ok(view.element.length, 'Has element');
    ok(typeof view.update === 'function', 'Has update');
});

test('createStatisticsView displays sections', function() {
    var view = magma.logsViewer.createStatisticsView({
        account: { username: 'testuser', plan: 'premium' },
        storage: { space: '5 GB', folders: '10' },
        logins: { smtp: '5', pop: '2' }
    });

    var sections = view.element.find('.mgm-logs-stat-section');
    equal(sections.length, 3, 'Has 3 sections');
});

test('createStatisticsView displays stat items', function() {
    var view = magma.logsViewer.createStatisticsView({
        account: { username: 'testuser', email: 'test@example.com' }
    });

    var items = view.element.find('.mgm-logs-stat-item');
    equal(items.length, 2, 'Has 2 stat items');
});

// Main viewer tests
test('create returns viewer API', function() {
    var $container = $('<div>');
    var viewer = magma.logsViewer.create({
        container: $container
    });

    ok(viewer.element.length, 'Has element');
    ok(typeof viewer.setEntries === 'function', 'Has setEntries');
    ok(typeof viewer.refresh === 'function', 'Has refresh');
    ok(typeof viewer.setLogType === 'function', 'Has setLogType');
    ok(typeof viewer.getLogType === 'function', 'Has getLogType');
    ok(typeof viewer.getEntries === 'function', 'Has getEntries');
    ok(typeof viewer.clearFilters === 'function', 'Has clearFilters');
    ok(typeof viewer.destroy === 'function', 'Has destroy');

    viewer.destroy();
});

test('create viewer has all components', function() {
    var $container = $('<div>');
    var viewer = magma.logsViewer.create({
        container: $container
    });

    ok($container.find('.mgm-logs-tabs').length, 'Has tabs');
    ok($container.find('.mgm-logs-toolbar').length, 'Has toolbar');
    ok($container.find('.mgm-logs-filters').length, 'Has filters');
    ok($container.find('.mgm-logs-pagination').length, 'Has pagination');

    viewer.destroy();
});

test('create viewer sets and gets log type', function() {
    var $container = $('<div>');
    var viewer = magma.logsViewer.create({
        container: $container,
        logType: 'security'
    });

    equal(viewer.getLogType(), 'security', 'Gets initial log type');

    viewer.setLogType('mail');
    equal(viewer.getLogType(), 'mail', 'Sets new log type');

    viewer.destroy();
});

test('create viewer setEntries stores entries', function() {
    var $container = $('<div>');
    var viewer = magma.logsViewer.create({
        container: $container,
        logType: 'security'
    });

    viewer.setEntries([
        { utc: '2024-01-15', type: 'login', severity: 'info' },
        { utc: '2024-01-14', type: 'logout', severity: 'info' }
    ]);

    equal(viewer.getEntries().length, 2, 'Stores entries');

    viewer.destroy();
});

test('create viewer calls onLoad callback', function() {
    expect(1);

    var $container = $('<div>');
    var loadedType = null;
    var viewer = magma.logsViewer.create({
        container: $container,
        logType: 'security',
        onLoad: function(type) {
            loadedType = type;
        }
    });

    equal(loadedType, 'security', 'onLoad called with type');

    viewer.destroy();
});

test('destroy cleans up viewer', function() {
    var $container = $('<div>');
    var viewer = magma.logsViewer.create({
        container: $container
    });

    ok($container.find('.mgm-logs-viewer').length, 'Viewer in DOM');

    viewer.destroy();
    equal($container.find('.mgm-logs-viewer').length, 0, 'Viewer removed');
});
