/**
 * Magma Folder Manager
 *
 * Enhanced folder management with:
 * - jQuery UI sortable for reordering folders
 * - Improved edit mode with visual feedback
 * - Delete confirmation dialogs
 * - Folder order persistence
 *
 * Usage:
 *   magma.folderManager.init(folderModel, container, options);
 */

var magma = magma || {};

magma.folderManager = (function() {
    'use strict';

    var defaults = {
        sortable: true,
        confirmDelete: true,
        animationDuration: 200
    };

    /**
     * Make folder list sortable for reordering
     * @param {jQuery} list - The folder list container
     * @param {Object} folderModel - The folder model for API calls
     * @param {Object} options - Configuration options
     */
    function initSortable(list, folderModel, options) {
        if (!$.fn.sortable) {
            console.warn('folderManager: jQuery UI sortable not available');
            return;
        }

        var originalOrder = [];

        list.sortable({
            items: '> li.toggle', // Only sort custom folders (with toggle class)
            handle: '.folder', // Drag by the folder link
            axis: 'y',
            containment: 'parent',
            placeholder: 'folder-sort-placeholder',
            tolerance: 'pointer',
            distance: 5,
            opacity: 0.8,
            cursor: 'move',

            start: function(event, ui) {
                // Store original order for potential revert
                originalOrder = list.sortable('toArray');

                // Add dragging class for visual feedback
                ui.item.addClass('sorting');

                // Set placeholder height to match dragged item
                ui.placeholder.height(ui.item.outerHeight());
            },

            stop: function(event, ui) {
                ui.item.removeClass('sorting');
            },

            update: function(event, ui) {
                var newOrder = list.sortable('toArray');
                var folderID = parseInt(ui.item.attr('id').match(/\d+/), 10);
                var newIndex = newOrder.indexOf(ui.item.attr('id'));

                // Find the folder that's now before this one (if any)
                var afterFolderID = null;
                if (newIndex > 0) {
                    var prevId = newOrder[newIndex - 1];
                    afterFolderID = parseInt(prevId.match(/\d+/), 10);
                }

                // Notify model of reorder (if the model supports it)
                if (typeof folderModel.reorderFolder === 'function') {
                    folderModel.reorderFolder(folderID, afterFolderID);
                }

                // Trigger custom event for order change
                list.trigger('folderOrderChanged', {
                    folderID: folderID,
                    newIndex: newIndex,
                    order: newOrder.map(function(id) {
                        return parseInt(id.match(/\d+/), 10);
                    })
                });
            }
        });

        // Disable sortable by default, enable in edit mode
        list.sortable('disable');
    }

    /**
     * Enable/disable sortable mode
     * @param {jQuery} list - The folder list
     * @param {boolean} enabled - Whether to enable sorting
     */
    function setSortable(list, enabled) {
        if (list.data('ui-sortable')) {
            list.sortable(enabled ? 'enable' : 'disable');
        }
    }

    /**
     * Show delete confirmation dialog
     * @param {string} folderName - Name of folder to delete
     * @param {Function} onConfirm - Callback on confirmation
     * @param {Function} onCancel - Callback on cancel
     */
    function confirmDelete(folderName, onConfirm, onCancel) {
        var escapedName = $('<div>').text(folderName).html();
        var message = 'Are you sure you want to delete "' + escapedName + '"?';
        message += '\n\nThis will also delete all messages in this folder.';

        // Use the existing dialog system if available
        if (magma.dialog && magma.dialog.confirm) {
            magma.dialog.confirm(message, onConfirm, onCancel);
        } else {
            // Fallback to native confirm
            if (window.confirm(message)) {
                if (onConfirm) onConfirm();
            } else {
                if (onCancel) onCancel();
            }
        }
    }

    /**
     * Enhanced edit mode with better visual feedback
     * @param {jQuery} list - The folder list
     * @param {jQuery} container - The folder menu container
     * @param {boolean} editing - Whether edit mode is active
     * @param {Object} options - Configuration options
     */
    function setEditMode(list, container, editing, options) {
        if (editing) {
            container.addClass('folder-edit-mode');
            list.addClass('editing');

            // Enable sorting if configured
            if (options.sortable) {
                setSortable(list, true);
            }

            // Add edit instructions
            if (!container.find('.edit-instructions').length) {
                var instructions = $('<div class="edit-instructions">')
                    .text('Drag folders to reorder. Click icons to rename or delete.');
                container.find('.folder-options').after(instructions);
            }
        } else {
            container.removeClass('folder-edit-mode');
            list.removeClass('editing');

            // Disable sorting
            if (options.sortable) {
                setSortable(list, false);
            }

            // Remove instructions
            container.find('.edit-instructions').remove();
        }
    }

    /**
     * Initialize folder manager on a folder view
     * @param {Object} folderModel - The folder model
     * @param {jQuery} container - The folder menu container
     * @param {Object} userOptions - User configuration
     */
    function init(folderModel, container, userOptions) {
        var options = $.extend({}, defaults, userOptions);
        var list = container.find('.folder-list');
        var editOpen = false;

        // Initialize sortable
        if (options.sortable) {
            initSortable(list, folderModel, options);
        }

        // Enhanced delete with confirmation
        if (options.confirmDelete) {
            list.off('click.folderManagerDelete', '.remove');
            list.on('click.folderManagerDelete', '.remove', function(event) {
                event.preventDefault();
                event.stopPropagation();

                var toggle = $(this).parents('.toggle');
                var id = parseInt(toggle.attr('id').match(/\d+/), 10);
                var name = toggle.find('.folder').text();

                confirmDelete(name, function() {
                    folderModel.removeFolder(id);
                });
            });
        }

        // Track edit mode state
        var editButton = container.find('.folder-options .edit');
        if (editButton.length) {
            editButton.off('click.folderManager');
            editButton.on('click.folderManager', function() {
                editOpen = !editOpen;
                setEditMode(list, container, editOpen, options);
            });
        }

        // Observe folder removal to check if we should exit edit mode
        folderModel.addObserver('removed', function() {
            // If no custom folders remain, exit edit mode
            if (!list.find('.toggle').length && editOpen) {
                editOpen = false;
                setEditMode(list, container, false, options);
                editButton.prop('checked', false)
                    .siblings('label')
                    .removeClass('ui-state-active');
            }
        });

        return {
            /**
             * Enable or disable edit mode
             */
            setEditMode: function(enabled) {
                editOpen = enabled;
                setEditMode(list, container, enabled, options);
            },

            /**
             * Check if in edit mode
             */
            isEditMode: function() {
                return editOpen;
            },

            /**
             * Get current folder order
             */
            getOrder: function() {
                var order = [];
                list.find('> li[id^="folder-"]').each(function() {
                    order.push(parseInt($(this).attr('id').match(/\d+/), 10));
                });
                return order;
            },

            /**
             * Refresh sortable
             */
            refresh: function() {
                if (list.data('ui-sortable')) {
                    list.sortable('refresh');
                }
            }
        };
    }

    /**
     * Add folder order support to model
     * This extends the folder model with reorder capabilities
     * @param {Object} folderModel - The folder model to extend
     */
    function extendModel(folderModel) {
        if (folderModel._folderManagerExtended) {
            return folderModel;
        }

        var folderOrder = [];

        folderModel.reorderFolder = function(folderID, afterFolderID) {
            // Update local order
            var currentIndex = folderOrder.indexOf(folderID);
            if (currentIndex > -1) {
                folderOrder.splice(currentIndex, 1);
            }

            if (afterFolderID === null) {
                // Move to beginning
                folderOrder.unshift(folderID);
            } else {
                var afterIndex = folderOrder.indexOf(afterFolderID);
                if (afterIndex > -1) {
                    folderOrder.splice(afterIndex + 1, 0, folderID);
                } else {
                    folderOrder.push(folderID);
                }
            }

            // Persist to server (if API exists)
            // This would need to be implemented on the backend
            // getData('folders.reorder', {order: folderOrder}, { ... });

            return folderOrder.slice();
        };

        folderModel.getFolderOrder = function() {
            return folderOrder.slice();
        };

        folderModel.setFolderOrder = function(order) {
            folderOrder = order.slice();
        };

        // Track folder additions
        var originalAddObserver = folderModel.addObserver;
        folderModel.addObserver = function(event, callback) {
            if (event === 'added') {
                var wrappedCallback = function(data) {
                    if (data && data.folderID) {
                        folderOrder.push(data.folderID);
                    }
                    callback(data);
                };
                return originalAddObserver.call(this, event, wrappedCallback);
            }
            return originalAddObserver.call(this, event, callback);
        };

        folderModel._folderManagerExtended = true;
        return folderModel;
    }

    // Public API
    return {
        init: init,
        extendModel: extendModel,
        confirmDelete: confirmDelete,
        setSortable: setSortable,
        setEditMode: setEditMode
    };
}());
