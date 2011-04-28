#/bin/bash

# Simple script to insert the OpenQwaq-backupDB script into the OpenQwaq
# service user's crontab file

user=`id -un`
#
echo "Installing the OpenQwaq backup script into user ${user}'s crontab."

crontab -l >> /tmp/cronFoo
echo "# Backup DB at 02.00 everyday" >> /tmp/cronFoo
echo "0 2 * * *	/home/openqwaq/server/etc/OpenQwaq-backupDB.sh" >> /tmp/cronFoo
crontab /tmp/cronFoo
rm -rf /tmp/cronFoo

echo ""
echo "Completed.  Showing crontab file contents:"
crontab -l
