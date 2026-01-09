/**
 * Search Manager Tests
 *
 * Tests for quick search, advanced search, filters,
 * history, saved searches, and results display
 */

module('search-manager');

test('searchManager exists', function() {
    ok(magma.searchManager, 'searchManager exists');
    ok(typeof magma.searchManager.create === 'function', 'create exists');
    ok(typeof magma.searchManager.createQuickSearch === 'function', 'createQuickSearch exists');
    ok(typeof magma.searchManager.createAdvancedSearch === 'function', 'createAdvancedSearch exists');
    ok(typeof magma.searchManager.createFilterRow === 'function', 'createFilterRow exists');
    ok(typeof magma.searchManager.createSearchHistory === 'function', 'createSearchHistory exists');
    ok(typeof magma.searchManager.createSavedSearches === 'function', 'createSavedSearches exists');
    ok(typeof magma.searchManager.createSearchResults === 'function', 'createSearchResults exists');
    ok(typeof magma.searchManager.createSuggestions === 'function', 'createSuggestions exists');
    ok(magma.searchManager.filterTypes, 'filterTypes exists');
    ok(magma.searchManager.fieldLabels, 'fieldLabels exists');
    ok(magma.searchManager.operatorLabels, 'operatorLabels exists');
});

// Filter types tests
test('filterTypes includes all expected types', function() {
    var types = magma.searchManager.filterTypes;

    ok(types.text, 'Has text type');
    ok(types.date, 'Has date type');
    ok(types.size, 'Has size type');
});

test('filterTypes have required properties', function() {
    var types = magma.searchManager.filterTypes;

    for (var name in types) {
        ok(types[name].label, name + ' has label');
        ok(types[name].fields, name + ' has fields');
        ok(types[name].operators, name + ' has operators');
    }
});

// Field labels tests
test('fieldLabels includes all expected fields', function() {
    var labels = magma.searchManager.fieldLabels;

    ok(labels.from, 'Has from field');
    ok(labels.to, 'Has to field');
    ok(labels.subject, 'Has subject field');
    ok(labels.body, 'Has body field');
    ok(labels.any, 'Has any field');
    ok(labels.date, 'Has date field');
    ok(labels.size, 'Has size field');
});

// Operator labels tests
test('operatorLabels includes all expected operators', function() {
    var labels = magma.searchManager.operatorLabels;

    ok(labels.contains, 'Has contains operator');
    ok(labels.not_contains, 'Has not_contains operator');
    ok(labels.equals, 'Has equals operator');
    ok(labels.before, 'Has before operator');
    ok(labels.after, 'Has after operator');
    ok(labels.between, 'Has between operator');
    ok(labels.greater, 'Has greater operator');
    ok(labels.less, 'Has less operator');
});

// Quick search tests
test('createQuickSearch returns search API', function() {
    var quickSearch = magma.searchManager.createQuickSearch();

    ok(quickSearch.element.length, 'Has element');
    ok(typeof quickSearch.getValue === 'function', 'Has getValue');
    ok(typeof quickSearch.setValue === 'function', 'Has setValue');
    ok(typeof quickSearch.clear === 'function', 'Has clear');
    ok(typeof quickSearch.focus === 'function', 'Has focus');
});

test('createQuickSearch has input field', function() {
    var quickSearch = magma.searchManager.createQuickSearch();

    ok(quickSearch.element.find('.mgm-quick-search-input').length, 'Has input');
});

test('createQuickSearch has search button', function() {
    var quickSearch = magma.searchManager.createQuickSearch({
        showButton: true
    });

    ok(quickSearch.element.find('.mgm-quick-search-btn').length, 'Has search button');
});

test('createQuickSearch has advanced button', function() {
    var quickSearch = magma.searchManager.createQuickSearch({
        showAdvanced: true
    });

    ok(quickSearch.element.find('.mgm-quick-search-advanced').length, 'Has advanced button');
});

test('createQuickSearch getValue returns input value', function() {
    var quickSearch = magma.searchManager.createQuickSearch();

    quickSearch.element.find('.mgm-quick-search-input').val('test query');
    equal(quickSearch.getValue(), 'test query', 'Returns input value');
});

test('createQuickSearch setValue sets input value', function() {
    var quickSearch = magma.searchManager.createQuickSearch();

    quickSearch.setValue('new query');
    equal(quickSearch.element.find('.mgm-quick-search-input').val(), 'new query', 'Sets input value');
});

test('createQuickSearch clear empties input', function() {
    var quickSearch = magma.searchManager.createQuickSearch();

    quickSearch.setValue('test');
    quickSearch.clear();
    equal(quickSearch.getValue(), '', 'Input cleared');
});

test('createQuickSearch calls onSearch callback', function() {
    expect(1);

    var searchQuery = null;
    var quickSearch = magma.searchManager.createQuickSearch({
        onSearch: function(query) {
            searchQuery = query;
        }
    });

    quickSearch.element.find('.mgm-quick-search-input').val('test');
    quickSearch.element.find('.mgm-quick-search-btn').click();

    equal(searchQuery, 'test', 'onSearch called with query');
});

// Filter row tests
test('createFilterRow returns filter API', function() {
    var filter = magma.searchManager.createFilterRow();

    ok(filter.element.length, 'Has element');
    ok(typeof filter.getValues === 'function', 'Has getValues');
    ok(typeof filter.setValues === 'function', 'Has setValues');
    ok(typeof filter.destroy === 'function', 'Has destroy');
});

test('createFilterRow has field selector', function() {
    var filter = magma.searchManager.createFilterRow();

    ok(filter.element.find('.mgm-search-filter-field').length, 'Has field selector');
});

test('createFilterRow has operator selector', function() {
    var filter = magma.searchManager.createFilterRow();

    ok(filter.element.find('.mgm-search-filter-operator').length, 'Has operator selector');
});

test('createFilterRow has value input', function() {
    var filter = magma.searchManager.createFilterRow();

    ok(filter.element.find('.mgm-search-filter-value').length, 'Has value input');
});

test('createFilterRow getValues returns filter values', function() {
    var filter = magma.searchManager.createFilterRow({
        field: 'from',
        operator: 'contains',
        value: 'test@example.com'
    });

    var values = filter.getValues();

    equal(values.field, 'from', 'Returns field');
    equal(values.operator, 'contains', 'Returns operator');
    equal(values.type, 'text', 'Returns type');
});

test('createFilterRow setValues updates filter', function() {
    var filter = magma.searchManager.createFilterRow();

    filter.setValues({
        field: 'subject',
        operator: 'equals',
        value: 'Important'
    });

    var values = filter.getValues();
    equal(values.field, 'subject', 'Field updated');
});

test('createFilterRow calls onRemove callback', function() {
    expect(1);

    var removed = false;
    var filter = magma.searchManager.createFilterRow({
        removable: true,
        onRemove: function() {
            removed = true;
        }
    });

    filter.element.find('.mgm-search-filter-remove').click();
    ok(removed, 'onRemove called');
});

// Advanced search tests
test('createAdvancedSearch returns advanced search API', function() {
    var advanced = magma.searchManager.createAdvancedSearch();

    ok(advanced.element.length, 'Has element');
    ok(typeof advanced.getFilters === 'function', 'Has getFilters');
    ok(typeof advanced.setFilters === 'function', 'Has setFilters');
    ok(typeof advanced.addFilter === 'function', 'Has addFilter');
    ok(typeof advanced.clear === 'function', 'Has clear');
    ok(typeof advanced.show === 'function', 'Has show');
    ok(typeof advanced.hide === 'function', 'Has hide');
    ok(typeof advanced.destroy === 'function', 'Has destroy');

    advanced.destroy();
});

test('createAdvancedSearch has header', function() {
    var advanced = magma.searchManager.createAdvancedSearch();

    ok(advanced.element.find('.mgm-advanced-search-header').length, 'Has header');

    advanced.destroy();
});

test('createAdvancedSearch has filters container', function() {
    var advanced = magma.searchManager.createAdvancedSearch();

    ok(advanced.element.find('.mgm-advanced-search-filters').length, 'Has filters container');

    advanced.destroy();
});

test('createAdvancedSearch has add filter button', function() {
    var advanced = magma.searchManager.createAdvancedSearch();

    ok(advanced.element.find('.mgm-advanced-search-add-filter').length, 'Has add filter button');

    advanced.destroy();
});

test('createAdvancedSearch has action buttons', function() {
    var advanced = magma.searchManager.createAdvancedSearch();

    ok(advanced.element.find('.mgm-advanced-search-btn-save').length, 'Has save button');
    ok(advanced.element.find('.mgm-advanced-search-btn-clear').length, 'Has clear button');
    ok(advanced.element.find('.mgm-advanced-search-btn-search').length, 'Has search button');

    advanced.destroy();
});

test('createAdvancedSearch getFilters returns filter array', function() {
    var advanced = magma.searchManager.createAdvancedSearch();

    var filters = advanced.getFilters();
    ok($.isArray(filters), 'Returns array');
    equal(filters.length, 1, 'Has default filter');

    advanced.destroy();
});

test('createAdvancedSearch addFilter adds new filter', function() {
    var advanced = magma.searchManager.createAdvancedSearch();

    advanced.addFilter({ field: 'subject', operator: 'contains' });
    var filters = advanced.getFilters();

    equal(filters.length, 2, 'Filter added');

    advanced.destroy();
});

test('createAdvancedSearch clear resets filters', function() {
    var advanced = magma.searchManager.createAdvancedSearch();

    advanced.addFilter();
    advanced.addFilter();
    advanced.clear();

    var filters = advanced.getFilters();
    equal(filters.length, 1, 'Filters reset to one');

    advanced.destroy();
});

test('createAdvancedSearch calls onSearch callback', function() {
    expect(1);

    var searchParams = null;
    var advanced = magma.searchManager.createAdvancedSearch({
        onSearch: function(params) {
            searchParams = params;
        }
    });

    advanced.element.find('.mgm-advanced-search-btn-search').click();
    ok(searchParams, 'onSearch called');

    advanced.destroy();
});

// Search history tests
test('createSearchHistory returns history API', function() {
    var history = magma.searchManager.createSearchHistory({
        storageKey: 'test_history_' + Date.now()
    });

    ok(history.element.length, 'Has element');
    ok(typeof history.addItem === 'function', 'Has addItem');
    ok(typeof history.getHistory === 'function', 'Has getHistory');
    ok(typeof history.clear === 'function', 'Has clear');
    ok(typeof history.refresh === 'function', 'Has refresh');
});

test('createSearchHistory has header', function() {
    var history = magma.searchManager.createSearchHistory({
        storageKey: 'test_history_' + Date.now()
    });

    ok(history.element.find('.mgm-search-history-header').length, 'Has header');
});

test('createSearchHistory has list', function() {
    var history = magma.searchManager.createSearchHistory({
        storageKey: 'test_history_' + Date.now()
    });

    ok(history.element.find('.mgm-search-history-list').length, 'Has list');
});

test('createSearchHistory addItem adds to history', function() {
    var history = magma.searchManager.createSearchHistory({
        storageKey: 'test_history_' + Date.now()
    });

    history.addItem({ query: 'test search' });
    var items = history.getHistory();

    equal(items.length, 1, 'Item added');
    equal(items[0].query, 'test search', 'Query stored');
});

test('createSearchHistory clear empties history', function() {
    var history = magma.searchManager.createSearchHistory({
        storageKey: 'test_history_' + Date.now()
    });

    history.addItem({ query: 'test1' });
    history.addItem({ query: 'test2' });
    history.clear();

    equal(history.getHistory().length, 0, 'History cleared');
});

test('createSearchHistory respects maxItems', function() {
    var history = magma.searchManager.createSearchHistory({
        storageKey: 'test_history_' + Date.now(),
        maxItems: 3
    });

    history.addItem({ query: 'test1' });
    history.addItem({ query: 'test2' });
    history.addItem({ query: 'test3' });
    history.addItem({ query: 'test4' });

    equal(history.getHistory().length, 3, 'Respects max items');
});

// Saved searches tests
test('createSavedSearches returns saved searches API', function() {
    var saved = magma.searchManager.createSavedSearches({
        storageKey: 'test_saved_' + Date.now()
    });

    ok(saved.element.length, 'Has element');
    ok(typeof saved.addSearch === 'function', 'Has addSearch');
    ok(typeof saved.getSearches === 'function', 'Has getSearches');
    ok(typeof saved.refresh === 'function', 'Has refresh');
});

test('createSavedSearches has header', function() {
    var saved = magma.searchManager.createSavedSearches({
        storageKey: 'test_saved_' + Date.now()
    });

    ok(saved.element.find('.mgm-saved-searches-header').length, 'Has header');
});

test('createSavedSearches has list', function() {
    var saved = magma.searchManager.createSavedSearches({
        storageKey: 'test_saved_' + Date.now()
    });

    ok(saved.element.find('.mgm-saved-searches-list').length, 'Has list');
});

test('createSavedSearches addSearch adds saved search', function() {
    var saved = magma.searchManager.createSavedSearches({
        storageKey: 'test_saved_' + Date.now()
    });

    saved.addSearch({ name: 'My Search', filters: [] });
    var searches = saved.getSearches();

    equal(searches.length, 1, 'Search added');
    equal(searches[0].name, 'My Search', 'Name stored');
});

// Search results tests
test('createSearchResults returns results API', function() {
    var results = magma.searchManager.createSearchResults();

    ok(results.element.length, 'Has element');
    ok(typeof results.setResults === 'function', 'Has setResults');
    ok(typeof results.setLoading === 'function', 'Has setLoading');
    ok(typeof results.getSelected === 'function', 'Has getSelected');
    ok(typeof results.clearSelected === 'function', 'Has clearSelected');
    ok(typeof results.clear === 'function', 'Has clear');
});

test('createSearchResults has header', function() {
    var results = magma.searchManager.createSearchResults();

    ok(results.element.find('.mgm-search-results-header').length, 'Has header');
});

test('createSearchResults has content area', function() {
    var results = magma.searchManager.createSearchResults();

    ok(results.element.find('.mgm-search-results-content').length, 'Has content');
});

test('createSearchResults has pagination', function() {
    var results = magma.searchManager.createSearchResults();

    ok(results.element.find('.mgm-search-results-pagination').length, 'Has pagination');
});

test('createSearchResults setResults displays results', function() {
    var results = magma.searchManager.createSearchResults();

    results.setResults([
        { id: 1, subject: 'Test Email', from: 'test@example.com' },
        { id: 2, subject: 'Another Email', from: 'other@example.com' }
    ], 1, 2);

    var items = results.element.find('.mgm-search-result-item');
    equal(items.length, 2, 'Shows 2 results');
});

test('createSearchResults setLoading shows loading state', function() {
    var results = magma.searchManager.createSearchResults();

    results.setLoading(true);
    ok(results.element.find('.mgm-search-results-loading:visible').length, 'Shows loading');
});

test('createSearchResults clear removes results', function() {
    var results = magma.searchManager.createSearchResults();

    results.setResults([
        { id: 1, subject: 'Test', from: 'test@example.com' }
    ], 1, 1);

    results.clear();
    equal(results.element.find('.mgm-search-result-item').length, 0, 'Results cleared');
});

test('createSearchResults getSelected returns selected IDs', function() {
    var results = magma.searchManager.createSearchResults();

    results.setResults([
        { id: 1, subject: 'Test 1' },
        { id: 2, subject: 'Test 2' }
    ], 1, 2);

    results.element.find('.mgm-search-result-checkbox').first().prop('checked', true).trigger('change');

    var selected = results.getSelected();
    equal(selected.length, 1, 'One selected');
    equal(selected[0], 1, 'Correct ID selected');
});

// Suggestions tests
test('createSuggestions returns suggestions API', function() {
    var suggestions = magma.searchManager.createSuggestions();

    ok(suggestions.element.length, 'Has element');
    ok(typeof suggestions.setSuggestions === 'function', 'Has setSuggestions');
    ok(typeof suggestions.selectNext === 'function', 'Has selectNext');
    ok(typeof suggestions.selectPrev === 'function', 'Has selectPrev');
    ok(typeof suggestions.getSelected === 'function', 'Has getSelected');
    ok(typeof suggestions.hide === 'function', 'Has hide');
    ok(typeof suggestions.isVisible === 'function', 'Has isVisible');
});

test('createSuggestions setSuggestions shows suggestions', function() {
    var suggestions = magma.searchManager.createSuggestions();

    suggestions.setSuggestions([
        { text: 'Suggestion 1', type: 'history' },
        { text: 'Suggestion 2', type: 'saved' }
    ]);

    ok(suggestions.isVisible(), 'Is visible');
    equal(suggestions.element.find('.mgm-search-suggestion-item').length, 2, 'Shows 2 suggestions');
});

test('createSuggestions selectNext cycles selection', function() {
    var suggestions = magma.searchManager.createSuggestions();

    suggestions.setSuggestions([
        { text: 'Suggestion 1', type: 'history' },
        { text: 'Suggestion 2', type: 'history' }
    ]);

    suggestions.selectNext();
    var selected = suggestions.getSelected();
    equal(selected.text, 'Suggestion 1', 'First item selected');

    suggestions.selectNext();
    selected = suggestions.getSelected();
    equal(selected.text, 'Suggestion 2', 'Second item selected');
});

test('createSuggestions hide clears suggestions', function() {
    var suggestions = magma.searchManager.createSuggestions();

    suggestions.setSuggestions([{ text: 'Test', type: 'history' }]);
    suggestions.hide();

    ok(!suggestions.isVisible(), 'Not visible');
});

// Main component tests
test('create returns search manager API', function() {
    var $container = $('<div>');
    var manager = magma.searchManager.create({
        container: $container
    });

    ok(manager.element.length, 'Has element');
    ok(typeof manager.search === 'function', 'Has search');
    ok(typeof manager.setResults === 'function', 'Has setResults');
    ok(typeof manager.setLoading === 'function', 'Has setLoading');
    ok(typeof manager.showAdvanced === 'function', 'Has showAdvanced');
    ok(typeof manager.hideAdvanced === 'function', 'Has hideAdvanced');
    ok(typeof manager.clear === 'function', 'Has clear');
    ok(typeof manager.destroy === 'function', 'Has destroy');

    manager.destroy();
});

test('create has quick search', function() {
    var $container = $('<div>');
    var manager = magma.searchManager.create({
        container: $container,
        showQuickSearch: true
    });

    ok($container.find('.mgm-quick-search').length, 'Has quick search');

    manager.destroy();
});

test('create has search results', function() {
    var $container = $('<div>');
    var manager = magma.searchManager.create({
        container: $container
    });

    ok($container.find('.mgm-search-results').length, 'Has search results');

    manager.destroy();
});

test('create getQuickSearch returns quick search', function() {
    var $container = $('<div>');
    var manager = magma.searchManager.create({
        container: $container,
        showQuickSearch: true
    });

    var quickSearch = manager.getQuickSearch();
    ok(quickSearch, 'Returns quick search');
    ok(quickSearch.element.length, 'Quick search has element');

    manager.destroy();
});

test('create getAdvancedSearch returns advanced search', function() {
    var $container = $('<div>');
    var manager = magma.searchManager.create({
        container: $container,
        showAdvanced: true
    });

    var advanced = manager.getAdvancedSearch();
    ok(advanced, 'Returns advanced search');
    ok(advanced.element.length, 'Advanced search has element');

    manager.destroy();
});

test('create setResults updates results display', function() {
    var $container = $('<div>');
    var manager = magma.searchManager.create({
        container: $container
    });

    manager.setResults([
        { id: 1, subject: 'Test Email' }
    ], 1, 1);

    ok($container.find('.mgm-search-result-item').length, 'Shows results');

    manager.destroy();
});

test('create clear resets all components', function() {
    var $container = $('<div>');
    var manager = magma.searchManager.create({
        container: $container,
        showQuickSearch: true
    });

    var quickSearch = manager.getQuickSearch();
    quickSearch.setValue('test');
    manager.setResults([{ id: 1, subject: 'Test' }], 1, 1);

    manager.clear();

    equal(quickSearch.getValue(), '', 'Quick search cleared');
    equal($container.find('.mgm-search-result-item').length, 0, 'Results cleared');

    manager.destroy();
});

test('destroy removes element', function() {
    var $container = $('<div>');
    var manager = magma.searchManager.create({
        container: $container
    });

    ok($container.find('.mgm-search-manager').length, 'Manager in DOM');

    manager.destroy();
    equal($container.find('.mgm-search-manager').length, 0, 'Manager removed');
});
