/**
 * Magma Logs Viewer
 *
 * Enhanced log viewing with:
 * - Multiple log types (security, mail, contacts, statistics)
 * - Expandable log entries with details
 * - Filtering by type, severity, date range
 * - Search within logs
 * - Export functionality
 * - Auto-refresh option
 * - Pagination
 * - Statistics dashboard
 *
 * Usage:
 *   var viewer = magma.logsViewer.create({
 *       container: $('#logs-panel'),
 *       logType: 'security',
 *       onLoad: function(type) { ... }
 *   });
 */

var magma = magma || {};

magma.logsViewer = (function() {
    'use strict';

    var PREFIX = 'mgm-logs';

    /**
     * Log type configurations
     */
    var logTypes = {
        statistics: {
            label: 'Statistics',
            icon: '📊',
            hasTable: false,
            description: 'Account usage and storage statistics'
        },
        security: {
            label: 'Security',
            icon: '🔒',
            hasTable: true,
            columns: ['UTC', 'Time', 'Type', 'Severity', 'IP', 'Protocol'],
            description: 'Login attempts and security events'
        },
        contacts: {
            label: 'Contacts',
            icon: '👥',
            hasTable: true,
            columns: ['Date', 'Action', 'Contact', 'Details'],
            description: 'Contact changes and access history'
        },
        mail: {
            label: 'Mail',
            icon: '✉',
            hasTable: true,
            columns: ['Queue', 'Type', 'From', 'To', 'Outcome', 'Time', 'Size'],
            expandable: true,
            description: 'Email sending and receiving history'
        }
    };

    /**
     * Severity levels with styling
     */
    var severityLevels = {
        critical: { label: 'Critical', className: 'critical', priority: 1 },
        error: { label: 'Error', className: 'error', priority: 2 },
        warning: { label: 'Warning', className: 'warning', priority: 3 },
        info: { label: 'Info', className: 'info', priority: 4 },
        debug: { label: 'Debug', className: 'debug', priority: 5 }
    };

    /**
     * Escape HTML for safe display
     */
    function escapeHtml(text) {
        if (text === null || text === undefined) return '';
        return $('<div>').text(String(text)).html();
    }

    /**
     * Format date for display
     */
    function formatDate(date) {
        if (!date) return '';
        var d = date instanceof Date ? date : new Date(date);
        return d.toLocaleString();
    }

    /**
     * Format bytes to human readable
     */
    function formatBytes(bytes) {
        if (!bytes || bytes === 0) return '0 B';
        var sizes = ['B', 'KB', 'MB', 'GB', 'TB'];
        var i = Math.floor(Math.log(bytes) / Math.log(1024));
        return (bytes / Math.pow(1024, i)).toFixed(2) + ' ' + sizes[i];
    }

    /**
     * Create filter controls
     */
    function createFilters(options) {
        options = $.extend({
            showSeverity: true,
            showDateRange: true,
            showSearch: true,
            onFilter: null
        }, options);

        var $filters = $('<div class="' + PREFIX + '-filters">');

        // Search
        if (options.showSearch) {
            var $search = $('<div class="' + PREFIX + '-filter-group">');
            $search.append('<label>Search:</label>');
            var $searchInput = $('<input type="text" class="' + PREFIX + '-search" placeholder="Search logs...">')
                .on('input', debounce(function() {
                    triggerFilter();
                }, 300));
            $search.append($searchInput);
            $filters.append($search);
        }

        // Severity filter
        if (options.showSeverity) {
            var $severity = $('<div class="' + PREFIX + '-filter-group">');
            $severity.append('<label>Severity:</label>');
            var $severitySelect = $('<select class="' + PREFIX + '-severity">');
            $severitySelect.append('<option value="">All</option>');

            for (var key in severityLevels) {
                $severitySelect.append('<option value="' + key + '">' + severityLevels[key].label + '</option>');
            }

            $severitySelect.on('change', triggerFilter);
            $severity.append($severitySelect);
            $filters.append($severity);
        }

        // Date range
        if (options.showDateRange) {
            var $dateRange = $('<div class="' + PREFIX + '-filter-group">');
            $dateRange.append('<label>Date:</label>');
            var $dateSelect = $('<select class="' + PREFIX + '-date-range">');
            $dateSelect.append('<option value="">All Time</option>');
            $dateSelect.append('<option value="today">Today</option>');
            $dateSelect.append('<option value="week">Last 7 Days</option>');
            $dateSelect.append('<option value="month">Last 30 Days</option>');
            $dateSelect.on('change', triggerFilter);
            $dateRange.append($dateSelect);
            $filters.append($dateRange);
        }

        // Clear filters button
        var $clear = $('<button type="button" class="' + PREFIX + '-btn ' + PREFIX + '-btn-clear">Clear</button>');
        $clear.on('click', function() {
            $filters.find('input').val('');
            $filters.find('select').val('');
            triggerFilter();
        });
        $filters.append($clear);

        function triggerFilter() {
            if (options.onFilter) {
                options.onFilter(getFilters());
            }
        }

        function getFilters() {
            return {
                search: $filters.find('.' + PREFIX + '-search').val() || '',
                severity: $filters.find('.' + PREFIX + '-severity').val() || '',
                dateRange: $filters.find('.' + PREFIX + '-date-range').val() || ''
            };
        }

        // Debounce helper
        function debounce(fn, delay) {
            var timeout;
            return function() {
                var context = this;
                var args = arguments;
                clearTimeout(timeout);
                timeout = setTimeout(function() {
                    fn.apply(context, args);
                }, delay);
            };
        }

        return {
            element: $filters,
            getFilters: getFilters,
            clear: function() {
                $filters.find('input').val('');
                $filters.find('select').val('');
            }
        };
    }

    /**
     * Create log entry row
     */
    function createLogEntry(entry, logType, options) {
        options = $.extend({
            expandable: false,
            onExpand: null
        }, options);

        var config = logTypes[logType] || logTypes.security;
        var $row = $('<tr class="' + PREFIX + '-entry">')
            .attr('data-entry-id', entry.id || entry.messageID || entry.utc);

        // Add severity class if applicable
        if (entry.severity && severityLevels[entry.severity]) {
            $row.addClass(PREFIX + '-' + severityLevels[entry.severity].className);
        }

        // Build cells based on log type
        switch (logType) {
            case 'security':
                $row.append('<td>' + escapeHtml(entry.utc) + '</td>');
                $row.append('<td>' + escapeHtml(entry.time || formatDate(entry.utc)) + '</td>');
                $row.append('<td>' + escapeHtml(entry.type) + '</td>');
                $row.append('<td><span class="' + PREFIX + '-severity-badge ' + PREFIX + '-severity-' + (entry.severity || 'info') + '">' +
                    escapeHtml(entry.severity || 'info') + '</span></td>');
                $row.append('<td>' + escapeHtml(entry.ip) + '</td>');
                $row.append('<td>' + escapeHtml(entry.protocol) + '</td>');
                break;

            case 'mail':
                $row.append('<td>' + escapeHtml(entry.queue) + '</td>');
                $row.append('<td>' + escapeHtml(entry.type) + '</td>');
                $row.append('<td>' + escapeHtml(entry.from) + '</td>');
                $row.append('<td>' + escapeHtml(entry.to) + '</td>');
                $row.append('<td><span class="' + PREFIX + '-outcome ' + PREFIX + '-outcome-' + (entry.outcome || '').toLowerCase() + '">' +
                    escapeHtml(entry.outcome) + '</span></td>');
                $row.append('<td>' + escapeHtml(entry.time || formatDate(entry.utc)) + '</td>');
                $row.append('<td>' + escapeHtml(entry.size || formatBytes(entry.bytes)) + '</td>');
                break;

            case 'contacts':
                $row.append('<td>' + escapeHtml(entry.date || formatDate(entry.utc)) + '</td>');
                $row.append('<td>' + escapeHtml(entry.action) + '</td>');
                $row.append('<td>' + escapeHtml(entry.contact) + '</td>');
                $row.append('<td>' + escapeHtml(entry.details) + '</td>');
                break;
        }

        // Expandable row
        if (options.expandable) {
            $row.addClass(PREFIX + '-expandable');
            $row.on('click', function() {
                if ($(this).hasClass('expanded')) {
                    $(this).removeClass('expanded');
                    $(this).next('.' + PREFIX + '-details').remove();
                } else {
                    $(this).addClass('expanded');
                    var $details = createEntryDetails(entry, logType);
                    $details.insertAfter($(this));

                    if (options.onExpand) {
                        options.onExpand(entry);
                    }
                }
            });
        }

        return $row;
    }

    /**
     * Create expanded entry details row
     */
    function createEntryDetails(entry, logType) {
        var colspan = (logTypes[logType] && logTypes[logType].columns) ?
            logTypes[logType].columns.length : 6;

        var $detailsRow = $('<tr class="' + PREFIX + '-details">');
        var $detailsCell = $('<td colspan="' + colspan + '">');
        var $content = $('<div class="' + PREFIX + '-details-content">');

        // Build details based on log type
        var $dl = $('<dl>');

        if (logType === 'mail') {
            if (entry.messageID) {
                $dl.append('<dt>Message ID</dt><dd>' + escapeHtml(entry.messageID) + '</dd>');
            }
            if (entry.subject) {
                $dl.append('<dt>Subject</dt><dd>' + escapeHtml(entry.subject) + '</dd>');
            }
            if (entry.headers) {
                $dl.append('<dt>Headers</dt><dd><pre>' + escapeHtml(entry.headers) + '</pre></dd>');
            }
            if (entry.deliveryStatus) {
                $dl.append('<dt>Delivery Status</dt><dd>' + escapeHtml(entry.deliveryStatus) + '</dd>');
            }
        } else if (logType === 'security') {
            if (entry.userAgent) {
                $dl.append('<dt>User Agent</dt><dd>' + escapeHtml(entry.userAgent) + '</dd>');
            }
            if (entry.location) {
                $dl.append('<dt>Location</dt><dd>' + escapeHtml(entry.location) + '</dd>');
            }
            if (entry.details) {
                $dl.append('<dt>Details</dt><dd>' + escapeHtml(entry.details) + '</dd>');
            }
        }

        // Raw data
        $dl.append('<dt>Raw Data</dt><dd><pre class="' + PREFIX + '-raw">' +
            escapeHtml(JSON.stringify(entry, null, 2)) + '</pre></dd>');

        $content.append($dl);
        $detailsCell.append($content);
        $detailsRow.append($detailsCell);

        return $detailsRow;
    }

    /**
     * Create log table
     */
    function createTable(logType, options) {
        options = $.extend({
            entries: [],
            onExpand: null
        }, options);

        var config = logTypes[logType] || logTypes.security;
        var $table = $('<table class="' + PREFIX + '-table">');

        // Header
        var $thead = $('<thead>');
        var $headerRow = $('<tr>');

        (config.columns || []).forEach(function(col) {
            $headerRow.append('<th>' + escapeHtml(col) + '</th>');
        });

        $thead.append($headerRow);
        $table.append($thead);

        // Body
        var $tbody = $('<tbody>');
        $table.append($tbody);

        // Populate entries
        function setEntries(entries) {
            $tbody.empty();

            if (!entries || entries.length === 0) {
                var colspan = config.columns ? config.columns.length : 1;
                $tbody.append('<tr class="' + PREFIX + '-empty"><td colspan="' + colspan + '">No log entries found</td></tr>');
                return;
            }

            entries.forEach(function(entry) {
                var $row = createLogEntry(entry, logType, {
                    expandable: config.expandable,
                    onExpand: options.onExpand
                });
                $tbody.append($row);
            });
        }

        setEntries(options.entries);

        return {
            element: $table,
            setEntries: setEntries,
            getTable: function() {
                return $table;
            }
        };
    }

    /**
     * Create statistics view
     */
    function createStatisticsView(data) {
        var $view = $('<div class="' + PREFIX + '-statistics">');

        // Create sections
        var sections = [
            { key: 'account', title: 'Account', icon: '👤' },
            { key: 'storage', title: 'Storage', icon: '💾' },
            { key: 'logins', title: 'Logins', icon: '🔑' },
            { key: 'messages', title: 'Messages', icon: '✉' },
            { key: 'transfer', title: 'Transfer', icon: '📊' },
            { key: 'blocked', title: 'Blocked', icon: '🚫' }
        ];

        sections.forEach(function(section) {
            if (!data[section.key]) return;

            var $section = $('<div class="' + PREFIX + '-stat-section">');
            $section.append('<h4><span class="stat-icon">' + section.icon + '</span> ' + section.title + '</h4>');

            var $grid = $('<div class="' + PREFIX + '-stat-grid">');

            for (var key in data[section.key]) {
                var value = data[section.key][key];
                var $stat = $('<div class="' + PREFIX + '-stat-item">');
                $stat.append('<span class="stat-label">' + escapeHtml(formatLabel(key)) + '</span>');
                $stat.append('<span class="stat-value">' + escapeHtml(value) + '</span>');
                $grid.append($stat);
            }

            $section.append($grid);
            $view.append($section);
        });

        function formatLabel(key) {
            return key.replace(/([A-Z])/g, ' $1')
                .replace(/^./, function(str) { return str.toUpperCase(); });
        }

        return {
            element: $view,
            update: function(newData) {
                $view.empty();
                createStatisticsView(newData).element.children().appendTo($view);
            }
        };
    }

    /**
     * Create toolbar
     */
    function createToolbar(options) {
        options = $.extend({
            showRefresh: true,
            showExport: true,
            showAutoRefresh: true,
            onRefresh: null,
            onExport: null,
            onAutoRefreshChange: null
        }, options);

        var $toolbar = $('<div class="' + PREFIX + '-toolbar">');
        var autoRefreshInterval = null;

        // Refresh button
        if (options.showRefresh) {
            var $refresh = $('<button type="button" class="' + PREFIX + '-btn ' + PREFIX + '-btn-refresh">')
                .html('⟳ Refresh');

            $refresh.on('click', function() {
                if (options.onRefresh) {
                    options.onRefresh();
                }
            });

            $toolbar.append($refresh);
        }

        // Auto-refresh toggle
        if (options.showAutoRefresh) {
            var $autoRefresh = $('<label class="' + PREFIX + '-auto-refresh">');
            var $checkbox = $('<input type="checkbox">');
            $autoRefresh.append($checkbox, ' Auto-refresh');

            var $intervalSelect = $('<select class="' + PREFIX + '-auto-refresh-interval">');
            $intervalSelect.append('<option value="30">30s</option>');
            $intervalSelect.append('<option value="60">1m</option>');
            $intervalSelect.append('<option value="300">5m</option>');
            $intervalSelect.hide();

            $checkbox.on('change', function() {
                var enabled = $(this).is(':checked');
                $intervalSelect.toggle(enabled);

                if (enabled) {
                    startAutoRefresh(parseInt($intervalSelect.val(), 10));
                } else {
                    stopAutoRefresh();
                }

                if (options.onAutoRefreshChange) {
                    options.onAutoRefreshChange(enabled);
                }
            });

            $intervalSelect.on('change', function() {
                if ($checkbox.is(':checked')) {
                    stopAutoRefresh();
                    startAutoRefresh(parseInt($(this).val(), 10));
                }
            });

            $toolbar.append($autoRefresh, $intervalSelect);
        }

        // Export button
        if (options.showExport) {
            var $export = $('<button type="button" class="' + PREFIX + '-btn ' + PREFIX + '-btn-export">')
                .html('↓ Export');

            $export.on('click', function() {
                if (options.onExport) {
                    options.onExport();
                }
            });

            $toolbar.append($export);
        }

        function startAutoRefresh(seconds) {
            stopAutoRefresh();
            autoRefreshInterval = setInterval(function() {
                if (options.onRefresh) {
                    options.onRefresh();
                }
            }, seconds * 1000);
        }

        function stopAutoRefresh() {
            if (autoRefreshInterval) {
                clearInterval(autoRefreshInterval);
                autoRefreshInterval = null;
            }
        }

        return {
            element: $toolbar,
            startAutoRefresh: startAutoRefresh,
            stopAutoRefresh: stopAutoRefresh,
            setRefreshing: function(refreshing) {
                $toolbar.find('.' + PREFIX + '-btn-refresh').toggleClass('refreshing', refreshing);
            }
        };
    }

    /**
     * Create pagination
     */
    function createPagination(options) {
        options = $.extend({
            page: 1,
            totalPages: 1,
            pageSize: 50,
            totalItems: 0,
            onPageChange: null
        }, options);

        var $pagination = $('<div class="' + PREFIX + '-pagination">');

        // Info
        var $info = $('<span class="' + PREFIX + '-page-info">');
        $pagination.append($info);

        // Controls
        var $controls = $('<div class="' + PREFIX + '-page-controls">');

        var $first = $('<button type="button" class="' + PREFIX + '-page-btn">').html('«').attr('title', 'First');
        var $prev = $('<button type="button" class="' + PREFIX + '-page-btn">').html('‹').attr('title', 'Previous');
        var $pageInput = $('<input type="number" class="' + PREFIX + '-page-input" min="1">');
        var $pageTotal = $('<span class="' + PREFIX + '-page-total">');
        var $next = $('<button type="button" class="' + PREFIX + '-page-btn">').html('›').attr('title', 'Next');
        var $last = $('<button type="button" class="' + PREFIX + '-page-btn">').html('»').attr('title', 'Last');

        $controls.append($first, $prev, $pageInput, ' / ', $pageTotal, $next, $last);
        $pagination.append($controls);

        // Event handlers
        $first.on('click', function() { goToPage(1); });
        $prev.on('click', function() { goToPage(options.page - 1); });
        $next.on('click', function() { goToPage(options.page + 1); });
        $last.on('click', function() { goToPage(options.totalPages); });

        $pageInput.on('change', function() {
            goToPage(parseInt($(this).val(), 10));
        });

        function goToPage(page) {
            if (page < 1) page = 1;
            if (page > options.totalPages) page = options.totalPages;

            if (page !== options.page) {
                options.page = page;
                updateDisplay();

                if (options.onPageChange) {
                    options.onPageChange(page);
                }
            }
        }

        function updateDisplay() {
            var start = (options.page - 1) * options.pageSize + 1;
            var end = Math.min(options.page * options.pageSize, options.totalItems);

            $info.text('Showing ' + start + '-' + end + ' of ' + options.totalItems);
            $pageInput.val(options.page).attr('max', options.totalPages);
            $pageTotal.text(options.totalPages);

            $first.prop('disabled', options.page === 1);
            $prev.prop('disabled', options.page === 1);
            $next.prop('disabled', options.page === options.totalPages);
            $last.prop('disabled', options.page === options.totalPages);
        }

        updateDisplay();

        return {
            element: $pagination,
            setPage: function(page, totalPages, totalItems) {
                options.page = page;
                options.totalPages = totalPages || options.totalPages;
                options.totalItems = totalItems || options.totalItems;
                updateDisplay();
            },
            getPage: function() {
                return options.page;
            }
        };
    }

    /**
     * Create log type tabs
     */
    function createTypeTabs(options) {
        options = $.extend({
            activeType: 'security',
            onTypeChange: null
        }, options);

        var $tabs = $('<div class="' + PREFIX + '-tabs">');

        for (var type in logTypes) {
            var config = logTypes[type];
            var $tab = $('<button type="button" class="' + PREFIX + '-tab">')
                .attr('data-type', type)
                .html('<span class="tab-icon">' + config.icon + '</span> ' + config.label);

            if (type === options.activeType) {
                $tab.addClass('active');
            }

            $tabs.append($tab);
        }

        $tabs.on('click', '.' + PREFIX + '-tab', function() {
            var type = $(this).attr('data-type');
            $tabs.find('.' + PREFIX + '-tab').removeClass('active');
            $(this).addClass('active');

            if (options.onTypeChange) {
                options.onTypeChange(type);
            }
        });

        return {
            element: $tabs,
            setActiveType: function(type) {
                $tabs.find('.' + PREFIX + '-tab').removeClass('active');
                $tabs.find('[data-type="' + type + '"]').addClass('active');
            },
            getActiveType: function() {
                return $tabs.find('.' + PREFIX + '-tab.active').attr('data-type');
            }
        };
    }

    /**
     * Create main logs viewer
     */
    function create(options) {
        options = $.extend({
            container: null,
            logType: 'security',
            pageSize: 50,
            onLoad: null,
            onExport: null
        }, options);

        var $container = $(options.container);
        var $viewer = $('<div class="' + PREFIX + '-viewer">');
        var currentType = options.logType;
        var currentEntries = [];
        var filteredEntries = [];

        // Create components
        var tabs = createTypeTabs({
            activeType: currentType,
            onTypeChange: function(type) {
                currentType = type;
                loadLogs(type);
            }
        });

        var toolbar = createToolbar({
            onRefresh: function() {
                loadLogs(currentType);
            },
            onExport: function() {
                exportLogs();
            }
        });

        var filters = createFilters({
            showSeverity: true,
            showDateRange: true,
            showSearch: true,
            onFilter: function(filterValues) {
                applyFilters(filterValues);
            }
        });

        var table = null;
        var statsView = null;
        var pagination = createPagination({
            pageSize: options.pageSize,
            onPageChange: function(page) {
                renderPage(page);
            }
        });

        // Build structure
        $viewer.append(tabs.element);

        var $header = $('<div class="' + PREFIX + '-header">');
        $header.append(toolbar.element);
        $viewer.append($header);

        $viewer.append(filters.element);

        var $content = $('<div class="' + PREFIX + '-content">');
        $viewer.append($content);

        $viewer.append(pagination.element);

        $container.append($viewer);

        // Load logs
        function loadLogs(type) {
            toolbar.setRefreshing(true);

            if (options.onLoad) {
                options.onLoad(type);
            }

            // Show/hide filters based on type
            if (type === 'statistics') {
                filters.element.hide();
                pagination.element.hide();
            } else {
                filters.element.show();
                pagination.element.show();
            }
        }

        // Set entries
        function setEntries(entries, type) {
            currentEntries = entries || [];
            currentType = type || currentType;
            toolbar.setRefreshing(false);

            // Update content based on type
            $content.empty();

            if (currentType === 'statistics') {
                statsView = createStatisticsView(currentEntries);
                $content.append(statsView.element);
            } else {
                filteredEntries = currentEntries.slice();
                applyFilters(filters.getFilters());
            }
        }

        // Apply filters
        function applyFilters(filterValues) {
            if (currentType === 'statistics') return;

            filteredEntries = currentEntries.filter(function(entry) {
                // Search filter
                if (filterValues.search) {
                    var searchLower = filterValues.search.toLowerCase();
                    var matchesSearch = false;

                    for (var key in entry) {
                        if (String(entry[key]).toLowerCase().indexOf(searchLower) >= 0) {
                            matchesSearch = true;
                            break;
                        }
                    }

                    if (!matchesSearch) return false;
                }

                // Severity filter
                if (filterValues.severity && entry.severity !== filterValues.severity) {
                    return false;
                }

                // Date range filter
                if (filterValues.dateRange && entry.utc) {
                    var entryDate = new Date(entry.utc);
                    var now = new Date();
                    var diffDays = (now - entryDate) / (1000 * 60 * 60 * 24);

                    switch (filterValues.dateRange) {
                        case 'today':
                            if (diffDays > 1) return false;
                            break;
                        case 'week':
                            if (diffDays > 7) return false;
                            break;
                        case 'month':
                            if (diffDays > 30) return false;
                            break;
                    }
                }

                return true;
            });

            // Update pagination
            var totalPages = Math.ceil(filteredEntries.length / options.pageSize) || 1;
            pagination.setPage(1, totalPages, filteredEntries.length);

            // Render first page
            renderPage(1);
        }

        // Render page
        function renderPage(page) {
            if (currentType === 'statistics') return;

            var start = (page - 1) * options.pageSize;
            var end = start + options.pageSize;
            var pageEntries = filteredEntries.slice(start, end);

            $content.empty();
            table = createTable(currentType, {
                entries: pageEntries
            });
            $content.append(table.element);
        }

        // Export logs
        function exportLogs() {
            if (options.onExport) {
                options.onExport(currentType, filteredEntries);
                return;
            }

            // Default CSV export
            var csv = '';
            var config = logTypes[currentType];

            if (config && config.columns) {
                csv += config.columns.join(',') + '\n';

                filteredEntries.forEach(function(entry) {
                    var row = [];
                    config.columns.forEach(function(col) {
                        var key = col.toLowerCase().replace(/\s+/g, '');
                        var value = entry[key] || '';
                        // Escape quotes and wrap in quotes
                        row.push('"' + String(value).replace(/"/g, '""') + '"');
                    });
                    csv += row.join(',') + '\n';
                });
            } else {
                csv = JSON.stringify(filteredEntries, null, 2);
            }

            // Download
            var blob = new Blob([csv], { type: 'text/csv' });
            var url = URL.createObjectURL(blob);
            var a = document.createElement('a');
            a.href = url;
            a.download = currentType + '-logs-' + new Date().toISOString().split('T')[0] + '.csv';
            a.click();
            URL.revokeObjectURL(url);
        }

        // Initial load
        loadLogs(currentType);

        // Public API
        return {
            element: $viewer,

            setEntries: setEntries,

            refresh: function() {
                loadLogs(currentType);
            },

            setLogType: function(type) {
                tabs.setActiveType(type);
                currentType = type;
                loadLogs(type);
            },

            getLogType: function() {
                return currentType;
            },

            getEntries: function() {
                return filteredEntries.slice();
            },

            clearFilters: function() {
                filters.clear();
                applyFilters({});
            },

            destroy: function() {
                toolbar.stopAutoRefresh();
                $viewer.remove();
            }
        };
    }

    // Public API
    return {
        create: create,
        createTable: createTable,
        createFilters: createFilters,
        createToolbar: createToolbar,
        createPagination: createPagination,
        createTypeTabs: createTypeTabs,
        createStatisticsView: createStatisticsView,
        createLogEntry: createLogEntry,
        logTypes: logTypes,
        severityLevels: severityLevels,
        formatBytes: formatBytes,
        formatDate: formatDate,
        PREFIX: PREFIX
    };
}());
