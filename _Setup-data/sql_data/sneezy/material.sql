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
-- Table structure for table `material`
--

DROP TABLE IF EXISTS `material`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `material` (
  `num` int(11) DEFAULT NULL,
  `name` varchar(32) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `material`
--

LOCK TABLES `material` WRITE;
/*!40000 ALTER TABLE `material` DISABLE KEYS */;
INSERT INTO `material` VALUES (0,'undefined');
INSERT INTO `material` VALUES (1,'paper');
INSERT INTO `material` VALUES (2,'cloth');
INSERT INTO `material` VALUES (3,'wax');
INSERT INTO `material` VALUES (4,'glass');
INSERT INTO `material` VALUES (5,'wood');
INSERT INTO `material` VALUES (6,'silk');
INSERT INTO `material` VALUES (7,'foodstuff');
INSERT INTO `material` VALUES (8,'plastic');
INSERT INTO `material` VALUES (9,'rubber');
INSERT INTO `material` VALUES (10,'cardboard');
INSERT INTO `material` VALUES (11,'string');
INSERT INTO `material` VALUES (12,'plasma');
INSERT INTO `material` VALUES (13,'toughened');
INSERT INTO `material` VALUES (14,'coral');
INSERT INTO `material` VALUES (15,'horsehair');
INSERT INTO `material` VALUES (16,'hair');
INSERT INTO `material` VALUES (17,'ash');
INSERT INTO `material` VALUES (18,'pumice');
INSERT INTO `material` VALUES (19,'laminate');
INSERT INTO `material` VALUES (50,'generic');
INSERT INTO `material` VALUES (51,'leather');
INSERT INTO `material` VALUES (52,'toughened');
INSERT INTO `material` VALUES (53,'dragon');
INSERT INTO `material` VALUES (54,'wool');
INSERT INTO `material` VALUES (55,'fur');
INSERT INTO `material` VALUES (56,'feathered');
INSERT INTO `material` VALUES (57,'water');
INSERT INTO `material` VALUES (58,'fire');
INSERT INTO `material` VALUES (59,'earth');
INSERT INTO `material` VALUES (60,'generic');
INSERT INTO `material` VALUES (61,'ice');
INSERT INTO `material` VALUES (62,'lightning');
INSERT INTO `material` VALUES (63,'chaos');
INSERT INTO `material` VALUES (64,'clay');
INSERT INTO `material` VALUES (65,'porcelain');
INSERT INTO `material` VALUES (66,'straw');
INSERT INTO `material` VALUES (67,'pearl');
INSERT INTO `material` VALUES (68,'flesh');
INSERT INTO `material` VALUES (69,'cat');
INSERT INTO `material` VALUES (70,'dog');
INSERT INTO `material` VALUES (71,'rabbit');
INSERT INTO `material` VALUES (72,'ghostly');
INSERT INTO `material` VALUES (73,'dwarven');
INSERT INTO `material` VALUES (74,'soft');
INSERT INTO `material` VALUES (75,'fishscale');
INSERT INTO `material` VALUES (76,'ogre');
INSERT INTO `material` VALUES (77,'hemp');
INSERT INTO `material` VALUES (100,'generic');
INSERT INTO `material` VALUES (101,'jeweled');
INSERT INTO `material` VALUES (102,'runed');
INSERT INTO `material` VALUES (103,'crystal');
INSERT INTO `material` VALUES (104,'diamond');
INSERT INTO `material` VALUES (105,'ebony');
INSERT INTO `material` VALUES (106,'emerald');
INSERT INTO `material` VALUES (107,'ivory');
INSERT INTO `material` VALUES (108,'obsidian');
INSERT INTO `material` VALUES (109,'onyx');
INSERT INTO `material` VALUES (110,'opal');
INSERT INTO `material` VALUES (111,'ruby');
INSERT INTO `material` VALUES (112,'sapphire');
INSERT INTO `material` VALUES (113,'marble');
INSERT INTO `material` VALUES (114,'stone');
INSERT INTO `material` VALUES (115,'skull');
INSERT INTO `material` VALUES (116,'jade');
INSERT INTO `material` VALUES (117,'amber');
INSERT INTO `material` VALUES (118,'turquoise');
INSERT INTO `material` VALUES (119,'amethyst');
INSERT INTO `material` VALUES (120,'mica');
INSERT INTO `material` VALUES (121,'dragonbone');
INSERT INTO `material` VALUES (122,'malachite');
INSERT INTO `material` VALUES (123,'granite');
INSERT INTO `material` VALUES (124,'quartz');
INSERT INTO `material` VALUES (125,'jet');
INSERT INTO `material` VALUES (126,'corundum');
INSERT INTO `material` VALUES (150,'generic');
INSERT INTO `material` VALUES (151,'copper');
INSERT INTO `material` VALUES (152,'scale');
INSERT INTO `material` VALUES (153,'banded');
INSERT INTO `material` VALUES (154,'chain');
INSERT INTO `material` VALUES (155,'plate');
INSERT INTO `material` VALUES (156,'bronze');
INSERT INTO `material` VALUES (157,'brass');
INSERT INTO `material` VALUES (158,'iron');
INSERT INTO `material` VALUES (159,'steel');
INSERT INTO `material` VALUES (160,'mithril');
INSERT INTO `material` VALUES (161,'admantium');
INSERT INTO `material` VALUES (162,'silver');
INSERT INTO `material` VALUES (163,'gold');
INSERT INTO `material` VALUES (164,'platinum');
INSERT INTO `material` VALUES (165,'titanium');
INSERT INTO `material` VALUES (166,'aluminum');
INSERT INTO `material` VALUES (167,'ringmail');
INSERT INTO `material` VALUES (168,'gnomemail');
INSERT INTO `material` VALUES (169,'electrum');
INSERT INTO `material` VALUES (170,'athanor');
INSERT INTO `material` VALUES (171,'tin');
INSERT INTO `material` VALUES (172,'tungsten');
INSERT INTO `material` VALUES (173,'admintite');
INSERT INTO `material` VALUES (174,'terbium');
INSERT INTO `material` VALUES (175,'mithril');
INSERT INTO `material` VALUES (176,'steel');
/*!40000 ALTER TABLE `material` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2019-05-02 21:42:31
