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
-- Table structure for table `poll`
--

DROP TABLE IF EXISTS `poll`;
CREATE TABLE `poll` (
  `poll_id` int(11) NOT NULL,
  `descr` varchar(127) default NULL,
  `status` varchar(8) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `poll`
--


/*!40000 ALTER TABLE `poll` DISABLE KEYS */;
LOCK TABLES `poll` WRITE;
INSERT INTO `poll` VALUES (1,'Least Favorite Immortal','closed'),(2,'Least Favorite Mortal','closed'),(3,'Mayor of Amber, November 2004','closed'),(4,'Mayor of Logrus, November 2004','closed'),(5,'Bush vs Kerry','closed'),(13,'Vote for District 1 - GH City Council','open'),(6,'who is your daddy','closed'),(0,'','closed'),(14,'Vote for District 2 - GH City Council','open'),(15,'Vote for District 3 - GH City Council','open'),(16,'Vote for District 4 - GH City Council','open'),(10,'What\'s the worst thing about Sneezy?','closed'),(7,'Mayor of Grimhaven, April 2005','closed'),(9,'What\'s the best thing about Sneezy?','closed'),(11,'I love Coral.','closed'),(12,'How old are you?','closed'),(8,'Who is your daddy?','closed');
UNLOCK TABLES;
/*!40000 ALTER TABLE `poll` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

