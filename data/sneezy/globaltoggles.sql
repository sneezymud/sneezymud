-- MySQL dump 10.10
--
-- Host: db.sneezymud.com    Database: sneezy
-- ------------------------------------------------------
-- Server version	5.0.24a-standard

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `globaltoggles`
--

DROP TABLE IF EXISTS `globaltoggles`;
CREATE TABLE `globaltoggles` (
  `tog_id` int(11) NOT NULL,
  `toggle` tinyint(1) default NULL,
  `testcode` tinyint(1) default NULL,
  `name` varchar(80) default NULL,
  `descr` varchar(256) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `globaltoggles`
--


/*!40000 ALTER TABLE `globaltoggles` DISABLE KEYS */;
LOCK TABLES `globaltoggles` WRITE;
INSERT INTO `globaltoggles` VALUES (1,0,0,'No-Shouting','allow shouting'),(2,1,0,'Sleep offensive','sleep spell offensive'),(4,1,0,'Gravity','gravity'),(5,1,0,'Clients','allow connections with SneezyMUD client'),(6,1,0,'Builder Wiznet','allow builders to hear wiznet'),(7,1,0,'PCs w/mob names','allow PCs with mob names'),(8,0,0,'Twinky Combat','twinky combat messages'),(9,0,0,'Time DB Queries','time database queries'),(10,0,0,'Game Loop Timing','print timing info for game loop'),(11,1,0,'Double Exp','turn on double exp'),(12,1,1,'Test Code 1','Queued player saves'),(14,0,1,'Test Code 3','not currently used'),(15,0,1,'Test Code 4','not currently used'),(17,0,1,'Test Code 6','not currently used'),(13,0,1,'Test Code 2','allow players to see item levels'),(16,0,1,'Test Code 5','disable/enable certain aspects of the new faction code - dash'),(18,0,1,'Quest Code 1','unknown'),(19,0,1,'Quest Code 2','unknown'),(20,0,1,'Quest Code 3','unknown'),(21,0,1,'Quest Code 4','unknown'),(3,0,0,'Newbie PK','allow PKing of newbies');
UNLOCK TABLES;
/*!40000 ALTER TABLE `globaltoggles` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

