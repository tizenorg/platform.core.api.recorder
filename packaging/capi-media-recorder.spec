Name:       capi-media-recorder
Summary:    A Recorder library in Tizen C API
Version:    0.2.0
Release:    0
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(mm-camcorder)
BuildRequires:  pkgconfig(audio-session-mgr)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-media-camera)
BuildRequires:  pkgconfig(capi-media-audio-io)


%description
A Recorder library in Tizen C API


%package devel
Summary:  A Recorder library in Tizen C API (Development)
Requires: %{name} = %{version}-%{release}


%description devel
A Recorder library in Tizen C API

Development Package.


%prep
%setup -q


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DFULLVER=%{version} -DMAJORVER=${MAJORVER}
make %{?jobs:-j%jobs}


%install
%make_install


%post -p /sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%manifest capi-media-recorder.manifest
%license LICENSE.APLv2
%{_libdir}/libcapi-media-recorder.so.*

%files devel
%{_includedir}/media/recorder.h
%{_libdir}/pkgconfig/*.pc
%{_libdir}/libcapi-media-recorder.so

