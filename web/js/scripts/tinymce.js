/*
var magma = magma || {};

magma.tinymce = function(editorID) {
    var tinyPath = 'tiny_mce/',
        init = function() {
            tinyMCE.dom.Event.domLoaded = true; // https://gist.github.com/379244
            tinyMCE.baseURL = tinyPath; // http://stackoverflow.com/questions/3636309/load-tinymce-on-demand-by-using-jquery

            tinyMCE.init({
                theme: 'advanced',
                mode: 'exact',
                elements: editorID,

                // plugins
                plugins: 'autolink,lists,table,inlinepopups,searchreplace,template',
                dialog_type: 'model',

                // toolbar
                theme_advanced_toolbar_location: 'top',
                theme_advanced_toolbar_align: 'left',
                theme_advanced_buttons1: 'formatselect,fontselect,fontsizeselect,|,forecolor,backcolor,|,bold,italic,underline,|,numlist,bullist,indent,outdent,|,justifyleft,justifycenter,justifyright,justifyfull,|,link,unlink,|,image,|,table,|,undo,redo',
                theme_advanced_buttons2: '',
                theme_advanced_buttons3: '',

                // layout
                width: '100%' // makes resizable
            });
        };

    if(typeof tinyMCE === 'undefined') {
        $.getScript(tinyPath + '/tiny_mce.js', function() {
            init();
        });
    } else {
        init();
    }
};
*/
