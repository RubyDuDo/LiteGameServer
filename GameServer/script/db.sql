-- 创建数据库 MyGame (如果不存在)，并设置默认字符集和排序规则
CREATE DATABASE IF NOT EXISTS `MyGame`
    DEFAULT CHARACTER SET = utf8mb4
    DEFAULT COLLATE = utf8mb4_unicode_ci;

-- 切换到 MyGame 数据库上下文
USE `MyGame`;

-- 创建 roles 表
-- (先创建 roles 表，因为 accounts 表可能需要引用它)
CREATE TABLE IF NOT EXISTS `roles` (
    `role_id` INT NOT NULL AUTO_INCREMENT COMMENT '角色ID，主键，自增',
    `name` VARCHAR(50) NOT NULL COMMENT '角色名称',
    `level` INT NOT NULL DEFAULT 1 COMMENT '角色等级，默认为1',
    `create_time` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间，默认为当前时间戳',
    `update_time` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间，默认为当前时间戳，并在更新时自动更新',
    PRIMARY KEY (`role_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='角色信息表';

-- 创建 accounts 表
CREATE TABLE IF NOT EXISTS `accounts` (
    `account` VARCHAR(50) NOT NULL COMMENT '账号/用户名，主键',
    `passwd` VARCHAR(64) NOT NULL COMMENT '密码 (注意：存储明文密码非常不安全，应存储哈希值)',
    `role_id` INT NOT NULL DEFAULT 0 COMMENT '关联的角色ID (注意: 默认值0可能需要一个对应的role_id=0的角色，或者这里应该允许NULL或有不同的默认逻辑)',
    -- 如果需要创建时间/更新时间，可以像 roles 表一样添加
    -- `create_time` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    -- `update_time` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`account`),
    INDEX `idx_role_id` (`role_id`), -- 为外键添加索引以提高性能
    CONSTRAINT `fk_accounts_roles` -- 添加外键约束，确保 role_id 对应 roles 表中的有效记录
        FOREIGN KEY (`role_id`)
        REFERENCES `roles` (`role_id`)
        ON DELETE RESTRICT -- 或 SET NULL, CASCADE, NO ACTION，根据业务逻辑决定删除角色时如何处理账号
        ON UPDATE CASCADE -- 如果角色ID更新，账号表也级联更新
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='账号信息表';

-- (可选) 插入一些初始数据示例
-- INSERT INTO `roles` (`name`, `level`) VALUES ('默认角色', 1); -- 如果 role_id=0 需要对应记录
-- INSERT INTO `roles` (`name`, `level`) VALUES ('管理员', 99);
-- INSERT INTO `roles` (`name`, `level`) VALUES ('普通玩家', 10);

-- INSERT INTO `accounts` (`account`, `passwd`, `role_id`) VALUES ('testuser', 'hashed_password', (SELECT role_id FROM roles WHERE name = '普通玩家'));
-- INSERT INTO `accounts` (`account`, `passwd`, `role_id`) VALUES ('admin', 'admin_hashed_password', (SELECT role_id FROM roles WHERE name = '管理员'));
