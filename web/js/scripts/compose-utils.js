/**
 * Magma Compose Utilities
 *
 * Enhanced compose functionality with:
 * - Email address validation
 * - Form validation with visual feedback
 * - Draft status indicator
 * - Character counters
 * - Keyboard shortcuts
 * - Plain text / HTML toggle
 *
 * Usage:
 *   var composeUtils = magma.composeUtils.init(form, composeModel);
 *   composeUtils.validateForm();
 */

var magma = magma || {};

magma.composeUtils = (function() {
    'use strict';

    // Email validation regex (RFC 5322 simplified)
    var EMAIL_REGEX = /^[a-zA-Z0-9.!#$%&'*+\/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$/;

    // Common email domain suggestions
    var COMMON_DOMAINS = [
        'gmail.com', 'yahoo.com', 'hotmail.com', 'outlook.com',
        'icloud.com', 'aol.com', 'mail.com', 'protonmail.com'
    ];

    /**
     * Validate a single email address
     * @param {string} email - Email address to validate
     * @returns {boolean} True if valid
     */
    function isValidEmail(email) {
        if (!email || typeof email !== 'string') {
            return false;
        }
        return EMAIL_REGEX.test(email.trim());
    }

    /**
     * Validate multiple email addresses (comma or semicolon separated)
     * @param {string} emails - Email addresses string
     * @returns {Object} {valid: boolean, invalid: Array, valid: Array}
     */
    function validateEmails(emails) {
        if (!emails || typeof emails !== 'string') {
            return { isValid: true, invalid: [], validEmails: [] };
        }

        var parts = emails.split(/[,;]\s*/).filter(function(e) { return e.trim(); });
        var invalid = [];
        var validEmails = [];

        parts.forEach(function(email) {
            email = email.trim();
            if (email) {
                if (isValidEmail(email)) {
                    validEmails.push(email);
                } else {
                    invalid.push(email);
                }
            }
        });

        return {
            isValid: invalid.length === 0,
            invalid: invalid,
            validEmails: validEmails
        };
    }

    /**
     * Suggest email domain corrections
     * @param {string} email - Potentially misspelled email
     * @returns {string|null} Suggested correction or null
     */
    function suggestEmailCorrection(email) {
        if (!email || email.indexOf('@') === -1) {
            return null;
        }

        var parts = email.split('@');
        var domain = parts[1].toLowerCase();

        // Check for common typos
        var typoMap = {
            'gmial.com': 'gmail.com',
            'gmal.com': 'gmail.com',
            'gamil.com': 'gmail.com',
            'gmail.co': 'gmail.com',
            'yaho.com': 'yahoo.com',
            'yahooo.com': 'yahoo.com',
            'hotmal.com': 'hotmail.com',
            'hotmial.com': 'hotmail.com',
            'outloo.com': 'outlook.com',
            'outlok.com': 'outlook.com'
        };

        if (typoMap[domain]) {
            return parts[0] + '@' + typoMap[domain];
        }

        // Check for similar domains using Levenshtein distance
        for (var i = 0; i < COMMON_DOMAINS.length; i++) {
            if (levenshteinDistance(domain, COMMON_DOMAINS[i]) <= 2 && domain !== COMMON_DOMAINS[i]) {
                return parts[0] + '@' + COMMON_DOMAINS[i];
            }
        }

        return null;
    }

    /**
     * Calculate Levenshtein distance between two strings
     */
    function levenshteinDistance(a, b) {
        if (a.length === 0) return b.length;
        if (b.length === 0) return a.length;

        var matrix = [];

        for (var i = 0; i <= b.length; i++) {
            matrix[i] = [i];
        }

        for (var j = 0; j <= a.length; j++) {
            matrix[0][j] = j;
        }

        for (i = 1; i <= b.length; i++) {
            for (j = 1; j <= a.length; j++) {
                if (b.charAt(i - 1) === a.charAt(j - 1)) {
                    matrix[i][j] = matrix[i - 1][j - 1];
                } else {
                    matrix[i][j] = Math.min(
                        matrix[i - 1][j - 1] + 1,
                        matrix[i][j - 1] + 1,
                        matrix[i - 1][j] + 1
                    );
                }
            }
        }

        return matrix[b.length][a.length];
    }

    /**
     * Create form validation for compose form
     * @param {jQuery} form - The compose form
     * @param {Object} options - Validation options
     * @returns {Object} Validation API
     */
    function createFormValidator(form, options) {
        options = $.extend({
            validateOnBlur: true,
            validateOnSubmit: true,
            showSuggestions: true,
            requiredFields: ['to'],
            maxSubjectLength: 200,
            maxRecipients: 50
        }, options);

        var errors = {};

        /**
         * Show validation error on field
         */
        function showError(field, message) {
            var $field = form.find(field);
            var $wrapper = $field.closest('.field-wrapper');

            $wrapper.addClass('has-error');

            var $error = $wrapper.find('.field-error');
            if (!$error.length) {
                $error = $('<span class="field-error">');
                $wrapper.append($error);
            }
            $error.text(message);

            errors[field] = message;
        }

        /**
         * Clear validation error on field
         */
        function clearError(field) {
            var $field = form.find(field);
            var $wrapper = $field.closest('.field-wrapper');

            $wrapper.removeClass('has-error');
            $wrapper.find('.field-error').remove();

            delete errors[field];
        }

        /**
         * Show email suggestion
         */
        function showSuggestion(field, suggestion) {
            if (!options.showSuggestions) return;

            var $field = form.find(field);
            var $wrapper = $field.closest('.field-wrapper');

            var $suggestion = $wrapper.find('.field-suggestion');
            if (!$suggestion.length) {
                $suggestion = $('<span class="field-suggestion">');
                $wrapper.append($suggestion);
            }

            $suggestion.html('Did you mean <a href="#" class="suggestion-link">' +
                $('<div>').text(suggestion).html() + '</a>?');

            $suggestion.find('.suggestion-link').on('click', function(e) {
                e.preventDefault();
                $field.val(suggestion);
                $suggestion.remove();
                validateField(field);
            });
        }

        /**
         * Clear suggestion
         */
        function clearSuggestion(field) {
            var $wrapper = form.find(field).closest('.field-wrapper');
            $wrapper.find('.field-suggestion').remove();
        }

        /**
         * Validate a single field
         */
        function validateField(field) {
            var $field = form.find(field);
            var value = $field.val();
            var fieldName = field.replace('#', '').replace(/-.*$/, '');

            clearError(field);
            clearSuggestion(field);

            // Required field check
            if (options.requiredFields.indexOf(fieldName) !== -1) {
                if (!value || !value.trim()) {
                    showError(field, 'This field is required');
                    return false;
                }
            }

            // Email field validation
            if (fieldName === 'to' || fieldName === 'cc' || fieldName === 'bcc') {
                if (value && value.trim()) {
                    var result = validateEmails(value);

                    if (!result.isValid) {
                        showError(field, 'Invalid email: ' + result.invalid[0]);

                        // Check for suggestion
                        var suggestion = suggestEmailCorrection(result.invalid[0]);
                        if (suggestion) {
                            var corrected = value.replace(result.invalid[0], suggestion);
                            showSuggestion(field, corrected);
                        }
                        return false;
                    }

                    if (result.validEmails.length > options.maxRecipients) {
                        showError(field, 'Too many recipients (max ' + options.maxRecipients + ')');
                        return false;
                    }
                }
            }

            // Subject length check
            if (fieldName === 'subject') {
                if (value && value.length > options.maxSubjectLength) {
                    showError(field, 'Subject too long (max ' + options.maxSubjectLength + ' characters)');
                    return false;
                }
            }

            return true;
        }

        /**
         * Validate entire form
         */
        function validateForm() {
            var isValid = true;

            // Validate To field
            var toField = form.find('[id^="to"]');
            if (toField.length && !validateField('#' + toField.attr('id'))) {
                isValid = false;
            }

            // Validate CC if visible
            var ccField = form.find('[id^="cc"]');
            if (ccField.is(':visible') && ccField.val() && !validateField('#' + ccField.attr('id'))) {
                isValid = false;
            }

            // Validate BCC if visible
            var bccField = form.find('[id^="bcc"]');
            if (bccField.is(':visible') && bccField.val() && !validateField('#' + bccField.attr('id'))) {
                isValid = false;
            }

            // Validate subject
            var subjectField = form.find('[id^="subject"]');
            if (subjectField.length && !validateField('#' + subjectField.attr('id'))) {
                isValid = false;
            }

            return isValid;
        }

        /**
         * Get all current errors
         */
        function getErrors() {
            return $.extend({}, errors);
        }

        /**
         * Check if form has errors
         */
        function hasErrors() {
            return Object.keys(errors).length > 0;
        }

        // Set up blur validation
        if (options.validateOnBlur) {
            form.on('blur', 'input[type="text"], input[type="email"], textarea', function() {
                var id = $(this).attr('id');
                if (id) {
                    validateField('#' + id);
                }
            });
        }

        return {
            validateField: validateField,
            validateForm: validateForm,
            showError: showError,
            clearError: clearError,
            getErrors: getErrors,
            hasErrors: hasErrors
        };
    }

    /**
     * Create draft status indicator
     * @param {jQuery} container - Container for status indicator
     * @param {Object} composeModel - The compose model
     * @returns {Object} Status indicator API
     */
    function createDraftStatus(container, composeModel) {
        var $status = $('<div class="draft-status">');
        var $text = $('<span class="draft-status-text">');
        var $icon = $('<span class="draft-status-icon">');

        $status.append($icon, $text);
        container.append($status);

        var states = {
            idle: { icon: '', text: '', className: '' },
            saving: { icon: '↻', text: 'Saving...', className: 'saving' },
            saved: { icon: '✓', text: 'Draft saved', className: 'saved' },
            error: { icon: '✗', text: 'Save failed', className: 'error' }
        };

        var currentState = 'idle';
        var hideTimer = null;

        function setState(state) {
            currentState = state;
            var config = states[state] || states.idle;

            $status.removeClass('saving saved error').addClass(config.className);
            $icon.text(config.icon);
            $text.text(config.text);

            // Clear any pending hide timer
            if (hideTimer) {
                clearTimeout(hideTimer);
                hideTimer = null;
            }

            // Auto-hide saved state after 3 seconds
            if (state === 'saved') {
                hideTimer = setTimeout(function() {
                    if (currentState === 'saved') {
                        setState('idle');
                    }
                }, 3000);
            }
        }

        // Listen to compose model events
        if (composeModel) {
            composeModel.addObserver('draftSaved', function() {
                setState('saved');
            });

            composeModel.addObserver('draftSavedError', function() {
                setState('error');
            });
        }

        return {
            element: $status,
            setState: setState,
            getState: function() {
                return currentState;
            },
            saving: function() {
                setState('saving');
            },
            saved: function() {
                setState('saved');
            },
            error: function() {
                setState('error');
            },
            hide: function() {
                setState('idle');
            }
        };
    }

    /**
     * Create character counter for input/textarea
     * @param {jQuery} field - The input or textarea
     * @param {number} maxLength - Maximum characters allowed
     * @returns {Object} Counter API
     */
    function createCharCounter(field, maxLength) {
        var $counter = $('<span class="char-counter">');
        var $wrapper = field.closest('.field-wrapper');

        if (!$wrapper.length) {
            $wrapper = field.parent();
        }

        $wrapper.append($counter);

        function update() {
            var length = field.val().length;
            var remaining = maxLength - length;

            $counter.text(length + ' / ' + maxLength);

            $counter.removeClass('warning danger');
            if (remaining <= 0) {
                $counter.addClass('danger');
            } else if (remaining <= maxLength * 0.1) {
                $counter.addClass('warning');
            }
        }

        field.on('input keyup', update);
        update();

        return {
            element: $counter,
            update: update,
            getCount: function() {
                return field.val().length;
            },
            getRemaining: function() {
                return maxLength - field.val().length;
            }
        };
    }

    /**
     * Set up keyboard shortcuts for compose
     * @param {jQuery} container - Compose container
     * @param {Object} callbacks - Shortcut callbacks
     */
    function setupKeyboardShortcuts(container, callbacks) {
        callbacks = $.extend({
            send: null,      // Ctrl+Enter
            save: null,      // Ctrl+S
            attach: null,    // Ctrl+Shift+A
            discard: null    // Escape
        }, callbacks);

        container.on('keydown', function(e) {
            var isCtrl = e.ctrlKey || e.metaKey;
            var isShift = e.shiftKey;

            // Ctrl+Enter: Send
            if (isCtrl && e.which === 13 && callbacks.send) {
                e.preventDefault();
                callbacks.send();
            }

            // Ctrl+S: Save draft
            if (isCtrl && e.which === 83 && callbacks.save) {
                e.preventDefault();
                callbacks.save();
            }

            // Ctrl+Shift+A: Attach
            if (isCtrl && isShift && e.which === 65 && callbacks.attach) {
                e.preventDefault();
                callbacks.attach();
            }

            // Escape: Discard
            if (e.which === 27 && callbacks.discard) {
                callbacks.discard();
            }
        });
    }

    /**
     * Create recipient chip input
     * @param {jQuery} input - The input field
     * @param {Object} options - Options
     * @returns {Object} Chip input API
     */
    function createRecipientChips(input, options) {
        options = $.extend({
            onAdd: null,
            onRemove: null,
            validate: true
        }, options);

        var $wrapper = $('<div class="recipient-chips">');
        var $chipList = $('<div class="chip-list">');
        var recipients = [];

        input.before($wrapper);
        $wrapper.append($chipList, input);

        input.addClass('chip-input');

        function addChip(email) {
            email = email.trim();
            if (!email) return false;

            // Validate if enabled
            if (options.validate && !isValidEmail(email)) {
                return false;
            }

            // Check for duplicates
            if (recipients.indexOf(email) !== -1) {
                return false;
            }

            recipients.push(email);

            var $chip = $('<span class="recipient-chip">')
                .attr('data-email', email);
            var $text = $('<span class="chip-text">').text(email);
            var $remove = $('<a class="chip-remove" href="#">&times;</a>');

            $chip.append($text, $remove);
            $chipList.append($chip);

            $remove.on('click', function(e) {
                e.preventDefault();
                removeChip(email);
            });

            if (options.onAdd) {
                options.onAdd(email);
            }

            updateHiddenValue();
            return true;
        }

        function removeChip(email) {
            var index = recipients.indexOf(email);
            if (index > -1) {
                recipients.splice(index, 1);
                $chipList.find('[data-email="' + email + '"]').remove();

                if (options.onRemove) {
                    options.onRemove(email);
                }

                updateHiddenValue();
            }
        }

        function updateHiddenValue() {
            input.val(recipients.join(', '));
        }

        // Handle input
        input.on('keydown', function(e) {
            var value = input.val().trim();

            // Enter or comma or semicolon: add chip
            if ((e.which === 13 || e.which === 188 || e.which === 186) && value) {
                e.preventDefault();
                // Handle multiple emails pasted
                var emails = value.split(/[,;]\s*/);
                emails.forEach(function(email) {
                    addChip(email);
                });
                input.val('');
            }

            // Backspace on empty input: remove last chip
            if (e.which === 8 && !value && recipients.length) {
                removeChip(recipients[recipients.length - 1]);
            }
        });

        // Handle paste
        input.on('paste', function(e) {
            setTimeout(function() {
                var value = input.val().trim();
                if (value) {
                    var emails = value.split(/[,;]\s*/);
                    emails.forEach(function(email) {
                        addChip(email);
                    });
                    input.val('');
                }
            }, 0);
        });

        // Handle blur
        input.on('blur', function() {
            var value = input.val().trim();
            if (value) {
                addChip(value);
                input.val('');
            }
        });

        return {
            element: $wrapper,
            addChip: addChip,
            removeChip: removeChip,
            getRecipients: function() {
                return recipients.slice();
            },
            clear: function() {
                recipients = [];
                $chipList.empty();
                updateHiddenValue();
            }
        };
    }

    /**
     * Initialize compose utilities for a form
     * @param {jQuery} form - The compose form
     * @param {Object} composeModel - The compose model
     * @param {Object} options - Configuration options
     * @returns {Object} Compose utilities API
     */
    function init(form, composeModel, options) {
        options = $.extend({
            validation: true,
            draftStatus: true,
            charCounters: true,
            shortcuts: true,
            recipientChips: false // Disable by default for backwards compatibility
        }, options);

        var validator = null;
        var draftStatus = null;
        var charCounters = {};

        // Set up validation
        if (options.validation) {
            validator = createFormValidator(form, options.validationOptions);
        }

        // Set up draft status
        if (options.draftStatus) {
            var statusContainer = form.find('.composing-header');
            if (statusContainer.length) {
                draftStatus = createDraftStatus(statusContainer, composeModel);
            }
        }

        // Set up character counters
        if (options.charCounters) {
            var subjectField = form.find('[id^="subject"]');
            if (subjectField.length) {
                charCounters.subject = createCharCounter(subjectField, 200);
            }
        }

        // Set up keyboard shortcuts
        if (options.shortcuts && options.shortcutCallbacks) {
            setupKeyboardShortcuts(form, options.shortcutCallbacks);
        }

        return {
            validator: validator,
            draftStatus: draftStatus,
            charCounters: charCounters,

            validate: function() {
                return validator ? validator.validateForm() : true;
            },

            showDraftSaving: function() {
                if (draftStatus) {
                    draftStatus.saving();
                }
            },

            showDraftSaved: function() {
                if (draftStatus) {
                    draftStatus.saved();
                }
            },

            showDraftError: function() {
                if (draftStatus) {
                    draftStatus.error();
                }
            }
        };
    }

    // Public API
    return {
        init: init,
        isValidEmail: isValidEmail,
        validateEmails: validateEmails,
        suggestEmailCorrection: suggestEmailCorrection,
        createFormValidator: createFormValidator,
        createDraftStatus: createDraftStatus,
        createCharCounter: createCharCounter,
        createRecipientChips: createRecipientChips,
        setupKeyboardShortcuts: setupKeyboardShortcuts
    };
}());
