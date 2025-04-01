-- 确保我们在正确的数据库上下文中
USE `MyGame`;

-- 插入数据到 roles 表
-- 注意：我们只插入 name 和 level，让 role_id (AUTO_INCREMENT) 和时间戳 (DEFAULT) 自动生成。
-- 这假设执行时第一个插入的 role_id 是 1，第二个是 2。
INSERT INTO `roles` (`name`, `level`) VALUES
('ruby11', 1), -- 期望这个插入后 role_id 为 1
('ruby', 1);    -- 期望这个插入后 role_id 为 2

-- 插入数据到 accounts 表
-- 警告：下面的语句插入了明文密码 '111'，这非常不安全！实际应用中应存储密码哈希。
-- 它假设上面插入 'ruby' 角色后得到的 role_id 是 2。
INSERT INTO `accounts` (`account`, `passwd`, `role_id`) VALUES
('ruby', '111', 2); -- 这里的 '2' 必须对应 roles 表中实际存在的 role_id

-- 检查插入结果 (可选)
SELECT * FROM `roles`;
SELECT * FROM `accounts`;
