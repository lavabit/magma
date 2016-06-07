#!/usr/bin/php
<?php
/**
 * Prints JSON interface as html report
 *
 * Requires Markdown.pl - http://daringfireball.net/projects/markdown/
 * Markdwon.pl saved to /usr/local/bin/
 */

include "utils.php";

#$abs_path = '/var/www/html/webmail/dev/doc/';
$script_path = realpath(dirname(__FILE__));
$project_path = realpath($script_path . '/../../') . '/';
$output_path = realpath($project_path . '/res/pages/doc/') . '/';
$abs_path = realpath($project_path . '/web/doc/') . '/';

// report
$report_name = 'magma-doc.html';
$report_directory = $output_path;

// tester
$tester_name = 'method-tester.html';
$tester_directory = $output_path;

// inserted sections
$header = $abs_path . 'header.html';
$intro = $abs_path . 'intro.md';
$tester = $abs_path . 'tester-form.html';
$footer = $abs_path . 'footer.html';

// directories
$markdown_dir = $abs_path . 'methods/';
$responses_dir = $abs_path . '../json/responses/';
$requests_dir = $abs_path . '../json/requests/';

// method markdown headings
$request_params_heading = 'Request Params';
$request_example_heading = 'Example Request';
$response_params_heading = 'Response Params';
$response_example_heading = 'Example Response';

// workflow examples
$workflow_md = $abs_path . 'workflow.md';
$workflow_html = 'workflow.html';
$workflow_directory = $output_path;

// starting
echo shell_color( 'Start:', 'cyan' ) . " compiling $report_name\n";

// scan responses directory for file names
$filenames = get_filenames( $markdown_dir );

// execute Markdown script
function markdown( $filename ) {
    return shell_exec('Markdown.pl ' . $filename );
}

// insert html header
$html = file_get_contents( $header );
$tester_html = file_get_contents( $header );

$html .= "<h1>Magma JSON Interface</h1>\n";
$tester_html .= "<h1>Magma JSON Interface Method Tester</h1>\n";

// insert introduction from file
$html .= markdown( $intro );

// replace updated stub with date/time
date_default_timezone_set( 'America/Chicago' );
$html = preg_replace( '/\$\{updated\}/', date( 'r' ), $html);

// method tester
$html .= "<h2>Method Tester</h2>\n";
$html .= file_get_contents( $tester );
$tester_html .= file_get_contents( $tester );

$tester_methods = ''; // append to below
$tester_requests = array();

// table of contents
// leave a hook to replace later
$html .= '${contents}';
$contents = '<div id="contents"><h2>Table of Contents</h2><ul>';

foreach( $filenames as $filename ) {
    $method = preg_replace('/\.md/','', $filename);

    $html .= '<div class="method">';
    $html .= '<h2 id="' . $method . '">Method: ' . $method . '</h2>';
    $html .= '<a href="#contents">back to contents</a>';

    // add method to tester options
    $tester_methods .= '<option value="' . $method . '">' . $method . '</option>';

    // append example request to tester requests for stubs
    array_push( $tester_requests, "'$method': " . preg_replace( '/ {2}|\n/', '', file_get_contents( "$requests_dir$method.json" ) ) );

    // add method to table of contents
    $contents .= '<li><a href="#' . $method . '">' . $method . '</a></li>';

    if ( file_exists( "$markdown_dir$filename" ) ) {
        $markup = markdown( "$markdown_dir$filename" );

        // if response params
        if ( preg_match( "/<h3>$request_params_heading<\/h3>/", $markup ) ) {

            // fill in example requests file
            $example = "<h3>$request_example_heading</h3>";
            if ( file_exists( "$requests_dir$method.json" ) ) {
                $example .= '<pre>' . htmlentities( file_get_contents( "$requests_dir$method.json" ) ) . "</pre>\n";
            } else {
                $example .= "<p>No example request</p>\n";
            }

            // append before response params heading
            if ( $pos = strpos( $markup, "<h3>$response_params_heading</h3>" ) ) {
                $markup = substr( $markup, 0, $pos ) . $example . substr( $markup, $pos );
            
            // just append to end
            } else {
                $markup .= $example;
            }
        }

        // append example resonses from file
        $markup .= "<h3>$response_example_heading</h3>";
        if ( file_exists( "$responses_dir$method.json" ) ) {
            $markup .= '<pre>' . htmlentities( file_get_contents( "$responses_dir$method.json" ) ) . '</pre>';
        } else {
            $markup .= '<p>No example response</p>';
        }

        $html .= $markup;
    } else {
        $html .= '<p>No description</p>';
    }

    $html .= '</div>';
}

$contents .= '</ul></div>';

// insert test options
$html = preg_replace( '/\$\{method-options\}/', $tester_methods, $html );
$tester_html = preg_replace( '/\$\{method-options\}/', $tester_methods, $tester_html );

// insert table of contents
$html = preg_replace( '/\$\{contents\}/', $contents, $html );

// insert example requests into tester script
$footer = file_get_contents( $footer );
$footer = preg_replace( '/\$\{requests\}/', implode( ",\n", $tester_requests ), $footer );

$html .= $footer;
$tester_html .= $footer;

file_put_contents("$report_directory$report_name", $html);
file_put_contents("$tester_directory$tester_name", $tester_html);

echo shell_color( 'Success:', 'green' ) . " $report_name saved to $report_directory\n";
echo shell_color( 'Success:', 'green' ) . " $tester_name saved to $tester_directory\n";

// generate workflow example doc
$html = markdown( $workflow_md );

file_put_contents( "$workflow_directory$workflow_html", $html );

echo shell_color( 'Success:', 'green' ) . " $workflow_html saved to $workflow_directory\n";
