#!/bin/bash

VERSION=4
DATE=`date +"%Y.%m.%d"`
BASENAME=vizmo++-$VERSION-$DATE
TAR=$BASENAME.tar.gz
RPMBUILDDIR=/scratch/jdenny/rpmbuild
WORKINGDIR=/tmp
RELEASEDIR=VizmoRelease

echo Creating Vizmo Release
echo Version is $VERSION
echo Date is $DATE

cd $WORKINGDIR
mkdir $RELEASEDIR
cd $RELEASEDIR

echo Checking out repository
svn co svn+ssh://parasol-svn.cs.tamu.edu/research/parasol-svn/svnrepository/vizmo/trunk $BASENAME >& /dev/null

echo Removing .svn files
cd $BASENAME
rm -rf `find -name .svn`

echo Copying spec file
sed -i "s/define date 0000.00.00/define date $DATE/" Release/vizmo++.spec
sed -i "s/Version: 0/Version: $VERSION/" Release/vizmo++.spec
cp Release/vizmo++.spec $RPMBUILDDIR/SPECS

echo Creating TAR file:: $TAR
cd ..
tar -czvf $TAR $BASENAME >& /dev/null

echo Removing Repository
rm -rf $BASENAME

echo Copying TAR file
cp $TAR $RPMBUILDDIR/SOURCES

echo Removing all files from check out
cd ../
rm -rf $RELEASEDIR

echo Making spec files
cd $RPMBUILDDIR/SPECS
rpmbuild -ba vizmo++.spec

echo Done. Spec file located in $RPMBUILDDIR/RPMS

