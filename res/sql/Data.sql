
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
INSERT INTO `Mailboxes` (`address`, `usernum`) VALUES ('magma@lavabit.com',1),('magma@example.com',1),('princess@lavabit.com',2),('princess@example.com',2),('ladar@mailshack.com',3),('ladar@nerdshack.com',3),('ladar@lavabit.com',3),('ladar@example.com',3),('stacie@lavabit.com',4),('stacie@example.com',4);
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
INSERT INTO `Codes` VALUES (1,'A',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(2,'B',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(3,'C',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(4,'D',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(5,'E',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(6,'F',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(7,'G',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(8,'H',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(9,'I',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(10,'J',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(11,'K',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(12,'L',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(13,'M',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(14,'N',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(15,'O',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(16,'P',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(17,'Q',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(18,'R',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(19,'S',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(20,'T',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(21,'U',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(22,'V',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(23,'W',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(24,'X',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(25,'Y',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(26,'Z',NULL,'MANUAL','STANDARD',NULL,'0000-00-00 00:00:00',NOW()),(27,'AA',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(28,'BB',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(29,'CC',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(30,'DD',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(31,'EE',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(32,'FF',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(33,'GG',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(34,'HH',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(35,'II',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(36,'JJ',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(37,'KK',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(38,'LL',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(39,'MM',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(40,'NN',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(41,'OO',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(42,'PP',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(43,'QQ',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(44,'RR',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(45,'SS',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(46,'TT',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(47,'UU',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(48,'VV',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(49,'WW',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(50,'XX',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(51,'YY',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW()),(52,'ZZ',NULL,'MANUAL','PREMIER',NULL,'0000-00-00 00:00:00',NOW());
/*!40000 ALTER TABLE `Codes` ENABLE KEYS */;

--
-- The User Accounts Needed to Test Account Locking
--

INSERT INTO `Users` (`usernum`, `userid`, `salt`, `auth`, `bonus`, `legacy`, `tls`, `plan`, `locked`, `advertising`, `domain`, `email`, `chat`, `timezone`, `size`, `quota`, `overquota`, `plan_expiration`, `lock_expiration`) VALUES (5,'lock_inactive','JDIjkrTqVJi1a4UQeJdxpak6Z23_mA1UXLeMBQ-0vT0OVztYpXL4UKscyF-CZzWbrHLNFAsBjWWPkZyXchNythFrpJ051tdtjapKCQ0lb_dLDTeU8OxMj_E5RcnVU9ewzslBwwXSgGr2DUB4EJ2ELqPwMpddJYDg1jnFTabXPIw','G1_5u-jdaY4V8R4rNPtCfXNT7oGRcXdy3Z5KgPgXkGDiscu-kMetqLX2_J2MUe3PmCY7Uklyz6b3GNzZhzXtzg',0,NULL,0,'STANDARD',1,1,NULL,1,1,0,0,134217728,0,DATE(NOW()),DATE_ADD(DATE(NOW()),INTERVAL 120 DAY));
INSERT INTO `Users` (`usernum`, `userid`, `salt`, `auth`, `bonus`, `legacy`, `tls`, `plan`, `locked`, `advertising`, `domain`, `email`, `chat`, `timezone`, `size`, `quota`, `overquota`, `plan_expiration`, `lock_expiration`) VALUES (6,'lock_expired','MZ5-PVVxojRkV6HSnb-GOY5_hQbH-uH4M8COwrhNs7ozOtqThR8XOxzNLU7RnTGDoCjzzAmCcBvpz2gy3n3x9qSEpFpB60p33xEwPlJcuFLGeTj3976z79MDu_nGb50P4Y2DNqM0I9-UW-hKq45uvPvOJc2yAzChm4ut2EIlPQM','OI7fmRBNBwhGaM4e4vwTRBNEnXw6Amd2DShn7GzMkr34s7tGE46uGjzQq4JF4vSHGMtRDWcogDxu685MtGywbg',0,NULL,0,'STANDARD',2,1,NULL,1,1,0,0,134217728,0,DATE(NOW()),DATE_ADD(DATE(NOW()),INTERVAL 120 DAY));
INSERT INTO `Users` (`usernum`, `userid`, `salt`, `auth`, `bonus`, `legacy`, `tls`, `plan`, `locked`, `advertising`, `domain`, `email`, `chat`, `timezone`, `size`, `quota`, `overquota`, `plan_expiration`, `lock_expiration`) VALUES (7,'lock_admin','Lw0ba4RDXRkamDBDFWgvRif_EQ7GYaBR7c0ALg12EkFW-MNDOHuAf3SVGIRqJpXLqEnhDEHtO1zpoA49oW46nDU9CwBZjPoPKKDOUKJ2ziGp6iGCTRaaK8xTcjqUGpKxELV59QMcJe7JglaKqANOSiKSc0Oyhd1jSFIUmeWQLfg','rORS460yz49n9KqgD1CvkCDXKXqbXBNKmm8OeQ3HknqGbeXHpEnvTP_wvgVM3kUxkl8jO0G4jiMRTCgWFemGKw',0,NULL,0,'STANDARD',3,1,NULL,1,1,0,0,134217728,0,DATE(NOW()),DATE_ADD(DATE(NOW()),INTERVAL 120 DAY));
INSERT INTO `Users` (`usernum`, `userid`, `salt`, `auth`, `bonus`, `legacy`, `tls`, `plan`, `locked`, `advertising`, `domain`, `email`, `chat`, `timezone`, `size`, `quota`, `overquota`, `plan_expiration`, `lock_expiration`) VALUES (8,'lock_abuse','JQ9l3rf3V78HjBgTRbmLFGt0ez-uifQoDjUQEnTq02CaM3by3iwIJNd8VMks6J2c1R3DDpbYclUdLV7NviBw9u3bUMdVE1yThhYWuCIslcvNd5v7GXdIJFRretWpgsumpBCBHtjubpnaoHumlkWG-BXrcTafI60DEd9_YRfUoG0','oOh6Ti-oIr2aveFofBwoXYBBYWDcG4e1lzS44g43vBskmcoazItOVlwPjV9LCrraOqZhUN-R2T12W8lpnw0mEg',0,NULL,0,'PREMIER',4,1,NULL,1,1,0,0,134217728,0,DATE(NOW()),DATE_ADD(DATE(NOW()),INTERVAL 120 DAY));
INSERT INTO `Users` (`usernum`, `userid`, `salt`, `auth`, `bonus`, `legacy`, `tls`, `plan`, `locked`, `advertising`, `domain`, `email`, `chat`, `timezone`, `size`, `quota`, `overquota`, `plan_expiration`, `lock_expiration`) VALUES (9,'lock_user','8z7c-S7Jh_dfWhEe66rQ9_9JjnhJZCHfwnRYTcTK2ix-Ma23Jy68wU87-xsuZnwjEd0KeA8wCmhnBbcuVnEZyxqi1AgMTi_wwXs1pfkEdSwwiUoaAWHeXiPR00_79g2pBvAaKzFdQDxYiTqrTwbndgmO9vXXEkB47Ef6o2_RVEQ','F86hxmgFnGDksS4JtQ_dwt2C3R6d6spmLMBVn3aTlD2dzeEP0LYQmaS6fuwVEF119j6W5jhoFmH1iaBZGbaDbw',0,NULL,0,'PREMIER',5,1,NULL,1,1,0,0,134217728,0,DATE(NOW()),DATE_ADD(DATE(NOW()),INTERVAL 120 DAY));

INSERT INTO `Dispatch` (`usernum`, `secure`, `forwarded`, `rollout`, `bounces`, `greylist`, `greytime`, `rbl`, `rblaction`, `spf`, `spfaction`, `dkim`, `dkimaction`, `spam`, `spamaction`, `spamfolder`, `virus`, `virusaction`, `phish`, `phishaction`, `filters`, `autoreply`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`, `class`) VALUES (5,1,'',0,1,0,1,0,'REJECT',0,'REJECT',0,'MARK',0,'MARK',5,1,'DELETE',1,'DELETE',0,NULL,5,33554432,33554432,256,1024,1024,0);
INSERT INTO `Dispatch` (`usernum`, `secure`, `forwarded`, `rollout`, `bounces`, `greylist`, `greytime`, `rbl`, `rblaction`, `spf`, `spfaction`, `dkim`, `dkimaction`, `spam`, `spamaction`, `spamfolder`, `virus`, `virusaction`, `phish`, `phishaction`, `filters`, `autoreply`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`, `class`) VALUES (6,1,'',0,1,0,1,0,'REJECT',0,'REJECT',0,'MARK',0,'MARK',6,1,'DELETE',1,'DELETE',0,NULL,6,33554432,33554432,256,1024,1024,0);
INSERT INTO `Dispatch` (`usernum`, `secure`, `forwarded`, `rollout`, `bounces`, `greylist`, `greytime`, `rbl`, `rblaction`, `spf`, `spfaction`, `dkim`, `dkimaction`, `spam`, `spamaction`, `spamfolder`, `virus`, `virusaction`, `phish`, `phishaction`, `filters`, `autoreply`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`, `class`) VALUES (7,1,'',0,1,0,1,0,'REJECT',0,'REJECT',0,'MARK',0,'MARK',7,1,'DELETE',1,'DELETE',0,NULL,7,33554432,33554432,256,1024,1024,0);
INSERT INTO `Dispatch` (`usernum`, `secure`, `forwarded`, `rollout`, `bounces`, `greylist`, `greytime`, `rbl`, `rblaction`, `spf`, `spfaction`, `dkim`, `dkimaction`, `spam`, `spamaction`, `spamfolder`, `virus`, `virusaction`, `phish`, `phishaction`, `filters`, `autoreply`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`, `class`) VALUES (8,1,'',0,1,0,1,0,'REJECT',0,'REJECT',0,'MARK',0,'MARK',8,1,'DELETE',1,'DELETE',0,NULL,8,33554432,33554432,256,1024,1024,0);
INSERT INTO `Dispatch` (`usernum`, `secure`, `forwarded`, `rollout`, `bounces`, `greylist`, `greytime`, `rbl`, `rblaction`, `spf`, `spfaction`, `dkim`, `dkimaction`, `spam`, `spamaction`, `spamfolder`, `virus`, `virusaction`, `phish`, `phishaction`, `filters`, `autoreply`, `inbox`, `send_size_limit`, `recv_size_limit`, `daily_send_limit`, `daily_recv_limit`, `daily_recv_limit_ip`, `class`) VALUES (9,1,'',0,1,0,1,0,'REJECT',0,'REJECT',0,'MARK',0,'MARK',9,1,'DELETE',1,'DELETE',0,NULL,9,33554432,33554432,256,1024,1024,0);

INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `type`, `parent`) VALUES (5,5,'Inbox',0,1,0);
INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `type`, `parent`) VALUES (6,6,'Inbox',0,1,0);
INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `type`, `parent`) VALUES (7,7,'Inbox',0,1,0);
INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `type`, `parent`) VALUES (8,8,'Inbox',0,1,0);
INSERT INTO `Folders` (`foldernum`, `usernum`, `foldername`, `order`, `type`, `parent`) VALUES (9,9,'Inbox',0,1,0);

INSERT INTO `Keys` (`usernum`, `signet`, `key`) VALUES (5,'Bv0AAMcBICcoIIlYlOHp6VdaGEvEXN-UqJAT2cb-8WowPihsy0DvAiEC8jqMxQunZdsiUawkOYdtM1A-N9JM5xswQqqug7Wp6LUFttNrqGHydY6lmPE35KzX4RNKtop5KoBcP3zg-01Disjomea6zKaY-tnwULXCV8Uj3y1hmw_6Rt_EHgNiTQvDDAb6z1w5Il-FBjF_nPMO-os8zbIgmniobKF1kkWBn8LRYYhVGR0FkIJYXA6fPEDIp9PdXTOIly72D4RJSptzwWYO','B7gAAABwzM_6qBfFNEGEPY5mCa3s25BqpP05vk7UZH1h8i28aBu0zRyQtGbMi4kChFbO-5uGtwUJwnYuYCqi7ACSmgyZFrrQj557wI-wm3heHVxdOCl6B0XBZst_kqpDVrZasAn0fn2W4ZXTXnqmv9on3eD3KA');
INSERT INTO `Keys` (`usernum`, `signet`, `key`) VALUES (6,'Bv0AAMcBIGjq9I1m8mw-fbxNzdUHaQqMfAr-zq0_gSs1Hl5XuFD7AiECLoiw3N4j-rlqg1EptpTjUKMu8rjKoWJWv_Zz04U2AxQFCY9T25b0pPCwgYYqyal_-TTVHSgCWY18xmTaqrk9o7C07lZxKnUWl0_onELSDVoaFloJmlrcRJeujK-2qiGiBgaSHvbXZtf6fePTQHwc2_D1KgcYZL1Nui4tX5I6l9rZkLxBVbVKfaEF4H7E1usRD504hvDyXSNbMEXKq3mUWjAI','B7gAAABww6rMYLCkmrT9mmNZavsy4ekH1CchJMA4D2LDHeJf1zsVqXfBS-T8Qlu4dcq2jBT3ZP1443DBWnNNVhlMjOO-hSfWi7KCCdNqydqocdqTWVcp_XMLsn6vdz_OCkwzckK73-5E3cW_t8zq5nR9Cnx3og');
INSERT INTO `Keys` (`usernum`, `signet`, `key`) VALUES (7,'Bv0AAMcBIHv8SGmB2PaDyXVX49wsfnn9p2Tkf-jFfm6ykKSaFY0zAiEDc_Q1aKqcZKQf5EPC0AJP6tVFvgfKzmNqNVLcarTjV_sFMhOwQ1gyggO-Zk5wi7Ay4FmJSETcMkr0aR1q_NskOocQn5XVoLb_I_0AEVq2BdMNx6K8WilwKzbNJ-POLmC9Dgbr7kq8mvObKd234DSO-8NNA8ZWAj5hqeIQdDifhuYXXuLA0XRmQJg9vvd6L-ldcKEAb8GsGZTkfbCziByHwyUC','B7gAAABwnFuBNqYlG5mN0p10Q3CAhvv8rwldAT8lRGiCxr3CmGdnJ-ZSL0VKvSD_NH4r-QGsTR1mDTAg8dEPVUFEfBE6mn1jekUwFQwYPmV4PM6IBEVZds6z8H-FMu_pHz4jlBnrr9vx6xVn9k72pPtW0j_y_w');
INSERT INTO `Keys` (`usernum`, `signet`, `key`) VALUES (8,'Bv0AAMcBIGvSUm1_jMfKTyz5AzpzP8YY5MHjDiC0iDhigsijoMxnAiECdpYzZi2cvo_KVsQmIfeBXcaJPPH3Qo1b3LyV0qL_d7wFKN0MYCRbJ6GdSJnfn40z-UdLhVHCQmZut9SrQ4F6A45QRy8m6_-eCnZBQqydC_MgApHYffaxieGxTS0q4HGUBgaEquhsypHqPTb6WP4-i5KtleTQa4vdgL5EwhLgO5XyvnCqItvMJNDKax5M1aWzcEuDyVJiGhfO1_W7ZCFPFqYD','B7gAAABwS-LvSjwjeyCYsy_T0RFAHoi-Rst38JDl3kf4tZyXE6RBD64l2nucsVz5cN2V6nZt_2e0P6lSHmR1Eh2mgPjM1Zcl7eskWfew98cLTfhCLcTs6rdXnIM9MTqHXtaUVz95o_7l7ZBPSsg_6apA7XvOQw');
INSERT INTO `Keys` (`usernum`, `signet`, `key`) VALUES (9,'Bv0AAMcBIGyoHRHDS_ExrEOvd41pZVbqjTL9r5qvmIi--bzhFqmMAiECQh7-Y3SeSa5Yw0ncNnEw4bOcH8y9EawwmbhczHXUotwFkwdlFVYjRB0EIBHCPjxvf5OmQX_p1xhCr3s0l8GChg0KlJcMjZUZfVWzKRrK2RA6f7bMQ5NtEUCkyxfDY6VHBAa6tMsBWAqCzI9NANVBwHdEbjWAUjd_yYaPtRm51ROhsLwljHWJBPn1tlsKAOKqd_c85pdInIePfBeZXITjLGYI','B7gAAABw_7LafljuUhrjp8i5xWiD6MBX7HkAhC_ZzyXuFxpDEmLhl3ftyXjDtX4OnzKBoQI-MZ3OAKWSB2Cge-F_C794Go-cZDO31aWtQH_ZAZ0OU_wuVA3Jc3RIyH33dxqIfBVTdQ9qgls3IFRXxe6yrK-pnw');

INSERT INTO `Log` (`usernum`, `lastpop`, `lastmap`, `lastweb`, `lastchat`, `lastsent`, `lastreceived`, `popsessions`, `mapsessions`, `websessions`, `chatsessions`, `totalsent`, `totalreceived`, `totalbounces`, `created`, `created_ip`) VALUES (5,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2018-08-06 16:59:46','127.0.0.1');
INSERT INTO `Log` (`usernum`, `lastpop`, `lastmap`, `lastweb`, `lastchat`, `lastsent`, `lastreceived`, `popsessions`, `mapsessions`, `websessions`, `chatsessions`, `totalsent`, `totalreceived`, `totalbounces`, `created`, `created_ip`) VALUES (6,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2018-08-06 16:59:46','127.0.0.1');
INSERT INTO `Log` (`usernum`, `lastpop`, `lastmap`, `lastweb`, `lastchat`, `lastsent`, `lastreceived`, `popsessions`, `mapsessions`, `websessions`, `chatsessions`, `totalsent`, `totalreceived`, `totalbounces`, `created`, `created_ip`) VALUES (7,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2018-08-06 16:59:46','127.0.0.1');
INSERT INTO `Log` (`usernum`, `lastpop`, `lastmap`, `lastweb`, `lastchat`, `lastsent`, `lastreceived`, `popsessions`, `mapsessions`, `websessions`, `chatsessions`, `totalsent`, `totalreceived`, `totalbounces`, `created`, `created_ip`) VALUES (8,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2018-08-06 16:59:46','127.0.0.1');
INSERT INTO `Log` (`usernum`, `lastpop`, `lastmap`, `lastweb`, `lastchat`, `lastsent`, `lastreceived`, `popsessions`, `mapsessions`, `websessions`, `chatsessions`, `totalsent`, `totalreceived`, `totalbounces`, `created`, `created_ip`) VALUES (9,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2018-08-06 16:59:46','127.0.0.1');

INSERT INTO `Mailboxes` (`mailboxnum`, `address`, `usernum`) VALUES (11,'lock_inactive@lavabit.com',5);
INSERT INTO `Mailboxes` (`mailboxnum`, `address`, `usernum`) VALUES (12,'lock_expired@lavabit.com',6);
INSERT INTO `Mailboxes` (`mailboxnum`, `address`, `usernum`) VALUES (13,'lock_admin@lavabit.com',7);
INSERT INTO `Mailboxes` (`mailboxnum`, `address`, `usernum`) VALUES (14,'lock_abuse@lavabit.com',8);
INSERT INTO `Mailboxes` (`mailboxnum`, `address`, `usernum`) VALUES (15,'lock_user@lavabit.com',9);

INSERT INTO `Profile` (`usernum`, `name`, `address_one`, `address_two`, `city`, `state`, `zip`, `country`, `profile`, `phone`, `mobile`, `fax`, `gender`, `language`, `birthdate`, `industry`, `website`, `accessible`) VALUES (5,'','','','','',0,'',NULL,'','','',NULL,'','0000-00-00','','',0);
INSERT INTO `Profile` (`usernum`, `name`, `address_one`, `address_two`, `city`, `state`, `zip`, `country`, `profile`, `phone`, `mobile`, `fax`, `gender`, `language`, `birthdate`, `industry`, `website`, `accessible`) VALUES (6,'','','','','',0,'',NULL,'','','',NULL,'','0000-00-00','','',0);
INSERT INTO `Profile` (`usernum`, `name`, `address_one`, `address_two`, `city`, `state`, `zip`, `country`, `profile`, `phone`, `mobile`, `fax`, `gender`, `language`, `birthdate`, `industry`, `website`, `accessible`) VALUES (7,'','','','','',0,'',NULL,'','','',NULL,'','0000-00-00','','',0);
INSERT INTO `Profile` (`usernum`, `name`, `address_one`, `address_two`, `city`, `state`, `zip`, `country`, `profile`, `phone`, `mobile`, `fax`, `gender`, `language`, `birthdate`, `industry`, `website`, `accessible`) VALUES (8,'','','','','',0,'',NULL,'','','',NULL,'','0000-00-00','','',0);
INSERT INTO `Profile` (`usernum`, `name`, `address_one`, `address_two`, `city`, `state`, `zip`, `country`, `profile`, `phone`, `mobile`, `fax`, `gender`, `language`, `birthdate`, `industry`, `website`, `accessible`) VALUES (9,'','','','','',0,'',NULL,'','','',NULL,'','0000-00-00','','',0);

INSERT INTO `Realms` (`usernum`, `serial`, `label`, `shard`, `rotated`) VALUES (5,0,'mail','LWreVSWaVRKV4Xdw9GlsqlcPFbxythMTGtzO5xCoQWlaIjH5vLjE9rXTuOhaLBVsxOxbvtQNeUXILBZm0JfJKQ',1);
INSERT INTO `Realms` (`usernum`, `serial`, `label`, `shard`, `rotated`) VALUES (6,0,'mail','WBHWPFx2khiY7uChvtpWIeXfHXJXSuTBsdyHjUPmusy5IvmOCKJ2BPSQrADXhFjhYnmOdr340ZNRRQXfurtZrQ',1);
INSERT INTO `Realms` (`usernum`, `serial`, `label`, `shard`, `rotated`) VALUES (7,0,'mail','bOdEnn0D6YpNNnFFA-lcvCFnJJqz20VDRc7TJfNj7RahfaXonIb0lQd8ZXlJQvQyx-P1HpmWUO19ofhdx1ENTw',1);
INSERT INTO `Realms` (`usernum`, `serial`, `label`, `shard`, `rotated`) VALUES (8,0,'mail','mL7galUcnyDDTHMNHJuLxuqINhqzASqz9PIeKbSPGwLV3rCbzzfVsvbX_iXPkvhv4RforM_QgswhQ7u27ZA6wg',1);
INSERT INTO `Realms` (`usernum`, `serial`, `label`, `shard`, `rotated`) VALUES (9,0,'mail','vRVBZSMt4iw3_7bhhs2V51roTBePg6nnt7oxdha1cOzPaRXP2xcua12Fr7XQ8OXDQghI1gvbheFTlpz4TTF6Ww',1);
