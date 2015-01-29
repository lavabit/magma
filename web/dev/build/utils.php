<?php
    /*** Echo colored text ***/
    function shell_color($string, $color = 'white') {
        $colors = array(
            "black" => 30,
            "red" => 31,
            "green" => 32,
            "yellow" => 33,
            "blue" => 34,
            "purple" => 35,
            "cyan" => 36,
            "white" => 37);

        if(!array_key_exists($color, $colors)) {
            return $color . " is not a color choice!\n";
        } else {
            return shell_exec("echo -ne '\e[1;" . $colors[$color] . "m$string\e[m'");
        }
    }

    /*** Get files in a directory ***/
    function get_filenames($directory, $match = '') {
        $filenames = scandir($directory);

        // get rid of any hidden files (., .., and vims swp files) or files that don't match expression
        $count = count($filenames);

        for($i=0; $i<$count; $i++) {
            if ( preg_match('/^\./', $filenames[$i] ) || $match && !preg_match( $match, $filenames[$i] ) ) {
                unset($filenames[$i]);
            }
        }

        return $filenames;
    }
