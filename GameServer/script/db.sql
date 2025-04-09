
CREATE DATABASE IF NOT EXISTS `MyGame`
    DEFAULT CHARACTER SET = utf8mb4
    DEFAULT COLLATE = utf8mb4_unicode_ci;


USE `MyGame`;

CREATE TABLE `roles` (
  `roleid` bigint unsigned NOT NULL,
  `name` varchar(50) COLLATE utf8mb4_unicode_ci NOT NULL,
  `level` int NOT NULL DEFAULT '1',
  `create_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `update_time` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`roleid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE `accounts` (
  `account` varchar(50) COLLATE utf8mb4_unicode_ci NOT NULL,
  `passwd` varchar(64) COLLATE utf8mb4_unicode_ci NOT NULL,
  `roleid` bigint unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`account`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;


INSERT INTO `accounts` (`account`, `passwd`, `roleid`) VALUES ('ruby', '111', 0);