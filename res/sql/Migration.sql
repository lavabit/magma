

/* Statements needed to migrate the legacy schema to the new model. */


/* Alter table Folders. */
UPDATE Folders SET parent = 0 WHERE parent IS NULL;
ALTER TABLE `Folders` ADD COLUMN `type` INT(10) UNSIGNED NOT NULL DEFAULT 1 AFTER `order`, MODIFY COLUMN  `parent` bigint(20) unsigned NOT NULL DEFAULT 0;
ALTER TABLE `Folders` DROP INDEX `UNIQ_FOLDERNAME`, ADD UNIQUE INDEX `UNIQ_FOLDERNAME` (`usernum`, `type`, `parent`, `foldername`) ;

/* Alter the Users table for STACIE support. Deprecate any existing legacy passwords. */
/* ALTER TABLE `Lavabit`.`Users` CHANGE COLUMN `password` `legacy` VARCHAR(130) NULL DEFAULT NULL; */

ALTER TABLE `Users` CHANGE COLUMN `ssl` `tls` tinyint(1) NOT NULL DEFAULT '0';
ALTER TABLE `Limits` CHANGE COLUMN `ssl` `tls` tinyint(1) NOT NULL DEFAULT '1';

DROP TABLE `Aliases`;

ALTER TABLE `Mailboxes`
DROP PRIMARY KEY,
ADD COLUMN `mailboxnum` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT FIRST,
ADD PRIMARY KEY (`mailboxnum`),
ADD UNIQUE INDEX `UNIQ_ADDRESS` (`address` ASC);

CREATE TABLE `Aliases` (
  `aliasnum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `address` varchar(255) NOT NULL,
  `display` varchar(255) NOT NULL,
  `selected` tinyint(1) NOT NULL DEFAULT '0',
  `created` datetime NOT NULL,
  PRIMARY KEY (`aliasnum`),
  UNIQUE KEY `UNIQ_DISPLAY` (`usernum`,`address`,`display`),
  KEY `IX_USERNUM` (`usernum`),
  KEY `IX_USERNUM_ADDRESS` (`usernum`,`address`),
  CONSTRAINT `Aliases_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='Store mailbox display aliases and default status information';

INSERT INTO Aliases (`usernum`,`address`,`display`,`created`) SELECT usernum, address, address, NOW() FROM Mailboxes;
UPDATE Aliases SET selected = 1 WHERE address LIKE "%@lavabit.com";
CREATE TEMPORARY TABLE nums (num bigint);
INSERT INTO nums SELECT aliasnum FROM Aliases GROUP BY usernum ORDER BY selected, CHARACTER_LENGTH(address);
UPDATE Aliases SET selected = 0;
UPDATE Aliases SET selected = 1 WHERE aliasnum IN (SELECT num FROM nums);
DROP TABLE nums;

/*DROP TABLE  IF EXISTS `User_Keys`;
CREATE TABLE `User_Keys` (
  `keynum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `mailboxnum` bigint(20) unsigned NOT NULL,
  `fingerprint` VARCHAR(128) NOT NULL,
  `signet` TEXT NOT NULL,
  `private` TEXT NOT NULL,
  PRIMARY KEY (`keynum`),
  KEY `IX_USERNUM` (`usernum`),
  KEY `IX_MAILBOXNUM` (`mailboxnum`),
  CONSTRAINT `User_Keys_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON UPDATE CASCADE,
  CONSTRAINT `User_Keys_ibfk_2` FOREIGN KEY (`mailboxnum`) REFERENCES `Mailboxes` (`mailboxnum`) ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='Signets and keys for every mailbox.';*/

DROP TABLE  IF EXISTS `Realms`;
CREATE TABLE `Realms` (
  `usernum` bigint(20) unsigned NOT NULL DEFAULT '0',
  `serial` smallint(5) NOT NULL DEFAULT '0',
  `label` VARCHAR(16) NOT NULL,
  `shard` VARCHAR(86) NOT NULL,
  PRIMARY KEY (`usernum`, `serial`, `label`),
  KEY `IX_USERNUM` (`usernum`),
  CONSTRAINT `User_Realms_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=100 COMMENT='User shard values for the different realms.';

ALTER TABLE `Realms` 
ADD COLUMN `rotated` TINYINT(1) NOT NULL DEFAULT '0' AFTER `shard`;

ALTER TABLE `Codes` ADD COLUMN `years` tinyint(2) NOT NULL DEFAULT '1' AFTER `plan`;

ALTER TABLE `Users` ADD COLUMN `admin` tinyint(2) NOT NULL DEFAULT '0' AFTER `overquota`;

ALTER TABLE `Limits` ADD COLUMN `quota` bigint(20) NOT NULL DEFAULT '21474836480' AFTER `daily_recv_limit_ip_max`;

DROP TABLE IF EXISTS `Requests`;
CREATE TABLE `Requests` (
  `requestnum` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `usernum` bigint(20) unsigned NOT NULL,
  `requester` bigint(20) unsigned DEFAULT NULL,
  `requested` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `reviewer` bigint(20) unsigned DEFAULT NULL,
  `reviewed` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  `action` enum('RESET','DELETE') NOT NULL,
  `disposition` enum('PENDING', 'APPROVED','REJECTED') DEFAULT 'PENDING',
  `notes` text,
  `filename` varchar(255) NOT NULL,
  `attachment` mediumblob,
  PRIMARY KEY (`requestnum`),
  KEY `IX_REQUESTNUM` (`requestnum`),
  CONSTRAINT `Requests_ibfk_1` FOREIGN KEY (`usernum`) REFERENCES `Users` (`usernum`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `Requests_ibfk_2` FOREIGN KEY (`requester`) REFERENCES `Users` (`usernum`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `Requests_ibfk_4` FOREIGN KEY (`reviewer`) REFERENCES `Users` (`usernum`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 MAX_ROWS=4294967295 AVG_ROW_LENGTH=1000 COMMENT='Store administrative action requests which require approval.';


