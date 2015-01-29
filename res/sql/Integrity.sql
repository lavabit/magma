
/* Check for various error conditions. */

-- Look for domains without a mailbox.
SELECT domain FROM Domains WHERE mailboxes = 1 AND domain NOT IN (SELECT SUBSTRING_INDEX(address, '@', -1) FROM Mailboxes GROUP BY 1);

-- Look for mailbox domains without a domain record.
SELECT SUBSTRING_INDEX(address, '@', -1) FROM Mailboxes WHERE address REGEXP '.*\@.*' AND SUBSTRING_INDEX(address, '@', -1) NOT IN (SELECT domain FROM Domains WHERE mailboxes = 1);

-- Find wildcard domains where the domain record doesn't have the wildcard flag enabled.
SELECT address FROM Mailboxes WHERE address NOT REGEXP '.*\@.*' AND address NOT IN (SELECT domain FROM Domains WHERE wildcard = 1);

-- Find domain records indicating a wildcard but without an actual wildcard mailbox.
SELECT domain FROM Domains WHERE wildcard = 1 AND domain NOT IN (SELECT address FROM Mailboxes WHERE address NOT REGEXP '.*\@.*');


-- Find accounts whose size value may have underflowed.
CREATE TEMPORARY TABLE `#MailboxSizes` (usernum bigint unsigned, size bigint unsigned, actual bigint unsigned); 
INSERT INTO `#MailboxSizes` SELECT Users.usernum, Users.size, IFNULL(SUM(Messages.size), 0) FROM Users LEFT JOIN Messages ON Users.usernum = Messages.usernum GROUP BY Users.usernum; 
SELECT Users.usernum, Users.userid, Users.size, Users.quota FROM Users WHERE Users.usernum IN (SELECT usernum FROM `#MailboxSizes` WHERE size != actual);
DROP TABLE `#MailboxSizes`;

-- Find suspicious mailbox sizes without scanning the Messages table.
-- SELECT Users.usernum, Users.userid, Users.size, Users.quota FROM Users LEFT JOIN Limits ON Users.plan = Limits.plan WHERE Users.size > Users.quota + Limits.recv_size_limit_max;

-- Accounts without an Inbox.
SELECT Users.usernum, Users.userid FROM Users LEFT JOIN Folders ON Users.usernum = Folders.usernum AND Folders.foldername = 'Inbox' WHERE Folders.foldernum IS NULL;

-- Active accounts with a lock expiration date, or locked accounts without an expiration date.
SELECT Users.usernum, Users.userid FROM Users WHERE locked = 0 AND lock_expiration != '0000-00-00';
SELECT Users.usernum, Users.userid FROM Users WHERE locked != 0 AND lock_expiration = '0000-00-00'; 

-- Look for accounts with a plan expiration while using a free plan.
SELECT Users.usernum, Users.userid, Users.plan_expiration FROM Users LEFT JOIN Limits ON Users.plan = Limits.plan WHERE Limits.paid = 1 AND plan_expiration != '0000-00-00';

-- Accounts with more than one default mailbox address alias.
SELECT usernum, COUNT(*) AS total FROM Aliases WHERE selected = 1 GROUP BY usernum HAVING total > 1;

-- Accounts without a default mailbox address alias.
SELECT usernum, SUM(selected) AS total FROM Aliases GROUP BY usernum HAVING total = 0;

