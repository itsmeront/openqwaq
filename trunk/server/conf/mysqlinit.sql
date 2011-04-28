create user 'openqwaq'@'%';
create database if not exists OpenQwaqData;
create database if not exists OpenQwaqActivityLog;
grant all on OpenQwaqData.* to 'openqwaq'@'localhost' identified by 'openqwaq';
grant all on OpenQwaqActivityLog.* to 'openqwaq'@'localhost' identified by 'openqwaq';
flush privileges;
flush tables;
