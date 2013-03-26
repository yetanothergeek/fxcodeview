#!/bin/sh -e

if [ "$1" = "-c" ]
then
   rm -rf autom4te.cache */.deps config.guess config.sub
   for DIR in ./ ./src ./jef
   do
     for FILE in \
       'aclocal.m4' \
       'config.log' \
       'config.status' \
       'Makefile.in' \
       'Makefile' \
       '*.o' \
       '*.a' \
       'configure' \
       'depcomp'  \
       'install-sh' \
       'missing' \
       'fxcv'
     do
       rm -f $DIR/$FILE
     done
   done
else
  for CMD in \
    'aclocal --force' \
    'autoconf' \
    'automake --gnu --add-missing --copy' 
  do
    echo "Running ${CMD}"
    $CMD
  done
fi
