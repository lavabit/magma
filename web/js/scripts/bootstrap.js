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
