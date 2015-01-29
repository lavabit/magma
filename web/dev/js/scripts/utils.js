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
    message: function(message) {
        var message_box = $('<div id="message-box"><p>' + message + '</p></div>').appendTo('body').hide();

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

    die: function(message, type) {
        var error_box = $('<div id="error-message"><p>' + message + '</p></div>').appendTo('body').hide();

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
