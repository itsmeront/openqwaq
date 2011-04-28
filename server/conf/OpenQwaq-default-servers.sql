/* isql OpenQwaqData UserId UserPass -b < OpenQwaq-default-servers.sql  */
/* NOTE: isql is pathetic; everything must be single line, sorry! */
INSERT INTO servers(id, pool, role, status, internal_name, external_name, created) VALUES(1, "", "allInOne", "active", "localhost", "", NOW());
