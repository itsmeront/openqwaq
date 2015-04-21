# Introduction #

For security reasons, it's best to login to the OpenQwaq Admin pages using the secure HTTPS protocol. I took a stab at this and have been able to get it to work a couple times by implementing the directions verbatim (copy-and-paste recommended!).
<p>
Notes:<br>
-- this is an alternative to a Verisign-type certificate, saving you $$$<br>
-- your firewall needs to open up port 8443 for this to work<br>
-- the browser will bark at you for the self-signed key but since it's just your own admin pages your customers won't see it so shouldn't be a problem<br>
-- copy-and-paste the red bold lines (i.e., as well as the blue Wiki autolinking text - anyone know how to turn this off?)<br>
-- cobbled together from various sources, including Ron T. and Patrick (thanks to both)<br>

Feel free to try it out and let me know if there's any issues or typos.<br>
<br>
Good luck!<br>
<br>
Mark Loparco<br>
09.16.2011<br>
<br>
<br>
<h1>Details</h1>


# 0. gain root access to your server<br>
# ===========================================<br>
# depends on how your server and ssh login works:<br>
# can either be password-less root login (not recommended)<br>
# or login as openqwaq and "sudo su" to root (saves sudo-ing the commands)<br>

# 1. install prerequisite components<br>
# ===========================================<br>
# these may already be installed from step 1 of the OQ installation instructions, but just in case:<br>
<b><font color='red'>yum install mod_ssl</font></b><br>
<b><font color='red'>yum install openssl</font></b><br>

# 2. enable https connections<br>
# ===========================================<br>
# make a symbolic link to the OpenQwaq-https.conf file<br>
<b><font color='red'>ln -s /home/openqwaq/server/etc/OpenQwaq-https.conf /etc/httpd/conf.d/OpenQwaq-https.conf</font></b><br>
# edit the OpenQwaq-https.conf file<br>
<b><font color='red'>vi /home/openqwaq/server/etc/OpenQwaq-https.conf</font></b><br>
# change the ServerName from openqwaq.openqwaq.org to YOUR_SERVER_NAME or YOUR_SERVER_IP<br>
# change "loadlhost" typo in file (not sure if needed but why not?):<br>
# change "Allow from loadlhost.localdomain" to "Allow from localhost.localdomain"<br>
# make sure that the # SSL cert/key line after VirtualHost <b>:8443 reads:</b><br>
# SSLCertificateFile /home/openqwaq/server/etc/keys/openqwaq.com.crt<br>
# SSLCertificateKeyFile /home/openqwaq/server/etc/keys/openqwaq.com.key.insecure<br>
# note: these file names must match those created in Step #4 below<br>

# 3. generate the necessary keys and certificates<br>
# ===========================================<br>
# navigate to keys directory (not necessary to be in this directory but I like to work here)<br>
<b><font color='red'>cd /home/openqwaq/server/etc/keys</font></b><br>
# generate private key<br>
<b><font color='red'>openssl genrsa -out ca.key 1024</font></b><br>
# note: when I used 4096 it didn't work, perhaps for 32-bit reasons (?)<br>

# generate Certificate Signing Request (CSR)<br>
<b><font color='red'>openssl req -new -key ca.key -out ca.csr</font></b><br>

# generate self-signed key<br>
<b><font color='red'>openssl x509 -req -days 365 -in ca.csr -signkey ca.key -out ca.crt</font></b><br>

# 4. copy the files to correct location<br>
# ===========================================<br>
<b><font color='red'>cp ca.crt /home/openqwaq/server/etc/keys/openqwaq.com.crt</font></b><br>
<b><font color='red'>cp ca.key /home/openqwaq/server/etc/keys/openqwaq.com.key.insecure</font></b><br>

# 5. set admin username and password to what you want<br>
# ===========================================<br>
# option a: create a new user and assign password<br>
# replace someNewUserName with the login name of your choice<br>
<b><font color='red'>htdigest /home/openqwaq/server/etc/OpenQwaq-digests 'OpenQwaq Server Admin Pages' someNewUserName</font></b><br>
# you will be prompted to enter and confirm the password<br>
# option b: change the password on an existing user<br>
<b><font color='red'>htpasswd /home/openqwaq/server/etc/OpenQwaq-digests someExistingUserName</font></b><br>
# you will be prompted to enter and confirm the password<br>

# 6. restart apache<br>
# ===========================================<br>
<b><font color='red'>/sbin/service httpd restart</font></b><br>

# 7. test it out by going to the admin web page<br>
# ===========================================<br>
<b><font color='red'><a href='https://YOUR_IP:8443/admin/'>https://YOUR_IP:8443/admin/</a></font></b><br>
# enter username and password and hopefully you're in!<br>