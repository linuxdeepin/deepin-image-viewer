Name:           deepin-image-viewer
Version:        1.3.17
Release:        1
Summary:        Deepin Image Viewer
License:        GPLv3
URL:            https://github.com/linuxdeepin/deepin-image-viewer
Source0:        %{name}_%{version}.orig.tar.xz	


BuildRequires:  gcc-c++
BuildRequires:  freeimage-devel
BuildRequires:  qt5-linguist 
BuildRequires:  qt5-qttools-devel
BuildRequires:  dtkwidget2-devel
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5OpenGL)
BuildRequires:  pkgconfig(Qt5PrintSupport)
BuildRequires:  pkgconfig(Qt5Sql)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  pkgconfig(gio-unix-2.0)
BuildRequires:  pkgconfig(libraw)
BuildRequires:  pkgconfig(libexif)
BuildRequires:  pkgconfig(libstartup-notification-1.0)
BuildRequires:  pkgconfig(xcb-util)
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib
Requires:       hicolor-icon-theme

%description
%{summary}.

%prep
%setup -q
sed -i 's|lrelease|lrelease-qt5|g' viewer/generate_translations.sh
sed -i 's|lrelease|lrelease-qt5|g' viewer/viewer.pro


%build

%qmake_qt5 PREFIX=%{_prefix}
%make_build

%install
%make_install INSTALL_ROOT=%{buildroot}

%check
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop ||:
####appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/*.appdata.xml

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_qt5_plugindir}/imageformats/*.so
%{_datadir}/dbus-1/services/*.service
%{_datadir}/%{name}/
%{_datadir}/dman/%{name}/
#%{_metainfodir}/%{name}.appdata.xml
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/deepin/apps/scalable/%{name}.svg
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg

%changelog
* Thu Jan 31 2019 mosquito <sensor.wen@gmail.com> - 1.3.8-1
- Update to 1.3.8

* Thu Jan 31 2019 Fedora Release Engineering <releng@fedoraproject.org> - 1.3.6-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_30_Mass_Rebuild

* Wed Dec 12 2018 mosquito <sensor.wen@gmail.com> - 1.3.6-1
- Update to 1.3.6

* Thu Nov 29 2018 mosquito <sensor.wen@gmail.com> - 1.3.2-1
- Update to 1.3.2

* Wed Nov 21 2018 mosquito <sensor.wen@gmail.com> - 1.3.1-1
- Update to 1.3.1

* Fri Jul 27 2018 mosquito <sensor.wen@gmail.com> - 1.2.23-1
- Update to 1.2.23

* Thu Jul 19 2018 Adam Williamson <awilliam@redhat.com> - 1.2.16.8-3
- Rebuild for new libraw

* Thu Jul 12 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.16.8-2
- Rebuilt for https://fedoraproject.org/wiki/Fedora_29_Mass_Rebuild

* Sat Feb 10 2018 mosquito <sensor.wen@gmail.com> - 1.2.16.8-1
- Update to 1.2.16.8

* Wed Feb 07 2018 Fedora Release Engineering <releng@fedoraproject.org> - 1.2.16.7-3
- Rebuilt for https://fedoraproject.org/wiki/Fedora_28_Mass_Rebuild

* Thu Jan 11 2018 Igor Gnatenko <ignatenkobrain@fedoraproject.org> - 1.2.16.7-2
- Remove obsolete scriptlets

* Thu Dec 28 2017 mosquito <sensor.wen@gmail.com> - 1.2.16.7-1
- Update to 1.2.16.7

* Sat Dec  9 2017 mosquito <sensor.wen@gmail.com> - 1.2.16.5-1
- Update to 1.2.16.5

* Mon Nov 27 2017 mosquito <sensor.wen@gmail.com> - 1.2.16.4-1
- Update to 1.2.16.4

* Fri Oct 27 2017 mosquito <sensor.wen@gmail.com> - 1.2.16.1-1
- Update to 1.2.16.1

* Mon Aug 21 2017 mosquito <sensor.wen@gmail.com> - 1.2.15-1
- Update to 1.2.15

* Fri Jul 14 2017 mosquito <sensor.wen@gmail.com> - 1.2.14-1.gite77fde5
- Update to 1.2.14

* Fri May 19 2017 mosquito <sensor.wen@gmail.com> - 1.2.13-1.gita6ac784
- Update to 1.2.13

* Tue Mar  7 2017 mosquito <sensor.wen@gmail.com> - 1.2.4-1.gitfad9c98
- Update to 1.2.4

* Sat Jan 21 2017 mosquito <sensor.wen@gmail.com> - 1.2.1-1.git8378500
- Update to 1.2.1

* Tue Jan 17 2017 mosquito <sensor.wen@gmail.com> - 1.2.0-1.git933325f
- Update to 1.2.0

* Fri Jan 06 2017 Jaroslav <cz.guardian@gmail.com> Stepanek 1.1.3-2
- Fixed build dependecies

* Sat Dec 10 2016 Jaroslav <cz.guardian@gmail.com> Stepanek 1.1.3-1
- Initial package build
