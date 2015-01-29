#!/bin/bash

# Derive the folder, and file name from the first argument
DIR=`dirname $1`
FILE=`basename $1`
cd $DIR

# We can only generation function prototypes using code files
SUFFIX=`echo $FILE | awk -F'.' '{ print $NF }'`
if [ "$SUFFIX" != "c" ] && [ "$SUFFIX" != "h" ]; then
	 /usr/bin/tput setaf 1; echo "replacement can only be performed on code files..." 1>&2; /usr/bin/tput sgr0
	 read -n 1 -t 120
	 exit 1
fi

/bin/cp "$FILE" "$FILE.X" 

replace() {
	/bin/sed -i -e "s/$1/$2/g" $FILE
}

remove() {
	/bin/sed -i -e "/$1/d;" $FILE
}

# Ensure the heading has a leading space.
/bin/sed -i -e "1 s/\/\*\*/\n&/g" $FILE

#        A variable name 
#        \\([\"A-Za-z0-9\_\.\>\-\+\(\)\/]\+\\)

# Replacement rules
replace "sizer_t" "size_t"
replace "sql_row_t \\*" "row_t \\*"
replace "\\(\s\\)wildcard_config_t\\(\s\\)" "\\1smtp_wildcard_config_t\\2"
replace "reducer_t\(\s\)" "compress_t\1" 
replace "user_cache" "objects.users"
replace "sql_result_t \\*" "table_t \\*" 
replace "unsigned short int\(\s\)" "uint8_t\\1"
replace "\(\s\)short int\(\s[A-Za-z]\+\)" "\\1int_t\2"
replace "short\(\s\)" "int_t\1"
replace "unsigned long long\(\s\)" "uint64_t\1" 
replace "unsigned long \(\s\)" "uint64_t\1" 
replace "unsigned \(\s\)" "uint32_t\1" 
replace "unsigned int\(\s[A-Za-z]\+\)" "uint64_t\1" 
replace "\([^\s]\?\)int\(\s[A-Za-z]\+\)" "\\1int_t\\2"
replace "\([^\s]\?\)float\(\s\)" "\\1float_t\\2"
replace "unsigned char\(\s\)" "uchr_t\1"
replace "\([^\s]\?\)char\(\s\)" "\\1chr_t\\2"

replace "sizeof(unsigned long long)" "sizeof(uint64_t)"
replace "sizeof(unsigned long)" "sizeof(uint64_t)"
replace "sizeof(unsigned int)" "sizeof(uint32_t)"
replace "sizeof(unsigned)" "sizeof(uint32_t)"

remove "parameters\\[\\([0-9]*\\)\\]\\.buffer_length = sizeof\(.*\);"
replace "parameters\\[\\([0-9]*\\)\\]\\.is_unsigned = 1;" "parameters\\[\\1]\\.is_unsigned = true;"
replace "\(.*\)parameters\[\\([0-9]*\\)\]\.buffer_type = MYSQL_TYPE_LONGLONG;" "&\n\\1parameters[\\2].buffer_length = sizeof(uint64_t);"
replace "\(.*\)parameters\[\\([0-9]*\\)\]\.buffer_type = MYSQL_TYPE_LONG;" "&\n\\1parameters[\\2].buffer_length = sizeof(uint32_t);"
replace "\(.*\)parameters\[\\([0-9]*\\)\]\.buffer_type = MYSQL_TYPE_FLOAT;" "&\n\\1parameters[\\2].buffer_length = sizeof(float_t);"
replace "\(.*\)parameters\[\\([0-9]*\\)\]\.buffer_type = MYSQL_TYPE_DOUBLE;" "&\n\\1parameters[\\2].buffer_length = sizeof(double_t);"
replace "parameters\[\\([0-9]*\\)\]\.buffer\ = (chr\_t \*)\\&[(]\?\\([A-Za-z]*\\)[)]\?\;" "parameters\[\\1]\.buffer = \\&\\2\;"

replace "lavalog(LOG_INFO, LOG_POP, " "log_pedantic("
replace "lavalog(LOG_INFO, LOG_META, " "log_pedantic("
replace "lavalog(LOG_INFO, LOG_MAIL, " "log_pedantic("
replace "lavalog(LOG_INFO, LOG_SMTP, " "log_pedantic("
replace "lavalog(LOG_INFO, LOG_IMAP, " "log_pedantic("
replace "lavalog(LOG_INFO, LOG_HTTP, " "log_pedantic("
replace "lavalog(LOG_INFO, LOG_COMMON, " "log_pedantic("
replace "lavalog(LOG_INFO, LOG_FRAMEWORK, " "log_pedantic("

replace "lavalog(LOG_DEBUG, LOG_POP, " "log_pedantic("
replace "lavalog(LOG_DEBUG, LOG_META, " "log_pedantic("
replace "lavalog(LOG_DEBUG, LOG_MAIL, " "log_pedantic("
replace "lavalog(LOG_DEBUG, LOG_SMTP, " "log_pedantic("
replace "lavalog(LOG_DEBUG, LOG_IMAP, " "log_pedantic("
replace "lavalog(LOG_DEBUG, LOG_HTTP, " "log_pedantic("
replace "lavalog(LOG_DEBUG, LOG_COMMON, " "log_pedantic("
replace "lavalog(LOG_DEBUG, LOG_FRAMEWORK, " "log_pedantic("

replace "lavalog(LOG_PRODUCTION, LOG_POP, " "log_error("
replace "lavalog(LOG_PRODUCTION, LOG_META, " "log_error("
replace "lavalog(LOG_PRODUCTION, LOG_MAIL, " "log_error("
replace "lavalog(LOG_PRODUCTION, LOG_SMTP, " "log_error("
replace "lavalog(LOG_PRODUCTION, LOG_IMAP, " "log_error("
replace "lavalog(LOG_PRODUCTION, LOG_HTTP, " "log_error("
replace "lavalog(LOG_PRODUCTION, LOG_COMMON, " "log_error("
replace "lavalog(LOG_PRODUCTION, LOG_FRAMEWORK, " "log_error("

replace "lavalog(LOG_PEDANTIC, LOG_POP, " "log_pedantic("
replace "lavalog(LOG_PEDANTIC, LOG_META, " "log_pedantic("
replace "lavalog(LOG_PEDANTIC, LOG_MAIL, " "log_pedantic("
replace "lavalog(LOG_PEDANTIC, LOG_SMTP, " "log_pedantic("
replace "lavalog(LOG_PEDANTIC, LOG_IMAP, " "log_pedantic("
replace "lavalog(LOG_PEDANTIC, LOG_HTTP, " "log_pedantic("
replace "lavalog(LOG_PEDANTIC, LOG_COMMON, " "log_pedantic("
replace "lavalog(LOG_PEDANTIC, LOG_FRAMEWORK, " "log_pedantic("

replace "sprintf_st(" "st_sprint("
replace "allocate_st(" "st_alloc("
replace "duplicate_st(" "st_dupe("
replace "free_st(" "st_free("
replace "set_used_st(" "st_length_set("
replace "used_st(" "st_length_get("
replace "size_st(" "st_avail_get("
replace "data_st(" "st_char_get("
replace "trim_st(" "st_trim("
replace "import_bl(" "st_import("
replace "import_ns(\\(.*\\))" "st_import(\\1, ns_length_get(\\1))"
replace "merge_strings(" "st_merge("


replace "identical_st_st(\\([A-Za-z0-9\_\.\>\-]\+\\), \\([A-Za-z0-9\_\.\>\-\+]\+\\)) != 1" "st_cmp_cs_eq(\\1, \\2)"
replace "identical_st_st(\\([A-Za-z0-9\_\.\>\-]\+\\), \\([A-Za-z0-9\_\.\>\-\+]\+\\)) == 1" "!st_cmp_cs_eq(\\1, \\2)"
replace "identical_st_ns(\\(.*\\), \\(.*\\)) == 1" "!st_cmp_cs_eq(\\1, PLACER(\\2, xx00xx))"
replace "identical_st_ns_case(\\(.*\\), \\(.*\\)) == 1" "!st_cmp_ci_eq(\\1, PLACER(\\2, xx00xx))"
replace "ends_ns_ns(\\([\"A-Za-z0-9\_\.\>\-\+\(\)\/]+\\), \\([\"A-Za-z0-9\_\.\>\-\+\(\)\/]+\\), \\([\"A-Za-z0-9\_\.\>\-\+\(\)\/]+\\), \\([\"A-Za-z0-9\_\.\>\-\+\(\)\/]+\\)) != 1" "st_cmp_cs_ends(PLACER(\\1, \\2), PLACER(\\3, \\4))"
replace "ends_ns_ns(\\([\"A-Za-z0-9\_\.\>\-\+\(\)\/]+\\), \\([\"A-Za-z0-9\_\.\>\-\+\(\)\/]+\\), \\([\"A-Za-z0-9\_\.\>\-\+\(\)\/]+\\), \\([\"A-Za-z0-9\_\.\>\-\+\(\)\/]+\\)) == 1" "!st_cmp_cs_ends(PLACER(\\1, \\2), PLACER(\\3, \\4))"

replace "ends_ns_ns_case(\\(.*\\), \\(.*\\), \\(.*\\), \\(.*\\)) == 1" "\!st_cmp_ci_ends(PLACER(\\1, \\2) PLACER(\\3, \\4))"
replace "ends_ns_ns_case(\\(.*\\), \\(.*\\), \\(.*\\), \\(.*\\)) != 1" "st_cmp_ci_ends(PLACER(\\1, \\2) PLACER(\\3, \\4))"

replace "set_pl(" "pl_init("
replace "data_pl(" "pl_data_get("
replace "size_pl(" "pl_length_get("

replace "allocate_bl(" "mm_alloc("
replace "free_bl(" "mm_free("
replace "clear_bl(" "mm_wipe("
replace "move_bytes(" "mm_move("
replace "copy_ns_ns_amt(" "mm_copy("
replace "duplicate_bl(" "mm_dupe("

replace "free_ns(" "ns_free("
replace "allocate_ns(" "ns_alloc("
replace "size_ns(" "ns_length_get("
replace "duplicate_ns(" "ns_dupe("

replace "lowercase_c(" "lower_chr("
replace "lowercase_st(" "lower_st("
replace "uppercase_c(" "upper_chr("
replace "uppercase_st(" "upper_st("

replace "pthread_rwlock_destroy(" "rwlock_destroy("
replace "pthread_rwlock_init(" "rwlock_init("
replace "pthread_rwlock_rdlock(" "rwlock_lock_read("
replace "pthread_rwlock_wrlock(" "rwlock_lock_write("
replace "pthread_mutex_unlock(" "mutex_unlock("
replace "pthread_mutex_lock(" "mutex_lock("
replace "pthread_mutex_init(" "mutex_init("
replace "pthread_mutex_destroy(" "mutex_destroy("

replace "linked_list_t \\*" "inx_t \\*"
replace "ll_free(" "inx_free("
replace "ll_allocate(" "inx_alloc(M_INX_LINKED, "
replace "ll_nodes(" "inx_count("

replace "hl_free(" "inx_free("
replace "hl_lock(" "inx_lock("
replace "hl_unlock(" "inx_unlock("
replace "hl_allocate(" "inx_alloc(M_INX_HASHED, "

replace "get_ull_sql(" "res_field_uint64("
replace "get_ul_sql(" "res_field_uint64("
replace "get_char_sql(" "res_field_block("
replace "get_len_sql(" "res_field_length("
replace "get_ui_sql(" "res_field_uint32("
replace "get_st_sql(" "res_field_string("
replace "get_f_sql(" "res_field_float("
replace "get_d_sql(" "res_field_double("
replace "get_ll_sql(" "res_field_int64("
replace "get_l_sql(" "res_field_int64("
replace "get_i_sql(" "res_field_int32("
replace "get_uc_sql(" "res_field_uint8("
replace "get_c_sql(" "res_field_int8("

replace "free_sql(" "res_table_free("
replace "fetch_row_sql(" "res_row_next("
replace "exec_query_res_stmt(" "stmt_get_result("
replace "exec_insert_stmt_tran(" "stmt_insert_conn("
replace "exec_insert_stmt(" "stmt_insert("
replace "exec_write_stmt_tran(" "stmt_exec_affected_conn("
replace "exec_write_stmt(" "stmt_exec_affected("
replace "exec_query_stmt_tran(" "stmt_exec_conn("
replace "exec_query_stmt(" "stmt_exec("
replace "rollback_tran(" "tran_rollback("
replace "start_tran(\([A-Za-z0-9]*\))" "int64_t = tran_start()"
replace "commit_tran([ ]\?\([A-Za-z0-9]\+\))" "tran_commit(\\1) != 0"

replace "time_to_midnight(" "time_till_midnight("
replace "time_date_as_num(" "time_datestamp("

replace "user->user_checkpoint" "user->serials.user"
replace "user->messages_checkpoint" "user->serials.messages"
replace "user->folders_checkpoint" "user->serials.folders"
replace "user->poprefs" "user->refs.pop"
replace "user->imaprefs" "user->refs.imap"
replace "user->smtprefs" "user->refs.smtp"

replace "meta_user_dec_ref(" "meta_user_ref_dec("
replace "meta_user_add_ref(" "meta_user_ref_add("

replace "cache_get_ns([ ]\?\([A-Za-z0-9]\+\),[ ]\?\([A-Za-z0-9]\+\)" "cache_get(PLACER(\\1, \\2)"
replace "cache_add_ns([ ]\?\([A-Za-z0-9]\+\),[ ]\?\([A-Za-z0-9]\+\),[ ]\?\([A-Za-z0-9]\+\),[ ]\?\([A-Za-z0-9]\+\),[ ]\?\([0-9]\+\))" "cache_add(PLACER(\\1, \\2), PLACER(\\3, \\4), \\5)"

replace "meta_cache_get_checkpoint(\\([A-Za-z0-9_.>+\-]*\\)\, META_CHECKPOINT_USER)" "serial_get(OBJECT_USER, \\1)"
replace "meta_cache_get_checkpoint(\\([A-Za-z0-9_.>+\-]*\\)\, META_CHECKPOINT_FOLDERS)" "serial_get(OBJECT_FOLDERS, \\1)"
replace "meta_cache_get_checkpoint(\\([A-Za-z0-9_.>+\-]*\\)\, META_CHECKPOINT_MESSAGES)" "serial_get(OBJECT_MESSAGES, \\1)"
replace "meta_cache_set_checkpoint(\\([A-Za-z0-9_.>+\-]*\\)\, META_CHECKPOINT_USER)" "serial_increment(OBJECT_USER, \\1)"
replace "meta_cache_set_checkpoint(\\([A-Za-z0-9_.>+\-]*\\)\, META_CHECKPOINT_FOLDERS)" "serial_increment(OBJECT_FOLDERS, \\1)"
replace "meta_cache_set_checkpoint(\\([A-Za-z0-9_.>+\-]*\\)\, META_CHECKPOINT_MESSAGES)" "serial_increment(OBJECT_MESSAGES, \\1)"

replace "mail_header_fetch_all(\([A-Za-z0-9_ ]\+\),[ ]\?\(\".*\"\),[ ]\?\([0-9]\+\))" "mail_header_fetch_all(\\1, PLACER(\\2, \\3))"
replace "mail_header_fetch_cleaned(\([A-Za-z0-9_ ]\+\),[ ]\?\(\".*\"\),[ ]\?\([0-9]\+\))" "mail_header_fetch_cleaned(\\1, PLACER(\\2, \\3))"

replace "\([^_]\)message_path(" "\1mail_message_path("
replace "\([^_]\)create_directory(" "\1mail_create_directory("
replace "\([^_]\)path_finder(" "\1mail_path_finder("

replace "config\.pages" "content.fixed"
replace "config\.templates" "content.templates"
replace "config\.mail\_storage\_root" "magma.storage.root"
replace "config\.mail\_storage\_server" "magma.storage.active"
replace "config\.adcloud\.cache" "magma.advertising.cache"
replace "config\.admanage\.cache" "magma.advertising.cache"
replace "config\.bidvertiser\.cache" "magma.advertising.cache"
replace "config\.adcloud\.pumps" "magma.advertising.worker_threads"
replace "config\.admanage\.pumps" "magma.advertising.worker_threads"
replace "config\.bidvertiser\.pumps" "magma.advertising.worker_threads"

replace "length_ull(" "uint64_digits("
replace "length_ul(" "uint64_digits("
replace "length_ui(" "uint32_digits("
replace "length_us(" "uint16_digits("
replace "length_ll(" "int64_digits("
replace "length_l(" "int64_digits("
replace "length_i(" "int32_digits("
replace "length_s(" "int16_digits("

replace "isalnum(" "chr_alphanumeric("
replace "isascii(" "chr_ascii("
replace "isblank(" "chr_blank("
replace "islower(" "chr_lower("
replace "isdigit(" "chr_numeric("
replace "isprint(" "chr_printable("
replace "isupper(" "chr_upper("
replace "isspace(" "chr_whitespace("
replace "ispunct(" "chr_punctuation("

replace "stacked_list_t " "stacker_t "
replace "sa_free(" "stacker_free("
replace "sa_pop(" "stacker_pop("
replace "sa_nodes(" "stacker_nodes("
replace "sa_push(" "stacker_push("
replace "sa_allocate(" "stacker_alloc("

# Ensure the array function isn't one of the IMAP array siblings by checking a underscore at the front.

replace "\\([^_]\\)free_ar(" "\\1ar_free("
replace "\\([^_]\\)used_ar(" "\\1ar_length_get("
replace "\\([^_]\\)append_ar(" "\\1ar_append("
replace "\\([^_]\\)get_st_ar(" "\\1ar_field_st("
replace "\\([^_]\\)get_ptr_ar(" "\\1ar_field_ptr("
replace "\\([^_]\\)get_type_ar(" "\\1ar_field_type("
replace "\\([^_]\\)get_ar_ar(" "\\1ar_field_ar("

replace "[^\_]rand()" "rand_get_uint[8\/16\/32\/64]()"
replace "random_st_choices(\([0-9]\+\),[ ]\?\(.\+\),[ ]\?[0-9]\+);" "rand_choices(\\2, \\1);"

replace "web_encode(" "url_encode("
replace "url_encode_st(" "url_encode("
replace "hex_char_encode(" "hex_encode_chr("
replace "url_encode_ns(\\(.*\\));" "url_encode(NULLER(\\1));"

replace "xml_get_xpath_ull(" "xml_get_xpath_uint64"
replace "xml_get_xpath_ul(" "xml_get_xpath_uint64" 
replace "xml_xpath_eval(\\\"" "xml_xpath_eval((uchr_t *)\""
replace "xml_node_new(\\\"" "xml_node_new((uchr_t *)\\\""
replace "xml_node_set_content(node, \\\"" "xml_node_set_content(node, (uchr_t *)\""
replace "xml_node_set_property(node, \\\"class\\\", \\\"" "xml_node_set_property(node, (uchr_t *)\"class\", (uchr_t *)\""
 
replace "word_list_get_random(" "advert_word_rand("
replace "agent_list_get_random(" "advert_agent_rand("
replace "word_list_stat(" "advert_word_update("
replace "stats_adjust(\"\\([a-z]*\\)ErrorsForeign" "stats_adjust_by_name(\"advertising.\\1.foreign.errors"
replace "stats_adjust(\"\\([a-z]*\\)ErrorsDomestic" "stats_adjust_by_name(\"advertising.\\1.domestic.errors"
replace "stats_adjust(\"\\([a-z]*\\)ExpiredForeign" "stats_adjust_by_name(\"advertising.\\1.foreign.expired"
replace "stats_adjust(\"\\([a-z]*\\)ExpiredDomestic" "stats_adjust_by_name(\"advertising.\\1.domestic.expired"
replace "stats_adjust(\"\\([a-z]*\\)ImpressionsForeign" "stats_adjust_by_name(\"advertising.\\1.foreign.impressions"
replace "stats_adjust(\"\\([a-z]*\\)ImpressionsDomestic" "stats_adjust_by_name(\"advertising.\\1.domestic.impressions"
replace "\\([a-z]*\\)ErrorsForeign" "advertising.\\1.foreign.errors"
replace "\\([a-z]*\\)ErrorsDomestic" "advertising.\\1.domestic.errors"
replace "\\([a-z]*\\)ExpiredForeign" "advertising.\\1.foreign.expired"
replace "\\([a-z]*\\)ExpiredDomestic" "advertising.\\1.domestic.expired"
replace "\\([a-z]*\\)ImpressionsForeign" "advertising.\\1.foreign.impressions"
replace "\\([a-z]*\\)ImpressionsDomestic" "advertising.\\1.domestic.impressions"

replace "continue_processing(0)" "status()"
replace "continue_processing(-1);" "status_set(-1);"
replace "status()\\([ ]\?\\)==\\([ ]\?\\)1" "status()"

replace "po_get(" "pool_get_obj("
replace "po_set(" "pool_set_obj("
replace "po_free(" "pool_free("
replace "po_return(" "pool_release("

replace "get_account_lock(" "user_lock("
replace "release_account_lock(" "user_unlock("

replace "meta_cache_set_checkpoint([ ]\?\(.\+\), META_CHECKPOINT_MESSAGES)\;" "serial_increment(OBJECT_MESSAGES, \\1)\;"

# Network/client functions
replace "network_write_ns([ ]\?\(.\+\),[ ]\?\(.\+\),[ ]\?\(.\+\))\;" "client_write(client, PLACER(\\2, \\3))\;"
replace "network_write([ ]\?\(.\+\),[ ]\?\(.\+\))" "client_write(client, \\2)"
replace "network_printf(client->sock_descriptor, client->out_buffer, config.out_buffer," "client_print(client,"
replace "network_readline(sock_descriptor, client->in_buffer, config.in_buffer, &(client->line_length), &(client->buffered_bytes));" "client_read_line(client);"
replace "network_readline(client->sock_descriptor, client->in_buffer, config.in_buffer, &(client->line_length), &(client->buffered_bytes))" "client_read_line(client)" 

# These are pretty scary rules and need change based on protocol
replace "session_read((session_common_t \*)session)" "con_read(con)"
replace "session_readline((session_common_t \*)session)" "con_read_line(con)"
replace "session_printf((session_common_t \*)session," "con_print(con,"
replace "session_write_ns((session_common_t \*)session," "con_write_bl(con,"
replace "session_write((session_common_t \*)session," "con_write_st(con,"
 
 
# HTTP Session updates
if [[ "$DIR" =~ "servers/http" ]] || [[ "$DIR" =~ "web/" ]]; then

	replace "http_session_t \*session" "connection_t \*con"
	replace "http_session_t \*http" "connection_t \*con"
	replace "session->" "con->http."
	replace "http->" "con->http."
	replace "\\([^\_A-Za-z0-9]\\)session\\([^\_A-Za-z0-9]\\)" "\\1con\\2"

fi

# IMAP Session updates
if [[ "$DIR" =~ "servers/imap" ]]; then
	replace "imap_session_t \*session" "connection_t \*con"
	replace "imap_session_t \*pop" "connection_t \*con"
	replace "imap_update\_points(.*);" ""
	replace "session->" "con->imap."
	replace "imap->" "con->imap."
fi
 
# POP Session updates
if [[ "$DIR" =~ "servers/pop" ]]; then

	replace "pop_session_t \*session" "connection_t \*con"
	replace "pop_session_t \*pop" "connection_t \*con"
	replace "pop_update\_points(.*);" ""
	replace "session->" "con->pop."
	replace "pop->" "con->pop."

fi


# SMTP Session updates 
if [[ "$DIR" =~ "servers/smtp" ]]; then
		
	replace "session->spf_checked" "con->smtp.checked.spf"
	replace "session->rbl_checked" "con->smtp.checked.rbl"
	replace "session->dkim_checked" "con->smtp.checked.dkim"
	replace "session->virus_checked" "con->smtp.checked.virus"
	replace "session->address" "con->smtp.ip_address_v4"
	
	replace "session->max_size" "con->smtp.max_length"
	replace "session->proposed_size" "con->smtp.suggested_length"
	replace "session->num_recipients" "con->smtp.num_recipients"
	replace "session->out_prefs" "con->smtp.out_prefs"
	replace "session->in_prefs" "con->smtp.in_prefs"
	replace "session->message" "con->smtp.message"
	replace "session->helo" "con->smtp.helo"
	replace "session->mailfrom" "con->smtp.mailfrom"
	replace "session->authorized == 0" "con->smtp.authenticated == false"
	replace "session->authorized == 1" "con->smtp.authenticated == true"
	replace "session->authorized" "con->smtp.authenticated"
	replace "session->esmtp == 0" "con->smtp.esmtp == false"
	replace "session->esmtp == 1" "con->smtp.esmtp == true"
	replace "session->esmtp" "con->smtp.esmtp"
	
	replace "smtp_session_t \*session" "connection_t \*con"
	replace "smtp_session_t \*smtp" "connection_t \*con"
	replace "smtp_update\_points(.*);" ""
	replace "session\-\>" "con\-\>smtp."
	replace "smtp->" "con->smtp\."

fi

replace "inline " ""
replace "\(\/\/\ [A-Za-z]*\)\." "\\1"
replace "free_rt(" "compress_free("
replace "count_token_st_c(" "tok_get_count_st("
replace "\s\?__attribute__\s\?((packed))\s\?" " __attribute__ ((packed)) "

# If the output supports color this format is a little nicer...
# /usr/bin/dwdiff --no-deleted --color --context 3 "$FILE.X" "$FILE"

# /usr/bin/diff --side-by-side --suppress-common-lines --expand-tabs --width=180 "$FILE.X" "$FILE"
(/usr/bin/gnome-terminal --hide-menubar --full-screen -e \
"/bin/bash -c '/usr/bin/dwdiff --ignore-formatting --no-inserted --color --line-numbers \
--context 1 \"$FILE\" \"$FILE.X\" 2>&1 | /bin/sed -e \"s/^\([0-9 ]\{4\}\):[0-9]\+/\\1/\"; \
/usr/bin/dwdiff --ignore-formatting --statistics -1 -2 -3 \"$FILE\" \"$FILE.X\" 2>&1 | grep -v old | sed \"s/new\:/\n\n/g\"; printf \"\n\n\" \
; /bin/rm \"$FILE.X\"; read -n 1 -t 120'") &

touch $FILE




