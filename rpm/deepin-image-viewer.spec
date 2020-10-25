Name:           deepin-image-viewer
Version:        5.6.3.47
Release:        1%{?dist}
Summary:        Deepin Image Viewer
License:        GPLv3
URL:            https://github.com/linuxdeepin/deepin-image-viewer
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz
Source1:        %{name}-appdata.xml

BuildRequires:  gcc-c++
BuildRequires:  freeimage-devel
BuildRequires:  qt5-linguist
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5OpenGL)
BuildRequires:  pkgconfig(Qt5PrintSupport)
BuildRequires:  pkgconfig(Qt5Sql)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  pkgconfig(Qt5Multimedia)
BuildRequires:  pkgconfig(dtkwidget) >= 2.0.6
BuildRequires:  pkgconfig(gio-unix-2.0)
BuildRequires:  pkgconfig(libraw)
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(libstartup-notification-1.0)
BuildRequires:  pkgconfig(xcb-util)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(gio-qt)
BuildRequires:  udisks2-qt5-devel
BuildRequires:  qt5-qtbase-private-devel
%{?_qt5:Requires: %{_qt5}%{?_isa} = %{_qt5_version}}
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
Requires:       hicolor-icon-theme

%description
%{summary}.

%prep
%autosetup -p1
# Fedora freeimage 3.19.0-0.1.svn1859 does not support FAXG3
sed -i '/FIF_FAXG3/d' viewer/utils/unionimage.cpp

%build
# help find (and prefer) qt5 utilities, e.g. qmake, lrelease
export PATH=%{_qt5_bindir}:$PATH
%qmake_qt5 PREFIX=%{_prefix}
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}
install -Dm644 %SOURCE1 %{buildroot}%{_metainfodir}/%{name}.appdata.xml

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop ||:
appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/*.appdata.xml

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_qt5_plugindir}/imageformats/*.so
%{_datadir}/dbus-1/services/*.service
%{_datadir}/%{name}/
%{_datadir}/dman/%{name}/
%{_metainfodir}/%{name}.appdata.xml
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg

%changelog
