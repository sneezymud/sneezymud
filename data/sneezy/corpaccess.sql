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
-- Table structure for table `corpaccess`
--

DROP TABLE IF EXISTS `corpaccess`;
CREATE TABLE `corpaccess` (
  `corp_id` int(11) NOT NULL,
  `access` int(11) NOT NULL,
  `player_id` int(11) default NULL,
  `name` varchar(80) default NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `corpaccess`
--


/*!40000 ALTER TABLE `corpaccess` DISABLE KEYS */;
LOCK TABLES `corpaccess` WRITE;
INSERT INTO `corpaccess` VALUES (28,1,1658,'maror'),(2,1,2466,'knuckles'),(17,1,828,'ragamuffin'),(27,1,2308,'damescena'),(11,1,1330,'laren'),(4,1,1903,'frobozz'),(11,1,1928,'poppin'),(17,1,10170,'sephie'),(13,1,1917,'goldlama'),(3,1,1653,'birch'),(24,1,4061,'immorality'),(17,1,5868,'taenryl'),(17,1,4017,'legato'),(3,1,669,'fitz'),(8,1,2052,'adoresqua'),(24,1,553,'pirvan'),(12,1,553,'pirvan'),(6,1,1951,'spiff'),(30,1,9229,'sage'),(7,1,669,'fitz'),(7,1,1330,'laren'),(7,1,1932,'tenbutts'),(7,1,2466,'knuckles'),(21,1,2155,'peel');
UNLOCK TABLES;
/*!40000 ALTER TABLE `corpaccess` ENABLE KEYS */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

