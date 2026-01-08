/**
 * Magma Tag Manager
 *
 * Enhanced tag management with:
 * - Tag creation with input
 * - Tag color assignment and persistence
 * - Tag removal from messages
 * - Tag filtering/narrowing by tag
 * - Color picker integration
 *
 * Usage:
 *   var tagManager = magma.tagManager.init(tagsModel, options);
 *   tagManager.createTagInput(container, onTagCreated);
 *   tagManager.setTagColor('work', '#ff5722');
 */

var magma = magma || {};

magma.tagManager = (function() {
    'use strict';

    // Default tag colors palette
    var defaultColors = [
        '#e53935', // Red
        '#d81b60', // Pink
        '#8e24aa', // Purple
        '#5e35b1', // Deep Purple
        '#3949ab', // Indigo
        '#1e88e5', // Blue
        '#039be5', // Light Blue
        '#00acc1', // Cyan
        '#00897b', // Teal
        '#43a047', // Green
        '#7cb342', // Light Green
        '#c0ca33', // Lime
        '#fdd835', // Yellow
        '#ffb300', // Amber
        '#fb8c00', // Orange
        '#6d4c41', // Brown
        '#757575', // Grey
        '#546e7a'  // Blue Grey
    ];

    // Storage key for tag colors
    var STORAGE_KEY = 'magma_tag_colors';

    // Tag colors cache
    var tagColors = {};

    /**
     * Load tag colors from localStorage
     */
    function loadColors() {
        try {
            var stored = localStorage.getItem(STORAGE_KEY);
            if (stored) {
                tagColors = JSON.parse(stored);
            }
        } catch (e) {
            console.warn('tagManager: Could not load tag colors from storage');
            tagColors = {};
        }
    }

    /**
     * Save tag colors to localStorage
     */
    function saveColors() {
        try {
            localStorage.setItem(STORAGE_KEY, JSON.stringify(tagColors));
        } catch (e) {
            console.warn('tagManager: Could not save tag colors to storage');
        }
    }

    /**
     * Get color for a tag
     * @param {string} tagName - The tag name
     * @returns {string|null} The color hex code or null
     */
    function getColor(tagName) {
        var slug = slugify(tagName);
        return tagColors[slug] || null;
    }

    /**
     * Set color for a tag
     * @param {string} tagName - The tag name
     * @param {string} color - The color hex code
     */
    function setColor(tagName, color) {
        var slug = slugify(tagName);
        tagColors[slug] = color;
        saveColors();

        // Update any visible tags with this color
        $('.tag-' + slug + ', .tag[data-tag="' + slug + '"]').css({
            'background-color': color,
            'border-color': color
        });
    }

    /**
     * Remove color for a tag
     * @param {string} tagName - The tag name
     */
    function removeColor(tagName) {
        var slug = slugify(tagName);
        delete tagColors[slug];
        saveColors();
    }

    /**
     * Generate a slug from tag name
     * @param {string} name - The tag name
     * @returns {string} The slug
     */
    function slugify(name) {
        if (!name) return '';
        return name.toLowerCase()
            .replace(/[^\w\s-]/g, '')
            .replace(/[\s_-]+/g, '-')
            .replace(/^-+|-+$/g, '');
    }

    /**
     * Create a tag element
     * @param {Object} tag - Tag object with name, slug, optional color
     * @param {Object} options - Options like removable, clickable
     * @returns {jQuery} The tag element
     */
    function createTagElement(tag, options) {
        options = $.extend({
            removable: false,
            clickable: true,
            showColor: true
        }, options);

        var slug = tag.slug || slugify(tag.name || tag);
        var name = tag.name || tag;
        var color = tag.color || getColor(name);

        var $tag = $('<span class="tag tag-' + slug + '">')
            .attr('data-tag', slug)
            .attr('data-tag-name', name);

        var $name = $('<span class="tag-name">').text(name);
        $tag.append($name);

        if (options.removable) {
            var $remove = $('<a class="tag-remove" href="#remove" title="Remove tag">&times;</a>');
            $tag.append($remove);
        }

        if (color && options.showColor) {
            $tag.css({
                'background-color': color,
                'border-color': color,
                'color': getContrastColor(color)
            });
        }

        if (options.clickable) {
            $tag.addClass('tag-clickable');
        }

        return $tag;
    }

    /**
     * Get contrasting text color (black or white) for background
     * @param {string} hexColor - Background color
     * @returns {string} '#000' or '#fff'
     */
    function getContrastColor(hexColor) {
        if (!hexColor) return '#000';

        // Remove # if present
        var hex = hexColor.replace('#', '');

        // Convert to RGB
        var r = parseInt(hex.substr(0, 2), 16);
        var g = parseInt(hex.substr(2, 2), 16);
        var b = parseInt(hex.substr(4, 2), 16);

        // Calculate luminance
        var luminance = (0.299 * r + 0.587 * g + 0.114 * b) / 255;

        return luminance > 0.5 ? '#000' : '#fff';
    }

    /**
     * Create tag input component
     * @param {jQuery} container - Container to append input to
     * @param {Function} onTagCreated - Callback when tag is created
     * @param {Object} options - Additional options
     * @returns {Object} Input API
     */
    function createTagInput(container, onTagCreated, options) {
        options = $.extend({
            placeholder: 'Add tag...',
            autocomplete: true,
            maxLength: 50
        }, options);

        var $wrapper = $('<div class="tag-input-wrapper">');
        var $input = $('<input type="text" class="tag-input">')
            .attr('placeholder', options.placeholder)
            .attr('maxlength', options.maxLength);
        var $button = $('<button type="button" class="tag-input-add">Add</button>');
        var $suggestions = $('<ul class="tag-suggestions">').hide();

        $wrapper.append($input, $button, $suggestions);
        container.append($wrapper);

        // Handle input
        var submitTag = function() {
            var value = $.trim($input.val());
            if (value) {
                if (typeof onTagCreated === 'function') {
                    onTagCreated(value);
                }
                $input.val('').focus();
                $suggestions.hide();
            }
        };

        $button.on('click', submitTag);

        $input.on('keypress', function(e) {
            if (e.which === 13) { // Enter
                e.preventDefault();
                submitTag();
            }
        });

        // Autocomplete suggestions
        if (options.autocomplete && options.existingTags) {
            $input.on('input', function() {
                var value = $.trim($input.val()).toLowerCase();
                $suggestions.empty();

                if (value.length < 1) {
                    $suggestions.hide();
                    return;
                }

                var matches = options.existingTags.filter(function(tag) {
                    var tagName = (tag.name || tag).toLowerCase();
                    return tagName.indexOf(value) !== -1;
                });

                if (matches.length) {
                    $.each(matches.slice(0, 5), function(i, tag) {
                        var name = tag.name || tag;
                        var $li = $('<li>').text(name);
                        $li.on('click', function() {
                            $input.val(name);
                            submitTag();
                        });
                        $suggestions.append($li);
                    });
                    $suggestions.show();
                } else {
                    $suggestions.hide();
                }
            });

            $input.on('blur', function() {
                // Delay hiding to allow click on suggestion
                setTimeout(function() {
                    $suggestions.hide();
                }, 200);
            });
        }

        return {
            element: $wrapper,
            input: $input,
            focus: function() {
                $input.focus();
            },
            clear: function() {
                $input.val('');
            },
            setExistingTags: function(tags) {
                options.existingTags = tags;
            }
        };
    }

    /**
     * Create color picker for tag
     * @param {string} tagName - The tag name
     * @param {jQuery} container - Container to append picker to
     * @param {Function} onChange - Callback when color changes
     * @returns {Object} Color picker API
     */
    function createColorPicker(tagName, container, onChange) {
        var currentColor = getColor(tagName);

        var $picker = $('<div class="tag-color-picker">');
        var $preview = $('<div class="tag-color-preview">')
            .css('background-color', currentColor || '#ccc');
        var $colors = $('<div class="tag-color-options">');

        // Add color swatches
        $.each(defaultColors, function(i, color) {
            var $swatch = $('<div class="tag-color-swatch">')
                .css('background-color', color)
                .attr('data-color', color);

            if (color === currentColor) {
                $swatch.addClass('selected');
            }

            $swatch.on('click', function() {
                $colors.find('.selected').removeClass('selected');
                $swatch.addClass('selected');
                $preview.css('background-color', color);

                setColor(tagName, color);

                if (typeof onChange === 'function') {
                    onChange(color);
                }
            });

            $colors.append($swatch);
        });

        // Add "no color" option
        var $noColor = $('<div class="tag-color-swatch tag-color-none">')
            .attr('data-color', '')
            .text('×');

        if (!currentColor) {
            $noColor.addClass('selected');
        }

        $noColor.on('click', function() {
            $colors.find('.selected').removeClass('selected');
            $noColor.addClass('selected');
            $preview.css('background-color', '#ccc');

            removeColor(tagName);

            if (typeof onChange === 'function') {
                onChange(null);
            }
        });

        $colors.prepend($noColor);
        $picker.append($preview, $colors);
        container.append($picker);

        return {
            element: $picker,
            getColor: function() {
                return getColor(tagName);
            },
            setColor: function(color) {
                setColor(tagName, color);
                $preview.css('background-color', color || '#ccc');
                $colors.find('.selected').removeClass('selected');
                $colors.find('[data-color="' + (color || '') + '"]').addClass('selected');
            }
        };
    }

    /**
     * Create tag list display
     * @param {Array} tags - Array of tag objects or names
     * @param {Object} options - Display options
     * @returns {jQuery} Tag list element
     */
    function createTagList(tags, options) {
        options = $.extend({
            removable: false,
            clickable: true,
            onRemove: null,
            onClick: null
        }, options);

        var $list = $('<div class="tag-list">');

        $.each(tags, function(i, tag) {
            var $tag = createTagElement(tag, {
                removable: options.removable,
                clickable: options.clickable
            });

            if (options.removable && options.onRemove) {
                $tag.find('.tag-remove').on('click', function(e) {
                    e.preventDefault();
                    e.stopPropagation();
                    options.onRemove(tag);
                });
            }

            if (options.clickable && options.onClick) {
                $tag.on('click', function(e) {
                    if (!$(e.target).hasClass('tag-remove')) {
                        e.preventDefault();
                        options.onClick(tag);
                    }
                });
            }

            $list.append($tag);
        });

        return $list;
    }

    /**
     * Get common tags from multiple messages
     * @param {Array} messages - Array of message objects
     * @returns {Array} Array of tags present in ALL messages
     */
    function getCommonTags(messages) {
        if (!messages || !messages.length) {
            return [];
        }

        // Start with tags from first message
        var common = (messages[0].tags || []).map(function(t) {
            return t.name || t;
        });

        // Intersect with tags from remaining messages
        for (var i = 1; i < messages.length; i++) {
            var msgTags = (messages[i].tags || []).map(function(t) {
                return t.name || t;
            });

            common = common.filter(function(tag) {
                return msgTags.indexOf(tag) !== -1;
            });
        }

        return common;
    }

    /**
     * Get all unique tags from multiple messages
     * @param {Array} messages - Array of message objects
     * @returns {Array} Array of all unique tags
     */
    function getAllTags(messages) {
        var tagMap = {};

        $.each(messages, function(i, msg) {
            $.each(msg.tags || [], function(j, tag) {
                var name = tag.name || tag;
                tagMap[name] = true;
            });
        });

        return Object.keys(tagMap);
    }

    /**
     * Initialize tag manager
     * @param {Object} tagsModel - The tags model (optional)
     * @param {Object} options - Configuration options
     * @returns {Object} Tag manager API
     */
    function init(tagsModel, options) {
        options = $.extend({
            persistColors: true
        }, options);

        // Load persisted colors
        if (options.persistColors) {
            loadColors();
        }

        return {
            createTagElement: createTagElement,
            createTagInput: createTagInput,
            createColorPicker: createColorPicker,
            createTagList: createTagList,
            getColor: getColor,
            setColor: setColor,
            removeColor: removeColor,
            getCommonTags: getCommonTags,
            getAllTags: getAllTags,
            getContrastColor: getContrastColor,
            defaultColors: defaultColors
        };
    }

    // Initialize colors on load
    loadColors();

    // Public API
    return {
        init: init,
        createTagElement: createTagElement,
        createTagInput: createTagInput,
        createColorPicker: createColorPicker,
        createTagList: createTagList,
        getColor: getColor,
        setColor: setColor,
        removeColor: removeColor,
        getCommonTags: getCommonTags,
        getAllTags: getAllTags,
        getContrastColor: getContrastColor,
        defaultColors: defaultColors,
        slugify: slugify
    };
}());
