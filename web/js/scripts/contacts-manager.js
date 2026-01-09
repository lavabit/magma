/**
 * Magma Contacts Manager
 *
 * Enhanced contact management with:
 * - CRUD operations (New, Edit, Copy, Delete)
 * - Move contacts between folders
 * - Sort functionality
 * - Edit mode (view/edit toggle)
 * - Form validation
 * - Selection management
 * - Toolbar integration
 *
 * Usage:
 *   var manager = magma.contactsManager.create({
 *       container: $('#contacts-panel'),
 *       toolsModel: toolsModel,
 *       onSave: function(contact) { ... },
 *       onDelete: function(ids) { ... }
 *   });
 */

var magma = magma || {};

magma.contactsManager = (function() {
    'use strict';

    var PREFIX = 'mgm-contact';

    /**
     * Escape HTML for safe display
     */
    function escapeHtml(text) {
        if (!text) return '';
        return $('<div>').text(text).html();
    }

    /**
     * Contact field definitions
     */
    var contactFields = {
        name: {
            label: 'Full Name',
            type: 'text',
            required: true,
            placeholder: 'Enter full name'
        },
        email: {
            label: 'Primary Email',
            type: 'email',
            required: true,
            placeholder: 'email@example.com',
            validate: function(value) {
                if (!value) return true; // Let required handle empty
                var emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
                return emailRegex.test(value) ? true : 'Invalid email address';
            }
        },
        alternateEmail1: {
            label: 'Alternate Email 1',
            type: 'email',
            placeholder: 'email@example.com'
        },
        alternateEmail2: {
            label: 'Alternate Email 2',
            type: 'email',
            placeholder: 'email@example.com'
        },
        company: {
            label: 'Company',
            type: 'text',
            placeholder: 'Company name'
        },
        title: {
            label: 'Job Title',
            type: 'text',
            placeholder: 'Job title'
        },
        phoneHome: {
            label: 'Home Phone',
            type: 'tel',
            placeholder: '(555) 123-4567'
        },
        phoneWork: {
            label: 'Work Phone',
            type: 'tel',
            placeholder: '(555) 123-4567'
        },
        phoneMobile: {
            label: 'Mobile Phone',
            type: 'tel',
            placeholder: '(555) 123-4567'
        },
        phoneFax: {
            label: 'Fax',
            type: 'tel',
            placeholder: '(555) 123-4567'
        },
        address: {
            label: 'Address',
            type: 'textarea',
            placeholder: 'Street address\nCity, State ZIP',
            rows: 3
        },
        notes: {
            label: 'Notes',
            type: 'textarea',
            placeholder: 'Additional notes',
            rows: 4
        },
        // Chat/IM fields
        chatYahoo: {
            label: 'Yahoo Messenger',
            type: 'text',
            group: 'chat'
        },
        chatLive: {
            label: 'Windows Live',
            type: 'text',
            group: 'chat'
        },
        chatAim: {
            label: 'AIM',
            type: 'text',
            group: 'chat'
        },
        chatGoogle: {
            label: 'Google Chat',
            type: 'text',
            group: 'chat'
        },
        chatIcq: {
            label: 'ICQ',
            type: 'text',
            group: 'chat'
        }
    };

    /**
     * Create a contact form field
     */
    function createFormField(fieldName, fieldDef, value) {
        var $field = $('<div class="' + PREFIX + '-field">');
        var id = PREFIX + '-' + fieldName;

        // Label
        var $label = $('<label>')
            .attr('for', id)
            .text(fieldDef.label);

        if (fieldDef.required) {
            $label.append('<span class="required">*</span>');
        }

        $field.append($label);

        // Input
        var $input;
        if (fieldDef.type === 'textarea') {
            $input = $('<textarea>')
                .attr('id', id)
                .attr('name', fieldName)
                .attr('rows', fieldDef.rows || 3)
                .attr('placeholder', fieldDef.placeholder || '')
                .text(value || '');
        } else {
            $input = $('<input>')
                .attr('type', fieldDef.type || 'text')
                .attr('id', id)
                .attr('name', fieldName)
                .attr('placeholder', fieldDef.placeholder || '')
                .val(value || '');
        }

        $field.append($input);

        // Error message container
        $field.append('<span class="' + PREFIX + '-error"></span>');

        return {
            element: $field,
            getValue: function() {
                return $input.val();
            },
            setValue: function(val) {
                if (fieldDef.type === 'textarea') {
                    $input.text(val || '');
                } else {
                    $input.val(val || '');
                }
            },
            validate: function() {
                var value = $input.val().trim();
                var $error = $field.find('.' + PREFIX + '-error');

                $field.removeClass('has-error');
                $error.text('');

                // Required check
                if (fieldDef.required && !value) {
                    $field.addClass('has-error');
                    $error.text('This field is required');
                    return false;
                }

                // Custom validation
                if (fieldDef.validate && value) {
                    var result = fieldDef.validate(value);
                    if (result !== true) {
                        $field.addClass('has-error');
                        $error.text(result);
                        return false;
                    }
                }

                return true;
            },
            focus: function() {
                $input.focus();
            }
        };
    }

    /**
     * Create contact edit form
     */
    function createEditForm(contact, options) {
        options = $.extend({
            onSave: null,
            onCancel: null,
            isNew: false
        }, options);

        var $form = $('<form class="' + PREFIX + '-form">');
        var fields = {};
        var originalValues = $.extend({}, contact);

        // Form header
        var $header = $('<div class="' + PREFIX + '-form-header">');
        $header.append('<h3>' + (options.isNew ? 'New Contact' : 'Edit Contact') + '</h3>');
        $form.append($header);

        // Form body with sections
        var $body = $('<div class="' + PREFIX + '-form-body">');

        // Basic info section
        var $basicSection = $('<div class="' + PREFIX + '-section">');
        $basicSection.append('<h4>Basic Information</h4>');

        ['name', 'email', 'alternateEmail1', 'alternateEmail2', 'company', 'title'].forEach(function(fieldName) {
            var fieldDef = contactFields[fieldName];
            var value = contact ? contact[fieldName] : '';
            var field = createFormField(fieldName, fieldDef, value);
            fields[fieldName] = field;
            $basicSection.append(field.element);
        });

        $body.append($basicSection);

        // Phone section
        var $phoneSection = $('<div class="' + PREFIX + '-section">');
        $phoneSection.append('<h4>Phone Numbers</h4>');

        ['phoneHome', 'phoneWork', 'phoneMobile', 'phoneFax'].forEach(function(fieldName) {
            var fieldDef = contactFields[fieldName];
            var value = contact ? contact[fieldName] : '';
            var field = createFormField(fieldName, fieldDef, value);
            fields[fieldName] = field;
            $phoneSection.append(field.element);
        });

        $body.append($phoneSection);

        // Address section
        var $addressSection = $('<div class="' + PREFIX + '-section">');
        $addressSection.append('<h4>Address</h4>');

        var addressField = createFormField('address', contactFields.address, contact ? contact.address : '');
        fields.address = addressField;
        $addressSection.append(addressField.element);

        $body.append($addressSection);

        // Chat section (collapsible)
        var $chatSection = $('<div class="' + PREFIX + '-section ' + PREFIX + '-section-collapsible">');
        var $chatHeader = $('<h4 class="' + PREFIX + '-section-toggle">Chat/IM <span class="toggle-icon">+</span></h4>');
        var $chatFields = $('<div class="' + PREFIX + '-section-content" style="display:none;">');

        ['chatYahoo', 'chatLive', 'chatAim', 'chatGoogle', 'chatIcq'].forEach(function(fieldName) {
            var fieldDef = contactFields[fieldName];
            var value = contact ? contact[fieldName] : '';
            var field = createFormField(fieldName, fieldDef, value);
            fields[fieldName] = field;
            $chatFields.append(field.element);
        });

        $chatSection.append($chatHeader, $chatFields);
        $body.append($chatSection);

        // Toggle chat section
        $chatHeader.on('click', function() {
            $chatFields.slideToggle(200);
            $(this).find('.toggle-icon').text($chatFields.is(':visible') ? '-' : '+');
        });

        // Notes section
        var $notesSection = $('<div class="' + PREFIX + '-section">');
        $notesSection.append('<h4>Notes</h4>');

        var notesField = createFormField('notes', contactFields.notes, contact ? contact.notes : '');
        fields.notes = notesField;
        $notesSection.append(notesField.element);

        $body.append($notesSection);
        $form.append($body);

        // Form actions
        var $actions = $('<div class="' + PREFIX + '-form-actions">');
        var $cancelBtn = $('<button type="button" class="' + PREFIX + '-btn ' + PREFIX + '-btn-cancel">Cancel</button>');
        var $saveBtn = $('<button type="submit" class="' + PREFIX + '-btn ' + PREFIX + '-btn-save">Save Contact</button>');

        $actions.append($cancelBtn, $saveBtn);
        $form.append($actions);

        // Validate all fields
        function validateForm() {
            var isValid = true;
            var firstError = null;

            for (var fieldName in fields) {
                if (!fields[fieldName].validate()) {
                    isValid = false;
                    if (!firstError) {
                        firstError = fields[fieldName];
                    }
                }
            }

            if (firstError) {
                firstError.focus();
            }

            return isValid;
        }

        // Get all form values
        function getValues() {
            var values = {};
            for (var fieldName in fields) {
                values[fieldName] = fields[fieldName].getValue();
            }
            return values;
        }

        // Check for changes
        function hasChanges() {
            var values = getValues();
            for (var key in values) {
                if (values[key] !== (originalValues[key] || '')) {
                    return true;
                }
            }
            return false;
        }

        // Event handlers
        $form.on('submit', function(e) {
            e.preventDefault();
            if (validateForm()) {
                if (options.onSave) {
                    options.onSave(getValues());
                }
            }
        });

        $cancelBtn.on('click', function() {
            if (hasChanges()) {
                if (confirm('Discard unsaved changes?')) {
                    if (options.onCancel) {
                        options.onCancel();
                    }
                }
            } else {
                if (options.onCancel) {
                    options.onCancel();
                }
            }
        });

        return {
            element: $form,
            validate: validateForm,
            getValues: getValues,
            hasChanges: hasChanges,
            getField: function(name) {
                return fields[name];
            },
            focus: function() {
                fields.name.focus();
            }
        };
    }

    /**
     * Create contact view (read-only display)
     */
    function createContactView(contact) {
        var $view = $('<div class="' + PREFIX + '-view">');

        // Header with name
        var $header = $('<div class="' + PREFIX + '-view-header">');
        $header.append('<h3 class="' + PREFIX + '-name">' + escapeHtml(contact.name) + '</h3>');

        if (contact.company) {
            $header.append('<p class="' + PREFIX + '-company">' + escapeHtml(contact.company) + '</p>');
        }
        if (contact.title) {
            $header.append('<p class="' + PREFIX + '-title">' + escapeHtml(contact.title) + '</p>');
        }

        $view.append($header);

        // Email section
        var $emailSection = $('<div class="' + PREFIX + '-view-section">');
        $emailSection.append('<h4>Email</h4>');
        var $emailList = $('<dl>');

        if (contact.email) {
            $emailList.append('<dt>Primary</dt>');
            $emailList.append('<dd><a href="mailto:' + escapeHtml(contact.email) + '">' + escapeHtml(contact.email) + '</a></dd>');
        }
        if (contact.alternateEmail1) {
            $emailList.append('<dt>Alternate 1</dt>');
            $emailList.append('<dd><a href="mailto:' + escapeHtml(contact.alternateEmail1) + '">' + escapeHtml(contact.alternateEmail1) + '</a></dd>');
        }
        if (contact.alternateEmail2) {
            $emailList.append('<dt>Alternate 2</dt>');
            $emailList.append('<dd><a href="mailto:' + escapeHtml(contact.alternateEmail2) + '">' + escapeHtml(contact.alternateEmail2) + '</a></dd>');
        }

        if ($emailList.children().length) {
            $emailSection.append($emailList);
            $view.append($emailSection);
        }

        // Phone section
        var $phoneSection = $('<div class="' + PREFIX + '-view-section">');
        $phoneSection.append('<h4>Phone</h4>');
        var $phoneList = $('<dl>');

        var phoneFields = [
            { key: 'phoneHome', label: 'Home' },
            { key: 'phoneWork', label: 'Work' },
            { key: 'phoneMobile', label: 'Mobile' },
            { key: 'phoneFax', label: 'Fax' }
        ];

        phoneFields.forEach(function(field) {
            if (contact[field.key]) {
                $phoneList.append('<dt>' + field.label + '</dt>');
                $phoneList.append('<dd>' + escapeHtml(contact[field.key]) + '</dd>');
            }
        });

        if ($phoneList.children().length) {
            $phoneSection.append($phoneList);
            $view.append($phoneSection);
        }

        // Address section
        if (contact.address) {
            var $addressSection = $('<div class="' + PREFIX + '-view-section">');
            $addressSection.append('<h4>Address</h4>');
            $addressSection.append('<p class="' + PREFIX + '-address">' + escapeHtml(contact.address).replace(/\n/g, '<br>') + '</p>');
            $view.append($addressSection);
        }

        // Chat section
        var $chatSection = $('<div class="' + PREFIX + '-view-section">');
        $chatSection.append('<h4>Chat/IM</h4>');
        var $chatList = $('<dl>');

        var chatFields = [
            { key: 'chatYahoo', label: 'Yahoo' },
            { key: 'chatLive', label: 'Windows Live' },
            { key: 'chatAim', label: 'AIM' },
            { key: 'chatGoogle', label: 'Google' },
            { key: 'chatIcq', label: 'ICQ' }
        ];

        chatFields.forEach(function(field) {
            if (contact[field.key]) {
                $chatList.append('<dt>' + field.label + '</dt>');
                $chatList.append('<dd>' + escapeHtml(contact[field.key]) + '</dd>');
            }
        });

        if ($chatList.children().length) {
            $chatSection.append($chatList);
            $view.append($chatSection);
        }

        // Notes section
        if (contact.notes) {
            var $notesSection = $('<div class="' + PREFIX + '-view-section">');
            $notesSection.append('<h4>Notes</h4>');
            $notesSection.append('<p class="' + PREFIX + '-notes">' + escapeHtml(contact.notes).replace(/\n/g, '<br>') + '</p>');
            $view.append($notesSection);
        }

        return {
            element: $view
        };
    }

    /**
     * Create move/copy dialog
     */
    function createMoveDialog(options) {
        options = $.extend({
            title: 'Move Contact',
            folders: [],
            currentFolder: null,
            onMove: null,
            onCancel: null,
            isCopy: false
        }, options);

        var $dialog = $('<div class="' + PREFIX + '-dialog">');

        var $header = $('<div class="' + PREFIX + '-dialog-header">');
        $header.append('<h3>' + escapeHtml(options.title) + '</h3>');
        $header.append('<button type="button" class="' + PREFIX + '-dialog-close">&times;</button>');
        $dialog.append($header);

        var $body = $('<div class="' + PREFIX + '-dialog-body">');
        $body.append('<p>Select destination folder:</p>');

        var $folderList = $('<ul class="' + PREFIX + '-folder-list">');

        options.folders.forEach(function(folder) {
            if (folder.id !== options.currentFolder) {
                var $item = $('<li>')
                    .attr('data-folder-id', folder.id)
                    .text(folder.name);

                if (folder.icon) {
                    $item.prepend('<span class="folder-icon">' + folder.icon + '</span>');
                }

                $folderList.append($item);
            }
        });

        $body.append($folderList);
        $dialog.append($body);

        var $actions = $('<div class="' + PREFIX + '-dialog-actions">');
        var $cancelBtn = $('<button type="button" class="' + PREFIX + '-btn ' + PREFIX + '-btn-cancel">Cancel</button>');
        var $moveBtn = $('<button type="button" class="' + PREFIX + '-btn ' + PREFIX + '-btn-primary" disabled>' +
            (options.isCopy ? 'Copy' : 'Move') + '</button>');

        $actions.append($cancelBtn, $moveBtn);
        $dialog.append($actions);

        var selectedFolder = null;

        // Folder selection
        $folderList.on('click', 'li', function() {
            $folderList.find('li').removeClass('selected');
            $(this).addClass('selected');
            selectedFolder = $(this).attr('data-folder-id');
            $moveBtn.prop('disabled', false);
        });

        // Actions
        $moveBtn.on('click', function() {
            if (selectedFolder && options.onMove) {
                options.onMove(selectedFolder);
            }
        });

        $cancelBtn.on('click', function() {
            if (options.onCancel) {
                options.onCancel();
            }
        });

        $dialog.find('.' + PREFIX + '-dialog-close').on('click', function() {
            if (options.onCancel) {
                options.onCancel();
            }
        });

        return {
            element: $dialog,
            getSelectedFolder: function() {
                return selectedFolder;
            }
        };
    }

    /**
     * Create selection manager for contact list
     */
    function createSelectionManager(options) {
        options = $.extend({
            container: null,
            onSelectionChange: null
        }, options);

        var $container = $(options.container);
        var selectedIds = [];

        // Update selection state
        function updateSelection() {
            if (options.onSelectionChange) {
                options.onSelectionChange(selectedIds.slice());
            }
        }

        // Get selected IDs
        function getSelected() {
            return selectedIds.slice();
        }

        // Select by ID
        function select(id) {
            if (selectedIds.indexOf(id) === -1) {
                selectedIds.push(id);
                $container.find('[data-contact-id="' + id + '"]').addClass('selected');
                updateSelection();
            }
        }

        // Deselect by ID
        function deselect(id) {
            var index = selectedIds.indexOf(id);
            if (index !== -1) {
                selectedIds.splice(index, 1);
                $container.find('[data-contact-id="' + id + '"]').removeClass('selected');
                updateSelection();
            }
        }

        // Toggle selection
        function toggle(id) {
            if (selectedIds.indexOf(id) === -1) {
                select(id);
            } else {
                deselect(id);
            }
        }

        // Select all
        function selectAll() {
            selectedIds = [];
            $container.find('[data-contact-id]').each(function() {
                var id = $(this).attr('data-contact-id');
                selectedIds.push(id);
                $(this).addClass('selected');
            });
            updateSelection();
        }

        // Deselect all
        function deselectAll() {
            selectedIds = [];
            $container.find('[data-contact-id]').removeClass('selected');
            updateSelection();
        }

        // Invert selection
        function invertSelection() {
            $container.find('[data-contact-id]').each(function() {
                toggle($(this).attr('data-contact-id'));
            });
        }

        return {
            getSelected: getSelected,
            select: select,
            deselect: deselect,
            toggle: toggle,
            selectAll: selectAll,
            deselectAll: deselectAll,
            invertSelection: invertSelection,
            hasSelection: function() {
                return selectedIds.length > 0;
            },
            count: function() {
                return selectedIds.length;
            }
        };
    }

    /**
     * Create sort controls
     */
    function createSortControls(options) {
        options = $.extend({
            fields: [
                { key: 'name', label: 'Name' },
                { key: 'email', label: 'Email' },
                { key: 'company', label: 'Company' }
            ],
            currentField: 'name',
            currentDirection: 'asc',
            onSort: null
        }, options);

        var $controls = $('<div class="' + PREFIX + '-sort-controls">');
        $controls.append('<span class="sort-label">Sort by:</span>');

        var $select = $('<select class="' + PREFIX + '-sort-field">');
        options.fields.forEach(function(field) {
            var $option = $('<option>')
                .val(field.key)
                .text(field.label);

            if (field.key === options.currentField) {
                $option.prop('selected', true);
            }

            $select.append($option);
        });

        var $directionBtn = $('<button type="button" class="' + PREFIX + '-sort-direction">')
            .attr('data-direction', options.currentDirection)
            .html(options.currentDirection === 'asc' ? '&#9650;' : '&#9660;')
            .attr('title', options.currentDirection === 'asc' ? 'Ascending' : 'Descending');

        $controls.append($select, $directionBtn);

        // Sort change handler
        function triggerSort() {
            if (options.onSort) {
                options.onSort($select.val(), $directionBtn.attr('data-direction'));
            }
        }

        $select.on('change', triggerSort);

        $directionBtn.on('click', function() {
            var newDirection = $(this).attr('data-direction') === 'asc' ? 'desc' : 'asc';
            $(this)
                .attr('data-direction', newDirection)
                .html(newDirection === 'asc' ? '&#9650;' : '&#9660;')
                .attr('title', newDirection === 'asc' ? 'Ascending' : 'Descending');
            triggerSort();
        });

        return {
            element: $controls,
            getCurrentSort: function() {
                return {
                    field: $select.val(),
                    direction: $directionBtn.attr('data-direction')
                };
            },
            setSort: function(field, direction) {
                $select.val(field);
                $directionBtn
                    .attr('data-direction', direction)
                    .html(direction === 'asc' ? '&#9650;' : '&#9660;');
            }
        };
    }

    /**
     * Create delete confirmation
     */
    function confirmDelete(count, onConfirm, onCancel) {
        var message = count === 1
            ? 'Are you sure you want to delete this contact?'
            : 'Are you sure you want to delete ' + count + ' contacts?';

        if (magma.dialog && magma.dialog.confirm) {
            magma.dialog.confirm(message, onConfirm, onCancel);
        } else if (confirm(message)) {
            if (onConfirm) onConfirm();
        } else {
            if (onCancel) onCancel();
        }
    }

    /**
     * Create contacts manager
     */
    function create(options) {
        options = $.extend({
            container: null,
            toolsModel: null,
            folders: [],
            currentFolder: null,
            onNew: null,
            onEdit: null,
            onSave: null,
            onDelete: null,
            onMove: null,
            onCopy: null,
            onSort: null
        }, options);

        var $container = $(options.container);
        var $panel = $('<div class="' + PREFIX + '-panel">');
        var editMode = false;
        var currentForm = null;
        var currentContact = null;
        var selection = null;

        // Initialize selection manager
        if ($container.length) {
            selection = createSelectionManager({
                container: $container,
                onSelectionChange: function(ids) {
                    updateToolbarState(ids);
                }
            });
        }

        // Update toolbar based on selection
        function updateToolbarState(selectedIds) {
            if (!options.toolsModel) return;

            var count = selectedIds ? selectedIds.length : 0;

            if (count === 0) {
                options.toolsModel.disableTool('edit');
                options.toolsModel.disableTool('delete');
                options.toolsModel.disableTool('move');
                options.toolsModel.disableTool('copy');
            } else if (count === 1) {
                options.toolsModel.enableTool('edit');
                options.toolsModel.enableTool('delete');
                options.toolsModel.enableTool('move');
                options.toolsModel.enableTool('copy');
            } else {
                options.toolsModel.disableTool('edit'); // Can't edit multiple
                options.toolsModel.enableTool('delete');
                options.toolsModel.enableTool('move');
                options.toolsModel.enableTool('copy');
            }
        }

        // Enter edit mode
        function enterEditMode(contact, isNew) {
            editMode = true;
            currentContact = contact;

            currentForm = createEditForm(contact, {
                isNew: isNew,
                onSave: function(values) {
                    if (options.onSave) {
                        options.onSave(values, isNew);
                    }
                    exitEditMode();
                },
                onCancel: function() {
                    exitEditMode();
                }
            });

            $panel.empty().append(currentForm.element);
            $container.append($panel);
            currentForm.focus();

            // Update toolbar
            if (options.toolsModel) {
                options.toolsModel.setMode('edit');
            }
        }

        // Exit edit mode
        function exitEditMode() {
            editMode = false;
            currentForm = null;
            currentContact = null;
            $panel.empty().detach();

            // Update toolbar
            if (options.toolsModel) {
                options.toolsModel.setMode('default');
            }
        }

        // Show contact view
        function showContactView(contact) {
            var view = createContactView(contact);
            $panel.empty().append(view.element);

            if (!$panel.parent().length) {
                $container.append($panel);
            }
        }

        // Show move dialog
        function showMoveDialog(contactIds, isCopy) {
            var dialog = createMoveDialog({
                title: isCopy ? 'Copy Contact(s)' : 'Move Contact(s)',
                folders: options.folders,
                currentFolder: options.currentFolder,
                isCopy: isCopy,
                onMove: function(folderId) {
                    if (isCopy && options.onCopy) {
                        options.onCopy(contactIds, folderId);
                    } else if (!isCopy && options.onMove) {
                        options.onMove(contactIds, folderId);
                    }
                    hideDialog();
                },
                onCancel: function() {
                    hideDialog();
                }
            });

            // Create overlay
            var $overlay = $('<div class="' + PREFIX + '-overlay">');
            $overlay.append(dialog.element);
            $('body').append($overlay);

            // Close on overlay click
            $overlay.on('click', function(e) {
                if (e.target === this) {
                    hideDialog();
                }
            });
        }

        // Hide dialog
        function hideDialog() {
            $('.' + PREFIX + '-overlay').remove();
        }

        // Public API
        return {
            element: $panel,

            // Create new contact
            newContact: function() {
                enterEditMode({}, true);
                if (options.onNew) {
                    options.onNew();
                }
            },

            // Edit contact
            editContact: function(contact) {
                enterEditMode(contact, false);
                if (options.onEdit) {
                    options.onEdit(contact);
                }
            },

            // View contact (read-only)
            viewContact: function(contact) {
                showContactView(contact);
            },

            // Delete contacts
            deleteContacts: function(contactIds) {
                if (!contactIds || contactIds.length === 0) {
                    contactIds = selection ? selection.getSelected() : [];
                }

                if (contactIds.length === 0) return;

                confirmDelete(contactIds.length, function() {
                    if (options.onDelete) {
                        options.onDelete(contactIds);
                    }
                    if (selection) {
                        selection.deselectAll();
                    }
                });
            },

            // Move contacts
            moveContacts: function(contactIds) {
                if (!contactIds || contactIds.length === 0) {
                    contactIds = selection ? selection.getSelected() : [];
                }

                if (contactIds.length === 0) return;

                showMoveDialog(contactIds, false);
            },

            // Copy contacts
            copyContacts: function(contactIds) {
                if (!contactIds || contactIds.length === 0) {
                    contactIds = selection ? selection.getSelected() : [];
                }

                if (contactIds.length === 0) return;

                showMoveDialog(contactIds, true);
            },

            // Get selection manager
            getSelection: function() {
                return selection;
            },

            // Check if in edit mode
            isEditMode: function() {
                return editMode;
            },

            // Exit edit mode
            cancelEdit: function() {
                if (editMode && currentForm) {
                    if (currentForm.hasChanges()) {
                        if (confirm('Discard unsaved changes?')) {
                            exitEditMode();
                        }
                    } else {
                        exitEditMode();
                    }
                }
            },

            // Set folders for move/copy
            setFolders: function(folders) {
                options.folders = folders;
            },

            // Set current folder
            setCurrentFolder: function(folderId) {
                options.currentFolder = folderId;
            },

            // Create sort controls
            createSortControls: function(sortOptions) {
                return createSortControls($.extend({
                    onSort: options.onSort
                }, sortOptions));
            },

            // Destroy
            destroy: function() {
                hideDialog();
                $panel.remove();
            }
        };
    }

    // Public API
    return {
        create: create,
        createEditForm: createEditForm,
        createContactView: createContactView,
        createMoveDialog: createMoveDialog,
        createSelectionManager: createSelectionManager,
        createSortControls: createSortControls,
        confirmDelete: confirmDelete,
        contactFields: contactFields,
        PREFIX: PREFIX
    };
}());
