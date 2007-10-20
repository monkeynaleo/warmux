#!/bin/bash
DEBUG=${DEBUG:-"false"}
if $DEBUG; then set -x; fi
# A little script to install every needed package to compile wormux.
# Author : Yannig PERRE (drayan)
# Creation : 2007/10/20
#

if ! id | grep root > /dev/null; then
  echo
  echo "ERROR : You must be logged as root."
  echo
  echo "If you are running a Ubuntu distribution, you should try to launch"
  echo "this script using 'sudo' this way :"
  echo
  echo "  sudo ./pkg_install.sh"
  echo
  exit
fi

echo
echo "The aim of this shell is to install a bunch of package in order to"
echo "compile the dev version of Wormux."
echo
echo "List of needed packages :"
echo

cat package.lst

echo
echo -n "Do you want to install this list of package ?[y/n]"
read a

case $a in
  y|Y) : ;;
  *) echo "Exiting ..." ; exit ;;
esac

echo "Installing packages ..."
for pkg in $(cat package.lst)
do
  apt-get install $pkg
done
