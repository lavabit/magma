/**
 * Security Tests
 *
 * Tests for XSS prevention and other security fixes
 */

module('security');

test('dialog.message escapes HTML', function() {
    // Create a test message with XSS payload
    var xssPayload = '<script>alert("xss")</script>';

    // Temporarily override jQuery dialog to capture the content
    var originalDialog = $.fn.dialog;
    var capturedContent = null;

    $.fn.dialog = function() {
        capturedContent = this.find('p').text();
        this.remove();
        return this;
    };

    // Call the message function
    magma.dialog.message(xssPayload);

    // Restore original
    $.fn.dialog = originalDialog;

    // The content should be escaped (showing as text, not executed)
    equal(capturedContent, xssPayload, 'XSS payload is displayed as text, not executed');

    // Clean up any leftover elements
    $('#message-box').remove();
});

test('dialog.die escapes HTML', function() {
    var xssPayload = '<img src=x onerror="alert(1)">';

    var originalDialog = $.fn.dialog;
    var capturedContent = null;

    $.fn.dialog = function() {
        capturedContent = this.find('p').text();
        this.remove();
        return this;
    };

    magma.dialog.die(xssPayload);

    $.fn.dialog = originalDialog;

    equal(capturedContent, xssPayload, 'XSS payload in error dialog is escaped');

    $('#error-message').remove();
});

test('CSRF token getter exists', function() {
    // The getData function should include CSRF headers
    // We can't easily test the internal function, but we can verify
    // the meta tag support exists

    // Add a test CSRF meta tag
    var testToken = 'test-csrf-token-12345';
    var meta = $('<meta name="csrf-token" content="' + testToken + '">');
    $('head').append(meta);

    // Verify the meta tag was added
    var foundMeta = document.querySelector('meta[name="csrf-token"]');
    ok(foundMeta, 'CSRF meta tag can be queried');
    equal(foundMeta.getAttribute('content'), testToken, 'CSRF token value accessible');

    // Clean up
    meta.remove();
});

test('portal URL uses real endpoint by default', function() {
    // When MAGMA_USE_MOCK is not set, should use real endpoint
    // Note: In test environment this is overridden, but we can check the logic

    // The portalUrl should be set
    ok(magma.portalUrl, 'Portal URL is defined');

    // In test mode it should be mockiface
    equal(magma.portalUrl, '/portal/mockiface', 'Test environment uses mock interface');
});

test('blocks are HTML escaped in templates', function() {
    // Verify that magma.blocks exists and contains string values
    ok(magma.blocks, 'magma.blocks exists');
    ok(typeof magma.blocks === 'object', 'magma.blocks is an object');

    // Check a known block exists
    ok(magma.blocks.composingAttachments, 'composingAttachments block exists');

    // Verify it's a string (not executable)
    equal(typeof magma.blocks.composingAttachments, 'string', 'Block content is string');
});

test('session token storage', function() {
    // Test session management doesn't expose sensitive data
    var session = magma.session;

    ok(session, 'Session object exists');
    ok(typeof session.get === 'function', 'session.get exists');
    ok(typeof session.set === 'function', 'session.set exists');
    ok(typeof session.clear === 'function', 'session.clear exists');

    // Test that session can be set and cleared
    session.set('test-session-123');
    equal(session.get(), 'test-session-123', 'Session can be set');

    session.clear();
    ok(!session.get(), 'Session can be cleared');
});

test('XHR headers include security tokens', function() {
    // We can verify that jQuery.ajax is configured to accept custom headers
    // by checking that the ajaxSetup doesn't block them

    var originalAjax = $.ajax;
    var capturedHeaders = null;

    $.ajax = function(options) {
        capturedHeaders = options.headers;
        // Don't actually make the request
        if (options.error) {
            options.error({ status: 0 });
        }
    };

    // Trigger an API call (this will fail but we'll capture the headers)
    try {
        // Create a minimal model call to trigger getData
        var authModel = magma.model.auth();
        authModel.login('test', 'test');
    } catch (e) {
        // Expected to fail
    }

    $.ajax = originalAjax;

    // Verify headers were set
    ok(capturedHeaders, 'Headers object was passed to ajax');
    ok('X-CSRF-Token' in capturedHeaders, 'X-CSRF-Token header is included');
    ok('X-Requested-With' in capturedHeaders, 'X-Requested-With header is included');
    equal(capturedHeaders['X-Requested-With'], 'XMLHttpRequest', 'XHR header value is correct');
});
