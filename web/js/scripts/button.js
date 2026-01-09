/**
 * Magma Button Component
 *
 * Unified button system providing consistent API for all button types.
 *
 * Layouts:
 *   - 'icon-text': Icon on left, text on right (default)
 *   - 'icon': Icon only
 *   - 'text': Text only
 *   - 'shortcut': Icon with keyboard shortcut overlay
 *
 * Types:
 *   - 'button': Standard clickable button (default)
 *   - 'toggle': Toggles between active/inactive states
 *   - 'dropdown': Opens a dropdown menu
 *
 * States:
 *   - 'default': Normal state
 *   - 'hover': Mouse over
 *   - 'active': Pressed/selected
 *   - 'disabled': Cannot be clicked
 *   - 'pending': Loading/processing
 *
 * Usage:
 *   var btn = magma.button.create({
 *       icon: 'send',
 *       text: 'Send Message',
 *       layout: 'icon-text',
 *       type: 'button',
 *       shortcut: 'Ctrl+Enter',
 *       onClick: function() { ... }
 *   });
 *   btn.appendTo(container);
 *   btn.disable();
 *   btn.enable();
 *   btn.setPending(true);
 */

var magma = magma || {};

magma.button = (function() {
    'use strict';

    // CSS class prefix for all button classes
    var PREFIX = 'mgm-btn';

    // Counter for unique IDs
    var idCounter = 0;

    /**
     * Generate unique button ID
     */
    function generateId() {
        return PREFIX + '-' + (++idCounter);
    }

    /**
     * Create a button element
     * @param {Object} options - Button configuration
     * @returns {Object} Button API object
     */
    function create(options) {
        options = $.extend({
            icon: null,
            text: '',
            layout: 'icon-text',
            type: 'button',
            shortcut: null,
            disabled: false,
            active: false,
            onClick: null,
            onToggle: null,
            dropdownContent: null
        }, options);

        var id = generateId();
        var element;
        var isActive = options.active;
        var isDisabled = options.disabled;
        var isPending = false;

        // Build the button HTML based on type
        if (options.type === 'toggle' || options.type === 'dropdown') {
            // Checkbox-based button for toggle/dropdown
            element = $('<span class="' + PREFIX + ' ' + PREFIX + '-' + options.type + '">' +
                '<input type="checkbox" id="' + id + '" class="' + PREFIX + '-input" />' +
                '<label for="' + id + '" class="' + PREFIX + '-label"></label>' +
                '</span>');
        } else {
            // Standard button
            element = $('<a href="#" class="' + PREFIX + ' ' + PREFIX + '-standard" id="' + id + '"></a>');
        }

        // Add icon class for theming
        if (options.icon) {
            element.addClass(PREFIX + '-icon-' + options.icon);
        }

        // Build inner content based on layout
        var label = element.find('.' + PREFIX + '-label');
        if (!label.length) {
            label = element;
        }

        var content = buildContent(options);
        label.html(content);

        // Add layout class
        element.addClass(PREFIX + '-layout-' + options.layout);

        // Set initial states
        if (isDisabled) {
            element.addClass(PREFIX + '-disabled');
        }
        if (isActive) {
            element.addClass(PREFIX + '-active');
            element.find('input').prop('checked', true);
        }

        // Add dropdown indicator
        if (options.type === 'dropdown') {
            label.append('<span class="' + PREFIX + '-dropdown-arrow"></span>');
        }

        // Event handlers
        element.on('click', '.' + PREFIX + '-label, a', function(e) {
            e.preventDefault();

            if (isDisabled || isPending) {
                return;
            }

            if (options.type === 'toggle') {
                isActive = !isActive;
                element.toggleClass(PREFIX + '-active', isActive);
                element.find('input').prop('checked', isActive);
                if (options.onToggle) {
                    options.onToggle(isActive);
                }
            } else if (options.type === 'dropdown') {
                isActive = !isActive;
                element.toggleClass(PREFIX + '-active', isActive);
                element.find('input').prop('checked', isActive);
                if (options.onToggle) {
                    options.onToggle(isActive);
                }
            }

            if (options.onClick) {
                options.onClick(e);
            }
        });

        // Prevent default on standard buttons
        element.on('click', function(e) {
            if (element.is('a')) {
                e.preventDefault();
            }
        });

        // Public API
        var api = {
            element: element,

            /**
             * Append button to container
             */
            appendTo: function(container) {
                element.appendTo(container);
                return api;
            },

            /**
             * Prepend button to container
             */
            prependTo: function(container) {
                element.prependTo(container);
                return api;
            },

            /**
             * Insert after element
             */
            insertAfter: function(target) {
                element.insertAfter(target);
                return api;
            },

            /**
             * Insert before element
             */
            insertBefore: function(target) {
                element.insertBefore(target);
                return api;
            },

            /**
             * Disable the button
             */
            disable: function() {
                isDisabled = true;
                element.addClass(PREFIX + '-disabled');
                element.find('input').prop('disabled', true);
                return api;
            },

            /**
             * Enable the button
             */
            enable: function() {
                isDisabled = false;
                element.removeClass(PREFIX + '-disabled');
                element.find('input').prop('disabled', false);
                return api;
            },

            /**
             * Check if button is disabled
             */
            isDisabled: function() {
                return isDisabled;
            },

            /**
             * Set pending/loading state
             */
            setPending: function(pending) {
                isPending = pending;
                element.toggleClass(PREFIX + '-pending', pending);
                if (pending) {
                    element.addClass(PREFIX + '-disabled');
                } else if (!isDisabled) {
                    element.removeClass(PREFIX + '-disabled');
                }
                return api;
            },

            /**
             * Check if button is pending
             */
            isPending: function() {
                return isPending;
            },

            /**
             * Set active state (for toggles)
             */
            setActive: function(active) {
                isActive = active;
                element.toggleClass(PREFIX + '-active', active);
                element.find('input').prop('checked', active);
                return api;
            },

            /**
             * Check if button is active
             */
            isActive: function() {
                return isActive;
            },

            /**
             * Update button text
             */
            setText: function(text) {
                options.text = text;
                var textEl = element.find('.' + PREFIX + '-text');
                if (textEl.length) {
                    textEl.text(text);
                }
                return api;
            },

            /**
             * Update button icon
             */
            setIcon: function(icon) {
                if (options.icon) {
                    element.removeClass(PREFIX + '-icon-' + options.icon);
                }
                options.icon = icon;
                element.addClass(PREFIX + '-icon-' + icon);
                return api;
            },

            /**
             * Get the DOM element
             */
            getElement: function() {
                return element;
            },

            /**
             * Get button ID
             */
            getId: function() {
                return id;
            },

            /**
             * Remove the button
             */
            remove: function() {
                element.remove();
            },

            /**
             * Trigger click programmatically
             */
            click: function() {
                element.find('.' + PREFIX + '-label, a').first().trigger('click');
                return api;
            }
        };

        return api;
    }

    /**
     * Build button content HTML based on layout
     */
    function buildContent(options) {
        var html = '';

        switch (options.layout) {
            case 'icon':
                if (options.icon) {
                    html = '<span class="' + PREFIX + '-icon ' + PREFIX + '-icon-' + options.icon + '"></span>';
                }
                html += '<span class="' + PREFIX + '-text sr-only">' + escapeHtml(options.text) + '</span>';
                break;

            case 'text':
                html = '<span class="' + PREFIX + '-text">' + escapeHtml(options.text) + '</span>';
                break;

            case 'shortcut':
                if (options.icon) {
                    html = '<span class="' + PREFIX + '-icon ' + PREFIX + '-icon-' + options.icon + '"></span>';
                }
                html += '<span class="' + PREFIX + '-text">' + escapeHtml(options.text) + '</span>';
                if (options.shortcut) {
                    html += '<span class="' + PREFIX + '-shortcut">' + escapeHtml(options.shortcut) + '</span>';
                }
                break;

            case 'icon-text':
            default:
                if (options.icon) {
                    html = '<span class="' + PREFIX + '-icon ' + PREFIX + '-icon-' + options.icon + '"></span>';
                }
                html += '<span class="' + PREFIX + '-text">' + escapeHtml(options.text) + '</span>';
                break;
        }

        return html;
    }

    /**
     * Escape HTML to prevent XSS
     */
    function escapeHtml(text) {
        if (!text) return '';
        return $('<div>').text(text).html();
    }

    /**
     * Create a button group
     * @param {Array} buttons - Array of button options
     * @returns {Object} Button group API
     */
    function createGroup(buttons) {
        var group = $('<div class="' + PREFIX + '-group"></div>');
        var buttonApis = [];

        buttons.forEach(function(btnOptions) {
            var btn = create(btnOptions);
            btn.appendTo(group);
            buttonApis.push(btn);
        });

        return {
            element: group,
            buttons: buttonApis,

            appendTo: function(container) {
                group.appendTo(container);
                return this;
            },

            disableAll: function() {
                buttonApis.forEach(function(btn) {
                    btn.disable();
                });
                return this;
            },

            enableAll: function() {
                buttonApis.forEach(function(btn) {
                    btn.enable();
                });
                return this;
            },

            getButton: function(index) {
                return buttonApis[index];
            }
        };
    }

    /**
     * Initialize existing buttons in a container to use the new system
     * This provides backwards compatibility with existing HTML
     */
    function initExisting(container) {
        var $container = $(container);

        // Convert existing anchor buttons
        $container.find('a').not('.' + PREFIX).each(function() {
            var $btn = $(this);
            var icon = $btn.attr('class');
            $btn.addClass(PREFIX + ' ' + PREFIX + '-standard');
            if (icon) {
                $btn.addClass(PREFIX + '-icon-' + icon);
            }
        });

        // Convert existing checkbox toggles
        $container.find('input[type="checkbox"].toggle').each(function() {
            var $input = $(this);
            var $label = $container.find('label[for="' + $input.attr('id') + '"]');
            var wrapper = $('<span class="' + PREFIX + ' ' + PREFIX + '-toggle"></span>');
            $input.addClass(PREFIX + '-input');
            $label.addClass(PREFIX + '-label');
            $input.add($label).wrapAll(wrapper);
        });

        // Convert existing checkbox dropdowns
        $container.find('input[type="checkbox"].dropdown').each(function() {
            var $input = $(this);
            var $label = $container.find('label[for="' + $input.attr('id') + '"]');
            var wrapper = $('<span class="' + PREFIX + ' ' + PREFIX + '-dropdown"></span>');
            $input.addClass(PREFIX + '-input');
            $label.addClass(PREFIX + '-label');
            $label.append('<span class="' + PREFIX + '-dropdown-arrow"></span>');
            $input.add($label).wrapAll(wrapper);
        });
    }

    // Public API
    return {
        create: create,
        createGroup: createGroup,
        initExisting: initExisting,
        PREFIX: PREFIX
    };
}());
