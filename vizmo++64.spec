#
# RAPID.spec
#
# This is designed to build RAPID from the current
# Parasol SVN.  To create the source tarball, do:
#
# svn co svn+ssh://parasol-svn.cs.tamu.edu/research/parasol-svn/svnrepository/vizmo
# rm -rf `find vizmo -name .svn`
# mv vizmo/trunk vizmo++-0-`date +%Y.%m.%d`
# tar cvzf vizmo++-0-`date +%Y.%m.%d`.tar.gz vizmo++-0-`date +%Y.%m.%d`
#
%define date 2011.01.14
Name: vizmo++
Summary: vizmo++ - A visualization/authoring tool for motion planning 
Version: 0 
Release: %{date}%{dist}
License: Copyright 2011, Parasol Lab, Texas A&M University.  All Rights Reserved.
Group: Application/Engineering
Source: %{name}-%{version}-%{date}.tar.gz
# CR_icra06.pdf from http://parasol.tamu.edu/publications/download.php?file_id=519
URL: http://parasol.tamu.edu/groups/amatogroup/research/vizmo++/
Packager: Jory Denny <jdenny@neo.tamu.edu>, Parasol Laboratory, Texas A&M University -- http://parasol.tamu.edu/
BuildRoot: %{_tmppath}/%{name}-%{version}-%{date}-buildroot
Patch1: vizmo++-rpm64.diff
Requires: RAPID qt4
BuildRequires: RAPID-devel qt4-devel

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
%patch1

%build
cp libRAPID.so lib/
make -f Makefile.linux

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
install %{name} %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/lib64/vizmo++
cd lib
for file in `ls *.so` ; do
  install $file %{buildroot}/usr/lib64/vizmo++/$file.%{version}.%{release}
  #ln -s %{buildroot}/usr/lib64/vizmo++/$file.%{version}.%{release} $file.%{version}
done
cd ..

%clean
rm -rf %{buildroot}

%post
echo "/usr/lib64/vizmo++" > /etc/ld.so.conf.d/vizmo++-x86_64.conf
/sbin/ldconfig

%postun
rm -rf /usr/lib64/vizmo++
rm /etc/ld.so.conf.d/vizmo++-x86_64.conf
/sbin/ldconfig

%files
/usr/bin/%{name}
/usr/lib64/%{name}

%changelog
* Fri Jan 14 2011 Jory Denny <jdenny@neo.tamu.edu> 0-14jan2011 
- Initial version

# EOF - vizmo++.spec

