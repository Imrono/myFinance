-- MySQL dump 10.13  Distrib 5.5.45, for CYGWIN (x86_64)
--
-- Host: 127.0.0.1    Database: myfinance
-- ------------------------------------------------------
-- Server version	5.7.10

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
-- Table structure for table `资产`
--

DROP TABLE IF EXISTS `资产`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `资产` (
  `代号` varchar(16) NOT NULL,
  `名称` varchar(16) NOT NULL,
  `资产帐户代号` varchar(32) NOT NULL,
  `数量` int(11) NOT NULL DEFAULT '0',
  `单位成本` decimal(14,3) NOT NULL DEFAULT '0.000',
  `类别` varchar(16) NOT NULL,
  `pos` int(4) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`代号`,`资产帐户代号`),
  KEY `资产_fk_1` (`资产帐户代号`),
  CONSTRAINT `资产_fk_1` FOREIGN KEY (`资产帐户代号`) REFERENCES `资产帐户` (`代号`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `资产`
--

LOCK TABLES `资产` WRITE;
/*!40000 ALTER TABLE `资产` DISABLE KEYS */;
INSERT INTO `资产` VALUES ('cash','现金','15195977581_ZFB',1,396.400,'',0),('cash','现金','6215584301002835944',1,27727.881,'工资',0),('cash','负债','6225758338428617',1,-110.100,'负债',0),('cash','可用资金','666600081267',1,-4983.240,'现金',0),('cash','可用资金','8451990',1,12507.210,'现金',0),('sh.600820','隧道股份','8451990',300,9.367,'',1),('sh.601688','华泰证券','666600081267',200,18.365,'',1),('sz.000333','美的集团','666600081267',2000,28.887,'深圳股票',2),('sz.000651','格力电器','8451990',700,19.296,'',3),('sz.002550','千红制药','8451990',400,13.791,'',2);
/*!40000 ALTER TABLE `资产` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `资产变化`
--

DROP TABLE IF EXISTS `资产变化`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `资产变化` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `时间` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `变化类别` varchar(16) NOT NULL,
  `资产帐户代号1` varchar(16) NOT NULL,
  `变化资金` decimal(14,3) DEFAULT '0.000',
  `资产帐户代号2` varchar(16) NOT NULL,
  `代号` varchar(16) DEFAULT NULL,
  `名称` varchar(16) DEFAULT NULL,
  `单价` decimal(14,3) DEFAULT '0.000',
  `数量` int(11) DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `资产变化`
--

LOCK TABLES `资产变化` WRITE;
/*!40000 ALTER TABLE `资产变化` DISABLE KEYS */;
INSERT INTO `资产变化` VALUES (1,'2000-01-01 00:00:00','56','8451990',-5.000,'666600081267','cash','现有资金',5.000,1),(2,'2000-01-01 00:00:00','56','8451990',-5.000,'666600081267','cash','现有资金',5.000,1),(3,'2000-01-01 00:00:00','56','8451990',-5.000,'666600081267','cash','现有资金',5.000,1),(4,'2000-01-01 00:00:00','56','8451990',-5.000,'666600081267','cash','现有资金',5.000,1),(5,'2000-00-01 00:00:00','','8451990',-4.000,'666600081267','cash','现有资金',4.000,1),(6,'2000-00-01 00:00:00','','15195977581_ZFB',-5.000,'15195977581_ZFB','sh.','',0.000,0),(7,'2000-00-01 00:00:00','','15195977581_ZFB',-5.000,'15195977581_ZFB','sh.','',0.000,0),(8,'2000-00-01 00:00:00','','666600081267',-2.000,'8451990','cash','现有资金',2.000,1),(9,'2000-00-01 00:00:00','','8451990',-5.000,'666600081267','cash','现有资金',5.000,1),(10,'2000-00-01 00:00:00','','8451990',-4.000,'666600081267','cash','现有资金',4.000,1),(11,'2000-00-01 00:00:00','转帐','8451990',-2.000,'666600081267','cash','现有资金',2.000,1),(12,'2000-00-01 00:00:00','股票交易','666600081267',-2105.000,'666600081267','000333','8989798',7.000,300),(13,'2000-00-01 00:00:00','证券卖出','15195977581_ZFB',394.600,'15195977581_ZFB','000333','555',2.000,-200),(14,'2000-00-01 00:00:00','证券卖出','15195977581_ZFB',2192.800,'15195977581_ZFB','sz.000001','平安银行',11.000,-200);
/*!40000 ALTER TABLE `资产变化` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `资产帐户`
--

DROP TABLE IF EXISTS `资产帐户`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `资产帐户` (
  `代号` varchar(32) NOT NULL,
  `名称` varchar(16) NOT NULL,
  `类别` varchar(16) NOT NULL,
  `备注` varchar(32) DEFAULT NULL,
  `pos` int(4) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`代号`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `资产帐户`
--

LOCK TABLES `资产帐户` WRITE;
/*!40000 ALTER TABLE `资产帐户` DISABLE KEYS */;
INSERT INTO `资产帐户` VALUES ('15195977581_ZFB','支付宝','支付宝','rono',1),('6214835191119195','中国招商银行','借记卡','常州',5),('6215584301002835944','中国工商银行','借记卡','南京工资卡',0),('6225758338428617','中国招商银行（信）','信用卡','额度10000',4),('666600081267','华泰证券','券商','南京江宁金箔路证券营业部',3),('8451990','国泰君安','券商','常州广化街证券营业部',2);
/*!40000 ALTER TABLE `资产帐户` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test`
--

DROP TABLE IF EXISTS `test`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `test` (
  `id` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `test`
--

LOCK TABLES `test` WRITE;
/*!40000 ALTER TABLE `test` DISABLE KEYS */;
/*!40000 ALTER TABLE `test` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2016-03-26 11:40:01
