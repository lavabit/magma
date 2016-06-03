
/**
 * @file /magma/queries.h
 *
 * @brief	Assorted SQL queries used throughout Magma.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_DATA_QUERIES_H
#define MAGMA_DATA_QUERIES_H

// Domains table
#define SELECT_DOMAINS "SELECT domain, restricted, mailboxes, wildcard, dkim, spf FROM Domains"

// Config table
// The ORDER BY clause is used to ensure the host specific values come last, overwriting any cluster defaults.
#define SELECT_CONFIG "SELECT name, value FROM Host_Config LEFT JOIN Hosts ON (Host_Config.hostnum = Hosts.hostnum) WHERE application = 'magmad' AND (Host_Config.hostnum IS NULL OR Hosts.hostname = ?) ORDER BY Host_Config.hostnum ASC"
#define SELECT_HOST_NUMBER "SELECT hostnum FROM Hosts WHERE hostname = ?"

// Objects table
#define DELETE_OBJECT "DELETE FROM Objects WHERE objectnum = ? AND hostnum = ? AND tank = ? AND usernum = ?"
#define INSERT_OBJECT "INSERT INTO Objects (usernum, hostnum, tank, size, serial, flags, `references`, timestamp) VALUES (?, ?, ?, ?, 0, ?, 0, NOW())"

// User table
#define SELECT_USER "SELECT Dispatch.secure, locked, Users.usernum, `ssl`, overquota FROM Users INNER JOIN Dispatch ON Users.usernum = Dispatch.usernum WHERE userid = ? AND legacy = ? AND email = 1"
#define SELECT_USER_AUTH "SELECT Dispatch.secure, locked, Users.usernum, `ssl`, overquota FROM Users INNER JOIN Dispatch ON Users.usernum = Dispatch.usernum WHERE userid = ? AND auth = ? AND email = 1	LIMIT 1"
#define SELECT_USERNUM_AUTH_LEGACY "SELECT usernum FROM Users WHERE userid = ? AND legacy = ? AND email = 1"
#define SELECT_USERNUM_AUTH "SELECT usernum FROM Users WHERE userid = ? AND auth = ? AND email = 1"
#define SELECT_USER_RECORD "SELECT legacy, Dispatch.secure, locked, `ssl`, overquota FROM Users INNER JOIN Dispatch ON Users.usernum = Dispatch.usernum WHERE Users.usernum = ? AND email = 1"
#define SELECT_USER_SALT "SELECT salt FROM Users WHERE userid = ?"
#define SELECT_USER_STORAGE_KEYS "SELECT storage_pub, storage_priv FROM `Keys` WHERE usernum = ?"
#define UPDATE_USER_STORAGE_KEYS "INSERT INTO `Keys` (usernum, storage_pub, storage_priv) VALUES (?, ?, ?) ON DUPLICATE KEY UPDATE storage_pub = ?, storage_priv = ?"
#define UPDATE_USER_LOCK "UPDATE Users SET locked = ? WHERE usernum = ?"
#define UPDATE_USER_QUOTA_ADD "UPDATE Users SET size = size + ?, overquota = IF(size < quota, 0, 1) WHERE usernum = ?"
#define UPDATE_USER_QUOTA_SUBTRACT "UPDATE Users SET size = size - ?, overquota = IF(size < quota, 0, 1) WHERE usernum = ?"

// Mailbox Aliases
#define SELECT_MAILBOX_ALIASES "SELECT Aliases.aliasnum, Mailboxes.address, Aliases.display, Aliases.selected, UNIX_TIMESTAMP(Aliases.created) from Mailboxes " \
	"LEFT JOIN Aliases ON Mailboxes.address = Aliases.address AND Mailboxes.usernum = Aliases.usernum " \
	"WHERE Mailboxes.usernum = ? ORDER BY selected DESC, LOWER(Aliases.display) ASC, LOWER(Mailboxes.address) ASC"

// Alerts table
#define SELECT_ALERTS "SELECT alertnum, type, message, UNIX_TIMESTAMP(created) FROM Alerts WHERE usernum = ? AND acknowledged IS NULL"
#define UPDATE_ALERTS_ACKNOWLEDGE "UPDATE Alerts SET acknowledged = NOW() WHERE alertnum = ? AND usernum = ? AND acknowledged IS NULL"

// Log table
#define UPDATE_LOG_POP "UPDATE Log SET lastpop = NOW(), popsessions = popsessions + 1 WHERE usernum = ?"
#define UPDATE_LOG_IMAP "UPDATE Log SET lastmap = NOW(), mapsessions = mapsessions + 1 WHERE usernum = ?"
#define UPDATE_LOG_WEB "UPDATE Log SET lastweb = NOW(), websessions = websessions + 1 WHERE usernum = ?"

// Folder table
#define SELECT_FOLDERS "SELECT foldernum, parent, `order`, foldername FROM Folders WHERE usernum = ? AND type = ?"
#define INSERT_FOLDER "INSERT INTO Folders (usernum, foldername, `order`, parent, type) VALUES (?, ?, ?, ?, ?)"
#define DELETE_FOLDER "DELETE FROM Folders WHERE foldernum = ? AND usernum = ? AND type = ?"
#define UPDATE_FOLDER "UPDATE Folders SET foldername = ?, parent = ?, `order` = ? WHERE foldernum = ? AND usernum = ? AND type = ?"
#define RENAME_FOLDER "UPDATE Folders SET foldername = ? WHERE foldernum = ? AND usernum = ? AND type = ?"

// Messages table
#define SELECT_MESSAGES "SELECT messagenum, foldernum, server, status, size, signum, sigkey, UNIX_TIMESTAMP(created) FROM Messages WHERE usernum = ? AND visible = 1 ORDER BY messagenum ASC"
#define UPDATE_MESSAGE_VISIBILITY "UPDATE Messages SET visible = 0 WHERE messagenum = ?"
#define UPDATE_MESSAGE_FLAGS_ADD "UPDATE Messages SET status = (status | ?) WHERE usernum = ? AND foldernum = ? AND messagenum = ?"
#define UPDATE_MESSAGE_FLAGS_REMOVE "UPDATE Messages SET status = ((status | ?) ^ ?) WHERE usernum = ? AND foldernum = ? AND messagenum = ?"
#define UPDATE_MESSAGE_FLAGS_REPLACE  "UPDATE Messages SET status = (((status | ?) ^ ?) | ?) WHERE usernum = ? AND foldernum = ? AND messagenum = ?"
#define UPDATE_MESSAGE_FOLDER "UPDATE Messages SET foldernum = ? WHERE messagenum = ? AND usernum = ? AND foldernum = ?"
#define INSERT_MESSAGE "INSERT INTO Messages (usernum, foldernum, server, status, size, signum, sigkey, created) VALUES (?, ?, ?, ?, ?, ?, ?, NOW())"
#define INSERT_MESSAGE_DUPLICATE "INSERT INTO Messages (usernum, foldernum, server, status, size, signum, sigkey, created) VALUES (?, ?, ?, ?, ?, ?, ?, FROM_UNIXTIME(?))"
#define DELETE_MESSAGE "DELETE FROM Messages WHERE messagenum = ? AND usernum = ?"

// Message Tags table
#define SELECT_ALL_MESSAGE_TAGS "SELECT DISTINCT tag from Message_Tags LEFT JOIN Messages ON Message_Tags.messagenum = Messages.messagenum"
#define DELETE_MESSAGE_TAGS "DELETE FROM Message_Tags WHERE messagenum = ?"
#define SELECT_MESSAGE_TAGS "SELECT tag FROM Message_Tags WHERE messagenum = ?"
#define INSERT_MESSAGE_TAG "INSERT INTO Message_Tags (messagenum, tag) VALUES (?, ?)"
#define DELETE_MESSAGE_TAG "DELETE FROM Message_Tags WHERE messagenum = ? AND tag = ?"

// Advertising queries
#define SELECT_AGENTS "SELECT agentnum, agent, popularity FROM Agents"

// SMTP queries
#define SELECT_MAILBOX_ADDRESS "SELECT usernum FROM Mailboxes WHERE address = ? AND usernum = ?"
#define SELECT_MAILBOX_ADDRESS_ANY "SELECT * FROM Mailboxes WHERE address = ?"
#define SELECT_AUTOREPLY "SELECT message FROM Autoreplies WHERE replynum = ? AND usernum = ?"
#define SELECT_PATTERNS "SELECT pattern FROM Patterns"
#define SELECT_FILTERS "SELECT rulenum, location, type, action,	foldernum, field, label, expression FROM Filters WHERE usernum = ? ORDER BY rulenum ASC"
#define SELECT_MESSAGES_ROLLOUT "SELECT messagenum, size, server FROM Messages WHERE usernum = ? ORDER BY created ASC LIMIT 20"
#define SELECT_TRANSMITTING  "SELECT COUNT(*) FROM Transmitting WHERE usernum = ? AND timestamp >= DATE_SUB(NOW(), INTERVAL 1 DAY)"
#define SELECT_RECEIVING "SELECT COUNT(*), SUM(subnet = ?) FROM Receiving WHERE usernum = ? AND timestamp >= DATE_SUB(NOW(), INTERVAL 1 DAY)"
#define SELECT_USERS_AUTH "SELECT Users.usernum, Users.locked, Users.`ssl`, Users.domain, Dispatch.send_size_limit, Dispatch.daily_send_limit, Dispatch.class FROM Users LEFT JOIN Dispatch ON Users.usernum = Dispatch.usernum WHERE userid = ? AND legacy = ? AND email = 1"
#define SMTP_SELECT_USER_AUTH "SELECT Users.usernum, Users.locked, Users.`ssl`, Users.domain, Dispatch.send_size_limit, Dispatch.daily_send_limit, Dispatch.class FROM Users LEFT JOIN Dispatch ON Users.usernum = Dispatch.usernum WHERE userid = ? AND auth = ? AND email = 1"
#define SELECT_PREFS_INBOUND "SELECT Mailboxes.usernum, Users.locked, Users.size, Users.quota, Users.overquota, " \
		"Users.domain, Dispatch.secure, Dispatch.bounces, Dispatch.forwarded, Dispatch.rollout, " \
		"Dispatch.spam, Dispatch.spamaction, Dispatch.virus, Dispatch.virusaction, Dispatch.phish, Dispatch.phishaction, " \
		"Dispatch.autoreply, Dispatch.inbox, Dispatch.recv_size_limit, Dispatch.daily_recv_limit, Dispatch.daily_recv_limit_ip, " \
		"Dispatch.greylist, Dispatch.greytime, Dispatch.spf, Dispatch.spfaction, Dispatch.dkim, Dispatch.dkimaction, Dispatch.rbl, " \
		"Dispatch.rblaction, Dispatch.filters FROM Mailboxes LEFT JOIN Users ON Mailboxes.usernum = Users.usernum LEFT JOIN Dispatch ON " \
		"Mailboxes.usernum = Dispatch.usernum WHERE Mailboxes.address = ?"
#define INSERT_TRANSMITTING "INSERT INTO Transmitting (usernum, timestamp) VALUES (?, NOW())"
#define INSERT_SIGNATURE "INSERT INTO Signatures (usernum, cryptkey, junk, signature, created) VALUES (?, ?, ?, ?, NOW())"
#define INSERT_RECEIVING "REPLACE INTO Receiving (usernum, subnet, timestamp) VALUES (?, ?, NOW())"
#define UPDATE_LOG_SENT "UPDATE Log SET lastsent = NOW(), totalsent = totalsent + ? WHERE usernum = ?"
#define UPDATE_LOG_RECEIVED "UPDATE Log SET lastreceived = NOW(), totalreceived = totalreceived + ?, totalbounces = totalbounces + ? WHERE usernum = ?"

// Contacts
#define SELECT_CONTACTS "SELECT `contactnum`, `name` FROM `Contacts` WHERE `usernum` = ? AND `foldernum` = ?"
#define INSERT_CONTACT "INSERT INTO `Contacts` (`contactnum`, `usernum`, `foldernum`, `name`, `updated`, `created`) VALUES (NULL, ?, ?, ?, NOW(), NOW())"
#define UPDATE_CONTACT "UPDATE `Contacts` SET `foldernum` = IFNULL(?, `foldernum`), `name` = IFNULL(?, `name`), `updated` = NOW() WHERE `contactnum` = ? AND `usernum` = ? AND `foldernum` = ?"
#define UPDATE_CONTACT_STAMP "UPDATE `Contacts` SET `updated` = NOW() WHERE `contactnum` = ? AND `usernum` = ? AND `foldernum` = ?"
#define DELETE_CONTACT "DELETE `Contacts`, `Contact_Details` FROM `Contacts` LEFT JOIN `Contact_Details` ON `Contacts`.`contactnum` = `Contact_Details`.`contactnum` WHERE `Contacts`.`contactnum` = ? AND `Contacts`.`usernum` = ? AND `Contacts`.`foldernum` = ?"

// Contact Details
#define UPSERT_CONTACT_DETAIL "INSERT INTO `Contact_Details` (`contactnum`, `key`, `value`, `flags`) VALUES (?, ?, ?, 0) ON DUPLICATE KEY UPDATE `value` = VALUES(`value`), `flags` = VALUES(`flags`)"
#define SELECT_CONTACT_DETAILS "SELECT `key`, `value`, `flags` FROM `Contact_Details` WHERE `contactnum` = ?"
#define DELETE_CONTACT_DETAILS "DELETE FROM `Contact_Details` WHERE `contactnum` = ? AND `key` = ?"

// Message Folders
#define SELECT_MESSAGE_FOLDER "SELECT messagenum, UNIX_TIMESTAMP(created), signum, sigkey, status, server, size FROM Messages WHERE usernum = ? AND foldernum = ? AND visible = 1 ORDER BY messagenum ASC"

// User Config
#define UPSERT_USER_CONFIG "INSERT INTO `User_Config` (`usernum`, `key`, `value`, `flags`, `timestamp`) VALUES (?, ?, ?, ?, NOW()) ON DUPLICATE KEY UPDATE `value` = VALUES(`value`), `flags` = VALUES(`flags`)"
#define SELECT_USER_CONFIG "SELECT `key`, `value`, `flags` FROM `User_Config` WHERE `usernum` = ?"
#define DELETE_USER_CONFIG "DELETE FROM `User_Config` WHERE `usernum` = ? AND `key` = ?"

// For handling spam signatures.
#define FETCH_SIGNATURE "SELECT Users.userid, Users.legacy, Users.usernum, Signatures.junk, Signatures.cryptkey, Signatures.signature FROM Signatures LEFT JOIN Users ON (Signatures.usernum = Users.usernum) WHERE Signatures.signum = ?"
#define UPDATE_SIGNATURE_FLAGS_ADD "UPDATE Messages SET status = (status | ?) WHERE usernum = ? AND signum = ?"
#define UPDATE_SIGNATURE_FLAGS_REMOVE "UPDATE Messages SET status = ((status | ?) ^ ?) WHERE usernum = ? AND signum = ?"
#define DELETE_SIGNATURE "DELETE FROM Signatures WHERE signum = ?"

// For the portal/user management.
#define REGISTER_CHECK_USERNAME	"SELECT usernum FROM Users WHERE userid = ?"
#define REGISTER_INSERT_USER "INSERT INTO Users (`userid`, `legacy`, `plan`, `quota`, `plan_expiration`) VALUES (?, ?, ?, ?, ?)"
#define REGISTER_INSERT_STACIE_USER "INSERT INTO Users (`userid`, `salt`, `auth`, `bonus`, `plan`, `quota`, `plan_expiration`) VALUES (?, ?, ?, ?, ?, ?, ?)"
#define REGISTER_INSERT_PROFILE "INSERT INTO Profile (`usernum`) VALUES (?)"
#define REGISTER_INSERT_FOLDERS "INSERT INTO Folders (`usernum`) VALUES (?)"
#define REGISTER_INSERT_FOLDER_NAME "INSERT INTO Folders (`usernum`, `foldername`) VALUES (?, ?)"
#define REGISTER_INSERT_LOG "INSERT INTO Log (`usernum`, `created_ip`) VALUES (?, ?)"
#define REGISTER_INSERT_DISPATCH "INSERT INTO Dispatch (`usernum`, `spamfolder`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"
#define REGISTER_INSERT_MAILBOXES "INSERT INTO Mailboxes (`address`, `usernum`) VALUES (?, ?)"
#define REGISTER_FETCH_BLOCKLIST "SELECT sequence FROM Banned"
#define DELETE_USER "DELETE FROM Users WHERE userid = ?"

// For handling the portal statistics app
#define STATISTICS_GET_TOTAL_USERS "SELECT COUNT(*) FROM Users"
#define STATISTICS_GET_USERS_CHECKED_EMAIL_TODAY "SELECT COUNT(*) FROM Log WHERE lastpop >= DATE_SUB(NOW(), INTERVAL 1 DAY) OR lastweb >= DATE_SUB(NOW(), INTERVAL 1 DAY)"
#define STATISTICS_GET_USERS_CHECKED_EMAIL_WEEK "SELECT COUNT(*) FROM Log WHERE lastpop >= DATE_SUB(NOW(), INTERVAL 7 DAY) OR lastweb >= DATE_SUB(NOW(), INTERVAL 7 DAY)"
#define STATISTICS_GET_USERS_SENT_EMAIL_TODAY "SELECT COUNT(*) FROM Log WHERE lastsent >= DATE_SUB(NOW(), INTERVAL 1 DAY)"
#define STATISTICS_GET_USERS_SENT_EMAIL_WEEK "SELECT COUNT(*) FROM Log WHERE lastsent >= DATE_SUB(NOW(), INTERVAL 7 DAY)"
#define STATISTICS_GET_EMAILS_RECEIVED_TODAY "SELECT COUNT(*) FROM Receiving WHERE timestamp >= DATE_SUB(NOW(), INTERVAL 1 DAY)"
#define STATISTICS_GET_EMAILS_RECEIVED_WEEK "SELECT COUNT(*) FROM Receiving WHERE timestamp >= DATE_SUB(NOW(), INTERVAL 7 DAY)"
#define STATISTICS_GET_EMAILS_SENT_TODAY "SELECT COUNT(*) FROM Transmitting WHERE timestamp >= DATE_SUB(NOW(), INTERVAL 1 DAY)"
#define STATISTICS_GET_EMAILS_SENT_WEEK "SELECT COUNT(*) FROM Transmitting WHERE timestamp >= DATE_SUB(NOW(), INTERVAL 7 DAY)"
#define STATISTICS_GET_USERS_REGISTERED_TODAY "SELECT COUNT(*) FROM Creation WHERE timestamp >= DATE_SUB(NOW(), INTERVAL 1 DAY)"
#define STATISTICS_GET_USERS_REGISTERED_WEEK "SELECT COUNT(*) FROM Creation WHERE timestamp >= DATE_SUB(NOW(), INTERVAL 7 DAY)"

// For handling usernames and authentication.
#define AUTH_GET_BY_USERID "SELECT usernum, userid, salt, auth, bonus, legacy, `ssl`, locked FROM Users WHERE userid = ?"
#define AUTH_GET_BY_ADDRESS "SELECT Users.usernum, userid, salt, auth, bonus, legacy, `ssl`, locked FROM Users LEFT JOIN Mailboxes ON (Users.usernum = Mailboxes.usernum) WHERE Mailboxes.address = ?"
#define AUTH_UPDATE_LEGACY_TO_STACIE "UPDATE Users SET salt = ?, auth = ?, bonus = ?, legacy = NULL WHERE usernum = ? AND legacy = ?"

/*

 Queries + Stmts Init
 cat queries.h | grep "\#define" | egrep -v "MAGMA_DATA_QUERIES_H|INIT" | grep -v "//" | awk -F' ' '{ print $2 }' | egrep "^[A-Z_]+$" | awk -F' ' '{ print "\t\t\t\t\t\t\t\t\t\t\t" $1 ", \\" }'; \
 cat queries.h | grep "\#define" | egrep -v "MAGMA_DATA_QUERIES_H|INIT" | grep -v "//" | awk -F' ' '{ print $2 }' | egrep "^[A-Z_]+$" | awk -F' ' '{ print "\t\t\t\t\t\t\t\t\t\t\t**" tolower($1) ", \\" }'

*/

/**
 * @note Be sure to add any new queries to this list.
 */
#define QUERIES_INIT						SELECT_DOMAINS, \
											SELECT_CONFIG, \
											SELECT_HOST_NUMBER, \
											DELETE_OBJECT, \
											INSERT_OBJECT, \
											SELECT_USER, \
											SELECT_USER_AUTH, \
											SELECT_USERNUM_AUTH_LEGACY, \
											SELECT_USERNUM_AUTH, \
											SELECT_USER_RECORD, \
											SELECT_USER_SALT, \
											SELECT_USER_STORAGE_KEYS, \
											UPDATE_USER_STORAGE_KEYS, \
											UPDATE_USER_LOCK, \
											UPDATE_USER_QUOTA_ADD, \
											UPDATE_USER_QUOTA_SUBTRACT, \
											SELECT_MAILBOX_ALIASES, \
											SELECT_ALERTS, \
											UPDATE_ALERTS_ACKNOWLEDGE, \
											UPDATE_LOG_POP, \
											UPDATE_LOG_IMAP, \
											UPDATE_LOG_WEB, \
											SELECT_FOLDERS, \
											INSERT_FOLDER, \
											DELETE_FOLDER, \
											UPDATE_FOLDER, \
											RENAME_FOLDER, \
											SELECT_MESSAGES, \
											UPDATE_MESSAGE_VISIBILITY, \
											UPDATE_MESSAGE_FLAGS_ADD, \
											UPDATE_MESSAGE_FLAGS_REMOVE, \
											UPDATE_MESSAGE_FLAGS_REPLACE, \
											UPDATE_MESSAGE_FOLDER, \
											INSERT_MESSAGE, \
											INSERT_MESSAGE_DUPLICATE, \
											DELETE_MESSAGE, \
											SELECT_ALL_MESSAGE_TAGS, \
											DELETE_MESSAGE_TAGS, \
											SELECT_MESSAGE_TAGS, \
											INSERT_MESSAGE_TAG, \
											DELETE_MESSAGE_TAG, \
											SELECT_AGENTS, \
											SELECT_MAILBOX_ADDRESS, \
											SELECT_MAILBOX_ADDRESS_ANY, \
											SELECT_AUTOREPLY, \
											SELECT_PATTERNS, \
											SELECT_FILTERS, \
											SELECT_MESSAGES_ROLLOUT, \
											SELECT_TRANSMITTING, \
											SELECT_RECEIVING, \
											SELECT_USERS_AUTH, \
											SMTP_SELECT_USER_AUTH, \
											SELECT_PREFS_INBOUND, \
											INSERT_TRANSMITTING, \
											INSERT_SIGNATURE, \
											INSERT_RECEIVING, \
											UPDATE_LOG_SENT, \
											UPDATE_LOG_RECEIVED, \
											SELECT_CONTACTS, \
											INSERT_CONTACT, \
											UPDATE_CONTACT, \
											UPDATE_CONTACT_STAMP, \
											DELETE_CONTACT, \
											UPSERT_CONTACT_DETAIL, \
											SELECT_CONTACT_DETAILS, \
											DELETE_CONTACT_DETAILS, \
											SELECT_MESSAGE_FOLDER, \
											UPSERT_USER_CONFIG, \
											SELECT_USER_CONFIG, \
											DELETE_USER_CONFIG, \
											FETCH_SIGNATURE, \
											UPDATE_SIGNATURE_FLAGS_ADD, \
											UPDATE_SIGNATURE_FLAGS_REMOVE, \
											DELETE_SIGNATURE, \
											REGISTER_CHECK_USERNAME, \
											REGISTER_INSERT_USER, \
											REGISTER_INSERT_STACIE_USER, \
											REGISTER_INSERT_PROFILE, \
											REGISTER_INSERT_FOLDERS, \
											REGISTER_INSERT_FOLDER_NAME, \
											REGISTER_INSERT_LOG, \
											REGISTER_INSERT_DISPATCH, \
											REGISTER_INSERT_MAILBOXES, \
											REGISTER_FETCH_BLOCKLIST, \
											DELETE_USER, \
											STATISTICS_GET_TOTAL_USERS, \
											STATISTICS_GET_USERS_CHECKED_EMAIL_TODAY, \
											STATISTICS_GET_USERS_CHECKED_EMAIL_WEEK, \
											STATISTICS_GET_USERS_SENT_EMAIL_TODAY, \
											STATISTICS_GET_USERS_SENT_EMAIL_WEEK, \
											STATISTICS_GET_EMAILS_RECEIVED_TODAY, \
											STATISTICS_GET_EMAILS_RECEIVED_WEEK, \
											STATISTICS_GET_EMAILS_SENT_TODAY, \
											STATISTICS_GET_EMAILS_SENT_WEEK, \
											STATISTICS_GET_USERS_REGISTERED_TODAY, \
											STATISTICS_GET_USERS_REGISTERED_WEEK, \
											AUTH_GET_BY_USERID, \
											AUTH_GET_BY_ADDRESS, \
											AUTH_UPDATE_LEGACY_TO_STACIE

#define STMTS_INIT							**select_domains, \
											**select_config, \
											**select_host_number, \
											**delete_object, \
											**insert_object, \
											**select_user, \
											**select_user_auth, \
											**select_usernum_auth_legacy, \
											**select_usernum_auth, \
											**select_user_record, \
											**select_user_salt, \
											**select_user_storage_keys, \
											**update_user_storage_keys, \
											**update_user_lock, \
											**update_user_quota_add, \
											**update_user_quota_subtract, \
											**select_mailbox_aliases, \
											**select_alerts, \
											**update_alerts_acknowledge, \
											**update_log_pop, \
											**update_log_imap, \
											**update_log_web, \
											**select_folders, \
											**insert_folder, \
											**delete_folder, \
											**update_folder, \
											**rename_folder, \
											**select_messages, \
											**update_message_visibility, \
											**update_message_flags_add, \
											**update_message_flags_remove, \
											**update_message_flags_replace, \
											**update_message_folder, \
											**insert_message, \
											**insert_message_duplicate, \
											**delete_message, \
											**select_all_message_tags, \
											**delete_message_tags, \
											**select_message_tags, \
											**insert_message_tag, \
											**delete_message_tag, \
											**select_agents, \
											**select_mailbox_address, \
											**select_mailbox_address_any, \
											**select_autoreply, \
											**select_patterns, \
											**select_filters, \
											**select_messages_rollout, \
											**select_transmitting, \
											**select_receiving, \
											**select_users_auth, \
											**smtp_select_user_auth, \
											**select_prefs_inbound, \
											**insert_transmitting, \
											**insert_signature, \
											**insert_receiving, \
											**update_log_sent, \
											**update_log_received, \
											**select_contacts, \
											**insert_contact, \
											**update_contact, \
											**update_contact_stamp, \
											**delete_contact, \
											**upsert_contact_detail, \
											**select_contact_details, \
											**delete_contact_details, \
											**select_message_folder, \
											**upsert_user_config, \
											**select_user_config, \
											**delete_user_config, \
											**fetch_signature, \
											**update_signature_flags_add, \
											**update_signature_flags_remove, \
											**delete_signature, \
											**register_check_username, \
											**register_insert_user, \
											**register_insert_stacie_user, \
											**register_insert_profile, \
											**register_insert_folders, \
											**register_insert_folder_name, \
											**register_insert_log, \
											**register_insert_dispatch, \
											**register_insert_mailboxes, \
											**register_fetch_blocklist, \
											**delete_user, \
											**statistics_get_total_users, \
											**statistics_get_users_checked_email_today, \
											**statistics_get_users_checked_email_week, \
											**statistics_get_users_sent_email_today, \
											**statistics_get_users_sent_email_week, \
											**statistics_get_emails_received_today, \
											**statistics_get_emails_received_week, \
											**statistics_get_emails_sent_today, \
											**statistics_get_emails_sent_week, \
											**statistics_get_users_registered_today, \
											**statistics_get_users_registered_week, \
											**auth_get_by_userid, \
											**auth_get_by_address, \
											**auth_update_legacy_to_stacie

extern chr_t *queries[];
struct { MYSQL_STMT STMTS_INIT; } stmts __attribute__ ((common));

#endif
