Name:       ohm
Summary:    Open Hardware Manager
Version:    1.3.0
Release:    1
License:    LGPLv2+
URL:        https://github.com/sailfishos/ohm
Source0:    %{name}-%{version}.tar.gz
Source1:    ohm-rpmlintrc
Requires:   ohm-configs
Requires:   systemd
Requires(preun): systemd
Requires(post): systemd
Requires(postun): systemd
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(dbus-1) >= 0.70
BuildRequires:  pkgconfig(dbus-glib-1) >= 0.70
BuildRequires:  pkgconfig(check)
BuildRequires:  pkgconfig(libsimple-trace)

%description
Open Hardware Manager.


%package tracing
Summary:    Enable tracing for %{name}
Requires:   %{name}

%description tracing
This package enables verbose logging for %{name}.


%package configs-default
Summary:    Common configuration files for %{name}
Requires:   %{name} = %{version}-%{release}
Provides:   ohm-config > 1.1.15
Provides:   ohm-configs
Obsoletes:  ohm-config <= 1.1.15

%description configs-default
This package contains common OHM configuration files.


%package plugin-core
Summary:    Common %{name} libraries
Requires:   %{name} = %{version}-%{release}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description plugin-core
This package contains libraries needed by both for running OHM and
developing OHM plugins.


%package devel
Summary:    Development files for %{name}
Requires:   %{name} = %{version}-%{release}

%description devel
Development files for %{name}.

%prep
%setup -q -n %{name}-%{version}

%build
echo %{version} | cut -d+ -f1 | cut -d- -f1 > .tarball-version
./autogen.sh
%configure --disable-static \
    --without-xauth \
    --with-distro=meego

make %{_smp_mflags}

%install
%make_install

# make sure we get a plugin config dir even with legacy plugins disabled
mkdir -p %{buildroot}/%{_sysconfdir}/ohm/plugins.d

# enable ohmd in the basic systemd target
install -d %{buildroot}%{_unitdir}/basic.target.wants
ln -s ../ohmd.service %{buildroot}%{_unitdir}/basic.target.wants/ohmd.service
(cd %{buildroot}%{_unitdir} && ln -s ohmd.service dbus-org.freedesktop.ohm.service)

install -d %{buildroot}/%{_libdir}/ohm
install -d %{buildroot}/%{_sharedstatedir}/ohm

rm -rf %{buildroot}/%{_libdir}/lib*.la

%check
./tests/test-fact

%preun
if [ "$1" -eq 0 ]; then
systemctl stop ohmd.service || :
fi

%post
systemctl daemon-reload || :
systemctl reload-or-try-restart ohmd.service || :

%postun
systemctl daemon-reload || :

%post plugin-core -p /sbin/ldconfig

%postun plugin-core -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%license COPYING
%dir %{_sharedstatedir}/ohm
%dir %{_sysconfdir}/ohm
%dir %{_sysconfdir}/ohm/plugins.d
%{_sbindir}/*ohm*
%{_unitdir}/ohmd.service
%{_unitdir}/basic.target.wants/ohmd.service
%{_unitdir}/dbus-org.freedesktop.ohm.service
%{_datadir}/dbus-1/system-services/org.freedesktop.ohm.service
%config %{_sysconfdir}/dbus-1/system.d/ohm.conf

%files tracing
%defattr(-,root,root,-)
%{_sysconfdir}/sysconfig/ohmd.debug

%files configs-default
%defattr(-,root,root,-)
%config %{_sysconfdir}/ohm/modules.ini

%files plugin-core
%defattr(-,root,root,-)
%{_libdir}/libohmplugin.so.*
%{_libdir}/libohmfact.so.*
%dir %{_libdir}/ohm

%files devel
%defattr(-,root,root,-)
%{_includedir}/ohm
%{_libdir}/pkgconfig/*
%{_libdir}/libohmplugin.so
%{_libdir}/libohmfact.so
