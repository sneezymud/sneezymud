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
-- Temporary table structure for view `shop_overview`
--

DROP TABLE IF EXISTS `shop_overview`;
/*!50001 DROP VIEW IF EXISTS `shop_overview`*/;
/*!50001 CREATE TABLE `shop_overview` (
  `shop_nr` int(11),
  `keeper_name` char(127),
  `room_name` varchar(127),
  `corp_name` varchar(80),
  `bank_room` varchar(127),
  `tax_room` varchar(127),
  `gold` int(11),
  `keeper` int(11),
  `in_room` int(11),
  `corp_id` int(11),
  `tax_nr` int(11)
) */;

--
-- Final view structure for view `shop_overview`
--

/*!50001 DROP TABLE IF EXISTS `shop_overview`*/;
/*!50001 DROP VIEW IF EXISTS `shop_overview`*/;
/*!50001 CREATE ALGORITHM=UNDEFINED */
/*!50013 DEFINER=`sneezy`@`192.168.100.12` SQL SECURITY DEFINER */
/*!50001 VIEW `shop_overview` AS select `s1`.`shop_nr` AS `shop_nr`,`m1`.`name` AS `keeper_name`,`r1`.`name` AS `room_name`,`c1`.`name` AS `corp_name`,`r2`.`name` AS `bank_room`,`r3`.`name` AS `tax_room`,`o1`.`gold` AS `gold`,`s1`.`keeper` AS `keeper`,`s1`.`in_room` AS `in_room`,`o1`.`corp_id` AS `corp_id`,`o1`.`tax_nr` AS `tax_nr` from ((((((((`shop` `s1` left join `mob` `m1` on((`s1`.`keeper` = `m1`.`vnum`))) left join `room` `r1` on((`s1`.`in_room` = `r1`.`vnum`))) left join `shopowned` `o1` on((`s1`.`shop_nr` = `o1`.`shop_nr`))) left join `corporation` `c1` on((`o1`.`corp_id` = `c1`.`corp_id`))) left join `shop` `s2` on((`c1`.`bank` = `s2`.`shop_nr`))) left join `room` `r2` on((`s2`.`in_room` = `r2`.`vnum`))) left join `shop` `s3` on((`o1`.`tax_nr` = `s3`.`shop_nr`))) left join `room` `r3` on((`s3`.`in_room` = `r3`.`vnum`))) */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

