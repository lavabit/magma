/**
 * Magma Search Manager
 *
 * Provides enhanced search functionality including:
 * - Quick search with live filtering
 * - Advanced search form with dynamic filter rows
 * - Multiple filter types (text, date, size)
 * - Search history and saved searches
 * - Search results display
 * - Search suggestions/autocomplete
 */

(function($, magma) {
    'use strict';

    // Search manager namespace
    magma.searchManager = magma.searchManager || {};

    // Filter types
    magma.searchManager.filterTypes = {
        text: {
            label: 'Text',
            fields: ['from', 'to', 'subject', 'body', 'any'],
            operators: ['contains', 'not_contains', 'equals', 'starts_with', 'ends_with']
        },
        date: {
            label: 'Date',
            fields: ['date'],
            operators: ['is', 'before', 'after', 'between']
        },
        size: {
            label: 'Size',
            fields: ['size'],
            operators: ['greater', 'less', 'between'],
            units: ['KB', 'MB', 'GB']
        }
    };

    // Field labels
    magma.searchManager.fieldLabels = {
        from: 'From',
        to: 'To',
        subject: 'Subject',
        body: 'Body',
        any: 'Any Field',
        date: 'Date',
        size: 'Size'
    };

    // Operator labels
    magma.searchManager.operatorLabels = {
        contains: 'Contains',
        not_contains: 'Does not contain',
        equals: 'Equals',
        starts_with: 'Starts with',
        ends_with: 'Ends with',
        is: 'Is',
        before: 'Before',
        after: 'After',
        between: 'Between',
        greater: 'Greater than',
        less: 'Less than'
    };

    /**
     * Create quick search bar
     */
    magma.searchManager.createQuickSearch = function(options) {
        options = $.extend({
            placeholder: 'Search messages...',
            showButton: true,
            showClear: true,
            showAdvanced: true,
            onSearch: function() {},
            onClear: function() {},
            onAdvanced: function() {}
        }, options);

        var $element = $('<div class="mgm-quick-search">');
        var $inputWrapper = $('<div class="mgm-quick-search-input-wrapper">');
        var $icon = $('<span class="mgm-quick-search-icon">&#128269;</span>');
        var $input = $('<input type="text" class="mgm-quick-search-input">');
        var $clear = $('<button type="button" class="mgm-quick-search-clear" title="Clear">&#10005;</button>');
        var $button = $('<button type="button" class="mgm-quick-search-btn">Search</button>');
        var $advanced = $('<button type="button" class="mgm-quick-search-advanced" title="Advanced Search">&#9776;</button>');

        $input.attr('placeholder', options.placeholder);

        $inputWrapper.append($icon, $input);

        if (options.showClear) {
            $clear.hide();
            $inputWrapper.append($clear);
        }

        $element.append($inputWrapper);

        if (options.showButton) {
            $element.append($button);
        }

        if (options.showAdvanced) {
            $element.append($advanced);
        }

        // Event handlers
        $input.on('input', function() {
            var value = $(this).val();
            if (value.length > 0) {
                $clear.show();
            } else {
                $clear.hide();
            }
        });

        $input.on('keypress', function(e) {
            if (e.which === 13) {
                e.preventDefault();
                options.onSearch($input.val());
            }
        });

        $clear.on('click', function() {
            $input.val('').focus();
            $clear.hide();
            options.onClear();
        });

        $button.on('click', function() {
            options.onSearch($input.val());
        });

        $advanced.on('click', function() {
            options.onAdvanced();
        });

        return {
            element: $element,
            getValue: function() {
                return $input.val();
            },
            setValue: function(value) {
                $input.val(value);
                if (value && value.length > 0) {
                    $clear.show();
                } else {
                    $clear.hide();
                }
            },
            clear: function() {
                $input.val('');
                $clear.hide();
            },
            focus: function() {
                $input.focus();
            }
        };
    };

    /**
     * Create search filter row
     */
    magma.searchManager.createFilterRow = function(options) {
        options = $.extend({
            field: 'any',
            operator: 'contains',
            value: '',
            value2: '',
            unit: 'KB',
            removable: true,
            onRemove: function() {},
            onChange: function() {}
        }, options);

        var $element = $('<div class="mgm-search-filter-row">');
        var filterType = getFilterType(options.field);

        // Field selector
        var $fieldSelect = $('<select class="mgm-search-filter-field">');
        $.each(magma.searchManager.fieldLabels, function(value, label) {
            $fieldSelect.append($('<option>').val(value).text(label));
        });
        $fieldSelect.val(options.field);

        // Operator selector
        var $operatorSelect = $('<select class="mgm-search-filter-operator">');
        updateOperators($operatorSelect, filterType);
        $operatorSelect.val(options.operator);

        // Value inputs wrapper
        var $valueWrapper = $('<div class="mgm-search-filter-value-wrapper">');
        var $valueInput = $('<input type="text" class="mgm-search-filter-value">');
        var $value2Input = $('<input type="text" class="mgm-search-filter-value2">');
        var $unitSelect = $('<select class="mgm-search-filter-unit">');

        $valueInput.val(options.value);
        $value2Input.val(options.value2).hide();

        // Unit selector for size
        $.each(magma.searchManager.filterTypes.size.units, function(i, unit) {
            $unitSelect.append($('<option>').val(unit).text(unit));
        });
        $unitSelect.val(options.unit).hide();

        // Date inputs
        var $dateInput = $('<input type="date" class="mgm-search-filter-date">').hide();
        var $date2Input = $('<input type="date" class="mgm-search-filter-date2">').hide();

        $valueWrapper.append($valueInput, $value2Input, $dateInput, $date2Input, $unitSelect);

        // Remove button
        var $removeBtn = $('<button type="button" class="mgm-search-filter-remove" title="Remove">&#10005;</button>');
        if (!options.removable) {
            $removeBtn.hide();
        }

        $element.append($fieldSelect, $operatorSelect, $valueWrapper, $removeBtn);

        // Update UI based on filter type
        function updateUI() {
            var field = $fieldSelect.val();
            var operator = $operatorSelect.val();
            var type = getFilterType(field);

            // Update operators
            updateOperators($operatorSelect, type);

            // Show/hide value inputs based on type
            $valueInput.hide();
            $value2Input.hide();
            $dateInput.hide();
            $date2Input.hide();
            $unitSelect.hide();

            if (type === 'date') {
                $dateInput.show();
                if (operator === 'between') {
                    $date2Input.show();
                }
            } else if (type === 'size') {
                $valueInput.show().attr('type', 'number').attr('min', '0');
                $unitSelect.show();
                if (operator === 'between') {
                    $value2Input.show().attr('type', 'number').attr('min', '0');
                }
            } else {
                $valueInput.show().attr('type', 'text');
            }
        }

        function updateOperators($select, type) {
            var currentVal = $select.val();
            $select.empty();

            var operators = magma.searchManager.filterTypes[type].operators;
            $.each(operators, function(i, op) {
                $select.append($('<option>').val(op).text(magma.searchManager.operatorLabels[op]));
            });

            // Restore value if valid
            if (operators.indexOf(currentVal) !== -1) {
                $select.val(currentVal);
            }
        }

        // Event handlers
        $fieldSelect.on('change', function() {
            updateUI();
            options.onChange(getValues());
        });

        $operatorSelect.on('change', function() {
            updateUI();
            options.onChange(getValues());
        });

        $valueInput.add($value2Input).add($dateInput).add($date2Input).add($unitSelect).on('change input', function() {
            options.onChange(getValues());
        });

        $removeBtn.on('click', function() {
            options.onRemove();
        });

        function getValues() {
            var field = $fieldSelect.val();
            var type = getFilterType(field);

            return {
                field: field,
                operator: $operatorSelect.val(),
                value: type === 'date' ? $dateInput.val() : $valueInput.val(),
                value2: type === 'date' ? $date2Input.val() : $value2Input.val(),
                unit: $unitSelect.val(),
                type: type
            };
        }

        // Initial UI setup
        updateUI();

        return {
            element: $element,
            getValues: getValues,
            setValues: function(values) {
                if (values.field) $fieldSelect.val(values.field);
                if (values.operator) $operatorSelect.val(values.operator);
                if (values.value) {
                    if (getFilterType(values.field) === 'date') {
                        $dateInput.val(values.value);
                    } else {
                        $valueInput.val(values.value);
                    }
                }
                if (values.value2) {
                    if (getFilterType(values.field) === 'date') {
                        $date2Input.val(values.value2);
                    } else {
                        $value2Input.val(values.value2);
                    }
                }
                if (values.unit) $unitSelect.val(values.unit);
                updateUI();
            },
            destroy: function() {
                $element.remove();
            }
        };
    };

    /**
     * Get filter type from field
     */
    function getFilterType(field) {
        if (field === 'date') return 'date';
        if (field === 'size') return 'size';
        return 'text';
    }

    /**
     * Create advanced search form
     */
    magma.searchManager.createAdvancedSearch = function(options) {
        options = $.extend({
            filters: [],
            folders: [],
            showFolderSelect: true,
            showMatchType: true,
            onSearch: function() {},
            onCancel: function() {},
            onSave: function() {}
        }, options);

        var $element = $('<div class="mgm-advanced-search">');
        var $header = $('<div class="mgm-advanced-search-header">');
        var $title = $('<h3>Advanced Search</h3>');
        var $closeBtn = $('<button type="button" class="mgm-advanced-search-close" title="Close">&#10005;</button>');

        $header.append($title, $closeBtn);

        // Options row
        var $optionsRow = $('<div class="mgm-advanced-search-options">');

        // Match type
        if (options.showMatchType) {
            var $matchGroup = $('<div class="mgm-advanced-search-option-group">');
            $matchGroup.append('<label>Match:</label>');
            var $matchSelect = $('<select class="mgm-advanced-search-match">');
            $matchSelect.append(
                $('<option>').val('all').text('All conditions'),
                $('<option>').val('any').text('Any condition')
            );
            $matchGroup.append($matchSelect);
            $optionsRow.append($matchGroup);
        }

        // Folder select
        if (options.showFolderSelect && options.folders.length > 0) {
            var $folderGroup = $('<div class="mgm-advanced-search-option-group">');
            $folderGroup.append('<label>In folder:</label>');
            var $folderSelect = $('<select class="mgm-advanced-search-folder">');
            $folderSelect.append($('<option>').val('').text('All folders'));
            $.each(options.folders, function(i, folder) {
                $folderSelect.append($('<option>').val(folder.id).text(folder.name));
            });
            $folderGroup.append($folderSelect);
            $optionsRow.append($folderGroup);
        }

        // Filters container
        var $filtersContainer = $('<div class="mgm-advanced-search-filters">');
        var filterRows = [];

        // Add filter button
        var $addFilterBtn = $('<button type="button" class="mgm-advanced-search-add-filter">+ Add Filter</button>');

        // Actions
        var $actions = $('<div class="mgm-advanced-search-actions">');
        var $saveBtn = $('<button type="button" class="mgm-advanced-search-btn-save">Save Search</button>');
        var $clearBtn = $('<button type="button" class="mgm-advanced-search-btn-clear">Clear</button>');
        var $searchBtn = $('<button type="button" class="mgm-advanced-search-btn-search">Search</button>');

        $actions.append($saveBtn, $clearBtn, $searchBtn);

        $element.append($header, $optionsRow, $filtersContainer, $addFilterBtn, $actions);

        // Add initial filter if none provided
        function addFilter(filterOptions) {
            var row = magma.searchManager.createFilterRow($.extend({
                removable: filterRows.length > 0,
                onRemove: function() {
                    removeFilter(row);
                },
                onChange: function() {}
            }, filterOptions));

            filterRows.push(row);
            $filtersContainer.append(row.element);

            // Update removable state
            updateRemovable();
        }

        function removeFilter(row) {
            var index = filterRows.indexOf(row);
            if (index !== -1) {
                filterRows.splice(index, 1);
                row.destroy();
                updateRemovable();
            }
        }

        function updateRemovable() {
            $.each(filterRows, function(i, row) {
                row.element.find('.mgm-search-filter-remove').toggle(filterRows.length > 1);
            });
        }

        function getFilters() {
            return $.map(filterRows, function(row) {
                return row.getValues();
            });
        }

        function clearFilters() {
            $.each(filterRows.slice(), function(i, row) {
                row.destroy();
            });
            filterRows = [];
            addFilter();
        }

        // Event handlers
        $closeBtn.on('click', function() {
            options.onCancel();
        });

        $addFilterBtn.on('click', function() {
            addFilter();
        });

        $saveBtn.on('click', function() {
            options.onSave({
                match: $element.find('.mgm-advanced-search-match').val() || 'all',
                folder: $element.find('.mgm-advanced-search-folder').val() || '',
                filters: getFilters()
            });
        });

        $clearBtn.on('click', function() {
            clearFilters();
        });

        $searchBtn.on('click', function() {
            options.onSearch({
                match: $element.find('.mgm-advanced-search-match').val() || 'all',
                folder: $element.find('.mgm-advanced-search-folder').val() || '',
                filters: getFilters()
            });
        });

        // Initialize with provided filters or default
        if (options.filters.length > 0) {
            $.each(options.filters, function(i, filter) {
                addFilter(filter);
            });
        } else {
            addFilter();
        }

        return {
            element: $element,
            getFilters: getFilters,
            setFilters: function(filters) {
                clearFilters();
                $.each(filters, function(i, filter) {
                    addFilter(filter);
                });
            },
            addFilter: addFilter,
            clear: clearFilters,
            show: function() {
                $element.show();
            },
            hide: function() {
                $element.hide();
            },
            destroy: function() {
                $.each(filterRows, function(i, row) {
                    row.destroy();
                });
                $element.remove();
            }
        };
    };

    /**
     * Create search history panel
     */
    magma.searchManager.createSearchHistory = function(options) {
        options = $.extend({
            maxItems: 10,
            storageKey: 'magma_search_history',
            onSelect: function() {},
            onClear: function() {}
        }, options);

        var $element = $('<div class="mgm-search-history">');
        var $header = $('<div class="mgm-search-history-header">');
        var $title = $('<h4>Recent Searches</h4>');
        var $clearBtn = $('<button type="button" class="mgm-search-history-clear">Clear</button>');

        $header.append($title, $clearBtn);

        var $list = $('<ul class="mgm-search-history-list">');
        var $empty = $('<div class="mgm-search-history-empty">No recent searches</div>');

        $element.append($header, $list, $empty);

        var history = loadHistory();

        function loadHistory() {
            try {
                var stored = localStorage.getItem(options.storageKey);
                return stored ? JSON.parse(stored) : [];
            } catch (e) {
                return [];
            }
        }

        function saveHistory() {
            try {
                localStorage.setItem(options.storageKey, JSON.stringify(history));
            } catch (e) {
                // Storage not available
            }
        }

        function renderList() {
            $list.empty();

            if (history.length === 0) {
                $list.hide();
                $empty.show();
                return;
            }

            $empty.hide();
            $list.show();

            $.each(history, function(i, item) {
                var $item = $('<li class="mgm-search-history-item">');
                var $text = $('<span class="mgm-search-history-text">');
                var $remove = $('<button type="button" class="mgm-search-history-remove" title="Remove">&#10005;</button>');

                $text.text(item.query || formatFilters(item.filters));

                $item.append($text, $remove);
                $list.append($item);

                $text.on('click', function() {
                    options.onSelect(item);
                });

                $remove.on('click', function(e) {
                    e.stopPropagation();
                    removeItem(i);
                });
            });
        }

        function formatFilters(filters) {
            if (!filters || filters.length === 0) return 'Empty search';

            return $.map(filters, function(f) {
                return magma.searchManager.fieldLabels[f.field] + ' ' +
                       magma.searchManager.operatorLabels[f.operator] + ' "' + f.value + '"';
            }).join(', ');
        }

        function addItem(item) {
            // Remove duplicate if exists
            history = $.grep(history, function(h) {
                return JSON.stringify(h) !== JSON.stringify(item);
            });

            // Add to beginning
            history.unshift(item);

            // Trim to max items
            if (history.length > options.maxItems) {
                history = history.slice(0, options.maxItems);
            }

            saveHistory();
            renderList();
        }

        function removeItem(index) {
            history.splice(index, 1);
            saveHistory();
            renderList();
        }

        function clearHistory() {
            history = [];
            saveHistory();
            renderList();
            options.onClear();
        }

        // Event handlers
        $clearBtn.on('click', function() {
            clearHistory();
        });

        // Initial render
        renderList();

        return {
            element: $element,
            addItem: addItem,
            getHistory: function() {
                return history.slice();
            },
            clear: clearHistory,
            refresh: function() {
                history = loadHistory();
                renderList();
            }
        };
    };

    /**
     * Create saved searches panel
     */
    magma.searchManager.createSavedSearches = function(options) {
        options = $.extend({
            searches: [],
            storageKey: 'magma_saved_searches',
            onSelect: function() {},
            onDelete: function() {}
        }, options);

        var $element = $('<div class="mgm-saved-searches">');
        var $header = $('<div class="mgm-saved-searches-header">');
        var $title = $('<h4>Saved Searches</h4>');

        $header.append($title);

        var $list = $('<ul class="mgm-saved-searches-list">');
        var $empty = $('<div class="mgm-saved-searches-empty">No saved searches</div>');

        $element.append($header, $list, $empty);

        var searches = loadSearches();

        function loadSearches() {
            try {
                var stored = localStorage.getItem(options.storageKey);
                return stored ? JSON.parse(stored) : options.searches;
            } catch (e) {
                return options.searches;
            }
        }

        function saveSearches() {
            try {
                localStorage.setItem(options.storageKey, JSON.stringify(searches));
            } catch (e) {
                // Storage not available
            }
        }

        function renderList() {
            $list.empty();

            if (searches.length === 0) {
                $list.hide();
                $empty.show();
                return;
            }

            $empty.hide();
            $list.show();

            $.each(searches, function(i, item) {
                var $item = $('<li class="mgm-saved-search-item">');
                var $name = $('<span class="mgm-saved-search-name">');
                var $delete = $('<button type="button" class="mgm-saved-search-delete" title="Delete">&#10005;</button>');

                $name.text(item.name || 'Untitled Search');

                $item.append($name, $delete);
                $list.append($item);

                $name.on('click', function() {
                    options.onSelect(item);
                });

                $delete.on('click', function(e) {
                    e.stopPropagation();
                    deleteSearch(i);
                });
            });
        }

        function addSearch(search) {
            searches.push(search);
            saveSearches();
            renderList();
        }

        function deleteSearch(index) {
            var deleted = searches.splice(index, 1)[0];
            saveSearches();
            renderList();
            options.onDelete(deleted);
        }

        // Initial render
        renderList();

        return {
            element: $element,
            addSearch: addSearch,
            getSearches: function() {
                return searches.slice();
            },
            refresh: function() {
                searches = loadSearches();
                renderList();
            }
        };
    };

    /**
     * Create search results display
     */
    magma.searchManager.createSearchResults = function(options) {
        options = $.extend({
            results: [],
            page: 1,
            pageSize: 25,
            totalResults: 0,
            loading: false,
            onPageChange: function() {},
            onResultClick: function() {},
            onResultSelect: function() {}
        }, options);

        var $element = $('<div class="mgm-search-results">');
        var $header = $('<div class="mgm-search-results-header">');
        var $info = $('<span class="mgm-search-results-info">');
        var $actions = $('<div class="mgm-search-results-actions">');

        $header.append($info, $actions);

        var $content = $('<div class="mgm-search-results-content">');
        var $list = $('<ul class="mgm-search-results-list">');
        var $loading = $('<div class="mgm-search-results-loading">Searching...</div>').hide();
        var $empty = $('<div class="mgm-search-results-empty">No results found</div>').hide();

        $content.append($list, $loading, $empty);

        // Pagination
        var $pagination = $('<div class="mgm-search-results-pagination">');

        $element.append($header, $content, $pagination);

        var state = {
            results: options.results,
            page: options.page,
            pageSize: options.pageSize,
            totalResults: options.totalResults,
            loading: options.loading,
            selected: []
        };

        function render() {
            // Update info
            if (state.loading) {
                $info.text('Searching...');
            } else if (state.totalResults === 0) {
                $info.text('No results');
            } else {
                var start = (state.page - 1) * state.pageSize + 1;
                var end = Math.min(state.page * state.pageSize, state.totalResults);
                $info.text('Showing ' + start + '-' + end + ' of ' + state.totalResults + ' results');
            }

            // Show/hide loading
            $loading.toggle(state.loading);
            $list.toggle(!state.loading && state.results.length > 0);
            $empty.toggle(!state.loading && state.results.length === 0);

            // Render results
            if (!state.loading) {
                renderResults();
                renderPagination();
            }
        }

        function renderResults() {
            $list.empty();

            $.each(state.results, function(i, result) {
                var $item = $('<li class="mgm-search-result-item">');
                var $checkbox = $('<input type="checkbox" class="mgm-search-result-checkbox">');
                var $content = $('<div class="mgm-search-result-content">');
                var $subject = $('<div class="mgm-search-result-subject">');
                var $meta = $('<div class="mgm-search-result-meta">');
                var $snippet = $('<div class="mgm-search-result-snippet">');

                $checkbox.prop('checked', state.selected.indexOf(result.id) !== -1);

                $subject.text(result.subject || '(No subject)');
                $meta.text((result.from || 'Unknown') + ' - ' + formatDate(result.date));
                $snippet.text(result.snippet || '');

                $content.append($subject, $meta, $snippet);
                $item.append($checkbox, $content);
                $list.append($item);

                // Highlight if unread
                if (!result.read) {
                    $item.addClass('unread');
                }

                // Event handlers
                $checkbox.on('change', function() {
                    var checked = $(this).prop('checked');
                    if (checked) {
                        if (state.selected.indexOf(result.id) === -1) {
                            state.selected.push(result.id);
                        }
                    } else {
                        state.selected = $.grep(state.selected, function(id) {
                            return id !== result.id;
                        });
                    }
                    options.onResultSelect(state.selected);
                });

                $content.on('click', function() {
                    options.onResultClick(result);
                });
            });
        }

        function renderPagination() {
            $pagination.empty();

            var totalPages = Math.ceil(state.totalResults / state.pageSize);
            if (totalPages <= 1) {
                $pagination.hide();
                return;
            }

            $pagination.show();

            var $prevBtn = $('<button type="button" class="mgm-search-page-btn">&laquo;</button>');
            var $nextBtn = $('<button type="button" class="mgm-search-page-btn">&raquo;</button>');
            var $pageInfo = $('<span class="mgm-search-page-info">');

            $prevBtn.prop('disabled', state.page <= 1);
            $nextBtn.prop('disabled', state.page >= totalPages);
            $pageInfo.text('Page ' + state.page + ' of ' + totalPages);

            $prevBtn.on('click', function() {
                if (state.page > 1) {
                    state.page--;
                    options.onPageChange(state.page);
                }
            });

            $nextBtn.on('click', function() {
                if (state.page < totalPages) {
                    state.page++;
                    options.onPageChange(state.page);
                }
            });

            $pagination.append($prevBtn, $pageInfo, $nextBtn);
        }

        function formatDate(date) {
            if (!date) return '';
            var d = new Date(date);
            return d.toLocaleDateString() + ' ' + d.toLocaleTimeString();
        }

        // Initial render
        render();

        return {
            element: $element,
            setResults: function(results, page, totalResults) {
                state.results = results;
                state.page = page || 1;
                state.totalResults = totalResults || results.length;
                state.loading = false;
                render();
            },
            setLoading: function(loading) {
                state.loading = loading;
                render();
            },
            getSelected: function() {
                return state.selected.slice();
            },
            clearSelected: function() {
                state.selected = [];
                render();
            },
            clear: function() {
                state.results = [];
                state.totalResults = 0;
                state.selected = [];
                render();
            }
        };
    };

    /**
     * Create search suggestions dropdown
     */
    magma.searchManager.createSuggestions = function(options) {
        options = $.extend({
            maxItems: 5,
            onSelect: function() {}
        }, options);

        var $element = $('<div class="mgm-search-suggestions">').hide();
        var $list = $('<ul class="mgm-search-suggestions-list">');

        $element.append($list);

        var suggestions = [];
        var selectedIndex = -1;

        function render() {
            $list.empty();

            if (suggestions.length === 0) {
                $element.hide();
                return;
            }

            $.each(suggestions, function(i, suggestion) {
                var $item = $('<li class="mgm-search-suggestion-item">');
                var $icon = $('<span class="mgm-search-suggestion-icon">');
                var $text = $('<span class="mgm-search-suggestion-text">');

                if (suggestion.type === 'history') {
                    $icon.html('&#128337;'); // Clock
                } else if (suggestion.type === 'saved') {
                    $icon.html('&#128190;'); // Floppy
                } else {
                    $icon.html('&#128269;'); // Magnifier
                }

                $text.text(suggestion.text);

                $item.append($icon, $text);
                $list.append($item);

                if (i === selectedIndex) {
                    $item.addClass('selected');
                }

                $item.on('click', function() {
                    options.onSelect(suggestion);
                    hide();
                });

                $item.on('mouseenter', function() {
                    selectedIndex = i;
                    updateSelection();
                });
            });

            $element.show();
        }

        function updateSelection() {
            $list.find('.mgm-search-suggestion-item').removeClass('selected');
            if (selectedIndex >= 0 && selectedIndex < suggestions.length) {
                $list.find('.mgm-search-suggestion-item').eq(selectedIndex).addClass('selected');
            }
        }

        function hide() {
            suggestions = [];
            selectedIndex = -1;
            $element.hide();
        }

        return {
            element: $element,
            setSuggestions: function(items) {
                suggestions = items.slice(0, options.maxItems);
                selectedIndex = -1;
                render();
            },
            selectNext: function() {
                if (suggestions.length === 0) return;
                selectedIndex = (selectedIndex + 1) % suggestions.length;
                updateSelection();
            },
            selectPrev: function() {
                if (suggestions.length === 0) return;
                selectedIndex = selectedIndex <= 0 ? suggestions.length - 1 : selectedIndex - 1;
                updateSelection();
            },
            getSelected: function() {
                if (selectedIndex >= 0 && selectedIndex < suggestions.length) {
                    return suggestions[selectedIndex];
                }
                return null;
            },
            hide: hide,
            isVisible: function() {
                return $element.is(':visible');
            }
        };
    };

    /**
     * Create main search component
     */
    magma.searchManager.create = function(options) {
        options = $.extend({
            container: null,
            folders: [],
            showQuickSearch: true,
            showAdvanced: true,
            showHistory: true,
            showSaved: true,
            showSuggestions: true,
            onSearch: function() {},
            onResultClick: function() {}
        }, options);

        var $container = options.container ? $(options.container) : $('<div>');
        var $element = $('<div class="mgm-search-manager">');

        // Components
        var quickSearch = null;
        var advancedSearch = null;
        var searchHistory = null;
        var savedSearches = null;
        var searchResults = null;
        var suggestions = null;

        // Quick search
        if (options.showQuickSearch) {
            quickSearch = magma.searchManager.createQuickSearch({
                onSearch: function(query) {
                    performSearch({ query: query });
                },
                onClear: function() {
                    searchResults && searchResults.clear();
                },
                onAdvanced: function() {
                    toggleAdvanced();
                }
            });
            $element.append(quickSearch.element);

            // Suggestions
            if (options.showSuggestions) {
                suggestions = magma.searchManager.createSuggestions({
                    onSelect: function(suggestion) {
                        if (suggestion.type === 'saved') {
                            performSearch(suggestion.search);
                        } else {
                            quickSearch.setValue(suggestion.text);
                            performSearch({ query: suggestion.text });
                        }
                    }
                });
                quickSearch.element.append(suggestions.element);
            }
        }

        // Advanced search
        if (options.showAdvanced) {
            advancedSearch = magma.searchManager.createAdvancedSearch({
                folders: options.folders,
                onSearch: function(searchParams) {
                    performSearch(searchParams);
                },
                onCancel: function() {
                    advancedSearch.hide();
                },
                onSave: function(searchParams) {
                    saveSearch(searchParams);
                }
            });
            advancedSearch.hide();
            $element.append(advancedSearch.element);
        }

        // Sidebar (history + saved)
        if (options.showHistory || options.showSaved) {
            var $sidebar = $('<div class="mgm-search-sidebar">');

            if (options.showHistory) {
                searchHistory = magma.searchManager.createSearchHistory({
                    onSelect: function(item) {
                        if (item.query) {
                            quickSearch && quickSearch.setValue(item.query);
                            performSearch({ query: item.query });
                        } else {
                            performSearch(item);
                        }
                    }
                });
                $sidebar.append(searchHistory.element);
            }

            if (options.showSaved) {
                savedSearches = magma.searchManager.createSavedSearches({
                    onSelect: function(search) {
                        performSearch(search);
                    }
                });
                $sidebar.append(savedSearches.element);
            }

            $element.append($sidebar);
        }

        // Search results
        searchResults = magma.searchManager.createSearchResults({
            onPageChange: function(page) {
                // Would trigger search with new page
            },
            onResultClick: function(result) {
                options.onResultClick(result);
            }
        });
        $element.append(searchResults.element);

        $container.append($element);

        function performSearch(params) {
            // Add to history
            if (searchHistory) {
                searchHistory.addItem(params);
            }

            // Show loading
            searchResults.setLoading(true);

            // Call search callback
            options.onSearch(params, function(results, totalResults) {
                searchResults.setResults(results, 1, totalResults);
            });
        }

        function toggleAdvanced() {
            if (advancedSearch) {
                if (advancedSearch.element.is(':visible')) {
                    advancedSearch.hide();
                } else {
                    advancedSearch.show();
                }
            }
        }

        function saveSearch(searchParams) {
            var name = prompt('Enter a name for this search:');
            if (name) {
                searchParams.name = name;
                if (savedSearches) {
                    savedSearches.addSearch(searchParams);
                }
            }
        }

        return {
            element: $element,
            search: function(params) {
                performSearch(params);
            },
            setResults: function(results, page, totalResults) {
                searchResults.setResults(results, page, totalResults);
            },
            setLoading: function(loading) {
                searchResults.setLoading(loading);
            },
            showAdvanced: function() {
                advancedSearch && advancedSearch.show();
            },
            hideAdvanced: function() {
                advancedSearch && advancedSearch.hide();
            },
            clear: function() {
                quickSearch && quickSearch.clear();
                advancedSearch && advancedSearch.clear();
                searchResults && searchResults.clear();
            },
            getQuickSearch: function() {
                return quickSearch;
            },
            getAdvancedSearch: function() {
                return advancedSearch;
            },
            getHistory: function() {
                return searchHistory;
            },
            getSavedSearches: function() {
                return savedSearches;
            },
            getResults: function() {
                return searchResults;
            },
            destroy: function() {
                advancedSearch && advancedSearch.destroy();
                $element.remove();
            }
        };
    };

})(jQuery, magma);
