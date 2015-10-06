Name:       capi-media-recorder
Summary:    A Recorder library in Tizen Native API
Version:    0.2.6
Release:    1
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-media-camera)
BuildRequires:  pkgconfig(mmsvc-camera)
BuildRequires:  pkgconfig(mmsvc-recorder)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(mused)
BuildRequires:  pkgconfig(capi-media-tool)
BuildRequires:  pkgconfig(json)
BuildRequires:  pkgconfig(capi-media-audio-io)
BuildRequires:  pkgconfig(mm-camcorder)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
A Recorder library in Tizen Native API


%package devel
Summary:  A Recorder library in Tizen C API (Development)
Requires: %{name} = %{version}-%{release}


%description devel
A Recorder library in Tizen Native API Development Package.


%prep
%setup -q


%build
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} -DFULLVER=%{version} -DMAJORVER=${MAJORVER}
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
