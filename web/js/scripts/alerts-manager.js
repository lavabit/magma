/**
 * Magma Alerts Manager
 *
 * Enhanced alert/notification system with:
 * - Multiple alert types (system, warning, info, success, error)
 * - Acknowledge/dismiss functionality
 * - Notification badge in global chrome
 * - Toast notifications
 * - Persistent alerts panel
 * - Alert history
 *
 * Usage:
 *   var alerts = magma.alertsManager.create({
 *       container: $('#alert-container'),
 *       badgeContainer: $('#alert-badge'),
 *       onAcknowledge: function(alertId) { ... },
 *       onDismiss: function(alertId) { ... }
 *   });
 *
 *   alerts.show({ type: 'warning', message: 'Your session will expire soon' });
 */

var magma = magma || {};

magma.alertsManager = (function() {
    'use strict';

    var PREFIX = 'mgm-alert';
    var idCounter = 0;

    /**
     * Alert types with their default configurations
     */
    var alertTypes = {
        system: {
            icon: '⚠',
            title: 'System Alert',
            className: 'system',
            persistent: true,
            requiresAcknowledge: true
        },
        warning: {
            icon: '⚡',
            title: 'Warning',
            className: 'warning',
            persistent: true,
            requiresAcknowledge: false
        },
        error: {
            icon: '✕',
            title: 'Error',
            className: 'error',
            persistent: true,
            requiresAcknowledge: false
        },
        info: {
            icon: 'ℹ',
            title: 'Information',
            className: 'info',
            persistent: false,
            requiresAcknowledge: false
        },
        success: {
            icon: '✓',
            title: 'Success',
            className: 'success',
            persistent: false,
            requiresAcknowledge: false
        }
    };

    /**
     * Generate unique ID
     */
    function generateId() {
        return PREFIX + '-' + (++idCounter) + '-' + Date.now();
    }

    /**
     * Escape HTML for safe display
     */
    function escapeHtml(text) {
        if (!text) return '';
        return $('<div>').text(text).html();
    }

    /**
     * Format timestamp
     */
    function formatTime(date) {
        if (!date) return '';
        var d = date instanceof Date ? date : new Date(date);
        var now = new Date();
        var diff = now - d;

        // Less than a minute
        if (diff < 60000) {
            return 'Just now';
        }
        // Less than an hour
        if (diff < 3600000) {
            var mins = Math.floor(diff / 60000);
            return mins + ' minute' + (mins === 1 ? '' : 's') + ' ago';
        }
        // Less than a day
        if (diff < 86400000) {
            var hours = Math.floor(diff / 3600000);
            return hours + ' hour' + (hours === 1 ? '' : 's') + ' ago';
        }
        // Format as date
        return d.toLocaleDateString();
    }

    /**
     * Create a single alert element
     */
    function createAlertElement(alert, options) {
        options = $.extend({
            showActions: true,
            showTimestamp: true,
            onAcknowledge: null,
            onDismiss: null
        }, options);

        var typeConfig = alertTypes[alert.type] || alertTypes.info;
        var $alert = $('<div class="' + PREFIX + ' ' + PREFIX + '-' + typeConfig.className + '">')
            .attr('data-alert-id', alert.id);

        // Icon
        var $icon = $('<span class="' + PREFIX + '-icon">')
            .text(alert.icon || typeConfig.icon);
        $alert.append($icon);

        // Content
        var $content = $('<div class="' + PREFIX + '-content">');

        // Title
        var $title = $('<h4 class="' + PREFIX + '-title">')
            .text(alert.title || typeConfig.title);
        $content.append($title);

        // Message
        var $message = $('<p class="' + PREFIX + '-message">')
            .text(alert.message);
        $content.append($message);

        // Timestamp
        if (options.showTimestamp && alert.date) {
            var $time = $('<span class="' + PREFIX + '-time">')
                .text(formatTime(alert.date));
            $content.append($time);
        }

        $alert.append($content);

        // Actions
        if (options.showActions) {
            var $actions = $('<div class="' + PREFIX + '-actions">');

            // Acknowledge button (for system alerts)
            if (typeConfig.requiresAcknowledge && !alert.acknowledged) {
                var $ackBtn = $('<button type="button" class="' + PREFIX + '-btn ' + PREFIX + '-btn-acknowledge">')
                    .text('Acknowledge');

                $ackBtn.on('click', function(e) {
                    e.stopPropagation();
                    $alert.addClass('acknowledged');
                    if (options.onAcknowledge) {
                        options.onAcknowledge(alert.id);
                    }
                });

                $actions.append($ackBtn);
            }

            // Dismiss button
            var $dismissBtn = $('<button type="button" class="' + PREFIX + '-btn ' + PREFIX + '-btn-dismiss">')
                .attr('title', 'Dismiss')
                .html('&times;');

            $dismissBtn.on('click', function(e) {
                e.stopPropagation();
                $alert.fadeOut(200, function() {
                    $(this).remove();
                });
                if (options.onDismiss) {
                    options.onDismiss(alert.id);
                }
            });

            $actions.append($dismissBtn);
            $alert.append($actions);
        }

        return $alert;
    }

    /**
     * Create notification badge
     */
    function createBadge(options) {
        options = $.extend({
            container: null,
            onClick: null
        }, options);

        var $badge = $('<div class="' + PREFIX + '-badge">');
        var $icon = $('<span class="' + PREFIX + '-badge-icon">').text('🔔');
        var $count = $('<span class="' + PREFIX + '-badge-count">').text('0').hide();

        $badge.append($icon, $count);

        if (options.container) {
            $(options.container).append($badge);
        }

        $badge.on('click', function() {
            if (options.onClick) {
                options.onClick();
            }
        });

        return {
            element: $badge,

            setCount: function(count) {
                if (count > 0) {
                    $count.text(count > 99 ? '99+' : count).show();
                    $badge.addClass('has-alerts');
                } else {
                    $count.hide();
                    $badge.removeClass('has-alerts');
                }
            },

            pulse: function() {
                $badge.addClass('pulse');
                setTimeout(function() {
                    $badge.removeClass('pulse');
                }, 500);
            },

            show: function() {
                $badge.show();
            },

            hide: function() {
                $badge.hide();
            }
        };
    }

    /**
     * Create toast notification
     */
    function createToast(alert, options) {
        options = $.extend({
            duration: 5000,
            position: 'top-right',
            onDismiss: null
        }, options);

        var typeConfig = alertTypes[alert.type] || alertTypes.info;
        var $toast = $('<div class="' + PREFIX + '-toast ' + PREFIX + '-toast-' + typeConfig.className + '">')
            .attr('data-alert-id', alert.id);

        // Icon
        var $icon = $('<span class="' + PREFIX + '-toast-icon">')
            .text(alert.icon || typeConfig.icon);
        $toast.append($icon);

        // Content
        var $content = $('<div class="' + PREFIX + '-toast-content">');
        if (alert.title) {
            $content.append($('<strong>').text(alert.title + ': '));
        }
        $content.append($('<span>').text(alert.message));
        $toast.append($content);

        // Close button
        var $close = $('<button type="button" class="' + PREFIX + '-toast-close">')
            .html('&times;');

        $close.on('click', function() {
            dismissToast();
        });

        $toast.append($close);

        // Progress bar for auto-dismiss
        if (options.duration > 0 && !typeConfig.requiresAcknowledge) {
            var $progress = $('<div class="' + PREFIX + '-toast-progress">');
            var $progressBar = $('<div class="' + PREFIX + '-toast-progress-bar">');
            $progress.append($progressBar);
            $toast.append($progress);

            // Animate progress
            $progressBar.css('animation-duration', options.duration + 'ms');
        }

        var dismissTimeout = null;

        function dismissToast() {
            if (dismissTimeout) {
                clearTimeout(dismissTimeout);
            }
            $toast.addClass('dismissing');
            setTimeout(function() {
                $toast.remove();
                if (options.onDismiss) {
                    options.onDismiss(alert.id);
                }
            }, 300);
        }

        // Auto-dismiss
        if (options.duration > 0 && !typeConfig.requiresAcknowledge) {
            dismissTimeout = setTimeout(dismissToast, options.duration);

            // Pause on hover
            $toast.on('mouseenter', function() {
                clearTimeout(dismissTimeout);
                $toast.find('.' + PREFIX + '-toast-progress-bar').css('animation-play-state', 'paused');
            });

            $toast.on('mouseleave', function() {
                dismissTimeout = setTimeout(dismissToast, 2000);
                $toast.find('.' + PREFIX + '-toast-progress-bar').css('animation-play-state', 'running');
            });
        }

        return {
            element: $toast,
            dismiss: dismissToast
        };
    }

    /**
     * Create toast container
     */
    function createToastContainer(position) {
        position = position || 'top-right';
        var containerId = PREFIX + '-toast-container-' + position;
        var $container = $('#' + containerId);

        if (!$container.length) {
            $container = $('<div id="' + containerId + '" class="' + PREFIX + '-toast-container ' + PREFIX + '-toast-' + position + '">');
            $('body').append($container);
        }

        return $container;
    }

    /**
     * Create alerts panel (dropdown/sidebar)
     */
    function createPanel(options) {
        options = $.extend({
            container: null,
            onAcknowledge: null,
            onDismiss: null,
            onClearAll: null,
            onViewAll: null
        }, options);

        var $panel = $('<div class="' + PREFIX + '-panel">');

        // Header
        var $header = $('<div class="' + PREFIX + '-panel-header">');
        $header.append('<h3>Notifications</h3>');

        var $headerActions = $('<div class="' + PREFIX + '-panel-header-actions">');
        var $clearAllBtn = $('<button type="button" class="' + PREFIX + '-panel-clear">Clear All</button>');
        $headerActions.append($clearAllBtn);
        $header.append($headerActions);
        $panel.append($header);

        // Body (alert list)
        var $body = $('<div class="' + PREFIX + '-panel-body">');
        var $list = $('<div class="' + PREFIX + '-panel-list">');
        $body.append($list);
        $panel.append($body);

        // Footer
        var $footer = $('<div class="' + PREFIX + '-panel-footer">');
        var $viewAllBtn = $('<button type="button" class="' + PREFIX + '-panel-view-all">View All Notifications</button>');
        $footer.append($viewAllBtn);
        $panel.append($footer);

        // Empty state
        var $empty = $('<div class="' + PREFIX + '-panel-empty">')
            .html('<span class="empty-icon">🔔</span><p>No notifications</p>')
            .hide();
        $list.append($empty);

        // Event handlers
        $clearAllBtn.on('click', function() {
            if (options.onClearAll) {
                options.onClearAll();
            }
            $list.find('.' + PREFIX).remove();
            updateEmptyState();
        });

        $viewAllBtn.on('click', function() {
            if (options.onViewAll) {
                options.onViewAll();
            }
        });

        function updateEmptyState() {
            var hasAlerts = $list.find('.' + PREFIX).length > 0;
            $empty.toggle(!hasAlerts);
            $clearAllBtn.prop('disabled', !hasAlerts);
        }

        if (options.container) {
            $(options.container).append($panel);
        }

        updateEmptyState();

        return {
            element: $panel,

            addAlert: function(alert) {
                var $alertEl = createAlertElement(alert, {
                    showActions: true,
                    showTimestamp: true,
                    onAcknowledge: options.onAcknowledge,
                    onDismiss: function(id) {
                        if (options.onDismiss) {
                            options.onDismiss(id);
                        }
                        updateEmptyState();
                    }
                });

                $list.prepend($alertEl);
                updateEmptyState();
            },

            removeAlert: function(alertId) {
                $list.find('[data-alert-id="' + alertId + '"]').remove();
                updateEmptyState();
            },

            clear: function() {
                $list.find('.' + PREFIX).remove();
                updateEmptyState();
            },

            setAlerts: function(alerts) {
                $list.find('.' + PREFIX).remove();

                alerts.forEach(function(alert) {
                    var $alertEl = createAlertElement(alert, {
                        showActions: true,
                        showTimestamp: true,
                        onAcknowledge: options.onAcknowledge,
                        onDismiss: function(id) {
                            if (options.onDismiss) {
                                options.onDismiss(id);
                            }
                            updateEmptyState();
                        }
                    });

                    $list.append($alertEl);
                });

                updateEmptyState();
            },

            getCount: function() {
                return $list.find('.' + PREFIX).length;
            },

            show: function() {
                $panel.addClass('open');
            },

            hide: function() {
                $panel.removeClass('open');
            },

            toggle: function() {
                $panel.toggleClass('open');
            },

            isOpen: function() {
                return $panel.hasClass('open');
            }
        };
    }

    /**
     * Create inline alert (for embedding in pages)
     */
    function createInlineAlert(alert, options) {
        options = $.extend({
            dismissible: true,
            onDismiss: null
        }, options);

        var typeConfig = alertTypes[alert.type] || alertTypes.info;
        var $alert = $('<div class="' + PREFIX + '-inline ' + PREFIX + '-inline-' + typeConfig.className + '">');

        // Icon
        var $icon = $('<span class="' + PREFIX + '-inline-icon">')
            .text(alert.icon || typeConfig.icon);
        $alert.append($icon);

        // Content
        var $content = $('<div class="' + PREFIX + '-inline-content">');

        if (alert.title) {
            $content.append($('<strong>').text(alert.title + ' '));
        }

        $content.append($('<span>').text(alert.message));
        $alert.append($content);

        // Dismiss button
        if (options.dismissible) {
            var $dismiss = $('<button type="button" class="' + PREFIX + '-inline-dismiss">')
                .html('&times;');

            $dismiss.on('click', function() {
                $alert.slideUp(200, function() {
                    $(this).remove();
                });
                if (options.onDismiss) {
                    options.onDismiss(alert.id);
                }
            });

            $alert.append($dismiss);
        }

        return {
            element: $alert,
            dismiss: function() {
                $alert.slideUp(200, function() {
                    $(this).remove();
                });
            }
        };
    }

    /**
     * Create main alerts manager
     */
    function create(options) {
        options = $.extend({
            container: null,
            badgeContainer: null,
            toastPosition: 'top-right',
            toastDuration: 5000,
            maxToasts: 5,
            onAcknowledge: null,
            onDismiss: null,
            onClearAll: null
        }, options);

        var alerts = [];
        var toasts = [];
        var badge = null;
        var panel = null;
        var toastContainer = null;

        // Initialize badge
        if (options.badgeContainer) {
            badge = createBadge({
                container: options.badgeContainer,
                onClick: function() {
                    if (panel) {
                        panel.toggle();
                    }
                }
            });
        }

        // Initialize panel
        if (options.container) {
            panel = createPanel({
                container: options.container,
                onAcknowledge: function(id) {
                    markAcknowledged(id);
                    if (options.onAcknowledge) {
                        options.onAcknowledge(id);
                    }
                },
                onDismiss: function(id) {
                    removeAlert(id);
                    if (options.onDismiss) {
                        options.onDismiss(id);
                    }
                },
                onClearAll: function() {
                    clearAll();
                    if (options.onClearAll) {
                        options.onClearAll();
                    }
                }
            });
        }

        // Initialize toast container
        toastContainer = createToastContainer(options.toastPosition);

        // Update badge count
        function updateBadge() {
            if (badge) {
                var unacknowledged = alerts.filter(function(a) {
                    return !a.acknowledged && !a.dismissed;
                }).length;
                badge.setCount(unacknowledged);
            }
        }

        // Add alert
        function addAlert(alert) {
            // Ensure ID
            if (!alert.id) {
                alert.id = generateId();
            }

            // Ensure date
            if (!alert.date) {
                alert.date = new Date();
            }

            // Add to list
            alerts.push(alert);

            // Add to panel
            if (panel) {
                panel.addAlert(alert);
            }

            // Update badge
            updateBadge();

            // Show toast for non-persistent alerts
            var typeConfig = alertTypes[alert.type] || alertTypes.info;
            if (!typeConfig.persistent || alert.showToast) {
                showToast(alert);
            }

            // Pulse badge for important alerts
            if (badge && (alert.type === 'system' || alert.type === 'error')) {
                badge.pulse();
            }

            return alert.id;
        }

        // Show toast
        function showToast(alert) {
            // Limit number of toasts
            while (toasts.length >= options.maxToasts) {
                var oldest = toasts.shift();
                if (oldest) {
                    oldest.dismiss();
                }
            }

            var toast = createToast(alert, {
                duration: options.toastDuration,
                position: options.toastPosition,
                onDismiss: function(id) {
                    toasts = toasts.filter(function(t) {
                        return t.element.attr('data-alert-id') !== id;
                    });
                }
            });

            toastContainer.append(toast.element);
            toasts.push(toast);

            // Trigger animation
            setTimeout(function() {
                toast.element.addClass('show');
            }, 10);

            return toast;
        }

        // Remove alert
        function removeAlert(id) {
            alerts = alerts.filter(function(a) {
                return a.id !== id;
            });

            if (panel) {
                panel.removeAlert(id);
            }

            updateBadge();
        }

        // Mark as acknowledged
        function markAcknowledged(id) {
            alerts.forEach(function(a) {
                if (a.id === id) {
                    a.acknowledged = true;
                }
            });
            updateBadge();
        }

        // Clear all
        function clearAll() {
            alerts = [];
            if (panel) {
                panel.clear();
            }
            updateBadge();
        }

        // Get unread count
        function getUnreadCount() {
            return alerts.filter(function(a) {
                return !a.acknowledged && !a.dismissed;
            }).length;
        }

        // Public API
        return {
            // Show an alert
            show: function(alertConfig) {
                return addAlert(alertConfig);
            },

            // Show specific types
            system: function(message, title) {
                return addAlert({ type: 'system', message: message, title: title });
            },

            warning: function(message, title) {
                return addAlert({ type: 'warning', message: message, title: title });
            },

            error: function(message, title) {
                return addAlert({ type: 'error', message: message, title: title });
            },

            info: function(message, title) {
                return addAlert({ type: 'info', message: message, title: title });
            },

            success: function(message, title) {
                return addAlert({ type: 'success', message: message, title: title });
            },

            // Toast only (no panel)
            toast: function(alertConfig) {
                if (!alertConfig.id) {
                    alertConfig.id = generateId();
                }
                return showToast(alertConfig);
            },

            // Acknowledge alert
            acknowledge: function(id) {
                markAcknowledged(id);
                if (options.onAcknowledge) {
                    options.onAcknowledge(id);
                }
            },

            // Dismiss alert
            dismiss: function(id) {
                removeAlert(id);
                if (options.onDismiss) {
                    options.onDismiss(id);
                }
            },

            // Clear all alerts
            clearAll: clearAll,

            // Get all alerts
            getAlerts: function() {
                return alerts.slice();
            },

            // Get unread count
            getUnreadCount: getUnreadCount,

            // Set alerts (for loading from server)
            setAlerts: function(alertList) {
                alerts = alertList.map(function(a) {
                    if (!a.id) {
                        a.id = generateId();
                    }
                    return a;
                });

                if (panel) {
                    panel.setAlerts(alerts);
                }

                updateBadge();
            },

            // Panel controls
            showPanel: function() {
                if (panel) {
                    panel.show();
                }
            },

            hidePanel: function() {
                if (panel) {
                    panel.hide();
                }
            },

            togglePanel: function() {
                if (panel) {
                    panel.toggle();
                }
            },

            isPanelOpen: function() {
                return panel ? panel.isOpen() : false;
            },

            // Badge reference
            getBadge: function() {
                return badge;
            },

            // Panel reference
            getPanel: function() {
                return panel;
            },

            // Destroy
            destroy: function() {
                if (panel) {
                    panel.element.remove();
                }
                if (badge) {
                    badge.element.remove();
                }
                toastContainer.remove();
                alerts = [];
                toasts = [];
            }
        };
    }

    // Public API
    return {
        create: create,
        createBadge: createBadge,
        createPanel: createPanel,
        createToast: createToast,
        createToastContainer: createToastContainer,
        createInlineAlert: createInlineAlert,
        createAlertElement: createAlertElement,
        alertTypes: alertTypes,
        formatTime: formatTime,
        PREFIX: PREFIX
    };
}());
