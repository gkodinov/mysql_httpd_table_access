CREATE DATABASE IF NOT EXISTS httpd;
CREATE TABLE IF NOT EXISTS httpd.replies(
  id INT NOT NULL PRIMARY KEY,
  data VARCHAR(2048) CHARACTER SET utf8mb4);
REPLACE INTO httpd.replies (id, data)
  VALUES (1, '<p>data1</p>'), (2, null);
