/**
 * utils.js
 *
 * various magma methods, jQuery extentions, and javascript extentions
 */

/**
 * store session ID
 */
var magma = magma || {};

magma.session = (function() {
    // session consists of username and session_id
    var sid;

    return {
        set: function(session) {
            sid = session;
        },
        get: function() {
            return sid;
        }
    };
}());

// dialog helpers
magma.dialog = {
    // SECURITY FIX (V-002): Use jQuery's text() method to safely escape HTML entities
    // in dialog messages. This prevents XSS attacks through error messages
    // that might contain user-controlled or server-supplied content.
    // Previously, messages were concatenated directly into HTML which allowed
    // script injection via payloads like <script>alert(1)</script>
    message: function(message) {
        // Create elements separately and use .text() to safely set content
        // jQuery's .text() automatically escapes HTML entities like < > & "
        var message_box = $('<div id="message-box"></div>');
        var paragraph = $('<p></p>').text(message);  // Safe: escapes HTML
        message_box.append(paragraph).appendTo('body').hide();

        message_box.dialog({
            resizable: false,
            draggable: false,
            modal: true,
            title: "Message",
            buttons: {
                "Ok": function() {
                    $(this).dialog("close");
                }
            },
            close: function() {
                $(this).remove();
            }
        });
    },

    // SECURITY FIX (V-002): Same fix applied to error dialogs.
    // Error messages from server responses could contain malicious content.
    die: function(message, type) {
        // Create elements separately and use .text() to safely set content
        var error_box = $('<div id="error-message"></div>');
        var paragraph = $('<p></p>').text(message);  // Safe: escapes HTML
        error_box.append(paragraph).appendTo('body').hide();

        type = type || "error";

        error_box.dialog({
            resizable: false,
            draggable: false,
            modal: true,
            title: "Error",
            buttons: {
                "Ok": function() {
                    $(this).dialog("close");
                }
            },
            close: function() {
                $(this).remove();
            }
        });
    }
};

/**
 * Fade in jquery objects
 *
 * Saves a bit of typing
 */
(function($) {
    $.fn.reveal = function() {
        this.css('display', 'none').fadeIn(magma.animation_speed);
        return this;
    };
}(jQuery));

/**
 * Fill in input placeholders with labels using jquery-watermark.js
 *
 * Acts on selected inputs to be filled.
 * Labels for field must match an inputs id.
 * 
 * @param labels    jQuery object containing labels to fill inputs with
 */
(function($) {
    $.fn.fillWatermarks = function(labels) {
        if(!labels.length) {
            throw new Error('fillWatermarks: must provide a set of labels to fillWatermarks');
        }

        // this is selected inputs
        this.each(function() {
            var label = labels.filter('[for=' + $(this).attr('id') + ']');
            if(!label.length) {
                throw new Error('fillWatermarks: no matching label provided for input');
            }
            $(this).watermark(label.html());
        });
    };
}(jQuery));
