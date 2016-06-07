#!/usr/bin/php
<?php 
/*** Usage ***/
if($argc == 2 && in_array($argv[1], array('help', '--help', '-help', '-h'))) { ?>
Usage:
    clean [-c] - cleans compiled html files before building
    dryrun [-d] - shows html files that will be removed with clean
    verbose [-v] - lists compiled html files that were added

<?php
} else {
    /*** Includes ***/
    include "utils.php";
    
    /*** Local Vars ***/
    $script_path = realpath(dirname(__FILE__));
	$project_path = realpath($script_path . '/../../') . '/';
	$output_path = realpath($project_path . '/res/pages/webmail/') . '/';
	$abs_path = realpath($project_path . '/web/') . '/';
    #$abs_path = '/var/www/html/webmail/dev/';

    // html blocks
    $blocks_directory = $abs_path . 'blocks/';

    // state files and compiled states
    $static_directory = $abs_path . 'static/';
    $static_states_directory = $static_directory . 'states/';
    $static_compiled_directory = $output_path; // webmail root

    // state names to give to nav modal
    $statenames_path = $static_directory . 'statenames.json';

    // js states for ajax html blocks
    $js_states_directory = $abs_path . 'js/states/';
    $js_compiled_directory = $abs_path . 'js/blocks/';

    // start
    echo shell_color( 'Start:', 'cyan' ) . " compiling html blocks\n";

    /*** Clean compiled html files ***/
    function clean_html_directory($directory, $dryrun = false) {
        $filenames = get_filenames($directory);

        foreach($filenames as $filename) {
            // skip anything that isn't html
            if(preg_match('/\.html$/', $filename)) {
                if($dryrun) {
                    echo shell_color('Will remove:', 'yellow') . " $directory$filename\n";
                } else {
                    unlink($directory . $filename);
                    echo shell_color('Removed:', 'yellow') . " $directory$filename\n";
                }
            }
        }
    }
    
    // run dryrun then exit
    if($argc == 2 && in_array($argv[1], array('dryrun', '-d'))) {
        clean_html_directory($static_compiled_directory, true);
        clean_html_directory($js_compiled_directory, true);
    } else {
        /*** Static HTML ***/
        /*
         * Gets indent and returns level
         */
        function get_level($blockname, $statename) {
            // match spaces at beginning of line
            // pass spaces by ref -> returns an array
            preg_match('/^\s*/', $blockname, $indent);

            // assumes indentions are always four spaces
            if(strlen($indent[0])%4) {
                die(shell_color('Error:', 'red') . " Use four spaces for indention of block " . trim($blockname) . " in state file$statename\n");
            } else {
                // convert array to a string since always one element
                // count number of spaces and divide by four
                return strlen($indent[0])/4;
            }
        }

        /*
         * Embeds html blocks according to state file
         */
        function embed_html_blocks($states_directory, $blocks_directory, $compiled_directory, $verbose = false, $wrap = true, $strip_comments = false) {
            if($wrap) {
                // get wrap.html for wrapping all blocks with body/head html
                $wrap_html = file_get_contents($blocks_directory . "wrap.html");
            }

            $statenames = get_filenames($states_directory);

            // regex pattern for block stub
            $block_stub_regex = '/ *<!--[ ]*\[BLOCK\][ ]*-->\n/';

            foreach($statenames as $statename) {
                // check for .state extension
                if(preg_match('/\.state$/', $statename)) {
                    // returns file lines in an array
                    // files contain blocknames
                    $blocknames = file($states_directory . $statename);

                    $html = array('');  // html stack
                    $namespace = array(''); // namespace stack for filenames

                    // action of current block depends on indention of next block
                    for($i = 0; $i < count($blocknames); $i++) {
                        $blockname = trim($blocknames[$i]);
                        $level = get_level($blocknames[$i], $statename);

                        if($i < count($blocknames) - 1) {
                            $next_blockname = $blocknames[$i + 1];
                            $next_level = get_level($next_blockname, $statename);
                        } else {
                            $next_blockname = 'EOF';
                            $next_level = 0;
                        }

                        // grab the html
                        $block = file_get_contents($blocks_directory . implode($namespace) . $blockname . ".html");
                        if(!$block) {
                            die(shell_color('Error:', 'red') . " No block named $blockname in $states_directory$statename\n");
                        }

                        // concat block to top of html stack
                        $html[count($html) - 1] .= $block;
                        
                        // block is higher level than next - embed block(s)
                        if(($level_diff = $level - $next_level) > 0) {

                            // embed for each level
                            for($j = 0; $j < $level_diff; $j++) {
                                $block = array_pop($html);
                                
                                preg_match_all($block_stub_regex, end($html), $block_stub);

                                if(!count($block_stub[0])) {
                                    die(shell_color('Error:', 'red') . " No block comment stub to embed into state $states_directory$statename\n" . end($html));
                                } elseif(count($block_stub[0]) > 1) {
                                    die(shell_color('Error:', 'red') . " Too many block comment stubs to embed into state $states_directory$statename\n" . end($html));
                                }

                                // get block indention
                                preg_match('/^\s*/', $block_stub[0][0], $indent);

                                // get rid of any unused [BLOCK] statements
                                $block = preg_replace($block_stub_regex, '', $block);

                                // add indents to html blocks so alignment is correct when embedded
                                $block = preg_replace('/(.+)\n/', $indent[0] . '\1' . "\n", $block);

                                array_pop($namespace);
                                $html[count($html) - 1] = preg_replace($block_stub_regex, $block, end($html));
                            }

                        // block is lower level than next - concat block to current level then push a new level
                        } elseif(($level_diff = $next_level - $level) > 0) {
                            // make sure contained blocks are indicated by one level of indention in
                            if($level_diff != 1) {
                                die(shell_color('Error:', 'red') . " Contained block " . trim($next_blockname) . " indicated by more than one level of indention in state file $statename\n");
                            } else {
                                array_push($html, '');
                                
                                // trim anything after + for overloaded blocks
                                if(strpos($blockname, '+')) {
                                    $blockname = substr($blockname, 0, strpos($blockname, '+'));
                                }

                                array_push($namespace, $blockname . '.');
                            }

                        // get rid of unused embed comments
                        } else {
                            $html[count($html) - 1] = preg_replace($block_stub_regex, '', $html[count($html) - 1]);
                        }
                    }

                    // convert array to string
                    $html = $html[0];

                    if($wrap) {
                        // indent one level then wrap in main body/head html
                        $html = preg_replace('/(.+)\n/', str_repeat(' ', 4) . '\1' . "\n", $html);
                        $html = preg_replace($block_stub_regex, $html, $wrap_html);
                    }

                    if($strip_comments) {
                        $html = preg_replace('/ *\<![ \r\n\t]*(--.*--[ \r\n\t]*)\>\n?/', '', $html);

                    // strip any leftover block comments
                    } else {
                        $html = preg_replace($block_stub_regex, '', $html);
                    }

                    // save to statename.html file
                    $filename = preg_replace('/\.state/', '.html', $statename);
                    file_put_contents($compiled_directory . $filename, $html);
                    if($verbose) {
                        echo shell_color('Added:', 'cyan') . " $compiled_directory$filename\n";
                    }

                // filename doesn't have .state extension
                } else {
                    echo shell_color('Warning:', 'red') . " ignoring $states_directory$statename since it does not have state extension\n";
                }
            }

            echo shell_color('Success:', 'green') . " html saved to $compiled_directory\n";
        }

        // remove unused files
        if($argc == 2 && in_array($argv[1], array('clean', '-c'))) {
            clean_html_directory($static_compiled_directory);
            clean_html_directory($js_compiled_directory);
        }

        $verbose = $argc == 2 && in_array($argv[1], array('verbose', '-v'));

        embed_html_blocks($static_states_directory, $blocks_directory, $static_compiled_directory, $verbose);
        embed_html_blocks($js_states_directory, $blocks_directory, $js_compiled_directory, $verbose, false, true);

        /*** Generate JSON array of static template filenames for listing in dev js ***/ 
        file_put_contents($statenames_path, json_encode(array_values(get_filenames($static_states_directory))));
        echo shell_color('Success:', 'green') . " statenames JSON saved to $statenames_path\n";
    }
}
