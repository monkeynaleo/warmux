#!/usr/bin/perl -w
use strict;

# Compilation script make for Wormux under MAC OS X
# http://wormux.org
# Done by Yoann Katchourine
#
#
# TODO LIST
# init
# compile
# installlibs
#

# Provide data about using this script, and give all data to compile Wormux under Mac OS X.
sub help
{
    print"***************************************\n";
    print"* Wormux - Compilation under Mac OS X *\n";
    print"***************************************\n";
    print"Usage - \n";
    print"Use as args the followings commands\n";
    print"\n";
    print"help : display an help of using this script\n";
    print"\n";
    print"installlibs : install all libs in order to compile\n";
    print"src : Compile using system libraries\n";
    print"fink : Compile using fink libraries ( /sw )\n";
    print"macports : Compile using macports libraries ( /opt )\n";
    print"\n";
    print"debug : Make a debug release using System libraries\n";
    print"debug-fink : Make a debug release using fink libraries\n";
    print"debug-macports : Make a debug release using macports libraries\n";
    print"\n";
    print"init : do only exports in order to make a manual compilation\n";
    print"\n";
    print"perl script by Yoann Katchourine, for Wormux.org\n";
}


# Make features in order to make all deps available
sub installlibs
{
    print"Available soon...\n";
    print"Be able to install libraries...\n";
}


# Compile wormux using System libraries
sub src
{
    print"Available soon...\n";
    print"Compile using system libraries\n";
}

# Compile wormux using fink libraries ( /sw )
sub fink
{
    print"Available soon...\n";
    print"Compile using fink libraries\n";
}

# Compile wormux using macports libraries ( /opt )
sub macports
{
    print"Available soon...\n";
    print"Compile using macports libraries\n";
}


sub debugsystem
{
    print"Available soon...\n";
}

sub debugfink
{
    print"Available soon...\n";
}

sub debugmacports
{
    print"Available soon...\n";
}


sub init
{
    print"Available soon...\n";
    print"loading init...\n";
}



if ($#ARGV == -1)
{
    # Menu Usage (no args)
    print"Wormux - Compilation under Mac OS X\n";
    print"\n";
    print"use ./Compilation help for usage.\n";
    print"Quick use :"; 
    print"help, installlibs, src, fink, macports\n";
    print"\n";
}
else
{
    # Dialog saying wich functions will be launched
    print "Show help...\n" if ($ARGV[0] eq "help");
    print "Start install libs...\n" if ($ARGV[0] eq "installlibs");
    print "Compile from system libraries...\n"  if($ARGV[0] eq "src");
    print "Compile from fink libraries...\n"  if($ARGV[0] eq "fink");
    print "Compile from macports libraries...\n"  if($ARGV[0] eq "macports");
    print "Make debug from System libraries...\n"  if($ARGV[0] eq "debug");
    print "Make debug from Fink libraries...\n"  if($ARGV[0] eq "debug-fink");
    print "Make debug from Macports libraries...\n"  if($ARGV[0] eq "debug-macports");
    print "Do init only...\n"  if($ARGV[0] eq "init");

    
    # When this item has been selected launch this :
    if($ARGV[0] eq "help")
    {
        help();
    }


    # When this item has been selected launch this :
    if($ARGV[0] eq "installlibs")
    {
        installlibs();
    }


    # When this item has been selected launch this :
    if($ARGV[0] eq "src")
    {
        src();
    }

    
    # When this item has been selected launch this :
    if($ARGV[0] eq "fink")
    {
        init();
        fink();
    }


    # When this item has been selected launch this :
    if($ARGV[0] eq "macports")
    {
        init();
        macports();
    }


    # When this item has been selected launch this :
    if($ARGV[0] eq "debug")
    {
        init();
        debugsystem();
    }


    # When this item has been selected launch this :
    if($ARGV[0] eq "debug-fink")
    {
        init();
        debugfink();
    }


    # When this item has been selected launch this :
    if($ARGV[0] eq "debug-macports")
    {
        init();
        debugmacports();
    }


    # When this item has been selected launch this :
    if($ARGV[0] eq "init")
    {
        init();
    }

}
