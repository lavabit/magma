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
