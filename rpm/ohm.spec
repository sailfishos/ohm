Name:       ohm
Summary:    Open Hardware Manager
Version:    1.2.4
Release:    1
Group:      System/Resource Policy
License:    LGPLv2+
URL:        https://git.sailfishos.org/mer-core/ohm
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
Group:      System/Resource Policy
Requires:   %{name}

%description tracing
This package enables verbose logging for %{name}.


%package configs-default
Summary:    Common configuration files for %{name}
Group:      System/Resource Policy
Requires:   %{name} = %{version}-%{release}
Provides:   ohm-config > 1.1.15
Provides:   ohm-configs
Obsoletes:  ohm-config <= 1.1.15

%description configs-default
This package contains common OHM configuration files.


%package plugin-core
Summary:    Common %{name} libraries
Group:      System/Resource Policy
Requires:   %{name} = %{version}-%{release}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description plugin-core
This package contains libraries needed by both for running OHM and
developing OHM plugins.


%package devel
Summary:    Development files for %{name}
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
Development files for %{name}.

%prep
%setup -q -n %{name}-%{version}

%build
echo %{version} | cut -d+ -f1 | cut -d- -f1 > .tarball-version
%autogen --disable-static
%configure --disable-static \
    --without-xauth \
    --with-distro=meego

make %{_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

# make sure we get a plugin config dir even with legacy plugins disabled
mkdir -p %{buildroot}/%{_sysconfdir}/ohm/plugins.d

# enable ohmd in the basic systemd target
install -d %{buildroot}/%{_lib}/systemd/system/basic.target.wants
ln -s ../ohmd.service %{buildroot}/%{_lib}/systemd/system/basic.target.wants/ohmd.service
(cd %{buildroot}/%{_lib}/systemd/system && ln -s ohmd.service dbus-org.freedesktop.ohm.service)

install -d %{buildroot}/%{_libdir}/ohm
install -d %{buildroot}/%{_sharedstatedir}/ohm

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
%dir %{_sharedstatedir}/ohm
%dir %{_sysconfdir}/ohm
%dir %{_sysconfdir}/ohm/plugins.d
%{_sbindir}/*ohm*
/%{_lib}/systemd/system/ohmd.service
/%{_lib}/systemd/system/basic.target.wants/ohmd.service
/%{_lib}/systemd/system/dbus-org.freedesktop.ohm.service
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
