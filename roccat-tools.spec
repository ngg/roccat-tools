%define luaversion 5.3
Name: roccat
Version: 3.7.0
Release: 1
License: GPL
Source: roccat-tools-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-build
BuildRequires: gtk2-devel >= 2.20
BuildRequires: cmake >= 2.6.4
BuildRequires: dbus-glib
BuildRequires: dbus-glib-devel
BuildRequires: libgudev1-devel
BuildRequires: libX11-devel
BuildRequires: libgaminggear-devel >= 0.11.1
BuildRequires: lua-devel = %{luaversion}
BuildRequires: gettext >= 0.15
ExclusiveOS: linux
URL: http://roccat.sourceforge.net
Packager: Stefan Achatz <erazor_de@users.sourceforge.net>

%define default_udevdir /lib/udev/rules.d
%{!?udevdir: %define udevdir %{default_udevdir}}

%define default_eventhandlerdir %{_libdir}/roccat
%{!?eventhandlerdir: %define eventhandlerdir %{default_eventhandlerdir}}

%define default_gfx_plugindir %{_libdir}/gaminggear_plugins
%{!?gfx_plugindir: %define gfx_plugindir %{default_gfx_plugindir}}

Summary: Roccat common files
Group: System Environment/Libraries

%package arvo
Requires: roccat
Summary: Roccat Arvo userlandtools
Group: Applications/System

%package isku
Requires: roccat
Summary: Roccat Isku userlandtools
Group: Applications/System

%package iskufx
Requires: roccat
Requires: roccat-isku
Summary: Roccat IskuFX userlandtools
Group: Applications/System

%package kone
Requires: roccat
Summary: Roccat Kone userlandtools
Group: Applications/System

%package koneplus
Requires: roccat
Summary: Roccat Kone[+] userlandtools
Group: Applications/System

%package konepure
Requires: roccat
Requires: roccat-konextd
Summary: Roccat KonePure userlandtools
Group: Applications/System

%package konepuremilitary
Requires: roccat
Requires: roccat-konepure
Summary: Roccat KonePureMilitary/Black userlandtools
Group: Applications/System

%package konepureoptical
Requires: roccat
Requires: roccat-konepure
Summary: Roccat KonePureOptical userlandtools
Group: Applications/System

%package konextd
Requires: roccat
Requires: roccat-koneplus
Summary: Roccat KoneXTD userlandtools
Group: Applications/System

%package konextdoptical
Requires: roccat
Requires: roccat-konepuremilitary
Summary: Roccat KoneXTDOptical userlandtools
Group: Applications/System

%package kovaplus
Requires: roccat
Summary: Roccat Kova[+] userlandtools
Group: Applications/System

%package lua
Requires: roccat
Summary: Roccat Lua userlandtools
Group: Applications/System

%package nyth
Requires: roccat
Summary: Roccat Nyth userlandtools
Group: Applications/System

%package pyra
Requires: roccat
Summary: Roccat Pyra userlandtools
Group: Applications/System

%package ryosmk
Requires: roccat
Requires: lua = 5.1
Obsoletes: roccat-ryos
Summary: Roccat Ryos MK userlandtools
Group: Applications/System

%package ryostkl
Requires: roccat
Requires: roccat-ryosmk
Requires: lua = 5.1
Summary: Roccat Ryos TKL userlandtools
Group: Applications/System

%package savu
Requires: roccat
Summary: Roccat Savu userlandtools
Group: Applications/System

%package tyon
Requires: roccat
Summary: Roccat Tyon userlandtools
Group: Applications/System

%description
Roccat consists of a shared library and other files shared by device specific
applications for roccat hardware.

%description arvo
Arvo consists of a console application and a GUI application. It helps users to
manipulate the Profiles and Settings of a Roccat Arvo keyboard.

%description isku
Isku consists of a console application and a GUI application. It helps users to
manipulate the Profiles and Settings of a Roccat Isku keyboard.

%description iskufx
IskuFX consists of a console application and a GUI application. It helps users to
manipulate the Profiles and Settings of a Roccat IskuFX keyboard.

%description kone
Kone consists of a console application and a GUI application. It helps users to
manipulate the Profiles and Settings of a Roccat Kone mouse.

%description koneplus
Koneplus consists of a console application and a GUI application. It helps users
to manipulate the Profiles and Settings of a Roccat Kone[+] mouse.

%description konepure
Konepure consists of a console application and a GUI application. It helps users
to manipulate the Profiles and Settings of a Roccat KonePure mouse.

%description konepuremilitary
Konepuremilitary consists of a console application and a GUI application. It helps
to manipulate the Profiles and Settings of a Roccat KonePureMilitary/Black mouse.

%description konepureoptical
Konepureoptical consists of a console application and a GUI application. It helps
users to manipulate the Profiles and Settings of a Roccat KonePureOptical mouse.

%description konextd
Konextd consists of a console application and a GUI application. It helps users
to manipulate the Profiles and Settings of a Roccat KoneXTD mouse.

%description konextdoptical
Konextdoptical consists of a console application and a GUI application. It helps
users to manipulate the Profiles and Settings of a Roccat KoneXTDOptical mouse.

%description kovaplus
Kovaplus consists of a console application and a GUI application. It helps users
to manipulate the Profiles and Settings of a Roccat Kova[+] mouse.

%description lua
Lua consists of a console application and a GUI application. It helps users to
manipulate the Settings of a Roccat Lua mouse.

%description nyth
Nyth consists of a console application and a GUI application. It helps users to
manipulate the Settings of a Roccat Nyth mouse.

%description pyra
Pyra consists of a console application and a GUI application. It helps users to
manipulate the Profiles and Settings of a Roccat Pyra mouse.

%description ryosmk
Ryosmk consists of a console application and a GUI application. It helps users
to manipulate the Profiles and Settings of Roccat Ryos MK keyboards.

%description ryostkl
Ryostkl consists of a console application and a GUI application. It helps users
to manipulate the Profiles and Settings of Roccat Ryos TKL keyboards.

%description savu
Savu consists of a console application and a GUI application. It helps users to
manipulate the Profiles and Settings of a Roccat Savu mouse.

%description tyon
Tyon consists of a console application and a GUI application. It helps users to
manipulate the Profiles and Settings of a Roccat Tyon mouse.

%prep
%setup -q -n roccat-tools-%{version}
%{__mkdir} build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="%{_prefix}" -DLIBDIR="%{_libdir}" -DUDEVDIR="%{udevdir}" -DEVENTHANDLER_PLUGIN_DIR="%{eventhandlerdir}" -DWITH_LUA=%{luaversion} ..

%build
cd build
%{__make} %{?_smp_mflags}

%install
cd build
%{__rm} -rf "%{buildroot}"
%{__make} %{?_smp_mflags} DESTDIR="%{buildroot}" install
%{__mkdir} -p "%{buildroot}/var/lib/roccat"

%clean
%{__rm} -rf "%{buildroot}"

%pre
/usr/sbin/groupadd --force --system roccat >/dev/null || :

%post
/sbin/ldconfig
touch --no-create %{_prefix}/share/icons/hicolor &>/dev/null || :

%postun
/sbin/ldconfig
if [ $1 -eq 0 ] ; then
  touch --no-create %{_prefix}/share/icons/hicolor &>/dev/null
  gtk-update-icon-cache %{_prefix}/share/icons/hicolor &>dev/null || :
fi

%posttrans
gtk-update-icon-cache %{_prefix}/share/icons/hicolor &>dev/null || :

%post arvo
/sbin/ldconfig

%postun arvo
/sbin/ldconfig

%post isku
/sbin/ldconfig

%postun isku
/sbin/ldconfig

%post iskufx
/sbin/ldconfig

%postun iskufx
/sbin/ldconfig

%post kone
/sbin/ldconfig

%postun kone
/sbin/ldconfig

%post koneplus
/sbin/ldconfig

%postun koneplus
/sbin/ldconfig

%post konepure
/sbin/ldconfig

%postun konepure
/sbin/ldconfig

%post konepuremilitary
/sbin/ldconfig

%postun konepuremilitary
/sbin/ldconfig

%post konepureoptical
/sbin/ldconfig

%postun konepureoptical
/sbin/ldconfig

%post konextd
/sbin/ldconfig

%post konextdoptical
/sbin/ldconfig

%postun konextd
/sbin/ldconfig

%post kovaplus
/sbin/ldconfig

%postun kovaplus
/sbin/ldconfig

%post lua
/sbin/ldconfig

%postun lua
/sbin/ldconfig

%post nyth
/sbin/ldconfig

%postun nyth
/sbin/ldconfig

%post pyra
/sbin/ldconfig

%postun pyra
/sbin/ldconfig

%post ryosmk
/sbin/ldconfig

%postun ryosmk
/sbin/ldconfig

%postun ryostkl
/sbin/ldconfig

%post savu
/sbin/ldconfig

%postun savu
/sbin/ldconfig

%post tyon
/sbin/ldconfig

%postun tyon
/sbin/ldconfig

%files
%defattr(-,root,root)
%doc README COPYING Changelog
%{_bindir}/roccateventhandler
%{_libdir}/libroccat.so*
%{_libdir}/libroccatwidget.so*
%{_datadir}/icons
%{_datadir}/roccat/sounds
/etc/xdg/autostart/roccateventhandler.desktop
%{_datadir}/locale/*/LC_MESSAGES/roccat-tools.mo
%attr(2770,root,roccat) /var/lib/roccat

%files arvo
%defattr(-,root,root)
%{udevdir}/90-roccat-arvo.rules
%{_bindir}/roccatarvo*
%{_libdir}/libroccatarvo.so*
%{eventhandlerdir}/libarvoeventhandler.so
%{_datadir}/applications/roccatarvoconfig.desktop
%{_mandir}/*/man1/roccatarvo*

%files isku
%defattr(-,root,root)
%{udevdir}/90-roccat-isku.rules
%{_bindir}/roccatisku*
%{_libdir}/libroccatisku.so*
%{_libdir}/libroccatiskuwidget.so*
%{eventhandlerdir}/libiskueventhandler.so
%{_datadir}/applications/roccatiskuconfig.desktop
%{_mandir}/*/man1/roccatisku*

%files iskufx
%defattr(-,root,root)
%{udevdir}/90-roccat-iskufx.rules
%{_bindir}/roccatiskufx*
%{_libdir}/libroccatiskufx.so*
%{eventhandlerdir}/libiskufxeventhandler.so
%{gfx_plugindir}/libiskufxgfxplugin.so
%{_datadir}/applications/roccatiskufxconfig.desktop
%{_mandir}/*/man1/roccatiskufx*

%files kone
%defattr(-,root,root)
%{udevdir}/90-roccat-kone.rules
%{_bindir}/roccatkone*
%{_libdir}/libroccatkone.so*
%{eventhandlerdir}/libkoneeventhandler.so
%{_datadir}/applications/roccatkoneconfig.desktop
%{_mandir}/*/man1/roccatkone*

%files koneplus
%defattr(-,root,root)
%{udevdir}/90-roccat-koneplus.rules
%{_bindir}/roccatkoneplus*
%{_libdir}/libroccatkoneplus.so*
%{_libdir}/libroccatkonepluswidget.so*
%{eventhandlerdir}/libkonepluseventhandler.so
%{gfx_plugindir}/libkoneplusgfxplugin.so
%{_datadir}/applications/roccatkoneplusconfig.desktop
%{_mandir}/*/man1/roccatkoneplus*

%files konepure
%defattr(-,root,root)
%{udevdir}/90-roccat-konepure.rules
%{_bindir}/roccatkonepure*
%{_libdir}/libroccatkonepure.so*
%{_libdir}/libroccatkonepurewidget.so*
%{eventhandlerdir}/libkonepureeventhandler.so
%{gfx_plugindir}/libkonepuregfxplugin.so
%{_datadir}/applications/roccatkonepureconfig.desktop
%{_mandir}/*/man1/roccatkonepure*

%files konepuremilitary
%defattr(-,root,root)
%{udevdir}/90-roccat-konepuremilitary.rules
%{_bindir}/roccatkonepuremilitary*
%{_libdir}/libroccatkonepuremilitary.so*
%{_libdir}/libroccatkonepuremilitarywidget.so*
%{eventhandlerdir}/libkonepuremilitaryeventhandler.so
%{gfx_plugindir}/libkonepuremilitarygfxplugin.so
%{_datadir}/applications/roccatkonepuremilitaryconfig.desktop
%{_mandir}/*/man1/roccatkonepuremilitary*

%files konepureoptical
%defattr(-,root,root)
%{udevdir}/90-roccat-konepureoptical.rules
%{_bindir}/roccatkonepureoptical*
%{_libdir}/libroccatkonepureoptical.so*
%{eventhandlerdir}/libkonepureopticaleventhandler.so
%{gfx_plugindir}/libkonepureopticalgfxplugin.so
%{_datadir}/applications/roccatkonepureopticalconfig.desktop
%{_mandir}/*/man1/roccatkonepureoptical*

%files konextd
%defattr(-,root,root)
%{udevdir}/90-roccat-konextd.rules
%{_bindir}/roccatkonextd*
%{_libdir}/libroccatkonextd.so*
%{_libdir}/libroccatkonextdwidget.so*
%{eventhandlerdir}/libkonextdeventhandler.so
%{gfx_plugindir}/libkonextdgfxplugin.so
%{_datadir}/applications/roccatkonextdconfig.desktop
%{_mandir}/*/man1/roccatkonextd*

%files konextdoptical
%defattr(-,root,root)
%{udevdir}/90-roccat-konextdoptical.rules
%{_bindir}/roccatkonextdoptical*
%{_libdir}/libroccatkonextdoptical.so*
%{eventhandlerdir}/libkonextdopticaleventhandler.so
%{gfx_plugindir}/libkonextdopticalgfxplugin.so
%{_datadir}/applications/roccatkonextdopticalconfig.desktop
%{_mandir}/*/man1/roccatkonextdoptical*

%files kovaplus
%defattr(-,root,root)
%{udevdir}/90-roccat-kovaplus.rules
%{_bindir}/roccatkovaplus*
%{_libdir}/libroccatkovaplus.so*
%{eventhandlerdir}/libkovapluseventhandler.so
%{_datadir}/applications/roccatkovaplusconfig.desktop
%{_mandir}/*/man1/roccatkovaplus*

%files lua
%defattr(-,root,root)
%{udevdir}/90-roccat-lua.rules
%{_bindir}/roccatlua*
%{_libdir}/libroccatlua.so*
%{eventhandlerdir}/libluaeventhandler.so
%{_datadir}/applications/roccatluaconfig.desktop
%{_mandir}/*/man1/roccatlua*

%files nyth
%defattr(-,root,root)
%{udevdir}/90-roccat-nyth.rules
%{_bindir}/roccatnyth*
%{_libdir}/libroccatnyth.so*
%{eventhandlerdir}/libnytheventhandler.so
%{gfx_plugindir}/libnythgfxplugin.so
%{_datadir}/applications/roccatnythconfig.desktop
%{_mandir}/*/man1/roccatnyth*

%files pyra
%defattr(-,root,root)
%{udevdir}/90-roccat-pyra.rules
%{_bindir}/roccatpyra*
%{_libdir}/libroccatpyra.so*
%{eventhandlerdir}/libpyraeventhandler.so
%{_datadir}/applications/roccatpyraconfig.desktop
%{_mandir}/*/man1/roccatpyra*

%files ryosmk
%defattr(-,root,root)
%{udevdir}/90-roccat-ryosmk.rules
%{_bindir}/roccatryosmk*
%{_libdir}/libroccatryosmk.so*
%{_libdir}/libroccatryosmkwidget.so*
%{eventhandlerdir}/libryosmkeventhandler.so
%{_datadir}/applications/roccatryosmkconfig.desktop
%{_mandir}/*/man1/roccatryosmk*
%{_datadir}/roccat/ryosmk_ripple_modules

%files ryostkl
%defattr(-,root,root)
%{udevdir}/90-roccat-ryostkl.rules
%{_bindir}/roccatryostkl*
%{_libdir}/libroccatryostkl.so*
%{eventhandlerdir}/libryostkleventhandler.so
%{_datadir}/applications/roccatryostklconfig.desktop
%{_mandir}/*/man1/roccatryostkl*
%{_datadir}/roccat/ryostkl_ripple_modules

%files savu
%defattr(-,root,root)
%{udevdir}/90-roccat-savu.rules
%{_bindir}/roccatsavu*
%{_libdir}/libroccatsavu.so*
%{eventhandlerdir}/libsavueventhandler.so
%{_datadir}/applications/roccatsavuconfig.desktop
%{_mandir}/*/man1/roccatsavu*

%files tyon
%defattr(-,root,root)
%{udevdir}/90-roccat-tyon.rules
%{_bindir}/roccattyon*
%{_libdir}/libroccattyon.so*
%{eventhandlerdir}/libtyoneventhandler.so
%{gfx_plugindir}/libtyongfxplugin.so
%{_datadir}/applications/roccattyonconfig.desktop
%{_mandir}/*/man1/roccattyon*

%changelog
* Sun Sep 13 2015 Stefan Achatz <erazor_de@users.sourceforge.net> 3.7.0-1
- Initial version
