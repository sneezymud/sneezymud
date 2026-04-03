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
DROP TABLE IF EXISTS `mob`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `mob` (
  `vnum` int(11) NOT NULL,
  `name` varchar(127) NOT NULL,
  `short_desc` varchar(127) NOT NULL,
  `long_desc` varchar(255) NOT NULL,
  `description` mediumtext NOT NULL,
  `actions` int(11) unsigned NOT NULL,
  `affects` int(11) unsigned NOT NULL,
  `faction` int(11) NOT NULL,
  `fact_perc` int(11) NOT NULL,
  `letter` char(1) NOT NULL,
  `attacks` decimal(5,1) NOT NULL,
  `class` int(11) NOT NULL,
  `level` int(11) NOT NULL,
  `tohit` int(11) NOT NULL,
  `ac` decimal(5,1) NOT NULL,
  `hpbonus` decimal(5,1) NOT NULL,
  `damage_level` decimal(5,1) NOT NULL,
  `damage_precision` int(11) NOT NULL,
  `gold` int(11) NOT NULL,
  `race` int(11) NOT NULL,
  `weight` int(11) NOT NULL,
  `height` int(11) NOT NULL,
  `str` int(11) NOT NULL,
  `bra` int(11) NOT NULL,
  `con` int(11) NOT NULL,
  `dex` int(11) NOT NULL,
  `agi` int(11) NOT NULL,
  `intel` int(11) NOT NULL,
  `wis` int(11) NOT NULL,
  `foc` int(11) NOT NULL,
  `per` int(11) NOT NULL,
  `cha` int(11) NOT NULL,
  `kar` int(11) NOT NULL,
  `spe` int(11) NOT NULL,
  `pos` int(11) NOT NULL,
  `def_position` int(11) NOT NULL,
  `sex` int(11) NOT NULL,
  `spec_proc` int(11) NOT NULL,
  `skin` int(11) NOT NULL,
  `vision` int(11) NOT NULL,
  `can_be_seen` int(11) NOT NULL,
  `max_exist` int(11) NOT NULL,
  `local_sound` varchar(255) DEFAULT NULL,
  `adjacent_sound` varchar(255) DEFAULT NULL,
  `player_id` bigint(20) unsigned NOT NULL,
  PRIMARY KEY (`player_id`,`vnum`),
  CONSTRAINT `fk_mob_player_id` FOREIGN KEY (`player_id`) REFERENCES `sneezy`.`player` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*M!100616 SET NOTE_VERBOSITY=@OLD_NOTE_VERBOSITY */;

