/**
 * Contains all the html blocks for the views
 *
 * compiled blocks inserted by build-js.php
 */
var magma = magma || {};

magma.blocks = {
'alertContainer': '<div id="alert-box" class="dialog-box"><a class="close" href="#close">Close</a></div>',
'checkboxControls': '<div class="checkbox-controls"><li><a class="none" href="#check-none">None</a></li><li><a class="page" href="#check-page">Page</a></li><li><a class="all" href="#check-all">All</a></li></div>',
'chrome': '<div id="chrome"><h1 class="logo">Lavabit Webmail</h1><div id="account"><h2 class="sr-hidden">Account</h2><a class="sr-hidden" href="#sr-global-nav">Skip to global navigation</a><ul></ul></div><div id="global-nav"><h2 class="sr-hidden" id="sr-global-nav">Global Navigation</h2><ul><li><a class="mail" href="#mail">Mail</a></li><li><a class="contacts" href="#contacts">Contacts</a></li><li><a class="options" href="#options">Options</a></li><li><a class="help" href="#help">Help</a></li></ul></div></div>',
'chromeAlert': '<li><a class="alert" href="#alert">Alert</a></li>',
'chromeMeta': '<li><a class="meta" href="#info">Information</a></li>',
'composing': '<div class="composing-wrapper"><form name="composing" method="POST" action="#send"><div class="composing-header"><div class="field-wrapper"><div class="input-wrapper"><div class="input"><select id="from" name="from"></select></div></div><div class="label-wrapper"><label for="from">From:</label></div></div><div class="field-wrapper"><div class="input-wrapper"><div class="input text"><input type="text" id="to" name="to" /></div></div><div class="label-wrapper"><label for="to">To:</label></div></div><div class="field-wrapper"><div class="input-wrapper"><div class="input text"><input type="text" id="subject" name="subject" /></div></div><div class="label-wrapper"><label for="subject">Subject:</label></div></div></div><div class="composing-body"><label class="sr-hidden watermark" for="body">Type your message here...</label><textarea id="body" name="body"></textarea></div></form></div>',
'composingAttachments': '<div class="composing-attachments"><span class="file-wrapper"><input type="file" name="attachment" /><span class="browse">Browse</span></span><div><p>file1.ext</p><p class="size">20K</p><a href="#delete" class="delete">Delete</a></div><div><p>file2.ext</p><p class="size">40K</p><a href="#delete" class="delete">Delete</a></div></div>',
'composingBCC': '<div class="field-wrapper"><div class="input-wrapper"><div class="input text"><input type="text" id="bcc" name="bcc" /></div></div><div class="label-wrapper"><label for="bcc">BCC:</label></div></div>',
'composingCC': '<div class="field-wrapper"><div class="input-wrapper"><div class="input text"><input type="text" id="cc" name="cc" /></div></div><div class="label-wrapper"><label for="cc">CC:</label></div></div>',
'contactAddField': '<div class="add-field"><form action="#new-field"><label class="sr-hidden" for="contact-info-field-select">Field Select</label><select id="contact-info-field-select"><optgroup label="Email"><option value="alternate-1">Alternate 1</option><option value="alternate-2">Alternate 2</option></optgroup><optgroup label="Messaging"><option value="yahoo">Yahoo</option><option value="windows-live">Windows Live</option><option value="aol-aim">AOL/AIM</option><option value="google">Google</option><option value="ICQ">ICQ</option></optgroup><optgroup label="Contact"><option value="home">Home</option><option value="work">Work</option><option value="mobile">Mobile</option><option value="pager">Pager</option><option value="fax">Fax</option><option value="home-address">Home Address</option><option value="work-address">Work Address</option></optgroup></select><label class="sr-hidden" for="contacts-new-field">New Field</label><input id="contacts-new-field" type="text" /><input type="submit" value="Add" /></form></div>',
'contactInfo': '<div class="column-wrapper"><div class="contact-info"><a class="close" href="close-contact-info">Close</a></div></div>',
'contactsContainer': '<div class="view-wrapper"><div class="view"><h2 class="sr-hidden" id="sr-view">View</h2><div class="contacts-wrapper"><h3 class="sr-hidden">Contacts</h3><table class="contacts"><thead><tr><th><span class="checkbox-controls-button">✔</span></th><th>Gravatar</th><th>Name</th><th>Email</th><th>Company</th></tr></thead><tbody></tbody></table></div></div></div>',
'folderMenu': '<div class="folder-menu"><h2 class="sr-hidden" id="sr-folder-menu">Folder Menu</h2><div class="folder-scroll-wrapper"><ul class="folder-list"></ul></div></div>',
'folderMenuWithOptions': '<div class="folder-menu"><h2 class="sr-hidden" id="sr-folder-menu">Folder Menu</h2><div class="folder-scroll-wrapper options"><ul class="folder-list"></ul></div></div>',
'folderOptions': '<div class="folder-options"><h3 class="sr-hidden" id="sr-folder-options">Folder Options</h3><input id="add-folder" class="add" type="checkbox" /><label for="add-folder">Add Folder</label><input id="edit-folders" class="edit" type="checkbox" /><label for="edit-folders">Edit Folders</label></div>',
'folderOptionsAdd': '<form class="add-folder-box"><label for="folder-name-input">Name</label><input id="folder-name-input" name="folder-name-input" type="text" /><input class="simple-button" type="submit" value="Add" /></form>',
'folderOptionsEdit': '<div class="edit"><a class="rename" title="Rename folder" href="#rename">✎</a><a class="remove" title="Remove folder" href="#remove">✗</a></div>',
'folderOptionsRename': '<form class="rename-folder"><input type="text" /><input type="submit" title="Save name" value="✓" /></form>',
'help': '<div class="help-wrapper"><div class="help-page-wrapper"><div class="help-page"></div></div><div class="help-categories"><div><ul></ul></div></div><div class="help-topics"><div><ul></ul></div></div></div>',
'loadingContainer': '<div id="startup-box" class="loading"><h1 class="logo">Lavabit</h1></div>',
'loadingFinished': '<div id="loading-finished"><h2 class="finished-image">Finished Loading</h2><div><p>We have finished loading.</p><p>Click continue if you\'re not redirected.</p></div><a class="button" href="main.html">Continue</a></div>',
'loadingWaiting': '<div id="loading-waiting"><h2 class="loading-image">Loading...</h2><p>Please wait while we finish loading</p><a class="button disabled" href="loading.html">Loading</a></div>',
'loadingWarning': '<div id="loading-warning"><h2 class="white-logo">Warning</h2><div><p>Lavabit makes no representation whatsoever regarding the content of any other web site.</p><p>The ad you clicked will take you to the advertisers site. Is this okay?</p></div><a id="cancel" class="button cancel" href="#finished">Cancel</a></div>',
'lockedContainer': '<div id="startup-box" class="locked"><h1 class="logo">Lavabit</h1><div id="locked"><div><h2>Your account has been locked:</h2></div><a class="appeal button" href="#locked-appeal">Submit an appeal</a><a class="login button" href="/">Login</a><a class="home button" href="http://lavabit.com">Home Page</a></div></div>',
'loginContainer': '<div id="startup-box" class="login"><h1 class="logo">Lavabit</h1><div id="login"><h2 class="sr-hidden">Login</h2><a class="sr-hidden" href="#sr-registration">Skip to registration</a><form method="POST" action="#server"><label class="sr-hidden" for="username">Username</label><input type="text" id="username" name="username" /><label class="sr-hidden" for="password">Password</label><input type="password" id="password" name="password" autocomplete="off" /><input class="button" type="submit" value="Login"></form></div><div id="registration"><h2 class="sr-hidden" id="sr-registration">Registration</h2><div><h3>Need an Account?</h3><p>Our security measures is what makes us unique. See the various ways to protect your account.</p></div><a class="button" href="https://lavabit.com/apps/register">Register</a></div></div>',
'logoutContainer': '<div id="startup-box" class="locked"><h1 class="logo">Lavabit</h1><div id="logout"><h2>You have been logged out</h2><a class="login button" href="/">Login</a><a class="home button" href="http://lavabit.com">Home Page</a></div></div>',
'logs': '<div class="view-wrapper"><div class="view"><h2 class="sr-hidden" id="sr-view">View</h2><div class="settings"></div></div></div>',
'logsMail': '<table class="mail-log"><thead><tr><th>Queue</th><th>Type</th><th>From</th><th>To</th><th>Outcome</th><th>UTC</th><th>Time</th><th>Bytes</th><th>Size</th></tr></thead><tbody></tbody></table>',
'logsSecurity': '<table class="mail-log"><thead><tr><th>UTC</th><th>Time</th><th>Type</th><th>Severity</th><th>IP</th><th>Protocol</th></tr></thead><tbody></tbody></table>',
'main': '<div id="main-wrapper"></div>',
'messageAd': '<div class="message-ad"></div>',
'messageAttachments': '<div class="message-attachments"><a class="close" href="#close-attachments">Close</a></div>',
'messageBody': '<div class="message-body"></div>',
'messageHeader': '<div class="message-header"><div class="extras"><ul><li><a class="info" href="#info">Message Info</a></li><li><a class="scrape" href="#scrape-contacts">Scrape Contacts</a></li><li><a class="view-header" href="#view-source">View Full Header</a></li><li><a class="attachments" href="#attachments">Attachments</a></li></ul><a href="#profile"><img src="#profile" alt="Profile" /></a></div></div>',
'messageInfo': '<div class="message-info"><a class="close" href="#close-info">Close</a></div>',
'messageList': '<div class="view-wrapper"><div class="view"><h2 class="sr-hidden" id="sr-view">View</h2><div class="messages-wrapper"><h3 class="sr-hidden">Inbox</h3><table class="messages"><thead><tr><th><span class="checkbox-controls-button">✔</span></th><th>ID</th><th>Read</th><th>Replied</th><th>Starred</th><th>Attachment</th><th>From</th><th>To</th><th>Addressed To</th><th>Reply To</th><th>Return Path</th><th>Carbon</th><th>Subject</th><th>Unix Time</th><th>Date Received</th><th>Arrival Unix Time</th><th>Arrival Date</th><th>Bytes</th><th>Size</th><th>Snippet</th><th>Tags</th></tr></thead><tbody></tbody></table></div></div></div>',
'messageListEditing': '<div class="editing-overlay"></div>',
'meta': '<div id="meta-box" class="dialog-box"></div>',
'overlay': '<div class="overlay"></div>',
'previewPane': '<div class="preview-pane"></div>',
'quickSearch': '<div class="search"><h4 class="sr-hidden">Search</h4><a class="sr-hidden" href="#sr-tools">Skip to tools</a><form action="#search"><label class="sr-hidden watermark" for="search-input">Enter search your here.</label><input type="text" id="search-input" name="search-text" /><a class="gray button clear" href="#clear-search" title="Clear search">✖</a><input class="gray button" type="submit" name="search" value="Search" /></form><a class="advanced" href="#advanced">Advanced</a></div>',
'scrapeContacts': '<div class="scrape-contacts"><form><div><label for="contact-name">Name:</label><input type="text" id="contact-name" name="contact-name" /></div><div><label for="contact-email">Email:</label><input type="text" id="contact-email" name="contact-email" /></div><div class="next-row"><label for="contact-phone">Phone:</label><input type="text" id="contact-phone" name="contact-phone" /></div><div><label for="contact-work">Work:</label><input type="text" id="contact-work" name="contact-work" /></div><div><label for="contact-cell">Cell:</label><input type="text" id="contact-cell" name="contact-cell" /></div><input type="submit" class="button" value="Add" /></form></div>',
'scrapeContactsList': '<table class="contacts"><thead><tr><th>Name</th><th>Email</th></tr></thead><tbody></tbody></table>',
'search': '<div class="advanced-search"><form><div class="column-wrapper"><div class="middle search-options"></div><input class="button" type="submit" value="Search" /></div><div class="left column-wrapper"><label for="search-in">Search in:</label><select id="search-in" name="search-in"><option value="inbox">Inbox</option><option value="drafts">Drafts</option><option value="sent">Sent</option><option value="junk">Junk</option><option value="trash">Trash</option></select></div></form></div>',
'searchControlsAdd': '<a class="add button" href="#add">+</a>',
'searchControlsRemove': '<a class="remove button" href="#remove">-</a>',
'searchFilterDate': '<label class="sr-hidden" for="search-filter-date">Filter by date</label><select id="search-filter-date" name="search-filter-date"><option class="single" value="greater">Greater than</option><option class="single" value="less">Less than</option><option class="range" value="range">Range</option></select>',
'searchFilterDateRange': '<label class="sr-hidden" for="search-from-date">From date</label><input type="text" id="search-from-date" name="search-from-date" /><label class="sr-hidden" for="search-to-date">To date</label><input type="text" id="search-to-date" name="search-to-date" />',
'searchFilterDateSingle': '<label class="sr-hidden" for="search-date">Date</label><input type="text" id="search-date" name="search-date" />',
'searchFilterSize': '<label class="sr-hidden" for="search-filter-size">Filter by size</label><select id="search-filter-size" name="search-filter-size"><option class="single" value="greater">Greater than</option><option class="single" value="less">Less than</option><option class="range" value="range">Range</option></select>',
'searchFilterSizeRange': '<label class="sr-hidden" for="search-from-size">Enter a size...</label><input type="text" id="search-from-size" name="search-from-size" /><label class="sr-hidden" for="search-from-size-unit">Filter by size</label><select id="search-from-size-unit" name="search-from-size-unit"><option value="kb">KB</option><option value="kb">MB</option><option value="kb">GB</option></select><label class="sr-hidden" for="search-to-size">Enter a size...</label><input type="text" id="search-to-size" name="search-to-size" /><label class="sr-hidden" for="search-to-size-unit">Filter by size</label><select id="search-to-size-unit" name="search-to-size-unit"><option value="kb">KB</option><option value="kb">MB</option><option value="kb">GB</option></select>',
'searchFilterSizeSingle': '<label class="sr-hidden" for="search-size">Enter a size...</label><input type="text" id="search-size" name="search-size" /><label class="sr-hidden" for="search-size-unit">Filter by size</label><select id="search-size-unit" name="search-size-unit"><option value="kb">KB</option><option value="kb">MB</option><option value="kb">GB</option></select>',
'searchFilterString': '<label class="sr-hidden" for="search-filter">Filter by string</label><select id="search-filter" name="search-filter"><option value="contains">Contains</option><option value="not-containt">Does not contain</option></select><label class="sr-hidden" for="search-filter-string">Enter your search here...</label><input type="text" id="search-filter-string" name="search-filter-string" />',
'searchOption': '<div class="search-option"><div class="search-field"><label class="sr-hidden" for="search-field">Search field</label><select id="search-field" name="search-field"><option class="string" value="to">To</option><option class="string" value="from">From</option><option class="string" value="subject">Subject</option><option class="date" value="date">Date</option><option class="size" value="size">Size</option></select></div><div class="search-filter"></div><div class="search-controls"></div></div>',
'searchResultsList': '<div class="search-results"><table class="results messages"><thead><tr><th><span class="checkbox-controls-button">✔</span></th><th>ID</th><th>Read</th><th>Replied</th><th>Starred</th><th>Attachment</th><th>From</th><th>To</th><th>Addressed To</th><th>Reply To</th><th>Return Path</th><th>Carbon</th><th>Subject</th><th>Unix Time</th><th>Date Received</th><th>Arrival Unix Time</th><th>Arrival Date</th><th>Bytes</th><th>Size</th><th>Snippet</th><th>Tags</th></tr></thead><tbody></tbody></table></div>',
'searchResultsProgress': '<div class="progress"></div>',
'settings': '<div class="view-wrapper"><div class="view"><h2 class="sr-hidden" id="sr-view">View</h2><div class="settings"></div></div></div>',
'settingsIdentityAddField': '<form class="add-field" action="#new-field">	<label class="sr-hidden" for="identity-field-select">Field Select</label>	<select id="contact-info-field-select"><option value="signature">Signature</option><option value="website">Website</option>	</select><label class="sr-hidden" for="contacts-new-field">New Field</label><input id="identity-new-field" type="text" /><input type="submit" value="Add" /></form>',
'startup': '<div id="startup-wrapper"><div class="vertical-wrapper"><div class="vertical-align"></div></div></div>',
'tabs': '<div id="tabs"><h2 class="sr-hidden" id="sr-tabs">Tabs</h2></div>',
'toolOptionsContainer': '<div class="tool-options tools"><h4 class="sr-hidden" id="sr-tool-options">Tool Options</h4></div>',
'toolOptionsMark': '<ul><li><a class="read" href="#read">Read</a></li><li><a class="unread" href="#unread">Unread</a></li><li><a class="star" href="#star">Star</a></li><li><a class="unstar" href="#unstar">Unstar</a></li></ul>',
'toolOptionsNarrow': '<ul class="no-icons"><li><a class="all" href="#all">All</a></li><li><a class="0-9" href="#0-9">0-9</a></li><li><a class="a" href="#a">A</a></li><li><a class="b" href="#b">B</a></li><li><a class="c" href="#c">C</a></li><li><a class="d" href="#d">D</a></li><li><a class="e" href="#e">E</a></li><li><a class="f" href="#f">F</a></li><li><a class="g" href="#g">G</a></li><li><a class="h" href="#h">H</a></li><li><a class="i" href="#i">I</a></li><li><a class="j" href="#j">J</a></li><li><a class="k" href="#k">K</a></li><li><a class="l" href="#l">L</a></li><li><a class="m" href="#m">M</a></li><li><a class="n" href="#n">N</a></li><li><a class="o" href="#o">O</a></li><li><a class="p" href="#p">P</a></li><li><a class="q" href="#q">Q</a></li><li><a class="r" href="#r">R</a></li><li><a class="s" href="#s">S</a></li><li><a class="t" href="#t">T</a></li><li><a class="u" href="#u">U</a></li><li><a class="v" href="#v">V</a></li><li><a class="w" href="#w">W</a></li><li><a class="x" href="#x">X</a></li><li><a class="y" href="#y">Y</a></li><li><a class="z" href="#z">Z</a></li></ul>',
'toolOptionsSort': '<ul class="no-icons"><li><a class="first-name" href="#first-name">First Name</a></li><li><a class="last-name" href="#last-name">Last Name</a></li><li><a class="email" href="#email">Email</a></li><li><a class="company" href="#company">Company</a></li></ul>',
'toolOptionsTags': '<div class="right"><ul class="add-tag no-icons"><li><a class="add" href="#add">+</a></li></ul><ul class="tags"></ul></div>',
'toolbar': '<div class="toolbar"><h3 class="sr-hidden">Toolbar</h3></div>',
'tools': '<div class="tools"><h4 class="sr-hidden" id="sr-tools">Tools</h4></div>',
'toolsCancelSave': '<div class="span"><ul class="left"><li><a class="cancel" href="#cancel">Cancel</a></li></ul><ul class="right"><li><a class="save" href="#save">Save</a></li></ul></div>',
'toolsComposing': '<div class="tools"><h4 class="sr-hidden" id="sr-tools">Tools</h4><div class="span"><ul class="left"><li><a class="cancel" href="#cancel">Cancel</a></li></ul><ul class="right"><li><input id="cc" class="toggle" type="checkbox" /><label for="cc">CC</label></li><li><input id="bcc" class="toggle" type="checkbox" /><label for="bcc">BCC</label></li><li><input id="attach" class="toggle" type="checkbox" /><label for="attach">Attach</label></li><li><a class="save" href="#save">Save</a></li><li><a class="send" href="#send">Send</a></li></ul></div></div>',
'toolsContacts': '<ul><li><a class="new" href="#new">New</a></li><li>	<input id="sort" class="dropdown" type="checkbox" />	<label for="sort">Sort</label></li><li>	<input id="narrow" class="dropdown" type="checkbox" />	<label for="narrow">Narrow</label></li><li>	<input id="copy" class="dropdown" type="checkbox" />	<label for="copy">Copy</label></li><li>	<input id="move" class="dropdown" type="checkbox" />	<label for="move">Move</label></li><li class="disabled">	<input id="edit" class="toggle" type="checkbox" />	<label for="edit">Edit</label></li><li><a class="delete" href="#delete">Delete</a></li></ul>',
'toolsDefault': '<ul><li><a href="#new" class="new">New</a></li><li><a href="#print" class="print">Print</a></li><li><a href="#reply" class="reply">Reply</a></li><li><a href="#reply-all" class="reply-all">Reply All</a></li><li><a href="#forward" class="forward">Forward</a></li><li><a href="#delete" class="delete">Delete</a></li><li><a href="#junk" class="junk">Junk</a></li><li>	<input id="copy" class="dropdown" type="checkbox" />	<label for="copy">Copy</label></li><li>	<input id="move" class="dropdown" type="checkbox" />	<label for="move">Move</label></li><li>	<input id="mark" class="dropdown" type="checkbox" />	<label for="mark">Mark</label></li><li>	<input id="tags" class="dropdown" type="checkbox" />	<label for="tags">Tags</label></li><li>	<input id="display" class="toggle" type="checkbox" />	<label for="display">Display</label></li></ul>',
'toolsSettings': '<ul><li>	<input id="advanced" class="toggle" type="checkbox" />	<label for="advanced">Advanced</label></li><li>	<input id="defaults" class="toggle" type="checkbox" />	<label for="defaults">Defaults</label></li><li><a class="edit" href="#edit">Edit</a></li></ul>',
'workspaceContainer': '<div class="workspace-wrapper active"><div class="workspace"></div></div>',
'workspacesContainer': '<div id="workspaces-wrapper"></div>'
};
/**
 * bootstrap.js
 *
 * Loads MVCs
 */
var magma = magma || {};

magma.bootstrap = (function() {
    // private vars and mothods

    // public methods
    return {
        // displays login dialog
        login: function() {
            var authModel = magma.model.auth(),
                container = magma.view.login(authModel).root;

            $('#startup-box').replaceWith(container).reveal();
        },

        // locked message must be provided by login
        locked: function(message) {
            if(!message) {
                throw new Error('locked message not provided to bootstrap!');
            }

            var container = magma.view.locked.container();
            $('#startup-box').replaceWith(container.root).reveal();

            magma.view.locked.message(message).root.insertAfter('#locked h2');
        },

        logout: function() {
            $('body').append(magma.view.container('startup').root);

            var container = magma.view.logout.container();
            $('.vertical-align').append(container.root).reveal();
        },

        loading: function() {
            var loadingModel = magma.model.loading(),
                adModel = magma.model.ad();

            // global models that will be used by inbox
            magma.folderModel = magma.model.folders();
            magma.messageListModel = magma.model.messageList();

            var loadingContainer = magma.view.loading.container(loadingModel);
            $('#startup-box').replaceWith(loadingContainer.root).reveal();

            magma.view.loading.waiting(loadingModel).root.appendTo('#startup-box').reveal();

            magma.controller.loading().loadAd(adModel);

            loadingModel.registerItem(function(finished) {
                magma.folderModel.observeOnce('loaded', function() {
                    // load messages after folders so inbox folderID is known
                    magma.messageListModel.observeOnce('loaded', function() {
                        finished();
                    });

                    magma.messageListModel.load('inbox');
                });

                magma.folderModel.loadFolders('mail');
            });

            loadingModel.begin();
        },

        // adModel must be provided by loading
        loadingFact: function(adModel) {
            magma.view.loading.fact(adModel, function(fact) {
                fact.root.appendTo('#startup-box').reveal();
            });
        },

        loadingAd: function(adModel) {
            magma.view.loading.ad(adModel, function(ad) {
                ad.root.appendTo('#startup-box').reveal();
            });

            magma.view.loading.adWarning(adModel).root.appendTo('#startup-box').css('display', 'none');
        },

        loadingFinished: function(loadingModel) {
            if(!loadingModel) {
                throw new Error('loadingModel not provided to laodingFinished bootstrap!');
            }

            magma.view.loading.finished(loadingModel).root.appendTo('#startup-box').reveal();
        },

        // initializes chrome and workspace
        main: function() {
            var main = magma.view.container('main').root.appendTo('body');
            magma.globalNavModel = magma.model.globalNav();
            var chrome = magma.view.chrome(magma.globalNavModel).root.appendTo(main);

            var metaModel = magma.model.meta();
            magma.view.meta.box(metaModel).setLocation(main);
            magma.view.meta.link(metaModel).root.appendTo(chrome.find('#account ul'));

            var alertModel = magma.model.alrt();
            magma.view.alrt.box(alertModel).setLocation(main);
            magma.view.alrt.link(alertModel).root.appendTo(chrome.find('#account ul'));

            var authModel = magma.model.auth();
            magma.view.logout.link(authModel).root.appendTo(chrome.find('#account ul'));

            // create global workspaces and tabs containers
            magma.tabsModel = magma.model.tabs();
            magma.tabsContainer = magma.view.tabs(magma.tabsModel).root.appendTo(main);
            magma.workspacesModel = magma.model.workspaces();
            magma.workspacesContainer = magma.view.workspaces(magma.workspacesModel, magma.tabsModel).root.appendTo(main);

            // start with the inbox
            magma.bootstrap.inbox();
        },

        inbox: function() {
            // create inbox models
            var workspaceModel = magma.model.workspace(),
                tabModel = magma.model.tab('Inbox', workspaceModel.getID(), true),
                toolsModel = magma.model.tools();

            // create workspace view
            var workspaceView = magma.view.workspace(workspaceModel, tabModel, toolsModel);

            // add folders
            // use global folder and message list model
            magma.view.folders(magma.folderModel, magma.messageListModel, workspaceModel.getID()).root.appendTo(workspaceView.workspace);

            // add toolbar
            var toolbar = magma.view.container('toolbar').root.prependTo(workspaceView.root);
            var quickSearch = magma.view.quickSearch(workspaceModel.getID()).root.appendTo(toolbar);

            // add tools
            magma.view.tools(toolsModel, workspaceModel, magma.folderModel).root.appendTo(toolbar);

            // list messages
            var messageListView = magma.view.messageList(magma.messageListModel, magma.folderModel, toolsModel, quickSearch, workspaceModel.getID());
            messageListView.root.prependTo(workspaceView.workspace);

            // add workspace to workspaces container
            workspaceView.root.appendTo(magma.workspacesContainer);
            magma.workspacesModel.addWorkspace(workspaceModel);

            // add inbox tab to the tabs container
            magma.view.tab(tabModel, function(tabView) {
                tabView.root.appendTo(magma.tabsContainer);
                magma.tabsModel.addTab(tabModel);
            });
        },

        read: function(messageID) {
            if(typeof messageID !== 'number') {
                throw new Error('bootstrap.read: must provide a messageID');
            }

            var workspaceModel = magma.model.workspace();
            // title set with header subject below
            var tabModel = magma.model.tab('No subject', workspaceModel.getID());
            var toolsModel = magma.model.tools();

            var workspaceView = magma.view.workspace(workspaceModel, tabModel, toolsModel);

            // reading tools
            var toolbar = magma.view.container('toolbar').root.prependTo(workspaceView.root);

            // add tools
            magma.view.tools(toolsModel, workspaceModel).root.appendTo(toolbar);

            // pass message model to views
            var messageModel = magma.model.message(messageID);

            // message section views
            magma.view.message.header(messageModel).root.appendTo(workspaceView.workspace);
            magma.view.message.body(messageModel).root.appendTo(workspaceView.workspace);

            // message ad
            var adModel = magma.model.ad();
            magma.view.message.ad(adModel).root.appendTo(workspaceView.workspace);

            // show message once loaded
            messageModel.observeOnce('messageLoaded', function() {
                if(messageModel.hasField('header', 'subject')) {
                    tabModel.setTitle(messageModel.getField('header', 'subject'));
                }

                // add workspace to the view
                workspaceView.root.appendTo(magma.workspacesContainer);
                magma.workspacesModel.addWorkspace(workspaceModel);

                // add tab to the view
                magma.view.tab(tabModel, function(tabView) {
                    tabView.root.appendTo(magma.tabsContainer);
                    magma.tabsModel.addTab(tabModel);
                });
            });

            // load message sections
            messageModel.loadSections(['header', 'body']);
        },

        compose: function(composeModel) {
            var workspaceModel = magma.model.workspace();
            // title set with header subject below
            var tabModel = magma.model.tab('Subject...', workspaceModel.getID());
            var toolsModel = magma.model.tools();

            var workspaceView = magma.view.workspace(workspaceModel, tabModel, toolsModel);

            // compose tools
            var toolbar = magma.view.container('toolbar').root.prependTo(workspaceView.root);
            magma.view.tools(toolsModel, workspaceModel, null, 'composing').root.appendTo(toolbar);

            // compose view
            magma.view.compose(composeModel, toolsModel, tabModel).root.appendTo(workspaceView.workspace);

            // add workspace to the view
            workspaceView.root.appendTo(magma.workspacesContainer);
            magma.workspacesModel.addWorkspace(workspaceModel);

            // init ckeditor
            magma.ckeditor('body-' + composeModel.getComposeID());

            // add tab to the view
            magma.view.tab(tabModel, function(tabView) {
                tabView.root.appendTo(magma.tabsContainer);
                magma.tabsModel.addTab(tabModel);
            });
        },

        mail: function() {
            if(!magma.tabsModel.isOpen()) {
                magma.tabsModel.showTabs();
                magma.workspacesModel.focusLastActive();
            }
        },

        contacts: function() {
            var workspaceModel = magma.model.workspace();
            var toolsModel = magma.model.tools();

            var workspaceView = magma.view.workspace(workspaceModel, null, toolsModel);
            // folder model shared between folder list and tools
            var folderModel = magma.model.folders('contacts');

            // add toolbar
            var toolbar = magma.view.container('toolbar').root.prependTo(workspaceView.root);
            var quickSearch = magma.view.quickSearch(workspaceModel.getID(), false).root.appendTo(toolbar);
            magma.view.tools(toolsModel, workspaceModel, folderModel, 'contacts').root.appendTo(toolbar);

            // list contacts
            var contactListModel = magma.model.contactList();

            // add folders
            magma.view.folders(folderModel, contactListModel, workspaceModel.getID(), true, function() {
                // add contacts to view
                var contactListView = magma.view.contactList(contactListModel, toolsModel, quickSearch, workspaceModel.getID()).root.prependTo(workspaceView.workspace);
            }).root.appendTo(workspaceView.workspace);

            if(magma.tabsModel.isOpen()) {
                magma.tabsModel.hideTabs();
            }

            // add workspace to workspaces container
            workspaceView.root.appendTo(magma.workspacesContainer);
            magma.workspacesModel.addDedicatedWorkspace(workspaceModel);
        },

        options: function() {
            var workspaceModel = magma.model.workspace();
            var toolsModel = magma.model.tools();

            var workspaceView = magma.view.workspace(workspaceModel, null, toolsModel);

            // add toolbar
            var toolbar = magma.view.container('toolbar').root.prependTo(workspaceView.root);
            magma.view.tools(toolsModel, workspaceModel, null, 'settings').root.appendTo(toolbar);

            // settings
            var settingsModel = magma.model.settings();

            // add folders
            var folderModel = magma.model.folders('settings');
            magma.view.folders(folderModel, settingsModel, workspaceModel.getID(), false, null, false).root.appendTo(workspaceView.workspace);
            magma.view.settings(settingsModel, toolsModel, folderModel).root.prependTo(workspaceView.workspace);

            if(magma.tabsModel.isOpen()) {
                magma.tabsModel.hideTabs();
            }

            // add workspace to workspaces container
            workspaceView.root.appendTo(magma.workspacesContainer);
            magma.workspacesModel.addDedicatedWorkspace(workspaceModel);
        },

        logs: function(initialize) {
            initialize = initialize || 'mail';

            if(typeof initialize !== 'string') {
                throw new Error('bootstrap.logs: initialize must be a string');
            }

            var workspaceModel = magma.model.workspace();
            var workspaceView = magma.view.workspace(workspaceModel);

            // settings
            var logsModel = magma.model.logs();

            // add folders
            var folderModel = magma.model.folders('logs');
            magma.view.folders(folderModel, logsModel, workspaceModel.getID(), false, null, false).root.appendTo(workspaceView.workspace);
            magma.view.logs(logsModel, folderModel, initialize).root.prependTo(workspaceView.workspace);

            // no tools
            workspaceView.workspace.addClass('no-tools');

            if(magma.tabsModel.isOpen()) {
                magma.tabsModel.hideTabs();
            }

            // add workspace to workspaces container
            workspaceView.root.appendTo(magma.workspacesContainer);
            magma.workspacesModel.addDedicatedWorkspace(workspaceModel);
        },

        help: function() {
            if(magma.tabsModel.isOpen()) {
                magma.tabsModel.hideTabs();
            }

            var workspaceModel = magma.model.workspace();
            var workspaceView = magma.view.workspace(workspaceModel);

            var helpModel = magma.model.help();
            magma.view.help(helpModel).root.appendTo(workspaceView.workspace);

            // init help page with first category / topic / page
            helpModel.initHelp();

            // no tools
            workspaceView.workspace.addClass('no-tools');

            // add workspace to workspaces container
            workspaceView.root.appendTo(magma.workspacesContainer);
            magma.workspacesModel.addDedicatedWorkspace(workspaceModel);
        },

        search: function() {
            var workspaceModel = magma.model.workspace();
            var tabModel = magma.model.tab('Advanced Search', workspaceModel.getID());

            var workspaceView = magma.view.workspace(workspaceModel, tabModel);

            // results model
            var searchResultsModel = magma.model.searchResults();

            // search form
            var searchOptionsModel = magma.model.searchOptions();
            magma.view.searchOptions(searchOptionsModel, searchResultsModel, workspaceModel.getID()).root.appendTo(workspaceView.workspace);

            // results view
            magma.view.searchResults(searchResultsModel, workspaceModel.getID()).root.appendTo(workspaceView.workspace);

            // no tools
            workspaceView.workspace.addClass('no-tools');

            // add workspace to the view
            workspaceView.root.appendTo(magma.workspacesContainer);
            magma.workspacesModel.addWorkspace(workspaceModel);

            // add tab to the view
            magma.view.tab(tabModel, function(tabView) {
                tabView.root.appendTo(magma.tabsContainer);
                magma.tabsModel.addTab(tabModel);
            });
        },

        scrape: function(messageID) {
            var workspaceModel = magma.model.workspace();
            var tabModel = magma.model.tab('Scrape Contacts', workspaceModel.getID());

            var workspaceView = magma.view.workspace(workspaceModel, tabModel);

            // scrape model
            var scrapeContactsModel = magma.model.scrape(messageID);
            magma.view.scrapeContacts(scrapeContactsModel, workspaceModel.getID()).root.appendTo(workspaceView.workspace);

            // no tools
            workspaceView.workspace.addClass('no-tools');

            // add workspace to the view
            workspaceView.root.appendTo(magma.workspacesContainer);
            magma.workspacesModel.addWorkspace(workspaceModel);

            // add tab to the view
            magma.view.tab(tabModel, function(tabView) {
                tabView.root.appendTo(magma.tabsContainer);
                magma.tabsModel.addTab(tabModel);
            });
        }
    };
}());
var magma = magma || {};

/*
 * initialize ckeditor
 * loads the ckeditor script on first init
 *
 * @param textareaID    id of textarea html element
 */
magma.ckeditor = function(textareaID) {
    var editorPath = '/ckeditor/',
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
/**
 * magma.js
 *
 * Provide any constants here
 *
 * This script must be inserted before any method definitions!
 * Handled in build-js.php
 */
var magma = magma || {};

magma.animationSpeed = "fast";
magma.portalUrl = false ? '/portal/camel' : '/portal/mockiface';

// TODO: better way to get table header height
// 27px + 1px border
magma.tableHeaderHeight = 28;
/**
 * controller.js
 *
 * Place any logic here that doesn't fit easily into view
 */
var magma = magma || {};

magma.controller = (function() {
    // private vars and methods

    // public methods
    return {
        /*** loading ***/
        loading: function() {
            var loadAd = function(adModel) {
                adModel.observeOnce('loaded', function() {
                    if(adModel.hasFact()) {
                        magma.bootstrap.loadingFact(adModel);
                    } else if(adModel.hasAd()) {
                        magma.bootstrap.loadingAd(adModel);
                    }
                });

                var fallback = function() {
                    // log or post error

                    if(adModel.hasFact()) {
                        magma.bootstrap.loadingFact(adModel);
                    } else if(adModel.hasAd()) {
                        magma.bootstrap.loadingAd(adModel);
                    }
                };

                adModel.observeOnce('loadedError', fallback);
                adModel.observeOnce('loadedFailed', fallback);

                adModel.loadAd('loading');
            };

            return {
                loadAd: loadAd
            };
        }
    };
}());
/*
 * prepulated data for fallbacks
 */
var magma = magma || {};

magma.fallback = {
    ad: {
        loading: {
            "href": "/advertisement.html",
            "title": "Loading Ad",
            "img": {
                "src": "http://placehold.it/300x250&text=fallback+ad",
                "alt": "Loading Ad"
            }
        }
    }
};
/**
 * model.js
 *
 * Contains all models for magma
 */
var magma = magma || {};

magma.model = (function() {
    // private variables and methods

    // share permanent folder IDs
    var permaFolders = (function() {
        var folders = {},
            names = {
                mail: [
                    'inbox',
                    'drafts',
                    'sent',
                    'junk',
                    'trash'
                ],
                contacts: [
                    'all',
                    'people',
                    'business',
                    'collected'
                ],
                settings: [
                    'identity',
                    'mail-settings',
                    'portal-settings',
                    'account-upgrades',
                    'password'
                ],
                logs: [
                    'statistics',
                    'security',
                    'contacts',
                    'mail'
                ]
            };

        return {
            getNames: function(context) {
                if(!names[context]) {
                    throw new Error('permaFolders.getNames: no context ' + context);
                }
                return names[context];
            },
            getName: function(id) {
                for(var name in folders) {
                    if(folders[name] === id) {
                        return name;
                    }
                }
                throw new Error('permaFolders.getName: no folder with id ' + id);
            },
            getID: function(name) {
                if(typeof folders[name] === 'number') {
                    return folders[name];
                }
                throw new Error('getFolderID: no folder named ' + name);
            },
            setID: function(name, folderID) {
                folders[name] = folderID;
            }
        };
    }());

    /*
     * Loads data from server
     *
     * @param method            JSON method name
     * @param params            method parameters
     * @param callbacks         object containing callbacks
     */
    var getData = (function() {
        var ID = 0;

        return function(method, params, callbacks) {
            ID += 1;

            var session = magma.session.get(),
                data = $.extend({method: method, id: ID}, params ? {params: params} : {});

            $.ajax({
                type: 'POST',
                url: magma.portalUrl + (session ? '/' + session + '/' : ''),
                dataType: 'json',
                contentType: 'application/json',
                cache: false,
                processDate: false,
                data: JSON.stringify(data),
                success: function(data) {
                    if(data.result) {
                        if(callbacks.success) {
                            callbacks.success(data.result);
                        }
                    } else if(data.error) {
                        if(callbacks.error) {
                            callbacks.error(data.error);
                        }
                    }
                },
                error: function(xhr) {
                    if(xhr.status !== 0 && callbacks.error) {
                        callbacks.error(xhr.responseText);
                    } else if(callbacks.fatal) {
                        callbacks.fatal();
                    } else {
                        magma.dialog.die("Could not connect to server!");
                    }
                }
            });
        };
    }());

    /**
     * Loads the jquery template if it's not cached and handles composite template calls
     *
     * @param template string   template name
     * @param callback function callback to be called on load
     */
    var loadTmpl = function(template, callback) {
        if(!template) {
            throw new Error('loadTmpl must be provided a template to load!');
        }

        // check if template is cached
        if($.template[template]) {
            if(callback) {
                callback();
            }

        // load the template
        } else {
            // make sure the template exists
            if(!magma.tmpl[template]) {
                throw new Error('loadTmpl: no tmpl named ' + template + '. this may have occured from an embedded tmpl call.');
            }

            // compile the template
            $.template(template, magma.tmpl[template]);

            // look for references to other templates
            var matches = magma.tmpl[template].match(/\{\{tmpl[\(]?[^\)]*[\)]?[^"]+"[^"]+"\}\}/g);

            // may need to load before issuing callback
            if(matches) {
                var i = 0;
                while(i < matches.length) {
                    // grab last string which is the template reference
                    matches[i] = matches[i].match(/"[^"]+"/g).pop().replace(/"/g,'');

                    // can have recursive templates so check if already loaded
                    if(!$.template[matches[i]]) {
                        loadTmpl(matches[i]);
                        i += 1;
                    } else {
                        // remove element since self reference
                        matches.splice(i,1);
                    }
                }

                // delay callback until dependancies loaded
                if(matches.length && callback) {
                    $.each(matches, function(i, tmplname) {
                        // pull for existance of template
                        var interval = setInterval(function() {
                            if($.template[tmplname]) {
                                clearInterval(interval); // stop pulling
                                matches.splice(i,1); // remove match
                                if(!matches.length) {
                                    callback();
                                }
                            }
                        }, 10);
                    });

                // all matches were recursive and removed
                } else if(callback) {
                    callback();
                }

            // no matches, go ahead and fill template if callback
            } else if(callback) {
                callback();
            }
        }
    };

    /**
     * Convert number of bytes into human readable format
     *
     * @source https://github.com/codeaid/snippets/blob/master/js/functions/bytesToSize.js
     *
     * @param integer bytes Number of bytes to convert
     * @param integer precision Number of digits after the decimal separator
     * @return string
     */
    var bytesToSize = function(bytes, precision) {
        var kilobyte = 1024;
        var megabyte = kilobyte * 1024;
        var gigabyte = megabyte * 1024;
        var terabyte = gigabyte * 1024;

        if ((bytes >= 0) && (bytes < kilobyte)) {
            return bytes + ' B';
        } else if ((bytes >= kilobyte) && (bytes < megabyte)) {
            return (bytes / kilobyte).toFixed(precision) + ' KB';
        } else if ((bytes >= megabyte) && (bytes < gigabyte)) {
            return (bytes / megabyte).toFixed(precision) + ' MB';
        } else if ((bytes >= gigabyte) && (bytes < terabyte)) {
            return (bytes / gigabyte).toFixed(precision) + ' GB';
        } else if (bytes >= terabyte) {
            return (bytes / terabyte).toFixed(precision) + ' TB';
        } else {
            return bytes + ' B';
        }
    };

    /**
     * Transform to lower case and replace spaces with hyphens
     *
     * @param str   string to slugify
     */
    var slugify = function(str) {
        return str.toLowerCase().replace(/ /,'-');
    };

    /**
     * Transform to camel case
     *
     * @param str   string to convert to camel case
     */
    var camelCasify = function(str) {
        return str.toLowerCase().replace(/[ -_]+([a-z])/, function(s, p1) {
            return p1.toUpperCase();
        });
    };

    /**
     * Sort an array of strings ignoring case
     * Pass this to the sort method
     */
    var ignoreCaseSort = function(a, b) {
        if(a.toLowerCase() === b.toLowerCase()) {
            return 0;
        }
        return a.toLowerCase() > b.toLowerCase() ? 1 : -1;
    };

    /**
     * Sort an array of strings ignoring case
     * Pass this to the sort method
     */
    var ignoreCaseSortName = function(a, b) {
        if(a.name.toLowerCase() === b.name.toLowerCase()) {
            return 0;
        }
        return a.name.toLowerCase() > b.name.toLowerCase() ? 1 : -1;
    };

    /**
     * Create success, error, and failed events from a list of events
     *
     * @param eventList     array of events
     */
    var listEvents = function(eventList) {
        var events = [];

        for(var i in eventList) {
            events.push(eventList[i]);
            events.push(eventList[i] + 'Error');
            events.push(eventList[i] + 'Failed');
        }

        return events;
    };

    /**
     * Provides a way for model to notify subscribers of changes
     *
     * @param events    event names that subscribers will subscribe to
     *
     * @source http://michaux.ca/articles/mvc-to-do-application
     */
    var newObservable = function(events) {
        var groups = {},
            addEvent,
            addObserver,
            observeOnce,
            removeObserver,
            notifyObservers;

        events = events || [];

        addEvent = function(event) {
            if (groups.hasOwnProperty(event)) {
                throw new Error('addEvent: Already an event named "' + event + '"');
            }
            groups[event] = [];
        };

        addObserver = function(event, observer) {
            if (!groups.hasOwnProperty(event)) {
                throw new Error('addObserver: No event "' + event + '".');
            }
            var group = groups[event];
            for (var i=0, ilen=group.length; i<ilen; i += 1) {
                if (group[i] === observer) {
                    throw new Error('Cannot add the same listener more than once.');
                }
            }
            group.push(observer);
        };

        observeOnce = function(event, observe) {
            var observer = function(data) {
                observe(data);
                removeObserver(event, observer);
            };

            addObserver(event, observer);
        };

        removeObserver = function(event, observer) {
            if (!groups.hasOwnProperty(event)) {
                throw new Error('removeObserver: No event "' + event + '".');
            }
            var group = groups[event];
            for (var i=0, ilen=group.length; i<ilen; i += 1) {
                if (group[i] === observer) {
                    group.splice(i, 1);
                    return;
                }
            }
            throw new Error('removeObserver: Did not find the observer and so could not remove it.');
        };

        notifyObservers = function(event, data) {
            if (!groups.hasOwnProperty(event)) {
                throw new Error('notifyObservers: No event "' + event + '".');
            }
            var group = groups[event];
            for (var i=0, ilen=group.length; i<ilen; i += 1) {
                group[i](data);
            }
        };

        // initialize
        for (var i=0; i<events.length; i += 1) {
            var event = events[i];
            addEvent(event);
        }

        return {
            addEvent: addEvent,
            addObserver: addObserver,
            observeOnce: observeOnce,
            removeObserver: removeObserver,
            notifyObservers: notifyObservers
        };
    };

    var addModel = function(params) {
        // make sure all the necessary params are added
        (function(properties) {
            for(var i in properties) {
                if(!params[properties[i]]) {
                    throw new Error('addModel: missing parameter ' + properties[i]);
                }
            }
        }(['method', 'models', 'observable', 'event']));

        return function(model) {
            if(!model) {
                throw new Error(params.method + ': model must be provided');
            }

            for(var i in params.models) {
                if(model === params.models[i]) {
                    throw new Error(params.method + ': cannot add the same model twice');
                }
            }

            params.models.push(model);
            params.observable.notifyObservers(params.event, model);
        };
    };

    var removeModel = function(params) {
        // make sure all the necessary params are present
        (function(properties) {
            for(var i in properties) {
                if(!params[properties[i]]) {
                    throw new Error('removeModel: missing parameter ' + properties[i]);
                }
            }
        }(['method', 'models', 'observable', 'event']));

        return function(model) {
            if(!model) {
                throw new Error(params.method + ': no model given.');
            }

            for(var i in params.models) {
                if(model === params.models[i]) {
                    params.models.splice(i, 1);
                    params.observable.notifyObservers(params.event, model);
                    return;
                }
            }

            throw new Error(params.method + ': model not in list saved models.');
        };
    };

    // models
    return {
        /*** ad ***/
        ad: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['clicked'])),
                ad,
                fact;

            var load = function(context) {
                context = context || 'message';

                getData('ad', {context: context}, {
                    success: function(data) {
                        ad = data.ad;
                        fact = data.fact;

                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        ad = magma.fallback.ad[context];
                        observable.notifyObservers('loadedError');
                    },
                    fatal: function() {
                        ad = magma.fallback.ad[context];
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            var getProperty = function(obj, property) {
                if(obj !== ad && obj !== fact) {
                    throw new Error('getProperty: must provide ad or fact object.');
                }

                if(obj) {
                    if(obj.hasOwnProperty(property)) {
                        return obj[property];
                    } else {
                        throw new Error('getProperty: no property ' + property + '.');
                    }
                }
            };

            return {
                loadAd: load,
                hasAd: function() {
                    return ad ? true : false;
                },
                hasFact: function() {
                    return fact ? true : false;
                },
                getAd: function() {
                    if(!ad) {
                        throw new Error('getAd: no ad to get!');
                    }
                    return ad;
                },
                getFact: function() {
                    if(!fact) {
                        throw new Error('getFact: no fact to get!');
                    }
                    return fact;
                },
                getAdHref: function() {
                    return getProperty(ad, 'href');
                },
                clicked: function() {
                    observable.notifyObservers('clicked');
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** alrt ***/
        alrt: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['open'])),
                open = false;

            var get = function() {
                getData('alert.list', null, {
                    success: function(data) {
                        // group alerts by type
                        var groups = {};

                        $.each(data, function() {
                            if(!groups[this.type]) {
                                groups[this.type] = [];
                            }

                            groups[this.type].push({
                                alertID: this.alertID,
                                message: this.message,
                                date: this.date
                            });
                        });

                        observable.notifyObservers('loaded', groups);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failed: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                getAlerts: get,
                open: function() {
                    observable.notifyObservers('open');
                },
                isOpen: function() {
                    return open;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** aliases ***/
        aliases: function() {
            var observable = newObservable(listEvents(['loaded']));

            var aliasesToOptions = function(data) {
                data.sort(function(a, b) {
                    if(a.name.toLowerCase() === b.name.toLowerCase()) {
                        return 0;
                    }
                    return a.name.toLowerCase() > b.name.toLowerCase() ? 1 : -1;
                });

                for(var i in data) {
                    data[i].value = data[i].email;
                    data[i].text = data[i].name + ' <' + data[i].email + '>';
                }

                return data;
            };

            var load = function(transform) {
                transform = transform || {};

                getData('aliases', {}, {
                    success: function(data) {
                        if(transform['options']) {
                            data = aliasesToOptions(data);
                        }
                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                loadAliases: load,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** auth ***/
        auth: function() {
            var observable = newObservable([
                    'success',
                    'failed',
                    'incorrect',
                    'noUsername',
                    'noPassword',
                    'locked'
                ].concat(listEvents([
                    'logout'
                ]))),
                message;

            var login = function(username, password) {
                if(!username || !password) {
                    if(!username && !password) {
                        message = 'Username and password are required.';
                        observable.notifyObservers('failed', message);
                        observable.notifyObservers('noUsername');
                    } else if(!username) {
                        message = 'Username is required.';
                        observable.notifyObservers('failed', message);
                        observable.notifyObservers('noUsername');
                    } else if(!password) {
                        message = 'Password is required.';
                        observable.notifyObservers('failed', message);
                        observable.notifyObservers('noPassword');
                    }
                } else {
                    getData('auth', {username: username, password: password}, {
                        success: function(data) {
                            switch(data.auth) {
                                case 'success':
                                    magma.session.set(data.session);
                                    observable.notifyObservers('success');
                                break;

                                case 'failed':
                                    observable.notifyObservers('failed', data.message);
                                    observable.notifyObservers('incorrect');
                                break;

                                case 'locked':
                                    observable.notifyObservers('locked', data.message);
                                break;
                            }
                        },
                        error: function(data) {
                            observable.notifyObservers('failed', 'Could not connect to the server');
                        },
                        fatal: function() {
                            // display error in login dialog
                            observable.notifyObservers('failed', 'Could not connect to the server');
                        }
                    });
                }
            };

            var logout = function() {
                getData('logout', null, {
                    success: function() {
                        // clear session
                        magma.session.set('');

                        observable.notifyObservers('logout');
                    },
                    error: function() {
                        observable.notifyObservers('logoutError');
                    },
                    failed: function() {
                        observable.notifyObservers('logoutFailed');
                    }
                });
            };

            return {
                login: login,
                logout: logout,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** compose ***/
        compose: function() {
            var observable = newObservable(listEvents(['ready']).concat(['send'])),
                id;

            // gets an id to keep track of drafts / added attachments / etc
            var compose = function() {
                getData('messages.compose', null, {
                    success: function(data) {
                        id = data.composeID;
                        observable.notifyObservers('ready');
                    },
                    error: function() {
                        observable.notifyObservers('readyError');
                    },
                    failure: function() {
                        observable.notifyObservers('readyFailed');
                    }
                });
            };

            return {
                composeMessage: compose,
                sendMessage: function() {
                    observable.notifyObservers('send');
                },
                getComposeID: function() {
                    return id;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** contact info ***/
        contactInfo: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['new', 'edit', 'show', 'hide'])),
                open = false;

            var load = function(contactID) {
                if(typeof contactID !== 'number') {
                    throw new Error('contactInfo.load: must provide a contactID');
                }

                getData('contacts.load', {contactID: contactID}, {
                    success: function(data) {
                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                loadContact: load,
                newContact: function() {
                    observable.notifyObservers('new');
                },
                editContact: function() {
                    observable.notifyObservers('edit');
                },
                isOpen: function() {
                    return open;
                },
                showInfo: function(type) {
                    open = true;
                    observable.notifyObservers('show', type);
                },
                hideInfo: function() {
                    open = false;
                    observable.notifyObservers('hide');
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** contact list ***/
        contactList: function() {
            var observable = newObservable(listEvents(['loaded', 'deleted']));

            var load = function(folder) {
                var folderID = folder;

                if(typeof folder === 'string') {
                    folderID = permaFolders.getID(folder);
                }

                if(typeof folderID !== 'number') {
                    throw new Error('model.contactList: requires folderID.');
                }

                getData('contacts.list', {folderID: folderID}, {
                    success: function(data) {
                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            var del = function(ids) {
                getData('contacts.remove', {contactIDs: ids}, {
                    success: function(data) {
                        observable.notifyObservers('deleted', ids);
                    },
                    error: function(error) {
                        observable.notifyObservers('deletedError', error);
                    },
                    failure: function() {
                        observable.notifyObservers('deletedFailed');
                    }
                });
            };

            return {
                load: load,
                deleteContacts: del,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** folder list ***/
        folders: function(context) {
            var observable = newObservable(listEvents(['loaded', 'added', 'removed', 'renamed', 'moved'])),
                folders = (function() {
                    var permanent = {}, // holds permanent named folders
                        custom = {}; // holds user defined folders

                    return {
                        add: function(name, folderID, parentID) {
                            // check unique
                            if(this.hasFolder(folderID)) {
                                throw new Error('model.folders folders.add: folderID ' + folderID + ' already set');
                            }

                            var folder = $.extend({name: name}, parentID ? {parentID: parentID} : {}),
                                slug = slugify(name);

                            // look for permanent folder names
                            if(permaFolders.getNames(context).join(' ').match(slug)) {
                                permaFolders.setID(slug, folderID);
                                permanent[folderID] = folder;
                            } else {
                                custom[folderID] = folder;
                            }

                            // use method to handle permant/custom discrepancy
                            return this.getFolder(folderID);
                        },
                        remove: function(folderID) {
                            if(this.hasFolder(folderID, 'permanent')) {
                                throw new Error('model.folders folders.remove: cannot remove permanent folder');
                            }

                            if(this.hasFolder(folderID, 'custom')) {
                                delete custom[folderID];

                                // look for children
                                for(var id in custom) {
                                    if(custom[id].parentID === folderID) {
                                        delete custom[id];
                                    }
                                }
                                return;
                            }

                            throw new Error('model.folders folders.remove: folder ' + folderID + ' not found');
                        },
                        move: function(folderID, parentID) {
                            // TODO: folder location data structure
                        },
                        rename: function(name, folderID) {
                            if(this.hasFolder(folderID, 'permanent')) {
                                throw new Error('model.folders folders.remove: cannot rename permanent folder');
                            }

                            if(this.hasFolder(folderID, 'custom')) {
                                custom[folderID].name = name;
                                return this.getFolder(folderID);
                            }

                            throw new Error('model.folders folders.remove: folder ' + folderID + ' not found');
                        },
                        hasFolder: function(folderID, type) {
                            if(type === 'permanent') {
                                return permanent[folderID] ? true : false;
                            } else if(type === 'custom') {
                                return custom[folderID] ? true : false;
                            } else {
                                return permanent[folderID] || custom[folderID] ? true : false;
                            }
                        },
                        getFolder: function(folderID) {
                            if(this.hasFolder(folderID, 'permanent')) {
                                var slug = permanent[folderID].name,
                                    folder = {};

                                folder[slug] = $.extend({folderID: folderID}, permanent[folderID]);
                                return folder;
                            } else if(this.hasFolder(folderID, 'custom')) {
                                return $.extend({folderID: folderID}, custom[folderID]);
                            }
                        },
                        getFolderList: function() {
                            var perm = {},
                                cust = [],
                                id;

                            for(id in permanent) {
                                var camel = camelCasify(permanent[id].name);

                                // not parsing int leaves id as a string and ruins subfolders
                                perm[camel] = $.extend({folderID: parseInt(id, 10)}, permanent[id]);
                            }

                            for(id in custom) {
                                // not parsing int leaves id as a string and ruins subfolders
                                cust.push($.extend({folderID: parseInt(id, 10)}, custom[id]));
                            }

                            return {permanent: perm, custom: cust};
                        },
                        empty: function() {
                            return (function(objs) {
                                for(var i in objs) {
                                    for(var key in objs[i]) {
                                        if(objs[i].hasOwnProperty(key)) {
                                            return false;
                                        }
                                    }
                                }
                                return true;
                            }([permanent, custom]));
                        },
                        reset: function() {
                            permanent = {};
                            custom = {};
                        }
                    };
                }());

            // default context
            context = context || 'mail';

            if(typeof context !== 'string') {
                throw new Error('view.folders: must provide folder context as a string');
            }

            if(typeof context !== 'string') {
                throw new Error('view.folders: must provide folder context as a string');
            }

            var cleanName = function(method, name) {
                if(!name || typeof name !== 'string') {
                    throw new Error('model.folders.' + method + ': must provide a name');
                }
            };

            var cleanID = function(method, id) {
                if(typeof id !== 'number') {
                    throw new Error('model.folders.' + method + ': must provide a folderID');
                }
            };

            var listToTree = function(list) {
                // create a folder tree
                // loop with while to deal with splicing during loop
                var i = 0,
                    scan = function(folders) {
                        for(var j in folders) {
                            if(folders[j].folderID === list[i].parentID) {
                                // init if no folders yet
                                folders[j].subfolders = folders[j].subfolders || [];
                                folders[j].subfolders.push(list[i]);

                                // remove folder from root
                                list.splice(i, 1);

                                return true; // success!

                            // scan subfolders
                            } else if(folders[j].subfolders) {
                                if(scan(folders[j].subfolders)) {
                                    // found parent in subfolder
                                    return true;
                                }
                            }
                        }
                    };

                while(list[i]) {
                    // child folder
                    if(typeof list[i].parentID === "number") {
                        // find child's parent
                        if(!scan(list)) {
                            // goodbye little orphan...
                            list.splice(i, 1);
                        }

                    // increment since nothing removed
                    } else {
                        i += 1;
                    }
                }

                return list;
            };

            var listToAlphaOrder = function(list) {
                list.sort(function(a, b) {
                    if(a.name.toLowerCase() === b.name.toLowerCase()) {
                        return 0;
                    }
                    return a.name.toLowerCase() > b.name.toLowerCase() ? 1 : -1;
                });

                return list;
            };

            var listToOptions = function(list) {
                list = listToAlphaOrder(list);

                for(var i in list) {
                    list[i].value = 'folder-' + list[i].folderID;
                }

                return list;
            };

            var transform = function(folders, transformOption) {
                transformOption = transformOption || {};

                if(transformOption['options']) {
                    folders.custom = listToOptions(folders.custom);
                } else if(transformOption['alphabetical']) {
                    folders.custom = listToAlphaOrder(folders.custom);
                } else if(!transformOption['none']) {
                    folders.custom = listToTree(folders.custom);
                }

                return folders;
            };

            var load = function(transformOption) {
                if(folders.empty()) {
                    getData('folders.list', {context: context}, {
                        success: function(data) {
                            $.each(data, function(i, folder) {
                                folders.add(folder.name, folder.folderID, folder.parentID);
                            });

                            observable.notifyObservers('loaded', transform(folders.getFolderList(), transformOption));
                        },
                        error: function() {
                            observable.notifyObservers('loadedError');
                        },
                        failure: function() {
                            observable.notifyObservers('loadedFailed');
                        }
                    });
                } else {
                    observable.notifyObservers('loaded', transform(folders.getFolderList(), transformOption));
                }
            };

            var add = function(name, parentID) {
                cleanName('add', name);

                if(permaFolders.getNames(context).join(' ').match(name)) {
                    observable.notifyObservers('addedError', name + ' is a reserved folder name');
                } else {
                    getData('folders.add', $.extend({name: name}, parentID ? {parentID: parentID} : {}), {
                        success: function(data) {
                            if(data['folders.add'] !== 'failed') {
                                var folder = folders.add(name, data.folderID, parentID);
                                observable.notifyObservers('added', folder);
                            } else {
                                observable.notifyObservers('addedError', data.message);
                            }
                        },
                        error: function(data) {
                            observable.notifyObservers('addedError', data.error);
                        }
                    });
                }
            };

            var remove = function(folderID) {
                cleanID('remove', folderID);

                // make sure it exists
                var folder = folders.getFolder(folderID);

                getData('folders.remove', {folderID: folderID}, {
                    success: function(data) {
                        try {
                            folders.remove(folderID);
                            observable.notifyObservers('removed', {folderID: folder.folderID, parentID: folder.parentID});
                        } catch(e) {
                            observable.notifyObservers('removedError', e.message);
                        }
                    },
                    error: function(data) {
                        observable.notifyObservers('removedError', data.error);
                    }
                });
            };

            var rename = function(name, folderID) {
                cleanName('rename', name);
                cleanID('remove', folderID);

                getData('folders.rename', {name: name, folderID: folderID}, {
                    success: function(data) {
                        try {
                            var folder = folders.rename(name, folderID);
                            observable.notifyObservers('renamed', folder);
                        } catch(e) {
                            observable.notifyObservers('renamedError', e.message);
                        }
                    },
                    error: function(data) {
                        observable.notifyObservers('renamedError', data.error);
                    }
                });
            };

            var move = function(sourceID, targetID) {
                cleanID('move', sourceID);
                cleanID('move', targetID);

                var params = {sourceFolderID: sourceID, targetFolderID: targetID};

                getData('folders.move', params, {
                    success: function(data) {
                        observable.notifyObservers('moved', params);
                    },
                    error: function(error) {
                        observable.notifyObservers('movedError', {folderID: sourceID, errer: error});
                    },
                    failure: function() {
                        observable.notifyObservers('movedFailed');
                    }
                });
            };

            var change = function(newContext, transformOption) {
                if(typeof newContext !== 'string') {
                    throw new Error('model.folders.change: must provide new context as string');
                }

                context = newContext;
                folders.reset();
                load(transformOption);
            };

            return {
                loadFolders: load,
                changeContext: change,
                getContext: function() {
                    return context;
                },
                addFolder: add,
                removeFolder: remove,
                renameFolder: rename,
                moveFolder: move,
                listFolders: function(transformOption) {
                    return transform(folders.getFolderList(), transformOption);
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** global nav ***/
        globalNav: function() {
            var observable = newObservable(['open']),
                openStates = {
                    mail: true,
                    contacts: false,
                    options: false,
                    logs: false,
                    help: false
                };

            var open = function(loc, init) {
                if(typeof openStates[loc] === 'undefined') {
                    throw new Error('model.globalNav: undefined location');
                }

                observable.notifyObservers('open', {loc: loc, init: init});

                for(var prop in openStates) {
                    if(prop === loc) {
                        openStates[prop] = true;
                    } else {
                        openStates[prop] = false;
                    }
                }
            };

            return {
                isOpen: function(loc) {
                    if(typeof openStates[loc] === 'undefined') {
                        throw new Error('model.globalNav.isOpen: undefined location');
                    }
                    return openStates[loc];
                },
                openLocation: open,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** loading ***/
        loading: function() {
            var observable = newObservable(['finished', 'continue']),
                callbacks = [],
                items = 0;

            var complete = function() {
                items -= 1;

                // call the finished function if all the items are complete
                if(!items) {
                    observable.notifyObservers('finished');
                }
            };

            // items are ananymous functions
            var register = function(callback) {
                // add an function to the list of callbacks
                callbacks.push(callback);
                items += 1;
            };

            var begin = function() {
                // call each item
                $.each(callbacks, function(i, callback) {
                    callback(complete);
                });
            };

            var cont = function() {
                observable.notifyObservers('continue');
            };

            return {
                registerItem: register,
                begin: begin,
                cont: cont,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },


        /*** help ***/
        help: function() {
            var observable = newObservable(listEvents([
                    'updateCategories',
                    'updateTopics',
                    'updatePage'
                ])),
                currentCategory,
                currentTopic,
                self;

            var updateCategories = function() {
                getData('folders.list', {context: 'help'}, {
                    success: function(data) {
                        // convert to helpList template context
                        $.each(data, function() {
                            this.id = this.folderID;
                        });

                        observable.notifyObservers('updateCategories', data);
                    },
                    error: function(error) {
                        observable.notifyObservers('updateCategoriesError', error);
                    },
                    failed: function() {
                        observable.notifyObservers('updateCategoriesFailed');
                    }
                });
            };

            var updateTopics = function(id) {
                if(typeof id !== 'number') {
                    throw new Error('model.help.updateTopics: must privide an id');
                }

                // don't update if same category clicked
                if(id !== currentCategory) {
                    getData('help.topics', {categoryID: id}, {
                        success: function(data) {
                            currentCategory = id;

                            // convert to helpList template context
                            $.each(data, function() {
                                this.id = this.topicID;
                            });

                            observable.notifyObservers('updateTopics', data);
                        },
                        error: function(error) {
                            observable.notifyObservers('updateTopicsError', error);
                        },
                        failed: function() {
                            observable.notifyObservers('updateTopicsFailed');
                        }
                    });
                }
            };

            var updatePage = function(id) {
                if(typeof id !== 'number') {
                    throw new Error('model.help.updatePage: must privide an id');
                }

                // don't update if same category clicked
                if(id !== currentTopic) {
                    getData('help.page', {topicID: id}, {
                        success: function(data) {
                            currentTopic = id;
                            observable.notifyObservers('updatePage', data);
                        },
                        error: function(error) {
                            observable.notifyObservers('updatePageError', error);
                        },
                        failed: function() {
                            observable.notifyObservers('updatePageFailed');
                        }
                    });
                }
            };

            var init = function() {
                self.observeOnce('updateCategories', function(categories) {
                    self.observeOnce('updateTopics', function(topics) {
                        // load first page
                        self.updatePage(topics[0].topicID);
                    });

                    // load the first category
                    self.updateTopics(categories[0].folderID);
                });

                // load the categories
                self.updateCategories();
            };

            self = {
                updateCategories: updateCategories,
                updateTopics: updateTopics,
                updatePage: updatePage,
                initHelp: init,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
            return self;
        },

        /*** logs ***/
        logs: function() {
            var observable = newObservable(listEvents(['loaded']));

            var load = function(display) {
                var context;

                if(typeof display === 'string') {
                    context = display;
                } else if(typeof display === 'number') {
                    context = permaFolders.getName(display);
                } else {
                    throw new Error('model.logs.load: must either provide an id or settings context ' + display);
                }

                // multiple word context use lowercase with underscores
                getData('logs.' + context.replace(/-/, '_'), null, {
                    success: function(data) {
                        $.each(data, function(i) {
                            data[i].time = new Date(data[i].utc).toLocaleString();
                            data[i].size = bytesToSize(data[i].bytes, 2);
                        });

                        observable.notifyObservers('loaded', {context: camelCasify(context), data: data});
                    },
                    error: function(error) {
                        observable.notifyObservers('loadedError', error);
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                load: load,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** message ***/
        message: function(messageID) {
            if(typeof messageID !== 'number') {
                throw new Error('model.messages.load: must provide a messageID');
            }

            var observable = newObservable(listEvents([
                    'headerLoaded',
                    'bodyLoaded',
                    'infoLoaded',
                    'attachmentsLoaded'
                ]).concat([
                    'messageLoaded',
                    'showInfo',
                    'closeInfo',
                    'showAttachments',
                    'closeAttachments'
                ])),
                message = {},
                infoOpen = false,
                attachmentsOpen = false;

            var load = function(section) {
                if(!(section instanceof Array)) {
                    throw new Error('model.messages.load: section list must be an array');
                }

                for(var i in section) {
                    if(typeof section[i] !== 'string') {
                        throw new Error('model.messages.load: section list must contain strings');
                    }
                }

                getData('messages.load', {messageID: messageID, section: section}, {
                    success: function(data) {
                        for(var prop in data) {
                            message[prop] = data[prop];
                        }
                        for(var i in section) {
                            observable.notifyObservers(section[i] + 'Loaded');
                        }
                        observable.notifyObservers('messageLoaded');
                    },
                    error: function() {
                        observable.notifyObservers(section + 'LoadedError');
                    },
                    failure: function() {
                        observable.notifyObservers(section + 'LoadedFailed');
                    }
                });
            };

            return {
                getID: function() {
                    return messageID;
                },
                loadSections: load,
                hasSection: function(section) {
                    return message[section] ? true : false;
                },
                getSection: function(section) {
                    if(!message[section]) {
                        throw new Error('model.message: does not contain section ' + section);
                    }
                    return message[section];
                },
                hasField: function(section, field) {
                    if(message[section]) {
                        return message[section][field] ? true : false;
                    } else {
                        return false;
                    }
                },
                getField: function(section, field) {
                    if(!message[section]) {
                        throw new Error('model.message: does not contain section ' + section);
                    } else if(!message[section][field]) {
                        throw new Error('model.message: does not contain field ' + field);
                    }
                    return message[section][field];
                },
                showInfo: function() {
                    infoOpen = true;
                    observable.notifyObservers('showInfo');
                },
                closeInfo: function() {
                    infoOpen = false;
                    observable.notifyObservers('closeInfo');
                },
                infoIsOpen: function() {
                    return infoOpen;
                },
                showAttachments: function() {
                    attachmentsOpen = true;
                    observable.notifyObservers('showAttachments');
                },
                closeAttachments: function() {
                    attachmentsOpen = false;
                    observable.notifyObservers('closeAttachments');
                },
                attachmentsIsOpen: function() {
                    return attachmentsOpen;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** message list ***/
        messageList: function() {
            var observable = newObservable(listEvents([
                    'loaded',
                    'deleted',
                    'flagged',
                    'copied',
                    'moved',
                    'tagged'
                ])),
                messages = (function() {
                    var self,
                        _messages = {}, // {{message: message, refCount: num}, ...}
                        _folders = {}, // {folderID: messageList, folderID: messsageList, ...}
                        _flags = ['seen', 'flagged', 'answered'],
                        _indexOf = function(messageID, folderID) {
                            if(self.hasMessage(messageID, folderID)) {
                                return _folders[folderID].indexOf(_messages[messageID]);
                            }
                        };

                    self = {
                        add: function(messageID, folderID, message) {
                            if(this.hasMessage(messageID)) {
                                throw new Error('messsageList messages.add: message id ' + messageID + ' already exists');
                            }

                            if(!this.hasFolder(folderID)) {
                                this.initFolder(folderID);
                            }

                            _messages[messageID] = message;
                            this.updateFlags(messageID);
                            _folders[folderID].push(_messages[messageID]);
                        },
                        remove: function(messageID, folderID) {
                            var index;

                            if(this.hasMessage(messageID, folderID)) {
                                index = _indexOf(messageID, folderID);
                            }

                            if(index === -1) {
                                throw new Error('messsageList messages.remove: id ' + messageID + ' does not exists in ' + folderID);
                            }

                            _folders[folderID].splice(index, 1);
                            delete _messages[messageID];
                        },
                        move: function(messageID, oldFolderID, newFolderID) {
                            var index;

                            if(this.hasMessage(messageID, oldFolderID)) {
                                index = _indexOf(messageID, oldFolderID);
                            }

                            if(index === -1) {
                                throw new Error('messsageList messages.move: id ' + messageID + ' does not exists in ' + oldFolderID);
                            }

                            // temp copy of message
                            var message = _messages[messageID];
                            this.remove(messageID, oldFolderID);
                            this.add(messageID, newFolderID, message);
                        },
                        update: function(id, folderID, message) {
                            if(this.hasMessage(id, folderID)) {
                                // copy message instead of asigning to replicate changes to folder
                                for(var prop in _messages[id]) {
                                    _messages[id][prop] = message[prop];
                                }
                                this.updateFlags(id);
                                return true;
                            } else {
                                return false;
                            }
                        },
                        hasMessage: function(messageID, folderID) {
                            if(typeof folderID === 'number') {
                                if(this.hasMessage(messageID)) {
                                    return _folders[folderID].indexOf(_messages[messageID]) >= 0 ? true : false;
                                } else {
                                    return false;
                                }
                            } else {
                                return _messages[messageID] ? true : false;
                            }
                        },
                        updateFlags: function(id) {
                            if(this.hasMessage(id)) {
                                $.each(_flags, function(i, flag) {
                                    _messages[id].flag[flag] = false;
                                });

                                $.each(_messages[id].flags, function(i, flag) {
                                    _messages[id].flag[flag] = true;
                                });
                            }
                        },
                        getMessage: function(id) {
                            if(!this.hasMessage(id)) {
                                throw new Error('messsageList messages.getMessage: no message ' + id);
                            }
                            return $.extend(true, {}, _messages[id]);
                        },
                        getMessages: function(ids) {
                            var messages = [];

                            for(var i in ids) {
                                if(!this.hasMessage(ids[i])) {
                                    throw new Error('messsageList messages.getMessages: no message ' + ids[i]);
                                }
                                messages.push(this.getMessage(ids[i]));
                            }

                            return messages;
                        },
                        initFolder: function(id) {
                            if(this.hasFolder(id)) {
                                throw new Error('messsageList messages.initFolder: folder ' + id + ' already exists');
                            }
                            _folders[id] = [];
                        },
                        hasFolder: function(id) {
                            return _folders[id] ? true : false;
                        },
                        getFolder: function(id) {
                            return _folders[id].slice(0);
                        }
                    };

                    return self;
                }()),
                currentFolder;

            var load = function(folder) {
                var folderID = folder;

                if(typeof folder === 'string') {
                    folderID = permaFolders.getID(folder);
                }

                if(typeof folderID !== 'number') {
                    throw new Error('model.messages.list: requires folderID.');
                }

                getData('messages.list', {folderID: folderID}, {
                    success: function(data) {
                        // must be called before filling folder
                        // this creates an empty folder
                        // avoids error when valid request for empty folder is made
                        if(!messages.hasFolder(folderID)) {
                            messages.initFolder(folderID);
                        }

                        // transformations
                        $.each(data, function(i, message) {
                            message.flags = message.flags || [];
                            message.flag = {};
                            message.tags = message.tags || [];
                            message.attachment = message.attachment || false;

                            // convert unix time and bytes to human readable strings
                            message.date = new Date(message.utc).toLocaleString();
                            message.arrivalDate = new Date(message.arrivalUtc).toLocaleString();
                            message.size = bytesToSize(message.bytes, 2);

                            $.each(message.tags.sort(ignoreCaseSort), function(j, tag) {
                                message.tags[j] = {name: tag, slug: slugify(tag)};
                            });

                            if(!messages.update(message.messageID, folderID, message)) {
                                messages.add(message.messageID, folderID, message);
                            }
                        });

                        // save current folder for operations
                        currentFolder = folderID;

                        observable.notifyObservers('loaded', messages.getFolder(folderID));
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            var del = function(messageIDs) {
                if(!(messageIDs instanceof Array)) {
                    throw new Error('model.messsageList.deleteMessage: must provide messageIDs in an array.');
                }

                if(!messageIDs.length) {
                    observable.notifyObservers('deletedError', 'Please check messages to delete.');
                }

                // ignore request if emtpy
                if(messageIDs.length) {
                    getData('messages.remove', {messageIDs: messageIDs, folderID: currentFolder}, {
                        success: function() {
                            $.each(messageIDs, function(i, id) {
                                messages.remove(id, currentFolder);
                            });

                            observable.notifyObservers('deleted', messageIDs);
                        },
                        error: function(error) {
                            observable.notifyObservers('deletedError', error);
                        },
                        failure: function() {
                            observable.notifyObservers('deletedFailed', messageIDs);
                        }
                    });
                }
            };

            // handles junk and mark
            var flag = function(messageIDs, action) {
                if(!(messageIDs instanceof Array)) {
                    throw new Error('model.messsageList.flagMessages: must provide messageIDs in an array.');
                }

                if(messageIDs.length) {
                    var flag,
                        ids = [],
                        match,
                        params;

                    // use IMAP terms
                    if(action.match(/read|unread/i)) {
                        flag = 'seen';
                        match = /seen/i;
                    } else if(action.match(/star|unstar/i)) {
                        flag = 'flagged';
                        match = /flagged/i;
                    }

                    // look for removal types
                    if(action.match(/^unread$|^unstar$/i)) {

                        // narrow down request to applicable messageIDs
                        $.each(messageIDs, function(i, id) {
                            if(messages.hasMessage(id) && messages.getMessage(id).flags.length) {

                                // send only ids that have tag to be removed
                                $.each(messages.getMessage(id).flags, function(i, _flag) {
                                    if(_flag.match(match)) {
                                        ids.push(id);
                                    }
                                });
                            }
                        });

                        if(ids.length) {
                            params = {messageIDs: ids, flags: [flag], action: 'remove', folderID: currentFolder};

                            getData('messages.flag', params, {
                                success: function() {
                                    // update messages
                                    $.each(ids, function(i, id) {
                                        $.each(messages.getMessage(id).flags, function(i, _flag) {
                                            if(_flag.match(match)) {
                                                var message = messages.getMessage(id);
                                                message.flags.splice(i, 1);
                                                messages.update(id, currentFolder, message);
                                            }
                                        });
                                    });

                                    observable.notifyObservers('flagged', {messageIDs: ids, data: messages.getMessages(ids)});
                                },
                                error: function() {
                                    observable.notifyObservers('flaggedError', ids);
                                },
                                failure: function() {
                                    observable.notifyObservers('flaggedFailed', ids);
                                }
                            });
                        }

                    // else flags will be added
                    } else {
                        // only apply tag if not already present
                        $.each(messageIDs, function(i, id) {
                            var _flags = [];

                            if(messages.hasMessage(id) && messages.getMessage(id).flags.length) {
                                // build matchable flag string
                                $.each(messages.getMessage(id).flags, function(i, _flag) {
                                    _flags.push(_flag);
                                });
                            }

                            if(!_flags.join(' ').match(match)) {
                                ids.push(id);
                            }
                        });

                        if(ids.length) {
                            params = {messageIDs: ids, flags: [flag], action: 'add', folderID: currentFolder};

                            getData('messages.flag', params, {
                                success: function() {
                                    // update messages
                                    $.each(ids, function(i, id) {
                                        var message = messages.getMessage(id);
                                        message.flags.push(flag);
                                        messages.update(id, currentFolder, message);
                                    });

                                    observable.notifyObservers('flagged', {messageIDs: ids, data: messages.getMessages(ids)});
                                },
                                error: function() {
                                    observable.notifyObservers('flaggedError', ids);
                                },
                                failure: function() {
                                    observable.notifyObservers('flaggedFailed', ids);
                                }
                            });
                        }
                    }

                // empty messageIDs is invalid
                } else {
                    throw new Error('model.messageList messages.flag: Empty messageIDs');
                }
            };

            var copy = function(messageIDs, folderID) {
                if(!(messageIDs instanceof Array)) {
                    throw new Error('model.messsageList.copyMessages: must provide messageIDs in an array.');
                }

                if(typeof folderID !== 'number') {
                    throw new Error('model.messageList.copyMessages: must provide a folderID');
                }

                if(currentFolder === folderID) {
                    throw new Error('model.messsageList copyMessages: cannot copy to the same folder');
                }

                if(messageIDs.length) {
                    var params = {messageIDs: messageIDs, sourceFolderID: currentFolder, targetFolderID: folderID};

                    getData('messages.copy', params, {
                        // contains old and new messageIDs
                        success: function(messageIDs) {
                            var msgs = [];

                            $.each(messageIDs, function(i, id) {
                                if(messages.hasMessage(id.sourceMessageID)) {
                                    var newMessage = messages.getMessage(id.sourceMessageID);
                                    newMessage.messageID = id.targetMessageID;
                                    messages.add(id.targetMessageID, folderID, newMessage);

                                    msgs.push(newMessage);
                                } else {
                                    throw new Error('copyMessages: source message id ' + id.sourceMessageID + ' does not exist');
                                }
                            });
                        },
                        error: function(errorMessage) {
                            observable.notifyObservers('copiedError', [errorMessage, params]);
                        },
                        failure: function() {
                            observable.notifyObservers('copiedFailed', params);
                        }
                    });
                }
            };

            var move = function(messageIDs, folderID) {
                if(!(messageIDs instanceof Array)) {
                    throw new Error('model.messsageList.moveMessages: must provide messageIDs in an array.');
                }

                if(typeof folderID !== 'number') {
                    throw new Error('model.messageList.moveMessages: must provide a folderID');
                }

                if(currentFolder === folderID) {
                    throw new Error('model.messsageList moveMessages: cannot move to the same folder');
                }

                var params = {messageIDs: messageIDs, sourceFolderID: currentFolder, targetFolderID: folderID};

                getData('messages.move', params, {
                    success: function(data) {
                        $.each(messageIDs, function(i, id) {
                            // remove message from folder
                            messages.move(id, currentFolder, folderID);
                        });

                        observable.notifyObservers('moved', messageIDs);
                    },
                    error: function(data) {
                        observable.notifyObservers('movedError', data);
                    },
                    failure: function() {
                        observable.notifyObservers('movedFailed', params);
                    }
                });
            };

            var tag = function(messageIDs, tags) {
                // convert tags to array if just a single tag passed
                if(typeof tags === 'string') {
                    tags = [tags];
                }

                if(!(tags instanceof Array)) {
                    throw new Error('model.messsageList.tagMessages: must provide tags as an array of objects.');
                }

                var params = {messageIDs: messageIDs, tag: tags};

                getData('messages.tag', params, {
                    success: function() {
                        // update messages
                        $.each(messages.getMessages(messageIDs), function(i, message) {
                            $.each(tags, function(i, tag) {
                                message.tags.push({slug: slugify(tag), name: tag});
                            });

                            messages.update(message.messageID, currentFolder, message);
                        });

                        observable.notifyObservers('tagged', {messageIDs: messageIDs, data: messages.getMessages(messageIDs)});
                    },
                    error: function() {
                        observable.notifyObservers('taggedError', tags);
                    },
                    failure: function() {
                        observable.notifyObservers('taggedFailed', tags);
                    }
                });
            };

            return {
                load: load,
                del: del,
                flag: flag,
                copy: copy,
                move: move,
                tag: tag,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** meta ***/
        meta: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['open'])),
                open = false;

            var get = function() {
                getData('meta', null, {
                    success: function(data) {
                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failed: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                getMeta: get,
                open: function() {
                    observable.notifyObservers('open');
                },
                isOpen: function() {
                    return open;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** model ***/
        model: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['open'])),
                open = false;

            var get = function() {
                getData('meta', null, {
                    success: function(data) {
                        observable.notifyObservers('loaded', data);
                    },
                    error: function() {
                        observable.notifyObservers('loadedError');
                    },
                    failed: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                getMeta: get,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },


        /*** tab ***/
        tab: function(title, id, permatab) {
            if(!title) {
                throw new Error('model.tab: Must provide a title.');
            } else if(typeof title !== 'string') {
                throw new Error('model.tab: Tab title must be given as a string.');
            } else if(typeof id !== 'number') {
                throw new Error('model.tab: Tab id not provided');
            }

            // false if empty
            permatab = permatab || false;

            if(typeof permatab !== 'boolean') {
                throw new Error('model.tab: Permatab option must be given as a boolean');
            }

            var observable = newObservable(['focus', 'blur', 'closed', 'remove', 'titleModified']),
                active = false;

            return {
                focus: function() {
                    active = true;
                    observable.notifyObservers('focus', this);
                    return this;
                },
                blur: function() {
                    active = false;
                    observable.notifyObservers('blur', this);
                    return this;
                },
                close: function() {
                    observable.notifyObservers('closed', this);
                },
                remove: function() {
                    observable.notifyObservers('remove', this);
                },
                setTitle: function(t) {
                    if(typeof title !== 'string') {
                        throw new Error('Tab title must be given as a string');
                    }

                    title = t;
                    observable.notifyObservers('titleModified', t);
                },
                getTitle: function() {
                    return title;
                },
                getID: function() {
                    return id;
                },
                isActive: function() {
                    return active;
                },
                isPermatab: function() {
                    return permatab;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** tabs ***/
        tabs: function() {
            var observable = newObservable(['tabAdded', 'tabRemoved', 'show', 'hide']),
                tabs = [],
                open = true,
                params = {
                    observable: observable,
                    models: tabs
                };

            var add = addModel($.extend({
                method: 'addTab',
                event: 'tabAdded'
            }, params));

            var remove = removeModel($.extend({
                method: 'removeTab',
                event: 'tabRemoved'
            }, params));

            return {
                forEach: function(f) {
                    if(!f) {
                        throw new Error('tabs.forEach: must provide a callback function');
                    } else if(typeof f !== 'function') {
                        throw new Error('tabs.forEach: callback must be a function');
                    }
                    for(var i in tabs) {
                        f(tabs[i]);
                    }
                },
                tabCount: function() {
                    return tabs.length;
                },
                hasTabs: function() {
                    return tabs.length ? true : false;
                },
                lastTab: function() {
                    // check for tabs first
                    if(tabs.length === 0) {
                        throw new Error('model.tabs.last: no tabs to return');
                    }
                    return tabs[tabs.length - 1];
                },
                isOpen: function() {
                    return open;
                },
                showTabs: function() {
                    open = true;
                    observable.notifyObservers('show');
                },
                hideTabs: function() {
                    open = false;
                    observable.notifyObservers('hide');
                },
                addTab: add,
                removeTab: remove,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** tags ***/
        tags: function() {
            var observable = newObservable(listEvents(['loaded', 'added']));

            // takes a list of tag names and returns a list of objects with slug/href/name fields
            var toTagList = function(tags) {
                var list = [];

                $.each(tags, function(i, tag) {
                    var slug = slugify(tag);

                    list.push({
                        slug: slug,
                        href: '#' + slug,
                        name: tag
                    });
                });

                return list;
            };

            var get = function() {
                getData('messages.tags', null, {
                    success: function(data) {
                        observable.notifyObservers('loaded', toTagList(data));
                    },
                    error: function(error) {
                        observable.notifyObservers('loadedError');
                    },
                    failed: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            var add = function(name) {
                if(typeof tag !== 'string') {
                    throw new Error('tags.add: tag name must be of type string');
                }

                getData('tags.add', null, {
                    success: function(data) {
                        observable.notifyObservers('added', data);
                    },
                    error: function(error) {
                        observable.notifyObservers('addedError');
                    },
                    failed: function() {
                        observable.notifyObservers('addedFailed');
                    }
                });
            };

            return {
                loadTags: get,
                addTag: add,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** tmpl ***/
        tmpl: function() {
            var observable = newObservable(['completed']);

            var fill = function(template, data) {
                // compile the template if needed
                loadTmpl(template, function() {
                    var jquery = $.tmpl(template, data);
                    observable.notifyObservers('completed', jquery);
                });
            };

            return {
                fillTmpl: fill,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** tools ***/
        tools: function() {
            var observable = newObservable(['enabled', 'disabled', 'showDropdown', 'hideDropdown']);

            return {
                addTool: function(toolname, toggle) {
                    observable.addEvent(toolname + 'Clicked');
                },
                toolClicked: function(toolname, data) {
                    observable.notifyObservers(toolname + 'Clicked', data);
                },
                enableTool: function(toolname) {
                    observable.notifyObservers('enabled', toolname);
                },
                disableTool: function(toolname) {
                    observable.notifyObservers('disabled', toolname);
                },
                showDropdown: function() {
                    observable.notifyObservers('showDropdown');
                },
                hideDropdown: function() {
                    observable.notifyObservers('hideDropdown');
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** scrape contacts ***/
        scrape: function(messageID) {
            var observable = newObservable(['update'].concat(listEvents(['loaded', 'added'])));

            var load = function() {
                getData('scrape', {messageID: messageID}, {
                    success: function(data) {
                        observable.notifyObservers('loaded', data);
                    },
                    error: function(error) {
                        observable.notifyObservers('loadedError', error);
                    },
                    failed: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            var add = function(values) {
                getData('scrape.add', $.extend(values, {messageID: messageID}), {
                    success: function(data) {
                        observable.notifyObservers('added', values.id);
                    },
                    error: function(error) {
                        observable.notifyObservers('addedError', error);
                    },
                    failed: function() {
                        observable.notifyObservers('addedFailed');
                    }
                });
            };

            return {
                load: load,
                add: add,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** search results ***/
        searchResults: function() {
            var observable = newObservable(['loading'].concat(listEvents(['loaded']))),
                loading;

            var search = function(query) {
                // start loading
                loading = true;
                getData('search', query, {
                    success: function(results) {
                        // TODO: remove artificial search results delay
                        setTimeout(function() {
                        loading = false;

                        // transformations
                        $.each(results, function(i, message) {
                            message.flags = message.flags || [];
                            message.flag = {};
                            message.tags = message.tags || [];
                            message.attachment = message.attachment || false;

                            // convert unix time and bytes to human readable strings
                            message.date = new Date(message.utc).toLocaleString();
                            message.arrivalDate = new Date(message.arrivalUtc).toLocaleString();
                            message.size = bytesToSize(message.bytes, 2);

                            $.each(message.tags.sort(ignoreCaseSort), function(j, tag) {
                                message.tags[j] = {name: tag, slug: slugify(tag)};
                            });
                        });

                        observable.notifyObservers('loaded', results);
                        }, 6000);
                    },
                    error: function(error) {
                        loading = false;
                        observable.notifyObservers('loadedError', error);
                    },
                    failed: function() {
                        loading = false;
                        observable.notifyObservers('loadedFailed');
                    }
                });

                // wait to start loading bar
                setTimeout(function() {
                    // TODO: remove progress bar test
                    var progress = 10;

                    var i = setInterval(function() {
                        // get progress
                        // TODO: to get search result loading progress, need to identify search - i.e. return id on initial search
                        if(loading) {
                            if(progress < 100) {
                                progress += 10;
                                observable.notifyObservers('loading', progress);
                            }
                        } else {
                            clearInterval(i);
                        }
                    }, 500);
                }, 500);
            };

            return {
                search: search,
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** search option ***/
        searchOption: function(id, row) {
            var observable = newObservable(['remove', 'update', 'rowUpdated', 'controlsUpdated']),
                controls = {add: false, remove: false};

            var remove = function() {
                observable.notifyObservers('remove');
            };

            var update = function(params) {
                observable.notifyObservers('update', params);
            };

            return {
                removeOption: remove,
                updateOption: update,
                getID: function() {
                    return id;
                },
                getRow: function() {
                    return row;
                },
                setRow: function(num) {
                    if(typeof num !== 'number') {
                        throw new Error('model.searchOption.setRow: row must be an integer');
                    }
                    row = num;
                    observable.notifyObservers('rowUpdated', row);
                },
                getControls: function() {
                    return controls;
                },
                setControls: function(ctrls) {
                    ctrls = ctrls || {};
                    if(typeof ctrls.add !== 'boolean') {
                        ctrls.add = false;
                    }
                    if(typeof ctrls.remove !== 'boolean') {
                        ctrls.remove = false;
                    }
                    controls = ctrls;
                    observable.notifyObservers('controlsUpdated', controls);
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** search options ***/
        searchOptions: function() {
            var observable = newObservable(['optionAdded', 'optionRemoved']),
                options = [],
                limit = 5,
                params = {
                    observable: observable,
                    models: options
                };

            var add = addModel($.extend({
                method: 'addOption',
                event: 'optionAdded'
            }, params));

            var remove = removeModel($.extend({
                method: 'removeOption',
                event: 'optionRemoved'
            }, params));

            var updateRows = function() {
                for(var i=0; i<options.length; i += 1) {
                    options[i].updateOption({
                        row: i,
                        singleRow: options.length === 1,
                        atLimit: options.length === limit
                    });
                }
            };

            return {
                addOption: function(searchOptionModel) {
                    if(options.length < limit) {
                        add(searchOptionModel);
                        updateRows();
                    }
                },
                removeOption: function(searchOptionModel) {
                    remove(searchOptionModel);
                    updateRows();
                },
                getCurrRow: function() {
                    return options.length;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** settings ***/
        settings: function() {
            var observable = newObservable(listEvents(['loaded']).concat(['advanced', 'default', 'edit']));

            // handles sending in context or folder id
            var load = function(display) {
                var context;

                if(typeof display === 'string') {
                    context = display;
                } else if(typeof display === 'number') {
                    context = permaFolders.getName(display);
                } else {
                    throw new Error('model.settings.load: must either provide an id or settings context ' + display);
                }

                // multiple word context use lowercase with underscores
                getData('settings.' + context.replace(/-/, '_'), null, {
                    success: function(data) {
                        $.each(data, function(i) {
                            data[i].time = new Date(data[i].utc).toLocaleString();
                            data[i].size = bytesToSize(data[i].bytes, 2);
                        });

                        observable.notifyObservers('loaded', {context: camelCasify(context), data: data});
                    },
                    error: function(error) {
                        observable.notifyObservers('loadedError', error);
                    },
                    failure: function() {
                        observable.notifyObservers('loadedFailed');
                    }
                });
            };

            return {
                load: load,
                advancedSettings: function() {
                    observable.notifyObservers('advanced');
                },
                defaultSettings: function() {
                    observable.notifyObservers('defaults');
                },
                editSettings: function() {
                    observable.notifyObservers('edit');
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        },

        /*** workspace ***/
        workspace: (function() {
            // close over id counter
            var ID = 0;

            return function() {
                ID += 1;

                var observable = newObservable(['focus', 'blur', 'closed', 'remove']),
                    id = ID,
                    active = false;

                return {
                    focus: function() {
                        active = true;
                        observable.notifyObservers('focus', this);
                        return this;
                    },
                    blur: function() {
                        active = false;
                        observable.notifyObservers('blur', this);
                        return this;
                    },
                    close: function() {
                        observable.notifyObservers('closed', this);
                    },
                    remove: function() {
                        observable.notifyObservers('remove', this);
                    },
                    getID: function() {
                        return id;
                    },
                    isActive: function() {
                        return active;
                    },
                    addObserver: observable.addObserver,
                    observeOnce: observable.observeOnce,
                    removeObserver: observable.removeObserver
                };
            };
        }()),

        /*** workspaces ***/
        workspaces: function() {
            var observable = newObservable(['workspaceAdded', 'workspaceRemoved', 'dedicatedWorkspaceAdded']),
                workspaces = [],
                lastActive,
                params = {
                    observable: observable,
                    models: workspaces
                };

            var add = addModel($.extend({
                method: 'addWorkspace',
                event: 'workspaceAdded'
            }, params));

            var remove = removeModel($.extend({
                method: 'addWorkspace',
                event: 'workspaceRemoved'
            }, params));

            return {
                forEach: function(f) {
                    if(!f) {
                        throw new Error('workspaces.forEach: must provide a callback function');
                    } else if(typeof f !== 'function') {
                        throw new Error('workspaces.forEach: callback must be a function');
                    }
                    for(var i in workspaces) {
                        f(workspaces[i]);
                    }
                },
                workspaceCount: function() {
                    return workspaces.length;
                },
                hasWorkspace: function() {
                    return workspaces.length ? true : false;
                },
                lastWorkspace: function() {
                    if(workspaces.length === 0) {
                        throw new Error('model.workspaces.last: no workspace to return');
                    }
                    return workspaces[workspaces.length - 1];
                },
                addWorkspace: add,
                removeWorkspace: remove,
                addDedicatedWorkspace: function(workspaceModel) {
                    add(workspaceModel);
                    observable.notifyObservers('dedicatedWorkspaceAdded', workspaceModel);
                },
                saveLastActive: function(workspace) {
                    for(var i in workspaces) {
                        if(workspace === workspaces[i]) {
                            lastActive = i;
                            return;
                        }
                    }
                    throw new Error('saveLastActive: no workspace found.');
                },
                focusLastActive: function() {
                    if(!lastActive) {
                        throw new Error('focusLastActive: lastActive not set.');
                    }
                    workspaces[lastActive].focus();
                    lastActive = undefined;
                },
                addObserver: observable.addObserver,
                observeOnce: observable.observeOnce,
                removeObserver: observable.removeObserver
            };
        }
    };
}());
/*
var magma = magma || {};

magma.tinymce = function(editorID) {
    var tinyPath = '/tiny_mce',
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
/**
 * Contains all the jquery templates
 *
 * templates inserted by build-js.php
 */
var magma = magma || {};

magma.tmpl = {
'ad': '<a class="{{if context}}${context}-{{/if}}ad" href="${href}" {{if title}}title="${title}"{{/if}}>{{tmpl(img) "image"}}</a>',
'alert': '{{if alert}}<div class="alerts">{{each alert}}<div class="alert"><h2 {{if alertID}}id="alert-${alertID}"{{/if}}>System Alert</h2><p>${message}</p></div>{{/each}}</div>{{/if}}{{if warning}}<div class="warnings">{{each warning}}<div class="warning"><h2 {{if alertID}}id="alert-${alertID}"{{/if}}>Account Warning</h2><p>${message}</p></div>{{/each}}</div>{{/if}}',
'contactInfo': '<h4>${name}</h4><dl class="email"><dt>Primary Email</dt><dd class="primary">${email.primary}</dt>{{if email.alternate1}}<dt>Alternate 1</dt><dd class="alternate-1">${email.alternate1}</dd>{{/if}}{{if email.alternate2}}<dt>Alternate 2</dt><dd class="alternate-2">${email.alternate2}</dd>{{/if}}</dl>{{if chat}}<dl class="chat">{{if chat.yahoo}}<dt>Yahoo</dt><dd class="yahoo">${chat.yahoo}</dd>{{/if}}{{if chat.live}}<dt>Windows Live</dt><dd class="live">${chat.live}</dd>{{/if}}{{if chat.aim}}<dt>AOL/AIM</dt><dd class="aim">${chat.aim}</dd>{{/if}}{{if chat.google}}<dt>Google</dt><dd class="google">${chat.google}</dd>{{/if}}{{if chat.icq}}<dt>ICQ</dt><dd class="icq">${chat.icq}</dd>{{/if}}</dl>{{/if}}{{if contact}}<dl class="contact">{{if contact.home}}<dt>Home</dt><dd class="home">${contact.home}</dd>{{/if}}{{if contact.home}}<dt>Work</dt><dd class="work">${contact.work}</dd>{{/if}}{{if contact.mobile}}<dt>Mobile</dt><dd class="mobile">${contact.mobile}</dd>{{/if}}{{if contact.pager}}<dt>Pager</dt><dd class="pager">${contact.pager}</dd>{{/if}}{{if contact.fax}}<dt>Fax</dt><dd class="fax">${contact.fax}</dd>{{/if}}{{if contact.address}}<dt>Address</dt><dd class="address">{{html contact.address}}</dd>{{/if}}</dl>{{/if}}',
'contactList': '<tr id="contact-${contactID}"><td><input type="checkbox" name="check-${messageID}" value="${messageID}" /></td><td>{{if img}}{{tmpl(img) "image"}}{{/if}}</td><td>{{if name}}${name}{{/if}}</td><td>{{if email}}${email}{{/if}}</td><td>{{if company}}${company}{{/if}}</td></tr>',
'dictionary': '<dl>{{each list}}<dt>${key}</dt><dd {{if type}}class="${type}"{{/if}}>{{html value}}</dd>{{/each}}</dl>',
'fact': '{{if text.length > 1 }}<ul class="facts">{{each text}}<li class="fact">${$value}</li>{{/each}}</ul>{{else}}<p class="facts">${text[0]}</p>{{/if}}',
'foldersContacts': '{{if all}}<li id="folder-${all.folderID}"><a class="all folder permanent" href="#folder-${all.folderID}">${all.name}</a></li>{{/if}}{{if people}}<li id="folder-${people.folderID}"><a class="people folder permanent" href="#folder-${people.folderID}">${people.name}</a></li>{{/if}}{{if business}}<li id="folder-${business.folderID}"><a class="business folder permanent" href="#folder-${business.folderID}">${business.name}</a></li>{{/if}}{{if collected}}<li id="folder-${collected.folderID}"><a class="collected folder permanent" href="#folder-${collected.folderID}">${collected.name}</a></li>{{/if}}',
'foldersExpandable': '<li id="folder-${folderID}" class="toggle{{if subfolders}} expandable{{/if}}{{if opened}} open{{/if}}"> {{if subfolders}}<a class="expander">Toggle</a>{{/if}} <a class="folder" href="#folder">${name}</a></li>{{if subfolders}}<li id="subfolders-${folderID}" class="collapsed"><ul>{{tmpl($item.data.subfolders) "foldersExpandable"}}</ul></li>{{/if}}',
'foldersList': '<li id="folder-${folderID}"><a class="folder" href="#folder-${folderID}">${name}</a></li>',
'foldersLogs': '{{if statistics}}<li id="folder-${statistics.folderID}"><a class="statistics folder permanent" href="#folder-${statistics.folderID}">${statistics.name}</a></li>{{/if}}{{if security}}<li id="folder-${security.folderID}"><a class="security folder permanent" href="#folder-${security.folderID}">${security.name}</a></li>{{/if}}{{if contacts}}<li id="folder-${contacts.folderID}"><a class="contacts folder permanent" href="#folder-${contacts.folderID}">${contacts.name}</a></li>{{/if}}{{if mail}}<li id="folder-${mail.folderID}"><a class="mail folder permanent" href="#folder-${mail.folderID}">${mail.name}</a></li>{{/if}}',
'foldersMail': '{{if inbox}}<li id="folder-${inbox.folderID}"><a class="inbox folder permanent" href="#folder-${inbox.folderID}">${inbox.name}</a></li>{{/if}}{{if drafts}}<li id="folder-${drafts.folderID}"><a class="drafts folder permanent" href="#folder-${drafts.folderID}">${drafts.name}</a></li>{{/if}}{{if sent}}<li id="folder-${sent.folderID}"><a class="sent folder permanent" href="#folder-${sent.folderID}">${sent.name}</a></li>{{/if}}{{if junk}}<li id="folder-${junk.folderID}"><a class="junk folder permanent" href="#folder-${junk.folderID}">${junk.name}</a></li>{{/if}}{{if trash}}<li id="folder-${trash.folderID}"><a class="trash folder permanent" href="#folder-${trash.folderID}">${trash.name}</a></li>{{/if}}',
'foldersSettings': '{{if identity}}<li id="folder-${identity.folderID}"><a class="identity folder permanent" href="#folder-${identity.folderID}">${identity.name}</a></li>{{/if}}{{if mailSettings}}<li id="folder-${mailSettings.folderID}"><a class="mail-settings folder permanent" href="#folder-${mailSettings.folderID}">${mailSettings.name}</a></li>{{/if}}{{if portalSettings}}<li id="folder-${portalSettings.folderID}"><a class="portal-settings folder permanent" href="#folder-${portalSettings.folderID}">${portalSettings.name}</a></li>{{/if}}{{if accountUpgrades}}<li id="folder-${accountUpgrades.folderID}"><a class="account-upgrades folder permanent" href="#folder-${accountUpgrades.folderID}">${accountUpgrades.name}</a></li>{{/if}}{{if password}}<li id="folder-${password.folderID}"><a class="password folder permanent" href="#folder-${password.folderID}">${password.name}</a></li>{{/if}}',
'helpList': '<li id="${id}">${name}</li>',
'image': '<img src="${src}" alt="${alt}" />',
'logsMail': '<tr id="message-${messageID}"><td>${queue}</td><td>${type}</td><td>${from}</td><td>${to}</td><td>${outcome}</td><td>${utc}</td><td>${time}</td><td>${bytes}</td><td>${size}</td></tr>',
'logsSecurity': '<tr><td>${utc}</td><td>${time}</td><td>${type}</td><td>${severity}</td><td>${ip}</td><td>${protocol}</td></tr>',
'logsStatistics': '<h3>Statistics</h3><h4>Account</h4><dl><dt>Username</dt><dd>${account.username}</dd><dt>Name</dt><dd>${account.name}</dd><dt>User Number</dt><dd>${account.number}</dd><dt>Reputation</dt><dd>${account.reputation}</dd><dt>Plan</dt><dd>${account.plan}</dd><dt>Registration Date</dt><dd>${account.date}</dd></dl><h4>Storage</h4><dl><dt>Space</dt><dd>${storage.space}</dd><dt>Number of Folders</dt><dd>${storage.folders}</dd><dt>Objects Stored</dt><dd>${storage.stored}</dd><dt>Objects Archived</dt><dd>${storage.archived}</dd><dt>Encrypted</dt><dd>${storage.encrypted}</dd></dl><h4>Logins</h4><dl><dt>SMTP</dt><dd>${logins.smtp}</dd><dt>POP</dt><dd>${logins.pop}</dd><dt>IMAP</dt><dd>${logins.imap}</dd><dt>Web</dt><dd>${logins.web}</dd></dl><h4>Messages</h4><dl><dt>Received</dt><dd>${messages.received}</dd><dt>Sent</dt><dd>${messages.sent}</dd></dl><h4>Transfer</h4><dl><dt>Bytes Sent</dt><dd>${transfer.sent}</dd><dt>Bytes Received</dt><dd>${transfer.received}</dd></dl><h4>Email</h4><dl><dt>Address</dt><dd>${email.address}</dd></dl><h4>Blocked</h4><dl><dt>Spam</dt><dd>${blocked.spam}</dd><dt>Bounced</dt><dd>${blocked.bounced}</dd></dl>',
'messageBody': '{{if html}}{{html html}}{{else}}<p>${text}</p>{{/if}}',
'messageHeader': '<dl class="header-info"><dt>From</dt><dd class="from">${from}</dt><dt>Subject</dt><dd class="subject">${subject}</dd><dt>Date</dt><dd class="date">${date}</dd><dt>To</dt><dd class="to">${to}</dd>{{if cc}}<dt>CC</dt><dd class="cc">${cc}</dd>{{/if}}{{if bcc}}<dt>BCC</dt><dd class="bcc">${bcc}</dd>{{/if}}{{if sender}}<dt>Sender</dt><dd class="sender">${sender}</dd>{{/if}}<dt>Reply-To</dt><dd class="replyto">${replyto}</dd></dl>',
'messageInfo': '<dl class="source"><dt>Source IP</dt><dd class="source-ip">${source.ip}</dd><dt>DNS</dt><dd class="dns">${source.dns}</dd><dt>Reputation</dt><dd class="reputation">${source.reputation}</dd></dl><dl class="security"><dt>Received Securely</dt>{{if security.secure}}<dd class="secure true">Yes</dd>{{else}}<dd class="secure false">No</dd>{{/if}}<dt>SPF</dt>{{if security.spf}}<dd class="spf true">Yes</dd>{{else}}<dd class="spf false">No</dd>{{/if}}<dt>DKIM</dt>{{if security.dkim}}<dd class="dkim true">Yes</dd>{{else}}<dd class="dkim false">No</dd>{{/if}}</dl><div class="server"><p class="date">${server.date}</p><dl><dt>Remote Images Available</dt>{{if server.images}}<dd class="remote-images true">Yes</dd>{{else}}<dd class="remote-images false">No</dd>{{/if}}</dl><p class={{if server.warnings}}"warnings"{{else}}"no-warnings"{{/if}}>Warnings</p></div>',
'messageList': '<tr><td><input type="checkbox" name="check-${messageID}" value="${messageID}" /></td><td>${messageID}</td><td>{{if flag.seen}}<span>Read</span>{{/if}}</td><td>{{if flag.answered}}<span>Replied</span>{{/if}}</td><td>{{if flag.flagged}}<span>Starred</span>{{/if}}</td><td>{{if attachment}}<span>Attachment</span>{{/if}}</td><td>${from}</td><td>${to}</td><td>${addressedTo}</td><td>${replyTo}</td><td>${returnPath}</td><td>${carbon}</td><td>${subject}</td><td>${utc}</td><td>${date}</td><td>${arrivalUtc}</td><td>${arrivalDate}</td><td>${bytes}</td><td>${size}</td><td>${snippet}</td><td>{{each tags}}<div class="tag"><span class="${$value.slug} left">${$value.name}</span><span class="right">✕</span></div>{{/each}}</td></tr>',
'meta': '<dl class="user"><dt>User Number</dt><dd class="user-number">${userID}</dd><dt>Client IP</dt><dd class="client-ip">${clientIP}</dd><dt>Browser</dt><dd class="browser">${browser}</dd><dt>Location</dt><dd class="location">${location}</dd><dt>Timezone</dt><dd class="timezone">${timezone}</dd><dt>Reputation</dt><dd class="reputation">${reputation}</dd><dt>Plan</dt><dd class="plan">${plan}</dd><dt>Quota</dt><dd class="quota">${quota}</dd></dl><dl class="browser"><dt>Cookies</dt>{{if cookies}}<dd class="cookies true">Yes</dd>{{else}}<dd class="cookies false">No</dd>{{/if}}<dt>Javascript</dt>{{if javascript}}<dd class="javascript true">Yes</dd>{{else}}<dd class="javascript false">No</dd>{{/if}}<dt>Stylesheets</dt>{{if stylesheets}}<dd class="stylesheets true">Yes</dd>{{else}}<dd class="stylesheets false">No</dd>{{/if}}<dt>Connection</dt>{{if secure}}<dd class="connection secure">Secure</dd>{{else}}<dd class="connection insecure">Insecure</dd>{{/if}}<dt>Portal Version</dt><dd class="version">${version}</dd></dl><a class="close" href="#close-meta-box">Close</a><a class="button view-logs" href="#view-logs">View Logs</a>',
'options': '<option {{if type}}class="${type}"{{/if}} {{if def}}selected="selected"{{/if}} value="${value}">${name}</option>',
'scrapeList': '<tr id="contact-${contactID}"><td>{{if name}}${name}{{/if}}</td><td>{{if email}}${email}{{/if}}</td></tr>',
'settingsIdentity': '<h3>Identity</h3><div><dl><dt>Display Name</dt><dd class="name">${name}</dd></dl><p class="help-text">Help text here</p><div class="actions"><a class="help" href="#help">Help</a></div></div><div><dl><dt>First Name</dt><dd class="first">${first}</dd></dl><p class="help-text">Help text here</p><div class="actions"><a class="help" href="#help">Help</a></div></div><div><dl><dt>Last Name</dt><dd class="last">${last}</dd></dl><p class="help-text">Help text here</p><div class="actions"><a class="help" href="#help">Help</a></div></div><div class="removable"><dl><dt>Website</dt><dd class="website">${website}</dd></dl><p class="help-text">Help text here</p><div class="actions"><a class="help" href="#help">Help</a></div></div></dl>',
'skiplink': '<a class="sr-hidden" href="#${skiplink.href}">${skiplink.text}</a>',
'tab': '<div class="tab{{if permatab}} permatab{{/if}}"> <h3>${title}</h3> {{tmpl "skiplink"}} {{if !permatab}}<a class="close" title="Close this tab" href="#close-tab">Close</a> {{/if}}</div>',
'tagsList': '<li><a class="${slug}" href="${href}">${name}</a></li>'
};
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
/**
 * view.js
 *
 * Contains all magma views
 */
var magma = magma || {};

magma.view = (function() {
    // private variables and methods

    /*
     * grab html blocks and return jquery object
     *
     * @param block     name of html block
     * @return          returns jquery object of html block
     */
    var getBlock = function(block) {
        if(!magma.blocks[block]) {
            throw new Error('getBlock: no block named ' + block);
        }

        return $(magma.blocks[block]);
    };

    /**
     * Convert number of bytes into human readable format
     *
     * @source https://github.com/codeaid/snippets/blob/master/js/functions/bytesToSize.js
     *
     * @param integer bytes Number of bytes to convert
     * @param integer precision Number of digits after the decimal separator
     * @return string
     */
    var bytesToSize = function(bytes, precision) {
        var kilobyte = 1024;
        var megabyte = kilobyte * 1024;
        var gigabyte = megabyte * 1024;
        var terabyte = gigabyte * 1024;

        if ((bytes >= 0) && (bytes < kilobyte)) {
            return bytes + ' B';
        } else if ((bytes >= kilobyte) && (bytes < megabyte)) {
            return (bytes / kilobyte).toFixed(precision) + ' KB';
        } else if ((bytes >= megabyte) && (bytes < gigabyte)) {
            return (bytes / megabyte).toFixed(precision) + ' MB';
        } else if ((bytes >= gigabyte) && (bytes < terabyte)) {
            return (bytes / gigabyte).toFixed(precision) + ' GB';
        } else if (bytes >= terabyte) {
            return (bytes / terabyte).toFixed(precision) + ' TB';
        } else {
            return bytes + ' B';
        }
    };

    // check all boxes for a page when heading checkbox clicked
    var closeControlsTimeout = 500;

    /*
     * create datatables checkbox controls for a dataTable
     *
     * @param controlsOpen  boolean state of checkbox controls box
     * @param dataTable     jquery object of data table
     * @param workspaceID   workspaceID dataTable is in
     */
    var checkboxControls = function(controlsOpen, dataTable, workspaceID) {
        return function(event) {
            if(!controlsOpen) {
                controlsOpen = true;

                var controls = getBlock('checkboxControls'),
                    removeTimer;

                var removeControls = function() {
                    controls.fadeOut(function() {
                        controls.remove();
                        controlsOpen = false;
                    });
                };

                var timedRemove = function() {
                    removeTimer = setTimeout(function() {
                        removeControls();
                    }, closeControlsTimeout);
                };

                var clearCheckboxes = function() {
                    var checkboxes = $('input[type="checkbox"]', dataTable.fnGetNodes());

                    checkboxes.each(function() {
                        this.checked = false;
                    });
                };

                controls.find('.none').click(function(event) {
                    event.preventDefault();
                    clearCheckboxes();
                    removeControls();
                });

                controls.find('.page').click(function(event) {
                    event.preventDefault();
                    clearCheckboxes();

                    var checkboxes = dataTable.find('tbody').find('input[type="checkbox"]');

                    checkboxes.each(function() {
                        this.checked = true;
                    });

                    removeControls();
                });

                controls.find('.all').click(function(event) {
                    event.preventDefault();

                    var checkboxes = $('input[type="checkbox"]', dataTable.fnGetNodes());

                    checkboxes.each(function() {
                        this.checked = true;
                    });

                    removeControls();
                });

                timedRemove();

                controls
                    .css({'top': '46px', 'left': event.pageX + 'px'})
                    .mouseenter(function() {
                        clearTimeout(removeTimer);
                    })
                    .mouseleave(function() {
                        timedRemove();
                    })
                    .appendTo('#workspace-' + workspaceID);
            }

            // prevents column sort
            return false;
        };
    };

    // get messageIDs of checked messages
    var getCheckedIDs = function(dataTable) {
        if(!dataTable) {
            throw new Error('view.messagesList.getChecked: dataTable not initialized yet');
        }

        var a = [];
        $('input:checked', dataTable.fnGetNodes()).each(function(i, v) {
            a.push(parseInt($(v).val(), 10));
        });
        return a;
    };

    // register a datatables sort function for checkboxes
    (function() {
        $.fn.dataTableExt.afnSortData['checkbox'] = function(oSettings, iColumn) {
            var aData = [];
            $('td:eq(' + iColumn + ') input', oSettings.oApi._fnGetTrNodes(oSettings)).each(function() {
                aData.push(this.checked === true ? '1' : '0');
            });
            return aData;
        };
    }());

    // views
    return {
        /*** alrt ***/
        alrt: {
            link: function(alertModel) {
                var link = getBlock('chromeAlert');

                link.click(function(event) {
                    event.preventDefault();
                    if(!alertModel.isOpen()) {
                        alertModel.open();
                    }
                });

                return {
                    root: link
                };
            },

            box: function(alertModel) {
                var location;

                var load = function() {
                    if(!location) {
                        throw new Error('alert.box.load: no location set');
                    }

                    alertModel.getAlerts();
                };

                alertModel.addObserver('open', load);

                alertModel.addObserver('loaded', function(data) {
                    var tmplModel = magma.model.tmpl();

                    tmplModel.addObserver('completed', function(alerts) {
                        var box = getBlock('alertContainer'),
                            overlay = getBlock('overlay');

                        box
                            .prepend(alerts)
                            .appendTo(location)
                            .reveal()
                            .delegate('a', 'click', function(event) {
                                event.preventDefault();

                                overlay.fadeOut(magma.animationSpeed, function() {
                                    $(this).remove();
                                });

                                $(this).parent().fadeOut(magma.animationSpeed, function() {
                                    $(this).remove();
                                });
                            });

                        overlay.appendTo('body').fadeIn(magma.animationSpeed);
                    });

                    tmplModel.fillTmpl('alert', data);
                });

                return {
                    setLocation: function(loc) {
                        location = loc;
                        return this;
                    }
                };
            }
        },

        /*** chrome ***/
        chrome: function(globalNavModel) {
            var container = getBlock('chrome');

            // TODO: need to grab email from loaded settings
            container.find('#account ul')
                .before('<p>magma@lavabit.com</p>');

            container.find('#global-nav').find('ul a').click(function(event) {
                event.preventDefault();
                globalNavModel.openLocation($(this).attr('class'));
            });

            // object contains location and initialization
            globalNavModel.addObserver('open', function(o) {
                // save last active workspace if navigating away from mail
                if(globalNavModel.isOpen('mail') && o.loc !== 'mail') {
                    magma.workspacesModel.forEach(function(workspace) {
                        if(workspace.isActive()) {
                            magma.workspacesModel.saveLastActive(workspace);
                        }
                    });
                }

                if(!globalNavModel.isOpen(o.loc)) {
                    magma.bootstrap[o.loc](o.init);
                }
            });

            return {
                root: container
            };
        },

        /*** compose ***/
        compose: function(composeModel, toolsModel, tabModel) {
            var container = getBlock('composing'),
                form = container.find('form'),
                from = form.find('#from'),
                to = form.find('#to'),
                subject = form.find('#subject'),
                body = form.find('#body'),
                defaultTitle = tabModel.getTitle(),
                cc = getBlock('composingCC'),
                bcc = getBlock('composingBCC'),
                attachments = getBlock('composingAttachments');

            // ui button for browse
            attachments.find('.browse').button();

            // add bcc
            toolsModel.addObserver('ccClicked', function() {
                // TODO: unique ids
                if(cc.is(':visible')) {
                    cc.remove();
                } else {
                    cc.insertAfter(to.parents('.field-wrapper'));
                }
            });

            // add bcc
            toolsModel.addObserver('bccClicked', function() {
                // TODO: unique ids
                if(bcc.is(':visible')) {
                    bcc.remove();
                } else {
                    bcc.insertBefore(subject.parents('.field-wrapper'));
                }
            });

            // open attachments bar
            toolsModel.addObserver('attachClicked', function() {
                if(attachments.is(':visible')) {
                    attachments.remove();
                } else {
                    attachments.insertAfter(container.find('.composing-header')).slideDown();
                }
            });

            // simulate attachments not finished
            // TODO: remove unfinished attach on send proto
            toolsModel.addObserver('sendClicked', function() {
                if(attachments.is(':visible') && !attachments.hasClass('unfinished-warning')) {
                    attachments.addClass('unfinished-warning');
                    attachments.append('<div><p class="warning">Your attachments have not finished uploading yet!</p></div>');
                }
            });

            // unique ids
            form.find('input, select, textarea').each(function() {
                var id = $(this).attr('id')
                if(id) {
                    $(this).attr('id', id + '-' + composeModel.getComposeID());
                    form.find('label').filter('#' + id).attr('for', $(this).attr('id'));
                }
            });

            // fill in from drop down
            var aliasModel = magma.model.aliases();

            aliasModel.addObserver('loaded', function(data) {
                var tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(options) {
                    options.appendTo(from);
                });

                tmplModel.fillTmpl('options', data);
            });

            aliasModel.loadAliases({options: true});

            // update tab on subject change
            subject.bind('keyup blur', function() {
                if($(this).val()) {
                    tabModel.setTitle($(this).val());
                } else {
                    tabModel.setTitle(defaultTitle);
                }
            });

            return {
                root: container
            };
        },

        /*** containers ***/
        container: function(containerName) {
            var container = getBlock(containerName);

            return {
                root: container
            };
        },

        /*** contactsList ***/
        contactList: function(contactListModel, toolsModel, quickSearch, workspaceID) {
            var container = getBlock('contactsContainer'),
                list = container.find('.contacts'),
                contactInfoModel = magma.model.contactInfo(),
                contactsTable,
                checkboxCtrls,
                controlsOpen = false,
                showGravatars = false;

            // init and hide contact info view
            magma.view.contactInfo(contactInfoModel).root.insertBefore(list).css('display', 'none');

            // open contact info when clicked
            container.find('tbody').delegate('tr', 'click', function() {
                var id = $(this).attr('id');
                id = parseInt(id.substr(id.search(/\d/)), 10);

                contactInfoModel.loadContact(id);
            });

            // prevent row click when checkbox clicked
            container.find('tbody').delegate('input[type="checkbox"]', 'click', function(event) {
                event.stopPropagation();
            });

            // tools actions
            toolsModel.addObserver('newClicked', contactInfoModel.newContact);
            toolsModel.addObserver('editClicked', contactInfoModel.editContact);
            toolsModel.addObserver('deleteClicked', function() {
                contactListModel.deleteContacts(getCheckedIDs(contactsTable));
            });

            contactListModel.observeOnce('loaded', function(data) {
                var tmplModel = magma.model.tmpl();

                // show gravatars if any row has one
                for(var i in data) {
                    if(data[i].img) {
                        showGravatars = true;
                        break;
                    }
                }

                tmplModel.addObserver('completed', function(contactList) {
                    contactList.appendTo(container.find('tbody'));

                    contactsTable = list.dataTable({
                        'sDom': 'tf',
                        'sScrollY': container.height() - magma.tableHeaderHeight,
                        'sScrollX': '100%',
                        'aaSorting': [[1, 'desc']],
                        'aoColumndDefs': [
                            {sWidth: '40px', aTargets: [0]}
                        ],
                        'aoColumns': [
                            {'sClass': 'checkbox', 'sSortDataType': 'checkbox'}, // 0
                            {'sClass': 'avatar', 'bVisible': showGravatars}, // 1
                            {'sClass': 'name'}, // 2
                            {'sClass': 'email'}, // 3
                            {'sClass': 'company'} // 4
                        ],
                        'iDisplayLength': 25, // initial pagination length
                        'sPaginationType': 'full_numbers',
                        'fnDrawCallback': function() {
                            $('.checkbox-controls-button').click(checkboxCtrls);
                        },
                        'oLanguage': {
                            'sSearch': '',
                            'sEmptyTable': 'No contacts in this folder'
                        }
                    });

                    // checkboxControls returns a function
                    // used in above draw callback
                    checkboxCtrls = checkboxControls(controlsOpen, contactsTable, workspaceID);

                    // update wrapper class when info show/hide
                    // also enable/disable edit tool
                    var wrapper = contactsTable.parents('.dataTables_wrapper');

                    contactInfoModel.addObserver('show', function(type) {
                        wrapper.addClass('info');
                        if(type !== 'new') {
                            toolsModel.enableTool('edit');
                        }
                        contactsTable.fnAdjustColumnSizing();
                    });

                    contactInfoModel.addObserver('hide', function() {
                        wrapper.removeClass('info');
                        toolsModel.disableTool('edit');
                        contactsTable.fnAdjustColumnSizing();
                    });

                    // replace toolbar quick search with dataTables search
                    var filter = container.find('.dataTables_filter'),
                        filterInput = filter.find('input'),
                        label = quickSearch.find('form label'),
                        input = quickSearch.find('form input:text');

                    input.replaceWith(filter.detach());
                    label.detach().prependTo(filter);
                    filterInput.attr('id', label.attr('for'));
                    filterInput.fillWatermarks(label);

                    // clear input action
                    quickSearch.find('.clear').click(function(event) {
                        event.preventDefault();
                        filterInput.val('');
                        contactsTable.fnFilter('');
                    });

                    // fix columns
                    // TODO: better column fix
                    setTimeout(function() {
                        contactsTable.fnAdjustColumnSizing();
                    }, 50);

                    // observe subsequent folder loads
                    contactListModel.addObserver('loaded', function(data) {
                        var tmplModel = magma.model.tmpl();

                        // show gravatars if any row has one
                        showGravatars = false;
                        for(var i in data) {
                            if(data[i].img) {
                                showGravatars = true;
                                break;
                            }
                        }

                        tmplModel.addObserver('completed', function(contacts) {
                            var contactData = [];

                            $.each(contacts, function(i, contact) {
                                var row = [];

                                $(contact).find('td').each(function() {
                                    row.push($(this).html());
                                });

                                contactData.push(row);
                            });

                            // empty the table
                            // don't redraw table
                            if(contactData.length) {
                                // clear and don't redraw
                                contactsTable.fnClearTable(false);
                                // hide or show gravatars
                                contactsTable.fnSetColumnVis(1, showGravatars);
                                contactsTable.fnAddData(contactData);
                            } else {
                                contactsTable.fnClearTable();
                            }
                        });

                        tmplModel.fillTmpl('contactList', data);
                    });
                });

                tmplModel.fillTmpl('contactList', data);
            });

            contactListModel.load('all');

            return {
                root: container
            };
        },

        /*** contactInfo ***/
        contactInfo: function(contactInfoModel) {
            var container = getBlock('contactInfo'),
                infoSection = container.find('.contact-info');

            var clearInfo = function() {
                infoSection
                    .children()
                    .not('.close')
                    .remove();
                return infoSection;
            };

            var updateInfo = function(info, type) {
                clearInfo();
                infoSection.prepend(info);
                if(!contactInfoModel.isOpen()) {
                    contactInfoModel.showInfo(type);
                }
                return infoSection;
            };

            // show contact info when row clicked
            contactInfoModel.addObserver('loaded', function(data) {
                var tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(info) {
                    updateInfo(info);
                });

                tmplModel.fillTmpl('contactInfo', data);
            });

            contactInfoModel.addObserver('new', function() {
                var tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(info) {
                    var addField = getBlock('contactAddField');
                    addField.submit(function(event) {
                        event.preventDefault();
                        // TODO: add field to info
                    });
                    updateInfo(info, 'new').append(addField);
                });

                tmplModel.fillTmpl('contactInfo', {name: 'Full Name', email: {primary: 'Address'}});
            });

            contactInfoModel.addObserver('edit', function() {
                var addField = getBlock('contactAddField');
                addField.submit(function(event) {
                    event.preventDefault();
                    // TODO: add field to info
                });
                infoSection.append(addField);
            });

            contactInfoModel.addObserver('show', function() {
                container.show();
            });

            contactInfoModel.addObserver('hide', function() {
                container.hide();
            });

            container.find('.close').click(function(event) {
                event.preventDefault();
                contactInfoModel.hideInfo();
            });

            return {
                root: container
            };
        },

        /*** folders ***/
        folders: function(folderModel, displayModel, workspaceID, showOptions, readyCallback, observeOnce) {
            // initialize add/edit buttons and observe loaded once options
            showOptions = typeof showOptions === 'boolean' ? showOptions : true;
            observeOnce = typeof observeOnce === 'boolean' ? observeOnce : true;

            var container = getBlock(showOptions ? 'folderMenuWithOptions' : 'folderMenu'),
                list = container.find('.folder-list'),
                options,
                addFolder,
                editFolders,
                addEditableFolder,
                renaming = false,
                addOpen = false,
                editOpen = false,
                menuID;

            menuID = container.attr('id', container.attr('class') + '-' + workspaceID).attr('id');

            var dragDrop = function(folders) {
                folders.draggable({
                    containment: menuID + ' .folder-scroll-wrapper',
                    distance: 5, // distance in pixels mouse must move before initiating drag
                    opacity: 0.2,
                    revert: 'invalid', // reverts back to original position if not valid drop
                    axis: 'y',
                    scrollSensitivity: 20, // distance in pixels from edge of screen to start scroll
                    stack: '[id^="folder"]',
                    scope: 'folders'
                })
                .droppable({
                    scope: 'folders',
                    hoverClass: 'hover',
                    drop: function(event, ui) {
                        var targetID = parseInt($(this).attr('id').match(/\d+/), 10),
                            sourceID = parseInt(ui.draggable.attr('id').match(/\d+/), 10);

                        folderModel.moveFolder(sourceID, targetID);
                    }
                });
            };

            // load messages of folder when clicked if not current folder
            list.delegate('.folder', 'click', function(event) {
                event.preventDefault();

                // make sure this isn't the current open folder
                if(!$(this).hasClass('open')) {
                    var open = list.find('.open'),
                        folder = $(this).parent();

                    // close other folder if open
                    if(open.length) {
                        open.removeClass('open');
                    }

                    folder.addClass('open');

                    // get folder id
                    var id = folder.attr('id');
                    id = parseInt(id.substr(id.search(/\d/)), 10);

                    displayModel.load(id);
                }
            });

            // expand folders with left icon
            list.delegate('.expander', 'click', function(event) {
                event.preventDefault();

                var id = parseInt($(this).parent().attr('id').match(/\d+/), 10),
                    subfolders = list.find('#subfolders-' + id);

                if(subfolders.hasClass('collapsed')) {
                    $(this).addClass('expand');
                    subfolders.removeClass('collapsed');
                } else {
                    $(this).removeClass('expand');
                    subfolders.addClass('collapsed');
                }
            });

            // update changes
            folderModel.addObserver('added', function(data) {
                var tmplModel = magma.model.tmpl();

                tmplModel.observeOnce('completed', function(folder) {
                    dragDrop(folder);

                    // pass to edit if it's open to append editing buttons
                    if(editOpen) {
                        addEditableFolder(folder);
                    } else {
                        list.append(folder);
                    }
                });

                tmplModel.fillTmpl('foldersExpandable', data);
            });

            folderModel.addObserver('addedError', function(error) {
                magma.dialog.die(error);
            });

            folderModel.addObserver('removed', function(folder) {
                var id = folder.folderID,
                    parentSubfolders = list.find('#subfolders-' + folder.parentID),
                    parentFolder = list.find('#folder-' + folder.parentID);

                // remove this folder and it's subfolders
                list.find('#folder-' + id + ', #subfolders-' + id).remove();

                // remove parent's toggle if this was the last subfolder
                if(parentSubfolders.length && !parentSubfolders.find('ul').children().length) {
                    parentFolder
                        .find('.expander')
                        .remove()
                        .end()
                        .removeClass('expandable');
                }
            });

            folderModel.addObserver('removedError', function(error) {
                magma.dialog.die(error);
            });

            folderModel.addObserver('renamed', function(folder) {
                list.find('#folder-' + folder.folderID).find('.folder').text(folder.name);
            });

            folderModel.addObserver('renamedError', function(error) {
                magma.dialog.die(error);
            });

            folderModel.addObserver('moved', function(ids) {
                var source = list.find('#folder-' + ids.sourceFolderID),
                    sourceSubfolders = $('#subfolders-' + ids.sourceFolderID),
                    target = list.find('#folder-' + ids.targetFolderID),
                    targetSubfolders = list.find('#subfolders-' + ids.targetFolderID).children(),
                    parentList,
                    parent;

                // look for parent of source if it's not a root folder
                if(!list.children().filter(source).length) {
                    parentList = source.parent().parent();
                    parent = $('#folder-' + parseInt(parentList.attr('id').match(/\d+/), 10));
                }

                // make sure target is not child of source
                if(sourceSubfolders.length && sourceSubfolders.find(target).length) {
                    magma.dialog.message("I'm afraid I can't let you do that.");

                // check if parent is the target (move to same folder)
                } else if(!parent || !parent.is(target)) {
                    // don't detatch above to get parent first
                    source.detach();
                    sourceSubfolders.detach();

                    // remove parent subfolders if this is the last one
                    if(parentList && !parentList.find('ul').children().length) {
                        parentList.remove();
                        parent.removeClass('expandable').find('.expander').remove();
                    }

                    // folder already contains subfolder list
                    if(targetSubfolders.length) {
                        targetSubfolders.append($.merge(source, sourceSubfolders));

                    // need to crate a subfolder list
                    } else {
                        var targetID = parseInt(target.attr('id').match(/\d+/), 10),
                            subfolders = $('<li/>')
                                .attr('id', 'subfolders-' + targetID)
                                .append('<ul/>');

                        // add expander to target
                        target
                            .prepend('<a class="expander expand">Toggle</a>')
                            .addClass('expandable');

                        subfolders.find('ul').append($.merge(source, sourceSubfolders));
                        subfolders.insertAfter(target);
                    }
                }

                // reset position
                source.css('top', 0);
            });

            folderModel.addObserver('movedError', function(obj) {
                list.find('#folder-' + obj.folderID).attr('top', 0);
                magma.dialog.die(obj.error);
            });

            if(showOptions) {
                options = getBlock('folderOptions').appendTo(container)
                    .find('input').each(function() {
                        var id = $(this).attr('id');
                        $(this)
                            .siblings('[for="' + id + '"]')
                            .attr('for', id + '-' + workspaceID);
                        $(this).attr('id', id + '-' + workspaceID);
                    })
                    .end();
                addFolder = getBlock('folderOptionsAdd')
                    .find('input[type="text"], select').each(function() {
                        $(this).attr('id', $(this).attr('id') + '-' + workspaceID);
                    })
                    .end();
                editFolders = getBlock('folderOptionsEdit');

                var add = options.find('.add'),
                    edit = options.find('.edit'),
                    addBoxTimer,
                    addBoxTimeout = 3000,
                    renameTimer,
                    renameTimeout = 1000;

                var timedRemove = function(callback, timer, timeout) {
                    timer = setTimeout(function() {
                        callback();
                    }, timeout);
                };

                // ui toggles
                add.button({icons: {primary: 'add'}});
                edit.button({icons: {secondary: 'edit'}});

                // delegate editing actions to list
                list.delegate('.rename', 'click', function(event) {
                    var folder = $(this).parents('.toggle'),
                        id = parseInt(folder.attr('id').match(/\d+/), 10),
                        link = folder.find('.folder'),
                        oldName = link.text(),
                        edit = folder.find('.edit'),
                        rename = getBlock('folderOptionsRename'),
                        renameRemove = function() {
                            clearTimeout(renameTimer);
                            rename.remove();
                            folder.removeClass('renaming').children().show();
                        };

                folder
                    .children()
                    .hide()
                    .end()
                    .addClass('renaming')
                    .prepend(rename)
                    .find('.rename-folder')
                    .submit(function(event) {
                        event.preventDefault();
                        var newName = rename.find('input[type="text"]').val();
                        if(newName !== oldName) {
                            folderModel.renameFolder(rename.find('input[type="text"]').val(), id);
                        }
                        renameRemove();
                    })
                    .find('input[type="text"]')
                    .val(oldName)
                    .focus()
                    .select()
                    .focusin(function() {
                        clearTimeout(renameTimer);
                    })
                    .focusout(function() {
                        timedRemove(renameRemove, renameTimer, renameTimeout);
                    });
                })
                .delegate('.remove', 'click', function(event) {
                    var id = parseInt($(this).parents('.toggle').attr('id').match(/\d+/), 10);
                    folderModel.removeFolder(id);
                });

                var removeAddBox = function() {
                    addFolder.remove();
                    addOpen = false;
                    add.siblings('[for="' + add.attr('id') + '"]')
                        .removeClass('ui-state-active');
                    clearTimeout(addBoxTimer);
                    list.parent().removeClass('editing');
                };

                addEditableFolder = function(folder) {
                    // cloning prevents odd behavior of stealing dom element from
                    // the last folder the buttons were appended to
                    folder.append(editFolders.clone()).appendTo(list);
                };

                add.click(function(event) {
                    event.preventDefault();

                    if(!addOpen) {
                        addOpen = true;
                        list.parent().addClass('editing');

                        options.before(addFolder
                            .delegate('input[type="text"]', 'focusin', function() {
                                clearTimeout(addBoxTimer);
                            })
                            .delegate('input[type="text"]', 'focusout', function() {
                                timedRemove(removeAddBox, addBoxTimer, addBoxTimeout);
                            })
                            .submit(function(event) {
                                event.preventDefault();

                                var name = $(this).find('input[type="text"]').val();

                                if(!name) {
                                    magma.dialog.message('Please enter a folder name.');
                                } else {
                                    folderModel.addFolder(name);
                                    removeAddBox();
                                }
                            })
                        );

                        addFolder.find('input[type="text"]').focus().select();
                    } else {
                        removeAddBox();
                    }
                });

                edit.click(function(event) {
                    event.preventDefault();

                    if(!editOpen) {
                        editOpen = true;
                        list.find('.toggle').append(editFolders);
                    } else {
                        editOpen = false;
                        list.find('.edit').remove();
                    }
                });
            }

            var showFolders = function(data) {
                var permanentTmplModel = magma.model.tmpl(),
                    customTmplModel = magma.model.tmpl();

                // empty list if changing context
                list.empty();

                permanentTmplModel.addObserver('completed', function(folders) {
                    folders.prependTo(list);
                });

                customTmplModel.addObserver('completed', function(folders) {
                    folders.appendTo(list);

                    // draggable custom folders
                    dragDrop($.merge(folders.filter('[id^="folder"]'), folders.find('[id^="folder"]')));
                });

                var context = folderModel.getContext();
                context = context.replace(context[0], context[0].toUpperCase());

                permanentTmplModel.fillTmpl('folders' + context, data.permanent);
                customTmplModel.fillTmpl('foldersExpandable', data.custom);

                // hacky callback to allow permanent folders to initialize
                // before list loads messages using permafolder id
                // see permaFolders at the top of model.js
                if(readyCallback) {
                    readyCallback();
                }
            };

            // get folder list
            if(observeOnce) {
                folderModel.observeOnce('loaded', function(data) {
                    showFolders(data);
                });
            } else {
                folderModel.addObserver('loaded', function(data) {
                    showFolders(data);
                });
            }

            // initialize folder list
            folderModel.loadFolders();

            return {
                root: container,
                list: list
            };
        },

        /*** help ***/
        help: function(helpModel) {
            var container = getBlock('help'),
                self;

            helpModel.addObserver('updateCategories', function(data) {
                var tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(categories) {
                    self.categories.empty().append(categories);
                });

                tmplModel.fillTmpl('helpList', data);
            });

            container.delegate('li', 'click', function() {
                var id = parseInt($(this).attr('id').match(/\d+/), 10);
                helpModel.updateTopics(id);
            });

            // categories load topic lists
            helpModel.addObserver('updateTopics', function(data) {
                var tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(topics) {
                    self.topics.empty().append(topics);
                });

                tmplModel.fillTmpl('helpList', data);
            });

            // load a page from topic id when clicked
            container.delegate('li', 'click', function() {
                var id = parseInt($(this).attr('id').match(/\d+/), 10);
                helpModel.updatePage(id);
            });

            // show page when topic clicked
            helpModel.addObserver('updatePage', function(page) {
                self.page.empty().html(page);
            });

            self = {
                root: container,
                categories: container.find('.help-categories').find('ul'),
                topics: container.find('.help-topics').find('ul'),
                page: container.find('.help-page')
            };
            return self;
        },

        /*** login ***/
        login: function(authModel) {
            var container = getBlock('loginContainer'),
                form = container.find('form'),
                inputs = form.find('#username, #password'),
                username = inputs.eq(0),
                password = inputs.eq(1),
                message = $('<div id="login-error"/>').append('<p/>'),
                promptUsername,
                promptPassword,
                hideError,
                displayError,
                handlingLogin = false;

            var gotoLoading = function() {
                container.fadeOut(magma.animationSpeed, function() {
                    // kick off loading screen
                    magma.bootstrap.loading();
                });
            };

            var gotoLocked = function(message) {
                container.fadeOut(magma.animationSpeed, function() {
                    // kick off locked screen
                    magma.bootstrap.locked(message);
                });
            };

            authModel.addObserver('success', gotoLoading);
            authModel.addObserver('locked', gotoLocked);

            // [DEV]
            username.val('magma');
            password.val('test');
            // [/DEV]

            // handle login submit
            form.find('.button').click(function(event) {
                event.preventDefault();
                hideError(function() {
                    // break out and allow another attempt if failed
                    authModel.observeOnce('failed', function() {
                        handlingLogin = false;
                    });

                    // prevent double clicking
                    if(!handlingLogin) {
                        handlingLogin = true;

                        authModel.login(username.val(), password.val());
                    }
                });
            });

            promptUsername = function() {
                username.focus();
            };

            promptPassword = function() {
                password.focus();
            };

            authModel.addObserver('incorrect', promptUsername);
            authModel.addObserver('noUsername', promptUsername);
            authModel.addObserver('noPassword', promptPassword);

            inputs.fillWatermarks(form.find('label'));

            hideError = function(callback) {
                if($('#login-error').length) {
                    // slide opposite since absolutely position with bottom
                    $('#login-error').slideUp(magma.animationSpeed, function() {
                        // search instead of 'this' to prevent double clicking removing multiples
                        $('#login-error').remove();
                        if(callback) {
                            callback();
                        }
                    });
                } else {
                    if(callback) {
                        callback();
                    }
                }
            };

            displayError = function(msg) {
                message.find('p').text(msg);

                message
                    .appendTo(container)
                    .hide()
                    .slideDown(magma.animationSpeed); // slide down since absolutely pos bottom
            };

            //authModel.addObserver('attempt', hide);
            authModel.addObserver('failed', displayError);

            return {
                root: container
            };
        },

        /*** logout ***/
        logout: {
            container: function() {
                var container = getBlock('logoutContainer');

                container.find('.login').click(function(event) {
                    event.preventDefault();
                    container.fadeOut(magma.animationSpeed, function() {
                        // go back to login
                        magma.bootstrap.login();
                    });
                });

                return {
                    root: container
                };
            },

            link: function(authModel) {
                var link = $('<li><a class="sign-out" href="#signout">Sign Out</a></li>');

                link.click(function(event) {
                    event.preventDefault();
                    authModel.logout();
                });

                authModel.addObserver('logout', function() {
                    // fade out body
                    $('#main-wrapper').fadeOut(magma.animationSpeed, function() {
                        $(this).remove();
                        magma.bootstrap.logout();
                    });
                });

                return {
                    root: link
                };
            }
        },

        /*** logs ***/
        logs: function(logsModel, folderModel, initialize) {
            var container = getBlock('logs'),
                logsContainer = container.find('.settings');

            initialize = initialize || 'mail';

            if(typeof initialize !== 'string') {
                throw new Error('view.logs: initialize must be given as a string');
            }

            logsModel.addObserver('loaded', function(o) {
                var context = o.context[0].toUpperCase() + o.context.substr(1),
                    data = o.data,
                    tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(html) {
                    if(o.context.match(/security|contacts|mail/)) {

                        // context capitolized above
                        var table = getBlock('logs' + context),
                            sort = function(context) {
                                switch(context) {
                                    case 'security':
                                        return [{'iDataSort': 1, 'aTargets': [0]}]
                                    break;

                                    case 'contacts':
                                    break;

                                    case 'mail':
                                        return [
                                            {'iDataSort': 6, 'aTargets': [5]},
                                            {'iDataSort': 8, 'aTargets': [7]}
                                        ];
                                    break;
                                }
                            },
                            initSort = function(context) {
                                switch(context) {
                                    case 'security':
                                        return [[0, 'desc']];
                                    break;

                                    case 'contacts':
                                    break;

                                    case 'mail':
                                        return [[0, 'asc']];
                                    break;
                                }
                            },
                            cols = function(context) {
                                switch(context) {
                                    case 'security':
                                        return [
                                            {'sClass': 'utc', 'bVisible': false},
                                            {'sClass': 'time'},
                                            {'sClass': 'type'},
                                            {'sClass': 'severity'},
                                            {'sClass': 'ip'},
                                            {'sClass': 'protocol'}
                                        ];
                                    break;

                                    case 'contacts':
                                    break;

                                    case 'mail':
                                        return [
                                            {'sClass': 'queue'}, // 0
                                            {'sClass': 'type'}, // 1
                                            {'sClass': 'from'}, // 2
                                            {'sClass': 'to'}, // 3
                                            {'sClass': 'outcome'}, // 4
                                            {'sClass': 'utc', 'bVisible': false}, // 5
                                            {'sClass': 'time'}, // 6
                                            {'sClass': 'bytes', 'bVisible': false}, // 7
                                            {'sClass': 'size'} // 8
                                        ];
                                    break;
                                }
                            };

                        // add table
                        logsContainer.html(table).addClass('logs');
                        // append rows
                        table.find('tbody').append(html);
                        // init dataTable
                        table.dataTable({
                            'sDom': 't',
                            'sScrollY': container.height() - magma.tableHeaderHeight,
                            'sScrollX': '100%',
                            'aoColumndDefs': sort(o.context),
                            'aaSorting': initSort(o.context),
                            'aoColumns': cols(o.context),
                            'iDisplayLength': 25, // initial pagination length
                            'sPaginationType': 'full_numbers',
                            'oLanguage': {
                                'sSearch': '',
                                'sEmptyTable': 'No messages in this log'
                            }
                        });

                        // expand if mail logs
                        if(o.context === 'mail') {
                            table.delegate('tr', 'click', function() {
                                if($(this).attr('id')) {
                                    if($(this).hasClass('open')) {
                                        table.fnClose(this);
                                        $(this).removeClass('open');
                                    } else {
                                        var id = parseInt($(this).attr('id').match(/\d+/), 10),
                                            messageModel = magma.model.message(id),
                                            row = this;

                                        magma.view.message.info(messageModel, function(details) {
                                            table.fnOpen(row, details.html(), 'message-info message-' + id);
                                            table.find('.message-info.message-' + id).find('.close').click(function(event) {
                                                table.fnClose(row);
                                                $(row).removeClass('open');
                                            });
                                        });

                                        messageModel.loadSections(['info']);
                                        $(this).addClass('open');
                                    }
                                }
                            });
                        }

                        // fix columns
                        // TODO: better column fix
                        setTimeout(function() {
                            table.fnAdjustColumnSizing();
                        }, 50);

                    // just pop it in as is
                    } else {
                        logsContainer.html(html).removeClass('logs');
                    }
                });


                tmplModel.fillTmpl('logs' + context, data);
            });

            logsModel.load(initialize);

            return {
                root: container
            };
        },

        /*** locked ***/
        locked: {
            container: function() {
                var container = getBlock('lockedContainer');

                var gotoLogin = function() {
                    container.fadeOut(magma.animationSpeed, function() {
                        // go back to login
                        magma.bootstrap.login();
                    });
                };

                container.find('.login').click(function(event) {
                    event.preventDefault();
                    gotoLogin();
                });

                return {
                    root: container
                };
            },

            message: function(msg) {
                var message = $('<p/>').text(msg);

                return {
                    root: message
                };
            }
        },

        /*** loading ***/
        loading: {
            container: function(loadingModel) {
                var container = getBlock('loadingContainer');

                var toMain = function() {
                    container.fadeOut(magma.animationSpeed, function() {
                        // remove startup containers
                        $('body').children().remove();

                        // start main
                        magma.bootstrap.main();
                    });
                };

                loadingModel.addObserver('continue', toMain);

                return {
                    root: container
                };
            },

            ad: function(adModel, callback) {
                if(!callback) {
                    throw new Error('loading.ad: must provide a callback');
                }

                var tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(ad) {
                    ad.click(function(event) {
                        event.preventDefault();
                        adModel.clicked();
                    });

                    callback({root: ad});
                });

                tmplModel.fillTmpl('ad', $.extend(adModel.getAd(), {context: "loading"}));
            },

            fact: function(adModel, callback) {
                if(!callback) {
                    throw new Error('loading.fact: must provide a callback');
                }

                var container = $('<div id="loading-fact"/>').append('<h2>Fun Fact</h2>'),
                    tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(fact) {
                    container.append(fact);
                    callback({root: container});
                });

                tmplModel.fillTmpl('fact', adModel.getFact());
            },

            adWarning: function(adModel) {
                // add continue link with filled in href
                var container = getBlock('loadingWarning')
                    .append('<a id="continue" class="button" target="_blank" href="' + adModel.getAdHref() + '">Continue</a>');

                var showWarning = function() {
                    container
                        .filter(':hidden')
                        .show('slide', magma.animationSpeed);
                };

                var hideWarning = function() {
                    container
                        .filter(':visible')
                        .hide('slide', magma.animationSpeed);
                };

                container.find('#cancel').click(function(event) {
                    event.preventDefault();
                    hideWarning();
                });

                container.find('#continue').click(function() {
                    container.css('display', 'none');
                });

                // show warning when add clicked
                adModel.addObserver('clicked', showWarning);

                return {
                    root: container
                };
            },

            waiting: function(loadingModel) {
                var container = getBlock('loadingWaiting');

                // prevent the loading button from doing anything
                container
                    .find('.button')
                    .click(function(event) {
                        event.preventDefault();
                    });

                var showFinished = function() {
                    container.fadeOut(magma.animationSpeed, function() {
                        $(this).remove();
                        magma.bootstrap.loadingFinished(loadingModel);
                    });
                };

                loadingModel.addObserver('finished', showFinished);

                return {
                    root: container
                };
            },

            finished: function(loadingModel) {
                var container = getBlock('loadingFinished');

                container
                    .find('.button')
                    .click(function(event) {
                        event.preventDefault();
                        loadingModel.cont();
                    });

                return {
                    root: container
                };
            }
        },

        /*** message ***/
        message: {
            header: function(messageModel) {
                var container = getBlock('messageHeader'),
                    info,
                    attachments;

                messageModel.addObserver('headerLoaded', function() {
                    var tmplModel = magma.model.tmpl();

                    tmplModel.addObserver('completed', function(header) {
                        header.prependTo(container);
                    });

                    tmplModel.fillTmpl('messageHeader', messageModel.getSection('header'));
                });

                container.find('.extras').find('.info').click(function(event) {
                    event.preventDefault();

                    if(!messageModel.infoIsOpen()) {
                        info = magma.view.message.info(messageModel).root.insertAfter(container).css('display', 'none');
                        messageModel.showInfo();
                    } else {
                        info.effect('highlight');
                    }
                });

                container.find('.extras').find('.scrape').click(function(event) {
                    event.preventDefault();
                    magma.bootstrap.scrape(messageModel.getID());
                });

                container.find('.extras').find('.attachments').click(function(event) {
                    event.preventDefault();

                    if(!messageModel.attachmentsIsOpen()) {
                        attachments = magma.view.message.attachments(messageModel).root;

                        // insert after info if it's open
                        if(messageModel.infoIsOpen()) {
                            attachments.insertAfter(info).css('display', 'none');
                        } else {
                            attachments.insertAfter(container).css('display', 'none');
                        }

                        messageModel.showAttachments();
                    } else {
                        attachments.effect('highlight');
                    }
                });

                return {
                    root: container
                };
            },

            body: function(messageModel) {
                var container = getBlock('messageBody');

                messageModel.addObserver('bodyLoaded', function() {
                    var tmplModel = magma.model.tmpl();

                    tmplModel.addObserver('completed', function(body) {
                        body.appendTo(container);
                    });

                    tmplModel.fillTmpl('messageBody', messageModel.getSection('body'));
                });

                return {
                    root: container
                };
            },

            ad: function(adModel) {
                var container = getBlock('messageAd');

                adModel.addObserver('loaded', function(data) {

                    var tmplModel = magma.model.tmpl();

                    tmplModel.addObserver('completed', function(ad) {
                        ad.appendTo(container);

                        ad.click(function(event) {
                            event.preventDefault();

                            // show warning
                            // magma.bootstrap.adWarning(adModel.getAdHref());
                        });
                    });

                    tmplModel.fillTmpl('ad', data.ad);
                });

                adModel.loadAd();

                return {
                    root: container
                };
            },

            info: function(messageModel, callback) {
                var container = getBlock('messageInfo');

                messageModel.observeOnce('infoLoaded', function() {
                    var tmplModel = magma.model.tmpl();

                    tmplModel.addObserver('completed', function(info) {
                        if(callback) {
                            callback(container.prepend(info));
                        } else {
                            container
                                .prepend(info)
                                .css('display', 'none')
                                .slideDown(magma.animationSpeed);
                        }
                    });

                    tmplModel.fillTmpl('messageInfo', messageModel.getSection('info'));
                });

                messageModel.observeOnce('showInfo', function() {
                    messageModel.loadSections(['info']);
                });

                messageModel.observeOnce('closeInfo', function() {
                    container.slideUp(magma.animationSpeed, function() {
                        $(this).remove();
                    });
                });

                container.find('.close').click(function(event) {
                    event.preventDefault();
                    messageModel.closeInfo();
                });

                return {
                    root: container
                };
            },

            attachments: function(messageModel) {
                var container = getBlock('messageAttachments');

                messageModel.observeOnce('attachmentsLoaded', function() {
                    var attachments = messageModel.getSection('attachments');

                    for(var i in attachments) {
                        container.find('.close').before($('<a/>')
                            .attr('id', 'attachment-' + attachments[i].attachmentID)
                            .attr('href', '#open-attachment')
                            .text(attachments[i].name + ' (' + bytesToSize(attachments[i].size) + ')')
                        );
                    }

                    container.slideDown(magma.animationSpeed);
                });

                messageModel.observeOnce('showAttachments', function() {
                    messageModel.loadSections(['attachments']);
                });

                messageModel.observeOnce('closeAttachments', function() {
                    container.slideUp(magma.animationSpeed, function() {
                        $(this).remove();
                    });
                });

                container.find('.close').click(function(event) {
                    event.preventDefault();
                    messageModel.closeAttachments();
                });

                return {
                    root: container
                };
            }
        },

        /*** messageList ***/
        messageList: function(messageListModel, folderModel, toolsModel, quickSearch, workspaceID) {
            var container = getBlock('messageList'),
                list = container.find('.messages'),
                messageTable,
                messageTableSettings,
                filter,
                checkboxCtrls,
                controlsOpen = false,
                preview = getBlock('previewPane'),
                currentPreview;

            // single click opens in preview pane
            container.find('tbody').delegate('tr', 'click', function() {
                var id = parseInt($(this).attr('id').match(/\d+/), 10);
                if(preview.is(':visible') && currentPreview !== id) {
                    // clear the preview
                    preview.empty();

                    // message model/view
                    var messageModel = magma.model.message(id);
                    magma.view.message.header(messageModel).root.appendTo(preview);
                    magma.view.message.body(messageModel).root.appendTo(preview);
                    messageModel.loadSections(['header', 'body']);

                    currentPreview = id;
                }
            });

            // double click opens a message
            container.find('tbody').delegate('tr', 'dblclick', function() {
                var id = parseInt($(this).attr('id').match(/\d+/), 10);
                magma.bootstrap.read(id);
            });

            // prevent row click when checkbox or tag clicked
            container.find('tbody').delegate('input[type="checkbox"], span', 'click dblclick', function(event) {
                event.stopPropagation();
            });

            // message action listeners
            messageListModel.addObserver('deleted', function(messageIDs) {
                var nodes = messageTable.fnGetNodes(),
                    node;

                $.each(messageIDs, function(i,id) {
                    node = $(nodes).filter('#message-' + id)[0];

                    // second param is callback, third is redraw option
                    // only redraw on last delete
                    messageTable.fnDeleteRow(node, null, i === messageIDs.length - 1);
                });
            });

            messageListModel.addObserver('deletedError', function(errorMessage) {
                magma.dialog.message(errorMessage);
            });

            messageListModel.addObserver('copiedError', function(error) {
                magma.dialog.die(error);
            });

            // remove moved messages
            messageListModel.addObserver('moved', function(messageIDs) {
                var nodes = messageTable.fnGetNodes(),
                    node;

                $.each(messageIDs, function(i,id) {
                    node = $(nodes).filter('#message-' + id)[0];
                    messageTable.fnDeleteRow(node, null, i === messageIDs.length - 1);
                });
            });

            messageListModel.addObserver('flagged', function(msgObj) {
                var tmplModel = magma.model.tmpl(),
                    nodes = messageTable.fnGetNodes(),
                    node;

                tmplModel.observeOnce('completed', function(messages) {
                    $.each(messages, function(i, message) {
                        var row = [],
                            update = i === messages.length - 1,
                            id = parseInt($(message).children().eq(1).text(), 10);

                        $(message).children().each(function() {
                            row.push($(this).html());
                        });

                        node = $(nodes).filter('#message-' + id)[0];

                        // third param is column to update ignored and is ignored since passing entire row
                        // fourth and fifth are for redrawing table and rebuilding search data
                        messageTable.fnUpdate(row, node, null, update, update);
                    });

                    // keep checkboxes checked
                    $(nodes)
                        .filter($.map(msgObj.messageIDs, function(id) {
                            return '#message-' + id;
                        }).join(', '))
                        .find('.checkbox')
                        .children()
                        .prop('checked', true);
                });

                tmplModel.fillTmpl('messageList', msgObj.data);
            });

            messageListModel.addObserver('tagged', function(msgObj) {
                var tmplModel = magma.model.tmpl(),
                    nodes = messageTable.fnGetNodes(),
                    node;

                tmplModel.observeOnce('completed', function(messages) {
                    $.each(messages, function(i, message) {
                        var row = [],
                            update = i === messages.length - 1,
                            id = parseInt($(message).children().eq(1).text(), 10);

                        $(message).children().each(function() {
                            row.push($(this).html());
                        });

                        node = $(nodes).filter('#message-' + id)[0];

                        // third param is column to update ignored and is ignored since passing entire row
                        // fourth and fifth are for redrawing table and rebuilding search data
                        messageTable.fnUpdate(row, node, null, update, update);
                    });

                    // keep checkboxes checked
                    $(nodes)
                        .filter($.map(msgObj.messageIDs, function(id) {
                            return '#message-' + id;
                        }).join(', '))
                        .find('.checkbox')
                        .children()
                        .prop('checked', true);
                });

                tmplModel.fillTmpl('messageList', msgObj.data);
            });

            messageListModel.addObserver('flaggedError', function() {
                magma.dialog.die('Sorry, there was an error.');
            });

            // tool listeners
            toolsModel.addObserver('replyClicked', function() {
                var ids = getCheckedIDs(messageTable);
                // bootstrap reply
            });

            toolsModel.addObserver('replyAllClicked', function() {
                var ids = getCheckedIDs(messageTable);
                // bootstrap reply all
            });

            toolsModel.addObserver('forwardClicked', function() {
                var ids = getCheckedIDs(messageTable);
                // bootstrap forward
            });

            toolsModel.addObserver('deleteClicked', function() {
                messageListModel.del(getCheckedIDs(messageTable));
            });

            toolsModel.addObserver('junkClicked', function() {
                var ids = getCheckedIDs(messageTable);

                if(ids.length) {
                    messageListModel.flag(ids, 'junk');
                } else {
                    magma.dialog.message('Please check messages to mark as junk.');
                }
            });

            toolsModel.addObserver('copyClicked', function(folderID) {
                var ids = getCheckedIDs(messageTable);

                if(ids.length) {
                    messageListModel.copy(ids, folderID);
                } else {
                    magma.dialog.message('Please check messages to copy.');
                }
            });

            toolsModel.addObserver('moveClicked', function(folderID) {
                var ids = getCheckedIDs(messageTable);

                if(ids.length) {
                    messageListModel.move(ids, folderID);
                } else {
                    magma.dialog.message('Please check messages to move.');
                }
            });

            toolsModel.addObserver('markClicked', function(flag) {
                var ids = getCheckedIDs(messageTable);

                if(ids.length) {
                    messageListModel.flag(ids, flag);
                } else {
                    magma.dialog.message('Please check messages to mark.');
                }
            });

            toolsModel.addObserver('tagsClicked', function(tag) {
                var ids = getCheckedIDs(messageTable);

                if(ids.length) {
                    messageListModel.tag(ids, tag);
                } else {
                    magma.dialog.message('Please check messages to tag.');
                }
            });

            toolsModel.addObserver('displayClicked', function() {
                if(preview.is(':visible')) {
                    // remove preview pane
                    preview.hide();
                    messageTableSettings.sScrollY = container.height() - magma.tableHeaderHeight;
                    messageTable.dataTable(messageTableSettings);
                } else {
                    // show preview pane
                    half = container.height()/2;
                    messageTableSettings.sScrollY = half - magma.tableHeaderHeight;
                    messageTable.dataTable(messageTableSettings);
                    // append if first click
                    if(!preview.parent().length) {
                        preview.height(half - 1).appendTo(container.find('.view'));
                    } else {
                        preview.height(half - 1).show();
                    }
                }

            });

            // adjust table on window resize
            $(window).resize(function() {
                if(messageTable) {
                    messageTable.fnAdjustColumnSizing();
                }
            });

            // initialize dataTables once
            messageListModel.observeOnce('loaded', function(data) {
                var tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(messageList) {
                    messageList.appendTo(container.find('tbody'));

                    messageTableSettings = {
                        'sDom': 'tf',
                        'sScrollY': container.height() - magma.tableHeaderHeight,
                        'sScrollX': '100%',
                        'sScrollXInner': '200%',
                        'aaSorting': [[1, 'desc']],
                        'aoColumnDefs': [
                            {'iDataSort': 14, 'aTargets': [13]}, // use unix time for date sort
                            {'iDataSort': 16, 'aTargets': [15]}, // use unix time for arrival date sort
                            {'iDataSort': 18, 'aTargets': [17]} // use bytes for size sort
                        ],
                        'aoColumns': [
                            {'sClass': 'checkbox', 'sSortDataType': 'checkbox'}, // 0
                            {'sClass': 'messageID'}, // 1
                            {'sClass': 'seen'}, // 2
                            {'sClass': 'answered'}, // 3
                            {'sClass': 'flagged'}, // 4
                            {'sClass': 'attachment'}, // 5
                            {'sClass': 'from'}, // 6
                            {'sClass': 'to'}, // 7
                            {'sClass': 'addressed-to'}, // 8
                            {'sClass': 'reply-to'}, // 9
                            {'sClass': 'return-path'}, // 10
                            {'sClass': 'carbon'}, // 11
                            {'sClass': 'subject'}, // 12
                            {'sClass': 'utc', 'bVisible': false}, // 13
                            {'sClass': 'data'}, // 14
                            {'sClass': 'arrival-utc', 'bVisible': false}, // 15
                            {'sClass': 'arrival-date'}, // 16
                            {'sClass': 'bytes', 'bVisible': false}, // 17
                            {'sClass': 'size'}, // 18
                            {'sClass': 'snippet'}, //19
                            {'sClass': 'tags'} // 20
                        ],
                        'iDisplayLength': 25, // initial pagination length
                        'sPaginationType': 'full_numbers',
                        'fnDrawCallback': function() {
                            $('.checkbox-controls-button').click(checkboxCtrls);
                        },
                        'fnRowCallback': function(row, data, index, fullIndex) {
                            $(row).attr('id', 'message-' + data[1]);

                            if(data[2].match(/read/i)) {
                                $(row).addClass('seen');
                            } else {
                                $(row).removeClass('seen');
                            }

                            return row;
                        },
                        'oLanguage': {
                            'sSearch': '',
                            'sEmptyTable': 'No messages in this folder'
                        }
                    };

                    messageTable = list.dataTable(messageTableSettings);

                    // setup settings for reinit in the feature
                    // get rid of 'f' for filter
                    messageTableSettings.sDom = messageTableSettings.sDom.replace(/f/, '');
                    messageTableSettings.bDestroy = true;

                    // checkboxControls returns a function
                    // used in above draw callback
                    checkboxCtrls = checkboxControls(controlsOpen, messageTable, workspaceID);

                    // replace toolbar quick search with dataTables search
                    var filter = container.find('.dataTables_filter'),
                        filterInput = filter.find('input'),
                        label = quickSearch.find('form label'),
                        input = quickSearch.find('form input:text');

                    input.replaceWith(filter.detach());
                    label.detach().prependTo(filter);
                    filterInput.attr('id', label.attr('for'));
                    filterInput.fillWatermarks(label);

                    // clear input action
                    quickSearch.find('.clear').click(function(event) {
                        event.preventDefault();
                        filterInput.val('');
                        messageTable.fnFilter('');
                    });

                    // fix columns
                    // TODO: better column fix
                    setTimeout(function() {
                        messageTable.fnAdjustColumnSizing();
                    }, 50);

                    // observe subsequent folder loads
                    messageListModel.addObserver('loaded', function(data) {
                        var tmplModel = magma.model.tmpl();

                        tmplModel.addObserver('completed', function(messages) {
                            var messageData = [];

                            $.each(messages, function(i, message) {
                                var row = [];

                                $(message).find('td').each(function() {
                                    row.push($(this).html());
                                });

                                messageData.push(row);
                            });

                            // empty the table
                            // don't redraw table
                            if(messageData.length) {
                                // clear and don't redraw
                                messageTable.fnClearTable(false);
                                messageTable.fnAddData(messageData);
                            } else {
                                messageTable.fnClearTable();
                            }
                        });

                        tmplModel.fillTmpl('messageList', data);
                    });
                });

                tmplModel.fillTmpl('messageList', data);
            });

            // loads inbox by default
            messageListModel.load('inbox');

            return {
                root: container
            };
        },

        /*** meta ***/
        meta: {
            link: function(metaModel) {
                var link = getBlock('chromeMeta');

                link.click(function(event) {
                    event.preventDefault();
                    if(!metaModel.isOpen()) {
                        metaModel.open();
                    }
                });

                return {
                    root: link
                };
            },

            box: function(metaModel) {
                var location;

                var load = function() {
                    if(!location) {
                        throw new Error('metaBox.load: no location set');
                    }

                    metaModel.getMeta();
                };

                metaModel.addObserver('open', load);

                metaModel.addObserver('loaded', function(data) {
                    var tmplModel = magma.model.tmpl();

                    tmplModel.addObserver('completed', function(meta) {
                        var container = getBlock('meta'),
                            overlay = getBlock('overlay');

                        // ui progress bar
                        var quota = parseInt(meta.find('.quota').text().match(/\d+/), 10);
                        meta.find('.quota')
                            .html($('<div/>').progressbar({value: quota}))
                            .addClass('progress');

                        container
                            .append(meta)
                            .appendTo(location)
                            .reveal()
                            .delegate('a', 'click', function(event) {
                                event.preventDefault();

                                overlay.fadeOut(magma.animationSpeed, function() {
                                    $(this).remove();
                                });

                                $(this).parent().fadeOut(magma.animationSpeed, function() {
                                    $(this).remove();
                                });
                            })
                            .find('.view-logs')
                            .click(function(event) {
                                event.preventDefault();
                                magma.globalNavModel.openLocation('logs', 'mail');
                            });

                        overlay.appendTo('body').fadeIn(magma.animationSpeed);

                    });

                    tmplModel.fillTmpl('meta', data);
                });

                return {
                    setLocation: function(loc) {
                        location = loc;
                        return this;
                    }
                };
            }
        },

        /*** tab ***/
        tab: function(tabModel, callback) {
            if(!callback || typeof callback !== 'function') {
                throw new Error('view.tab: requires a callback');
            }

            var tmplModel = magma.model.tmpl();

            tmplModel.addObserver('completed', function(tab) {
                tab.find('.close')
                    .click(function(event) {
                        event.preventDefault();
                        tabModel.close();
                    })
                    .end()
                    .click(function() {
                        // if not dragging
                        if(!$(this).hasClass('ui-sortable-helper')) {
                            tabModel.focus();
                        }
                    });

                tabModel.addObserver('focus', function() {
                    if(!tab.hasClass('active')) {
                        tab.addClass('active');
                    }
                });

                tabModel.addObserver('blur', function() {
                    tab.removeClass('active');
                });

                tabModel.addObserver('closed', function() {
                    tab.remove();
                });

                tabModel.addObserver('remove', function() {
                    tab.remove();
                });

                tabModel.addObserver('titleModified', function(title) {
                    tab.find('h3').text(title);
                });

                callback({
                    root: tab
                });
            });

            tmplModel.fillTmpl('tab', {
                permatab: tabModel.isPermatab(),
                title: tabModel.getTitle(),
                skiplink: {
                    href: 'workspace-' + tabModel.getID(),
                    text: 'Skip to workspace'
                }
            });
        },

        /*** tabs ***/
        tabs: function(tabsModel) {
            var container = getBlock('tabs');

            tabsModel.addObserver('tabAdded', function(tabModel) {

                // blur other tabs when added
                tabsModel.forEach(function(tab) {
                    if(tab === tabModel) {
                        tab.focus();
                    } else {
                        tab.blur();
                    }
                });

                // blur other tabs when focused
                tabModel.addObserver('focus', function(tabModel) {
                    tabsModel.forEach(function(tab) {
                        if(tab !== tabModel) {
                            tab.blur();
                        }
                    });
                });

                var close = function(tabModel) {
                    if(tabModel.isActive()) {
                        tabsModel.removeTab(tabModel);
                        if(tabsModel.hasTabs()) {
                            tabsModel.lastTab().focus();
                        }
                    } else {
                        tabsModel.removeTab(tabModel);
                    }
                };

                tabModel.addObserver('closed', close);
                tabModel.addObserver('remove', close);
            });

            tabsModel.addObserver('show', function() {
                container.show();
            });

            tabsModel.addObserver('hide', function() {
                container.hide();
            });

            // init sortable
            container.sortable({
                items: 'div.tab:not(.permatab)',
                cancel: '.close',
                axis: 'x',
                revert: true,
                dropOnEmpty: false,
                opacity: 0.5
            });

            return {
                root: container
            };
        },

        /*** tools ***/
        tools: function(toolsModel, workspaceModel, folderModel, type) {
            var container = getBlock('tools'),
                toolOptionsContainer,
                tools;

            type = type || 'default';

            if(typeof type !== 'string') {
                throw new Error('view.tools: must provide type as a string');
            }

            try {
                tools = getBlock('tools' + type.replace(type.charAt(0), type.charAt(0).toUpperCase()));
            } catch(error) {
                throw new Error('view.tools: no tool type ' + type);
            }

            container.attr('id', 'toolbar-' + workspaceModel.getID());

            // ui buttons
            tools.find('a').each(function() {
                var icon = $(this).attr('class');
                $(this).button({icons: {primary: icon}});
                $(this).click(function(event) {
                    event.preventDefault();
                });
            });

            // ui toggles
            tools.find('input[type="checkbox"]').each(function() {
                var icon = $(this).attr('id');

                if($(this).hasClass('dropdown')) {
                    $(this).button({
                        icons: {
                            primary: icon,
                            secondary: 'ui-icon-triangle-1-s'
                        }
                    });
                } else {
                    $(this).button({
                        icons: {
                            primary: icon
                        }
                    });
                }

                // make ids unique
                var id = icon + '-' + workspaceModel.getID();
                tools.find('label[for="' + icon + '"]').attr('for', id);
                $(this).attr('id', id);
            });

            // add tools to toolbar
            tools.appendTo(container);

            // clicking tool just fires clicked event
            var registerTools = function(toolnames) {
                $.each(toolnames, function(i, toolname) {
                    toolsModel.addTool(toolname);

                    // convert camelcase to class slug
                    var slug = toolname.replace(/([A-Z])/g, '-$1').toLowerCase();
                    tools.find('.' + slug + ', input[id^="' + slug + '"]').click(function(event) {
                        event.preventDefault();
                        toolsModel.toolClicked(toolname);
                    });
                });
            };

            // handles tool dropdowns
            var toolOptionsDropdown = function(toolnames, toolOptions, action) {
                var toggles = $();

                toolnames = toolnames.split(' ');

                $.each(toolnames, function() {
                    $.merge(toggles, tools.find('input[id^="' + this + '"]'));
                });

                toggles.click(function() {
                    var toggle = $(this);

                    // make sure this isn't already opened
                    if(!toggle.data('open')) {
                        // untoggle other dropdowns if opened
                        tools.find('.dropdown').each(function() {
                            if($(this).data('open')) {
                                // toggles button and removes previous tool options
                                $(this).click();
                            }
                        });

                        toggle.data('open', true);

                        toolOptionsContainer = getBlock('toolOptionsContainer');
                        var addOptions = function(options) {
                            toolOptionsContainer
                                .append(options)
                                .insertAfter(container.parent())
                                .find('a')
                                .each(function() {
                                    $(this).click(function(event) {
                                        event.preventDefault();
                                        action($(this));
                                        toggle.click();
                                    });
                                });
                        };

                        // handle async methods
                        if(typeof toolOptions === 'function') {
                            toolOptions(function(options) {
                                addOptions(options);
                            });
                        } else {
                            addOptions(toolOptions);
                        }

                        // alert observers
                        toolsModel.showDropdown();

                    // close
                    } else {
                        toggle.data('open', false);
                        toolOptionsContainer.remove();
                        toolsModel.hideDropdown();
                    }
                });
            };

            // get folders for list dropdown
            var listFolders = function(callback) {
                var list = $('<ul/>');

                folderModel.observeOnce('loaded', function(data) {
                    var permaTmplModel = magma.model.tmpl();

                    permaTmplModel.observeOnce('completed', function(folders) {
                        list.append(folders);

                        var tmplModel = magma.model.tmpl();

                        tmplModel.observeOnce('completed', function(folders) {
                            list.append(folders);
                            return callback(list);
                        });

                        tmplModel.fillTmpl('foldersList', data.custom);
                    });

                    permaTmplModel.fillTmpl('foldersMail', data.permanent);
                });

                folderModel.loadFolders({alphabetical: true});
            };

            // add toolbars for specific workspaces
            switch(type) {
                case 'default':
                    tools.find('.new').click(function(event) {
                        event.preventDefault();

                        var composeModel = magma.model.compose();

                        composeModel.addObserver('ready', function() {
                            magma.bootstrap.compose(composeModel);
                        });

                        composeModel.addObserver('readyError', function() {
                            magma.dialog.die('Sorry, there was an error.');
                        });

                        composeModel.addObserver('readyFailed', function() {
                            magma.dialog.die('Could not connect to server.');
                        });

                        composeModel.composeMessage();
                    });

                    toolsModel.addTool('copy');
                    toolsModel.addTool('move');
                    toolOptionsDropdown(
                        'copy move',
                        listFolders,
                        function(clicked) {
                            // get folder id
                            var id = clicked.parent().attr('id');
                            id = parseInt(id.substr(id.search(/\d/)), 10);

                            // copy or move
                            var tool;
                            tools.find('input').each(function() {
                                if($(this).data('open')) {
                                    tool = $(this).attr('id').match(/\w+/)[0];
                                }
                            });
                            toolsModel.toolClicked(tool, id);
                        }
                    );

                    toolsModel.addTool('mark');
                    toolOptionsDropdown(
                        'mark',
                        getBlock('toolOptionsMark'),
                        function(clicked) {
                            toolsModel.toolClicked('mark', clicked.attr('class'));
                        }
                    );

                    toolsModel.addTool('tags');
                    toolOptionsDropdown(
                        'tags',
                        function(callback) {
                            var tagList = getBlock('toolOptionsTags'),
                                tagsModel = magma.model.tags();

                            tagsModel.addObserver('loaded', function(data) {
                                var tmplModel = magma.model.tmpl();

                                tmplModel.addObserver('completed', function(tags) {
                                    tagList.find('.tags').append(tags);
                                    callback(tagList);
                                });

                                tmplModel.fillTmpl('tagsList', data);
                            });

                            tagsModel.loadTags();
                        },
                        function(clicked) {
                            toolsModel.toolClicked('tags', clicked.text());
                        }
                    );

                    registerTools([
                        'reply',
                        'replyAll',
                        'forward',
                        'delete',
                        'junk',
                        'display'
                    ]);
                break;

                case 'composing':
                    tools.find('.cancel').click(function(event) {
                        event.preventDefault();
                        workspaceModel.close();
                    });
                    registerTools([
                        'cc',
                        'bcc',
                        'attach',
                        'save',
                        'send'
                    ]);
                break;

                case 'contacts':

                    toolsModel.addTool('sort');
                    toolOptionsDropdown(
                        'sort',
                        getBlock('toolOptionsSort'),
                        function(clicked) {
                            toolsModel.toolClicked('sort', $(this).attr('class'));
                        }
                    );

                    toolsModel.addTool('narrow');
                    toolOptionsDropdown(
                        'narrow',
                        getBlock('toolOptionsNarrow'),
                        function(clicked) {
                            toolsModel.toolClicked('narrow', $(this).attr('class'));
                        }
                    );

                    toolsModel.addTool('copy');
                    toolOptionsDropdown(
                        'copy',
                        listFolders,
                        function(clicked) {
                            // get folder id
                            var id = $(this).parent().attr('id');
                            id = parseInt(id.substr(id.search(/\d/)), 10);

                            toolsModel.toolClicked('copy', id);
                        }
                    );

                    toolsModel.addTool('move');
                    toolOptionsDropdown(
                        'move',
                        listFolders,
                        function(clicked) {
                            // get folder id
                            var id = $(this).parent().attr('id');
                            id = parseInt(id.substr(id.search(/\d/)), 10);

                            toolsModel.toolClicked('move', id);
                        }
                    );

                    // just send click events
                    registerTools([
                        'new',
                        'edit',
                        'delete'
                    ]);

                    // enable/disable listeners
                    toolsModel.addObserver('enabled', function(tool) {
                        tools.find('.' + tool).parent().removeClass('disabled');
                    });

                    toolsModel.addObserver('disabled', function(tool) {
                        tools.find('.' + tool).parent().addClass('disabled');
                    });
                break;

                case 'settings':
                    // just send click events
                    registerTools([
                        'advanced',
                        'defaults',
                        'edit'
                    ]);
                break;
            }

            return {
                root: container
            };
        },

        /*** quick search ***/
        quickSearch: function(workspaceID, advanced) {
            var search = getBlock('quickSearch'),
                input = search.find('input').filter(':text');

            if(typeof advanced !== 'boolean') {
                advanced = true;
            }

            search.find('input').filter(':submit').click(function(event) {
                event.preventDefault();
                //quickSearchModel.search(input.val());
            });

            if(advanced) {
                search.find('.advanced').click(function(event) {
                    event.preventDefault();
                    magma.bootstrap.search();
                });
            } else {
                search.find('.advanced').remove();
            }

            // provide unique ids
            search.attr('id', 'search-' + workspaceID);
            search.find('form').find('label, input:text').each(function(i) {
                var a = ['for', 'id'];
                $(this).attr(a[i], $(this).attr(a[i]) + '-' + workspaceID);
            });

            input.fillWatermarks(search.find('label'));

            return {
                root: search
            };
        },

        /*** scrape contacts ***/
        scrapeContacts: function(scrapeContactsModel, workspaceID) {
            var container = getBlock('scrapeContactsList'),
                contactsTable;

            // remove contact when successfully added
            scrapeContactsModel.addObserver('added', function(id) {
                var nodes = contactsTable.fnGetNodes(),
                    node = $(nodes).filter('#contact-' + id)[0];

                contactsTable.fnClose(node);
                contactsTable.fnDeleteRow(node);
            });

            scrapeContactsModel.observeOnce('loaded', function(data) {
                var tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(contactList) {
                    contactList.appendTo(container.find('tbody'));

                    // get height from workspace
                    var containerHeight = $('#workspace-' + workspaceID).find('.workspace').height();

                    contactsTable = container.dataTable({
                        'sDom': 't',
                        'sScrollY': containerHeight - magma.tableHeaderHeight,
                        'sScrollX': '100%',
                        'aoColumns': [
                            {'sClass': 'name'},
                            {'sClass': 'email'}
                        ],
                        'iDisplayLength': 25, // initial pagination length
                        'sPaginationType': 'full_numbers',
                        'oLanguage': {
                            'sSearch': '',
                            'sEmptyTable': 'No more contacts to add from this message'
                        }
                    });

                    // fix columns
                    // TODO: better column fix
                    setTimeout(function() {
                        contactsTable.fnAdjustColumnSizing();
                    }, 50);

                    contactsTable.delegate('tr', 'click', function() {
                        if($(this).attr('id')) {
                            if($(this).hasClass('open')) {
                                contactsTable.fnClose(this);
                                $(this).removeClass('open');
                            } else {
                                var id = parseInt($(this).attr('id').match(/\d+/), 10),
                                    row = this,
                                    scrapeDetails = getBlock('scrapeContacts');

                                // close other rows
                                contactsTable.find('.open').each(function() {
                                    contactsTable.fnClose(this);
                                    $(this).removeClass('open');
                                });

                                contactsTable.fnOpen(row, scrapeDetails.html(), 'scrape-contacts contact-' + id);

                                // get jquery object again
                                scrapeDetails = contactsTable.find('.scrape-contacts.contact-' + id);

                                var name = scrapeDetails.find('#contact-name'),
                                    email = scrapeDetails.find('#contact-email'),
                                    phone = scrapeDetails.find('#contact-phone'),
                                    work = scrapeDetails.find('#contact-work'),
                                    cell = scrapeDetails.find('#contact-cell');

                                name.val($(row).find('.name').text());
                                email.val($(row).find('.email').text());

                                scrapeDetails.find('form').submit(function(event) {
                                    event.preventDefault();
                                    scrapeContactsModel.add({
                                        id: id,
                                        name: name.val(),
                                        email: email.val(),
                                        phone: phone.val(),
                                        work: work.val(),
                                        cell: cell.val()
                                    });
                                });

                                $(row).addClass('open');
                            }
                        }
                    });
                });

                tmplModel.fillTmpl('scrapeList', data);
            });

            scrapeContactsModel.load();

            return {
                root: container
            };
        },

        /*** search results ***/
        searchResults: function(searchResultsModel, workspaceID) {
            var container = getBlock('searchResultsList'),
                list = container.find('.results'),
                searchResultsTable,
                checkboxCtrls,
                controlsOpen = false,
                progressBar = getBlock('searchResultsProgress');

            container.find('tbody').delegate('tr', 'dbclick', function(event) {
                event.preventDefault();

                var id = $(this).attr('href');
                id = parseInt(id.substr(id.search(/\d/)), 10);

                magma.bootstrap.read(id);
            });

            // prevent row click when checkbox clicked
            container.find('tbody').delegate('input[type="checkbox"]', 'click', function(event) {
                event.stopPropagation();
            });

            // TODO: container height does not exist yet, neigther does workspace height such as in scrape contacts
            searchResultsTable = list.dataTable({
                'sDom': 't',
                'sScrollY': container.height() - magma.tableHeaderHeight,
                'sScrollX': '100%',
                'sScrollXInner': '200%',
                'aaSorting': [[1, 'desc']],
                'aoColumnDefs': [
                    {'iDataSort': 14, 'aTargets': [13]}, // use unix time for date sort
                    {'iDataSort': 16, 'aTargets': [15]}, // use unix time for arrival date sort
                    {'iDataSort': 18, 'aTargets': [17]} // use bytes for size sort
                ],
                'aoColumns': [
                    {'sClass': 'checkbox', 'sSortDataType': 'checkbox'}, // 0
                    {'sClass': 'messageID'}, // 1
                    {'sClass': 'seen'}, // 2
                    {'sClass': 'answered'}, // 3
                    {'sClass': 'flagged'}, // 4
                    {'sClass': 'attachment'}, // 5
                    {'sClass': 'from'}, // 6
                    {'sClass': 'to'}, // 7
                    {'sClass': 'addressed-to'}, // 8
                    {'sClass': 'reply-to'}, // 9
                    {'sClass': 'return-path'}, // 10
                    {'sClass': 'carbon'}, // 11
                    {'sClass': 'subject'}, // 12
                    {'sClass': 'utc', 'bVisible': false}, // 13
                    {'sClass': 'data'}, // 14
                    {'sClass': 'arrival-utc', 'bVisible': false}, // 15
                    {'sClass': 'arrival-date'}, // 16
                    {'sClass': 'bytes', 'bVisible': false}, // 17
                    {'sClass': 'size'}, // 18
                    {'sClass': 'snippet'}, //19
                    {'sClass': 'tags'} // 20
                ],
                'fnDrawCallback': function() {
                    $('.checkbox-controls-button').click(checkboxCtrls);
                },
                'fnRowCallback': function(row, data, index, fullIndex) {
                    $(row).attr('id', 'message-' + data[1]);

                    if(data[2].match(/read/i)) {
                        $(row).addClass('seen');
                    } else {
                        $(row).removeClass('seen');
                    }

                    return row;
                },
                'oLanguage': {
                    'sSearch': '',
                    'sEmptyTable': 'Use the search fields above to begin your search.'
                }
            });

            // checkboxControls returns a function
            // used in above draw callback
            checkboxCtrls = checkboxControls(controlsOpen, searchResultsTable, workspaceID);

            // fix columns
            // TODO: better column fix
            setTimeout(function() {
                searchResultsTable.fnAdjustColumnSizing();
            }, 50);

            // progress bar
            progressBar.bind('progressbarcreate', function(event, ui) {
                $(this).wrap('<tr class="progress-row"><td colspan="' + container.find('thead tr').children().length + '"></td></tr>');
                $(this).parents('tr').appendTo(container.find('tbody'));
            });

            progressBar.bind('progressbarcomplete', function(event, ui) {
                $(this).progressbar('destroy');
            });

            searchResultsModel.addObserver('loading', function(progress) {
                progressBar.progressbar({value: progress});
            });

            searchResultsModel.addObserver('loaded', function(data) {
                var tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(messages) {
                    var messageData = [];

                    $.each(messages, function(i, message) {
                        var row = [];

                        $(message).find('td').each(function() {
                            row.push($(this).html());
                        });

                        messageData.push(row);
                    });

                    // empty the table
                    // don't redraw table
                    if(messageData.length) {
                        // clear and don't redraw
                        searchResultsTable.fnClearTable(false);
                        searchResultsTable.fnAddData(messageData);
                    } else {
                        searchResultsTable.fnClearTable();
                    }
                });

                tmplModel.fillTmpl('messageList', data);
            });

            return {
                root: container
            };
        },

        /*** search option ***/
        searchOption: function(searchOptionModel) {
            var container = getBlock('searchOption'),
                field = container.find('.search-field'),
                filter = container.find('.search-filter'),
                controls = container.find('.search-controls'),
                remove = getBlock('searchControlsRemove'),
                add = getBlock('searchControlsAdd'),
                inputs = {},
                currentFilter = 'string',
                currentDateFilter = 'single',
                currentSizeFilter = 'single',
                datepicker;

            // used for updating row nums and keeping ids unique
            var registerInput = function(type, input, label) {
                inputs[type] = inputs[type] || [];

                for(var i in inputs[type]) {
                    if(input === inputs[type][i].input) {
                        throw new Error('searchOptions.registerInput: cannot add the same input twice');
                    }
                }
                inputs[type].push({
                    input: input,
                    inputID: input.attr('id'),
                    inputName: input.attr('name'),
                    label: label,
                    labelFor: label.attr('for')
                });
            };

            var clearInputs = function(type) {
                delete inputs[type];
            };

            // deal with datepicker needing initialized after id chagne
            var initDatepicker = function(datepicker, range) {
                if(range) {
                    datepicker.datepicker({
                        changeMonth: true,
                        changeYear: true,
                        minDate: range.minDate,
                        maxDate: range.maxDate,
                        onSelect: function(selectedDate) {
                            var option = this.id.match(/from/) ? "minDate" : "maxDate",
                                instance = $(this).data("datepicker"),
                                date = $.datepicker.parseDate(
                                    instance.settings.dateFormat ||
                                    $.datepicker._defaults.dateFormat,
                                    selectedDate, instance.settings
                                );
                            datepicker.not(this).datepicker("option", option, date);
                        }
                    });
                } else {
                    datepicker.datepicker({
                        changeMonth: true,
                        changeYear: true
                    });
                }
            };

            // update filter on certain fields
            field.find('select').change(function() {
                var filt = $(this).children().filter(':selected').attr('class');

                if(currentFilter !== filt) {
                    clearInputs(currentFilter);
                    filter.empty();

                    datepicker = undefined;

                    switch(filt) {
                        case 'date':
                            var dates = getBlock('searchFilterDate'),
                                dateForm = getBlock('searchFilterDateSingle');

                            datepicker = dateForm.filter('input');
                            datepicker.fillWatermarks(dateForm.filter('label'));

                            registerInput('date', dates.filter('select'), dates.filter('label'));
                            registerInput('single', datepicker, dateForm.filter('label'));

                            dates.filter('select').change(function() {
                                var dateFilter = $(this).children().filter(':selected').attr('class');

                                if(currentDateFilter !== dateFilter) {
                                    clearInputs(dateFilter);
                                    dateForm.remove();

                                    if(dateFilter === 'range') {
                                        dateForm = getBlock('searchFilterDateRange');
                                        datepicker = dateForm.filter('input');
                                        datepicker.fillWatermarks(dateForm.filter('label'));
                                        registerInput(dateFilter, datepicker.filter(':first'), dateForm.filter('label:first'));
                                        registerInput(dateFilter, datepicker.filter(':last'), dateForm.filter('label:last'));
                                        filter.append(dateForm);
                                    } else {
                                        dateForm = getBlock('searchFilterDateSingle');
                                        datepicker = dateForm.filter('input');
                                        datepicker.fillWatermarks(dateForm.filter('label'));
                                        registerInput(dateFilter, datepicker, dateForm.filter('label'));
                                        filter.append(dateForm);
                                    }
                                }

                                // fill in the row ids
                                searchOptionModel.setRow(searchOptionModel.getRow());

                                currentDateFilter = dateFilter;
                            });

                            filter.append(dates);
                            filter.append(dateForm);
                        break;

                        case 'size':
                            var size = getBlock('searchFilterSize'),
                                sizeForm = getBlock('searchFilterSizeSingle');

                            sizeForm.filter('input').fillWatermarks(sizeForm.filter('label'));

                            registerInput('size', size.filter('select'), size.filter('label'));
                            registerInput('single', sizeForm.filter('input'), sizeForm.filter('label:first'));
                            registerInput('single', sizeForm.filter('select'), sizeForm.filter('label:last'));

                            size.filter('select').change(function() {
                                var sizeFilter = $(this).children().filter(':selected').attr('class');

                                if(currentSizeFilter !== sizeFilter) {
                                    clearInputs(sizeFilter);
                                    sizeForm.remove();

                                    if(sizeFilter === 'range') {
                                        sizeForm = getBlock('searchFilterSizeRange');
                                        sizeForm.filter('input').fillWatermarks(sizeForm.filter('label'));
                                        registerInput(sizeFilter, sizeForm.filter('input:first'), sizeForm.filter('label').eq(0));
                                        registerInput(sizeFilter, sizeForm.filter('select:first'), sizeForm.filter('label').eq(1));
                                        registerInput(sizeFilter, sizeForm.filter('input:last'), sizeForm.filter('label').eq(2));
                                        registerInput(sizeFilter, sizeForm.filter('select:last'), sizeForm.filter('label').eq(3));
                                        filter.append(sizeForm);
                                    } else {
                                        sizeForm = getBlock('searchFilterSizeSingle');
                                        sizeForm.filter('input').fillWatermarks(sizeForm.filter('label'));
                                        registerInput('single', sizeForm.filter('input'), sizeForm.filter('label:first'));
                                        registerInput('single', sizeForm.filter('select'), sizeForm.filter('label:last'));
                                        filter.append(sizeForm);
                                    }
                                }

                                // fill in the row ids
                                searchOptionModel.setRow(searchOptionModel.getRow());

                                currentSizeFilter = sizeFilter;
                            });

                            filter.append(size);
                            filter.append(sizeForm);
                        break;

                        // string
                        default:
                            var str = getBlock('searchFilterString');
                            str.filter('select, input').fillWatermarks(str.filter('label'));
                            registerInput('string', str.filter('select'), str.filter('label:first'));
                            registerInput('string', str.filter('input'), str.filter('label:last'));
                            filter.append(str);
                        break;
                    }

                    // fill in the row ids
                    searchOptionModel.setRow(searchOptionModel.getRow());

                    currentFilter = filt;
                }
            });

            searchOptionModel.addObserver('remove', function() {
                container.remove();
            });

            searchOptionModel.addObserver('controlsUpdated', function(ctrls) {
                if(ctrls.remove) {
                    if(!controls.find('.remove').length) {
                        controls.prepend(remove);
                    }
                } else {
                    if(controls.find('.remove').length) {
                        controls.find('.remove').remove();
                    }
                }

                if(ctrls.add) {
                    if(!controls.find('.add').length) {
                        controls.append(add);
                    }
                } else {
                    if(controls.find('.add').length) {
                        controls.find('.add').remove();
                    }
                }
            });

            searchOptionModel.addObserver('rowUpdated', function(row) {
                var maxDate,
                    minDate;

                if(datepicker && datepicker.length > 1) {
                    maxDate = datepicker.filter(':first').datepicker('option', 'maxDate');
                    minDate = datepicker.filter(':last').datepicker('option', 'minDate');
                }

                for(var type in inputs) {
                    for(var i in inputs[type]) {
                        inputs[type][i].input.attr({
                            id: inputs[type][i].inputID + '-' + searchOptionModel.getID() + '.' + row,
                            name: inputs[type][i].inputName + '-' + row
                        });

                        inputs[type][i].label.attr({
                            'for': inputs[type][i].labelFor + '-' + searchOptionModel.getID() + '.' + row
                        });
                    }

                    // init date pickers since they die after id change
                    if(datepicker) {
                        datepicker.removeClass('hasDatepicker');

                        if(datepicker.length > 1) {
                            initDatepicker(datepicker, {maxDate: maxDate, minDate: minDate});
                        } else {
                            initDatepicker(datepicker);
                        }
                    }
                }
            });

            searchOptionModel.addObserver('update', function(params) {
                searchOptionModel.setRow(params.row);
                if(params.atLimit) {
                    searchOptionModel.setControls({remove: true});
                } else if(params.singleRow) {
                    searchOptionModel.setControls({add: true});
                } else {
                    searchOptionModel.setControls({add: true, remove: true});
                }
            });

            // insert default filter
            filter.append(getBlock('searchFilterString'));
            filter.find('select, input').fillWatermarks(filter.find('label'));

            // register inputs
            registerInput('field', field.find('select'), field.find('label'));
            registerInput('string', filter.find('select'), filter.find('label:first'));
            registerInput('string', filter.find('input'), filter.find('label:last'));

            return {
                root: container
            };
        },

        /*** search options ***/
        searchOptions: function(searchOptionsModel, searchResultsModel, workspaceID) {
            var container = getBlock('search'),
                searchIn = container.find('#search-in'),
                options = container.find('.search-options'),
                clicked;

            container.find('form').submit(function(event) {
                event.preventDefault();

                // disable fields
                container.find('input, select').attr('disabled', 'disabled');

                // disable additional fields
                container
                    .find('.search-controls')
                    .find('.button')
                    .addClass('disabled')
                    .click(function(event) {
                        event.preventDefault();
                        event.stopPropagation();
                    });

                // search to stop
                container
                    .find('input[type="submit"]')
                    .val('Stop');

                // TODO: transform form data to search method params object
                searchResultsModel.search('query');
            });

            // enable fields on loaded
            searchResultsModel.addObserver('loaded', function() {
                // enable fields
                container
                    .find('input, select')
                    .removeAttr('disabled')
                    .end()
                    .find('.search-controls .button')
                    .removeClass('disabled')
                    .unbind('click')
                    .end()
                    .find('input[type="submit"]')
                    .val('Search')
            });

            searchOptionsModel.addObserver('optionAdded', function(searchOptionModel) {
                var searchOption = magma.view.searchOption(searchOptionModel).root;

                searchOption.delegate('.add', 'click', function(event) {
                    event.preventDefault();
                    clicked = $(this).parents('.search-option');

                    var searchOptionModel = magma.model.searchOption(workspaceID, searchOptionsModel.getCurrRow());
                    searchOptionsModel.addOption(searchOptionModel);
                });

                searchOption.delegate('.remove', 'click', function(event) {
                    event.preventDefault();
                    searchOptionsModel.removeOption(searchOptionModel);
                });

                if(clicked) {
                    searchOption.insertAfter(clicked);
                } else {
                    searchOption.appendTo(options);
                }
            });

            searchOptionsModel.addObserver('optionRemoved', function(searchOptionModel) {
                searchOptionModel.removeOption();
            });

            // add mail folders to search in list
            var folderModel = magma.model.folders();

            folderModel.addObserver('loaded', function(data) {
                var tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(folderOptions) {
                    folderOptions.appendTo(searchIn);
                });

                tmplModel.fillTmpl('options', data);
            });

            folderModel.loadFolders({options: true});

            // ensure unique ids
            searchIn
                .attr({id: searchIn.attr('id') + '-' + workspaceID})
                .siblings('label').attr({
                    'for': searchIn.siblings('label').attr('for') + '-' + workspaceID
                });

            // fill in first option
            var searchOptionModel = magma.model.searchOption(workspaceID, searchOptionsModel.getCurrRow());
            searchOptionsModel.addOption(searchOptionModel);

            return {
                root: container
            };
        },

        /*** settings ***/
        settings: function(settingsModel, toolsModel, folderModel, initialize) {
            var container = getBlock('settings'),
                settingsContainer = container.find('.settings');

            initialize = initialize || 'identity';

            if(typeof initialize !== 'string') {
                throw new Error('view.settings: initialize must be given as a string');
            }

            // tool actions
            toolsModel.addObserver('advancedClicked', settingsModel.advancedSettings);
            toolsModel.addObserver('defaultsClicked', settingsModel.defaultSettings);
            toolsModel.addObserver('editClicked', settingsModel.editSettings);

            settingsModel.addObserver('edit', function() {
                var items = settingsContainer.find('dd'),
                    input = $('<input/>')
                        .attr('type', 'text')
                        .addClass('value-input'),
                    addField = getBlock('settingsIdentityAddField');

                // replace values with inputs
                items.each(function() {
                    var value = $(this).text(),
                        container = $(this).parent().parent();

                    $(this)
                        .html(input.clone().val(value))
                        .find('input')
                        .bind('change blur', function() {
                            if($(this).val() !== value) {
                                container.addClass('edited');
                            } else {
                                container.removeClass('edited');
                            }
                        })
                        .focus(function() {
                            container.addClass('edited');
                        });
                });

                // add remove action to removable items
                settingsContainer
                    .find('.removable')
                    .find('.actions')
                    .append($('<a/>')
                        .addClass('remove')
                        .text('Remove'))
                    .delegate('.remove', 'click', function(event) {
                        event.preventDefault();
                        $(this).parents('.removable').remove();
                    });

                // add field container
                settingsContainer.append(addField);
            });

            // returns context (identity, mail settings, etc) and data
            settingsModel.addObserver('loaded', function(o) {
                var context = o.context[0].toUpperCase() + o.context.substr(1),
                    data = o.data,
                    tmplModel = magma.model.tmpl();

                tmplModel.addObserver('completed', function(html) {
                    // add help text functionality
                    if(o.context === 'identity') {
                        // show help icon
                        html.filter('div').mouseenter(function() {
                            $(this).find('.actions').show();
                        });

                        // show help icon
                        html.filter('div').mouseleave(function() {
                            $(this).find('.actions').hide();
                        });

                        html.delegate('.help', 'click', function(event) {
                            event.preventDefault();
                            var closeHelp = $('<a/>').addClass('close-help').text('Close Help')

                            $(this).parent().siblings('.help-text').slideDown();
                            $(this).replaceWith(closeHelp);
                        });

                        html.delegate('.close-help', 'click', function(event) {
                            event.preventDefault();
                            var container = $(this).parent(),
                                closeHelp = $(this),
                                help = $('<a/>').addClass('help').text('Help');

                            closeHelp.parent().siblings('.help-text').slideUp(function() {
                                closeHelp.replaceWith(help);
                                // LOW: check hover state to show if div hovered
                            });
                        });

                        settingsContainer.html(html).addClass('help-text');

                    // just pop it in as is
                    } else {
                        settingsContainer.html(html).removeClass('help-text');
                    }
                });

                tmplModel.fillTmpl('settings' + context, data);
            });

            settingsModel.load(initialize);

            return {
                root: container
            };
        },

        /*** workspace ***/
        workspace: function(workspaceModel, tabModel, toolsModel) {
            var container = getBlock('workspaceContainer').attr('id', 'workspace-' + workspaceModel.getID()),
                workspace = container.find('.workspace');

            var show = function() {
                if(!container.hasClass('active')) {
                    container.addClass('active');
                }
            };

            var hide = function() {
                container.removeClass('active');
            };

            var close = function() {
                container.remove();
            };

            workspaceModel.addObserver('focus', show);
            workspaceModel.addObserver('blur', hide);
            workspaceModel.addObserver('closed', close);
            workspaceModel.addObserver('remove', close);

            // contacts/options/help do not have tabs
            if(tabModel) {
                // focus this workspace when its tab is clicked
                tabModel.addObserver('focus', function() {
                    workspaceModel.focus();
                });

                // close this workspace when its tab is closed
                tabModel.addObserver('closed', function() {
                    workspaceModel.remove();
                });

                // remove tab when workspace closed
                workspaceModel.addObserver('closed', function() {
                    tabModel.remove();
                });
            }

            // handle tool menu dropdowns
            if(toolsModel) {
                toolsModel.addObserver('showDropdown', function() {
                    workspace.addClass('tool-dropdown');
                });

                toolsModel.addObserver('hideDropdown', function() {
                    workspace.removeClass('tool-dropdown');
                });
            }

            return {
                root: container,
                workspace: workspace
            };
        },

        /*** workspaces ***/
        workspaces: function(workspacesModel, tabsModel) {
            var container = getBlock('workspacesContainer');

            workspacesModel.addObserver('workspaceAdded', function(workspaceModel) {
                // hide other workspaces and show new one
                workspacesModel.forEach(function(workspace) {
                    if(workspace === workspaceModel) {
                        workspace.focus();
                    } else {
                        workspace.blur();
                    }
                });

                // blur other workspaces when one focuses
                workspaceModel.addObserver('focus', function(workspaceModel) {
                    workspacesModel.forEach(function(workspace) {
                        if(workspace !== workspaceModel) {
                            workspace.blur();
                        }
                    });
                });

                var close = function(workspaceModel) {
                    if(workspaceModel.isActive()) {
                        workspacesModel.removeWorkspace(workspaceModel);
                        if(workspacesModel.hasWorkspace()) {
                            workspacesModel.lastWorkspace().focus();
                        }
                    } else {
                        workspacesModel.removeWorkspace(workspaceModel);
                    }
                };

                workspaceModel.addObserver('closed', close);
                workspaceModel.addObserver('remove', close);
            });

            // remove dedicated workspaces when they're navigated away from
            workspacesModel.addObserver('dedicatedWorkspaceAdded', function(workspaceModel) {
                workspaceModel.addObserver('blur', function(workspaceModel) {
                    workspaceModel.close();
                });
            });

            // adjust top when tabs shown/hidden
            tabsModel.addObserver('show', function() {
                container.removeClass('no-tabs');
            });

            tabsModel.addObserver('hide', function() {
                container.addClass('no-tabs');
            });

            return {
                root: container
            };
        }
    };
}());
/**
 * application.js
 *
 * Kicks off Magma
 *
 * This script must be inserted last since it's calling functions
 * Handled in build-js.php
 */
var magma = magma || {};

$(document).ready(function() {
    // startup login
    magma.bootstrap.login();
});
/**
 * Gets json method data on calls to mockiface using mockjax plugin
 */
$(document).ready(function() {
    /*
     * helper ajax call
     *
     * @param method    interface mathod name
     */
    var get_response = function(method) {
        var response;

        $.ajax({
            type: 'GET',
            url: '/json/responses/' + method + '.json',
            async: false,
            processData: true,
            dataType: 'json',
            success: function(data) {
                response = data;
            },
            error: function(data) {
                if(data.statusText === "parsererror") {
                    console.log('Parse error in response ' + method + '.json');
                } else if(data.status === 404) {
                    console.log('json/responses/' + method + '.json does not exist.');
                }
            }
        });

        return response;
    };

    /*
     * Ajax Mockups
     *
     * All Ajax requests will go to a single url during development.
     * Request and response data based on JSON-RPC spec: http://json-rpc.org/wiki/specification
     *
     * Request data
     * method: string name of method to be invoked
     * params: array containing arguments to pass to the function
     *
     * Response data
     * result: true or message if success, undefined if error
     * error: undefined if success, message string if error
     */
    $.mockjax({
        url: '/portal/mockiface',
        status: 200,    // change to 500 to test errors
        responseTime: 50,
        response: function(settings) {
            var data;

            // data could be string or json object
            if(typeof settings.data === 'string') {
                data = JSON.parse(settings.data);
            } else {
                data = settings.data;
            }

            var method = data.method,
                params = data.params,
                response = {},
                respond = function(result, num) {
                    // use first result if no index provided
                    num = num || 0;

                    var full_response = get_response(method);

                    // returns undefined if error
                    if(!full_response) {
                        response = {error: true};
                        return;
                    }

                    // get rid of results we don't need
                    var i = 0;
                    while(i < full_response.length) {
                        if(!full_response[i][result]) {
                            full_response.splice(i, 1);
                        } else {
                            i++;
                        }
                    }

                    response[result] = full_response[num][result];
                };

            // select specific results depending on method params
            // result number is array index of hard coded json response
            switch(method) {

                case "auth":
                    if(params && params.username === "magma" && params.password === "test") {
                        // authenticated!
                        respond("result");
                    } else if(params && params.username === "locked") {
                        // account locked
                        respond("result", 2);
                    } else {
                        // wrong username/password
                        respond("result", 1);
                    }
                break;

                case "aliases":
                    if(params && params.def) {
                        // only return default
                        respond("result", 1);
                    } else {
                        // return full list of aliases
                        respond("result");
                    }
                break;

                case "contacts.list":
                    // no gravatars
                    if(params && params.folderID === 999) {
                        respond("result", 1);
                    } else {
                        respond("result");
                    }
                break;

                case "folders.list":
                    if(params && params.context === "permanent") {
                        respond("result", 2);
                    } else if(params && params.context === "contacts") {
                        respond("result", 1);
                    } else if(params && params.context === "settings") {
                        respond("result", 2);
                    // TODO: change to logs
                    } else if(params && params.context === "logs") {
                        respond("result", 3);
                    } else if(params && params.context === "help") {
                        respond("result", 4);
                    } else {
                        // inbox
                        respond("result");
                    }
                break;

                case "folders.add":
                    respond("result");
                    response.result.folderID = Math.floor(Math.random()*1000);
                break;

                case "messages.list":
                    if(params && params.folderID === 0) {
                        // empty folder
                        respond("result");
                    } else {
                        respond("result", 1);
                        $.each(response.result, function(i, message) {
                            message.messageID = Math.floor(Math.random()*10000000);
                        });
                    }
                break;

                case "messages.load":
                    if(params && params.section[0] === "info") {
                        respond("result", 1);
                    } else {
                        respond("result");
                    }
                break;

                case "ad":
                    if(params && params.context === "loading") {
                        // loading context
                        respond("result");
                    } else {
                        // messages context
                        respond("result", 1);
                    }
                break;

                default:
                    respond("result");
                break;
            }

            this.responseText = response;
        }
    });
});
/**
 * Displays static html file selector with backtick and fills in templates using template comments in html blocks
 */
$(document).ready(function() {
    /*** load tmpl ***/
    /**
     * Loads the template if it's not cached and handles composite template calls
     *
     * @param template string   template name
     * @param callback function callback to be called on load
     */
    var loadTmpl = function(template, callback) {
        if(!template) {
            throw new Error('loadTmpl must be provided a template to load!');
        }

        // check if template is cached
        if($.template[template]) {
            if(callback) {
                callback();
            }

        // load the template
        } else {
            // make sure the template exists
            if(!magma.tmpl[template]) {
                throw new Error('loadTmpl: no tmpl named ' + template);
            }

            // compile the template
            $.template(template, magma.tmpl[template]);

            // look for references to other templates
            var matches = magma.tmpl[template].match(/{{tmpl[\(]?[^\)]*[\)]?[^"]+"[^"]+"}}/g);

            // may need to load before issuing callback
            if(matches) {
                var i = 0;
                while(i < matches.length) {
                    // grab last string which is the template reference
                    matches[i] = matches[i].match(/"[^"]+"/g).pop().replace(/"/g,'');

                    // can have recursive templates so check if already loaded
                    if(!$.template[matches[i]]) {
                        loadTmpl(matches[i++]);
                    } else {
                        // remove element since self reference
                        matches.splice(i,1);
                    }
                }

                // delay callback until dependancies loaded
                if(matches.length && callback) {
                    $.each(matches, function(i, tmplname) {
                        // pull for existance of template
                        var interval = setInterval(function() {
                            if($.template[tmplname]) {
                                clearInterval(interval); // stop pulling
                                matches.splice(i,1); // remove template
                                if(!matches.length) {
                                    callback();
                                }
                            }
                        }, 10);
                    });

                // all matches were recursive and removed
                } else if(callback) {
                    callback();
                }

            // no matches, go ahead and fill template if callback
            } else if(callback) {
                callback();
            }

            /*

            used to load templates from server

            $.ajax({
                url: "/js/templates/" + template + ".html",
                success: function(data) {
                    // code above here
                },
                error: function() {
                    throw new Error('No template named ' + template + '.');
                }
            });
            */
        }
    };

    /*** bytes to size ***/
    /**
     * Convert number of bytes into human readable format
     *
     * @source https://github.com/codeaid/snippets/blob/master/js/functions/bytesToSize.js
     *
     * @param integer bytes Number of bytes to convert
     * @param integer precision Number of digits after the decimal separator
     * @return string
     */
    var bytesToSize = function(bytes, precision) {
        var kilobyte = 1024;
        var megabyte = kilobyte * 1024;
        var gigabyte = megabyte * 1024;
        var terabyte = gigabyte * 1024;

        if ((bytes >= 0) && (bytes < kilobyte)) {
            return bytes + ' B';
        } else if ((bytes >= kilobyte) && (bytes < megabyte)) {
            return (bytes / kilobyte).toFixed(precision) + ' KB';
        } else if ((bytes >= megabyte) && (bytes < gigabyte)) {
            return (bytes / megabyte).toFixed(precision) + ' MB';
        } else if ((bytes >= gigabyte) && (bytes < terabyte)) {
            return (bytes / gigabyte).toFixed(precision) + ' GB';
        } else if (bytes >= terabyte) {
            return (bytes / terabyte).toFixed(precision) + ' TB';
        } else {
            return bytes + ' B';
        }
    };

    /*** fill tmpl ***/
    /**
     * Grabs the template from the server if it's not cached then fills it
     * Also massages data from server into form suitable for templates
     *
     * @param template string   template name
     * @param data object       object containing fields for template
     * @param callback function callback function to be called after template filled
     */
    var fill_tmpl = function(template, data, callback) {

        // transform data for certain templates
        switch(template) {
            case "folderList":
                // create a folder tree
                // loop with while to deal with splicing during loop
                var i = 0;

                while(data[i]) {
                    // folder href is lower case name with hyphens in place of spaces
                    data[i].link = data[i].name.toLowerCase().replace(/ /g, '-');

                    // child folder
                    if(typeof data[i].parentID == "number") {
                        // find child's parent
                        if(!(function(folders) {
                            for(var j in folders) {
                                if(folders[j].folderID === data[i].parentID) {
                                    // init if no folders yet
                                    folders[j].subfolders = folders[j].subfolders || [];
                                    folders[j].subfolders.push(data[i]);

                                    // remove folder from root
                                    data.splice(i, 1);

                                    return true; // success!

                                // scan subfolders
                                } else if(folders[j].subfolders) {
                                    if(arguments.callee(folders[j].subfolders)) {
                                        // found parent in subfolder
                                        return true;
                                    }
                                }
                            }

                        // no parent found!
                        })(data)) {
                            //console.log("Orphaned folder!", data[i]);

                            // goodbye little orphan...
                            data.splice(i, 1);
                        }

                    // increment since nothing removed
                    } else {
                        i++;
                    }
                }
            break;

            case "messageList":
                //convert dates and sizes to pretty strings
                for(var i in data) {
                    data[i].date = new Date(data[i].utc).toLocaleString();
                    data[i].size = bytesToSize(data[i].bytes, 2);
                    data[i].flag = {};
                }
            break;

            case "messageHeader":
                data.date = new Date(data.utc).toLocaleString();
            break;

            case "messageInfo":
                data.server.date = new Date(data.server.utc).toLocaleString();
            break;

            case "meta":
                if($.browser.webkit) {
                    data.browser = "Webkit";
                } else if($.browser.mozilla) {
                    data.browser = "Firefox";
                } else if($.browser.opera) {
                    data.browser = "Opera";
                } else if($.browser.msie) {
                    data.browser = "IE";
                }
            break;

            case "options":
                // check if listing aliases in composing
                if(data[0].email && data[0].name) {
                    // sort alphabetically by name
                    data.sort(function(a, b) {
                        if(a.name.toLowerCase() == b.name.toLowerCase()) {
                            return 0;
                        }
                        return a.name.toLowerCase() > b.name.toLowerCase() ? 1 : -1;
                    });

                    for(var i in data) {
                        data[i].value = data[i].email;
                        data[i].text = data[i].name + ' <' + data[i].email + '>';
                    }
                }

                // check if listing folders in search options
                if(typeof data[0].folderID === "number") {
                    data.sort(function(a, b) {
                        if(a.name.toLowerCase() == b.name.toLowerCase()) {
                            return 0;
                        }
                        return a.name.toLowerCase() > b.name.toLowerCase() ? 1 : -1;
                    });

                    for(var i in data) {
                        data[i].value = data[i].folderID;
                        data[i].text = data[i].name;
                    }
                }
            break;
        }

        // compile the template if needed
        loadTmpl(template, function() {
            var jquery = $.tmpl(template, data);

            // call the callback if given
            if(callback) {
                callback.call(jquery);
            }
        });
    };

    /*** fill state ***/
    var fill_state = function(state) {
        $.ajax({
            method: "get",
            url: "/" + state + ".html",
            success: function(data) {
                // cannot grab body tag so assumes outer dives and gets them
                // strips new lines and extra spaces
                var content = $(data.replace(/\n| {2,}/g,'')).filter('div');

                // replace body with content
                $('body').html(content);

                // watermarks
                switch(state) {
                    case "index":
                        $('#username, #password').fillWatermarks($('#login').find('label'));
                    break;

                    case "index.error":
                        $('#username, #password').fillWatermarks($('#login').find('label'));
                    break;

                    case "main.composing":
                        $('.composing-body textarea').fillWatermarks($('.composing-body label'));
                    break;

                    default:
                        // look for seach box
                        if($('.search').length) {
                            $('#search-input').fillWatermarks($('.search label'));
                        }
                    break;
                }

                // fill template comments with data
                content.comments(true).each(function() {
                    var stub = $(this).text(),
                        template,
                        options,
                        method,
                        params,
                        keys,
                        insert,
                        selector,
                        get_template = function(stub) {
                            // check for template options
                            if(stub.indexOf('(') > 0) {
                                template = stub.substr(0,stub.indexOf('('));
                                options = stub.substring(stub.indexOf('(') + 1, stub.indexOf(')')) || '{}';
                                // eval JSON - never do in production!!!
                                options = eval('(' + options + ')');
                            } else {
                                template = stub;
                                options = {};
                            }
                        },
                        get_destination = function(stub) {
                            insert = stub.split(':')[0];

                            if(insert === 'appendTo' || insert === 'prependTo' || insert === 'insertAfter' || insert === 'insertBefore') {
                                selector = stub.split(':')[1].replace(/'/g,'');
                            } else {
                                insert = '';
                            }
                        },
                        fill = function(data) {
                            switch(template) {
                                case "auth.failed":
                                    $('<div id="login-error"><p>' + data.failed + '</p></div>')[insert](selector);
                                break;

                                case "paragraph":
                                    if(data.locked) {
                                        $('<p>' + data.locked + '</p>')[insert](selector);
                                    } else if(data.text) {
                                        $('<p>' + data.text + '</p>')[insert](selector);
                                    }
                                break;

                                case "link":
                                    if(data.id === "continue") {
                                        $('<a id="continue" class="button" target="_blank" href="' + data.href + '">Continue</a>')[insert](selector);
                                    } else if(data instanceof Array) {
                                        if(data[0].attachmentID) {
                                            for(var i in data) {
                                                $('<a/>')
                                                    .attr({
                                                        id: "attachment-" + data[i].attachmentID,
                                                        "class": "attachment",
                                                        href: "attachment/" + data[i].name.toLowerCase().replace(/ /g, '-')
                                                    })
                                                    .text(data[i].name + " (" + bytesToSize(data[i].bytes, 2) + ")")
                                                    [insert](selector);
                                            }
                                        }
                                    }
                                break;

                                case "messageList":
                                    for(var i in data) {
                                        data[i].date = new Date(data[i].utc).toLocaleString();
                                        data[i].size = bytesToSize(data[i].bytes, 2);
                                        for(var j in data[i].tags) {
                                            data[i].tags[j] = [data[i].tags[j].toLowerCase().replace(/ /,'-'), data[i].tags[j]];
                                        }
                                    }

                                    fill_tmpl(template, data, function() {
                                        this[insert](selector);
                                    });
                                break;

                                case "contacts.list":
                                    if(state === 'main.contacts.info') {
                                        fill_tmpl(template, data, function() {
                                            this.each(function() {
                                                for(var i=1; i<$(this).children().length; i++) {
                                                    $(this).children().eq(i).remove();
                                                }
                                            });
                                            this[insert](selector);
                                        });
                                    } else {
                                        fill_tmpl(template, data, function() {
                                            this[insert](selector);
                                        });
                                    }
                                break;

                                case "tags":
                                    for(var i in data.tags) {
                                        $('<li/>')
                                            .append($('<a/>')
                                                .text(data.tags[i])
                                            )
                                            .appendTo('.tool-options ul');

                                    }
                                break;

                                default:
                                    fill_tmpl(template, data, function() {
                                        this[insert](selector);
                                    });
                                break;
                            }
                        };

                    // match template[({options})] method([{params}])[.keys] insert:'selector'
                    stub = stub.match(/[^ ]+(\([^\)]*\)[^ ]*|'[^']*')|[^ ]+/g);

                    // template method-call and destination
                    if(stub.length === 3) {
                        get_template(stub[0]);

                        method = stub[1].substr(0,stub[1].indexOf('('));

                        if(method) {
                            params = stub[1].substring(stub[1].indexOf('(') + 1, stub[1].indexOf(')')) || '{}';
                            params = eval('(' + params + ')');
                            keys = stub[1].substr(stub[1].indexOf(')') + 1, stub[1].length).split('.');

                            get_destination(stub[2]);

                            if(insert) {
                                $.get('/portal/mockiface', {method: method, params: params}, function(data) {
                                    data = data["error"] || data["result"];

                                    // get to the template data from the given key
                                    for(var i=0; i<keys.length; i++) {
                                        // ignore empty strings
                                        if(keys[i]) {
                                            data = data[keys[i]];
                                        }
                                    }

                                    // see if options were set
                                    if((function(obj) {
                                        for(var prop in obj) {
                                            if(obj.hasOwnProperty(prop)) { return true; }
                                        }
                                        return false;
                                    })(options)) {
                                        if(data instanceof Array && typeof data[0] == "object") {
                                            // add options to beginning
                                            data.unshift(options);

                                        } else if(typeof data == "object") {
                                            $.extend(data, options);

                                        // see if options were set
                                        } else {
                                            throw new Error(data, "is not an object and cannot be extended with:", options);
                                        }
                                    }

                                    fill(data);
                                });
                            } else {
                                throw new Error("Bad insert method: " + stub[2]);
                            }
                        // probably forgot method call parans
                        } else {
                            throw new Error("Bad method call: " + stub[1]);
                        }
                    // template and destination
                    } else if(stub.length === 2) {
                        get_template(stub[0]);
                        get_destination(stub[1]);

                        if(insert) {
                            fill(options);
                        } else {
                            throw new Error("Bad insert method: " + stub[1]);
                        }
                    }
                });
            },
            error: function(data) {
                throw new Error('State ' + url_args.state + ' was not found.');
            }
        });
    };

    /*** List static templates ***/
    var states = [],
        state,
        dialog_box,
        state_dialog = function() {

        // list with numberings to figure out folder structure
        list = false;

        if(list) {
            var template = '<input type="radio" name="templates" value="${state}" />${index}. ${state}<br />',
                dialog_box;

            dialog_box = $('<div id="state-selector"><form></form></div>').appendTo('body').css({height: "auto", backgroundColor: "white"});
            $.tmpl(template, states)
                .appendTo(dialog_box.children('form'))
                .siblings('input').css({margin: "0 5px 5px 0"})
                .filter(':first').attr('checked', true);
        } else {
            var treeTmpl = '<li id="folder-${folderID}" class="{{if subfolders}}expandable{{/if}}{{if opened}} open{{/if}}">{{if subfolders}}<span>+</span>{{/if}}<a href="${state}">${name}</a></li> {{if expanded}} <li> <ul> {{tmpl($item.data.subfolders) "tree"}} </ul> </li> {{/if}}';

            // order of folder list maps to state
            // set list to true above to see a numbered list with backtick to make changes below easier
            var tree = [{
                "name": "Advertisement"
            }, {
                "parentID": 4,
                "name": "Error"
            }, {
                "parentID": 4,
                "name": "Locked"
            }, {
                "name": "Logout"
            }, {
                "name": "Login"
            }, {
                "parentID": 8,
                "name": "Ad"
            }, {
                "parentID": 8,
                "name": "Finished Ad"
            }, {
                "parentID": 8,
                "name": "Finished"
            }, {
                "name": "Loading"
            }, {
                "parentID": 8,
                "name": "Warning"
            }, {
                "name": "Alert"
            }, {
                "name": "Composing"
            }, {
                "parentID": 13,
                "name": "Info"
            }, {
                "name": "Contacts"
            }, {
                "parentID": 17,
                "name": "Fields"
            }, {
                "parentID": 17,
                "name": "Reading Pane"
            }, {
                "parentID": 17,
                "name": "Sort"
            }, {
                "parentID": 27,
                "name": "Display Options"
            }, {
                "parentID": 27,
                "name": "Editing Folders"
            }, {
                "name": "Fatal Error"
            }, {
                "name": "Meta Info"
            }, {
                "parentID": 23,
                "name": "Display Attachments"
            }, {
                "parentID": 23,
                "name": "Extra Info"
            }, {
                "name": "Reading"
            }, {
                "name": "Scrape Contacts"
            }, {
                "name": "Search"
            }, {
                "name": "Identity Settings"
            }, {
                "name": "Mail"
            }, {
                "parentID": 27,
                "name": "Tags"
            }];

            for(var index in tree) {
                tree[index].folderID = parseInt(index);
                tree[index].state = states[index].state;
            }

            // create a folder tree
            // loop with while to deal with splicing during loop
            var i = 0;

            while(tree[i]) {
                // child folder
                if(typeof tree[i].parentID === "number") {
                    // find child's parent
                    if(!(function(folders) {
                        for(var j in folders) {
                            if(folders[j].folderID === tree[i].parentID) {
                                // init if no folders yet
                                folders[j].subfolders = folders[j].subfolders || [];
                                folders[j].subfolders.push(tree[i]);

                                // remove folder from root
                                tree.splice(i, 1);

                                return true; // success!

                            // scan subfolders
                            } else if(folders[j].subfolders) {
                                if(arguments.callee(folders[j].subfolders)) {
                                    // found parent in subfolder
                                    return true;
                                }
                            }
                        }

                    // no parent found!
                    })(tree)) {
                        console.log(tree[i]);

                        // goodbye little orphan...
                        tree.splice(i, 1);
                    }

                // increment since nothing removed
                } else {
                    i++;
                }
            }

            dialog_box = $('<div id="state-selector"><ul></ul></div>');

            $.template('tree', treeTmpl);
            $.tmpl('tree', tree)
                .appendTo(dialog_box.find('ul'));
        }

        dialog_box.appendTo('body')
            .css({height: "auto", backgroundColor: "white"})
            .delegate('span', 'click', function(event) {
                event.preventDefault();

                var tmplItem = $(this).parent().tmplItem();
                tmplItem.data.opened = !tmplItem.data.opened;
                if($(this).parent().hasClass("expandable")) {
                    tmplItem.data.expanded = !tmplItem.data.expanded;
                }
                tmplItem.update();
            }).delegate('a', 'click', function(event) {
                event.preventDefault();
                fill_state($(this).attr('href'));
            });

        // prevent overlay from overflowing
        $('body').css("overflow", "hidden");

        dialog_box.dialog({
            resizable: false,
            draggable: true,
            modal: true,
            title: "Choose a state",
            close: function() {
                $(this).remove();
                $('body').css("overflow", "auto");
            }
        });
    };

    $(window).keypress(function(event) {
        // type back tick '`'
        if(event.which === 96 && !$('#state-selector').length) {
            if(!states.length) {
                $.getJSON("dev/static/statenames.json", function(data) {
                    for(var index in data) {
                        var state = data[index];
                        states.push({state: state.substring(0, state.indexOf('.state')), index: index});
                    }
                    state_dialog();
                });
            } else {
                state_dialog();
            }
        }
    });
});
