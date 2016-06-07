var magma = magma || {};

/*
 * initialize ckeditor
 * loads the ckeditor script on first init
 *
 * @param textareaID    id of textarea html element
 */
magma.ckeditor = function(textareaID) {
    var editorPath = 'ckeditor/',
        init = function() {
            // make sure the instance doesn't exist already
            if(CKEDITOR && CKEDITOR.instances[textareaID]) {
                delete CKEDITOR.instances[textareaID];
            }
            CKEDITOR.replace(textareaID, {
                toolbar: 'Compose',
                height: $('#' + textareaID).height(),
                toolbar_Compose: [{
                    name: 'styles',
                    items: ['Format', 'Font', 'FontSize']
                }, {
                    name: 'colors',
                    items: ['TextColor', 'BGColor']
                }, {
                    name: 'basicstyles',
                    items: ['Bold', 'Italic', 'Underline']
                }, {
                    name: 'paragraph',
                    items: ['NumberedList', 'BulletedList', 'Outdent', 'Indent','-','JustifyLeft','JustifyCenter','JustifyRight','JustifyBlock']
                }, {
                    name: 'links',
                    items: ['Link', 'Unlink']
                }, {
                    name: 'insert',
                    items: ['Image','-','Table']
                }]
            });
        };

    if(typeof CKEDITOR === 'undefined') {
        // must set base path since it looks for for path in head script tags which we aren't using
        window.CKEDITOR_BASEPATH = editorPath;

        $.getScript(editorPath + 'ckeditor.js', function() {
            $.getScript(editorPath + 'adapters/jquery.js', function() {
                init();
            });
        });
    } else {
        init();
    }
};
