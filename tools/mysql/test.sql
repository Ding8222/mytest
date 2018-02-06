/*
 Navicat Premium Data Transfer

 Source Server         : mytest
 Source Server Type    : MySQL
 Source Server Version : 50721
 Source Host           : localhost:3306
 Source Schema         : test

 Target Server Type    : MySQL
 Target Server Version : 50721
 File Encoding         : 65001

 Date: 06/02/2018 15:37:39
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for account
-- ----------------------------
DROP TABLE IF EXISTS `account`;
CREATE TABLE `account`  (
  `uid` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `createtime` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `logintime` varchar(255) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  PRIMARY KEY (`uid`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for playerdate
-- ----------------------------
DROP TABLE IF EXISTS `playerdate`;
CREATE TABLE `playerdate`  (
  `uid` varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `name` varchar(64) CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL,
  `guid` bigint(64) NOT NULL,
  `sex` tinyint(4) NOT NULL,
  `job` tinyint(4) NOT NULL,
  `level` int(11) NOT NULL,
  `createtime` int(11) NOT NULL,
  `logintime` int(11) NOT NULL,
  `mapid` int(11) NOT NULL,
  `x` float(32, 0) NULL DEFAULT NULL,
  `y` float(32, 0) NULL DEFAULT NULL,
  `z` float(32, 0) NULL DEFAULT NULL,
  `data` mediumtext CHARACTER SET latin1 COLLATE latin1_swedish_ci NULL,
  PRIMARY KEY (`guid`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = latin1 COLLATE = latin1_swedish_ci ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
