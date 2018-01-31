/*
Navicat MySQL Data Transfer

Source Server         : mysql
Source Server Version : 50711
Source Host           : localhost:3306
Source Database       : test

Target Server Type    : MYSQL
Target Server Version : 50711
File Encoding         : 65001

Date: 2018-01-29 12:41:09
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for account
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account` (
  `uid` varchar(255) NOT NULL,
  `createtime` varchar(255) NOT NULL,
  `logintime` varchar(255) NOT NULL,
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for playerdate
-- ----------------------------
DROP TABLE IF EXISTS `playerdate`;
CREATE TABLE `playerdate` (
  `uid` varchar(255) NOT NULL,
  `name` varchar(64) NOT NULL,
  `uuid` bigint(20) NOT NULL,
  `sex` tinyint(4) NOT NULL,
  `job` tinyint(4) NOT NULL,
  `level` int(11) NOT NULL,
  `createtime` varchar(255) NOT NULL,
  `logintime` varchar(255) NOT NULL,
  `mapid` int(11) NOT NULL,
  `x` float(32,0) NOT NULL,
  `y` float(32,0) NOT NULL,
  `z` float(32,0) NOT NULL,
  `data` mediumtext NOT NULL,
  PRIMARY KEY (`uuid`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
