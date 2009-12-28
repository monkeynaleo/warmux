#!/bin/sh

# check there is no case-sensitive file
check_case_files() {
    casefiles=`find ./ -name '*[A-Z]*' | grep -v '.svn' | grep -v 'README' | grep -v 'Makefile'`
    nbcasefiles=`/bin/echo -e "$casefiles" | wc -l`

    if [ $nbcasefiles -ne 0 ]; then
	/bin/echo -e "***********************************************************"
	/bin/echo -e "ERROR: there are $nbcasefiles case sensitive files. Please "
	/bin/echo -e "       rename them and check config.xml files"
	/bin/echo -e "$casefiles"
	/bin/echo -e "***********************************************************"
	return 1
    fi

    return 0
}

# check there is no space in filenames!
check_space_files() {
    spacefiles=`find ./ -name '* *' | grep -v '.svn' | grep -v 'README' | grep -v 'Makefile'`
    nbspacefiles=`/bin/echo -e "$spacefiles" | wc -l`

    if [ $nbspacefiles -ne 0 ]; then
	/bin/echo -e "***********************************************************"
	/bin/echo -e "ERROR: there are $nbspacefiles files with space in their names."
	/bin/echo -e "Please rename them and check config.xml files"
	/bin/echo -e "$spacefiles"
	/bin/echo -e "***********************************************************"
	return 1
    fi

    return 0
}

check_files_exist() {
    /bin/echo -e "\n* checking map $1"
    grep ' file=' $1/config.xml | sed 's/.*file=\"\(.*\)".*/\1/'
}

check_one_map_config() {
    check_files_exist $1
}

check_maps_config() {
    for d in *; do
	if [ -d '$d' ]; then
	    check_one_map_config '$d';
	fi
    done
}

check_space_files || exit 1

check_case_files || exit 1

# check_maps_config || exit 1

DATE=`date +%Y%m%d`
tar -czf Wormux-BonusMaps-${DATE}.tar.gz * --exclude=.svn --exclude=src --exclude=compress.sh --exclude=*~
