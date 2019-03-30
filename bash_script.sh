#!/bin/bash

# Si dichiara che il contenuto di questo file è in ogni sua parte opera
# originale dell'autore
#
# Lorenzo Beretta, 536242, loribere@gmail.com

if [[ -z $1 ]] || [[ $1 == "--help" ]]; then
	echo "Please use:"
	echo $0 file t
	echo "file: is the configuration file"
	echo "t: is the minimum age of files to delete"
	exit 1
fi

DIR=$(grep 'DirName' $1)
DIR=${DIR##*'= '}
ARCHIVE_DIR="/tmp/archive_tmp"

if [[ $2 != 0 ]]; then
	mkdir $ARCHIVE_DIR
	find $DIR -mindepth 1 -mmin +$2 -execdir mv {} $ARCHIVE_DIR \;
	tar -C $ARCHIVE_DIR/ -czf oldfiles.tar.gz .
	rm -r $ARCHIVE_DIR
else
	find $DIR -mindepth 1 -print 
fi
