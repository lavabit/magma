
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
INSERT INTO `Dispatch` VALUES (1,1,'',0,1,0,5,1,'REJECT',1,'REJECT',1,'MARK',1,'MARK',1,1,'DELETE',1,'DELETE',0,NULL,1,33554432,33554432,256,32768,32768,0),(2,1,'',0,1,0,5,1,'REJECT',1,'REJECT',1,'MARK',1,'MARK',2,1,'DELETE',1,'DELETE',0,NULL,2,33554432,33554432,256,1024,256,0),(3,1,'',0,1,0,5,1,'REJECT',1,'REJECT',1,'MARK',0,'MARK',3,1,'DELETE',1,'DELETE',0,NULL,3,33554432,33554432,256,1024,256,0),(4,1,'',0,1,0,5,1,'REJECT',1,'REJECT',1,'MARK',1,'MARK',4,1,'DELETE',1,'DELETE',0,NULL,4,33554432,33554432,256,1024,256,0);
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
INSERT INTO `Folders` VALUES (1,1,'Inbox',0,NULL),(2,2,'Inbox',0,NULL),(3,3,'Inbox',0,NULL);
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
INSERT INTO `Host_Config` VALUES (1,NULL,'magmad','magma.system.increase_resource_limits','false','2009-12-16 04:32:54'),(2,NULL,'magmad','magma.storage.default','local','2011-03-24 12:11:22'),(3,NULL,'magmad','magma.smtp.blacklist','bl.spamcop.net.','2011-04-13 16:03:55');
/*!40000 ALTER TABLE `Host_Config` ENABLE KEYS */;

--
-- Dumping data for table `Hosts`
--
-- ORDER BY:  `hostnum`

/*!40000 ALTER TABLE `Hosts` DISABLE KEYS */;
INSERT INTO `Hosts` VALUES (1,'dark.lavabit.com','2009-12-16 04:32:54');
/*!40000 ALTER TABLE `Hosts` ENABLE KEYS */;

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
INSERT INTO `Limits` VALUES ('BASIC','The Lavabit basic account plan.',0,0,0,0,1,1,1,1,30,1,1,'REJECT,DELETE,MARK',1,'REJECT,DELETE,MARK',1,'DELETE,MARK',0,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK','DELETE',0,1,1048576,33554432,1,1048576,33554432,1,8,256,1,8,1024,1,8,1024,1,1),('ENHANCED','The Lavabit enhanced account plan.',0,0,1,1,1,1,1,1,30,1,1,'REJECT,DELETE,MARK',1,'REJECT,DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK','DELETE',1,1,1048576,67108864,1,1048576,67108864,1,8,512,1,8,1024,1,8,1024,1,1),('PERSONAL','The Lavabit personal account plan.',0,0,0,1,1,1,1,1,30,1,1,'REJECT,DELETE,MARK',1,'REJECT,DELETE,MARK',1,'DELETE,MARK',0,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK','DELETE',1,1,1048576,67108864,1,1048576,67108864,1,8,256,1,8,1024,1,8,1024,1,1),('PREMIUM','The Lavabit premium account plan.',0,0,1,1,1,1,1,1,30,1,1,'REJECT,DELETE,MARK',1,'REJECT,DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK',1,'DELETE,MARK','DELETE',1,1,1048576,134217728,1,1048576,134217728,1,8,768,1,8,8196,1,8,8196,1,1);
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
INSERT INTO `Log` VALUES (1,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2011-05-05 14:15:59','0.0.0.0'),(2,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2011-05-05 14:16:03','0.0.0.0'),(3,'0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00','0000-00-00 00:00:00',0,0,0,0,0,0,0,'2011-05-05 14:16:09','0.0.0.0');
/*!40000 ALTER TABLE `Log` ENABLE KEYS */;

--
-- Dumping data for table `Mailboxes`
--
-- ORDER BY:  `address`

/*!40000 ALTER TABLE `Mailboxes` DISABLE KEYS */;
INSERT INTO `Mailboxes` VALUES    ('ladar@mailshack.com',3),('ladar@nerdshack.com',3),('ladar@lavabit.com',3),('ladar@example.com',3),('magma@lavabit.com',1),('magma@example.com',1),('princess@example.com',2),('stacie@lavabit.com',4);
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
INSERT INTO `Users` VALUES (1,'magma',NULL, NULL,0,'42a7ead6550ee52360222a0e8783645216c3bb4ade95ab531be13cd3060dcdd085fb534e424e067bc4edfe27a87277daaad6428fee737b3adc3c55608fb23ea5',0,'BASIC',0,0,NULL,1,1,0,0,134217728,0,'0000-00-00','0000-00-00'),(2,'princess', NULL, NULL,0,'3a5799cbc019beebd5e779a62343a76ff949c829b792ebe2c2fae406eda57268ce200db1ada838936b0ac6804115d60e83e88189705bd3f52f1723f29ce9cfa1',0,'BASIC',0,0,NULL,1,1,0,0,134217728,0,'0000-00-00','0000-00-00'),(3,'ladar', NULL, NULL,0,'eb5ff977cd0ef6487677b4961995088d3a86caa2e6710829a28047017406e493b6839fbdf69d2e4ee290fee1181ba1a4c105afe7c507e91e5773d71d0461adba',0,'BASIC',0,0,NULL,1,1,0,0,134217728,0,'0000-00-00','0000-00-00'),(4,'stacie','MOgx1F13HmoSGt05L2AvYwjWVqS_NmEU1b6eaWE9EOb819su6Z2qUvxdsQyx1CCL_xlCYhh2OJhaoxN0UlUIjvZ-yz08TBaWZ7Z0B3ZNrBtTs3OOio4K7pMkDLpXxCjjS2eboU7nNxn1sdrgKLICOZSWtPIDJmAAIyr9GOPF-x4','MAtbJr6lIPOmrYuQMQaPzDq8mNRN8qp9MefYk8vyxnL3DrsuzFeSMhGL5Ew4tDTYA1hNzqroJaoGB8jWpUKAwA',0,NULL,0,'BASIC',0,0,NULL,1,1,0,0,134217728,0,'0000-00-00','0000-00-00');
/*!40000 ALTER TABLE `Users` ENABLE KEYS */;
