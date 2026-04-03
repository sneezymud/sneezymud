/*M!999999\- enable the sandbox mode */ 

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*M!100616 SET @OLD_NOTE_VERBOSITY=@@NOTE_VERBOSITY, NOTE_VERBOSITY=0 */;
DROP TABLE IF EXISTS `ship_destinations`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `ship_destinations` (
  `vnum` int(11) NOT NULL,
  `name` varchar(32) NOT NULL,
  `room` int(11) DEFAULT NULL,
  PRIMARY KEY (`vnum`,`name`),
  KEY `fk_ship_destinations_room` (`room`),
  CONSTRAINT `fk_ship_destinations_room` FOREIGN KEY (`room`) REFERENCES `room` (`vnum`),
  CONSTRAINT `fk_ship_destinations_vnum` FOREIGN KEY (`vnum`) REFERENCES `mob` (`vnum`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

SET @OLD_AUTOCOMMIT=@@AUTOCOMMIT, @@AUTOCOMMIT=0;
LOCK TABLES `ship_destinations` WRITE;
/*!40000 ALTER TABLE `ship_destinations` DISABLE KEYS */;
INSERT INTO `ship_destinations` VALUES
(19000,'Gnath',2400),
(15375,'cardac',2471),
(15375,'Brazzed',2492),
(19000,'creed',2495),
(15375,'Ranger',5408),
(19000,'WestGrimhaven',5417),
(15375,'Xanesla',6301),
(19000,'Xanesla',6301),
(19000,'Citadel',10847),
(19000,'NorthEastMyrian',12545),
(19000,'lotsofcrap',12607),
(15375,'garbage',12623),
(19000,'Garbageisland',12623),
(19000,'JungleBeach',12643),
(19000,'WhirlpoolVortex',12781),
(19000,'smalltropicalisland',12802),
(19000,'Whirlpool',12814),
(19000,'Benjamin',12854),
(19000,'siren',12874),
(19000,'Yola',12889),
(19000,'LazyGuy',13106),
(19000,'sunkenship',13111),
(19000,'StrangeIsland',13142),
(15375,'fog',13280),
(15375,'DD',13899),
(19000,'SpiritBeach',14114),
(15375,'kalysia',14117),
(15375,'fishmaster',15150),
(19000,'Fishingshack',15150),
(15375,'Waterfall',15286),
(19000,'poachers',15294),
(19000,'Mansion',26869),
(15375,'Neghya',27274);
/*!40000 ALTER TABLE `ship_destinations` ENABLE KEYS */;
UNLOCK TABLES;
COMMIT;
SET AUTOCOMMIT=@OLD_AUTOCOMMIT;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*M!100616 SET NOTE_VERBOSITY=@OLD_NOTE_VERBOSITY */;

