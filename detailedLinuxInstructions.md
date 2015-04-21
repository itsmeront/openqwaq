# Introduction #

Sorry for the wall of text, I didn't know where else to put this. I was having problems like everyone else, but I managed to get a server working with the following instructions. I wrote them out so that some of the "less linux than me" people could follow along too.

Some disclaimers, I'm not a linux guru or particularly knowledgeable about server maintenance or anything like that. I just know enough commands to get in trouble. With the old instructions, google, and a few phone calls to people who know more than me I managed to compile this list here.

Good luck!

Brandon Pate


# Details #

0. The server has been verified with SELinux disabled, on 32-bit Linuxen. Changes for 64-bit or SELinux is not covered here.
  * I know there is a command to disable SElinux but I didn't go look it up, I saw it on the forms here. The reason I didn't disable it, is there is a check box when you install CentOS to turn it off, so I used that instead.
  * Edit: The command is 'sudo setenforce 0'

1. Install necessary packages
On RHEL/CentOS:
  * sudo yum install subversion
  * sudo yum install gcc
  * sudo yum install php
  * sudo yum install php-xml
  * sudo yum install mysql-server
  * sudo yum install unixODBC
  * sudo yum install mysql-connector-odbc
  * sudo yum install php-odbc
  * sudo yum install vnc
  * sudo yum install vnc-server
  * sudo yum install mod\_ssl
  * sudo yum install postfix
  * sudo yum install ntp

2. Add user and group openqwaq with correct uid and gid:
  * groupadd -g 1234 openqwaq
  * useradd -g 1234 -G 1234 -u 1234 -c "OpenQwaq service user" -d /home/openqwaq -s /bin/bash openqwaq
  * chmod 750 /home/openqwaq
  * passwd openqwaq
    * enter the password with confirmation




3. Open the sudoers file via the visudo command:
  * Scroll down until you get to the list of users, it will look like this:
    * root       ALL=(ALL)       ALL
  * press the i button to enter "insert mode" then add the following line below the ROOT user
    * openqwaq       ALL=(ALL)       ALL
  * press the escape button to exit "insert mode"
  * scroll up to the line "Defaults      requiretty", press the i button to enter "insert mode" again.
  * Add the character "#" to the front of the line, to comment out this line and no longer "requiretty"
  * press the escape button to exit "insert mode"
  * press the ":" button to bring up the prompt.
  * type "wq" and press enter
    * "w" is the command for write (or save), and "q" is the command for quit. By combining them we are saving the file and quitting.

5. Switch to user "openqwaq"
  * su openqwaq
    * type password setup earlier.

6. Check out server from SVN
  * cd /home/openqwaq
  * svn co http://openqwaq.googlecode.com/svn/trunk/server

7. Set up httpd for the server:
  * sudo ln -s /home/openqwaq/server/etc/OpenQwaq-http.conf /etc/httpd/conf.d
  * replace "User apache" in /etc/httpd/conf/httpd.conf with "User openqwaq"
  * replace "Group apache" in /etc/httpd/conf/httpd.conf with "Group openqwaq"
    * sudo vi /etc/httpd/conf/httpd.conf
      * if you don't type sudo, and just type vi you wont be able to save over the "read-only" file.
    * scroll to the users section
    * press "i" to enter insert mode, and replace "apache" with "openqwaq" for both user and group
    * press the escape key to exit insert mode
    * type "wq" and press enter to save, and exit the vi editor.
  * sudo /sbin/service httpd restart
    * this restarts the web server

8. Verify that http works properly by going to:
  * http://localhost/clients
  * http://localhost/admin

9. Set up the MySQL database:

  * sudo /sbin/service mysqld start
  * sudo /usr/bin/mysqladmin -u root password 'openqwaq'
    * this is setting the mysql root password to 'openqwaq', so don't change the password part to some other password, that is part of the command. The password you are choosing is the 'openqwaq' part (if you change it, remember to change it in the directions further along)
  * cd /home/openqwaq/server/conf
  * /usr/bin/mysql -uroot -popenqwaq -b < ./mysqlinit.sql
  * sudo odbcinst -i -s -l -f ./OpenQwaqData.dsn.in
  * sudo odbcinst -i -s -l -f ./OpenQwaqActivityLog.dsn.in
  * sudo vi /etc/odbcinst.ini
    * scroll down to the line that says #[MySQL](MySQL.md)
    * enter insert mode by pressing the "i" button
    * remove the # tags on the lines between [MySQL](MySQL.md) and FileUsage     =1"
    * replace the text after the "=" on the Driver line with  " /usr/lib/libmyodbc3.so"
    * replace the text after the "=" on the Setup line with "/usr/lib/libodbcmyS.so"
    * press escape to leave insert mode
    * type ":wq" and press enter
  * isql OpenQwaqData openqwaq openqwaq -b < ./OpenQwaqData.sql
  * isql OpenQwaqActivityLog openqwaq openqwaq -b < ./OpenQwaqActivityLog.sql
    * The isql commands create the SQL tables required by openqwaq
  * isql OpenQwaqData openqwaq openqwaq -b < ./default-servers.sql
  * isql OpenQwaqData openqwaq openqwaq -b < ./default-visitor.sql
    * The previous commands are using the "isql" command to add default data to the SQL tables you set up above.

10. Verify mysql initialization by going to
  * http://localhost/admin
    * Ensure existence of default server in server tab
    * Ensure existence of default organization in org tab
    * Ensure existence of default user in user tab


11. Set up app server scripts
  * cd /home/openqwaq/server/apps/utils/
  * sudo ./MakeWrappers.sh /home/openqwaq/server/apps/scripts/


12. Prepare various server bits
  * cd /home/openqwaq/server/mail\_templates
  * ./fixlinks.sh hosted
  * cp /home/openqwaq/server/conf/server.conf.in /home/openqwaq/server/conf/server.conf
    * copying the file "server.conf.in" to the same directory with the new name "server.conf"
  * mkdir /home/openqwaq/realms
  * mkdir /home/openqwaq/users
  * mkdir /home/openqwaq/tmp
  * mkdir /home/openqwaq/OpenQwaqApps
  * cp /home/openqwaq/server/etc/forums.properties /home/openqwaq/realms
  * ln -s /home/openqwaq/server/system-resources /home/openqwaq/realms
  * sudo ln -s /home/openqwaq/server/etc/OpenQwaq /etc/init.d/
  * sudo ln -s /home/openqwaq/server/etc/OpenQwaq-iptables /etc/init.d/
  * sudo ln -s /home/openqwaq/server/etc/OpenQwaq-tunnel /etc/init.d/


13. Fix permissions and Start the server
  * cd /home/openqwaq/server/etc
  * chmod 775 OpenQwaq
  * chmod 755 OpenQwaq-iptables
  * chmod 755 OpenQwaq-tunnel
  * sudo /sbin/service OpenQwaq start
  * sudo /sbin/service OpenQwaq-iptables start


14. Start Postfix
  * sudo /sbin/service postfix start

15. Configure OpenQwaq and necessary services to start on reboot
  * sudo /sbin/chkconfig --levels 345 OpenQwaq on
  * sudo /sbin/chkconfig --levels 345 OpenQwaq-iptables on
  * sudo /sbin/chkconfig --levels 345 OpenQwaq-tunnel on
  * sudo /sbin/chkconfig --levels 345 mysqld on
  * sudo /sbin/chkconfig --levels 345 postfix on
  * sudo /sbin/chkconfig --levels 345 httpd on
  * sudo /sbin/chkconfig --levels 345 ntpd on

16. Configure the Server
  * Goto: http://localhost/admin/serverconf.php?server=localhost
  * Configure SMTP options
  * Configure support and bug email

17. Run the server tests
  * http://localhost/admin/servertest.php

18. Installing Firefox
  * sudo yum install firefox
    * Test Firefox by launching the client and choosing File>>Web Link...

19. Installing Adobe Reader
  * sudo rpm -ivh http://linuxdownload.adobe.com/adobe-release/adobe-release-i386-1.0-1.noarch.rpm
  * sudo rpm --import /etc/pki/rpm-gpg/RPM-GPG-KEY-adobe-linux
  * sudo yum install AdobeReader\_enu
    * Test Adobe Reader by dropping a PDF file on the client

20. Installing OpenOffice.org
  * sudo yum install openoffice.org-core.i386
  * sudo yum install openoffice.org-writer.i386
  * sudo yum install openoffice.org-calc.i386
  * sudo yum install openoffice.org-base.i386
  * sudo yum install openoffice.org-draw.i386
  * sudo yum install openoffice.org-impress.i386
    * Test OpenOffice by dropping a .doc or .odt file on the client

21. Installing the Adobe Flash Plugin for Firefox
  * sudo yum groupinstall "Sound and Video"
  * sudo yum install flash-plugin nspluginwrapper curl
    * Test this by going to your favorite flash site

Notes:
  * This list of instructions was edited from previous versions to help people who don't know much about CentOS or Linux in general, there is no regard to server security when following these instructions. Best advice is to do this in a virtual machine, to avoid angering IT-Security people :)
  * There are no admin or user docs, nor support pages prepared for OpenQwaq so don't bother clicking help anywhere as the links wont work
  * To connect to this server type "/sbin/ifconfig" into the terminal, and get the IP address from the print out, and connect to that.
    * Remember, all users need an account - they can be created in the http://localhost/admin page, under users.