#!/usr/bin/php
<?php

include 'utils.php';

// use dev directory for abs path
#$abs_path = '/var/www/html/webmail/';
$script_path = realpath(dirname(__FILE__));
$project_path = realpath($script_path . '/../../') . '/';
$output_path = realpath($project_path . '/res/pages/webmail/') . '/';
$abs_path = realpath($project_path . '/web/') . '/';

$dev_path = $abs_path;

// directories
$plugins_directory = $dev_path . 'js/plugins/';
$dev_plugins_directory = $plugins_directory . 'dev/';
$test_directory = $dev_path . 'js/test/';

$scripts_directory = $dev_path . 'js/scripts/';
$dev_scripts_directory = $scripts_directory . 'dev/';

// order scripts
$last_scripts = array( 'application.js' );

// save paths
$js_directory = $output_path . 'js/';
$plugins_filename = 'plugins.js';
$script_filename = 'script.js';

// blocks and templates paths
$blocks_directory = $dev_path . 'js/blocks/';
$templates_directory = $dev_path . 'js/templates/';

// run jshint
echo shell_color( 'Start:', 'cyan' ) . " jshint\n";

$scripts = get_filenames( $scripts_directory, '/\.js$/' );

foreach ( $scripts as $script ) {
    echo shell_color( $script, 'yellow' ) . "\n";
    $result = `cd $test_directory && js lint.js < $scripts_directory$script`;

    if ( preg_match( '/^Passed!/', $result ) ) {
        echo shell_color( $result, 'green' );
    } else {
        echo shell_color( $result, 'red' );
    }
}

// start concat
echo shell_color( 'Start:', 'cyan' ) . " compiling html blocks and jquery templates into js properties\n";

/**
 * Assembles html blocks into name value pairs
 *
 * @param directory     directory to scan for html files
 */
function build_properties( $directory ) {
    $filenames = get_filenames( $directory );

    $properties = '';

    foreach ( $filenames as $filename ) {

        // use filenames for property names
        $property = preg_replace('/\.html/', '', $filename);

        // make property names are valid javascript property names
        if ( preg_match('/[\.\- ]/', $property) ) {
            echo shell_color( 'Error', 'red' ) . " file $filename contains improper javascript object property characters\n";
        } else {
            $value = file_get_contents( "$directory$filename" );
            
            // remove newlines and extra spaces
            $value = preg_replace( '/ {2,}|\n/', '', $value );
            
            // escape single quotes
            $value = preg_replace( '/\'/', '\\\'', $value );

            if( $filename == end($filenames) ) {
                $properties .= "'$property': '$value'\n";
            } else {
                $properties .= "'$property': '$value',\n";
            }
        }
    }

    return $properties;
}

// insert into scripts below
$blocks = build_properties( $blocks_directory );
$templates = build_properties( $templates_directory );

echo shell_color( 'Start:', 'cyan' ) . " concating scripts\n";

/**
 * Concats javascript files in a given directory into a single script
 * 
 * @param directory directory to scan for html files
 * @param first     array of filenames to be inserted first - order of array preserved
 * @param last      array of filenames to be inserted last - order of array upheld
 * @param ignore    filenames to ignore
 */
function concat_js( $directory, $first = null, $last = null ) {
    $js = '';
    $filenames = get_filenames( $directory, '/\.js$/' );

    // insert first files
    if ( $first ) {
        foreach( $first as $filename ) {
            // array_search returns false if search failed
            if ( is_numeric( $key = array_search( $filename, $filenames ) ) ) {
                $js .= file_get_contents( "$directory$filename" );
                unset( $filenames[ $key ] );
            } else {
                echo shell_color( 'Error:', 'red' ) . " given first file $filename is not in $directory\n";
            }
        }
    }

    // temp store last files and remove
    $temp = '';

    if ( $last ) {
        foreach( $last as $filename ) {
            // array_search returns false if search failed
            if ( is_numeric( $key = array_search( $filename, $filenames ) ) ) {
                $temp .= file_get_contents( "$directory$filename" );
                unset( $filenames[ $key ] );
            } else {
                echo shell_color( 'Error:', 'red' ) . " given last file $filename is not in $directory\n";
            }
        }
    }
    
    // concat other files
    foreach( $filenames as $filename ) {
        $js .= file_get_contents( "$directory$filename" );
    }

    // append last
    $js .= $temp;

    return $js;
}

// concat plugins
$plugin = concat_js( $plugins_directory );
$plugin .= concat_js( $dev_plugins_directory );
if ( file_put_contents( "$js_directory$plugins_filename", $plugin ) ) {
    echo shell_color( 'Success:', 'green' ) . " saved $plugins_filename to $js_directory\n";
} else {
    echo shell_color( 'Failure:', 'red' ) . " could not save $plugins_filename to $js_directory\n";
}

// concat scripts
$scripts = concat_js( $scripts_directory, null, $last_scripts );

// fill in blocks and templates
$scripts = preg_replace( '/\/\/\$\{blocks\}\n/', $blocks, $scripts );
$scripts = preg_replace( '/\/\/\$\{tmpl\}\n/', $templates, $scripts );

$scripts .= concat_js( $dev_scripts_directory );
if ( file_put_contents( "$js_directory$script_filename", $scripts ) ) {
    echo shell_color( 'Success:', 'green' ) . " saved $script_filename to $js_directory\n";
} else {
    echo shell_color( 'Failure:', 'red' ) . " could not save $script_filename to $js_directory\n";
}
