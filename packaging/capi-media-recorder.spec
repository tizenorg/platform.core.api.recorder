Name:       capi-media-recorder
Summary:    A Recorder library in Tizen C API
Version:    0.1.1
Release:    3
Group:      libdevel
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-camcorder)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-media-camera)
BuildRequires:  pkgconfig(capi-media-audio-io)
Requires(post): /sbin/ldconfig  
Requires(postun): /sbin/ldconfig

%description

%package devel
Summary:  A Recorder library in Tizen C API (Development)
Group:    TO_BE/FILLED_IN
Requires: %{name} = %{version}-%{release}

%description devel

%prep
%setup -q


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}


make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}

%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest capi-media-recorder.manifest
%{_libdir}/libcapi-media-recorder.so.*
%{_datadir}/license/%{name}

%files devel
%{_includedir}/media/recorder.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-recorder.so

