

/* Statements needed to migrate the legacy schema to the new model. */

INSERT INTO Aliases (`usernum`,`address`,`display`,`created`) SELECT usernum, address, address, NOW() FROM Mailboxes;
UPDATE Aliases SET selected = 1 WHERE address LIKE "%@lavabit.com";
CREATE TEMPORARY TABLE nums (num bigint);
INSERT INTO nums SELECT aliasnum FROM Aliases GROUP BY usernum ORDER BY selected, CHARACTER_LENGTH(address);
UPDATE Aliases SET selected = 0;
UPDATE Aliases SET selected = 1 WHERE aliasnum IN (SELECT num FROM nums);
DROP TABLE nums;

/* Alter table Folders. */
UPDATE Folders SET parent = 0 WHERE parent IS NULL;
ALTER TABLE `Folders` ADD COLUMN `type` INT(10) UNSIGNED NOT NULL DEFAULT 1 AFTER `order`, MODIFY COLUMN  `parent` bigint(20) unsigned NOT NULL DEFAULT 0;
ALTER TABLE `Folders` DROP INDEX `UNIQ_FOLDERNAME`, ADD UNIQUE INDEX `UNIQ_FOLDERNAME` (`usernum`, `type`, `parent`, `foldername`) ;

/* Alter the Users table for STACIE support. Deprecate any existing legacy passwords. */
/* ALTER TABLE `Lavabit`.`Users` CHANGE COLUMN `password` `legacy` VARCHAR(130) NULL DEFAULT NULL; */

ALTER TABLE `Lavabit`.`Users` CHANGE COLUMN `ssl` `tls` tinyint(1) NOT NULL DEFAULT '0';
ALTER TABLE `Lavabit`.`Limits` CHANGE COLUMN `ssl` `tls` tinyint(1) NOT NULL DEFAULT '1';
