CREATE TABLE IF NOT EXISTS `links` (
    `id` VARCHAR(50) NOT NULL,
    `link` TEXT NOT NULL,
    `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    PRIMARY KEY (`id`)
    );