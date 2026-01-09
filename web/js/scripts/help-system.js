/**
 * Magma Help System
 *
 * Provides enhanced help functionality including:
 * - Help categories and topics browser
 * - Topic content display with navigation
 * - Search within help
 * - Keyboard shortcuts reference
 * - Quick tips and tooltips
 * - FAQ section
 * - Getting started guide
 * - Contextual help
 */

(function($, magma) {
    'use strict';

    // Help system namespace
    magma.helpSystem = magma.helpSystem || {};

    // Default keyboard shortcuts
    magma.helpSystem.defaultShortcuts = {
        navigation: {
            label: 'Navigation',
            shortcuts: [
                { keys: ['g', 'i'], description: 'Go to Inbox' },
                { keys: ['g', 's'], description: 'Go to Sent' },
                { keys: ['g', 'd'], description: 'Go to Drafts' },
                { keys: ['g', 't'], description: 'Go to Trash' },
                { keys: ['g', 'c'], description: 'Go to Contacts' },
                { keys: ['g', 'o'], description: 'Go to Options' },
                { keys: ['g', 'h'], description: 'Go to Help' }
            ]
        },
        mail: {
            label: 'Mail Actions',
            shortcuts: [
                { keys: ['c'], description: 'Compose new message' },
                { keys: ['r'], description: 'Reply to message' },
                { keys: ['a'], description: 'Reply all' },
                { keys: ['f'], description: 'Forward message' },
                { keys: ['#'], description: 'Delete message' },
                { keys: ['e'], description: 'Archive message' },
                { keys: ['s'], description: 'Star/flag message' },
                { keys: ['l'], description: 'Label message' }
            ]
        },
        selection: {
            label: 'Selection',
            shortcuts: [
                { keys: ['x'], description: 'Select/deselect message' },
                { keys: ['*', 'a'], description: 'Select all' },
                { keys: ['*', 'n'], description: 'Deselect all' },
                { keys: ['*', 'r'], description: 'Select read' },
                { keys: ['*', 'u'], description: 'Select unread' }
            ]
        },
        compose: {
            label: 'Compose',
            shortcuts: [
                { keys: ['Ctrl', 'Enter'], description: 'Send message' },
                { keys: ['Ctrl', 'S'], description: 'Save draft' },
                { keys: ['Ctrl', 'Shift', 'D'], description: 'Discard draft' },
                { keys: ['Tab'], description: 'Next field' },
                { keys: ['Shift', 'Tab'], description: 'Previous field' }
            ]
        },
        general: {
            label: 'General',
            shortcuts: [
                { keys: ['/'], description: 'Search' },
                { keys: ['?'], description: 'Show keyboard shortcuts' },
                { keys: ['Esc'], description: 'Close dialog/cancel' },
                { keys: ['j'], description: 'Next message' },
                { keys: ['k'], description: 'Previous message' },
                { keys: ['o'], description: 'Open message' },
                { keys: ['u'], description: 'Back to list' }
            ]
        }
    };

    // Default FAQ items
    magma.helpSystem.defaultFAQ = [
        {
            question: 'How do I compose a new email?',
            answer: 'Click the "Compose" button in the toolbar or press "C" on your keyboard. A new compose window will open where you can enter recipients, subject, and your message.'
        },
        {
            question: 'How do I add attachments to my email?',
            answer: 'When composing an email, click the "Attach" button (paperclip icon) or drag and drop files directly into the compose window.'
        },
        {
            question: 'How do I organize my emails into folders?',
            answer: 'You can create folders by clicking "Edit Folders" in the folder list. Drag and drop emails to move them between folders, or use the "Move" button in the toolbar.'
        },
        {
            question: 'How do I search for emails?',
            answer: 'Use the search bar at the top of the page. You can search by sender, subject, or content. For advanced search options, click the filter icon next to the search bar.'
        },
        {
            question: 'How do I change my password?',
            answer: 'Go to Options > Security and click "Change Password". You will need to enter your current password and then your new password twice to confirm.'
        },
        {
            question: 'How do I set up an email signature?',
            answer: 'Go to Options > Identity and scroll to the "Signature" section. You can create and edit your email signature there.'
        },
        {
            question: 'How do I mark emails as spam?',
            answer: 'Select the email(s) you want to mark as spam and click the "Spam" button in the toolbar. The messages will be moved to your Spam folder.'
        },
        {
            question: 'How do I recover deleted emails?',
            answer: 'Deleted emails are moved to the Trash folder and kept for 30 days. Open the Trash folder, select the email(s) you want to recover, and click "Move" to restore them to another folder.'
        }
    ];

    // Default quick tips
    magma.helpSystem.defaultTips = [
        'Press "?" at any time to see keyboard shortcuts.',
        'Double-click a message to open it in a new tab.',
        'Drag and drop emails to move them between folders.',
        'Use labels to organize emails that belong to multiple categories.',
        'Press "/" to quickly access the search bar.',
        'Star important emails to find them easily later.',
        'Use filters to automatically organize incoming mail.',
        'Right-click on a message for quick actions.'
    ];

    /**
     * Create help navigation/sidebar
     */
    magma.helpSystem.createNavigation = function(options) {
        options = $.extend({
            categories: [],
            activeCategory: null,
            onCategorySelect: function() {}
        }, options);

        var $element = $('<div class="mgm-help-nav">');
        var $list = $('<ul class="mgm-help-nav-list">');

        $element.append($list);

        function render() {
            $list.empty();

            $.each(options.categories, function(i, category) {
                var $item = $('<li class="mgm-help-nav-item">');
                var $link = $('<a class="mgm-help-nav-link" href="#">');
                var $icon = $('<span class="mgm-help-nav-icon">');
                var $label = $('<span class="mgm-help-nav-label">');

                $icon.html(category.icon || '&#128196;');
                $label.text(category.label || category.name);

                $link.append($icon, $label);
                $item.append($link);

                if (category.id === options.activeCategory) {
                    $item.addClass('active');
                }

                $link.on('click', function(e) {
                    e.preventDefault();
                    setActive(category.id);
                    options.onCategorySelect(category);
                });

                $list.append($item);
            });
        }

        function setActive(categoryId) {
            options.activeCategory = categoryId;
            $list.find('.mgm-help-nav-item').removeClass('active');
            $list.find('.mgm-help-nav-item').each(function() {
                var $item = $(this);
                var category = options.categories[$item.index()];
                if (category && category.id === categoryId) {
                    $item.addClass('active');
                }
            });
        }

        render();

        return {
            element: $element,
            setCategories: function(categories) {
                options.categories = categories;
                render();
            },
            setActive: setActive,
            getActive: function() {
                return options.activeCategory;
            }
        };
    };

    /**
     * Create help topic list
     */
    magma.helpSystem.createTopicList = function(options) {
        options = $.extend({
            topics: [],
            activeTopic: null,
            onTopicSelect: function() {}
        }, options);

        var $element = $('<div class="mgm-help-topics">');
        var $header = $('<div class="mgm-help-topics-header">');
        var $title = $('<h3>Topics</h3>');
        var $list = $('<ul class="mgm-help-topics-list">');
        var $empty = $('<div class="mgm-help-topics-empty">No topics available</div>');

        $header.append($title);
        $element.append($header, $list, $empty);

        function render() {
            $list.empty();

            if (options.topics.length === 0) {
                $list.hide();
                $empty.show();
                return;
            }

            $empty.hide();
            $list.show();

            $.each(options.topics, function(i, topic) {
                var $item = $('<li class="mgm-help-topic-item">');
                var $link = $('<a class="mgm-help-topic-link" href="#">');

                $link.text(topic.name || topic.title);

                if (topic.id === options.activeTopic) {
                    $item.addClass('active');
                }

                $link.on('click', function(e) {
                    e.preventDefault();
                    setActive(topic.id);
                    options.onTopicSelect(topic);
                });

                $item.append($link);
                $list.append($item);
            });
        }

        function setActive(topicId) {
            options.activeTopic = topicId;
            $list.find('.mgm-help-topic-item').removeClass('active');
            $list.find('.mgm-help-topic-item').each(function(i) {
                if (options.topics[i] && options.topics[i].id === topicId) {
                    $(this).addClass('active');
                }
            });
        }

        render();

        return {
            element: $element,
            setTopics: function(topics) {
                options.topics = topics;
                options.activeTopic = null;
                render();
            },
            setActive: setActive,
            getActive: function() {
                return options.activeTopic;
            },
            clear: function() {
                options.topics = [];
                options.activeTopic = null;
                render();
            }
        };
    };

    /**
     * Create help content viewer
     */
    magma.helpSystem.createContentViewer = function(options) {
        options = $.extend({
            title: '',
            content: '',
            showBreadcrumb: true,
            breadcrumb: [],
            onBreadcrumbClick: function() {}
        }, options);

        var $element = $('<div class="mgm-help-content">');
        var $breadcrumb = $('<nav class="mgm-help-breadcrumb">');
        var $header = $('<div class="mgm-help-content-header">');
        var $title = $('<h2 class="mgm-help-content-title">');
        var $body = $('<div class="mgm-help-content-body">');
        var $empty = $('<div class="mgm-help-content-empty">Select a topic to view help content</div>');

        $header.append($title);
        $element.append($breadcrumb, $header, $body, $empty);

        if (!options.showBreadcrumb) {
            $breadcrumb.hide();
        }

        function renderBreadcrumb() {
            $breadcrumb.empty();

            if (options.breadcrumb.length === 0) {
                $breadcrumb.hide();
                return;
            }

            $breadcrumb.show();

            $.each(options.breadcrumb, function(i, item) {
                if (i > 0) {
                    $breadcrumb.append('<span class="mgm-help-breadcrumb-sep">/</span>');
                }

                var $link = $('<a class="mgm-help-breadcrumb-link" href="#">');
                $link.text(item.label);

                if (i === options.breadcrumb.length - 1) {
                    $link.addClass('current');
                } else {
                    $link.on('click', function(e) {
                        e.preventDefault();
                        options.onBreadcrumbClick(item, i);
                    });
                }

                $breadcrumb.append($link);
            });
        }

        function render() {
            renderBreadcrumb();

            if (!options.content && !options.title) {
                $header.hide();
                $body.hide();
                $empty.show();
                return;
            }

            $empty.hide();
            $header.show();
            $body.show();

            $title.text(options.title);

            // Sanitize content before rendering
            var sanitizedContent = options.content;
            if (typeof DOMPurify !== 'undefined') {
                sanitizedContent = DOMPurify.sanitize(options.content, {
                    ALLOWED_TAGS: ['p', 'br', 'b', 'i', 'strong', 'em', 'ul', 'ol', 'li', 'a', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'code', 'pre', 'blockquote', 'table', 'thead', 'tbody', 'tr', 'th', 'td'],
                    ALLOWED_ATTR: ['href', 'target', 'class']
                });
            }

            $body.html(sanitizedContent);
        }

        render();

        return {
            element: $element,
            setContent: function(title, content, breadcrumb) {
                options.title = title;
                options.content = content;
                if (breadcrumb) {
                    options.breadcrumb = breadcrumb;
                }
                render();
            },
            setBreadcrumb: function(breadcrumb) {
                options.breadcrumb = breadcrumb;
                renderBreadcrumb();
            },
            clear: function() {
                options.title = '';
                options.content = '';
                options.breadcrumb = [];
                render();
            }
        };
    };

    /**
     * Create keyboard shortcuts panel
     */
    magma.helpSystem.createShortcutsPanel = function(options) {
        options = $.extend({
            shortcuts: magma.helpSystem.defaultShortcuts,
            columns: 2
        }, options);

        var $element = $('<div class="mgm-help-shortcuts">');
        var $header = $('<div class="mgm-help-shortcuts-header">');
        var $title = $('<h2>Keyboard Shortcuts</h2>');
        var $close = $('<button type="button" class="mgm-help-shortcuts-close" title="Close">&#10005;</button>');
        var $content = $('<div class="mgm-help-shortcuts-content">');

        $header.append($title, $close);
        $element.append($header, $content);

        function render() {
            $content.empty();

            var $grid = $('<div class="mgm-help-shortcuts-grid">');
            $grid.css('grid-template-columns', 'repeat(' + options.columns + ', 1fr)');

            $.each(options.shortcuts, function(categoryKey, category) {
                var $section = $('<div class="mgm-help-shortcuts-section">');
                var $sectionTitle = $('<h4>').text(category.label);
                var $table = $('<table class="mgm-help-shortcuts-table">');

                $.each(category.shortcuts, function(i, shortcut) {
                    var $row = $('<tr>');
                    var $keys = $('<td class="mgm-help-shortcut-keys">');
                    var $desc = $('<td class="mgm-help-shortcut-desc">');

                    $.each(shortcut.keys, function(j, key) {
                        if (j > 0) {
                            $keys.append('<span class="mgm-help-key-sep">+</span>');
                        }
                        $keys.append($('<kbd>').text(key));
                    });

                    $desc.text(shortcut.description);
                    $row.append($keys, $desc);
                    $table.append($row);
                });

                $section.append($sectionTitle, $table);
                $grid.append($section);
            });

            $content.append($grid);
        }

        $close.on('click', function() {
            $element.hide();
        });

        render();

        return {
            element: $element,
            setShortcuts: function(shortcuts) {
                options.shortcuts = shortcuts;
                render();
            },
            show: function() {
                $element.show();
            },
            hide: function() {
                $element.hide();
            },
            toggle: function() {
                $element.toggle();
            }
        };
    };

    /**
     * Create FAQ panel
     */
    magma.helpSystem.createFAQPanel = function(options) {
        options = $.extend({
            items: magma.helpSystem.defaultFAQ,
            expandFirst: false
        }, options);

        var $element = $('<div class="mgm-help-faq">');
        var $header = $('<div class="mgm-help-faq-header">');
        var $title = $('<h2>Frequently Asked Questions</h2>');
        var $list = $('<div class="mgm-help-faq-list">');

        $header.append($title);
        $element.append($header, $list);

        function render() {
            $list.empty();

            $.each(options.items, function(i, item) {
                var $item = $('<div class="mgm-help-faq-item">');
                var $question = $('<button class="mgm-help-faq-question">');
                var $icon = $('<span class="mgm-help-faq-icon">&#9654;</span>');
                var $text = $('<span class="mgm-help-faq-text">');
                var $answer = $('<div class="mgm-help-faq-answer">');

                $text.text(item.question);
                $question.append($icon, $text);
                $answer.text(item.answer);

                if (options.expandFirst && i === 0) {
                    $item.addClass('expanded');
                    $answer.show();
                } else {
                    $answer.hide();
                }

                $question.on('click', function() {
                    var isExpanded = $item.hasClass('expanded');

                    if (isExpanded) {
                        $item.removeClass('expanded');
                        $answer.slideUp(200);
                    } else {
                        $item.addClass('expanded');
                        $answer.slideDown(200);
                    }
                });

                $item.append($question, $answer);
                $list.append($item);
            });
        }

        render();

        return {
            element: $element,
            setItems: function(items) {
                options.items = items;
                render();
            },
            expandAll: function() {
                $list.find('.mgm-help-faq-item').addClass('expanded');
                $list.find('.mgm-help-faq-answer').slideDown(200);
            },
            collapseAll: function() {
                $list.find('.mgm-help-faq-item').removeClass('expanded');
                $list.find('.mgm-help-faq-answer').slideUp(200);
            }
        };
    };

    /**
     * Create quick tips widget
     */
    magma.helpSystem.createQuickTips = function(options) {
        options = $.extend({
            tips: magma.helpSystem.defaultTips,
            autoRotate: true,
            rotateInterval: 10000
        }, options);

        var $element = $('<div class="mgm-help-tips">');
        var $header = $('<div class="mgm-help-tips-header">');
        var $title = $('<h4>Quick Tip</h4>');
        var $close = $('<button type="button" class="mgm-help-tips-close" title="Dismiss">&#10005;</button>');
        var $content = $('<div class="mgm-help-tips-content">');
        var $text = $('<p class="mgm-help-tips-text">');
        var $nav = $('<div class="mgm-help-tips-nav">');
        var $prev = $('<button type="button" class="mgm-help-tips-prev" title="Previous">&#9664;</button>');
        var $next = $('<button type="button" class="mgm-help-tips-next" title="Next">&#9654;</button>');
        var $counter = $('<span class="mgm-help-tips-counter">');

        $header.append($title, $close);
        $nav.append($prev, $counter, $next);
        $content.append($text, $nav);
        $element.append($header, $content);

        var currentIndex = 0;
        var rotateTimer = null;

        function showTip(index) {
            if (options.tips.length === 0) return;

            currentIndex = ((index % options.tips.length) + options.tips.length) % options.tips.length;
            $text.text(options.tips[currentIndex]);
            $counter.text((currentIndex + 1) + ' / ' + options.tips.length);
        }

        function startRotation() {
            if (options.autoRotate && options.tips.length > 1) {
                stopRotation();
                rotateTimer = setInterval(function() {
                    showTip(currentIndex + 1);
                }, options.rotateInterval);
            }
        }

        function stopRotation() {
            if (rotateTimer) {
                clearInterval(rotateTimer);
                rotateTimer = null;
            }
        }

        $prev.on('click', function() {
            stopRotation();
            showTip(currentIndex - 1);
        });

        $next.on('click', function() {
            stopRotation();
            showTip(currentIndex + 1);
        });

        $close.on('click', function() {
            stopRotation();
            $element.hide();
        });

        showTip(0);
        startRotation();

        return {
            element: $element,
            setTips: function(tips) {
                options.tips = tips;
                currentIndex = 0;
                showTip(0);
            },
            show: function() {
                $element.show();
                startRotation();
            },
            hide: function() {
                stopRotation();
                $element.hide();
            },
            next: function() {
                showTip(currentIndex + 1);
            },
            prev: function() {
                showTip(currentIndex - 1);
            },
            destroy: function() {
                stopRotation();
                $element.remove();
            }
        };
    };

    /**
     * Create help search
     */
    magma.helpSystem.createSearch = function(options) {
        options = $.extend({
            placeholder: 'Search help...',
            onSearch: function() {}
        }, options);

        var $element = $('<div class="mgm-help-search">');
        var $inputWrapper = $('<div class="mgm-help-search-wrapper">');
        var $icon = $('<span class="mgm-help-search-icon">&#128269;</span>');
        var $input = $('<input type="text" class="mgm-help-search-input">');
        var $clear = $('<button type="button" class="mgm-help-search-clear" title="Clear">&#10005;</button>');

        $input.attr('placeholder', options.placeholder);
        $clear.hide();

        $inputWrapper.append($icon, $input, $clear);
        $element.append($inputWrapper);

        $input.on('input', function() {
            var value = $(this).val();
            $clear.toggle(value.length > 0);
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
            options.onSearch('');
        });

        return {
            element: $element,
            getValue: function() {
                return $input.val();
            },
            setValue: function(value) {
                $input.val(value);
                $clear.toggle(value.length > 0);
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
     * Create contextual help tooltip
     */
    magma.helpSystem.createTooltip = function(options) {
        options = $.extend({
            content: '',
            position: 'bottom',
            trigger: null
        }, options);

        var $element = $('<div class="mgm-help-tooltip">');
        var $arrow = $('<div class="mgm-help-tooltip-arrow">');
        var $content = $('<div class="mgm-help-tooltip-content">');

        $element.append($arrow, $content);
        $element.addClass('mgm-help-tooltip-' + options.position);
        $element.hide();

        function updateContent(content) {
            $content.text(content);
        }

        function show($trigger) {
            if (!$trigger) return;

            updateContent(options.content);

            // Position relative to trigger
            var offset = $trigger.offset();
            var triggerWidth = $trigger.outerWidth();
            var triggerHeight = $trigger.outerHeight();

            $('body').append($element);
            $element.show();

            var tooltipWidth = $element.outerWidth();
            var tooltipHeight = $element.outerHeight();

            var top, left;

            switch (options.position) {
                case 'top':
                    top = offset.top - tooltipHeight - 8;
                    left = offset.left + (triggerWidth - tooltipWidth) / 2;
                    break;
                case 'bottom':
                    top = offset.top + triggerHeight + 8;
                    left = offset.left + (triggerWidth - tooltipWidth) / 2;
                    break;
                case 'left':
                    top = offset.top + (triggerHeight - tooltipHeight) / 2;
                    left = offset.left - tooltipWidth - 8;
                    break;
                case 'right':
                    top = offset.top + (triggerHeight - tooltipHeight) / 2;
                    left = offset.left + triggerWidth + 8;
                    break;
            }

            $element.css({ top: top, left: left });
        }

        function hide() {
            $element.hide().detach();
        }

        // Auto-bind to trigger if provided
        if (options.trigger) {
            $(options.trigger).on('mouseenter focus', function() {
                show($(this));
            }).on('mouseleave blur', function() {
                hide();
            });
        }

        return {
            element: $element,
            show: show,
            hide: hide,
            setContent: function(content) {
                options.content = content;
                updateContent(content);
            },
            destroy: function() {
                hide();
                if (options.trigger) {
                    $(options.trigger).off('mouseenter focus mouseleave blur');
                }
            }
        };
    };

    /**
     * Create getting started guide
     */
    magma.helpSystem.createGettingStarted = function(options) {
        options = $.extend({
            steps: [
                {
                    title: 'Welcome to Lavabit Webmail',
                    content: 'This guide will help you get started with the key features of your secure email account.',
                    icon: '&#128075;'
                },
                {
                    title: 'Composing Messages',
                    content: 'Click the "Compose" button or press "C" to start a new email. Add recipients, a subject, and your message.',
                    icon: '&#9993;'
                },
                {
                    title: 'Organizing Your Mail',
                    content: 'Use folders to organize your emails. Drag and drop messages or use the "Move" button to file them.',
                    icon: '&#128193;'
                },
                {
                    title: 'Using Labels',
                    content: 'Add labels to categorize emails. Unlike folders, an email can have multiple labels.',
                    icon: '&#127991;'
                },
                {
                    title: 'Keyboard Shortcuts',
                    content: 'Press "?" at any time to see available keyboard shortcuts for faster navigation.',
                    icon: '&#9000;'
                },
                {
                    title: 'Get Help',
                    content: 'Visit the Help section for detailed guides, FAQs, and contact support options.',
                    icon: '&#10067;'
                }
            ],
            onComplete: function() {},
            onSkip: function() {}
        }, options);

        var $element = $('<div class="mgm-help-getting-started">');
        var $overlay = $('<div class="mgm-help-getting-started-overlay">');
        var $modal = $('<div class="mgm-help-getting-started-modal">');
        var $header = $('<div class="mgm-help-getting-started-header">');
        var $content = $('<div class="mgm-help-getting-started-content">');
        var $footer = $('<div class="mgm-help-getting-started-footer">');

        var $icon = $('<div class="mgm-help-getting-started-icon">');
        var $title = $('<h2 class="mgm-help-getting-started-title">');
        var $text = $('<p class="mgm-help-getting-started-text">');
        var $progress = $('<div class="mgm-help-getting-started-progress">');

        var $skip = $('<button type="button" class="mgm-help-getting-started-skip">Skip</button>');
        var $prev = $('<button type="button" class="mgm-help-getting-started-prev">Previous</button>');
        var $next = $('<button type="button" class="mgm-help-getting-started-next">Next</button>');

        $header.append($icon, $title);
        $content.append($text);
        $footer.append($skip, $progress, $prev, $next);
        $modal.append($header, $content, $footer);
        $element.append($overlay, $modal);

        var currentStep = 0;

        function renderStep() {
            var step = options.steps[currentStep];

            $icon.html(step.icon);
            $title.text(step.title);
            $text.text(step.content);

            // Update progress dots
            $progress.empty();
            for (var i = 0; i < options.steps.length; i++) {
                var $dot = $('<span class="mgm-help-getting-started-dot">');
                if (i === currentStep) {
                    $dot.addClass('active');
                }
                $progress.append($dot);
            }

            // Update buttons
            $prev.toggle(currentStep > 0);
            $next.text(currentStep === options.steps.length - 1 ? 'Get Started' : 'Next');
        }

        $skip.on('click', function() {
            hide();
            options.onSkip();
        });

        $prev.on('click', function() {
            if (currentStep > 0) {
                currentStep--;
                renderStep();
            }
        });

        $next.on('click', function() {
            if (currentStep < options.steps.length - 1) {
                currentStep++;
                renderStep();
            } else {
                hide();
                options.onComplete();
            }
        });

        $overlay.on('click', function() {
            hide();
            options.onSkip();
        });

        function show() {
            currentStep = 0;
            renderStep();
            $element.show();
        }

        function hide() {
            $element.hide();
        }

        renderStep();
        $element.hide();

        return {
            element: $element,
            show: show,
            hide: hide,
            goToStep: function(step) {
                if (step >= 0 && step < options.steps.length) {
                    currentStep = step;
                    renderStep();
                }
            },
            destroy: function() {
                $element.remove();
            }
        };
    };

    /**
     * Create main help system component
     */
    magma.helpSystem.create = function(options) {
        options = $.extend({
            container: null,
            categories: [],
            showSearch: true,
            showNav: true,
            showShortcuts: true,
            showFAQ: true,
            showTips: true,
            onCategoryLoad: function() {},
            onTopicLoad: function() {}
        }, options);

        var $container = options.container ? $(options.container) : $('<div>');
        var $element = $('<div class="mgm-help-system">');

        // Components
        var search = null;
        var navigation = null;
        var topicList = null;
        var contentViewer = null;
        var shortcutsPanel = null;
        var faqPanel = null;
        var quickTips = null;

        // Main layout
        var $main = $('<div class="mgm-help-main">');
        var $sidebar = $('<div class="mgm-help-sidebar">');
        var $content = $('<div class="mgm-help-content-area">');

        // Header with search
        if (options.showSearch) {
            search = magma.helpSystem.createSearch({
                onSearch: function(query) {
                    performSearch(query);
                }
            });
            $element.append(search.element);
        }

        // Navigation
        if (options.showNav) {
            navigation = magma.helpSystem.createNavigation({
                categories: options.categories,
                onCategorySelect: function(category) {
                    loadCategory(category);
                }
            });
            $sidebar.append(navigation.element);
        }

        // Topic list
        topicList = magma.helpSystem.createTopicList({
            onTopicSelect: function(topic) {
                loadTopic(topic);
            }
        });
        $sidebar.append(topicList.element);

        // Content viewer
        contentViewer = magma.helpSystem.createContentViewer({
            onBreadcrumbClick: function(item, index) {
                if (index === 0) {
                    // Go to category
                    topicList.clear();
                    contentViewer.clear();
                } else if (index === 1) {
                    // Go to topic list
                    contentViewer.clear();
                }
            }
        });
        $content.append(contentViewer.element);

        $main.append($sidebar, $content);
        $element.append($main);

        // Shortcuts panel (hidden by default)
        if (options.showShortcuts) {
            shortcutsPanel = magma.helpSystem.createShortcutsPanel();
            shortcutsPanel.element.hide();
            $element.append(shortcutsPanel.element);
        }

        // FAQ panel
        if (options.showFAQ) {
            faqPanel = magma.helpSystem.createFAQPanel();
        }

        // Quick tips
        if (options.showTips) {
            quickTips = magma.helpSystem.createQuickTips();
            quickTips.element.hide();
        }

        $container.append($element);

        function loadCategory(category) {
            options.onCategoryLoad(category, function(topics) {
                topicList.setTopics(topics);
                contentViewer.clear();
                contentViewer.setBreadcrumb([
                    { label: 'Help', id: 'home' },
                    { label: category.label || category.name, id: category.id }
                ]);
            });
        }

        function loadTopic(topic) {
            options.onTopicLoad(topic, function(content) {
                var breadcrumb = contentViewer.element.find('.mgm-help-breadcrumb-link').map(function() {
                    return { label: $(this).text() };
                }).get();
                breadcrumb.push({ label: topic.name || topic.title, id: topic.id });

                contentViewer.setContent(topic.name || topic.title, content, breadcrumb);
            });
        }

        function performSearch(query) {
            if (!query) {
                return;
            }
            // Search would be implemented here
            // For now, just show a message
            contentViewer.setContent('Search Results', '<p>Search results for: ' + $('<div>').text(query).html() + '</p>');
        }

        return {
            element: $element,
            setCategories: function(categories) {
                if (navigation) {
                    navigation.setCategories(categories);
                }
            },
            loadCategory: loadCategory,
            loadTopic: loadTopic,
            showShortcuts: function() {
                if (shortcutsPanel) {
                    shortcutsPanel.show();
                }
            },
            hideShortcuts: function() {
                if (shortcutsPanel) {
                    shortcutsPanel.hide();
                }
            },
            showFAQ: function() {
                if (faqPanel) {
                    contentViewer.element.hide();
                    $content.append(faqPanel.element);
                }
            },
            hideFAQ: function() {
                if (faqPanel) {
                    faqPanel.element.detach();
                    contentViewer.element.show();
                }
            },
            showTips: function() {
                if (quickTips) {
                    $element.append(quickTips.element);
                    quickTips.show();
                }
            },
            hideTips: function() {
                if (quickTips) {
                    quickTips.hide();
                }
            },
            getSearch: function() {
                return search;
            },
            getNavigation: function() {
                return navigation;
            },
            getTopicList: function() {
                return topicList;
            },
            getContentViewer: function() {
                return contentViewer;
            },
            getShortcutsPanel: function() {
                return shortcutsPanel;
            },
            getFAQPanel: function() {
                return faqPanel;
            },
            getQuickTips: function() {
                return quickTips;
            },
            destroy: function() {
                if (quickTips) {
                    quickTips.destroy();
                }
                $element.remove();
            }
        };
    };

})(jQuery, magma);
