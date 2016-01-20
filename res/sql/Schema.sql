
DROP TABLE IF EXISTS `Agents`;
CREATE TABLE `Agents` (
  `agentnum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `agent` text NOT NULL,
  `popularity` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`agentnum`)
) ENGINE=InnoDB AUTO_INCREMENT=9330 DEFAULT CHARSET=latin1 COMMENT='Weighted user agent strings.';

DROP TABLE IF EXISTS `Alerts`;
CREATE TABLE `Alerts` (
  `alertnum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `type` varchar(32) NOT NULL,
  `message` varchar(255) NOT NULL DEFAULT '',
  `acknowledged` datetime DEFAULT NULL,
  `created` datetime NOT NULL,
  PRIMARY KEY (`alertnum`),
  KEY `IX_USERNUM` (`usernum`),
  CONSTRAINT `Alerts_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='User alerts.';

DROP TABLE IF EXISTS `Aliases`;
CREATE TABLE `Aliases` (
  `aliasnum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `address` varchar(255) NOT NULL DEFAULT '',
  `display` varchar(255) NOT NULL DEFAULT '',
  `selected` tinyint(1) NOT NULL DEFAULT '0',
  `created` datetime NOT NULL,
  PRIMARY KEY (`aliasnum`),
  UNIQUE KEY `UNIQ_DISPLAY` (`usernum`,`address`,`display`),
  KEY `IX_USERNUM` (`usernum`),
  KEY `IX_USERNUM_ADDRESS` (`usernum`,`address`),
  CONSTRAINT `Aliases_fk_1` FOREIGN KEY (`usernum`, `address`) REFERENCES `Mailboxes` (`usernum`, `address`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='Store mailbox display aliases and default status information.';

DROP TABLE IF EXISTS `Autoreplies`;
CREATE TABLE `Autoreplies` (
  `replynum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `message` mediumtext NOT NULL,
  PRIMARY KEY (`replynum`),
  KEY `IX_USERNUM` (`usernum`),
  KEY `IX_REPLYNUM_USERNUM` (`replynum`,`usernum`),
  CONSTRAINT `Autoreplies_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=1000 COMMENT='Autoresponder replies are here.';

DROP TABLE IF EXISTS `Banned`;
CREATE TABLE `Banned` (
  `sequence` varchar(255) NOT NULL DEFAULT '',
  `created` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`sequence`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MIN_ROWS=4294967295 MAX_ROWS=4294967295 AVG_ROW_LENGTH=300 COMMENT='Stores a list of IPs banned from registration.';

DROP TABLE IF EXISTS `Contacts`;
CREATE TABLE `Contacts`(
    `contactnum` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    `usernum` BIGINT UNSIGNED NOT NULL,
    `foldernum` BIGINT UNSIGNED NOT NULL,
    `name` VARCHAR(255) NOT NULL,
    `updated` DATETIME NOT NULL,
    `created` DATETIME NOT NULL,
    PRIMARY KEY (`contactnum`),
    INDEX `fk_Contacts_1` (`usernum` ASC),
    INDEX `fk_Contacts_2` (`foldernum` ASC),
    UNIQUE KEY `uniq_Contacts_1` (`usernum`,`foldernum`,`name`),
    CONSTRAINT `fk_Contacts_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE,
    CONSTRAINT `fk_Contacts_2` FOREIGN KEY (`foldernum`) REFERENCES `Folders` (`foldernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='Address book contacts.';

CREATE TABLE `Contact_Details` (
  `contactdetailnum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `contactnum` bigint(20) unsigned NOT NULL,
  `key` varchar(255) NOT NULL,
  `value` text NOT NULL,
  `flags` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`contactdetailnum`),
  UNIQUE KEY `uniq_Contact_Details` (`contactnum`,`key`),
  KEY `fk_Contact_Details` (`contactnum`),
  CONSTRAINT `fk_Contact_Details_1` FOREIGN KEY (`contactnum`) REFERENCES `Contacts` (`contactnum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='Address book contact data entries.';

DROP TABLE IF EXISTS `Creation`;
CREATE TABLE `Creation` (
  `oct_one` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `oct_two` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `oct_three` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `timestamp` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  KEY `IX_OCTETS` (`oct_one`,`oct_two`,`oct_three`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MIN_ROWS=4294967295 MAX_ROWS=4294967295 AVG_ROW_LENGTH=24 COMMENT='Used to track new user creation.';

DROP TABLE IF EXISTS `Dispatch`;
CREATE TABLE `Dispatch` (
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `secure` tinyint(1) NOT NULL DEFAULT '0',
  `forwarded` varchar(100) DEFAULT '',
  `rollout` tinyint(1) NOT NULL DEFAULT '0',
  `bounces` tinyint(1) NOT NULL DEFAULT '1',
  `greylist` tinyint(1) NOT NULL DEFAULT '0',
  `greytime` int(10) unsigned NOT NULL DEFAULT '1',
  `rbl` tinyint(1) NOT NULL DEFAULT '0',
  `rblaction` enum('REJECT','MARK','MARK_READ','DELETE') DEFAULT 'REJECT',
  `spf` tinyint(1) NOT NULL DEFAULT '0',
  `spfaction` enum('REJECT','MARK','MARK_READ','DELETE') DEFAULT 'REJECT',
  `dkim` tinyint(1) NOT NULL DEFAULT '0',
  `dkimaction` enum('MARK','MARK_READ','BOUNCE','DELETE') DEFAULT 'MARK',
  `spam` tinyint(1) NOT NULL DEFAULT '0',
  `spamaction` enum('MARK','MARK_READ','BOUNCE','DELETE') DEFAULT 'MARK',
  `spamfolder` bigint(20) unsigned DEFAULT '0',
  `virus` tinyint(1) NOT NULL DEFAULT '1',
  `virusaction` enum('MARK','MARK_READ','BOUNCE','DELETE') DEFAULT 'DELETE',
  `phish` tinyint(1) NOT NULL DEFAULT '1',
  `phishaction` enum('MARK','MARK_READ','BOUNCE','DELETE') DEFAULT 'DELETE',
  `filters` tinyint(1) NOT NULL DEFAULT '0',
  `autoreply` bigint(20) unsigned DEFAULT NULL,
  `inbox` bigint(20) unsigned DEFAULT '0',
  `send_size_limit` int(10) unsigned NOT NULL DEFAULT '10485760',
  `recv_size_limit` int(10) unsigned NOT NULL DEFAULT '10485760',
  `daily_send_limit` int(10) unsigned NOT NULL DEFAULT '200',
  `daily_recv_limit` int(10) unsigned NOT NULL DEFAULT '400',
  `daily_recv_limit_ip` int(10) unsigned NOT NULL DEFAULT '400',
  `class` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`usernum`),
  KEY `IX_INBOX_USERNUM` (`inbox`,`usernum`),
  KEY `IX_COLORFOLDER_USERNUM` (`usernum`),
  KEY `IX_SPAMFOLDER_USERNUM` (`spamfolder`,`usernum`),
  KEY `IX_WHITEREPLY_USERNUM` (`usernum`),
  KEY `IX_AUTOREPLY_USERNUM` (`autoreply`,`usernum`),
  CONSTRAINT `Dispatch_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `Dispatch_ibfk_2` FOREIGN KEY (`inbox`, `usernum`) REFERENCES `Folders` (`foldernum`, `usernum`) ON UPDATE CASCADE,
  CONSTRAINT `Dispatch_ibfk_4` FOREIGN KEY (`spamfolder`, `usernum`) REFERENCES `Folders` (`foldernum`, `usernum`) ON UPDATE CASCADE,
  CONSTRAINT `Dispatch_ibfk_6` FOREIGN KEY (`autoreply`, `usernum`) REFERENCES `Autoreplies` (`replynum`, `usernum`) ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='Preferences used by the dispatchd stored here.';

DROP TABLE IF EXISTS `Display`;
CREATE TABLE `Display` (
  `displaynum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `wordnum` bigint(20) NOT NULL DEFAULT '0',
  `partner` varchar(16) NOT NULL DEFAULT '',
  `group` varchar(16) NOT NULL DEFAULT '',
  `domain` varchar(255) NOT NULL DEFAULT '',
  `text` varchar(255) NOT NULL DEFAULT '',
  `bid` float(20,6) NOT NULL DEFAULT '0.000000',
  `timestamp` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`displaynum`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=320 COMMENT='Record advertisements displayed.';

DROP TABLE IF EXISTS `Domains`;
CREATE TABLE `Domains` (
  `domnum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `domain` varchar(255) DEFAULT NULL,
  `restricted` tinyint(1) NOT NULL DEFAULT '0',
  `mailboxes` tinyint(1) NOT NULL DEFAULT '0',
  `wildcard` tinyint(1) NOT NULL DEFAULT '0',
  `dkim` tinyint(1) NOT NULL DEFAULT '0',
  `spf` tinyint(1) NOT NULL DEFAULT '0',
  `updated` datetime DEFAULT NULL,
  `created` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`domnum`),
  UNIQUE KEY `UNIQ_DOMAIN` (`domain`)
) ENGINE=InnoDB AUTO_INCREMENT=47 DEFAULT CHARSET=latin1 MIN_ROWS=4294967295 MAX_ROWS=4294967295 AVG_ROW_LENGTH=300 COMMENT='Stores domain routing information for the outbound cluster.';

DROP TABLE IF EXISTS `Filters`;
CREATE TABLE `Filters` (
  `rulenum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `location` int(10) unsigned NOT NULL DEFAULT '0',
  `type` int(10) unsigned NOT NULL DEFAULT '0',
  `action` int(10) unsigned NOT NULL DEFAULT '0',
  `foldernum` bigint(20) unsigned DEFAULT NULL,
  `field` varchar(100) DEFAULT NULL,
  `label` varchar(100) DEFAULT NULL,
  `expression` varchar(100) NOT NULL DEFAULT 'EMPTY',
  `name` varchar(100) NOT NULL DEFAULT 'Nameless',
  PRIMARY KEY (`rulenum`),
  KEY `IX_USERNUM` (`usernum`),
  KEY `IX_FOLDERNUM` (`foldernum`),
  CONSTRAINT `Filters_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `Filters_ibfk_2` FOREIGN KEY (`foldernum`) REFERENCES `Folders` (`foldernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=350 COMMENT='Rules used for sorting user messages stored here.';

DROP TABLE IF EXISTS `Folders`;
CREATE TABLE `Folders` (
  `foldernum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `foldername` varchar(128) NOT NULL DEFAULT '',
  `order` int(10) unsigned NOT NULL DEFAULT '0',
  `parent` bigint(20) unsigned DEFAULT NULL,
  PRIMARY KEY (`foldernum`),
  UNIQUE KEY `UNIQ_FOLDERNAME` (`usernum`,`foldername`,`parent`),
  KEY `IX_USERNUM` (`usernum`),
  KEY `IX_FOLDERNUM_USERNUM` (`foldernum`,`usernum`),
  CONSTRAINT `Folders_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='Aggregate folder statistics stored here.';

DROP TABLE IF EXISTS `Greylist`;
CREATE TABLE `Greylist` (
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `oct_one` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `oct_two` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `oct_three` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `oct_four` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `first_contact` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `last_contact` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`usernum`,`oct_one`,`oct_two`,`oct_three`,`oct_four`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1 MIN_ROWS=4294967295 MAX_ROWS=4294967295 AVG_ROW_LENGTH=28 COMMENT='Greylist information is stored here.';

DROP TABLE IF EXISTS `Host_Config`;
CREATE TABLE `Host_Config` (
  `confignum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `hostnum` bigint(20) unsigned DEFAULT NULL,
  `application` varchar(32) NOT NULL,
  `name` varchar(128) NOT NULL,
  `value` varchar(128) NOT NULL,
  `timestamp` datetime NOT NULL,
  PRIMARY KEY (`confignum`),
  KEY `IX_HOST` (`hostnum`),
  CONSTRAINT `FK_HOST_CONFIG_HOSTS` FOREIGN KEY (`hostnum`) REFERENCES `Hosts` (`hostnum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=latin1 COMMENT='Application configs are stored in name value pairings.';

DROP TABLE IF EXISTS `Hosts`;
CREATE TABLE `Hosts` (
  `hostnum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `hostname` varchar(255) DEFAULT NULL,
  `timestamp` datetime NOT NULL,
  PRIMARY KEY (`hostnum`),
  UNIQUE KEY `UNIQ_PREF_HOSTNAME` (`hostname`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1 COMMENT='Hostnames are paired with host numbers.';

DROP TABLE IF EXISTS `Keys`;
CREATE TABLE `Keys` (
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `storage_pub` text,
  `storage_priv` text,
  PRIMARY KEY (`usernum`),
  CONSTRAINT `Keys_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=5000 COMMENT='Encryption keys are stored here.';

DROP TABLE IF EXISTS `Limits`;
CREATE TABLE `Limits` (
  `plan` varchar(25) NOT NULL DEFAULT '',
  `description` varchar(255) NOT NULL DEFAULT '',
  `corporate` tinyint(1) NOT NULL DEFAULT '0',
  `paid` tinyint(1) NOT NULL DEFAULT '0',
  `secure` tinyint(1) NOT NULL DEFAULT '1',
  `forwarded` tinyint(1) NOT NULL DEFAULT '0',
  `rollout` tinyint(1) NOT NULL DEFAULT '1',
  `bounces` tinyint(1) NOT NULL DEFAULT '1',
  `greylist` tinyint(1) NOT NULL DEFAULT '1',
  `greytime_min` int(10) NOT NULL DEFAULT '1',
  `greytime_max` int(10) NOT NULL DEFAULT '30',
  `filters` tinyint(1) NOT NULL DEFAULT '1',
  `rbl` tinyint(1) NOT NULL DEFAULT '1',
  `rblaction_list` varchar(255) NOT NULL DEFAULT 'REJECT,DELETE,MARK',
  `spf` tinyint(1) NOT NULL DEFAULT '1',
  `spfaction_list` varchar(255) NOT NULL DEFAULT 'REJECT,DELETE,MARK',
  `dkim` tinyint(1) NOT NULL DEFAULT '1',
  `dkimaction_list` varchar(255) NOT NULL DEFAULT 'DELETE,MARK',
  `spam` tinyint(1) NOT NULL DEFAULT '1',
  `spamaction_list` varchar(255) NOT NULL DEFAULT 'DELETE,MARK',
  `virus` tinyint(1) NOT NULL DEFAULT '1',
  `virusaction_list` varchar(255) NOT NULL DEFAULT 'DELETE,MARK',
  `phish` tinyint(1) NOT NULL DEFAULT '1',
  `phishaction_list` varchar(255) NOT NULL DEFAULT 'DELETE,MARK',
  `wormaction_list` varchar(255) NOT NULL DEFAULT 'DELETE',
  `autoreply` tinyint(1) DEFAULT '0',
  `send_size_limit` tinyint(1) NOT NULL DEFAULT '1',
  `send_size_limit_min` int(10) NOT NULL DEFAULT '1048576',
  `send_size_limit_max` int(10) NOT NULL DEFAULT '524288000',
  `recv_size_limit` tinyint(1) NOT NULL DEFAULT '1',
  `recv_size_limit_min` int(10) NOT NULL DEFAULT '1048576',
  `recv_size_limit_max` int(10) NOT NULL DEFAULT '524288000',
  `daily_send_limit` tinyint(1) NOT NULL DEFAULT '1',
  `daily_send_limit_min` int(10) NOT NULL DEFAULT '10',
  `daily_send_limit_max` int(10) NOT NULL DEFAULT '200',
  `daily_recv_limit` tinyint(1) NOT NULL DEFAULT '1',
  `daily_recv_limit_min` int(10) NOT NULL DEFAULT '10',
  `daily_recv_limit_max` int(10) NOT NULL DEFAULT '1000',
  `daily_recv_limit_ip` tinyint(1) NOT NULL DEFAULT '1',
  `daily_recv_limit_ip_min` int(10) NOT NULL DEFAULT '10',
  `daily_recv_limit_ip_max` int(10) NOT NULL DEFAULT '200',
  `ssl` tinyint(1) NOT NULL DEFAULT '1',
  `password` tinyint(1) NOT NULL DEFAULT '1',
  PRIMARY KEY (`plan`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `Locking`;
CREATE TABLE `Locking` (
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `timestamp` varchar(100) NOT NULL DEFAULT '',
  PRIMARY KEY (`usernum`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1 MIN_ROWS=4294967295 MAX_ROWS=4294967295 AVG_ROW_LENGTH=24 COMMENT='Account locking is handled by this table.';

DROP TABLE IF EXISTS `Log`;
CREATE TABLE `Log` (
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `lastpop` datetime DEFAULT '0000-00-00 00:00:00',
  `lastmap` datetime DEFAULT '0000-00-00 00:00:00',
  `lastweb` datetime DEFAULT '0000-00-00 00:00:00',
  `lastchat` datetime DEFAULT '0000-00-00 00:00:00',
  `lastsent` datetime DEFAULT '0000-00-00 00:00:00',
  `lastreceived` datetime DEFAULT '0000-00-00 00:00:00',
  `popsessions` bigint(20) unsigned NOT NULL DEFAULT '0',
  `mapsessions` bigint(20) unsigned NOT NULL DEFAULT '0',
  `websessions` bigint(20) unsigned NOT NULL DEFAULT '0',
  `chatsessions` bigint(20) unsigned NOT NULL DEFAULT '0',
  `totalsent` bigint(20) unsigned NOT NULL DEFAULT '0',
  `totalreceived` bigint(20) unsigned NOT NULL DEFAULT '0',
  `totalbounces` bigint(20) unsigned NOT NULL DEFAULT '0',
  `created` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `created_ip` varchar(15) NOT NULL DEFAULT '0.0.0.0',
  PRIMARY KEY (`usernum`),
  CONSTRAINT `Log_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=150 COMMENT='Table used to track aggregate user activity stats.';

DROP TABLE IF EXISTS `Mailboxes`;
CREATE TABLE `Mailboxes` (
  `address` varchar(255) NOT NULL DEFAULT '',
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`address`),
  KEY `IX_USERNUM` (`usernum`),
  CONSTRAINT `Mailboxes_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='All mailboxes for which we accept e-mail.';

DROP TABLE IF EXISTS `Messages`;
CREATE TABLE `Messages` (
  `messagenum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `foldernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `server` enum('mary','mary2','local') NOT NULL DEFAULT 'mary',
  `status` int(10) unsigned NOT NULL DEFAULT '0',
  `size` int(11) unsigned NOT NULL DEFAULT '0',
  `signum` bigint(20) unsigned DEFAULT '0',
  `sigkey` bigint(20) unsigned DEFAULT '0',
  `visible` tinyint(1) NOT NULL DEFAULT '1',
  `created` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`messagenum`),
  KEY `IX_USERNUM` (`usernum`),
  KEY `IX_SIGNUM` (`signum`),
  KEY `IX_FOLDERNUM_USERNUM` (`foldernum`,`usernum`),
  CONSTRAINT `Messages_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON UPDATE CASCADE,
  CONSTRAINT `Messages_ibfk_2` FOREIGN KEY (`foldernum`, `usernum`) REFERENCES `Folders` (`foldernum`, `usernum`) ON UPDATE CASCADE,
  CONSTRAINT `Messages_ibfk_3` FOREIGN KEY (`signum`) REFERENCES `Signatures` (`signum`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=300 COMMENT='A list of all e-mails we have stored on the system.';

DROP TABLE IF EXISTS `Message_Tags`;
CREATE TABLE `Message_Tags` (
  `messagetagnum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `messagenum` bigint(20) unsigned NOT NULL,
  `tag` varchar(255) NOT NULL,
  PRIMARY KEY (`messagetagnum`),
  KEY `IX_MESSAGENUM` (`messagenum`),
  UNIQUE KEY `UNIQ_MESSAGENUM_TAG` (`messagenum`,`tag`),
  CONSTRAINT `Message_Tags_ibfk_1` FOREIGN KEY (`messagenum`) REFERENCES `Messages` (`messagenum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=60 COMMENT='The list of the user generated message tags.';

DROP TABLE IF EXISTS `Objects`;
CREATE TABLE `Objects` (
  `objectnum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned DEFAULT NULL,
  `hostnum` bigint(20) unsigned DEFAULT NULL,
  `tank` bigint(20) unsigned DEFAULT NULL,
  `size` bigint(20) unsigned NOT NULL,
  `serial` int(10) NOT NULL DEFAULT '0',
  `flags` bigint(20) unsigned DEFAULT '0',
  `references` int(11) unsigned NOT NULL DEFAULT '0',
  `timestamp` datetime NOT NULL,
  PRIMARY KEY (`objectnum`),
  KEY `IX_USER` (`usernum`),
  KEY `IX_HOST` (`hostnum`),
  CONSTRAINT `FK_OBJECTS_HOSTS` FOREIGN KEY (`hostnum`) REFERENCES `Hosts` (`hostnum`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `FK_OBJECTS_USERS` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COMMENT='A list of all objects stored throughout the system.';

DROP TABLE IF EXISTS `Patterns`;
CREATE TABLE `Patterns` (
  `pattern` varchar(255) NOT NULL DEFAULT '',
  `created` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`pattern`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MIN_ROWS=4294967295 MAX_ROWS=4294967295 AVG_ROW_LENGTH=255 COMMENT='Contains strings that result in blocking an outbound message';

DROP TABLE IF EXISTS `Profile`;
CREATE TABLE `Profile` (
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `name` varchar(100) DEFAULT '',
  `address_one` varchar(255) DEFAULT '',
  `address_two` varchar(255) DEFAULT '',
  `city` varchar(100) DEFAULT '',
  `state` varchar(100) DEFAULT '',
  `zip` int(11) DEFAULT '0',
  `country` varchar(100) DEFAULT '',
  `profile` text,
  `phone` varchar(100) DEFAULT '',
  `mobile` varchar(100) DEFAULT '',
  `fax` varchar(100) DEFAULT '',
  `gender` enum('MALE','FEMALE') DEFAULT NULL,
  `language` varchar(100) DEFAULT '',
  `birthdate` date DEFAULT '0000-00-00',
  `industry` varchar(100) DEFAULT '',
  `website` varchar(255) DEFAULT '',
  `accessible` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`usernum`),
  CONSTRAINT `Profile_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=1000 COMMENT='Any information the user has chosen to give us.';

DROP TABLE IF EXISTS `Receiving`;
CREATE TABLE `Receiving` (
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `subnet` varchar(64) DEFAULT NULL,
  `timestamp` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  KEY `IX_USERNUM` (`usernum`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='Used to track messages received by user.';

DROP TABLE IF EXISTS `Signatures`;
CREATE TABLE `Signatures` (
  `signum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `cryptkey` bigint(20) unsigned NOT NULL DEFAULT '0',
  `junk` tinyint(1) NOT NULL DEFAULT '0',
  `signature` blob NOT NULL,
  `created` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`signum`),
  KEY `IX_USERNUM` (`usernum`),
  KEY `IX_CREATED` (`created`),
  CONSTRAINT `Signatures_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=600 COMMENT='Spam signatures are stored here.';

DROP TABLE IF EXISTS `Tracker`;
CREATE TABLE `Tracker` (
  `script` varchar(255) NOT NULL DEFAULT '',
  `checkpoint` varchar(255) NOT NULL DEFAULT '',
  `comment` varchar(255) DEFAULT NULL,
  `timestamp` datetime NOT NULL DEFAULT '0000-00-00 00:00:00'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

DROP TABLE IF EXISTS `Transmitting`;
CREATE TABLE `Transmitting` (
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `timestamp` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  KEY `IX_USERNUM` (`usernum`)
) ENGINE=MEMORY DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='Used to track how many messages a user sends.';

DROP TABLE IF EXISTS `Users`;
CREATE TABLE `Users` (
  `usernum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `userid` varchar(255) NOT NULL DEFAULT '',
  `salt` varchar(256),
  `auth` varchar(128),
  `password` varchar(130) NOT NULL DEFAULT '',
  `ssl` tinyint(1) NOT NULL DEFAULT '0',
  `plan` varchar(25) NOT NULL DEFAULT 'FREE',
  `locked` tinyint(1) NOT NULL DEFAULT '0',
  `advertising` tinyint(1) NOT NULL DEFAULT '1',
  `domain` varchar(255) DEFAULT NULL,
  `email` tinyint(1) NOT NULL DEFAULT '1',
  `chat` tinyint(1) NOT NULL DEFAULT '1',
  `timezone` int(11) NOT NULL DEFAULT '0',
  `size` bigint(20) unsigned NOT NULL DEFAULT '0',
  `quota` bigint(20) unsigned NOT NULL DEFAULT '1073741824',
  `overquota` tinyint(1) NOT NULL DEFAULT '0',
  `plan_expiration` date DEFAULT '0000-00-00',
  `lock_expiration` date DEFAULT '0000-00-00',
  PRIMARY KEY (`usernum`),
  UNIQUE KEY `UNIQ_USERNAME` (`userid`),
  KEY `DEX_MAILSHACK` (`userid`,`password`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=1000 COMMENT='Basic user account information and settings.';

CREATE TABLE `User_Config` (
  `userconfignum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned NOT NULL,
  `key` varchar(255) NOT NULL,
  `value` text NOT NULL,
  `flags` bigint(20) unsigned NOT NULL DEFAULT '0',
  `timestamp` datetime NOT NULL,
  PRIMARY KEY (`userconfignum`),
  UNIQUE KEY `uniq_User_Config` (`usernum`,`key`),
  KEY `fk_User_Config` (`usernum`),
  CONSTRAINT `fk_User_Config_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='User config settings.';

DROP TABLE IF EXISTS `dspam_stats`;
CREATE TABLE `dspam_stats` (
  `uid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `spam_learned` int(11) NOT NULL DEFAULT '0',
  `innocent_learned` int(11) NOT NULL DEFAULT '0',
  `spam_misclassified` int(11) NOT NULL DEFAULT '0',
  `innocent_misclassified` int(11) NOT NULL DEFAULT '0',
  `spam_corpusfed` int(11) NOT NULL DEFAULT '0',
  `innocent_corpusfed` int(11) NOT NULL DEFAULT '0',
  `spam_classified` int(11) NOT NULL DEFAULT '0',
  `innocent_classified` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`uid`),
  CONSTRAINT `dspam_stats_ibfk_1` FOREIGN KEY (`uid`) REFERENCES `dspam_virtual_uids` (`uid`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='DSPAM aggregate statistics are stored here.';

DROP TABLE IF EXISTS `dspam_token_data`;
CREATE TABLE `dspam_token_data` (
  `uid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `token` bigint(20) unsigned NOT NULL DEFAULT '0',
  `spam_hits` int(11) NOT NULL DEFAULT '0',
  `innocent_hits` int(11) NOT NULL DEFAULT '0',
  `last_hit` date NOT NULL DEFAULT '0000-00-00',
  PRIMARY KEY (`uid`,`token`),
  KEY `id_token_data_01` (`innocent_hits`),
  KEY `id_token_data_02` (`spam_hits`),
  KEY `id_token_data_03` (`last_hit`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='DSPAM tokens are stored here.';

DROP TABLE IF EXISTS `dspam_virtual_uids`;
CREATE TABLE `dspam_virtual_uids` (
  `uid` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `username` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`uid`),
  UNIQUE KEY `id_virtual_uids_01` (`username`),
  CONSTRAINT `dspam_virtual_uids_ibfk_1` FOREIGN KEY (`username`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='DSPAM userid mappings are stored here.';

