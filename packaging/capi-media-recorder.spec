Name:       capi-media-recorder
Summary:    A Media Daemon Recorder library in Tizen Native API
Version:    0.2.4
Release:    0
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(audio-session-mgr)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-media-camera)
BuildRequires:  pkgconfig(mmsvc-camera)
BuildRequires:  pkgconfig(mmsvc-recorder)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(mused)
BuildRequires:  pkgconfig(capi-media-tool)
BuildRequires:  pkgconfig(json)
BuildRequires:  pkgconfig(capi-media-audio-io)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description


%package devel
Summary:  A Media Daemon library in Tizen (Development)
Group:    TO_BE/FILLED_IN
Requires: %{name} = %{version}-%{release}
%description devel

%prep
%setup -q


%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
mkdir -p %{buildroot}/opt/usr/devel
mkdir -p %{buildroot}/usr/bin
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}

%make_install

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%manifest capi-media-recorder.manifest
%{_libdir}/libcapi-media-recorder.so*
%{_datadir}/license/%{name}

%files devel
%{_includedir}/media/*.h
%{_libdir}/pkgconfig/*.pc
