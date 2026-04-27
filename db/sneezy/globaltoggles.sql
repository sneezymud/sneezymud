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
DROP TABLE IF EXISTS `globaltoggles`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `globaltoggles` (
  `tog_id` int(11) NOT NULL,
  `toggle` tinyint(1) DEFAULT NULL,
  `testcode` tinyint(1) DEFAULT NULL,
  `name` varchar(80) DEFAULT NULL,
  `descr` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`tog_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

SET @OLD_AUTOCOMMIT=@@AUTOCOMMIT, @@AUTOCOMMIT=0;
LOCK TABLES `globaltoggles` WRITE;
/*!40000 ALTER TABLE `globaltoggles` DISABLE KEYS */;
INSERT INTO `globaltoggles` VALUES
(1,0,0,'No-Shouting','allow shouting'),
(2,1,0,'Sleep offensive','sleep spell offensive'),
(3,0,0,'Newbie PK','allow PKing of newbies'),
(4,1,0,'Gravity','gravity'),
(5,1,0,'Clients','allow connections with SneezyMUD client'),
(6,1,0,'Builder Wiznet','allow builders to hear wiznet'),
(7,1,0,'PCs w/mob names','allow PCs with mob names'),
(8,0,0,'Twinky Combat','twinky combat messages'),
(9,0,0,'Time DB Queries','time database queries'),
(10,0,0,'Game Loop Timing','print timing info for game loop'),
(11,1,0,'Double Exp','turn on double exp'),
(12,1,1,'Test Code 1','Queued player saves'),
(13,0,1,'Test Code 2','allow players to see item levels'),
(14,0,1,'Test Code 3','not currently used'),
(15,0,1,'Test Code 4','not currently used'),
(16,0,1,'Test Code 5','disable/enable certain aspects of the new faction code - dash'),
(17,0,0,'DB Logging','log all db queries'),
(18,0,1,'Quest Code 1','unknown'),
(19,0,1,'Quest Code 2','unknown'),
(20,0,1,'Quest Code 3','unknown'),
(21,0,1,'Quest Code 4','unknown');
/*!40000 ALTER TABLE `globaltoggles` ENABLE KEYS */;
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

