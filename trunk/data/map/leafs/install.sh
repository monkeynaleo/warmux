mkdir /usr/share/games/wormux/map/leafs/
cp birds.png front.png back.png preview.jpg config.xml /usr/share/games/wormux/map/leafs/
chown -R root:games /usr/share/games/wormux/map/leafs
chmod -R o-rwx /usr/share/games/wormux/map/leafs
chmod -x /usr/share/games/wormux/map/leafs/*
