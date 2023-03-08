# !/bin/sh

# Config the runas program
chmod 111 /mnt/runas

# Create the database file
touch /etc/runas

# Create entries for the database files
echo "federer:djokovic:grass&hard" >> /etc/runas
echo "djokovic:federer:hard&grass" >> /etc/runas
echo "nadal:federer:clay&grass" >> /etc/runas
echo "federer:nadal:grass&clay" >> /etc/runas
echo "djokovic:nadal:hard&clay" >> /etc/runas
echo "nadal:djokovic:clay&hard" >> /etc/runas

echo "federer:*:10:101:federer:/:/bin/ash" >> /etc/passwd
echo "djokovic:*:20:101:djokovic:/:/bin/ash" >> /etc/passwd
echo "nadal:*:30:101:nadal:/:/bin/ash" >> /etc/passwd

# Config the database file
chmod 444 /etc/runas
