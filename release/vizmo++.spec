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
%define date 0000.00.00
Name: vizmo++
Summary: vizmo++ - A visualization/authoring tool for motion planning
Version: 0
Release: %{date}%{dist}
License: Copyright 2012, Parasol Lab, Texas A&M University.  All Rights Reserved.
Group: Application/Engineering
Source: %{name}-%{version}-%{date}.tar.gz
URL: http://parasol.tamu.edu/groups/amatogroup/research/vizmo++/
Packager: Jory Denny <jdenny@cse.tamu.edu>, Parasol Laboratory, Texas A&M University -- http://parasol.tamu.edu/
BuildRoot: %{_tmppath}/%{name}-%{version}-%{date}-buildroot/
Requires: qt4
BuildRequires: qt4-devel
%ifarch x86_64
%define PLATFORM LINUX_gcc
%else
%define PLATFORM LINUX_gcc
%endif

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
cd src
make reallyreallyclean
make platform=%{PLATFORM} -j4

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/bin
rm -f %{buildroot}/usr/bin/vizmo++
install src/%{name} %{buildroot}/usr/bin
cd ..

%clean
rm -rf %{buildroot}

%post

%postun

%files
/usr/bin/%{name}

%changelog
* Wed Jul 10 2015 Jory Denny <jdenny@cse.tamu.edu> 7-10jul2015
- Version 0.3.0 - Merging Env data structures with PMPL counterparts.
  Temporarily disabled functionality to add, copy, and delete obstacles as well
  as editing robot. Env format is also being updated for boundaries and
  color/texture options.
* Wed Jul 1 2015 Jory Denny <jdenny@cse.tamu.edu> 7-1jul2015
- Version 0.2.1 - Region Steering methods, few interface bug fixed, no major
  file version updates.
* Thu May 29 2014 Jory Denny <jdenny@cse.tamu.edu> 5-29may2014
- Version 0.2.0 - New user interface. All buttons moved to side of window. Text
  GUI located at the bottom. All dialogs are attached to pop up on the right of
  the GUI.
* Fri May 9 2014 Jory Denny <jdenny@cse.tamu.edu> 5-09may2014
- Version 0.1.2 - Query segmentation fault bug fix from previous release.
* Thu May 8 2014 Jory Denny <jdenny@cse.tamu.edu> 5-08may2014
- Version 0.1.1 - Query display bug fix. Now query correctly displayed in point
  and robot mode, with colors.
* Wed Apr 30 2014 Jory Denny <jdenny@cse.tamu.edu> 4-30apr2014
- Version 0.1.0 - Release for Fedora 20. Includes new camera controls, uniform
  looking buttons, consolidated UI, environment editing tools, map editing
  tools, OpenGL optimizations, and minor bug fixes.

# EOF - vizmo++.spec

