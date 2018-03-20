#!/bin/bash

http='gccxml-git pygccxml py++'; for h in $http ; do wget "https://aur.archlinux.org/cgit/aur.git/snapshot/$h.tar.gz";\
 tar -xzf `ls *.gz` \
 mv  `ls -F |grep / |grep -v dumux` x \
 rm *.gz \
 chown -R x x;\
 runuser -l x makepkg;\
 pacman -U --noconfirm `ls /home/x/*xz`;\
