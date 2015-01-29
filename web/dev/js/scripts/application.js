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
