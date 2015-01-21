-- MySQL dump 10.10
--
-- Host: localhost    Database: info_synd_demo
-- ------------------------------------------------------
-- Server version	5.0.27-log

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
-- Table structure for table `synd_access`
--

DROP TABLE IF EXISTS `synd_access`;
CREATE TABLE `synd_access` (
  `ROLE` varchar(64) NOT NULL default '',
  `NODE` varchar(64) NOT NULL default '',
  `PERM` varchar(64) NOT NULL default '',
  PRIMARY KEY  (`ROLE`,`NODE`,`PERM`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_access`
--

LOCK TABLES `synd_access` WRITE;
/*!40000 ALTER TABLE `synd_access` DISABLE KEYS */;
INSERT INTO `synd_access` (`ROLE`, `NODE`, `PERM`) VALUES ('node.role_anonymous.Anonymous','language','read'),('node.role_anonymous.Anonymous','user','read'),('node.role_anonymous.Anonymous','user','recover_password'),('node.role_anonymous.Anonymous','user','signup'),('node.role_authenticated.Authenticated','language','read'),('node.role_authenticated.Authenticated','user','read'),('node.user.1','access','admin');
/*!40000 ALTER TABLE `synd_access` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_access_inherit`
--

DROP TABLE IF EXISTS `synd_access_inherit`;
CREATE TABLE `synd_access_inherit` (
  `NODE` varchar(64) NOT NULL default '',
  `PERM` varchar(64) NOT NULL default '',
  `INHERIT` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`NODE`,`PERM`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_access_inherit`
--

LOCK TABLES `synd_access_inherit` WRITE;
/*!40000 ALTER TABLE `synd_access_inherit` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_access_inherit` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_access_role`
--

DROP TABLE IF EXISTS `synd_access_role`;
CREATE TABLE `synd_access_role` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `INFO_HEAD` text character set utf8 NOT NULL,
  `INFO_DESC` text character set utf8,
  PRIMARY KEY  (`NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_access_role`
--

LOCK TABLES `synd_access_role` WRITE;
/*!40000 ALTER TABLE `synd_access_role` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_access_role` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_alias`
--

DROP TABLE IF EXISTS `synd_alias`;
CREATE TABLE `synd_alias` (
  `SOURCE` varchar(512) NOT NULL,
  `TARGET` varchar(512) NOT NULL,
  `FUZZY_SOURCE` varchar(512) default NULL,
  PRIMARY KEY  (`SOURCE`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_alias`
--

LOCK TABLES `synd_alias` WRITE;
/*!40000 ALTER TABLE `synd_alias` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_alias` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_answer_option`
--

DROP TABLE IF EXISTS `synd_answer_option`;
CREATE TABLE `synd_answer_option` (
  `ATTEMPT_NODE_ID` varchar(64) NOT NULL,
  `OPTION_NODE_ID` varchar(64) NOT NULL,
  PRIMARY KEY  (`ATTEMPT_NODE_ID`,`OPTION_NODE_ID`),
  KEY `ATTEMPT_NODE_ID` (`ATTEMPT_NODE_ID`),
  KEY `OPTION_NODE_ID` (`OPTION_NODE_ID`),
  CONSTRAINT `synd_answer_option_ibfk_1` FOREIGN KEY (`ATTEMPT_NODE_ID`) REFERENCES `synd_attempt` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_answer_option_ibfk_2` FOREIGN KEY (`OPTION_NODE_ID`) REFERENCES `synd_question_option` (`OPTION_NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_answer_option`
--

LOCK TABLES `synd_answer_option` WRITE;
/*!40000 ALTER TABLE `synd_answer_option` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_answer_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_answer_text`
--

DROP TABLE IF EXISTS `synd_answer_text`;
CREATE TABLE `synd_answer_text` (
  `ATTEMPT_NODE_ID` varchar(64) NOT NULL,
  `QUESTION_NODE_ID` varchar(64) NOT NULL,
  `INFO_ANSWER` text character set utf8,
  PRIMARY KEY  (`ATTEMPT_NODE_ID`,`QUESTION_NODE_ID`),
  KEY `ATTEMPT_NODE_ID` (`ATTEMPT_NODE_ID`),
  KEY `QUESTION_NODE_ID` (`QUESTION_NODE_ID`),
  CONSTRAINT `synd_answer_text_ibfk_1` FOREIGN KEY (`ATTEMPT_NODE_ID`) REFERENCES `synd_attempt` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_answer_text_ibfk_2` FOREIGN KEY (`QUESTION_NODE_ID`) REFERENCES `synd_question` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_answer_text`
--

LOCK TABLES `synd_answer_text` WRITE;
/*!40000 ALTER TABLE `synd_answer_text` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_answer_text` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_attempt`
--

DROP TABLE IF EXISTS `synd_attempt`;
CREATE TABLE `synd_attempt` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `CLIENT_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `INFO_CORRECT` int(11) NOT NULL default '0',
  `INFO_ANSWERS` int(11) NOT NULL default '0',
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  KEY `CLIENT_NODE_ID` (`CLIENT_NODE_ID`),
  CONSTRAINT `synd_attempt_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_node_page` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_attempt`
--

LOCK TABLES `synd_attempt` WRITE;
/*!40000 ALTER TABLE `synd_attempt` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_attempt` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_attempt_questions`
--

DROP TABLE IF EXISTS `synd_attempt_questions`;
CREATE TABLE `synd_attempt_questions` (
  `ATTEMPT_NODE_ID` varchar(64) NOT NULL,
  `QUESTION_NODE_ID` varchar(64) NOT NULL,
  `INFO_WEIGHT` int(11) NOT NULL default '0',
  PRIMARY KEY  (`ATTEMPT_NODE_ID`,`QUESTION_NODE_ID`),
  KEY `ATTEMPT_NODE_ID` (`ATTEMPT_NODE_ID`),
  KEY `QUESTION_NODE_ID` (`QUESTION_NODE_ID`),
  CONSTRAINT `synd_attempt_questions_ibfk_1` FOREIGN KEY (`ATTEMPT_NODE_ID`) REFERENCES `synd_attempt` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_attempt_questions_ibfk_2` FOREIGN KEY (`QUESTION_NODE_ID`) REFERENCES `synd_question` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_attempt_questions`
--

LOCK TABLES `synd_attempt_questions` WRITE;
/*!40000 ALTER TABLE `synd_attempt_questions` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_attempt_questions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_class`
--

DROP TABLE IF EXISTS `synd_class`;
CREATE TABLE `synd_class` (
  `NODE_ID` varchar(64) NOT NULL,
  `NAME` text character set utf8,
  `FLAG_LEASE_ONLY` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_class`
--

LOCK TABLES `synd_class` WRITE;
/*!40000 ALTER TABLE `synd_class` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_class` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_class_fields`
--

DROP TABLE IF EXISTS `synd_class_fields`;
CREATE TABLE `synd_class_fields` (
  `FIELD_ID` int(11) NOT NULL default '0',
  `CLASS_NODE_ID` varchar(64) NOT NULL default '',
  `INFO_HEAD` varchar(255) character set utf8 default NULL,
  `INFO_DATATYPE` varchar(64) NOT NULL default 'string',
  PRIMARY KEY  (`FIELD_ID`,`CLASS_NODE_ID`),
  KEY `CLASS_NODE_ID` (`CLASS_NODE_ID`),
  KEY `FIELD_ID` (`FIELD_ID`),
  CONSTRAINT `synd_class_fields_ibfk_1` FOREIGN KEY (`CLASS_NODE_ID`) REFERENCES `synd_class` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_class_fields`
--

LOCK TABLES `synd_class_fields` WRITE;
/*!40000 ALTER TABLE `synd_class_fields` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_class_fields` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_class_values`
--

DROP TABLE IF EXISTS `synd_class_values`;
CREATE TABLE `synd_class_values` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `FIELD_ID` int(11) NOT NULL default '0',
  `VALUE` varchar(255) character set utf8 default NULL,
  PRIMARY KEY  (`NODE_ID`,`FIELD_ID`),
  KEY `FIELD_ID` (`FIELD_ID`),
  CONSTRAINT `synd_class_values_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_instance` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_class_values_ibfk_2` FOREIGN KEY (`FIELD_ID`) REFERENCES `synd_class_fields` (`FIELD_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_class_values`
--

LOCK TABLES `synd_class_values` WRITE;
/*!40000 ALTER TABLE `synd_class_values` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_class_values` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_comment`
--

DROP TABLE IF EXISTS `synd_comment`;
CREATE TABLE `synd_comment` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `CLIENT_NODE_ID` varchar(64) character set utf8 default NULL,
  `CLIENT_NAME` varchar(255) character set utf8 default NULL,
  `CLIENT_HOST` varchar(255) default NULL,
  `TS_CREATE` int(11) default NULL,
  `INFO_HEAD` varchar(255) character set utf8 default NULL,
  `INFO_BODY` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE` (`PARENT_NODE_ID`,`TS_CREATE`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_comment`
--

LOCK TABLES `synd_comment` WRITE;
/*!40000 ALTER TABLE `synd_comment` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_comment` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_costcenter`
--

DROP TABLE IF EXISTS `synd_costcenter`;
CREATE TABLE `synd_costcenter` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `INFO_HEAD` varchar(255) character set utf8 default NULL,
  `INFO_DESC` text character set utf8,
  `INFO_STREET` varchar(255) character set utf8 default NULL,
  `INFO_ZIP` varchar(255) character set utf8 default NULL,
  `INFO_CITY` varchar(255) character set utf8 default NULL,
  `INFO_COUNTRY` varchar(255) character set utf8 default NULL,
  `INFO_EMAIL` varchar(255) character set utf8 default NULL,
  `INFO_PHONE` varchar(255) character set utf8 default NULL,
  `INFO_FAX` varchar(255) character set utf8 default NULL,
  `INFO_NUMBER` varchar(255) character set utf8 default NULL,
  `INFO_PROJECT_CODE` varchar(255) character set utf8 default NULL,
  `INFO_LIABLE` varchar(255) character set utf8 default NULL,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_costcenter`
--

LOCK TABLES `synd_costcenter` WRITE;
/*!40000 ALTER TABLE `synd_costcenter` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_costcenter` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_course`
--

DROP TABLE IF EXISTS `synd_course`;
CREATE TABLE `synd_course` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `PAGE_NODE_ID` varchar(64) default NULL,
  `GROUP_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `INFO_COURSE_ID` varchar(64) default NULL,
  `INFO_STYLESHEET` text character set utf8,
  `FLAG_STYLESHEET` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`NODE_ID`),
  UNIQUE KEY `INFO_COURSE_ID` (`INFO_COURSE_ID`),
  KEY `PAGE_NODE_ID` (`PAGE_NODE_ID`),
  KEY `GROUP_NODE_ID` (`GROUP_NODE_ID`),
  CONSTRAINT `synd_course_ibfk_1` FOREIGN KEY (`PAGE_NODE_ID`) REFERENCES `synd_node_page` (`NODE_ID`) ON DELETE SET NULL,
  CONSTRAINT `synd_course_ibfk_2` FOREIGN KEY (`GROUP_NODE_ID`) REFERENCES `synd_group` (`NODE_ID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_course`
--

LOCK TABLES `synd_course` WRITE;
/*!40000 ALTER TABLE `synd_course` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_course` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_crypto_identity`
--

DROP TABLE IF EXISTS `synd_crypto_identity`;
CREATE TABLE `synd_crypto_identity` (
  `KID` int(11) NOT NULL,
  `INFO_NAME` varchar(255) character set utf8 default NULL,
  `INFO_EMAIL` varchar(255) character set utf8 default NULL,
  KEY `KID` (`KID`),
  KEY `INFO_EMAIL` (`INFO_EMAIL`),
  CONSTRAINT `synd_crypto_identity_ibfk_1` FOREIGN KEY (`KID`) REFERENCES `synd_crypto_key` (`KID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_crypto_identity`
--

LOCK TABLES `synd_crypto_identity` WRITE;
/*!40000 ALTER TABLE `synd_crypto_identity` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_crypto_identity` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_crypto_key`
--

DROP TABLE IF EXISTS `synd_crypto_key`;
CREATE TABLE `synd_crypto_key` (
  `KID` int(11) NOT NULL,
  `PROJECT_NODE_ID` varchar(64) NOT NULL,
  `PROTOCOL` varchar(32) NOT NULL,
  `KEYID` varchar(255) default NULL,
  `FINGERPRINT` varchar(255) default NULL,
  `FLAG_SIGN` tinyint(1) NOT NULL default '0',
  `FLAG_VERIFY` tinyint(1) NOT NULL default '0',
  `FLAG_ENCRYPT` tinyint(1) NOT NULL default '0',
  `FLAG_DECRYPT` tinyint(1) NOT NULL default '0',
  `INFO_TRUST` tinyint(4) NOT NULL default '0',
  `INFO_NAME` varchar(255) character set utf8 default NULL,
  `INFO_EMAIL` varchar(255) character set utf8 default NULL,
  `DATA_CONTENT` text,
  PRIMARY KEY  (`KID`),
  UNIQUE KEY `PROJECT_NODE_ID` (`PROJECT_NODE_ID`,`KEYID`),
  KEY `KEYID` (`KEYID`),
  KEY `FINGERPRINT` (`FINGERPRINT`),
  CONSTRAINT `synd_crypto_key_ibfk_1` FOREIGN KEY (`PROJECT_NODE_ID`) REFERENCES `synd_project` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_crypto_key`
--

LOCK TABLES `synd_crypto_key` WRITE;
/*!40000 ALTER TABLE `synd_crypto_key` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_crypto_key` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_group`
--

DROP TABLE IF EXISTS `synd_group`;
CREATE TABLE `synd_group` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `INFO_HEAD` text character set utf8,
  `INFO_DESC` text character set utf8,
  `INFO_BODY` text character set utf8,
  PRIMARY KEY  (`NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_group`
--

LOCK TABLES `synd_group` WRITE;
/*!40000 ALTER TABLE `synd_group` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_group` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_instance`
--

DROP TABLE IF EXISTS `synd_instance`;
CREATE TABLE `synd_instance` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CLASS_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  PRIMARY KEY  (`NODE_ID`),
  KEY `CLASS_NODE_ID` (`CLASS_NODE_ID`),
  CONSTRAINT `synd_instance_ibfk_1` FOREIGN KEY (`CLASS_NODE_ID`) REFERENCES `synd_class` (`NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_instance`
--

LOCK TABLES `synd_instance` WRITE;
/*!40000 ALTER TABLE `synd_instance` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_instance` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_computer`
--

DROP TABLE IF EXISTS `synd_inv_computer`;
CREATE TABLE `synd_inv_computer` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `INFO_MACHINE_NAME` text character set utf8,
  `INFO_REMOTE_URI` text,
  `INFO_REMOTE_ACTION` text,
  `INFO_REMOTE_METHOD` text,
  `INFO_REMOTE_VERSION` varchar(10) default NULL,
  `INFO_MB_MAKE` text character set utf8,
  `INFO_MB_BIOS` text character set utf8,
  `INFO_MB_SERIAL` text character set utf8,
  `INFO_CPU_DESC` text character set utf8,
  `INFO_CPU_CLOCK` int(11) default NULL,
  `INFO_CPU_COUNT` int(11) default NULL,
  `INFO_CPU_ID` text character set utf8,
  `INFO_RAM` int(11) default NULL,
  `INFO_KEYBOARD` text character set utf8,
  `DATA_DISK_DRIVES` text character set utf8,
  `DATA_ROM_DRIVES` text character set utf8,
  `DATA_MONITORS` text character set utf8,
  `DATA_SOUND_CARDS` text character set utf8,
  `DATA_GRAPHIC_CARDS` text character set utf8,
  `DATA_NETWORK_CARDS` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  CONSTRAINT `synd_inv_computer_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_inv_item` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_computer`
--

LOCK TABLES `synd_inv_computer` WRITE;
/*!40000 ALTER TABLE `synd_inv_computer` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_computer` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_config`
--

DROP TABLE IF EXISTS `synd_inv_config`;
CREATE TABLE `synd_inv_config` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  PRIMARY KEY  (`NODE_ID`),
  UNIQUE KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  CONSTRAINT `synd_inv_config_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_instance` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_config`
--

LOCK TABLES `synd_inv_config` WRITE;
/*!40000 ALTER TABLE `synd_inv_config` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_config` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_folder`
--

DROP TABLE IF EXISTS `synd_inv_folder`;
CREATE TABLE `synd_inv_folder` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `ACCEPT_CLASS_IDS` text,
  `INFO_HEAD` text character set utf8,
  `INFO_DESC` text character set utf8,
  `INFO_BODY` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  CONSTRAINT `synd_inv_folder_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_instance` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_folder`
--

LOCK TABLES `synd_inv_folder` WRITE;
/*!40000 ALTER TABLE `synd_inv_folder` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_folder` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_installation`
--

DROP TABLE IF EXISTS `synd_inv_installation`;
CREATE TABLE `synd_inv_installation` (
  `NODE_ID` varchar(64) NOT NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `INFO_NUMBER` varchar(255) default NULL,
  `DATA_FILES` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  UNIQUE KEY `INFO_NUMBER` (`INFO_NUMBER`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_installation`
--

LOCK TABLES `synd_inv_installation` WRITE;
/*!40000 ALTER TABLE `synd_inv_installation` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_installation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_interface`
--

DROP TABLE IF EXISTS `synd_inv_interface`;
CREATE TABLE `synd_inv_interface` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `NIC_NODE_ID` varchar(64) default NULL,
  `INFO_HOSTNAME` varchar(255) character set utf8 default NULL,
  `INFO_IP_ADDRESS` varchar(15) default NULL,
  `INFO_IP_ENCODED` int(11) unsigned default NULL,
  PRIMARY KEY  (`NODE_ID`),
  UNIQUE KEY `INFO_HOSTNAME` (`INFO_HOSTNAME`),
  UNIQUE KEY `NIC_NODE_ID` (`NIC_NODE_ID`),
  UNIQUE KEY `INFO_IP_ADDRESS` (`INFO_IP_ADDRESS`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  CONSTRAINT `synd_inv_interface_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_inv_config` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_inv_interface_ibfk_2` FOREIGN KEY (`NIC_NODE_ID`) REFERENCES `synd_inv_nic` (`NODE_ID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_interface`
--

LOCK TABLES `synd_inv_interface` WRITE;
/*!40000 ALTER TABLE `synd_inv_interface` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_interface` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_item`
--

DROP TABLE IF EXISTS `synd_inv_item`;
CREATE TABLE `synd_inv_item` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `OWNER_NODE_ID` varchar(64) default NULL,
  `INSTALLATION_NODE_ID` varchar(64) default NULL,
  `COSTCENTER_NODE_ID` varchar(64) default NULL,
  `PROJECT_NODE_ID` varchar(64) default NULL,
  `TS_DELIVERY` int(11) default NULL,
  `INFO_WARRANTY` text character set utf8,
  `INFO_MAKE` text character set utf8,
  `INFO_MODEL` text character set utf8,
  `INFO_SERIAL_MAKER` text character set utf8,
  `INFO_SERIAL_INTERNAL` text character set utf8,
  `INFO_LOCATION` text character set utf8,
  `INFO_COST` int(11) default NULL,
  `INFO_RUNNING_COST` int(11) default NULL,
  `DATA_FILES` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `INSTALLATION_NODE_ID` (`INSTALLATION_NODE_ID`),
  CONSTRAINT `synd_inv_item_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_instance` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_inv_item_ibfk_2` FOREIGN KEY (`INSTALLATION_NODE_ID`) REFERENCES `synd_inv_installation` (`NODE_ID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_item`
--

LOCK TABLES `synd_inv_item` WRITE;
/*!40000 ALTER TABLE `synd_inv_item` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_item` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_lease`
--

DROP TABLE IF EXISTS `synd_inv_lease`;
CREATE TABLE `synd_inv_lease` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `CLIENT_NODE_ID` varchar(64) NOT NULL default '',
  `COSTCENTER_NODE_ID` varchar(64) default NULL,
  `PROJECT_NODE_ID` varchar(64) default NULL,
  `RECEIPT_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `TS_EXPIRE` int(11) default NULL,
  `TS_TERMINATED` int(11) default NULL,
  `INFO_BODY` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  KEY `RECEIPT_NODE_ID` (`RECEIPT_NODE_ID`),
  KEY `COSTCENTER_NODE_ID` (`COSTCENTER_NODE_ID`),
  CONSTRAINT `synd_inv_lease_ibfk_1` FOREIGN KEY (`RECEIPT_NODE_ID`) REFERENCES `synd_node_file` (`NODE_ID`) ON DELETE SET NULL,
  CONSTRAINT `synd_inv_lease_ibfk_2` FOREIGN KEY (`COSTCENTER_NODE_ID`) REFERENCES `synd_costcenter` (`NODE_ID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_lease`
--

LOCK TABLES `synd_inv_lease` WRITE;
/*!40000 ALTER TABLE `synd_inv_lease` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_lease` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_lease_sld`
--

DROP TABLE IF EXISTS `synd_inv_lease_sld`;
CREATE TABLE `synd_inv_lease_sld` (
  `LEASE_NODE_ID` varchar(64) NOT NULL,
  `SLD_NODE_ID` varchar(64) NOT NULL,
  `CREATE_NODE_ID` varchar(64) NOT NULL,
  `TS_CREATE` int(11) NOT NULL,
  `TS_TERMINATED` int(11) default NULL,
  PRIMARY KEY  (`LEASE_NODE_ID`,`SLD_NODE_ID`),
  KEY `SLD_NODE_ID` (`SLD_NODE_ID`,`LEASE_NODE_ID`),
  KEY `TS_CREATE` (`TS_CREATE`,`TS_TERMINATED`),
  CONSTRAINT `synd_inv_lease_sld_ibfk_1` FOREIGN KEY (`LEASE_NODE_ID`) REFERENCES `synd_inv_lease` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_inv_lease_sld_ibfk_2` FOREIGN KEY (`SLD_NODE_ID`) REFERENCES `synd_inv_sld` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_inv_lease_sld`
--

LOCK TABLES `synd_inv_lease_sld` WRITE;
/*!40000 ALTER TABLE `synd_inv_lease_sld` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_lease_sld` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_license`
--

DROP TABLE IF EXISTS `synd_inv_license`;
CREATE TABLE `synd_inv_license` (
  `NODE_ID` varchar(64) default NULL,
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `INFO_MAKE` varchar(255) character set utf8 default NULL,
  `INFO_PRODUCT` varchar(255) character set utf8 default NULL,
  `INFO_DESC` text character set utf8,
  `INFO_COST` int(11) default NULL,
  `INFO_LICENSES` int(11) NOT NULL default '0',
  `FLAG_SITE_LICENSE` tinyint(1) NOT NULL default '0',
  KEY `NODE_ID` (`NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_license`
--

LOCK TABLES `synd_inv_license` WRITE;
/*!40000 ALTER TABLE `synd_inv_license` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_license` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_license_folder`
--

DROP TABLE IF EXISTS `synd_inv_license_folder`;
CREATE TABLE `synd_inv_license_folder` (
  `LICENSE_NODE_ID` varchar(64) NOT NULL,
  `FOLDER_NODE_ID` varchar(64) NOT NULL,
  PRIMARY KEY  (`LICENSE_NODE_ID`,`FOLDER_NODE_ID`),
  KEY `LICENSE_NODE_ID` (`LICENSE_NODE_ID`),
  KEY `FOLDER_NODE_ID` (`FOLDER_NODE_ID`),
  CONSTRAINT `synd_inv_license_folder_ibfk_1` FOREIGN KEY (`LICENSE_NODE_ID`) REFERENCES `synd_inv_license` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_inv_license_folder_ibfk_2` FOREIGN KEY (`FOLDER_NODE_ID`) REFERENCES `synd_inv_folder` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_license_folder`
--

LOCK TABLES `synd_inv_license_folder` WRITE;
/*!40000 ALTER TABLE `synd_inv_license_folder` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_license_folder` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_license_software`
--

DROP TABLE IF EXISTS `synd_inv_license_software`;
CREATE TABLE `synd_inv_license_software` (
  `LICENSE_NODE_ID` varchar(64) NOT NULL,
  `INFO_FILTER` varchar(255) character set utf8 NOT NULL,
  PRIMARY KEY  (`LICENSE_NODE_ID`,`INFO_FILTER`),
  KEY `LICENSE_NODE_ID` (`LICENSE_NODE_ID`),
  CONSTRAINT `synd_inv_license_software_ibfk_1` FOREIGN KEY (`LICENSE_NODE_ID`) REFERENCES `synd_inv_license` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_license_software`
--

LOCK TABLES `synd_inv_license_software` WRITE;
/*!40000 ALTER TABLE `synd_inv_license_software` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_license_software` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_nic`
--

DROP TABLE IF EXISTS `synd_inv_nic`;
CREATE TABLE `synd_inv_nic` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) default NULL,
  `INFO_MAC_ADDRESS` varchar(17) default NULL,
  `INFO_HEAD` varchar(255) character set utf8 default NULL,
  `INFO_LAST_SEEN` int(11) default NULL,
  `INFO_LAST_SWITCH` varchar(255) default NULL,
  `INFO_LAST_SWITCH_PORT` varchar(255) default NULL,
  `INFO_LAST_VLAN` varchar(255) default NULL,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  CONSTRAINT `synd_inv_nic_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_inv_computer` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_nic`
--

LOCK TABLES `synd_inv_nic` WRITE;
/*!40000 ALTER TABLE `synd_inv_nic` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_nic` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_os`
--

DROP TABLE IF EXISTS `synd_inv_os`;
CREATE TABLE `synd_inv_os` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `INFO_IDENTIFIER` varchar(64) character set utf8 default NULL,
  `INFO_RELEASE` text character set utf8,
  `INFO_VERSION` text character set utf8,
  `INFO_SOFTWARE_HASH` varchar(32) default NULL,
  `INFO_MACHINE_NAME` text character set utf8,
  `INFO_LOADED_IMAGE` text character set utf8,
  `INFO_ANTIVIRUS_NAME` text character set utf8,
  `INFO_ANTIVIRUS_DATE` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  CONSTRAINT `synd_inv_os_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_inv_computer` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_os`
--

LOCK TABLES `synd_inv_os` WRITE;
/*!40000 ALTER TABLE `synd_inv_os` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_os` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_peripheral`
--

DROP TABLE IF EXISTS `synd_inv_peripheral`;
CREATE TABLE `synd_inv_peripheral` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `INFO_COST` int(11) default NULL,
  `INFO_DESC` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  CONSTRAINT `synd_inv_peripheral_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_instance` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_inv_peripheral`
--

LOCK TABLES `synd_inv_peripheral` WRITE;
/*!40000 ALTER TABLE `synd_inv_peripheral` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_peripheral` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_sld`
--

DROP TABLE IF EXISTS `synd_inv_sld`;
CREATE TABLE `synd_inv_sld` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `INFO_HEAD` varchar(255) character set utf8 NOT NULL,
  `INFO_DESC` text character set utf8,
  `INFO_URI` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  CONSTRAINT `synd_inv_sld_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_inv_folder` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_inv_sld`
--

LOCK TABLES `synd_inv_sld` WRITE;
/*!40000 ALTER TABLE `synd_inv_sld` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_sld` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_software`
--

DROP TABLE IF EXISTS `synd_inv_software`;
CREATE TABLE `synd_inv_software` (
  `OS_NODE_ID` varchar(64) NOT NULL default '',
  `INFO_PRODUCT` text character set utf8,
  `INFO_VERSION` text character set utf8,
  `INFO_STATE` varchar(255) default NULL,
  KEY `OS_NODE_ID` (`OS_NODE_ID`),
  CONSTRAINT `synd_inv_software_ibfk_1` FOREIGN KEY (`OS_NODE_ID`) REFERENCES `synd_inv_os` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_software`
--

LOCK TABLES `synd_inv_software` WRITE;
/*!40000 ALTER TABLE `synd_inv_software` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_software` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_used`
--

DROP TABLE IF EXISTS `synd_inv_used`;
CREATE TABLE `synd_inv_used` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CHILD_NODE_ID` varchar(64) default NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `TS_EXPIRE` int(11) default NULL,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`,`TS_EXPIRE`),
  KEY `CHILD_NODE_ID` (`CHILD_NODE_ID`,`TS_EXPIRE`),
  CONSTRAINT `synd_inv_used_ibfk_1` FOREIGN KEY (`CHILD_NODE_ID`) REFERENCES `synd_inv_item` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_used`
--

LOCK TABLES `synd_inv_used` WRITE;
/*!40000 ALTER TABLE `synd_inv_used` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_used` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_vlan`
--

DROP TABLE IF EXISTS `synd_inv_vlan`;
CREATE TABLE `synd_inv_vlan` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `INFO_HEAD` varchar(255) character set utf8 NOT NULL,
  `INFO_DESC` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  CONSTRAINT `synd_inv_vlan_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_inv_folder` (`NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_vlan`
--

LOCK TABLES `synd_inv_vlan` WRITE;
/*!40000 ALTER TABLE `synd_inv_vlan` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_vlan` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_inv_vlan_network`
--

DROP TABLE IF EXISTS `synd_inv_vlan_network`;
CREATE TABLE `synd_inv_vlan_network` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `INFO_NETWORK_ADDRESS` varchar(15) NOT NULL,
  `INFO_NETWORK_MASK` varchar(15) NOT NULL,
  `INFO_ENCODED_NET` int(11) unsigned NOT NULL,
  `INFO_ENCODED_MASK` int(11) unsigned NOT NULL,
  PRIMARY KEY  (`NODE_ID`),
  UNIQUE KEY `INFO_ENCODED_NET` (`INFO_ENCODED_NET`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  CONSTRAINT `synd_inv_vlan_network_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_inv_vlan` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_inv_vlan_network`
--

LOCK TABLES `synd_inv_vlan_network` WRITE;
/*!40000 ALTER TABLE `synd_inv_vlan_network` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_inv_vlan_network` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_issue`
--

DROP TABLE IF EXISTS `synd_issue`;
CREATE TABLE `synd_issue` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `ISSUE_NODE_ID` varchar(64) default NULL,
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CREATE_NODE_ID` varchar(255) character set utf8 default NULL,
  `UPDATE_NODE_ID` varchar(255) character set utf8 default NULL,
  `ASSIGNED_NODE_ID` varchar(64) default NULL,
  `CLIENT_NODE_ID` varchar(255) character set utf8 default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `TS_ASSIGNED` int(11) default NULL,
  `TS_START` int(11) default NULL,
  `TS_RESOLVE` int(11) default NULL,
  `TS_RESOLVE_BY` int(11) default NULL,
  `INFO_ISSUE_ID` int(11) unsigned NOT NULL default '0',
  `INFO_STATUS` tinyint(4) unsigned NOT NULL default '0',
  `INFO_PRIO` tinyint(4) NOT NULL default '1',
  `INFO_HEAD` text character set utf8 NOT NULL,
  `INFO_ESTIMATE` int(11) default NULL,
  `INFO_PRIVATE_KEY` varchar(32) default NULL,
  `INFO_INITIAL_QUERY` varchar(255) default NULL,
  `DATA_CONTENT` text character set utf8,
  `DATA_FILES` text character set utf8,
  `DATA_NOTIFIER` text character set utf8,
  `DATA_EVENTLOG` text character set utf8,
  `DATA_ATTRIBUTES` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  UNIQUE KEY `INFO_ISSUE_ID` (`INFO_ISSUE_ID`),
  KEY `ISSUE_NODE_ID` (`ISSUE_NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`,`INFO_STATUS`,`TS_CREATE`),
  KEY `CREATE_NODE_ID` (`CREATE_NODE_ID`),
  KEY `UPDATE_NODE_ID` (`UPDATE_NODE_ID`),
  KEY `ASSIGNED_NODE_ID` (`ASSIGNED_NODE_ID`,`INFO_STATUS`),
  KEY `CLIENT_NODE_ID` (`CLIENT_NODE_ID`(32),`INFO_STATUS`,`TS_RESOLVE`),
  CONSTRAINT `synd_issue_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_project` (`NODE_ID`),
  CONSTRAINT `synd_issue_ibfk_2` FOREIGN KEY (`ISSUE_NODE_ID`) REFERENCES `synd_issue` (`NODE_ID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_issue`
--

LOCK TABLES `synd_issue` WRITE;
/*!40000 ALTER TABLE `synd_issue` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_issue` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_issue_feedback`
--

DROP TABLE IF EXISTS `synd_issue_feedback`;
CREATE TABLE `synd_issue_feedback` (
  `ISSUE_NODE_ID` varchar(64) NOT NULL,
  `AUTHENTICATION_TOKEN` varchar(32) NOT NULL,
  `INFO_RATING` tinyint(4) NOT NULL default '0',
  `TS_CREATE` int(11) default NULL,
  PRIMARY KEY  (`ISSUE_NODE_ID`,`AUTHENTICATION_TOKEN`),
  KEY `ISSUE_NODE_ID` (`ISSUE_NODE_ID`),
  CONSTRAINT `synd_issue_feedback_ibfk_1` FOREIGN KEY (`ISSUE_NODE_ID`) REFERENCES `synd_issue` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_issue_feedback`
--

LOCK TABLES `synd_issue_feedback` WRITE;
/*!40000 ALTER TABLE `synd_issue_feedback` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_issue_feedback` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_issue_invoice`
--

DROP TABLE IF EXISTS `synd_issue_invoice`;
CREATE TABLE `synd_issue_invoice` (
  `NODE_ID` varchar(64) NOT NULL,
  `LEASE_NODE_ID` varchar(64) default NULL,
  `RECEIPT_NODE_ID` varchar(64) default NULL,
  `TS_PAID` int(11) default NULL,
  `INFO_AMOUNT_TAXED` int(11) default NULL,
  `INFO_AMOUNT_UNTAXED` int(11) default NULL,
  `INFO_INVOICE_NUMBER` varchar(64) default NULL,
  PRIMARY KEY  (`NODE_ID`),
  KEY `synd_issue_invoice_ibfk_2` (`LEASE_NODE_ID`),
  CONSTRAINT `synd_issue_invoice_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_issue` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_issue_invoice_ibfk_2` FOREIGN KEY (`LEASE_NODE_ID`) REFERENCES `synd_inv_lease` (`NODE_ID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_issue_invoice`
--

LOCK TABLES `synd_issue_invoice` WRITE;
/*!40000 ALTER TABLE `synd_issue_invoice` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_issue_invoice` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_issue_keyword`
--

DROP TABLE IF EXISTS `synd_issue_keyword`;
CREATE TABLE `synd_issue_keyword` (
  `KEYWORD_NODE_ID` varchar(64) NOT NULL,
  `ISSUE_NODE_ID` varchar(64) NOT NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  PRIMARY KEY  (`KEYWORD_NODE_ID`,`ISSUE_NODE_ID`),
  KEY `KEYWORD_NODE_ID` (`KEYWORD_NODE_ID`),
  KEY `ISSUE_NODE_ID` (`ISSUE_NODE_ID`),
  CONSTRAINT `synd_issue_keyword_ibfk_1` FOREIGN KEY (`KEYWORD_NODE_ID`) REFERENCES `synd_keyword` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_issue_keyword_ibfk_2` FOREIGN KEY (`ISSUE_NODE_ID`) REFERENCES `synd_issue` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_issue_keyword`
--

LOCK TABLES `synd_issue_keyword` WRITE;
/*!40000 ALTER TABLE `synd_issue_keyword` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_issue_keyword` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_issue_leasing`
--

DROP TABLE IF EXISTS `synd_issue_leasing`;
CREATE TABLE `synd_issue_leasing` (
  `ISSUE_NODE_ID` varchar(64) NOT NULL,
  `LEASING_NODE_ID` varchar(64) NOT NULL,
  PRIMARY KEY  (`ISSUE_NODE_ID`,`LEASING_NODE_ID`),
  KEY `synd_issue_leasing_ibfk_2` (`LEASING_NODE_ID`),
  CONSTRAINT `synd_issue_leasing_ibfk_1` FOREIGN KEY (`ISSUE_NODE_ID`) REFERENCES `synd_issue` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_issue_leasing_ibfk_2` FOREIGN KEY (`LEASING_NODE_ID`) REFERENCES `synd_inv_used` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_issue_leasing`
--

LOCK TABLES `synd_issue_leasing` WRITE;
/*!40000 ALTER TABLE `synd_issue_leasing` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_issue_leasing` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_issue_repair`
--

DROP TABLE IF EXISTS `synd_issue_repair`;
CREATE TABLE `synd_issue_repair` (
  `NODE_ID` varchar(64) NOT NULL,
  `FLAG_NO_WARRANTY` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`NODE_ID`),
  CONSTRAINT `synd_issue_repair_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_issue` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_issue_repair`
--

LOCK TABLES `synd_issue_repair` WRITE;
/*!40000 ALTER TABLE `synd_issue_repair` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_issue_repair` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_issue_replaced`
--

DROP TABLE IF EXISTS `synd_issue_replaced`;
CREATE TABLE `synd_issue_replaced` (
  `NODE_ID` varchar(64) NOT NULL,
  `REPLACEMENT_NODE_ID` varchar(64) default NULL,
  PRIMARY KEY  (`NODE_ID`),
  KEY `REPLACEMENT_NODE_ID` (`REPLACEMENT_NODE_ID`),
  CONSTRAINT `synd_issue_replaced_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_relation` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_issue_replaced_ibfk_2` FOREIGN KEY (`REPLACEMENT_NODE_ID`) REFERENCES `synd_instance` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_issue_replaced`
--

LOCK TABLES `synd_issue_replaced` WRITE;
/*!40000 ALTER TABLE `synd_issue_replaced` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_issue_replaced` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_issue_task`
--

DROP TABLE IF EXISTS `synd_issue_task`;
CREATE TABLE `synd_issue_task` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CREATE_NODE_ID` varchar(255) default NULL,
  `UPDATE_NODE_ID` varchar(255) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `INFO_DURATION` int(11) default NULL,
  `FLAG_PROTECTED` tinyint(1) NOT NULL default '0',
  `DATA_CONTENT` text character set utf8,
  `DATA_FILES` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  KEY `CREATE_NODE_ID` (`CREATE_NODE_ID`, `TS_CREATE`),
  CONSTRAINT `synd_issue_task_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_issue` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_issue_task`
--

LOCK TABLES `synd_issue_task` WRITE;
/*!40000 ALTER TABLE `synd_issue_task` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_issue_task` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_keyword`
--

DROP TABLE IF EXISTS `synd_keyword`;
CREATE TABLE `synd_keyword` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `INFO_HEAD` varchar(255) character set utf8 NOT NULL,
  `INFO_DESC` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_keyword`
--

LOCK TABLES `synd_keyword` WRITE;
/*!40000 ALTER TABLE `synd_keyword` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_keyword` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_locale_gram`
--

DROP TABLE IF EXISTS `synd_locale_gram`;
CREATE TABLE `synd_locale_gram` (
  `LOCALE` varchar(5) NOT NULL,
  `GRAM` varchar(32) character set utf8 NOT NULL,
  `FREQUENCY` int(11) NOT NULL default '1',
  PRIMARY KEY  (`LOCALE`,`GRAM`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_locale_gram`
--

LOCK TABLES `synd_locale_gram` WRITE;
/*!40000 ALTER TABLE `synd_locale_gram` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_locale_gram` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_locale_string`
--

DROP TABLE IF EXISTS `synd_locale_string`;
CREATE TABLE `synd_locale_string` (
  `LID` varchar(32) NOT NULL default '',
  `LOCATION` varchar(255) character set utf8 default NULL,
  `STRING` text character set utf8 NOT NULL,
  PRIMARY KEY  (`LID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_locale_string`
--

LOCK TABLES `synd_locale_string` WRITE;
/*!40000 ALTER TABLE `synd_locale_string` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_locale_string` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_locale_translation`
--

DROP TABLE IF EXISTS `synd_locale_translation`;
CREATE TABLE `synd_locale_translation` (
  `LID` varchar(32) NOT NULL default '',
  `LOCALE` varchar(32) NOT NULL default '',
  `TRANSLATION` text character set utf8,
  PRIMARY KEY  (`LID`,`LOCALE`),
  KEY `LOCALE` (`LOCALE`),
  CONSTRAINT `synd_locale_translation_ibfk_1` FOREIGN KEY (`LID`) REFERENCES `synd_locale_string` (`LID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_locale_translation`
--

LOCK TABLES `synd_locale_translation` WRITE;
/*!40000 ALTER TABLE `synd_locale_translation` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_locale_translation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_log`
--

DROP TABLE IF EXISTS `synd_log`;
CREATE TABLE `synd_log` (
  `HASH` varchar(32) NOT NULL default '',
  `TIMESTAMP` int(11) default NULL,
  `FILENAME` text character set utf8,
  `LINE` int(11) default NULL,
  `STATUS` int(11) default NULL,
  `CODE` int(11) default NULL,
  `MESSAGE` text character set utf8,
  `DESCRIPTION` text character set utf8,
  `CONTEXT` text character set utf8,
  `STACKTRACE` text character set utf8,
  `CLIENT_NODE_ID` varchar(255) character set utf8 default NULL,
  `REQUEST_URI` text character set utf8,
  `REQUEST_DATA` text character set utf8,
  `REQUEST_ENV` text character set utf8,
  PRIMARY KEY  (`HASH`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_log`
--

LOCK TABLES `synd_log` WRITE;
/*!40000 ALTER TABLE `synd_log` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_log` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_node`
--

DROP TABLE IF EXISTS `synd_node`;
CREATE TABLE `synd_node` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(255) default NULL,
  `CREATE_NODE_ID` varchar(255) default NULL,
  `UPDATE_NODE_ID` varchar(255) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `FLAG_MODERATE` tinyint(1) NOT NULL default '0',
  `FLAG_PROMOTE` tinyint(1) NOT NULL default '0',
  `FLAG_PINGBACKS` tinyint(1) NOT NULL default '0',
  `INFO_LANG` varchar(255) default NULL,
  `DATA_PINGS` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `LIST` (`TS_CREATE`,`FLAG_MODERATE`,`FLAG_PROMOTE`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_node`
--

LOCK TABLES `synd_node` WRITE;
/*!40000 ALTER TABLE `synd_node` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_node` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_node_file`
--

DROP TABLE IF EXISTS `synd_node_file`;
CREATE TABLE `synd_node_file` (
  `NODE_ID` varchar(64) NOT NULL,
  `INFO_WEIGHT` int(11) NOT NULL default '0',
  `INFO_HEAD` varchar(255) character set utf8 default NULL,
  `INFO_DESC` text character set utf8,
  `DATA_FILE` text character set utf8,
  `DATA_IMAGE` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  CONSTRAINT `synd_node_file_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_node` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_node_file`
--

LOCK TABLES `synd_node_file` WRITE;
/*!40000 ALTER TABLE `synd_node_file` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_node_file` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_node_page`
--

DROP TABLE IF EXISTS `synd_node_page`;
CREATE TABLE `synd_node_page` (
  `NODE_ID` varchar(64) NOT NULL,
  `INFO_PAGE_ID` varchar(64) default NULL,
  `INFO_KEYWORDS` text character set utf8,
  `INFO_HEAD` text character set utf8,
  `INFO_DESC` text character set utf8,
  `INFO_BODY` text character set utf8,
  `INFO_WEIGHT` int(11) NOT NULL default '0',
  `FLAG_PROGRESS` tinyint(1) NOT NULL default '0',
  `FLAG_DIAGNOSTIC` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`NODE_ID`),
  CONSTRAINT `synd_node_page_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_node` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_node_page`
--

LOCK TABLES `synd_node_page` WRITE;
/*!40000 ALTER TABLE `synd_node_page` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_node_page` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_node_user`
--

DROP TABLE IF EXISTS `synd_node_user`;
CREATE TABLE `synd_node_user` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `USERNAME` varchar(255) character set utf8 NOT NULL default '',
  `PASSWORD` varchar(32) NOT NULL default '',
  `TS_LOGIN` int(11) default NULL,
  `TS_ACTIVITY` int(11) default NULL,
  `INFO_EMAIL` varchar(255) character set utf8 default NULL,
  `INFO_HEAD` varchar(255) character set utf8 default NULL,
  `INFO_DESC` text character set utf8,
  `INFO_BODY` text character set utf8,
  `INFO_PHOTO` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  UNIQUE KEY `USERNAME` (`USERNAME`),
  CONSTRAINT `synd_node_user_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_node` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_node_user`
--

LOCK TABLES `synd_node_user` WRITE;
/*!40000 ALTER TABLE `synd_node_user` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_node_user` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_plan_budgeted`
--

DROP TABLE IF EXISTS `synd_plan_budgeted`;
CREATE TABLE `synd_plan_budgeted` (
  `NODE_ID` varchar(64) NOT NULL,
  `RESOURCE_NODE_ID` varchar(64) default NULL,
  `PROJECT_NODE_ID` varchar(64) NOT NULL,
  `PERIOD_NODE_ID` varchar(64) NOT NULL,
  `INFO_AMOUNT` int(11) NOT NULL,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PERIOD_NODE_ID` (`PERIOD_NODE_ID`),
  KEY `PROJECT_NODE_ID` (`PROJECT_NODE_ID`),
  KEY `RESOURCE_NODE_ID` (`RESOURCE_NODE_ID`),
  CONSTRAINT `synd_plan_budgeted_ibfk_1` FOREIGN KEY (`RESOURCE_NODE_ID`) REFERENCES `synd_plan_resource` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_plan_budgeted_ibfk_2` FOREIGN KEY (`PROJECT_NODE_ID`) REFERENCES `synd_project` (`NODE_ID`) ON DELETE CASCADE,
  CONSTRAINT `synd_plan_budgeted_ibfk_3` FOREIGN KEY (`PERIOD_NODE_ID`) REFERENCES `synd_plan_period` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_plan_budgeted`
--

LOCK TABLES `synd_plan_budgeted` WRITE;
/*!40000 ALTER TABLE `synd_plan_budgeted` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_plan_budgeted` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_plan_period`
--

DROP TABLE IF EXISTS `synd_plan_period`;
CREATE TABLE `synd_plan_period` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `TS_START` int(11) NOT NULL,
  `TS_STOP` int(11) NOT NULL,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`),
  CONSTRAINT `synd_plan_period_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_project` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_plan_period`
--

LOCK TABLES `synd_plan_period` WRITE;
/*!40000 ALTER TABLE `synd_plan_period` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_plan_period` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_plan_person`
--

DROP TABLE IF EXISTS `synd_plan_person`;
CREATE TABLE `synd_plan_person` (
  `NODE_ID` varchar(64) NOT NULL,
  `INFO_PHONE` varchar(512) character set utf8 default NULL,
  `INFO_EMAIL` varchar(512) character set utf8 default NULL,
  `INFO_HANDLE` varchar(512) character set utf8 default NULL,
  PRIMARY KEY  (`NODE_ID`),
  CONSTRAINT `synd_plan_person_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_plan_resource` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_plan_person`
--

LOCK TABLES `synd_plan_person` WRITE;
/*!40000 ALTER TABLE `synd_plan_person` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_plan_person` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_plan_reported`
--

DROP TABLE IF EXISTS `synd_plan_reported`;
CREATE TABLE `synd_plan_reported` (
  `NODE_ID` varchar(64) NOT NULL,
  `PROJECT_NODE_ID` varchar(64) NOT NULL,
  `TS_START` int(11) NOT NULL,
  `TS_STOP` int(11) NOT NULL,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PROJECT_NODE_ID` (`PROJECT_NODE_ID`),
  CONSTRAINT `synd_plan_reported_ibfk_1` FOREIGN KEY (`PROJECT_NODE_ID`) REFERENCES `synd_project` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_plan_reported`
--

LOCK TABLES `synd_plan_reported` WRITE;
/*!40000 ALTER TABLE `synd_plan_reported` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_plan_reported` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_plan_resource`
--

DROP TABLE IF EXISTS `synd_plan_resource`;
CREATE TABLE `synd_plan_resource` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `INFO_HEAD` varchar(512) character set utf8 NOT NULL,
  `INFO_DESC` text character set utf8,
  `INFO_HOURLY_RATE` int(11) NOT NULL default '0',
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_plan_resource`
--

LOCK TABLES `synd_plan_resource` WRITE;
/*!40000 ALTER TABLE `synd_plan_resource` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_plan_resource` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_poll`
--

DROP TABLE IF EXISTS `synd_poll`;
CREATE TABLE `synd_poll` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `PAGE_NODE_ID` varchar(64) default NULL,
  `ERROR_NODE_ID` varchar(64) default NULL,
  `CONFIRM_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `TS_START` int(11) default NULL,
  `TS_STOP` int(11) default NULL,
  `INFO_REDIRECT` text character set utf8,
  `FLAG_PROMOTE` tinyint(1) NOT NULL default '0',
  `FLAG_ANONYMOUS` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`NODE_ID`),
  KEY `PAGE_NODE_ID` (`PAGE_NODE_ID`),
  KEY `ERROR_NODE_ID` (`ERROR_NODE_ID`),
  KEY `CONFIRM_NODE_ID` (`CONFIRM_NODE_ID`),
  CONSTRAINT `synd_poll_ibfk_1` FOREIGN KEY (`PAGE_NODE_ID`) REFERENCES `synd_node_page` (`NODE_ID`) ON DELETE SET NULL,
  CONSTRAINT `synd_poll_ibfk_2` FOREIGN KEY (`ERROR_NODE_ID`) REFERENCES `synd_node_page` (`NODE_ID`) ON DELETE SET NULL,
  CONSTRAINT `synd_poll_ibfk_3` FOREIGN KEY (`CONFIRM_NODE_ID`) REFERENCES `synd_node_page` (`NODE_ID`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_poll`
--

LOCK TABLES `synd_poll` WRITE;
/*!40000 ALTER TABLE `synd_poll` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_poll` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_project`
--

DROP TABLE IF EXISTS `synd_project`;
CREATE TABLE `synd_project` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `PARENT_NODE_ID` varchar(64) NOT NULL default 'null.issue',
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `INFO_PROJECT_ID` varchar(255) default NULL,
  `INFO_PROJECT_NUMBER` varchar(255) default NULL,
  `INFO_COST_CENTER` varchar(255) character set utf8 default NULL,
  `INFO_HEAD` varchar(255) character set utf8 default NULL,
  `INFO_DESC` text character set utf8,
  `INFO_EMAIL` varchar(255) character set utf8 default NULL,
  `INFO_DEFAULT_CLIENT` varchar(255) character set utf8 default NULL,
  `INFO_DEFAULT_RESOLVE` varchar(64) character set utf8 default '+7 days',
  `FLAG_INHERIT_MEMBERS` tinyint(1) NOT NULL default '1',
  `FLAG_INHERIT_CATEGORIES` tinyint(1) NOT NULL default '1',
  `FLAG_DISPLAY_SENDER` tinyint(1) NOT NULL default '1',
  `FLAG_ISSUE_SENDER` tinyint(1) NOT NULL default '1',
  `FLAG_DISCARD_SPAM` tinyint(1) NOT NULL default '1',
  `FLAG_HIDE_ISSUES` tinyint(1) NOT NULL default '0',
  `FLAG_RECEIPT` tinyint(1) NOT NULL default '1',
  `FLAG_ARCHIVE` tinyint(1) NOT NULL default '0',
  `DATA_ATTRIBUTES` text character set utf8,
  `DATA_NOTIFIER` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_project`
--

LOCK TABLES `synd_project` WRITE;
/*!40000 ALTER TABLE `synd_project` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_project` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_project_mapping`
--

DROP TABLE IF EXISTS `synd_project_mapping`;
CREATE TABLE `synd_project_mapping` (
  `PROJECT_NODE_ID` varchar(64) NOT NULL,
  `CUSTOMER_NODE_ID` varchar(64) NOT NULL,
  `QUERY` varchar(255) NOT NULL,
  PRIMARY KEY  (`PROJECT_NODE_ID`,`CUSTOMER_NODE_ID`,`QUERY`),
  KEY `PROJECT_NODE_ID` (`PROJECT_NODE_ID`),
  KEY `QUERY` (`QUERY`),
  CONSTRAINT `synd_project_mapping_ibfk_1` FOREIGN KEY (`PROJECT_NODE_ID`) REFERENCES `synd_project` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_project_mapping`
--

LOCK TABLES `synd_project_mapping` WRITE;
/*!40000 ALTER TABLE `synd_project_mapping` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_project_mapping` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_project_member`
--

DROP TABLE IF EXISTS `synd_project_member`;
CREATE TABLE `synd_project_member` (
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `CHILD_NODE_ID` varchar(64) NOT NULL,
  PRIMARY KEY  (`PARENT_NODE_ID`,`CHILD_NODE_ID`),
  CONSTRAINT `synd_project_member_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_project` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_project_member`
--

LOCK TABLES `synd_project_member` WRITE;
/*!40000 ALTER TABLE `synd_project_member` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_project_member` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_project_template`
--

DROP TABLE IF EXISTS `synd_project_template`;
CREATE TABLE `synd_project_template` (
  `PROJECT_NODE_ID` varchar(64) NOT NULL default '',
  `TEMPLATE_ID` varchar(64) NOT NULL default '',
  `LOCALE` varchar(32) NOT NULL default '',
  `DATA_CONTENTS` text character set utf8,
  PRIMARY KEY  (`PROJECT_NODE_ID`,`TEMPLATE_ID`,`LOCALE`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_project_template`
--

LOCK TABLES `synd_project_template` WRITE;
/*!40000 ALTER TABLE `synd_project_template` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_project_template` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_question`
--

DROP TABLE IF EXISTS `synd_question`;
CREATE TABLE `synd_question` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `INFO_WEIGHT` int(11) NOT NULL default '0',
  `INFO_THRESHOLD` int(11) NOT NULL default '1',
  `INFO_LAYOUT` varchar(32) NOT NULL,
  `INFO_QUESTION` text character set utf8,
  `INFO_CORRECT_ANSWER` text character set utf8,
  `INFO_CORRECT_EXPLANATION` text character set utf8,
  `INFO_INCORRECT_EXPLANATION` text character set utf8,
  `FLAG_DIAGNOSTIC` tinyint(1) NOT NULL default '0',
  `FLAG_PROGRESS` tinyint(1) NOT NULL default '0',
  `FLAG_CASE_SENSITIVE` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_question`
--

LOCK TABLES `synd_question` WRITE;
/*!40000 ALTER TABLE `synd_question` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_question` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_question_option`
--

DROP TABLE IF EXISTS `synd_question_option`;
CREATE TABLE `synd_question_option` (
  `OPTION_NODE_ID` varchar(64) NOT NULL,
  `QUESTION_NODE_ID` varchar(64) default NULL,
  `INFO_OPTION` text character set utf8,
  `INFO_WEIGHT` int(11) default NULL,
  PRIMARY KEY  (`OPTION_NODE_ID`),
  KEY `QUESTION_NODE_ID` (`QUESTION_NODE_ID`),
  CONSTRAINT `synd_question_option_ibfk_1` FOREIGN KEY (`QUESTION_NODE_ID`) REFERENCES `synd_question` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_question_option`
--

LOCK TABLES `synd_question_option` WRITE;
/*!40000 ALTER TABLE `synd_question_option` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_question_option` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_relation`
--

DROP TABLE IF EXISTS `synd_relation`;
CREATE TABLE `synd_relation` (
  `NODE_ID` varchar(64) NOT NULL default '',
  `PARENT_NODE_ID` varchar(64) NOT NULL default '',
  `CHILD_NODE_ID` varchar(64) NOT NULL default '',
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `FLAG_CASCADE_DELETE` tinyint(1) NOT NULL default '0',
  `FLAG_CASCADE_PERMS` tinyint(1) NOT NULL default '0',
  `INFO_WEIGHT` int(11) NOT NULL default '0',
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`,`CHILD_NODE_ID`),
  KEY `CHILD_NODE_ID` (`CHILD_NODE_ID`,`PARENT_NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_relation`
--

LOCK TABLES `synd_relation` WRITE;
/*!40000 ALTER TABLE `synd_relation` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_relation` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_attribute`
--

DROP TABLE IF EXISTS `synd_search_attribute`;
CREATE TABLE `synd_search_attribute` (
  `DOCID` int(11) NOT NULL,
  `ATTRIBUTE` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`DOCID`,`ATTRIBUTE`),
  CONSTRAINT `synd_search_attribute_ibfk_1` FOREIGN KEY (`DOCID`) REFERENCES `synd_search_document` (`DOCID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_search_attribute`
--

LOCK TABLES `synd_search_attribute` WRITE;
/*!40000 ALTER TABLE `synd_search_attribute` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_search_attribute` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_cosine`
--

DROP TABLE IF EXISTS `synd_search_cosine`;
CREATE TABLE `synd_search_cosine` (
  `DOCID` int(11) NOT NULL,
  `WEIGHT` smallint(6) unsigned default NULL,
  `MAXWDF` smallint(6) unsigned default NULL,
  PRIMARY KEY  (`DOCID`),
  CONSTRAINT `synd_search_cosine_ibfk_1` FOREIGN KEY (`DOCID`) REFERENCES `synd_search_document` (`DOCID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_search_cosine`
--

LOCK TABLES `synd_search_cosine` WRITE;
/*!40000 ALTER TABLE `synd_search_cosine` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_search_cosine` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_document`
--

DROP TABLE IF EXISTS `synd_search_document`;
CREATE TABLE `synd_search_document` (
  `DOCID` int(11) NOT NULL,
  `PAGEID` text character set utf8 NOT NULL,
  `SECTION` varchar(255) NOT NULL default '',
  `MODIFIED` int(11) unsigned NOT NULL default '0',
  `NDL` float(6,3) NOT NULL default '1.000',
  `LENGTH` smallint(6) unsigned NOT NULL default '1',
  `RANK` smallint(6) unsigned NOT NULL default '1',
  `LINKS` tinyint(4) unsigned NOT NULL default '1',
  PRIMARY KEY  (`DOCID`),
  KEY `SECTION` (`SECTION`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_search_document`
--

LOCK TABLES `synd_search_document` WRITE;
/*!40000 ALTER TABLE `synd_search_document` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_search_document` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_field`
--

DROP TABLE IF EXISTS `synd_search_field`;
CREATE TABLE `synd_search_field` (
  `DOCID` int(11) NOT NULL,
  `FIELD` tinyint(4) unsigned NOT NULL,
  `CONTENT` varchar(10) character set utf8 NOT NULL default '',
  PRIMARY KEY  (`DOCID`,`FIELD`),
  CONSTRAINT `synd_search_field_ibfk_1` FOREIGN KEY (`DOCID`) REFERENCES `synd_search_document` (`DOCID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_search_field`
--

LOCK TABLES `synd_search_field` WRITE;
/*!40000 ALTER TABLE `synd_search_field` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_search_field` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_link`
--

DROP TABLE IF EXISTS `synd_search_link`;
CREATE TABLE `synd_search_link` (
  `TARGET` int(11) NOT NULL,
  `SOURCE` int(11) NOT NULL,
  PRIMARY KEY  (`TARGET`,`SOURCE`),
  UNIQUE KEY `SOURCE` (`SOURCE`,`TARGET`),
  CONSTRAINT `synd_search_link_ibfk_1` FOREIGN KEY (`TARGET`) REFERENCES `synd_search_document` (`DOCID`) ON DELETE CASCADE,
  CONSTRAINT `synd_search_link_ibfk_2` FOREIGN KEY (`SOURCE`) REFERENCES `synd_search_document` (`DOCID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_search_link`
--

LOCK TABLES `synd_search_link` WRITE;
/*!40000 ALTER TABLE `synd_search_link` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_search_link` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_numeric`
--

DROP TABLE IF EXISTS `synd_search_numeric`;
CREATE TABLE `synd_search_numeric` (
  `TERMID` int(11) NOT NULL,
  `DOCID` int(11) NOT NULL,
  `TERM` bigint(20) NOT NULL,
  PRIMARY KEY  (`TERMID`,`DOCID`),
  KEY `TERM` (`TERM`),
  CONSTRAINT `synd_search_numeric_ibfk_1` FOREIGN KEY (`TERMID`, `DOCID`) REFERENCES `synd_search_termindex` (`TERMID`, `DOCID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_search_numeric`
--

LOCK TABLES `synd_search_numeric` WRITE;
/*!40000 ALTER TABLE `synd_search_numeric` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_search_numeric` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_queue`
--

DROP TABLE IF EXISTS `synd_search_queue`;
CREATE TABLE `synd_search_queue` (
  `NAMESPACE` varchar(64) NOT NULL,
  `NODE_ID` varchar(64) default NULL,
  `BACKENDS` tinyint(4) unsigned NOT NULL default '0'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_search_queue`
--

LOCK TABLES `synd_search_queue` WRITE;
/*!40000 ALTER TABLE `synd_search_queue` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_search_queue` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_spider`
--

DROP TABLE IF EXISTS `synd_search_spider`;
CREATE TABLE `synd_search_spider` (
  `DOCID` int(11) NOT NULL,
  `URI` text character set utf8,
  PRIMARY KEY  (`DOCID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_search_spider`
--

LOCK TABLES `synd_search_spider` WRITE;
/*!40000 ALTER TABLE `synd_search_spider` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_search_spider` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_term`
--

DROP TABLE IF EXISTS `synd_search_term`;
CREATE TABLE `synd_search_term` (
  `TERMID` int(11) NOT NULL,
  `TERM` varchar(32) character set utf8 NOT NULL default '',
  `ORIGINAL` varchar(32) character set utf8 NOT NULL default '',
  `FUZZY` varchar(8) NOT NULL default '',
  `N` int(11) unsigned NOT NULL default '1',
  PRIMARY KEY  (`TERMID`),
  KEY `TERM` (`TERM`(8)),
  KEY `FUZZY` (`FUZZY`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_search_term`
--

LOCK TABLES `synd_search_term` WRITE;
/*!40000 ALTER TABLE `synd_search_term` DISABLE KEYS */;
INSERT INTO `synd_search_term` (`TERMID`, `TERM`, `ORIGINAL`, `FUZZY`, `N`) VALUES (0,'','','',0);
/*!40000 ALTER TABLE `synd_search_term` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_termindex`
--

DROP TABLE IF EXISTS `synd_search_termindex`;
CREATE TABLE `synd_search_termindex` (
  `TERMID` int(11) NOT NULL,
  `DOCID` int(11) NOT NULL,
  `CONTEXT` smallint(6) unsigned NOT NULL default '0',
  `FIELD` tinyint(4) unsigned NOT NULL default '0',
  `WDF` tinyint(4) unsigned NOT NULL default '1',
  `WDW` tinyint(4) unsigned NOT NULL default '255',
  PRIMARY KEY  (`TERMID`,`DOCID`),
  KEY `DOCID` (`DOCID`),
  CONSTRAINT `synd_search_termindex_ibfk_1` FOREIGN KEY (`TERMID`) REFERENCES `synd_search_term` (`TERMID`) ON DELETE CASCADE,
  CONSTRAINT `synd_search_termindex_ibfk_2` FOREIGN KEY (`DOCID`) REFERENCES `synd_search_document` (`DOCID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_search_termindex`
--

LOCK TABLES `synd_search_termindex` WRITE;
/*!40000 ALTER TABLE `synd_search_termindex` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_search_termindex` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_textindex`
--

DROP TABLE IF EXISTS `synd_search_textindex`;
CREATE TABLE `synd_search_textindex` (
  `DOCID` int(11) NOT NULL,
  `TEXT` text character set utf8,
  PRIMARY KEY  (`DOCID`),
  FULLTEXT KEY `TEXT` (`TEXT`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_search_textindex`
--

LOCK TABLES `synd_search_textindex` WRITE;
/*!40000 ALTER TABLE `synd_search_textindex` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_search_textindex` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_search_webpage`
--

DROP TABLE IF EXISTS `synd_search_webpage`;
CREATE TABLE `synd_search_webpage` (
  `DOCID` int(11) NOT NULL,
  `HASH` int(11) NOT NULL,
  `MODIFIED` int(11) NOT NULL,
  `REVISIT` int(11) NOT NULL,
  `TTL` int(11) NOT NULL,
  `DOCLEN` int(11) NOT NULL,
  `FLAGS` tinyint(4) NOT NULL,
  `URI` text character set utf8 NOT NULL,
  `TITLE` varchar(255) character set utf8 NOT NULL default '',
  `CONTENT` mediumtext character set utf8 NOT NULL,
  PRIMARY KEY  (`DOCID`),
  KEY `HASH` (`HASH`),
  KEY `REVISIT` (`REVISIT`),
  CONSTRAINT `synd_search_webpage_ibfk_1` FOREIGN KEY (`DOCID`) REFERENCES `synd_search_document` (`DOCID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_search_webpage`
--

LOCK TABLES `synd_search_webpage` WRITE;
/*!40000 ALTER TABLE `synd_search_webpage` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_search_webpage` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_session`
--

DROP TABLE IF EXISTS `synd_session`;
CREATE TABLE `synd_session` (
  `SID` varchar(32) NOT NULL default '',
  `REFRESHED` int(11) unsigned NOT NULL default '0',
  `CONTENT` text character set utf8 NOT NULL,
  PRIMARY KEY  (`SID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_session`
--

LOCK TABLES `synd_session` WRITE;
/*!40000 ALTER TABLE `synd_session` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_session` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_sso_instance`
--

DROP TABLE IF EXISTS `synd_sso_instance`;
CREATE TABLE `synd_sso_instance` (
  `SID` varchar(32) NOT NULL default '',
  `SERVICE` varchar(512) NOT NULL,
  PRIMARY KEY  (`SID`,`SERVICE`),
  CONSTRAINT `synd_sso_instance_ibfk_1` FOREIGN KEY (`SID`) REFERENCES `synd_sso_session` (`SID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_sso_instance`
--

LOCK TABLES `synd_sso_instance` WRITE;
/*!40000 ALTER TABLE `synd_sso_instance` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_sso_instance` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_sso_session`
--

DROP TABLE IF EXISTS `synd_sso_session`;
CREATE TABLE `synd_sso_session` (
  `SID` varchar(32) NOT NULL default '',
  `USER_NODE_ID` varchar(64) NOT NULL default '',
  `TS_LOGIN` int(11) default NULL,
  PRIMARY KEY  (`SID`),
  KEY `user` (`USER_NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_sso_session`
--

LOCK TABLES `synd_sso_session` WRITE;
/*!40000 ALTER TABLE `synd_sso_session` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_sso_session` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_storage_device`
--

DROP TABLE IF EXISTS `synd_storage_device`;
CREATE TABLE `synd_storage_device` (
  `DEVID` smallint(6) unsigned NOT NULL auto_increment,
  `NSID` smallint(6) unsigned NOT NULL,
  `URN` varchar(512) NOT NULL,
  `SPACE_TOTAL` bigint(20) unsigned NOT NULL,
  `SPACE_USED` bigint(20) unsigned NOT NULL default '0',
  `FAILED` int(11) unsigned default '0',
  PRIMARY KEY  (`DEVID`),
  UNIQUE KEY `NSID` (`NSID`,`URN`),
  CONSTRAINT `synd_storage_device_ibfk_1` FOREIGN KEY (`NSID`) REFERENCES `synd_storage_namespace` (`NSID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_storage_device`
--

LOCK TABLES `synd_storage_device` WRITE;
/*!40000 ALTER TABLE `synd_storage_device` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_storage_device` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_storage_lob`
--

DROP TABLE IF EXISTS `synd_storage_lob`;
CREATE TABLE `synd_storage_lob` (
  `LOBID` bigint(20) unsigned NOT NULL auto_increment,
  `NSID` smallint(6) unsigned NOT NULL,
  `VARIABLE` varchar(512) NOT NULL,
  `CHECKSUM` varchar(32) NOT NULL,
  `SPACE` int(11) unsigned NOT NULL,
  `CREATED` int(11) unsigned NOT NULL,
  `MODIFIED` int(11) unsigned NOT NULL,
  `REPLICAS` tinyint(4) unsigned NOT NULL default '2',
  `DELETED` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`LOBID`),
  UNIQUE KEY `NSID` (`NSID`,`VARIABLE`),
  CONSTRAINT `synd_storage_lob_ibfk_1` FOREIGN KEY (`NSID`) REFERENCES `synd_storage_namespace` (`NSID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_storage_lob`
--

LOCK TABLES `synd_storage_lob` WRITE;
/*!40000 ALTER TABLE `synd_storage_lob` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_storage_lob` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_storage_namespace`
--

DROP TABLE IF EXISTS `synd_storage_namespace`;
CREATE TABLE `synd_storage_namespace` (
  `NSID` smallint(6) unsigned NOT NULL auto_increment,
  `NAMESPACE` varchar(512) NOT NULL,
  PRIMARY KEY  (`NSID`),
  UNIQUE KEY `NAMESPACE` (`NAMESPACE`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_storage_namespace`
--

LOCK TABLES `synd_storage_namespace` WRITE;
/*!40000 ALTER TABLE `synd_storage_namespace` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_storage_namespace` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_storage_replica`
--

DROP TABLE IF EXISTS `synd_storage_replica`;
CREATE TABLE `synd_storage_replica` (
  `DEVID` smallint(6) unsigned NOT NULL,
  `LOBID` bigint(20) unsigned NOT NULL,
  `CHECKSUM` char(32) NOT NULL,
  PRIMARY KEY  (`DEVID`,`LOBID`),
  KEY `LOBID` (`LOBID`),
  CONSTRAINT `synd_storage_replica_ibfk_1` FOREIGN KEY (`DEVID`) REFERENCES `synd_storage_device` (`DEVID`) ON DELETE CASCADE,
  CONSTRAINT `synd_storage_replica_ibfk_2` FOREIGN KEY (`LOBID`) REFERENCES `synd_storage_lob` (`LOBID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1 PACK_KEYS=1;

--
-- Dumping data for table `synd_storage_replica`
--

LOCK TABLES `synd_storage_replica` WRITE;
/*!40000 ALTER TABLE `synd_storage_replica` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_storage_replica` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_unit_test`
--

DROP TABLE IF EXISTS `synd_unit_test`;
CREATE TABLE `synd_unit_test` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) default NULL,
  `SEQUENCE_ID` int(11) NOT NULL auto_increment,
  PRIMARY KEY  (`NODE_ID`),
  UNIQUE KEY `SEQUENCE_ID` (`SEQUENCE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_unit_test`
--

LOCK TABLES `synd_unit_test` WRITE;
/*!40000 ALTER TABLE `synd_unit_test` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_unit_test` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_unit_test2`
--

DROP TABLE IF EXISTS `synd_unit_test2`;
CREATE TABLE `synd_unit_test2` (
  `NODE_ID` varchar(64) NOT NULL,
  `INFO_HEAD` varchar(255) character set utf8 default 'Test',
  `INFO_DESC` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  CONSTRAINT `synd_unit_test2_ibfk_1` FOREIGN KEY (`NODE_ID`) REFERENCES `synd_unit_test` (`NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_unit_test2`
--

LOCK TABLES `synd_unit_test2` WRITE;
/*!40000 ALTER TABLE `synd_unit_test2` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_unit_test2` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_variable`
--

DROP TABLE IF EXISTS `synd_variable`;
CREATE TABLE `synd_variable` (
  `NAMESPACE` varchar(64) NOT NULL,
  `VARIABLE` varchar(32) NOT NULL,
  `EXPIRES` int(11) default NULL,
  `VALUE` text character set utf8,
  PRIMARY KEY  (`NAMESPACE`,`VARIABLE`),
  KEY `EXPIRES` (`EXPIRES`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_variable`
--

LOCK TABLES `synd_variable` WRITE;
/*!40000 ALTER TABLE `synd_variable` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_variable` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_workflow`
--

DROP TABLE IF EXISTS `synd_workflow`;
CREATE TABLE `synd_workflow` (
  `NODE_ID` varchar(64) NOT NULL,
  `PARENT_NODE_ID` varchar(64) default NULL,
  `CREATE_NODE_ID` varchar(64) default NULL,
  `UPDATE_NODE_ID` varchar(64) default NULL,
  `TS_CREATE` int(11) default NULL,
  `TS_UPDATE` int(11) default NULL,
  `FLAG_PROTOTYPE` tinyint(1) NOT NULL default '1',
  `FLAG_CONTEXT_MENU` tinyint(1) NOT NULL default '0',
  `FLAG_SIDEBAR_MENU` tinyint(1) NOT NULL default '0',
  `INFO_ACCESSKEY` varchar(1) default NULL,
  `INFO_HEAD` varchar(255) character set utf8 NOT NULL,
  `INFO_DESC` text character set utf8,
  `DATA_ACTIVITY` text character set utf8,
  PRIMARY KEY  (`NODE_ID`),
  KEY `PARENT_NODE_ID` (`PARENT_NODE_ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_workflow`
--

LOCK TABLES `synd_workflow` WRITE;
/*!40000 ALTER TABLE `synd_workflow` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_workflow` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `synd_workflow_event`
--

DROP TABLE IF EXISTS `synd_workflow_event`;
CREATE TABLE `synd_workflow_event` (
  `PARENT_NODE_ID` varchar(64) NOT NULL,
  `EVENT` varchar(64) NOT NULL,
  PRIMARY KEY  (`PARENT_NODE_ID`,`EVENT`),
  CONSTRAINT `synd_workflow_event_ibfk_1` FOREIGN KEY (`PARENT_NODE_ID`) REFERENCES `synd_workflow` (`PARENT_NODE_ID`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table `synd_workflow_event`
--

LOCK TABLES `synd_workflow_event` WRITE;
/*!40000 ALTER TABLE `synd_workflow_event` DISABLE KEYS */;
/*!40000 ALTER TABLE `synd_workflow_event` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2006-12-13 15:47:48
