-- Tracking info.
INSERT INTO Tracker (script, checkpoint, timestamp) VALUES ("daily.sql", "STARTING", NOW());

-- This query will lock any user accounts which have not been accessed in 120 days.
UPDATE Users, Log, Limits SET Users.locked = 2, Users.lock_expiration = DATE_ADD(NOW(), INTERVAL 120 DAY) WHERE Users.usernum = Log.usernum AND Users.locked = 0 AND Log.lastpop < DATE_SUB(NOW(), INTERVAL 120 DAY) AND Log.lastweb < DATE_SUB(NOW(), INTERVAL 120 DAY) AND Log.lastsent < DATE_SUB(NOW(), INTERVAL 120 DAY) AND Log.lastmap < DATE_SUB(NOW(), INTERVAL 120 DAY) AND Log.created < DATE_SUB(NOW(), INTERVAL 120 DAY) AND Log.lastchat < DATE_SUB(NOW(), INTERVAL 120 DAY) AND Log.created < DATE_SUB(NOW(), INTERVAL 120 DAY) AND Users.plan_expiration = '0000-00-00' AND Users.plan = Limits.plan AND Limits.paid = 0;

-- This query will remove any records from the Creation table, more than 21 days old.
DELETE FROM Creation WHERE timestamp < DATE_SUB(NOW(), INTERVAL 21 DAY);

-- This will remove stale records from the Greylist table, more than 30 days old.
DELETE FROM Greylist WHERE last_contact < DATE_SUB(NOW(), INTERVAL 30 DAY);

-- Check for stale locks.
DELETE FROM Locking WHERE timestamp < DATE_SUB(NOW(), INTERVAL 1 HOUR);

-- Cleanup the transmitting table, delete records which are older than 7 days.
DELETE FROM Transmitting WHERE timestamp < DATE_SUB(NOW(), INTERVAL 7 DAY);

-- Cleanup the receiving table, delete records which are older than 7 days.
DELETE FROM Receiving WHERE timestamp < DATE_SUB(NOW(), INTERVAL 7 DAY);

-- New isolation level for these big updates.
SET SESSION TRANSACTION ISOLATION LEVEL READ UNCOMMITTED;

-- Tracking info.
INSERT INTO Tracker (script, checkpoint, timestamp) VALUES ("daily.sql", "SIGNATURES", NOW());

-- Delete DSPAM signatures which are older than 120 days.
START TRANSACTION;
DELETE FROM Signatures WHERE created < DATE_SUB(NOW(), INTERVAL 120 DAY);
COMMIT;

-- Tracking info.
INSERT INTO Tracker (script, checkpoint, timestamp) VALUES ("daily.sql", "TOKENS", NOW());

-- DSPAM data purge.
DELETE FROM dspam_token_data WHERE TO_DAYS(NOW()) - TO_DAYS(last_hit) > 365;

-- Tracking info.
INSERT INTO Tracker (script, checkpoint, timestamp) VALUES ("daily.sql", "FINISHED", NOW());

/*
set @a=to_days(current_date());

START TRANSACTION;
delete from dspam_token_data
  where (innocent_hits*2) + spam_hits < 5
  and @a-to_days(last_hit) > 90;
COMMIT;

START TRANSACTION;
delete from dspam_token_data
  where innocent_hits = 1 and spam_hits = 0
  and @a-to_days(last_hit) > 60;
COMMIT;

START TRANSACTION;
delete from dspam_token_data
  where innocent_hits = 0 and spam_hits = 1
  and @a-to_days(last_hit) > 60;
COMMIT;

START TRANSACTION;
delete from dspam_token_data
  where @a-to_days(last_hit) > 90;
COMMIT;
*/

