Name:           harbour-osmin
Version:        1.0.6
Release:        5
Summary:        Navigator
License:        GPL-3.0-or-later
Group:          Productivity/Location
URL:            http://janbar.github.io/osmin/index.html
Source0:        https://github.com/janbar/osmin/archive/%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  git
BuildRequires:  gcc-c++
BuildRequires:  openssl-devel
BuildRequires:  zlib-devel
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5Xml)
BuildRequires:  pkgconfig(Qt5Sql)
BuildRequires:  pkgconfig(Qt5Positioning)
BuildRequires:  pkgconfig(libxml-2.0)

%define __provides_exclude_from ^%{_datadir}/.*$

%description
A navigator.

%prep
%setup -q -n %{name}-%{version}

%build
mkdir -p build-%{_target_cpu}
pushd build-%{_target_cpu}
%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SAILFISHOS=ON ..
make -j2 VERBOSE=1
popd

%install
pushd build-%{_target_cpu}
%make_install -C build
popd

# strip symbols app
strip --keep-symbol=main %{buildroot}%{_bindir}/%{name}

%files
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/
%{_datadir}/%{name}

%changelog
