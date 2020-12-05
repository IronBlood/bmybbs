--
-- tables
--

CREATE TABLE `t_sections` (
	`id` char(1) NOT NULL COMMENT '这里直接使用一个字符保存分区的编号，同时作为主键，例如 A',
	`name` char(16) NOT NULL,
	PRIMARY KEY (`id`)
);

CREATE TABLE `t_users` (
	`usernum` int NOT NULL COMMENT '对应于在 PASSWDS 文件中的索引，从 1 开始计数',
	`userid` varchar(14) NOT NULL,
	PRIMARY KEY (`usernum`)
);

CREATE TABLE `t_boards` (
	`boardnum` int NOT NULL COMMENT '对应于 BOARDS 文件中的索引，从 1 开始计数',
	`boardname_en` varchar(40) NOT NULL COMMENT '对应于 ythtbbs::boardheader.filename char(24)',
	`boardname_zh` varchar(40) NOT NULL COMMENT '对应于 ythtbbs::boardheader.title char(24)使用 GBK 编码，转换为 UTF8 预计最长 36 字符',
	`secstr` char(1) NOT NULL,
	PRIMARY KEY (`boardnum`),
	KEY `fk_board_section_idx` (`secstr`),
	CONSTRAINT `fk_board_section` FOREIGN KEY (`secstr`) REFERENCES `t_sections` (`id`)
);

CREATE TABLE `t_threads` (
	`id` int NOT NULL AUTO_INCREMENT COMMENT '用于数据库内部管理，和 BMY 数据不产生映射',
	`boardnum` int NOT NULL COMMENT '外键，用于视图中显示版面名称（版面中英文名可能发生变化）',
	`timestamp` bigint NOT NULL COMMENT '使用长整形保存 time_t 主题创建时间',
	`title` varchar(120) NULL COMMENT '对应 ythtbbs::fileheader.title char(60)，留足 UTF8 编码空间',
	`author` varchar(16) NULL COMMENT '作者，这里就不使用外键关联了，原作者 ID 可能不存在',
	`comments` int NULL DEFAULT 1 COMMENT '包含原文在内的讨论计数',
	PRIMARY KEY (`id`),
	KEY `fk_thread_board_idx` (`boardnum`),
	KEY `idx_timestamp` (`timestamp`),
	CONSTRAINT `fk_thread_board` FOREIGN KEY (`boardnum`) REFERENCES `t_boards` (`boardnum`)
);

CREATE TABLE `t_feed_meta` (
	`id` int NOT NULL AUTO_INCREMENT,
	`usernum` int NOT NULL,
	`lastread` bigint NOT NULL DEFAULT 0,
	`latest_tid` bigint NOT NULL DEFAULT 0,
	PRIMARY KEY (`id`),
	KEY `fk_feedmeta_user_idx` (`usernum`),
	CONSTRAINT `fk_feedmeta_user` FOREIGN KEY (`usernum`) REFERENCES `t_users` (`usernum`) ON DELETE CASCADE
);

CREATE TABLE `t_user_subscriptions` (
	`id` int NOT NULL AUTO_INCREMENT,
	`usernum` int NOT NULL,
	`boardnum` int NOT NULL,
	`lastread` bigint DEFAULT '0',
	`latest_tid` bigint DEFAULT '0',
	PRIMARY KEY (`id`),
	KEY `fk_sub_user_idx` (`usernum`),
	KEY `fk_sub_board_idx` (`boardnum`),
	CONSTRAINT `fk_sub_board` FOREIGN KEY (`boardnum`) REFERENCES `t_boards` (`boardnum`),
	CONSTRAINT `fk_sub_user` FOREIGN KEY (`usernum`) REFERENCES `t_users` (`usernum`) ON DELETE CASCADE
);

--
-- procedures
--

DELIMITER $$

-- 依据 secstr 创建分区视图
CREATE PROCEDURE procedure_create_section_view(
	IN secstr char(1)
)
BEGIN
	SET @sql = CONCAT("CREATE VIEW v_section_", secstr, " AS SELECT `boardname_en`, `boardname_zh`, `timestamp`, `title`, `author`, `comments` FROM `t_boards`, `t_threads` where `t_boards`.`boardnum` = `t_threads`.`boardnum` and `t_boards`.`secstr` = \"", secstr, "\" order by `timestamp` desc");
	PREPARE stmt FROM @sql;
	EXECUTE stmt;
	DEALLOCATE PREPARE stmt;
END$$

-- 依据 boardnum, boardname_en 创建视图，使用 call 调用
CREATE PROCEDURE procedure_create_board_view(
	IN boardnum int,
	IN boardname_en varchar(40)
)
BEGIN
	SET @sql = CONCAT("CREATE VIEW v_board_", boardname_en, " AS SELECT `boardname_en`, `boardname_zh`, `timestamp`, `title`, `author`, `comments` FROM `t_boards`, `t_threads` where `t_boards`.`boardnum` = `t_threads`.`boardnum` and `t_threads`.`boardnum` = ", boardnum, " order by `timestamp` desc");
	PREPARE stmt FROM @sql;
	EXECUTE stmt;
	DEALLOCATE PREPARE stmt;
END$$

-- 依据 userid 创建用户订阅视图
CREATE PROCEDURE procedure_create_feed_view(
	IN usernum int,
	IN userid varchar(14)
)
BEGIN
	SET @sql = CONCAT("CREATE VIEW v_feed_", userid, " AS SELECT `boardname_en`, `boardname_zh`, `timestamp`, `title`, `author`, `comments` FROM `t_boards`, `t_threads` where `t_threads`.`boardnum` IN (SELECT boardnum FROM `t_user_subscriptions` WHERE usernum = ", usernum, ") and `t_threads`.`boardnum` = `t_boards`.`boardnum` order by `timestamp` desc");
	PREPARE stmt FROM @sql;
	EXECUTE stmt;
	DEALLOCATE PREPARE stmt;
END$$

-- 插入分区
CREATE PROCEDURE procedure_insert_section(
	IN secstr char(1),
	IN name char(16)
)
BEGIN
	INSERT INTO `t_sections` (`id`, `name`) VALUE (secstr, name);
	CALL procedure_create_section_view(secstr);
END$$

-- 插入版面
CREATE PROCEDURE procedure_insert_board(
	IN boardnum int,
	IN boardname_en varchar(40),
	IN boardname_zh varchar(40),
	IN secstr char(1)
)
BEGIN
	INSERT INTO `t_boards`
		(`boardnum`, `boardname_en`, `boardname_zh`, `secstr`)
	VALUE
		(boardnum, boardname_en, boardname_zh, secstr);
	CALL procedure_create_board_view(boardnum, boardname_en);
END$$

-- 插入用户
CREATE PROCEDURE procedure_insert_user(
	IN usernum int,
	IN userid varchar(14)
)
BEGIN
	INSERT INTO `t_users`(`usernum`, `userid`) VALUE(usernum, userid);
	INSERT INTO `t_feed_meta`(`usernum`) VALUE(usernum);
	CALL procedure_create_feed_view(usernum, userid);
END$$

-- 删除用户，通过外键层级删除订阅关系和订阅元数据
CREATE PROCEDURE procedure_delete_user(
	IN usernum int,
	IN userid varchar(14)
)
BEGIN
	SET @sql = CONCAT("DROP VIEW v_feed_", userid);
	PREPARE stmt FROM @sql;
	EXECUTE stmt;
	DEALLOCATE PREPARE stmt;
	DELETE FROM `t_users` where `t_users`.`usernum` = usernum;
END$$

-- 更新版面
-- TODO 使用条件判断语句？不过重命名版面、移动分区的这个事情不常见
CREATE PROCEDURE procedure_update_board(
	IN boardnum int,
	IN new_boardname_en varchar(40),
	IN new_boardname_zh varchar(40),
	IN new_secstr char(1)
)
BEGIN
	SELECT `boardname_en` INTO @old_boardname_en FROM `t_boards` WHERE `t_boards`.`boardnum` = boardnum LIMIT 1;
	SET @sql = CONCAT("DROP VIEW v_board_", @old_boardname_en);
	PREPARE stmt FROM @sql;
	EXECUTE stmt;
	DEALLOCATE PREPARE stmt;

	CALL procedure_create_board_view(boardnum, new_boardname_en);

	UPDATE `t_boards` SET `boardname_en` = new_boardname_en, `boardname_zh` = new_boardname_zh, `secstr` = new_secstr WHERE `t_boards`.`boardnum` = boardnum;
END$$

DELIMITER ;

--
-- triggers
--

DELIMITER $$

CREATE TRIGGER trigger_update_latest_timestamp
AFTER INSERT ON `t_threads`
FOR EACH ROW
BEGIN
	SET SQL_SAFE_UPDATES=0;
	UPDATE `t_feed_meta`
		SET latest_tid = unix_timestamp()
		WHERE usernum in (
			select usernum from `t_user_subscriptions`
			where boardnum = NEW.boardnum
		);

	UPDATE `t_user_subscriptions`
		SET latest_tid = unix_timestamp()
		WHERE boardnum = NEW.boardnum;
	SET SQL_SAFE_UPDATES=1;
END$$

DELIMITER ;

--
-- data
--

CALL procedure_insert_section("0", "本站系统");
CALL procedure_insert_section("1", "交通大学");
CALL procedure_insert_section("2", "开发技术");
CALL procedure_insert_section("3", "电脑应用");
CALL procedure_insert_section("4", "学术科学");
CALL procedure_insert_section("5", "社会科学");
CALL procedure_insert_section("6", "文学艺术");
CALL procedure_insert_section("7", "知性感性");
CALL procedure_insert_section("8", "体育运动");
CALL procedure_insert_section("9", "休闲音乐");
CALL procedure_insert_section("G", "游戏天地");
CALL procedure_insert_section("N", "新闻信息");
CALL procedure_insert_section("H", "乡音乡情");
CALL procedure_insert_section("A", "校务信息");
CALL procedure_insert_section("C", "俱乐部区");
