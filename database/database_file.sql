-- --------------------------------------------------------
-- Host:                         127.0.0.1
-- Wersja serwera:               10.6.5-MariaDB - mariadb.org binary distribution
-- Serwer OS:                    Win64
-- HeidiSQL Wersja:              11.3.0.6295
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;


-- Zrzut struktury bazy danych bankprojekt
CREATE DATABASE IF NOT EXISTS `bankprojekt` /*!40100 DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_polish_ci */;
USE `bankprojekt`;

-- Zrzut struktury tabela bankprojekt.account
CREATE TABLE IF NOT EXISTS `account` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `ID_Client` int(11) NOT NULL,
  `Account_Number` char(26) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT '0',
  `IBAN` char(30) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT '0',
  `Balance` double NOT NULL DEFAULT 0,
  `Opening_Balance` double NOT NULL DEFAULT 0,
  `Date_Opened` datetime NOT NULL DEFAULT current_timestamp(),
  `Interest` float NOT NULL DEFAULT 0,
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  UNIQUE KEY `Account_Number` (`Account_Number`),
  KEY `ID_Client` (`ID_Client`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_polish_ci;

-- Eksport danych został odznaczony.

-- Zrzut struktury tabela bankprojekt.client
CREATE TABLE IF NOT EXISTS `client` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `PrvtId` char(9) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT '',
  `Firstname` varchar(20) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT '',
  `Lastname` varchar(45) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT '',
  `PESEL` char(11) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT '',
  `Birth_date` date NOT NULL,
  `Telephone_number` char(15) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT '',
  `Email` varchar(30) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT '',
  `Pass_SHA384` char(96) COLLATE utf8mb3_polish_ci NOT NULL,
  `Country` char(3) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT 'PL',
  `State_province` varchar(50) COLLATE utf8mb3_polish_ci NOT NULL,
  `Town` varchar(30) COLLATE utf8mb3_polish_ci NOT NULL,
  `Street` varchar(30) COLLATE utf8mb3_polish_ci NOT NULL,
  `Nr_property` varchar(5) COLLATE utf8mb3_polish_ci NOT NULL,
  `Nr_apartament` varchar(5) COLLATE utf8mb3_polish_ci NOT NULL,
  `Zip_code` varchar(10) COLLATE utf8mb3_polish_ci NOT NULL,
  `Join_Date` datetime NOT NULL DEFAULT current_timestamp(),
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`),
  KEY `PrvtId` (`PrvtId`)
) ENGINE=InnoDB AUTO_INCREMENT=4 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_polish_ci;

-- Eksport danych został odznaczony.

-- Zrzut struktury tabela bankprojekt.transactions
CREATE TABLE IF NOT EXISTS `transactions` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Payment_File_MessageID` varchar(35) COLLATE utf8mb3_polish_ci NOT NULL,
  `Time_Date` datetime NOT NULL,
  `Number_Transactions` tinyint(3) unsigned NOT NULL DEFAULT 1,
  `Client_PrivateID` char(9) COLLATE utf8mb3_polish_ci NOT NULL,
  `Payment_BlockID` varchar(35) COLLATE utf8mb3_polish_ci NOT NULL,
  `Operation_Type_TRF` char(50) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT 'TRF',
  `Delivery_Date` date NOT NULL,
  `Orginator_Name` varchar(70) COLLATE utf8mb3_polish_ci NOT NULL,
  `Orginator_Country` char(2) COLLATE utf8mb3_polish_ci DEFAULT NULL,
  `Orginator_Address_Line1` varchar(35) COLLATE utf8mb3_polish_ci DEFAULT NULL,
  `Orginator_Address_Line2` varchar(35) COLLATE utf8mb3_polish_ci DEFAULT NULL,
  `Orginator_IBAN` varchar(34) COLLATE utf8mb3_polish_ci NOT NULL,
  `Clearing_Code` char(5) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT 'PLKNR',
  `Orginator_BankID` varchar(8) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT '73897310',
  `Payment_TransactionID` varchar(35) COLLATE utf8mb3_polish_ci NOT NULL,
  `Reference` varchar(16) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT ' ',
  `SORBNET` char(5) COLLATE utf8mb3_polish_ci DEFAULT NULL,
  `Amount` float NOT NULL,
  `Currency` char(3) COLLATE utf8mb3_polish_ci DEFAULT 'PLN',
  `Recipient_BankID` varchar(8) COLLATE utf8mb3_polish_ci NOT NULL,
  `Recipient_Name` varchar(70) COLLATE utf8mb3_polish_ci NOT NULL,
  `Recipient_Country` char(2) COLLATE utf8mb3_polish_ci DEFAULT NULL,
  `Recipient_Address_Line1` varchar(35) COLLATE utf8mb3_polish_ci DEFAULT NULL,
  `Recipient_Address_Line2` varchar(35) COLLATE utf8mb3_polish_ci DEFAULT NULL,
  `Recipient_Account_Number` char(26) COLLATE utf8mb3_polish_ci NOT NULL,
  `Operation_Type_PLKR` char(4) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT 'PLKR',
  `Transfer_Title` varchar(140) COLLATE utf8mb3_polish_ci NOT NULL DEFAULT ' ',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `Payment_InformationID` (`Payment_BlockID`) USING BTREE,
  UNIQUE KEY `Message_ID` (`Payment_File_MessageID`) USING BTREE,
  UNIQUE KEY `ID` (`ID`),
  KEY `Client_PrivateID` (`Client_PrivateID`)
) ENGINE=InnoDB AUTO_INCREMENT=12 DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_polish_ci;

-- Eksport danych został odznaczony.

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IFNULL(@OLD_FOREIGN_KEY_CHECKS, 1) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40111 SET SQL_NOTES=IFNULL(@OLD_SQL_NOTES, 1) */;
