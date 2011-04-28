#!/bin/bash

case "$1" in
        "hosted") piece=hosted  ;;
        "TES") piece=TES ;;
        *) echo "usage: $0 [hosted|TES]"; exit;;
esac

for f in invite notify reminder-admin resetPassword reminder-user welcome-admin welcome-user
  do
    echo "Fixing link to $f.txt"
    rm -rf "$f.txt"
    ln -s "$PWD/$1-$f.txt" "$f.txt"
    if [ $? -ne 0 ]; then
      echo "Need to create link for $1-$f.txt";
    fi
done
