var auth = function() {
    var authModel = magma.model.auth(),
        addObservers = function(callbacks) {
            if(callbacks) {
                if(callbacks.success) {
                    authModel.addObserver('success', callbacks.success);
                }

                if(callbacks.failed) {
                    authModel.addObserver('failed', callbacks.failed);
                }
            }
        };

    return {
        login: function(callbacks) {
            addObservers(callbacks);
            authModel.login('magma', 'test');
        },
        empty: function(type, callbacks) {
            addObservers(callbacks);

            switch(type) {
                case 'username':
                    authModel.login('', 'anything');
                break;

                case 'password':
                    authModel.login('anything', '');
                break;

                case 'both':
                    authModel.login('', '');
                break;
            }
        },
        fail: function(callbacks) {
            addObservers(callbacks);
            authModel.login('wrong', 'wrong');
        },
        logout: function(callbacks) {
            authModel.logout();
        }
    };
};

module('auth');

asyncTest('no username or password', function() {
    auth().empty('both', {
        success: function() {
            ok(false, 'Empty username and password should not be successful.');
            auth.logout();
            start();
        },
        failed: function(message) {
            equal(message, 'Username and password are required.');
            start();
        }
    });
});

asyncTest('no username', function() {
    auth().empty('username', {
        success: function() {
            ok(false, 'Empty username should not be successful.');
            auth.logout();
            start();
        },
        failed: function(message) {
            equal(message, 'Username is required.');
            start();
        }
    });
});

asyncTest('no password', function() {
    auth().empty('password', {
        success: function() {
            ok(false, 'Empty password should not be successful.');
            auth.logout();
            start();
        },
        failed: function(message) {
            equal(message, 'Password is required.');
            start();
        }
    });
});

asyncTest('incorrect', function() {
    auth().fail({
        success: function() {
            ok(false, 'Wrong username and password should not be successful.');
            auth.logout();
            start();
        },
        failed: function(message) {
            equal(message, 'The username and password provided are incorrect, please try again.');
            start();
        }
    });
});

asyncTest('success', function() {
    auth().login({
        success: function() {
            ok(true, 'Successfully logged in.');
            auth().logout();
            start();
        },
        failed: function(message) {
            ok(false, 'Failed on correct username and password.');
            start();
        }
    });
});
