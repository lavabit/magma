/*
 * runs JSHint in spidermonkey
 * must be run in this directory for paths to work
 *
 * depends on spidermonkey
 * with epel 6
 * yum install js-devel
 *
 * pass in script with shell redirect
 * js test.js < script.js
 */

// read in script
// http://whereisandy.com/code/jslint/
var input = '',
    line,
    blankcount = 0;

while(blankcount < 10) {
    line = readline();

    if(!line) {
        blankcount += 1;
    } else {
        blankcount = 0;
    }

    input += line + "\n";
}
input = input.substring(0, input.length - blankcount);

load('jshint.js');

// jshint returns true if passed and false on fail
var result = JSHINT(input, {
    predef: ['tinyMCE', 'CKEDITOR', 'CKEDITOR_BASEPATH'], // predefined globals
    browser: true, // allows browser globals
    jquery: true, // allows jquery global
    curly: true, // requires braces for if/while
    eqeqeq: true, // requires === / !==
    immed: true, // requires immediate invocation to be wrapped in parans
    maxerr: 1000, // stops at 50 by default
    noarg: true, // disallows arguments.callee/arguments.caller
    plusplus: true, // prohibit ++/--
    undef: true, // require non-globals to be declared before using
    sub: true // allows other forms of property access other than dot
});

if(result) {
    print('Passed!');
} else {
    for(var i in JSHINT.errors) {
        var e = JSHINT.errors[i];
        print('Problem at line ' + e.line + ' character ' + e.character + ': ' + e.reason + "\n\n" + e.evidence.replace(/^ +/, '') + "\n");
    }
}
