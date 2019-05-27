
--
-- Dumping data for table `Agents`
--
-- ORDER BY:  `agentnum`

/*!40000 ALTER TABLE `Agents` DISABLE KEYS */;
/*!40000 ALTER TABLE `Agents` ENABLE KEYS */;

--
-- Dumping data for table `Alerts`
--
-- ORDER BY:  `alertnum`

/*!40000 ALTER TABLE `Alerts` DISABLE KEYS */;
INSERT INTO `Alerts` VALUES (1,1,'alert','The US debt stands at $18,005,654,570,781.04. That is $153,723.01 for every American taxpayer.',NULL,'2011-05-06 17:57:34'),(2,2,'alert','The US debt stands at $18,005,654,570,781.04. That is $153,723.01 for every American taxpayer.',NULL,'2011-05-06 17:57:34'),(3,3,'alert','The US debt stands at $18,005,654,570,781.04. That is $153,723.01 for every American taxpayer.',NULL,'2011-05-06 17:57:34');
/*!40000 ALTER TABLE `Alerts` ENABLE KEYS */;

--
-- Dumping data for table `Aliases`
--
-- ORDER BY:  `aliasnum`

/*!40000 ALTER TABLE `Aliases` DISABLE KEYS */;
/*!40000 ALTER TABLE `Aliases` ENABLE KEYS */;

--
-- Dumping data for table `Autoreplies`
--
-- ORDER BY:  `replynum`

/*!40000 ALTER TABLE `Autoreplies` DISABLE KEYS */;
/*!40000 ALTER TABLE `Autoreplies` ENABLE KEYS */;

--
-- Dumping data for table `Creation`
--

/*!40000 ALTER TABLE `Creation` DISABLE KEYS */;
/*!40000 ALTER TABLE `Creation` ENABLE KEYS */;

--
-- Dumping data for table `Dispatch`
--
-- ORDER BY:  `usernum`

/*!40000 ALTER TABLE `Dispatch` DISABLE KEYS */;
INSERT INTO `Dispatch` VALUES (1,1,'',0,1,0,5,1,'REJECT',1,'REJECT',1,'MARK',1,'MARK',1,1,'DELETE',1,'DELETE',0,NULL,1,33554432,33554432,256,65536,65536,0),(2,0,'',0,1,0,5,1,'REJECT',1,'REJECT',1,'MARK',1,'MARK',2,1,'DELETE',1,'DELETE',0,NULL,2,33554432,33554432,256,65536,65536,0),(3,1,'',0,1,0,5,1,'REJECT',1,'REJECT',1,'MARK',0,'MARK',3,1,'DELETE',1,'DELETE',0,NULL,3,33554432,33554432,256,65536,65536,0),(4,1,'',0,1,0,5,1,'REJECT',1,'REJECT',1,'MARK',1,'MARK',4,1,'DELETE',1,'DELETE',0,NULL,4,33554432,33554432,256,65536,65536,0);
/*!40000 ALTER TABLE `Dispatch` ENABLE KEYS */;

--
-- Dumping data for table `Display`
--
-- ORDER BY:  `displaynum`

/*!40000 ALTER TABLE `Display` DISABLE KEYS */;
/*!40000 ALTER TABLE `Display` ENABLE KEYS */;

--
-- Dumping data for table `Domains`
--
-- ORDER BY:  `domnum`

/*!40000 ALTER TABLE `Domains` DISABLE KEYS */;
INSERT INTO `Domains` VALUES (1,'dark.lavabit.com',0,0,0,0,0,'2010-05-18 02:08:52','2008-03-06 14:52:16'),(2,'lavabit.com',1,1,0,1,1,'2010-05-18 02:08:55','2008-03-06 14:50:50'),(3,'mailshack.com',1,1,0,1,1,'2010-05-18 02:08:56','2008-03-06 14:50:59'),(4,'nerdshack.com',1,1,0,1,1,'2010-05-18 02:08:56','2008-03-06 14:50:55'),(33,'ronweb.net',0,0,1,1,1,'2010-05-18 02:09:03','2008-03-06 14:51:24'),(37,'squeak-seo.com',0,0,1,0,1,'2010-05-18 02:09:03','2008-03-06 14:51:34'),(40,'texasteenage.org',0,0,1,0,0,'2010-05-18 02:09:03','2009-03-05 11:50:04'),(47,'test.com',0,1,0,0,0,'2010-05-18 02:09:04','2008-11-10 10:00:32');
/*!40000 ALTER TABLE `Domains` ENABLE KEYS */;

--
-- Dumping data for table `Filters`
--
-- ORDER BY:  `rulenum`

/*!40000 ALTER TABLE `Filters` DISABLE KEYS */;
/*!40000 ALTER TABLE `Filters` ENABLE KEYS */;

--
-- Dumping data for table `Folders`
--
-- ORDER BY:  `foldernum`

/*!40000 ALTER TABLE `Folders` DISABLE KEYS */;
INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `parent`) VALUES (1,1,'Inbox',0,NULL),(2,2,'Inbox',0,NULL),(3,3,'Inbox',0,NULL),(4,4,'Inbox',0,NULL);
/*!40000 ALTER TABLE `Folders` ENABLE KEYS */;

--
-- Dumping data for table `Greylist`
--
-- ORDER BY:  `usernum`,`oct_one`,`oct_two`,`oct_three`,`oct_four`

/*!40000 ALTER TABLE `Greylist` DISABLE KEYS */;
/*!40000 ALTER TABLE `Greylist` ENABLE KEYS */;

--
-- Dumping data for table `Host_Config`
--
-- ORDER BY:  `confignum`

/*!40000 ALTER TABLE `Host_Config` DISABLE KEYS */;
INSERT INTO `Host_Config` (`hostnum`,`application`,`name`,`value`,`timestamp`) VALUES (NULL,'magmad','magma.system.increase_resource_limits','false','2009-12-16 04:32:54'),(NULL,'magmad','magma.storage.default','local','2011-03-24 12:11:22'),(NULL,'magmad','magma.smtp.blacklist','bl.spamcop.net.','2011-04-13 16:03:55');
/*!40000 ALTER TABLE `Host_Config` ENABLE KEYS */;

--
-- Dumping data for table `Keys`
--
-- ORDER BY:  `usernum`

/*!40000 ALTER TABLE `Keys` DISABLE KEYS */;
/*!40000 ALTER TABLE `Keys` ENABLE KEYS */;

--
-- Dumping data for table `Limits`
--
-- ORDER BY:  `plan`

/*!40000 ALTER TABLE `Limits` DISABLE KEYS */;
INSERT INTO `Limits` VALUES ('BASIC','The Lavabit basic account plan.',0,0,0,0,1,1,1,1,30,1,1,'REJECT,DELETE,MARK',1,'REJECT,DELETE,MARK',1,'DELETE,MARK',0,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK','DELETE',0,1,1048576,33554432,1,1048576,33554432,1,8,256,1,8,1024,1,8,1024,1,1),('ENHANCED','The Lavabit enhanced account plan.',0,0,1,1,1,1,1,1,30,1,1,'REJECT,DELETE,MARK',1,'REJECT,DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK','DELETE',1,1,1048576,67108864,1,1048576,67108864,1,8,512,1,8,1024,1,8,1024,1,1),('PERSONAL','The Lavabit personal account plan.',0,0,0,1,1,1,1,1,30,1,1,'REJECT,DELETE,MARK',1,'REJECT,DELETE,MARK',1,'DELETE,MARK',0,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK','DELETE',1,1,1048576,67108864,1,1048576,67108864,1,8,256,1,8,1024,1,8,1024,1,1),('PREMIUM','The Lavabit premium account plan.',0,0,1,1,1,1,1,1,30,1,1,'REJECT,DELETE,MARK',1,'REJECT,DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK','DELETE',1,1,1048576,134217728,1,1048576,134217728,1,8,768,1,8,8196,1,8,8196,1,1), ('STANDARD','The Lavabit standard account plan.',0,1,1,1,1,1,1,1,30,1,1,'REJECT,DELETE,MARK',1,'REJECT,DELETE,MARK',1,'DELETE,MARK',0,'MARK',1,'DELETE,MARK',1,'DELETE,MARK','DELETE',1,1,1048576,67108864,1,1048576,67108864,1,8,128,1,8,8192,1,8,8192,1,1),
('PREMIER','The Lavabit premier account plan.',0,1,1,1,1,1,1,1,30,1,1,'REJECT,DELETE,MARK',1,'REJECT,DELETE,MARK',1,'DELETE,MARK',0,'MARK',1,'DELETE,MARK',1,'DELETE,MARK','DELETE',1,1,1048576,134217728,1,1048576,134217728,1,8,128,1,8,8192,1,8,8192,1,1);
/*!40000 ALTER TABLE `Limits` ENABLE KEYS */;

--
-- Dumping data for table `Locking`
--
-- ORDER BY:  `usernum`

/*!40000 ALTER TABLE `Locking` DISABLE KEYS */;
/*!40000 ALTER TABLE `Locking` ENABLE KEYS */;

--
-- Dumping data for table `Log`
--
-- ORDER BY:  `usernum`

/*!40000 ALTER TABLE `Log` DISABLE KEYS */;
INSERT INTO `Log` VALUES (1,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2011-05-05 14:15:59','0.0.0.0'),(2,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2011-05-05 14:16:03','0.0.0.0'),(3,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2011-05-05 14:16:09','0.0.0.0'),(4,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2011-05-05 14:15:59','127.0.0.1');
/*!40000 ALTER TABLE `Log` ENABLE KEYS */;

--
-- Dumping data for table `Mailboxes`
--
-- ORDER BY:  `address`

/*!40000 ALTER TABLE `Mailboxes` DISABLE KEYS */;
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('magma@lavabit.com',1),('notify@lavabit.com',1),('magma@example.com',1),('princess@lavabit.com',2),('princess@example.com',2),('ladar@mailshack.com',3),('ladar@nerdshack.com',3),('ladar@lavabit.com',3),('ladar@example.com',3),('stacie@lavabit.com',4),('stacie@example.com',4);
/*!40000 ALTER TABLE `Mailboxes` ENABLE KEYS */;

--
-- Dumping data for table `Messages`
--
-- ORDER BY:  `messagenum`

/*!40000 ALTER TABLE `Messages` DISABLE KEYS */;
/*!40000 ALTER TABLE `Messages` ENABLE KEYS */;

--
-- Dumping data for table `Objects`
--
-- ORDER BY:  `objectnum`

/*!40000 ALTER TABLE `Objects` DISABLE KEYS */;
/*!40000 ALTER TABLE `Objects` ENABLE KEYS */;

--
-- Dumping data for table `Patterns`
--
-- ORDER BY:  `pattern`

/*!40000 ALTER TABLE `Patterns` DISABLE KEYS */;
/*!40000 ALTER TABLE `Patterns` ENABLE KEYS */;

--
-- Dumping data for table `Profile`
--
-- ORDER BY:  `usernum`

/*!40000 ALTER TABLE `Profile` DISABLE KEYS */;
INSERT INTO `Profile` VALUES (1,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0),(2,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0),(3,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,0);
/*!40000 ALTER TABLE `Profile` ENABLE KEYS */;

--
-- Dumping data for table `Receiving`
--

/*!40000 ALTER TABLE `Receiving` DISABLE KEYS */;
/*!40000 ALTER TABLE `Receiving` ENABLE KEYS */;

--
-- Dumping data for table `Signatures`
--
-- ORDER BY:  `signum`

/*!40000 ALTER TABLE `Signatures` DISABLE KEYS */;
/*!40000 ALTER TABLE `Signatures` ENABLE KEYS */;

--
-- Dumping data for table `Tracker`
--

/*!40000 ALTER TABLE `Tracker` DISABLE KEYS */;
/*!40000 ALTER TABLE `Tracker` ENABLE KEYS */;

--
-- Dumping data for table `Transmitting`
--

/*!40000 ALTER TABLE `Transmitting` DISABLE KEYS */;
/*!40000 ALTER TABLE `Transmitting` ENABLE KEYS */;

--
-- Dumping data for table `Users`
--
-- ORDER BY:  `usernum`

/*!40000 ALTER TABLE `Users` DISABLE KEYS */;
INSERT INTO `Users` VALUES (1,'magma',NULL, NULL,0,'42a7ead6550ee52360222a0e8783645216c3bb4ade95ab531be13cd3060dcdd085fb534e424e067bc4edfe27a87277daaad6428fee737b3adc3c55608fb23ea5',0,'STANDARD',0,0,NULL,1,1,0,0,4294967296,0,DATE_ADD(NOW(), INTERVAL 3 YEAR),'0000-00-00'),(2,'princess', NULL, NULL,0,'3a5799cbc019beebd5e779a62343a76ff949c829b792ebe2c2fae406eda57268ce200db1ada838936b0ac6804115d60e83e88189705bd3f52f1723f29ce9cfa1',0,'BASIC',0,0,NULL,1,1,0,0,4294967296,0,'0000-00-00','0000-00-00'),(3,'ladar', NULL, NULL,0,'eb5ff977cd0ef6487677b4961995088d3a86caa2e6710829a28047017406e493b6839fbdf69d2e4ee290fee1181ba1a4c105afe7c507e91e5773d71d0461adba',0,'BASIC',0,0,NULL,1,1,0,0,4294967296,0,'0000-00-00','0000-00-00'),(4,'stacie','MOgx1F13HmoSGt05L2AvYwjWVqS_NmEU1b6eaWE9EOb819su6Z2qUvxdsQyx1CCL_xlCYhh2OJhaoxN0UlUIjvZ-yz08TBaWZ7Z0B3ZNrBtTs3OOio4K7pMkDLpXxCjjS2eboU7nNxn1sdrgKLICOZSWtPIDJmAAIyr9GOPF-x4','MAtbJr6lIPOmrYuQMQaPzDq8mNRN8qp9MefYk8vyxnL3DrsuzFeSMhGL5Ew4tDTYA1hNzqroJaoGB8jWpUKAwA',0,NULL,0,'BASIC',0,0,NULL,1,1,0,0,4294967296,0,'0000-00-00','0000-00-00');
/*!40000 ALTER TABLE `Users` ENABLE KEYS */;

--
-- Dumping data for table `Codes`
--

/*!40000 ALTER TABLE `Codes` DISABLE KEYS */;
INSERT INTO `Codes` (`codeid`, `promo`, `email`, `origin`, `plan`, `years`, `usernum`, `redeemed`, `created`) VALUES (1,'A',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(2,'B',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(3,'C',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(4,'D',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(5,'E',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(6,'F',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(7,'G',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(8,'H',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(9,'I',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(10,'J',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(11,'K',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(12,'L',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(13,'M',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(14,'N',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(15,'O',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(16,'P',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(17,'Q',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(18,'R',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(19,'S',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(20,'T',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(21,'U',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(22,'V',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(23,'W',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(24,'X',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(25,'Y',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(26,'Z',NULL,'MANUAL','STANDARD',1,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(27,'AA',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(28,'BB',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(29,'CC',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(30,'DD',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(31,'EE',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(32,'FF',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(33,'GG',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(34,'HH',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(35,'II',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(36,'JJ',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(37,'KK',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(38,'LL',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(39,'MM',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(40,'NN',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(41,'OO',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(42,'PP',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(43,'QQ',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(44,'RR',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(45,'SS',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(46,'TT',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(47,'UU',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(48,'VV',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(49,'WW',NULL,'MANUAL','PREMIER',2,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(50,'XX',NULL,'MANUAL','PREMIER',10,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(51,'YY',NULL,'MANUAL','PREMIER',10,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36'),(52,'ZZ',NULL,'MANUAL','PREMIER',10,NULL,'0000-00-00 00:00:00','2019-05-27 08:49:36');
/*!40000 ALTER TABLE `Codes` ENABLE KEYS */;

--
-- The User Accounts Needed to Test Account Locking
--


INSERT INTO `Users` (`usernum`, `userid`, `salt`, `auth`, `bonus`, `legacy`, `tls`, `plan`, `locked`, `advertising`, `domain`, `email`, `chat`, `timezone`, `size`, `quota`, `overquota`, `plan_expiration`, `lock_expiration`) VALUES (5,'lock_inactive','Ll_eP0RI3pDqgknj8P4jplvIMGQc88BlWptzk62Ch4NFcBTPnoJNze-6aaYj6qtzew7dfe4huqy5hFM0SOJ74PBy4GsDlq4Dqy9UGNtyE_6C8nFOaJ7i4MB0huspNZgjwiVlCT_6se6PgDAEPe_HCB-d9_oFf5kbdVHPhIqcqCI','Pmt5yDl8AR4hleNKYIxDP-GCUdaTpDrMTMFRMbAO5rzuslCgQMPLpz6i-p0eL2EUMet_6evp1NvCZXYxoMLGhw',0,NULL,1,'STANDARD',1,0,NULL,1,1,0,0,5368709120,0,'2018-08-06','2018-12-04');
INSERT INTO `Users` (`usernum`, `userid`, `salt`, `auth`, `bonus`, `legacy`, `tls`, `plan`, `locked`, `advertising`, `domain`, `email`, `chat`, `timezone`, `size`, `quota`, `overquota`, `plan_expiration`, `lock_expiration`) VALUES (6,'lock_expired','8Y1MjRJO9C__q9QJPzQfV74K7txvuMlVGhjZUDECzeIYZLQNLxNJLAQOPfyrkSFa1Mdo48D0tE1Q0sQRnk-EImW1lfOjAdM4U4MyTXk8kIVU7JIVrt9MeF8Eyf8VFHQZVXhVRQULHw4uAppMIMeeZ9scwMCRH6ZH3wn8sdeRKrQ','AbxZf04xdLxNhHuH40Z8Zsu3QWwrQWNQdojdoZnjzXn3tS3Yt8lrtm-MM_vr5tY-C-1ksdQOVZ8oRpdRdEif7A',0,NULL,1,'STANDARD',2,0,NULL,1,1,0,0,5368709120,0,'2018-08-06','2018-12-04');
INSERT INTO `Users` (`usernum`, `userid`, `salt`, `auth`, `bonus`, `legacy`, `tls`, `plan`, `locked`, `advertising`, `domain`, `email`, `chat`, `timezone`, `size`, `quota`, `overquota`, `plan_expiration`, `lock_expiration`) VALUES (7,'lock_admin','y7iMohaZRNIZiPBhAwaaetmX_1B1tE42JS84GxnCTKLPLA4ypo13jGZojjAt8gcSIEahtHWgb9VrINBsSL4eASD__k6GPWB2kWkHMTg5PcKo86KU4KEFZa35eOQ1CW5J3aZoa_axUc2vNDy8taCmJUIYCCk9NWw-o6LSREt1oaU','bSH7E1cykKDoYSN2XdtZb7BGSHUVBDQkmkvzYeFbfKdvCDPc90CGMMSFSI-hjRuV_TXl_FB_cbwcH0dWaZl2hg',0,NULL,1,'STANDARD',3,0,NULL,1,1,0,0,5368709120,0,'2018-08-06','2018-12-04');
INSERT INTO `Users` (`usernum`, `userid`, `salt`, `auth`, `bonus`, `legacy`, `tls`, `plan`, `locked`, `advertising`, `domain`, `email`, `chat`, `timezone`, `size`, `quota`, `overquota`, `plan_expiration`, `lock_expiration`) VALUES (8,'lock_abuse','SoRKkslJPq8vrux5nEf4g16hrZwv4_5IHVnC6Pj6rPBshD4Q44DAS8eyxc90P2o-seViuc7cCJ4kJ4T9VJat-t6vmZZdFyzbzOzZAb-SajH4z7gKqDB-uSO35EgEXam4jLT5IGjpP5eybKPoSt9IHUDThvAPWiOMJNAYlYRPbNg','oc7DNB5kzIpRmi91dak-_kaksAIZQc1nTGUb3cWB7JLaT8pGjXHYRwfCNJA7nNMOhMPT2nVIPh0e9VEoAqsBag',0,NULL,1,'PREMIER',4,0,NULL,1,1,0,0,5368709120,0,'2018-08-06','2018-12-04');
INSERT INTO `Users` (`usernum`, `userid`, `salt`, `auth`, `bonus`, `legacy`, `tls`, `plan`, `locked`, `advertising`, `domain`, `email`, `chat`, `timezone`, `size`, `quota`, `overquota`, `plan_expiration`, `lock_expiration`) VALUES (9,'lock_user','xeyToHadHNNept5xy-yV5IR777gBFx4WGbiPPnbzE3iIJni65hUBhzLOohJHiwhS3kXzHiCCrZKsja036L2-6VZYpwR6f7Q-yQ8V0tQik69W5rKAfhGKS1v27pCNm8gbHkRtNOJZk0fmhpi4OqN3WDT517bUaSujGm_HBw7GFk8','O4sR-aNx1hX381WqXvWp49RiEW6Fg69Pc1eQ0-A0xeZFV05nyQ-88ap0FDxoGuZlN7ipqDivS22-3_1YP53R3A',0,NULL,1,'PREMIER',5,0,NULL,1,1,0,0,5368709120,0,'2018-08-06','2018-12-04');
INSERT INTO `Users` (`usernum`, `userid`, `salt`, `auth`, `bonus`, `legacy`, `tls`, `plan`, `locked`, `advertising`, `domain`, `email`, `chat`, `timezone`, `size`, `quota`, `overquota`, `plan_expiration`, `lock_expiration`) VALUES (10,'lock_none','kjyZp26SQZjU8OM0xcUTWhyKUl15_stxHeqMpjeFdzMx02Fo8XIVNO5Q8Ah82XSW0XetQ4oEZTik9wyE4SMw02m5lIQGv0HyTDs66o1AjRghWySmuZFFotzJBI_jmx1d66eNNa3HjJENslZJJzmeGBn-uHjT8h_PTlFseNx7EBc','3R38D_2Nucvc7bvTKgAjxr5rV5eawSh9N4sETiqo0EqphM95pRKOFm6xdBxNgxKdfxnSHsfLsiHA1Hc1u_kgyg',0,NULL,1,'STANDARD',0,0,NULL,1,1,0,0,5368709120,0,'2018-08-06','0000-00-00');

INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_inactive@lavabit.com',5);
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_inactive@example.com',5);
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_expired@lavabit.com',6);
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_expired@example.com',6);
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_admin@lavabit.com',7);
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_admin@example.com',7);
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_abuse@lavabit.com',8);
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_abuse@example.com',8);
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_user@lavabit.com',9);
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_user@example.com',9);
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_none@lavabit.com',10);
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('lock_none@example.com',10);

INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `type`, `parent`) VALUES (5,5,'Inbox',0,1,0);
INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `type`, `parent`) VALUES (6,6,'Inbox',0,1,0);
INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `type`, `parent`) VALUES (7,7,'Inbox',0,1,0);
INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `type`, `parent`) VALUES (8,8,'Inbox',0,1,0);
INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `type`, `parent`) VALUES (9,9,'Inbox',0,1,0);
INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `type`, `parent`) VALUES (10,10,'Inbox',0,1,0);

INSERT INTO `Dispatch` (`usernum`, `secure`, `forwarded`, `rollout`, `bounces`, `greylist`, `greytime`, `rbl`, `rblaction`, `spf`, `spfaction`, `dkim`, `dkimaction`, `spam`, `spamaction`, `spamfolder`, `virus`, `virusaction`, `phish`, `phishaction`, `filters`, `autoreply`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`, `class`) VALUES (5,1,'',0,1,0,1,0,'REJECT',0,'REJECT',0,'MARK',0,'MARK',5,1,'DELETE',1,'DELETE',0,NULL,5,67108864,67108864,128,8192,8192,0);
INSERT INTO `Dispatch` (`usernum`, `secure`, `forwarded`, `rollout`, `bounces`, `greylist`, `greytime`, `rbl`, `rblaction`, `spf`, `spfaction`, `dkim`, `dkimaction`, `spam`, `spamaction`, `spamfolder`, `virus`, `virusaction`, `phish`, `phishaction`, `filters`, `autoreply`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`, `class`) VALUES (6,1,'',0,1,0,1,0,'REJECT',0,'REJECT',0,'MARK',0,'MARK',6,1,'DELETE',1,'DELETE',0,NULL,6,67108864,67108864,128,8192,8192,0);
INSERT INTO `Dispatch` (`usernum`, `secure`, `forwarded`, `rollout`, `bounces`, `greylist`, `greytime`, `rbl`, `rblaction`, `spf`, `spfaction`, `dkim`, `dkimaction`, `spam`, `spamaction`, `spamfolder`, `virus`, `virusaction`, `phish`, `phishaction`, `filters`, `autoreply`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`, `class`) VALUES (7,1,'',0,1,0,1,0,'REJECT',0,'REJECT',0,'MARK',0,'MARK',7,1,'DELETE',1,'DELETE',0,NULL,7,67108864,67108864,128,8192,8192,0);
INSERT INTO `Dispatch` (`usernum`, `secure`, `forwarded`, `rollout`, `bounces`, `greylist`, `greytime`, `rbl`, `rblaction`, `spf`, `spfaction`, `dkim`, `dkimaction`, `spam`, `spamaction`, `spamfolder`, `virus`, `virusaction`, `phish`, `phishaction`, `filters`, `autoreply`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`, `class`) VALUES (8,1,'',0,1,0,1,0,'REJECT',0,'REJECT',0,'MARK',0,'MARK',8,1,'DELETE',1,'DELETE',0,NULL,8,67108864,67108864,128,8192,8192,0);
INSERT INTO `Dispatch` (`usernum`, `secure`, `forwarded`, `rollout`, `bounces`, `greylist`, `greytime`, `rbl`, `rblaction`, `spf`, `spfaction`, `dkim`, `dkimaction`, `spam`, `spamaction`, `spamfolder`, `virus`, `virusaction`, `phish`, `phishaction`, `filters`, `autoreply`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`, `class`) VALUES (9,1,'',0,1,0,1,0,'REJECT',0,'REJECT',0,'MARK',0,'MARK',9,1,'DELETE',1,'DELETE',0,NULL,9,67108864,67108864,128,8192,8192,0);
INSERT INTO `Dispatch` (`usernum`, `secure`, `forwarded`, `rollout`, `bounces`, `greylist`, `greytime`, `rbl`, `rblaction`, `spf`, `spfaction`, `dkim`, `dkimaction`, `spam`, `spamaction`, `spamfolder`, `virus`, `virusaction`, `phish`, `phishaction`, `filters`, `autoreply`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`, `class`) VALUES (10,1,'',0,1,0,1,0,'REJECT',0,'REJECT',0,'MARK',0,'MARK',10,1,'DELETE',1,'DELETE',0,NULL,10,67108864,67108864,128,8192,8192,0);

INSERT INTO `Log` (`usernum`, `lastpop`, `lastmap`, `lastweb`, `lastchat`, `lastsent`, `lastreceived`, `popsessions`, `mapsessions`, `websessions`, `chatsessions`, `totalsent`, `totalreceived`, `totalbounces`, `created`, `created_ip`) VALUES (5,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2018-08-06 21:51:57','127.0.0.1');
INSERT INTO `Log` (`usernum`, `lastpop`, `lastmap`, `lastweb`, `lastchat`, `lastsent`, `lastreceived`, `popsessions`, `mapsessions`, `websessions`, `chatsessions`, `totalsent`, `totalreceived`, `totalbounces`, `created`, `created_ip`) VALUES (6,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2018-08-06 21:51:57','127.0.0.1');
INSERT INTO `Log` (`usernum`, `lastpop`, `lastmap`, `lastweb`, `lastchat`, `lastsent`, `lastreceived`, `popsessions`, `mapsessions`, `websessions`, `chatsessions`, `totalsent`, `totalreceived`, `totalbounces`, `created`, `created_ip`) VALUES (7,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2018-08-06 21:51:57','127.0.0.1');
INSERT INTO `Log` (`usernum`, `lastpop`, `lastmap`, `lastweb`, `lastchat`, `lastsent`, `lastreceived`, `popsessions`, `mapsessions`, `websessions`, `chatsessions`, `totalsent`, `totalreceived`, `totalbounces`, `created`, `created_ip`) VALUES (8,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2018-08-06 21:51:57','127.0.0.1');
INSERT INTO `Log` (`usernum`, `lastpop`, `lastmap`, `lastweb`, `lastchat`, `lastsent`, `lastreceived`, `popsessions`, `mapsessions`, `websessions`, `chatsessions`, `totalsent`, `totalreceived`, `totalbounces`, `created`, `created_ip`) VALUES (9,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2018-08-06 21:51:57','127.0.0.1');
INSERT INTO `Log` (`usernum`, `lastpop`, `lastmap`, `lastweb`, `lastchat`, `lastsent`, `lastreceived`, `popsessions`, `mapsessions`, `websessions`, `chatsessions`, `totalsent`, `totalreceived`, `totalbounces`, `created`, `created_ip`) VALUES (10,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2018-08-06 21:51:57','127.0.0.1');

INSERT INTO `Profile` (`usernum`, `name`, `address_one`, `address_two`, `city`, `state`, `zip`, `country`, `profile`, `phone`, `mobile`, `fax`, `gender`, `language`, `birthdate`, `industry`, `website`, `accessible`) VALUES (5,'','','','','',0,'',NULL,'','','',NULL,'','0000-00-00','','',0);
INSERT INTO `Profile` (`usernum`, `name`, `address_one`, `address_two`, `city`, `state`, `zip`, `country`, `profile`, `phone`, `mobile`, `fax`, `gender`, `language`, `birthdate`, `industry`, `website`, `accessible`) VALUES (6,'','','','','',0,'',NULL,'','','',NULL,'','0000-00-00','','',0);
INSERT INTO `Profile` (`usernum`, `name`, `address_one`, `address_two`, `city`, `state`, `zip`, `country`, `profile`, `phone`, `mobile`, `fax`, `gender`, `language`, `birthdate`, `industry`, `website`, `accessible`) VALUES (7,'','','','','',0,'',NULL,'','','',NULL,'','0000-00-00','','',0);
INSERT INTO `Profile` (`usernum`, `name`, `address_one`, `address_two`, `city`, `state`, `zip`, `country`, `profile`, `phone`, `mobile`, `fax`, `gender`, `language`, `birthdate`, `industry`, `website`, `accessible`) VALUES (8,'','','','','',0,'',NULL,'','','',NULL,'','0000-00-00','','',0);
INSERT INTO `Profile` (`usernum`, `name`, `address_one`, `address_two`, `city`, `state`, `zip`, `country`, `profile`, `phone`, `mobile`, `fax`, `gender`, `language`, `birthdate`, `industry`, `website`, `accessible`) VALUES (9,'','','','','',0,'',NULL,'','','',NULL,'','0000-00-00','','',0);
INSERT INTO `Profile` (`usernum`, `name`, `address_one`, `address_two`, `city`, `state`, `zip`, `country`, `profile`, `phone`, `mobile`, `fax`, `gender`, `language`, `birthdate`, `industry`, `website`, `accessible`) VALUES (10,'','','','','',0,'',NULL,'','','',NULL,'','0000-00-00','','',0);

INSERT INTO `Realms` (`usernum`, `serial`, `label`, `shard`, `rotated`) VALUES (5,0,'mail','J7mUSSOGSL12qPUxFhyU50fu5ba-1hhGKRxW0EWFwRe11137-R3_oN55ashjtBxCySfcnLSYkJYGuuD_rTvxqQ',1);
INSERT INTO `Realms` (`usernum`, `serial`, `label`, `shard`, `rotated`) VALUES (6,0,'mail','yoU9p4UIJOy6uEyCu5xDsMd2JSl5S0zcaRfA795TdcFHe7nOspx0d7XS-ojdzdgEfM2NxnQjZj6vukWj4C26cg',1);
INSERT INTO `Realms` (`usernum`, `serial`, `label`, `shard`, `rotated`) VALUES (7,0,'mail','XiKCY33BnwlfXrwGOHHbnMGrK_bgWeVJ_vH1O3kJJg99alytEP71vE8RdiL1f020MbddqVSFGVVkUH-DhHK7cQ',1);
INSERT INTO `Realms` (`usernum`, `serial`, `label`, `shard`, `rotated`) VALUES (8,0,'mail','q2Zg_FtCXd-HOf6J4npsRFMuDqaPzqL3DlRthmvU7CDSpvMfeWLmAk48OxmlIrvpqtDBbYH_VEVzHtgSFPnLIg',1);
INSERT INTO `Realms` (`usernum`, `serial`, `label`, `shard`, `rotated`) VALUES (9,0,'mail','s-sRVHTkJ-Fb0hiMy67SZ8b7TjZj9vRcnN4TMFZnZ144Ymaexz5V-J9-P-rzwXz31gkefoRL5Aso5A2FzwhLfQ',1);
INSERT INTO `Realms` (`usernum`, `serial`, `label`, `shard`, `rotated`) VALUES (10,0,'mail','J-SNFdBrDZOdbQAWKmqOpMto2qzJ7OBwm5LjEyUVl_Hv0-hpifQOZbmK2fF2BlZlsk5v-zvRSx_waChcOeDwHg',1);

INSERT INTO `Keys` (`usernum`, `signet`, `key`) VALUES (5,'Bv0AAMcBIGjvYcdQfpPvkH87Ba5S_UrbANRKylQN_PbCu1__7mucAiECIlHjcxYlWwpKqG1rLChG_VfnLLNiw96w2U9D7LqE2TgFpK30lThNAhc0CXPlz52ojBrEHunqLOS0UYkmdcpV7ghy8ITNzeAuRD5JJc_35-ed57_Nc0G6PIajnX604ibODga94HB4x_Q9qW4AVn1NmVe9xtFaQ3DHY5tVorv37Hv5I97vW_i_JI7mBC4gq5xLqXzg0d7SvOPC1GBhreqaBLsG','B7gAAABwEbYYgGZ2KWZccjQnbAuKsAFV-BdFOES7nQEda_WBhFRQ4jOqq7QDNB2jLcVCrHDcUIQk05PhHmfUJRvSiTRkDd50jZ3EIb3r2ka26RdXHy851fGLFz6Bu488uArUcsXk205G3mPiNZNlDnoVSSeIbg');
INSERT INTO `Keys` (`usernum`, `signet`, `key`) VALUES (6,'Bv0AAMcBIMgKxxDI9j63snbL6BFc8vtnD4mWqX78CUYVqJdLUbCJAiED_qdPu8l7Exp38OgQPu5Nm4LX3aowPiP9DksEtfLRFScFuVKSGjHjM2y4DlctLgAMU_leBU6HrkW5cK5vMUNNzowTyUQjmt1Bg2SNne0RxaqY16_5v5p12jv17q8pHBvTBAa5bRTwiB9sPaJi8NYseS6tfF2EX0ZBAsKIth22EFu28mxKm-Y0muwD7_xk-48HjIP2XxpDBKGMBQpAl9Ub13cD','B7gAAABww4bGGnpnRfFspmSK85VeWddBe2-j2Y7F5A4ZV-yjbxWAnshItRBIup2Jx_hjwVjQOGC6RkLTDKXQgxiUaDLMchRU3YmWVIU9jrGZfrU5LPmaU2q2OXh3BFPv1pqM45pu9EV9QLecEsMg7I8hdF3BCA');
INSERT INTO `Keys` (`usernum`, `signet`, `key`) VALUES (7,'Bv0AAMcBIBUB_xhqusTb6rh-DnCYgTAISgxJwUJWwKLcaHiVhW2CAiEDwWdycQTuNBOYn10eGfCiAzBJ_nfBy-xL23A40gVhXRYFZaJ2FLqrxS8gVmD5Mj2oOV5bo6zwq8mTeDEUHhll_DIMDnSYSUPoo9GuTC92-AC3J6rieOgU1Hn2rTFA-ayGDAaIMheKwkBIABEnPrOCwRmjxLWOyo4p5-iG7rtnm9Z45hO6kHssv_wIaSN2lMOU-l5CxiSAruYA8b8t3LpxepkI','B7gAAABwzfMWIZdldDygMyp9vanDeSUHX9u7u5d3lc3YMAzXbuUczBRcLbltazaQ6yJ84tIyZ8lXKqaEE0Aybe34kmOdGdS0rOpQsau7qh04-WjuqXDcVs7Vvq6LOzOqsVMJfm9MaBdWrDF1iCylZ-bhLY5uSg');
INSERT INTO `Keys` (`usernum`, `signet`, `key`) VALUES (8,'Bv0AAMcBIBPzMUMtpD4ljHMNHFe5ErI3dFOOLgBDVgE2qAzqZ7zhAiEDWispbjAcBBqWRbytkVs8QRkczm9dj-JCXvyjWgD8BGcFI8mloo5VenKRKzUrqeKT4un66Oj5oITrTKqzYmdykC53vw_7vQu1HniycTTEKtGmaRy85j-grNBf4IEKCo3TDgZTYM3DlA_0-1VIZslx4bHIgB5dW1DnwK8V9WxmHJXs2tqzAebT39-iF_LstRrzXl31y_jEPC2sNqXDA6DnTxUF','B7gAAABw2L82NfcMDfohoosHwm1HTH-frSPR0mQG-4_p7o8MeWxJtmWclOQT66vdQMm1zttNQU9bz4LztFL38YrhFyNW1KMa1-S9lN6zOmNp_YcEjTcWIM4Xlo5HJ9UGkSc6uOyMWWI5EyG0_WmIemNq-KVdaQ');
INSERT INTO `Keys` (`usernum`, `signet`, `key`) VALUES (9,'Bv0AAMcBIFOpc1aSEwxEEC2Vej9iuFyuY9ukebtBrHNPwB21wRZeAiECqrOPSNCf-gG8eDSWSjlNnz-h8tllvOHX1aRpeMeX3cIF-xmtPFLFBuCWEHgaH2dOendbCLV5urjMJ9jQx8dMNMtek12sV6iZUU3y293vJjkFBdhBwJh-rWODSrds_acaCgYY2PQJ9aT5UP1ckb7E2YQbOuxB_cQdMJLv1y2KO9miutpS3fh73bH-CZhw1uFSz1i9gRnI6FBc5tGOWmHNZjME','B7gAAABwwXHGYntnrZh7CEbQVjKY_8rlpCW-7-VeudSWd8oiWc0e4VbKgMD9pfGy2eR7ZgpqqfUkcNG757el2wYEKCkwug4ukoTryJioZbngPtFRuYfgPXTLYMtkWKOjq3t6AZrBDuRpBV6WGYO2HbMH7XaLmA');
INSERT INTO `Keys` (`usernum`, `signet`, `key`) VALUES (10,'Bv0AAMcBIFRTWQk5ujJLTbLM1hDDIQksos-DkcnKnRXRF2u1N91LAiEC7FjFR8HM-F73_8Vuczq2dPCxcGLjQv6JHOLHpRuRUX8FK4R52zgWIXh2DOU7eL3jlez7LxGJN_wEiA4j0JwEgXWDqSI3U8IdXfeX0twz-lQLphqdxReu_eQCDW1GavFpAAY-6IWrOCAcU0xMuGhf85RfemNo2UNT8h_oIkTKPVfk8QISXmkC9AZK8lw-9dvViawPME58t9d9yFaBNg0iCzkJ','B7gAAABwc17c0xKII99ZsXlbn0beMsl9w0wOXX7MJbuRgrSvrvJY3L5KGee-dOTPMtUhmVHFOPZqSXw5a0bKa2ueVySQ8S_LyTLLDOSSaWgLjaX2knSpRHHhrq61OJAxDy7bCAhq2ERfJxMwtdUzQKbCd4QhFQ');

