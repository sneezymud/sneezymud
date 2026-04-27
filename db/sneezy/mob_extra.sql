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
DROP TABLE IF EXISTS `mob_extra`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `mob_extra` (
  `vnum` int(11) NOT NULL DEFAULT 0,
  `keyword` varchar(32) NOT NULL,
  `description` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`vnum`,`keyword`),
  CONSTRAINT `fk_mob_extra_vnum` FOREIGN KEY (`vnum`) REFERENCES `mob` (`vnum`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

SET @OLD_AUTOCOMMIT=@@AUTOCOMMIT, @@AUTOCOMMIT=0;
LOCK TABLES `mob_extra` WRITE;
/*!40000 ALTER TABLE `mob_extra` DISABLE KEYS */;
INSERT INTO `mob_extra` VALUES
(10,'bamfin','This is a <r>test<z> of the <k><n> broad<z><g>cast<z> system.'),
(4547,'repop','A young cannibal appears suddenly in a puff of smoke.'),
(5018,'movein','A <W>white doe<z> steps tentatively into view.'),
(5623,'movein','<c>Egdinia says, <z>\"What are you doing in here!!!\"'),
(6460,'deathcry','331608472'),
(6460,'repop','324946152'),
(8950,'deathcry','The ground shakes as it connects with the lifeless body of the Rhentek nearby.'),
(9095,'deathcry','<r>I have failed you L\'yrel!!!!!  AVENGE ME!!!'),
(10601,'deathcry','With her last burst of electrical energy, Shorenjin burns the caves walls in her death throes!'),
(10601,'repop','The dragon Shorenjin, the Ancient bursts from the water below your feet.'),
(10602,'deathcry','Loranth-gil, King of the Lake Elves invokes a curse upon right before his body falls lifeless.'),
(10602,'repop','Loranth-gil, King of the Lake elves steps out from the shadows.'),
(10603,'deathcry','Queen Fenban-gil cries out in anguish as her body falls lifeless!'),
(10603,'repop','Queen Fenban-gil makes a royal entry as the guards at the door step to the side.'),
(10604,'deathcry','Princess Scrya cries out, \"Father please....\"'),
(10604,'repop','The Princess Scrya suddenly becomes visible before your eyes!'),
(10605,'deathcry','Prince Rivzin crumples to the floor.'),
(10605,'repop','A young elven monk seems to materialize out of no-where.'),
(10606,'deathcry','Grayvirjin screams, \"To HELL WITH YOU!\"'),
(10606,'repop','Grayvirjin stalks into the room, obviously displeased.'),
(10608,'deathcry','Fenlannul screams in anger as his short life passes before his eyes.'),
(10608,'repop','A young elven monk seems to materialize out of no-where.'),
(10609,'deathcry','Ayinshan smiles to himself and he welcomes death.'),
(10609,'movein','Aysinshan-gir, the Kings Advisor, enters the room.'),
(10609,'repop','Queen Fenban-gil makes a royal entry as the guards at the door step to the side.'),
(15956,'repop','An elven scholar arrives to start this evenings study.'),
(15958,'repop','An elven scholar arrives to start this evenings study.'),
(19000,'repop','Captain Matho has arrived.'),
(19003,'repop','A scurvy sailor has arrived for duty.'),
(19008,'repop','A vicious & bloodthirsty pirate arises with a belch.'),
(19011,'repop','Hapke the cabinboy appears as if from nowhere.'),
(24689,'repop','A grizzled old werewolf awakens from his slumber.'),
(24695,'repop','An <g>imp<z> materializes from the shadows of the forest.'),
(24696,'repop','An <b>imp<z> materializes from the shadows of the forest.'),
(28913,'bamfin','A trolloc walks in from a night of killing to relax.'),
(28914,'bamfin','A trolloc walks in from a night of killing to relax.'),
(28918,'bamfin','A trolloc walks in from a night of killing to relax.'),
(28919,'bamfin','A trolloc walks in from a day of killing to relax.'),
(28920,'bamfin','A trolloc walks in from a day of killing relax.'),
(28921,'bamfin','A trolloc walks in from a night of killing to relax.'),
(30994,'repop','A thin, <w>pale-faced<z> gnome with <r>crimson lips<z> slowly rises from <o>a small sarcophagus<1> and howls, \"What fool dares disturb the sleep of the dead?!\"'),
(31310,'deathcry','Baron Samedi\'s soul leaps from his body and dives into the ground.'),
(33300,'bamfin','A fallow deer prances out of the forest.'),
(33779,'movein','A <W>white doe<z> steps tentatively into view.'),
(37138,'repop','$n appears and licks its stinking teeth.');
/*!40000 ALTER TABLE `mob_extra` ENABLE KEYS */;
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

