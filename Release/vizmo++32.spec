#
# Vizmo.spec
#
# This is designed to build VIZMO from the current
# Parasol SVN.  To create the source tarball, do:
#
# svn co svn+ssh://parasol-svn.cs.tamu.edu/research/parasol-svn/svnrepository/vizmo
# rm -rf `find -name .svn`
# mv vizmo/trunk vizmo++-0-`date +%Y.%m.%d`
# tar cvzf vizmo++-0-`date +%Y.%m.%d`.tar.gz vizmo++-0-`date +%Y.%m.%d`
#
%define date 2012.05.24
Name: vizmo++
Summary: vizmo++ - A visualization/authoring tool for motion planning 
Version: 3 
Release: %{date}%{dist}
License: Copyright 2012, Parasol Lab, Texas A&M University.  All Rights Reserved.
Group: Application/Engineering
Source: %{name}-%{version}-%{date}.tar.gz
URL: http://parasol.tamu.edu/groups/amatogroup/research/vizmo++/
Packager: Jory Denny <jdenny@cse.tamu.edu>, Parasol Laboratory, Texas A&M University -- http://parasol.tamu.edu/
BuildRoot: %{_tmppath}/%{name}-%{version}-%{date}-buildroot
Requires: qt4
BuildRequires: qt4-devel

%description
VIZMO is a 3D visualization/authoring tool for files 
provided/generated by the OBPRM motion planning library. 
This new version of VIZMO, VIZMO++, was developed for 
visualizing and editing motion planning environments, 
problem instances, and their solutions. VIZMO++ offers 
a nice and easy to use graphical user interface (GUI) 
that allows you to display workspace environments, 
roadmap, path, and start/goal positions. It enables 
users to interact with and edit the environment. 

%prep
%setup -n %{name}-%{version}-%{date}

%build
make platform=LINUX_gcc

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
rm -f %{buildroot}/usr/bin/vizmo++
install %{name} %{buildroot}/usr/bin
rm -rf %{buildroot}/usr/lib/vizmo++
mkdir -p %{buildroot}/usr/lib/vizmo++
cd lib
for file in `ls *.a` ; do
  install $file %{buildroot}/usr/lib/vizmo++/$file.%{version}.%{release}
done
cd ..

%clean
rm -rf %{buildroot}

%post
echo "/usr/lib/vizmo++" > /etc/ld.so.conf.d/vizmo++-i386.conf
/sbin/ldconfig

%postun
rm -rf /usr/lib/vizmo++
rm -f /etc/ld.so.conf.d/vizmo++-i386.conf
/sbin/ldconfig

%files
/usr/bin/%{name}
/usr/lib/%{name}

%changelog
* Thu May 24 2012 Jory Denny <jdenny@cse.tamu.edu> 3-24may2012 
- Version 3 - new environment definition

# EOF - vizmo++.spec

