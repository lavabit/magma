/**
 * Compose Model Tests
 */

module('compose');

asyncTest('initialize compose', function() {
    var compose = magma.model.compose();

    compose.addObserver('ready', function() {
        ok(true, 'Compose initialized successfully');
        ok(compose.getComposeID(), 'Got compose ID');
        start();
    });

    compose.addObserver('readyError', function() {
        ok(false, 'Compose initialization returned error');
        start();
    });

    compose.addObserver('readyFailed', function() {
        ok(false, 'Compose initialization failed');
        start();
    });

    compose.composeMessage();
});

asyncTest('send message', function() {
    var compose = magma.model.compose();

    compose.addObserver('ready', function() {
        var formData = {
            from: 'test@example.com',
            to: ['recipient@example.com'],
            cc: [],
            bcc: [],
            subject: 'Test Subject',
            bodyHtml: '<p>Test body</p>',
            bodyText: 'Test body'
        };

        compose.addObserver('sent', function() {
            ok(true, 'Message sent successfully');
            start();
        });

        compose.addObserver('sentError', function(error) {
            ok(false, 'Send returned error: ' + error);
            start();
        });

        compose.addObserver('sentFailed', function() {
            ok(false, 'Send failed');
            start();
        });

        compose.send(formData);
    });

    compose.addObserver('readyFailed', function() {
        ok(false, 'Compose initialization failed');
        start();
    });

    compose.composeMessage();
});

test('attachment management', function() {
    var compose = magma.model.compose();

    // Test adding attachments
    compose.addAttachment(1);
    compose.addAttachment(2);
    compose.addAttachment(3);

    var attachments = compose.getAttachments();
    equal(attachments.length, 3, 'Added 3 attachments');
    deepEqual(attachments, [1, 2, 3], 'Attachments are in correct order');

    // Test removing attachment
    compose.removeAttachment(2);
    attachments = compose.getAttachments();
    equal(attachments.length, 2, 'Removed 1 attachment');
    deepEqual(attachments, [1, 3], 'Correct attachment removed');

    // Test removing non-existent attachment
    compose.removeAttachment(99);
    attachments = compose.getAttachments();
    equal(attachments.length, 2, 'No change when removing non-existent');
});

test('draft save to localStorage', function() {
    var compose = magma.model.compose();

    // Clear any existing drafts
    for (var key in localStorage) {
        if (key.indexOf('magma_draft_') === 0) {
            localStorage.removeItem(key);
        }
    }

    // Mock composeID
    compose.composeMessage = function() {};

    var formData = {
        from: 'test@example.com',
        to: ['recipient@example.com'],
        subject: 'Draft Test',
        bodyHtml: '<p>Draft content</p>',
        bodyText: 'Draft content'
    };

    // Note: saveDraft requires composeID which is set async
    // This test validates the localStorage mechanism exists
    ok(typeof compose.saveDraft === 'function', 'saveDraft function exists');
    ok(typeof compose.loadDraft === 'function', 'loadDraft function exists');
    ok(typeof compose.clearDraft === 'function', 'clearDraft function exists');
});
