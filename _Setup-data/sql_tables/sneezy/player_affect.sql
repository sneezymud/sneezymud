-- SneezyMUD player affects table
-- Stores character buffs/debuffs/spell affects, replacing the fixed-size
-- charFile array (MAX_AFFECT=25) with an unlimited DB-backed store.

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
-- Table structure for table `player_affect`
--

DROP TABLE IF EXISTS `player_affect`;
CREATE TABLE `player_affect` (
  `id` bigint(20) unsigned NOT NULL auto_increment,
  `player_id` bigint(20) unsigned NOT NULL,
  `type` smallint(6) NOT NULL,
  `level` tinyint(4) NOT NULL,
  `duration` int(11) NOT NULL,
  `renew` int(11) NOT NULL,
  `modifier` bigint(20) NOT NULL,
  `modifier2` bigint(20) NOT NULL,
  `location` tinyint(3) unsigned NOT NULL,
  `bitvector` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_player_affect_player` (`player_id`),
  CONSTRAINT `fk_player_affect_player` FOREIGN KEY (`player_id`) REFERENCES `player` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
