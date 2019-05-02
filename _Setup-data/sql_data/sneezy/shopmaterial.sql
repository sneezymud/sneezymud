-- MySQL dump 10.16  Distrib 10.1.28-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: sneezy
-- ------------------------------------------------------
-- Server version	10.1.28-MariaDB

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
-- Table structure for table `shopmaterial`
--

DROP TABLE IF EXISTS `shopmaterial`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `shopmaterial` (
  `shop_nr` int(11) NOT NULL DEFAULT '0',
  `mat_type` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `shopmaterial`
--

LOCK TABLES `shopmaterial` WRITE;
/*!40000 ALTER TABLE `shopmaterial` DISABLE KEYS */;
INSERT INTO `shopmaterial` VALUES (85,75);
INSERT INTO `shopmaterial` VALUES (85,115);
INSERT INTO `shopmaterial` VALUES (86,18);
INSERT INTO `shopmaterial` VALUES (87,67);
INSERT INTO `shopmaterial` VALUES (90,2);
INSERT INTO `shopmaterial` VALUES (91,51);
INSERT INTO `shopmaterial` VALUES (91,52);
INSERT INTO `shopmaterial` VALUES (91,74);
INSERT INTO `shopmaterial` VALUES (92,54);
INSERT INTO `shopmaterial` VALUES (93,6);
INSERT INTO `shopmaterial` VALUES (94,163);
INSERT INTO `shopmaterial` VALUES (95,162);
INSERT INTO `shopmaterial` VALUES (96,157);
INSERT INTO `shopmaterial` VALUES (99,100);
INSERT INTO `shopmaterial` VALUES (100,9);
INSERT INTO `shopmaterial` VALUES (101,4);
INSERT INTO `shopmaterial` VALUES (102,8);
INSERT INTO `shopmaterial` VALUES (103,3);
INSERT INTO `shopmaterial` VALUES (98,55);
INSERT INTO `shopmaterial` VALUES (98,69);
INSERT INTO `shopmaterial` VALUES (98,70);
INSERT INTO `shopmaterial` VALUES (98,71);
/*!40000 ALTER TABLE `shopmaterial` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2019-05-02 21:42:46
