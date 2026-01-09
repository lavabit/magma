/**
 * Magma Settings Manager
 *
 * Comprehensive settings/options management with:
 * - Multiple input types (toggle, dropdown, text, textarea, checkbox)
 * - Info tooltips
 * - Default value handling
 * - Change tracking
 * - Save/Cancel functionality
 * - Validation
 *
 * Usage:
 *   var settings = magma.settingsManager.create({
 *       container: $('#settings-panel'),
 *       fields: [...],
 *       onSave: function(values) { ... },
 *       onCancel: function() { ... }
 *   });
 */

var magma = magma || {};

magma.settingsManager = (function() {
    'use strict';

    var PREFIX = 'mgm-setting';
    var idCounter = 0;

    /**
     * Generate unique ID
     */
    function generateId() {
        return PREFIX + '-' + (++idCounter);
    }

    /**
     * Escape HTML for safe display
     */
    function escapeHtml(text) {
        if (!text) return '';
        return $('<div>').text(text).html();
    }

    /**
     * Field type definitions
     */
    var fieldTypes = {
        /**
         * Toggle switch (boolean)
         */
        toggle: {
            render: function(field, value) {
                var id = field.id || generateId();
                var checked = value ? 'checked' : '';

                return '<div class="' + PREFIX + '-toggle">' +
                    '<input type="checkbox" id="' + id + '" ' + checked + '>' +
                    '<label for="' + id + '" class="toggle-switch">' +
                    '<span class="toggle-track"></span>' +
                    '<span class="toggle-thumb"></span>' +
                    '</label>' +
                    '</div>';
            },
            getValue: function($el) {
                return $el.find('input[type="checkbox"]').is(':checked');
            },
            setValue: function($el, value) {
                $el.find('input[type="checkbox"]').prop('checked', !!value);
            }
        },

        /**
         * Checkbox (boolean)
         */
        checkbox: {
            render: function(field, value) {
                var id = field.id || generateId();
                var checked = value ? 'checked' : '';

                return '<div class="' + PREFIX + '-checkbox">' +
                    '<input type="checkbox" id="' + id + '" ' + checked + '>' +
                    '<label for="' + id + '">' + escapeHtml(field.checkboxLabel || '') + '</label>' +
                    '</div>';
            },
            getValue: function($el) {
                return $el.find('input[type="checkbox"]').is(':checked');
            },
            setValue: function($el, value) {
                $el.find('input[type="checkbox"]').prop('checked', !!value);
            }
        },

        /**
         * Dropdown select
         */
        dropdown: {
            render: function(field, value) {
                var id = field.id || generateId();
                var options = '';

                (field.options || []).forEach(function(opt) {
                    var optValue = typeof opt === 'object' ? opt.value : opt;
                    var optLabel = typeof opt === 'object' ? opt.label : opt;
                    var selected = optValue === value ? 'selected' : '';
                    options += '<option value="' + escapeHtml(optValue) + '" ' + selected + '>' +
                        escapeHtml(optLabel) + '</option>';
                });

                return '<div class="' + PREFIX + '-dropdown">' +
                    '<select id="' + id + '">' + options + '</select>' +
                    '</div>';
            },
            getValue: function($el) {
                return $el.find('select').val();
            },
            setValue: function($el, value) {
                $el.find('select').val(value);
            }
        },

        /**
         * Text input
         */
        text: {
            render: function(field, value) {
                var id = field.id || generateId();
                var placeholder = field.placeholder ? 'placeholder="' + escapeHtml(field.placeholder) + '"' : '';
                var maxlength = field.maxLength ? 'maxlength="' + field.maxLength + '"' : '';

                return '<div class="' + PREFIX + '-text">' +
                    '<input type="text" id="' + id + '" value="' + escapeHtml(value || '') + '" ' +
                    placeholder + ' ' + maxlength + '>' +
                    '</div>';
            },
            getValue: function($el) {
                return $el.find('input').val();
            },
            setValue: function($el, value) {
                $el.find('input').val(value || '');
            }
        },

        /**
         * Number input
         */
        number: {
            render: function(field, value) {
                var id = field.id || generateId();
                var min = typeof field.min === 'number' ? 'min="' + field.min + '"' : '';
                var max = typeof field.max === 'number' ? 'max="' + field.max + '"' : '';
                var step = field.step ? 'step="' + field.step + '"' : '';

                return '<div class="' + PREFIX + '-number">' +
                    '<input type="number" id="' + id + '" value="' + (value || '') + '" ' +
                    min + ' ' + max + ' ' + step + '>' +
                    '</div>';
            },
            getValue: function($el) {
                var val = $el.find('input').val();
                return val ? parseFloat(val) : null;
            },
            setValue: function($el, value) {
                $el.find('input').val(value !== null && value !== undefined ? value : '');
            }
        },

        /**
         * Textarea
         */
        textarea: {
            render: function(field, value) {
                var id = field.id || generateId();
                var placeholder = field.placeholder ? 'placeholder="' + escapeHtml(field.placeholder) + '"' : '';
                var rows = field.rows || 4;

                return '<div class="' + PREFIX + '-textarea">' +
                    '<textarea id="' + id + '" rows="' + rows + '" ' + placeholder + '>' +
                    escapeHtml(value || '') + '</textarea>' +
                    '</div>';
            },
            getValue: function($el) {
                return $el.find('textarea').val();
            },
            setValue: function($el, value) {
                $el.find('textarea').val(value || '');
            }
        },

        /**
         * Email input
         */
        email: {
            render: function(field, value) {
                var id = field.id || generateId();
                var placeholder = field.placeholder ? 'placeholder="' + escapeHtml(field.placeholder) + '"' : '';

                return '<div class="' + PREFIX + '-email">' +
                    '<input type="email" id="' + id + '" value="' + escapeHtml(value || '') + '" ' +
                    placeholder + '>' +
                    '</div>';
            },
            getValue: function($el) {
                return $el.find('input').val();
            },
            setValue: function($el, value) {
                $el.find('input').val(value || '');
            }
        },

        /**
         * Password input
         */
        password: {
            render: function(field, value) {
                var id = field.id || generateId();
                var placeholder = field.placeholder ? 'placeholder="' + escapeHtml(field.placeholder) + '"' : '';

                return '<div class="' + PREFIX + '-password">' +
                    '<input type="password" id="' + id + '" value="' + escapeHtml(value || '') + '" ' +
                    placeholder + '>' +
                    (field.showToggle ? '<button type="button" class="password-toggle" title="Show/Hide">👁</button>' : '') +
                    '</div>';
            },
            getValue: function($el) {
                return $el.find('input').val();
            },
            setValue: function($el, value) {
                $el.find('input').val(value || '');
            }
        },

        /**
         * Radio button group
         */
        radio: {
            render: function(field, value) {
                var name = field.name || generateId();
                var options = '';

                (field.options || []).forEach(function(opt, i) {
                    var optValue = typeof opt === 'object' ? opt.value : opt;
                    var optLabel = typeof opt === 'object' ? opt.label : opt;
                    var checked = optValue === value ? 'checked' : '';
                    var id = name + '-' + i;

                    options += '<div class="radio-option">' +
                        '<input type="radio" name="' + name + '" id="' + id + '" value="' +
                        escapeHtml(optValue) + '" ' + checked + '>' +
                        '<label for="' + id + '">' + escapeHtml(optLabel) + '</label>' +
                        '</div>';
                });

                return '<div class="' + PREFIX + '-radio">' + options + '</div>';
            },
            getValue: function($el) {
                return $el.find('input:checked').val();
            },
            setValue: function($el, value) {
                $el.find('input[value="' + value + '"]').prop('checked', true);
            }
        },

        /**
         * Color picker
         */
        color: {
            render: function(field, value) {
                var id = field.id || generateId();

                return '<div class="' + PREFIX + '-color">' +
                    '<input type="color" id="' + id + '" value="' + (value || '#000000') + '">' +
                    '<span class="color-preview" style="background-color: ' + (value || '#000000') + '"></span>' +
                    '</div>';
            },
            getValue: function($el) {
                return $el.find('input').val();
            },
            setValue: function($el, value) {
                $el.find('input').val(value || '#000000');
                $el.find('.color-preview').css('background-color', value || '#000000');
            }
        }
    };

    /**
     * Create info tooltip
     */
    function createInfoTooltip(text) {
        return '<span class="' + PREFIX + '-info" title="' + escapeHtml(text) + '">' +
            '<span class="info-icon">ⓘ</span>' +
            '</span>';
    }

    /**
     * Create a single setting field
     */
    function createField(field, value) {
        var type = fieldTypes[field.type] || fieldTypes.text;
        var id = field.id || generateId();
        var hasChanged = false;

        var $wrapper = $('<div class="' + PREFIX + '-field">')
            .attr('data-field-name', field.name)
            .attr('data-field-type', field.type);

        // Label row
        var $label = $('<div class="' + PREFIX + '-label">');
        if (field.label) {
            $label.append('<label for="' + id + '">' + escapeHtml(field.label) + '</label>');
        }
        if (field.info) {
            $label.append(createInfoTooltip(field.info));
        }
        if (field.required) {
            $label.append('<span class="required-marker">*</span>');
        }

        // Input row
        var $input = $('<div class="' + PREFIX + '-input">');
        $input.html(type.render(field, value));

        // Default value indicator
        if (field.defaultValue !== undefined && value !== field.defaultValue) {
            $wrapper.addClass('has-custom-value');
        }

        // Description
        if (field.description) {
            $input.append('<p class="' + PREFIX + '-description">' + escapeHtml(field.description) + '</p>');
        }

        // Error container
        $input.append('<span class="' + PREFIX + '-error"></span>');

        $wrapper.append($label, $input);

        // Track changes
        $wrapper.on('change input', 'input, select, textarea', function() {
            var currentValue = type.getValue($wrapper);
            var isDefault = currentValue === field.defaultValue;

            $wrapper.toggleClass('has-custom-value', !isDefault);

            if (!hasChanged) {
                hasChanged = true;
                $wrapper.addClass('has-changes');
            }
        });

        // Password toggle
        if (field.type === 'password' && field.showToggle) {
            $wrapper.on('click', '.password-toggle', function() {
                var $pwInput = $wrapper.find('input');
                var type = $pwInput.attr('type') === 'password' ? 'text' : 'password';
                $pwInput.attr('type', type);
            });
        }

        // Color preview update
        if (field.type === 'color') {
            $wrapper.on('input', 'input', function() {
                $wrapper.find('.color-preview').css('background-color', $(this).val());
            });
        }

        return {
            element: $wrapper,
            field: field,

            getValue: function() {
                return type.getValue($wrapper);
            },

            setValue: function(val) {
                type.setValue($wrapper, val);
                hasChanged = false;
                $wrapper.removeClass('has-changes');
            },

            reset: function() {
                this.setValue(field.defaultValue);
            },

            hasChanges: function() {
                return hasChanged;
            },

            clearChanges: function() {
                hasChanged = false;
                $wrapper.removeClass('has-changes');
            },

            validate: function() {
                var value = this.getValue();
                var $error = $wrapper.find('.' + PREFIX + '-error');

                $wrapper.removeClass('has-error');
                $error.text('');

                // Required validation
                if (field.required && (value === '' || value === null || value === undefined)) {
                    $wrapper.addClass('has-error');
                    $error.text('This field is required');
                    return false;
                }

                // Custom validation
                if (field.validate && typeof field.validate === 'function') {
                    var result = field.validate(value);
                    if (result !== true) {
                        $wrapper.addClass('has-error');
                        $error.text(result || 'Invalid value');
                        return false;
                    }
                }

                return true;
            },

            showError: function(message) {
                $wrapper.addClass('has-error');
                $wrapper.find('.' + PREFIX + '-error').text(message);
            },

            clearError: function() {
                $wrapper.removeClass('has-error');
                $wrapper.find('.' + PREFIX + '-error').text('');
            }
        };
    }

    /**
     * Create a settings section
     */
    function createSection(section) {
        var $section = $('<div class="' + PREFIX + '-section">');

        if (section.title) {
            $section.append('<h3 class="' + PREFIX + '-section-title">' + escapeHtml(section.title) + '</h3>');
        }

        if (section.description) {
            $section.append('<p class="' + PREFIX + '-section-description">' + escapeHtml(section.description) + '</p>');
        }

        var $fields = $('<div class="' + PREFIX + '-section-fields">');
        $section.append($fields);

        return {
            element: $section,
            fieldsContainer: $fields
        };
    }

    /**
     * Create action buttons (Save/Cancel)
     */
    function createActions(options) {
        var $actions = $('<div class="' + PREFIX + '-actions">');

        if (options.showCancel !== false) {
            var $cancel = $('<button type="button" class="' + PREFIX + '-btn ' + PREFIX + '-btn-cancel">')
                .text(options.cancelText || 'Cancel');
            $actions.append($cancel);
        }

        if (options.showSave !== false) {
            var $save = $('<button type="button" class="' + PREFIX + '-btn ' + PREFIX + '-btn-save">')
                .text(options.saveText || 'Save Changes');
            $actions.append($save);
        }

        if (options.showReset) {
            var $reset = $('<button type="button" class="' + PREFIX + '-btn ' + PREFIX + '-btn-reset">')
                .text(options.resetText || 'Reset to Defaults');
            $actions.prepend($reset);
        }

        return $actions;
    }

    /**
     * Create settings panel
     */
    function create(options) {
        options = $.extend({
            container: null,
            fields: [],
            sections: null,
            values: {},
            showActions: true,
            showCancel: true,
            showSave: true,
            showReset: false,
            onSave: null,
            onCancel: null,
            onReset: null,
            onChange: null
        }, options);

        var $container = $(options.container);
        var $panel = $('<div class="' + PREFIX + '-panel">');
        var fieldInstances = {};
        var originalValues = $.extend({}, options.values);

        /**
         * Render fields
         */
        function renderFields(fields, container) {
            fields.forEach(function(field) {
                var value = options.values[field.name];
                if (value === undefined) {
                    value = field.defaultValue;
                }

                var fieldInstance = createField(field, value);
                fieldInstances[field.name] = fieldInstance;
                container.append(fieldInstance.element);

                // Track changes
                fieldInstance.element.on('change input', function() {
                    if (options.onChange) {
                        options.onChange(field.name, fieldInstance.getValue(), getValues());
                    }
                    updateActionState();
                });
            });
        }

        /**
         * Render with sections
         */
        if (options.sections) {
            options.sections.forEach(function(sectionConfig) {
                var section = createSection(sectionConfig);
                $panel.append(section.element);
                renderFields(sectionConfig.fields || [], section.fieldsContainer);
            });
        } else {
            var $fields = $('<div class="' + PREFIX + '-fields">');
            $panel.append($fields);
            renderFields(options.fields, $fields);
        }

        /**
         * Add action buttons
         */
        var $actions = null;
        if (options.showActions) {
            $actions = createActions(options);
            $panel.append($actions);

            // Save handler
            $actions.on('click', '.' + PREFIX + '-btn-save', function() {
                if (validate()) {
                    var values = getValues();
                    if (options.onSave) {
                        options.onSave(values);
                    }
                    originalValues = $.extend({}, values);
                    clearAllChanges();
                    updateActionState();
                }
            });

            // Cancel handler
            $actions.on('click', '.' + PREFIX + '-btn-cancel', function() {
                setValues(originalValues);
                clearAllChanges();
                if (options.onCancel) {
                    options.onCancel();
                }
                updateActionState();
            });

            // Reset handler
            $actions.on('click', '.' + PREFIX + '-btn-reset', function() {
                resetToDefaults();
                if (options.onReset) {
                    options.onReset();
                }
            });
        }

        /**
         * Get all current values
         */
        function getValues() {
            var values = {};
            for (var name in fieldInstances) {
                values[name] = fieldInstances[name].getValue();
            }
            return values;
        }

        /**
         * Set values
         */
        function setValues(values) {
            for (var name in values) {
                if (fieldInstances[name]) {
                    fieldInstances[name].setValue(values[name]);
                }
            }
        }

        /**
         * Reset to defaults
         */
        function resetToDefaults() {
            for (var name in fieldInstances) {
                fieldInstances[name].reset();
            }
            updateActionState();
        }

        /**
         * Validate all fields
         */
        function validate() {
            var isValid = true;
            for (var name in fieldInstances) {
                if (!fieldInstances[name].validate()) {
                    isValid = false;
                }
            }
            return isValid;
        }

        /**
         * Check if any field has changes
         */
        function hasChanges() {
            for (var name in fieldInstances) {
                if (fieldInstances[name].hasChanges()) {
                    return true;
                }
            }
            return false;
        }

        /**
         * Clear all change markers
         */
        function clearAllChanges() {
            for (var name in fieldInstances) {
                fieldInstances[name].clearChanges();
            }
        }

        /**
         * Update action button state
         */
        function updateActionState() {
            if ($actions) {
                var changed = hasChanges();
                $actions.find('.' + PREFIX + '-btn-save').prop('disabled', !changed);
                $actions.find('.' + PREFIX + '-btn-cancel').prop('disabled', !changed);
                $panel.toggleClass('has-unsaved-changes', changed);
            }
        }

        // Initial state
        $container.append($panel);
        updateActionState();

        // Initialize info tooltips
        $panel.find('.' + PREFIX + '-info').each(function() {
            var $info = $(this);
            var text = $info.attr('title');

            $info.removeAttr('title').on('click', function(e) {
                e.preventDefault();

                // Remove any existing tooltips
                $('.' + PREFIX + '-tooltip').remove();

                var $tooltip = $('<div class="' + PREFIX + '-tooltip">')
                    .text(text)
                    .appendTo('body');

                var offset = $info.offset();
                $tooltip.css({
                    top: offset.top + $info.outerHeight() + 5,
                    left: offset.left
                });

                // Close on click outside
                $(document).one('click', function() {
                    $tooltip.remove();
                });
            });
        });

        // Public API
        return {
            element: $panel,

            getValue: function(name) {
                return fieldInstances[name] ? fieldInstances[name].getValue() : undefined;
            },

            setValue: function(name, value) {
                if (fieldInstances[name]) {
                    fieldInstances[name].setValue(value);
                }
            },

            getValues: getValues,
            setValues: setValues,
            validate: validate,
            hasChanges: hasChanges,
            resetToDefaults: resetToDefaults,

            getField: function(name) {
                return fieldInstances[name];
            },

            showFieldError: function(name, message) {
                if (fieldInstances[name]) {
                    fieldInstances[name].showError(message);
                }
            },

            clearFieldError: function(name) {
                if (fieldInstances[name]) {
                    fieldInstances[name].clearError();
                }
            },

            destroy: function() {
                $panel.remove();
            }
        };
    }

    // Public API
    return {
        create: create,
        createField: createField,
        createSection: createSection,
        fieldTypes: fieldTypes,
        PREFIX: PREFIX
    };
}());
