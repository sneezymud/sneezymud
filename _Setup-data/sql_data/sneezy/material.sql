-- MariaDB dump 10.19  Distrib 10.5.9-MariaDB, for debian-linux-gnu (x86_64)
--
-- Host: sneezy-db    Database: sneezy
-- ------------------------------------------------------
-- Server version	10.5.9-MariaDB-1:10.5.9+maria~focal

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
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
INSERT INTO `material` VALUES (0,'undefined'),
(1,'paper'),
(2,'cloth'),
(3,'wax'),
(4,'glass'),
(5,'wood'),
(6,'silk'),
(7,'foodstuff'),
(8,'plastic'),
(9,'rubber'),
(10,'cardboard'),
(11,'string'),
(12,'plasma'),
(13,'toughened'),
(14,'coral'),
(15,'horsehair'),
(16,'hair'),
(17,'ash'),
(18,'pumice'),
(19,'laminate'),
(50,'generic'),
(51,'leather'),
(52,'toughened'),
(53,'dragon'),
(54,'wool'),
(55,'fur'),
(56,'feathered'),
(57,'water'),
(58,'fire'),
(59,'earth'),
(60,'generic'),
(61,'ice'),
(62,'lightning'),
(63,'chaos'),
(64,'clay'),
(65,'porcelain'),
(66,'straw'),
(67,'pearl'),
(68,'flesh'),
(69,'cat'),
(70,'dog'),
(71,'rabbit'),
(72,'ghostly'),
(73,'dwarven'),
(74,'soft'),
(75,'fishscale'),
(76,'ogre'),
(77,'hemp'),
(100,'generic'),
(101,'jeweled'),
(102,'runed'),
(103,'crystal'),
(104,'diamond'),
(105,'ebony'),
(106,'emerald'),
(107,'ivory'),
(108,'obsidian'),
(109,'onyx'),
(110,'opal'),
(111,'ruby'),
(112,'sapphire'),
(113,'marble'),
(114,'stone'),
(115,'skull'),
(116,'jade'),
(117,'amber'),
(118,'turquoise'),
(119,'amethyst'),
(120,'mica'),
(121,'dragonbone'),
(122,'malachite'),
(123,'granite'),
(124,'quartz'),
(125,'jet'),
(126,'corundum'),
(150,'generic'),
(151,'copper'),
(152,'scale'),
(153,'banded'),
(154,'chain'),
(155,'plate'),
(156,'bronze'),
(157,'brass'),
(158,'iron'),
(159,'steel'),
(160,'mithril'),
(161,'admantium'),
(162,'silver'),
(163,'gold'),
(164,'platinum'),
(165,'titanium'),
(166,'aluminum'),
(167,'ringmail'),
(168,'gnomemail'),
(169,'electrum'),
(170,'athanor'),
(171,'tin'),
(172,'tungsten'),
(173,'admintite'),
(174,'terbium'),
(175,'mithril'),
(176,'steel');
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

-- Dump completed on 2024-07-02 15:14:41
