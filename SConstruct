## SCons build recipe for the GPSD project

# Important targets:
#
# build      - build the software (default)
# dist       - make distribution tarball
# install    - install programs, libraries, and manual pages
# uninstall  - undo an install
#
# check      - run regression and unit tests.
# audit      - run code-auditing tools
# testbuild  - test-build the code from a tarball
# website    - refresh the website
# release    - ship a release
#
# --clean    - clean all normal build targets
# sconsclean - clean up scons dotfiles (but not the database)
#
# Setting the DESTDIR environment variable will prefix the install destinations
# without changing the --prefix prefix.

# Unfinished items:
# * Out-of-directory builds: see http://www.scons.org/wiki/UsingBuildDir
# * Coveraging mode: gcc "-coverage" flag requires a hack for building the python bindings

# Release identification begins here
gpsd_version = "3.17~dev"

# client library version
libgps_version_current   = 22
libgps_version_revision  = 0
libgps_version_age       = 0

# Release identification ends here

# Hosting information (mainly used for templating web pages) begins here
# Each variable foo has a corresponding @FOO@ expanded in .in files.
# There are no project-dependent URLs or references to the hosting site
# anywhere else in the distribution; preserve this property!
sitename   = "Savannah"
sitesearch = "catb.org"
website    = "http://catb.org/gpsd"
mainpage   = "https://savannah.nongnu.org/projects/gpsd/"
webupload  = "login.ibiblio.org:/public/html/catb/gpsd"
cgiupload  = "thyrsus.com:/home/www/thyrsus.com/cgi-bin/"
scpupload  = "dl.sv.nongnu.org:/releases/gpsd/"
mailman    = "https://lists.nongnu.org/mailman/listinfo/"
admin      = "https://savannah.nongnu.org/project/admin/?group=gpsd"
download   = "http://download-mirror.savannah.gnu.org/releases/gpsd/"
bugtracker = "https://savannah.nongnu.org/bugs/?group=gpsd"
browserepo = "http://git.savannah.gnu.org/cgit/gpsd.git"
clonerepo  = "https://savannah.nongnu.org/git/?group=gpsd"
gitrepo    = "git://git.savannah.nongnu.org/gpsd.git"
webform    = "http://www.thyrsus.com/cgi-bin/gps_report.cgi"
formserver = "www@thyrsus.com"
devmail    = "gpsd-dev@lists.nongnu.org"
usermail   = "gpsd-users@lists.nongnu.org"
annmail    = "gpsd-announce@nongnu.org"
ircchan    = "irc://chat.freenode.net/#gpsd"
tiplink    = "<a href='http://gratipay.com/esr'>leave a tip at Gratipay</a>"
tipwidget  = ("<script data-gratipay-username='esr' "
              "data-gratipay-widget='button' src='//gttp.co/v1.js'></script>")
# Hosting information ends here

EnsureSConsVersion(2, 3, 0)

import copy, os, sys, glob, re, platform, time
from distutils import sysconfig
from distutils.util import get_platform
import SCons

# replacement for functions from the commands module, which is deprecated.
from subprocess import PIPE, STDOUT, Popen


def _getstatusoutput(cmd, input=None, cwd=None, env=None):
    pipe = Popen(cmd, shell=True, cwd=cwd, env=env, stdout=PIPE, stderr=STDOUT)
    (output, errout) = pipe.communicate(input=input)
    status = pipe.returncode
    return (status, output)


def _getoutput(cmd, input=None, cwd=None, env=None):
    return _getstatusoutput(cmd, input, cwd, env)[1]


#
# Build-control options
#

# Start by reading configuration variables from the cache
opts = Variables('.scons-option-cache')

systemd_dir = '/lib/systemd/system'
systemd = os.path.exists(systemd_dir)

# Set distribution-specific defaults here
imloads = True

boolopts = (
    # GPS protocols
    ("nmea0183",      True,  "NMEA0183 support"),
    ("ashtech",       True,  "Ashtech support"),
    ("earthmate",     True,  "DeLorme EarthMate Zodiac support"),
    ("evermore",      True,  "EverMore binary support"),
    ("fv18",          True,  "San Jose Navigation FV-18 support"),
    ("garmin",        True,  "Garmin kernel driver support"),
    ("garmintxt",     True,  "Garmin Simple Text support"),
    ("geostar",       True,  "Geostar Protocol support"),
    ("itrax",         True,  "iTrax hardware support"),
    ("mtk3301",       True,  "MTK-3301 support"),
    ("navcom",        True,  "Navcom NCT support"),
    ("oncore",        True,  "Motorola OnCore chipset support"),
    ("sirf",          True,  "SiRF chipset support"),
    ("superstar2",    True,  "Novatel SuperStarII chipset support"),
    ("tnt",           True,  "True North Technologies support"),
    ("tripmate",      True,  "DeLorme TripMate support"),
    ("tsip",          True,  "Trimble TSIP support"),
    ("ublox",         True,  "u-blox Protocol support"),
    ("fury",          True,  "Jackson Labs Fury and Firefly support"),
    ("nmea2000",      True,  "NMEA2000/CAN support"),
    # Non-GPS protocols
    ("aivdm",         True,  "AIVDM support"),
    ("gpsclock",      True,  "GPSClock support"),
    ("ntrip",         True,  "NTRIP support"),
    ("oceanserver",   True,  "OceanServer support"),
    ("isync",         True,  "Spectratime iSync LNRClok/GRCLOK support"),
    ("rtcm104v2",     True,  "rtcm104v2 support"),
    ("rtcm104v3",     True,  "rtcm104v3 support"),
    ("passthrough",   True,  "build support for passing through JSON"),
    # Time service
    ("ntp",           True,  "NTP time hinting support"),
    ("ntpshm",        True,  "NTP time hinting via shared memory"),
    ("pps",           True,  "PPS time syncing support"),
    ("oscillator",    True,  "Disciplined oscillator support"),
    # Export methods
    ("socket_export", True,  "data export over sockets"),
    ("dbus_export",   True,  "enable DBUS export support"),
    ("shm_export",    True,  "export via shared memory"),
    # Communication
    ('usb',           True,  "libusb support for USB devices"),
    ("bluez",         True,  "BlueZ support for Bluetooth devices"),
    ("ipv6",          True,  "build IPv6 support"),
    ("netfeed",       True,  "build support for handling TCP/IP data sources"),
    # Other daemon options
    ("force_global",  False, "force daemon to listen on all addressses"),
    ("timing",        False, "latency timing support"),
    ("control_socket", True,  "control socket for hotplug notifications"),
    ("systemd",       systemd, "systemd socket activation"),
    ("stash",         True,  "enable stash buffer"),
    # Client-side options
    ("clientdebug",   True,  "client debugging support"),
    ("ncurses",       True,  "build with ncurses"),
    ("libgpsmm",      True,  "build C++ bindings"),
    ("qt",            True,  "build QT bindings"),
    # Daemon options
    ("reconfigure",   True,  "allow gpsd to change device settings"),
    ("controlsend",   True,  "allow gpsctl/gpsmon to change device settings"),
    ("nofloats",      False, "float ops are expensive, suppress error estimates"),
    ("squelch",       False, "squelch gpsd_log/gpsd_hexdump to save cpu"),
    # Build control
    ("shared",        True,  "build shared libraries, not static"),
    ("implicit_link", imloads, "implicit linkage is supported in shared libs"),
    ("python",        True,  "build Python support and modules."),
    ("debug",         False, "include debug information in build"),
    ("profiling",     False, "build with profiling enabled"),
    ("coveraging",    False, "build with code coveraging enabled"),
    ("nostrip",       False, "don't symbol-strip binaries at link time"),
    ("manbuild",      True,  "build help in man and HTML formats"),
    ("leapfetch",     True,  "fetch up-to-date data on leap seconds."),
    ("minimal",       False, "turn off every option not set on the command line"),
    ("xgps",          True,  "include xgps and xgpsspeed."),
    # Test control
    ("slow",          False, "run tests with realistic (slow) delays"),
    )
for (name, default, help) in boolopts:
    opts.Add(BoolVariable(name, help, default))

# Gentoo, Fedora, opensuse systems use uucp for ttyS* and ttyUSB*
if os.path.exists("/etc/gentoo-release"):
    def_group = "uucp"
else:
    def_group = "dialout"

nonboolopts = (
    ("gpsd_user",           "nobody",      "privilege revocation user",),
    ("gpsd_group",          def_group,     "privilege revocation group"),
    ("prefix",              "/usr/local",  "installation directory prefix"),
    ("python_libdir",       sysconfig.get_python_lib(plat_specific=1),
                                           "Python module directory prefix"),
    ("max_clients",         '64',          "maximum allowed clients"),
    ("max_devices",         '4',           "maximum allowed devices"),
    ("fixed_port_speed",    0,             "fixed serial port speed"),
    ("fixed_stop_bits",     0,             "fixed serial port stop bits"),
    ("target",              "",            "cross-development target"),
    ("sysroot",             "",            "cross-development system root"),
    ("qt_versioned",        "",            "version for versioned Qt"),
    )
for (name, default, help) in nonboolopts:
    opts.Add(name, help, default)

pathopts = (
    ("sysconfdir",          "etc",           "system configuration directory"),
    ("bindir",              "bin",           "application binaries directory"),
    ("includedir",          "include",       "header file directory"),
    ("libdir",              "lib",           "system libraries"),
    ("sbindir",             "sbin",          "system binaries directory"),
    ("mandir",              "share/man",     "manual pages directory"),
    ("docdir",              "share/doc",     "documents directory"),
    ("udevdir",             "/lib/udev",     "udev rules directory"),
    ("pkgconfig",           "$libdir/pkgconfig", "pkgconfig file directory"),
    )
for (name, default, help) in pathopts:
    opts.Add(PathVariable(name, help, default, PathVariable.PathAccept))

#
# Environment creation
#
import_env = (
    "MACOSX_DEPLOYMENT_TARGET",  # Required by MacOSX 10.4 (and probably earlier)
    "DISPLAY",         # Required for dia to run under scons
    "GROUPS",          # Required by gpg
    "HOME",            # Required by gpg
    "LOGNAME",         # LOGNAME is required for the flocktest production.
    'PATH',            # Required for ccache and Coverity scan-build
    'PKG_CONFIG_PATH',  # Set .pc file directory in a crossbuild
    'PKG_CONFIG_SYSROOT_DIR',  # Pass more environment variables to pkg-config (required for crossbuilds)
    'PKG_CONFIG_LIBDIR',      # Pass more environment variables to pkg-config (required for crossbuilds)
    'STAGING_DIR',     # Required by the OpenWRT and CeroWrt builds.
    'STAGING_PREFIX',  # Required by the OpenWRT and CeroWrt builds.
    'WRITE_PAD',       # So we can test WRITE_PAD values on the fly.
    )
envs = {}
for var in import_env:
    if var in os.environ:
        envs[var] = os.environ[var]
envs["GPSD_HOME"] = os.getcwd()

env = Environment(tools=["default", "tar", "textfile"], options=opts, ENV=envs)
opts.Save('.scons-option-cache', env)
env.SConsignFile(".sconsign.dblite")

#  Minimal build turns off every option not set on the command line,
if env['minimal']:
    for (name, default, help) in boolopts:
        if default is True and not ARGUMENTS.get(name):
            env[name] = False

# NTPSHM requires NTP
if env['ntpshm']:
    env['ntp'] = True

for (name, default, help) in pathopts:
    env[name] = env.subst(env[name])

env['VERSION'] = gpsd_version
env['PYTHON'] = sys.executable

# Set defaults from environment.  Note that scons doesn't cope well
# with multi-word CPPFLAGS/LDFLAGS/SHLINKFLAGS values; you'll have to
# explicitly quote them or (better yet) use the "=" form of GNU option
# settings.
env['STRIP'] = "strip"
env['PKG_CONFIG'] = "pkg-config"
for i in ["AR", "ARFLAGS", "CCFLAGS", "CFLAGS", "CC", "CXX", "CXXFLAGS", "LINKFLAGS", "STRIP", "PKG_CONFIG", "LD", "TAR"]:
    if i in os.environ:
        j = i
        if i == "LD":
            i = "SHLINK"
        if i in ("CFLAGS", "CCFLAGS", "LINKFLAGS"):
            env.Replace(**{j: Split(os.getenv(i))})
        else:
            env.Replace(**{j: os.getenv(i)})
for flag in ["LDFLAGS", "SHLINKFLAGS", "CPPFLAGS"]:
    if i in os.environ:
        env.MergeFlags({flag: [os.getenv(flag)]})


# Keep scan-build options in the environment
for key, value in os.environ.iteritems():
    if key.startswith('CCC_'):
        env.Append(ENV={key: value})

# Placeholder so we can kluge together something like VPATH builds.
# $SRCDIR replaces occurrences for $(srcdir) in the autotools build.
env['SRCDIR'] = '.'

# We may need to force slow regression tests to get around race
# conditions in the pty layer, especially on a loaded machine.
if env["slow"]:
    env['REGRESSOPTS'] = "-S"
else:
    env['REGRESSOPTS'] = ""


def announce(msg):
    if not env.GetOption("silent"):
        print msg

# DESTDIR environment variable means user wants to prefix the installation root.
DESTDIR = os.environ.get('DESTDIR', '')


def installdir(dir, add_destdir=True):
    # use os.path.join to handle absolute paths properly.
    wrapped = os.path.join(env['prefix'], env[dir])
    if add_destdir:
        wrapped = os.path.normpath(DESTDIR + os.path.sep + wrapped)
    wrapped.replace("/usr/etc", "/etc")
    wrapped.replace("/usr/lib/systemd", "/lib/systemd")
    return wrapped

# Honor the specified installation prefix in link paths.
if env["sysroot"]:
    env.Prepend(LIBPATH=[env["sysroot"] + installdir('libdir', add_destdir=False)])

# Give deheader a way to set compiler flags
if 'MORECFLAGS' in os.environ:
    env.Append(CFLAGS=Split(os.environ['MORECFLAGS']))

# Don't change CFLAGS if already set by environment.
if 'CFLAGS' not in os.environ:
    if '-Wmissing-prototypes' not in env['CFLAGS']:
        env.Append(CFLAGS=['-Wmissing-prototypes'])

# Don't change CCFLAGS if already set by environment.
if 'CCFLAGS' not in os.environ:
    env.Append(CCFLAGS=['-Wmissing-declarations'])
    # Should we build with profiling?
    if env['profiling']:
        env.Append(CCFLAGS=['-pg'])
        env.Append(LDFLAGS=['-pg'])
    # Should we build with coveraging?
    if env['coveraging']:
        env.Append(CFLAGS=['-coverage'])
        env.Append(LDFLAGS=['-coverage'])
        env.Append(LINKFLAGS=['-coverage'])
    # Should we build with debug symbols?
    if env['debug']:
        env.Append(CCFLAGS=['-g'])
    # Should we build with optimisation?
    if env['debug'] or env['coveraging']:
        env.Append(CCFLAGS=['-O0'])
    else:
        env.Append(CCFLAGS=['-O2'])

# Get a slight speedup by not doing automatic RCS and SCCS fetches.
env.SourceCode('.', None)

## Cross-development

devenv = (("ADDR2LINE", "addr2line"),
          ("AR", "ar"),
          ("AS", "as"),
          ("CXX", "c++"),
          ("CXXFILT", "c++filt"),
          ("CPP", "cpp"),
          ("GXX", "g++"),
          ("CC", "gcc"),
          ("GCCBUG", "gccbug"),
          ("GCOV", "gcov"),
          ("GPROF", "gprof"),
          ("LD", "ld"),
          ("NM", "nm"),
          ("OBJCOPY", "objcopy"),
          ("OBJDUMP", "objdump"),
          ("RANLIB", "ranlib"),
          ("READELF", "readelf"),
          ("SIZE", "size"),
          ("STRINGS", "strings"),
          ("STRIP", "strip"))

if env['target']:
    for (name, toolname) in devenv:
        env[name] = env['target'] + '-' + toolname

if env['sysroot']:
    env.MergeFlags({"CFLAGS": ["--sysroot=%s" % env['sysroot']]})
    env.MergeFlags({"LINKFLAGS": ["--sysroot=%s" % env['sysroot']]})

## Build help

Help("""Arguments may be a mixture of switches and targets in any order.
Switches apply to the entire build regardless of where they are in the order.
Important switches include:

    prefix=/usr     probably what you want for production tools

Options are cached in a file named .scons-option-cache and persist to later
invocations.  The file is editable.  Delete it to start fresh.  Current option
values can be listed with 'scons -h'.

""" + opts.GenerateHelpText(env, sort=cmp))

## Configuration


def CheckPKG(context, name):
    context.Message('Checking for %s... ' % name)
    ret = context.TryAction('%s --exists \'%s\'' % (env['PKG_CONFIG'], name))[0]
    context.Result(ret)
    return ret

# Stylesheet URLs for making HTML and man pages from DocBook XML.
docbook_url_stem = 'http://docbook.sourceforge.net/release/xsl/current/'
docbook_man_uri = docbook_url_stem + 'manpages/docbook.xsl'
docbook_html_uri = docbook_url_stem + 'html/docbook.xsl'


def CheckXsltproc(context):
    context.Message('Checking that xsltproc can make man pages... ')
    ofp = open("xmltest.xml", "w")
    ofp.write('''
       <refentry id="foo.1">
      <refmeta>
        <refentrytitle>foo</refentrytitle>
        <manvolnum>1</manvolnum>
        <refmiscinfo class='date'>9 Aug 2004</refmiscinfo>
      </refmeta>
      <refnamediv id='name'>
        <refname>foo</refname>
        <refpurpose>check man page generation from docbook source</refpurpose>
      </refnamediv>
    </refentry>
''')
    ofp.close()
    probe = "xsltproc --nonet --noout '%s' xmltest.xml" % (docbook_man_uri,)
    ret = context.TryAction(probe)[0]
    os.remove("xmltest.xml")
    if os.path.exists("foo.1"):
        os.remove("foo.1")
    context.Result(ret)
    return ret


def CheckCompilerOption(context, option):
    context.Message('Checking if compiler accepts %s ...' % (option,))
    old_CFLAGS = context.env['CFLAGS']
    context.env.Append(CFLAGS=option)
    ret = context.TryLink("""
        int main(int argc, char **argv) {
            return 0;
        }
    """, '.c')
    if not ret:
        context.env.Replace(CFLAGS=old_CFLAGS)
    context.Result(ret)
    return ret


def CheckHeaderDefines(context, file, define):
    context.Message('Checking if %s supplies %s ...' % (file, define))
    ret = context.TryLink("""
        #include <%s>
        #ifndef %s
        #error %s is not defined
        #endif
        int main(int argc, char **argv) {
            return 0;
        }
    """ % (file, define, define), '.c')
    context.Result(ret)
    return ret


def CheckCompilerDefines(context, define):
    context.Message('Checking if compiler supplies %s ...' % (define,))
    ret = context.TryLink("""
        #ifndef %s
        #error %s is not defined
        #endif
        int main(int argc, char **argv) {
            return 0;
        }
    """ % (define, define), '.c')
    context.Result(ret)
    return ret

# Check if this compiler is C11 or better


def CheckC11(context):
    context.Message('Checking if compiler is C11 ...')
    ret = context.TryLink("""
        #if (__STDC_VERSION__ < 201112L)
        #error Not C11
        #endif
        int main(int argc, char **argv) {
            return 0;
        }
    """, '.c')
    context.Result(ret)
    return ret


def GetLoadPath(context):
    context.Message("Getting system load path ...")

if env.GetOption("clean") or env.GetOption("help"):
    dbusflags = []
    rtlibs = []
    usbflags = []
    bluezflags = []
    ncurseslibs = []
    confdefs = []
    manbuilder = False
    htmlbuilder = False
    qt_env = None
else:
    config = Configure(env, custom_tests={'CheckPKG': CheckPKG,
                                             'CheckXsltproc': CheckXsltproc,
                                             'CheckCompilerOption': CheckCompilerOption,
                                             'CheckCompilerDefines': CheckCompilerDefines,
                                             'CheckC11': CheckC11,
                                             'CheckHeaderDefines': CheckHeaderDefines})

    # If supported by the compiler, enable all warnings except uninitialized and
    # missing-field-initializers, which we can't help triggering because
    # of the way some of the JSON-parsing code is generated.
    # Also not including -Wcast-qual and -Wimplicit-function-declaration,
    # because we can't seem to keep scons from passing these to g++.
    for option in ('-Wextra', '-Wall', '-Wno-uninitialized', '-Wno-missing-field-initializers',
                   '-Wcast-align', '-Wmissing-declarations', '-Wmissing-prototypes',
                   '-Wstrict-prototypes', '-Wpointer-arith', '-Wreturn-type'):
        if option not in config.env['CFLAGS']:
            config.CheckCompilerOption(option)

    # OS X aliases gcc to clang
    # clang accepts -pthread, then warns it is unused.
    if config.CheckCompilerOption("-pthread") and not sys.platform.startswith('darwin'):
        env.MergeFlags("-pthread")

    env.Prepend(LIBPATH=[os.path.realpath(os.curdir)])
    confdefs = ["/* gpsd_config.h.  Generated by scons, do not hand-hack.  */\n"]

    confdefs.append('#ifndef GPSD_CONFIG_H\n')

    confdefs.append('#define VERSION "%s"\n' % gpsd_version)

    confdefs.append('#define GPSD_URL "%s"\n' % website)

    cxx = config.CheckCXX()
    if not cxx and env["libgpsmm"]:
        announce("C++ doesn't work, suppressing libgpsmm build.")
        env["libgpsmm"] = False

    # define a helper function for pkg-config - we need to pass
    # --static for static linking, too.
    if env["shared"]:
        pkg_config = lambda pkg: ['!%s --cflags --libs %s' % (env['PKG_CONFIG'], pkg, )]
    else:
        pkg_config = lambda pkg: ['!%s --cflags --libs --static %s' % (env['PKG_CONFIG'], pkg, )]

    # The actual distinction here is whether the platform has ncurses in the
    # base system or not. If it does, pkg-config is not likely to tell us
    # anything useful. FreeBSD does, Linux doesn't. Most likely other BSDs
    # are like FreeBSD.
    ncurseslibs = []
    if env['ncurses']:
        if config.CheckPKG('ncurses'):
            ncurseslibs = pkg_config('ncurses')
            if config.CheckPKG('tinfo'):
                ncurseslibs += pkg_config('tinfo')
        elif WhereIs('ncurses5-config'):
            ncurseslibs = ['!ncurses5-config --libs --cflags']
        elif WhereIs('ncursesw5-config'):
            ncurseslibs = ['!ncursesw5-config --libs --cflags']
        elif sys.platform.startswith('freebsd'):
            ncurseslibs = ['-lncurses']
        elif sys.platform.startswith('openbsd'):
            ncurseslibs = ['-lcurses']
        elif sys.platform.startswith('darwin'):
            ncurseslibs = ['-lcurses']

    if env['usb']:
        # In FreeBSD except version 7, USB libraries are in the base system
        if config.CheckPKG('libusb-1.0'):
            confdefs.append("#define HAVE_LIBUSB 1\n")
            try:
                usbflags = pkg_config('libusb-1.0')
            except OSError:
                announce("pkg_config is confused about the state of libusb-1.0.")
                usbflags = []
        elif sys.platform.startswith("freebsd"):
            confdefs.append("#define HAVE_LIBUSB 1\n")
            usbflags = ["-lusb"]
        else:
            confdefs.append("/* #undef HAVE_LIBUSB */\n")
            usbflags = []
    else:
        confdefs.append("/* #undef HAVE_LIBUSB */\n")
        usbflags = []
        env["usb"] = False

    if config.CheckLib('librt'):
        confdefs.append("#define HAVE_LIBRT 1\n")
        # System library - no special flags
        rtlibs = ["-lrt"]
    else:
        confdefs.append("/* #undef HAVE_LIBRT */\n")
        rtlibs = []

    if env['dbus_export'] and config.CheckPKG('dbus-1'):
        confdefs.append("#define HAVE_DBUS 1\n")
        dbusflags = ["-ldbus-1"]
        env.MergeFlags(pkg_config("dbus-1"))
    else:
        confdefs.append("/* #undef HAVE_DBUS */\n")
        dbusflags = []
        if env["dbus_export"]:
            announce("Turning off dbus-export support, library not found.")
        env["dbus_export"] = False

    if env['bluez'] and config.CheckPKG('bluez'):
        confdefs.append("#define ENABLE_BLUEZ 1\n")
        bluezflags = pkg_config('bluez')
    else:
        confdefs.append("/* #undef ENABLE_BLUEZ */\n")
        bluezflags = []
        if env["bluez"]:
            announce("Turning off Bluetooth support, library not found.")
        env["bluez"] = False

    # in_port_t is not defined on Android
    if not config.CheckType("in_port_t", "#include <netinet/in.h>"):
        announce("Did not find in_port_t typedef, assuming unsigned short int")
        confdefs.append("typedef unsigned short int in_port_t;\n")

    # SUN_LEN is not defined on Android
    if not config.CheckDeclaration("SUN_LEN", "#include <sys/un.h>") and not config.CheckDeclaration("SUN_LEN", "#include <linux/un.h>"):
        announce("SUN_LEN is not system-defined, using local definition")
        confdefs.append("#ifndef SUN_LEN\n")
        confdefs.append("#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) + strlen((ptr)->sun_path))\n")
        confdefs.append("#endif /* SUN_LEN */\n")

    if config.CheckHeader(["bits/sockaddr.h", "linux/can.h"]):
        confdefs.append("#define HAVE_LINUX_CAN_H 1\n")
        announce("You have kernel CANbus available.")
    else:
        confdefs.append("/* #undef HAVE_LINUX_CAN_H */\n")
        announce("You do not have kernel CANbus available.")
        env["nmea2000"] = False

    # check for C11 or better, and __STDC__NO_ATOMICS__ is not defined
    # before looking for stdatomic.h
    if config.CheckC11() and not config.CheckCompilerDefines("__STDC_NO_ATOMICS__") and config.CheckHeader("stdatomic.h"):
        confdefs.append("#define HAVE_STDATOMIC_H 1\n")
    else:
        confdefs.append("/* #undef HAVE_STDATOMIC_H */\n")
        if config.CheckHeader("libkern/OSAtomic.h"):
            confdefs.append("#define HAVE_OSATOMIC_H 1\n")
        else:
            confdefs.append("/* #undef HAVE_OSATOMIC_H */\n")
            announce("No memory barriers - SHM export and time hinting may not be reliable.")

    # endian.h is required for rtcm104v2 unless the compiler defines
    # __ORDER_BIG_ENDIAN__, __ORDER_LITTLE_ENDIAN__ and __BYTE_ORDER__
    if config.CheckCompilerDefines("__ORDER_BIG_ENDIAN__") \
    and config.CheckCompilerDefines("__ORDER_LITTLE_ENDIAN__") \
    and config.CheckCompilerDefines("__BYTE_ORDER__"):
        confdefs.append("#define HAVE_BUILTIN_ENDIANNESS 1\n")
        confdefs.append("/* #undef HAVE_ENDIAN_H */\n")
        confdefs.append("/* #undef HAVE_SYS_ENDIAN_H */\n")
        announce("Your compiler has built-in endianness support.")
    else:
        confdefs.append("/* #undef HAVE_BUILTIN_ENDIANNESS\n */")
        if config.CheckHeader("endian.h"):
            confdefs.append("#define HAVE_ENDIAN_H 1\n")
            confdefs.append("/* #undef HAVE_SYS_ENDIAN_H */\n")
            confdefs.append("/* #undef HAVE_MACHINE_ENDIAN_H */\n")
        elif config.CheckHeader("sys/endian.h"):
            confdefs.append("/* #undef HAVE_ENDIAN_H */\n")
            confdefs.append("#define HAVE_SYS_ENDIAN_H 1\n")
            confdefs.append("/* #undef HAVE_MACHINE_ENDIAN_H */\n")
        elif config.CheckHeader("machine/endian.h"):
            confdefs.append("/* #undef HAVE_ENDIAN_H */\n")
            confdefs.append("/* #undef HAVE_SYS_ENDIAN_H */\n")
            confdefs.append("#define HAVE_MACHINE_ENDIAN_H 1\n")
        else:
            confdefs.append("/* #undef HAVE_ENDIAN_H */\n")
            confdefs.append("/* #undef HAVE_SYS_ENDIAN_H */\n")
            confdefs.append("/* #undef HAVE_MACHINE_ENDIAN_H */\n")
            announce("You do not have the endian.h header file. RTCM V2 support disabled.")
            env["rtcm104v2"] = False

    # check function after libraries, because some function require libraries
    # for example clock_gettime() require librt on Linux glibc < 2.17
    for f in ("daemon", "strlcpy", "strlcat", "clock_gettime"):
        if config.CheckFunc(f):
            confdefs.append("#define HAVE_%s 1\n" % f.upper())
        else:
            confdefs.append("/* #undef HAVE_%s */\n" % f.upper())

    # Map options to libraries required to support them that might be absent.
    optionrequires = {
        "bluez": ["libbluetooth"],
        "dbus_export": ["libdbus-1"],
        }

    keys = map(lambda x: (x[0], x[2]), boolopts) + map(lambda x: (x[0], x[2]), nonboolopts) + map(lambda x: (x[0], x[2]), pathopts)
    keys.sort()
    for (key, help) in keys:
        value = env[key]
        if value and key in optionrequires:
            for required in optionrequires[key]:
                if not config.CheckLib(required):
                    announce("%s not found, %s cannot be enabled." % (required, key))
                    value = False
                    break

        confdefs.append("/* %s */" % help)
        if type(value) == type(True):
            if value:
                confdefs.append("#define %s_ENABLE 1\n" % key.upper())
            else:
                confdefs.append("/* #undef %s_ENABLE */\n" % key.upper())
        elif value in (0, "", "(undefined)"):
            confdefs.append("/* #undef %s */\n" % key.upper())
        else:
            if value.isdigit():
                confdefs.append("#define %s %s\n" % (key.upper(), value))
            else:
                confdefs.append("#define %s \"%s\"\n" % (key.upper(), value))

    if config.CheckHeader(["sys/types.h", "sys/time.h", "sys/timepps.h"]):
        env.MergeFlags("-DHAVE_SYS_TIMEPPS_H=1")
        kpps = True
    else:
        kpps = False
    tiocmiwait = config.CheckHeaderDefines("sys/ioctl.h", "TIOCMIWAIT")
    if env["pps"] and not tiocmiwait and not kpps:
        announce("Forcing pps=no (neither TIOCMIWAIT nor RFC2783 API is available)")
        env["pps"] = False

    confdefs.append('''\
/* Some libcs do not have strlcat/strlcpy. Local copies are provided */
#ifndef HAVE_STRLCAT
# ifdef __cplusplus
extern "C" {
# endif
#include <string.h>
size_t strlcat(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
# ifdef __cplusplus
}
# endif
#endif
#ifndef HAVE_STRLCPY
# ifdef __cplusplus
extern "C" {
# endif
#include <string.h>
size_t strlcpy(/*@out@*/char *dst, /*@in@*/const char *src, size_t size);
# ifdef __cplusplus
}
# endif
#endif
#ifndef HAVE_CLOCK_GETTIME
# ifdef __cplusplus
extern "C" {
# endif
#ifndef CLOCKID_T_DEFINED
typedef int clockid_t;
#define CLOCKID_T_DEFINED
# endif
/*
 * OS X 10.5 and later use _STRUCT_TIMESPEC (like other OSes)
 * 10.4 uses _TIMESPEC
 * 10.3 and earlier use _TIMESPEC_DECLARED
 */
#if !defined(_STRUCT_TIMESPEC) \
    && !defined(_TIMESPEC) && !defined(_TIMESPEC_DECLARED)
#define _STRUCT_TIMESPEC
struct timespec {
    time_t  tv_sec;
    long    tv_nsec;
};
#endif
/* OS X does not have clock_gettime */
#define CLOCK_REALTIME 0
int clock_gettime(clockid_t, struct timespec *);
# ifdef __cplusplus
}
# endif
#endif

#define GPSD_CONFIG_H
#endif /* GPSD_CONFIG_H */
''')

    manbuilder = mangenerator = htmlbuilder = None
    if env['manbuild']:
        if config.CheckXsltproc():
            mangenerator = 'xsltproc'
            build = "xsltproc --nonet %s $SOURCE >$TARGET"
            htmlbuilder = build % docbook_html_uri
            manbuilder = build % docbook_man_uri
        elif WhereIs("xmlto"):
            mangenerator = 'xmlto'
            xmlto = "xmlto %s $SOURCE || mv `basename $TARGET` `dirname $TARGET`"
            htmlbuilder = xmlto % "html-nochunks"
            manbuilder = xmlto % "man"
        else:
            announce("Neither xsltproc nor xmlto found, documentation cannot be built.")
    else:
        announce("Build of man and HTML documentation is disabled.")
    if manbuilder:
        env['BUILDERS']["Man"] = Builder(action=manbuilder)
        env['BUILDERS']["HTML"] = Builder(action=htmlbuilder,
                                          src_suffix=".xml", suffix=".html")

    # Determine if Qt network libraries are present, and if not, force qt to off
    if env["qt"]:
        qt_net_name = 'Qt%sNetwork' % env["qt_versioned"]
        qt_network = config.CheckPKG(qt_net_name)
        if not qt_network:
            env["qt"] = False

    env = config.Finish()

    # Be explicit about what we're doing.
    changelatch = False
    for (name, default, help) in boolopts + nonboolopts + pathopts:
        if env[name] != env.subst(default):
            if not changelatch:
                announce("Altered configuration variables:")
                changelatch = True
            announce("%s = %s (default %s): %s" % (name, env[name], env.subst(default), help))
    if not changelatch:
        announce("All configuration flags are defaulted.")

    # Gentoo systems can have a problem with the Python path
    if os.path.exists("/etc/gentoo-release"):
        announce("This is a Gentoo system.")
        announce("Adjust your PYTHONPATH to see library directories under /usr/local/lib")

    # Should we build the Qt binding?
    if env["qt"] and env["shared"]:
        qt_env = env.Clone()
        qt_env.MergeFlags('-DUSE_QT')
        qt_env.Append(OBJPREFIX='qt-')
        try:
            qt_env.MergeFlags(pkg_config(qt_net_name))
        except OSError:
            announce("pkg_config is confused about the state of %s."
                     % qt_net_name)
            qt_env = None
    else:
        qt_env = None

## Two shared libraries provide most of the code for the C programs

libgps_version_soname = libgps_version_current - libgps_version_age
libgps_version = "%d.%d.%d" % (libgps_version_soname, libgps_version_age, libgps_version_revision)

libgps_sources = [
    "ais_json.c",
    "bits.c",
    "clock_gettime.c",
    "daemon.c",
    "gpsutils.c",
    "gpsdclient.c",
    "gps_maskdump.c",
    "hex.c",
    "json.c",
    "libgps_core.c",
    "libgps_dbus.c",
    "libgps_json.c",
    "libgps_shm.c",
    "libgps_sock.c",
    "netlib.c",
    "ntpshmread.c",
    "ntpshmwrite.c",
    "rtcm2_json.c",
    "rtcm3_json.c",
    "shared_json.c",
    "strl.c",
]

if env['libgpsmm']:
    libgps_sources.append("libgpsmm.cpp")

libgpsd_sources = [
    "bsd_base64.c",
    "crc24q.c",
    "gpsd_json.c",
    "geoid.c",
    "isgps.c",
    "libgpsd_core.c",
    "matrix.c",
    "net_dgpsip.c",
    "net_gnss_dispatch.c",
    "net_ntrip.c",
    "ppsthread.c",
    "packet.c",
    "pseudonmea.c",
    "pseudoais.c",
    "serial.c",
    "subframe.c",
    "timebase.c",
    "timespec_str.c",
    "drivers.c",
    "driver_ais.c",
    "driver_evermore.c",
    "driver_garmin.c",
    "driver_garmin_txt.c",
    "driver_geostar.c",
    "driver_italk.c",
    "driver_navcom.c",
    "driver_nmea0183.c",
    "driver_nmea2000.c",
    "driver_oncore.c",
    "driver_rtcm2.c",
    "driver_rtcm3.c",
    "driver_sirf.c",
    "driver_superstar2.c",
    "driver_tsip.c",
    "driver_ubx.c",
    "driver_zodiac.c",
]

if not env["shared"]:
    def Library(env, target, sources, version, parse_flags=[]):
        return env.StaticLibrary(target,
                                 [env.StaticObject(s) for s in sources],
                                 parse_flags=parse_flags)
    LibraryInstall = lambda env, libdir, sources, version: env.Install(libdir, sources)
else:
    def Library(env, target, sources, version, parse_flags=[]):
        # Note: We have a possibility of getting either Object or file
        # list for sources, so we run through the sources and try to make
        # them into SharedObject instances.
        obj_list = []
        for s in Flatten(sources):
            if type(s) is str:
                obj_list.append(env.SharedObject(s))
            else:
                obj_list.append(s)
        return env.SharedLibrary(target=target,
                                 source=obj_list,
                                 parse_flags=parse_flags,
                                 SHLIBVERSION=version)
    LibraryInstall = lambda env, libdir, sources, version: \
                     env.InstallVersionedLib(libdir, sources,
                                             SHLIBVERSION=version)

compiled_gpslib = Library(env=env,
                          target="gps",
                          sources=libgps_sources,
                          version=libgps_version,
                          parse_flags=rtlibs)
env.Clean(compiled_gpslib, "gps_maskdump.c")

static_gpslib = env.StaticLibrary("gps_static",
                                  [env.StaticObject(s) for s in libgps_sources],
                                  rtlibs)

compiled_gpsdlib = env.StaticLibrary(target="gpsd",
                           source=[env.StaticObject(s, parse_flags=usbflags + bluezflags) for s in libgpsd_sources],
                           parse_flags=usbflags + bluezflags)

libraries = [compiled_gpslib, compiled_gpsdlib]

# Only attempt to create the qt library if we have shared turned on otherwise we have a mismash of objects in library
if qt_env:
    qtobjects = []
    qt_flags = qt_env['CFLAGS']
    for c_only in ('-Wmissing-prototypes', '-Wstrict-prototypes'):
        if c_only in qt_flags:
            qt_flags.remove(c_only)
    # Qt binding object files have to be renamed as they're built to avoid
    # name clashes with the plain non-Qt object files. This prevents the
    # infamous "Two environments with different actions were specified
    # for the same target" error.
    for src in libgps_sources:
        if src not in ('ais_json.c', 'json.c', 'libgps_json.c', 'rtcm2_json.c', 'rtcm3_json.c', 'shared_json.c'):
            compile_with = qt_env['CXX']
            compile_flags = qt_flags
        else:
            compile_with = qt_env['CC']
            compile_flags = qt_env['CFLAGS']
        qtobjects.append(qt_env.SharedObject(src,
                                             CC=compile_with,
                                             CFLAGS=compile_flags))
    compiled_qgpsmmlib = Library(qt_env, "Qgpsmm", qtobjects, libgps_version)
    libraries.append(compiled_qgpsmmlib)

# The libraries have dependencies on system libraries
# libdbus appears multiple times because the linker only does one pass.

gpsflags = ["-lm"] + rtlibs + dbusflags
gpsdflags = usbflags + bluezflags + gpsflags

# Source groups

gpsd_sources = ['gpsd.c', 'timehint.c', 'shmexport.c', 'dbusexport.c']

if env['systemd']:
    gpsd_sources.append("sd_socket.c")

gpsmon_sources = [
    'gpsmon.c',
    'monitor_italk.c',
    'monitor_nmea0183.c',
    'monitor_oncore.c',
    'monitor_sirf.c',
    'monitor_superstar2.c',
    'monitor_tnt.c',
    'monitor_ubx.c',
    'monitor_garmin.c',
    ]

## Production programs

gpsd = env.Program('gpsd', gpsd_sources,
                   LIBS=['gpsd', 'gps_static'],
                   parse_flags=gpsdflags+gpsflags)
gpsdecode = env.Program('gpsdecode', ['gpsdecode.c'],
                        LIBS=['gpsd', 'gps_static'],
                        parse_flags=gpsdflags+gpsflags)
gpsctl = env.Program('gpsctl', ['gpsctl.c'],
                     LIBS=['gpsd', 'gps_static'],
                     parse_flags=gpsdflags+gpsflags)
gpsmon = env.Program('gpsmon', gpsmon_sources,
                     LIBS=['gpsd', 'gps_static'],
                     parse_flags=gpsdflags + gpsflags + ncurseslibs)
gpsdctl = env.Program('gpsdctl', ['gpsdctl.c'],
                      LIBS=['gps_static'],
                      parse_flags=gpsflags)
gpspipe = env.Program('gpspipe', ['gpspipe.c'],
                      LIBS=['gps_static'],
                      parse_flags=gpsflags)
gps2udp = env.Program('gps2udp', ['gps2udp.c'],
                      LIBS=['gps_static'],
                      parse_flags=gpsflags)
gpxlogger = env.Program('gpxlogger', ['gpxlogger.c'],
                        LIBS=['gps_static'],
                        parse_flags=gpsflags)
lcdgps = env.Program('lcdgps', ['lcdgps.c'],
                     LIBS=['gps_static'],
                     parse_flags=gpsflags)
cgps = env.Program('cgps', ['cgps.c'],
                   LIBS=['gps_static'],
                   parse_flags=gpsflags + ncurseslibs)
ntpshmmon = env.Program('ntpshmmon', ['ntpshmmon.c'],
                        LIBS=['gps_static'],
                        parse_flags=gpsflags)

binaries = [gpsd, gpsdecode, gpsctl, gpsdctl, gpspipe, gps2udp, gpxlogger, lcdgps, ntpshmmon]
if env["ncurses"]:
    binaries += [cgps, gpsmon]

# Test programs - always link locally and statically
test_bits = env.Program('test_bits', ['test_bits.c'], LIBS=['gps_static'])
test_float = env.Program('test_float', ['test_float.c'])
test_geoid = env.Program('test_geoid', ['test_geoid.c'],
                         LIBS=['gpsd', 'gps_static'],
                         parse_flags=gpsdflags)
test_matrix = env.Program('test_matrix', ['test_matrix.c'],
                          LIBS=['gpsd', 'gps_static'],
                          parse_flags=gpsdflags)
test_mktime = env.Program('test_mktime', ['test_mktime.c'],
                          LIBS=['gps_static'], parse_flags=["-lm"])
test_packet = env.Program('test_packet', ['test_packet.c'],
                          LIBS=['gpsd', 'gps_static'],
                          parse_flags=gpsdflags)
test_timespec = env.Program('test_timespec', ['test_timespec.c'],
                          LIBS=['gpsd', 'gps_static'],
                          parse_flags=gpsdflags)
test_trig = env.Program('test_trig', ['test_trig.c'], parse_flags=["-lm"])
# test_libgps for glibc older than 2.17
test_libgps = env.Program('test_libgps', ['test_libgps.c'],
                          LIBS=['gps_static'],
                          parse_flags=["-lm"] + rtlibs + dbusflags)

if not env['socket_export']:
    announce("test_json not building because socket_export is disabled")
    test_json = None
else:
    test_json = env.Program(
        'test_json', ['test_json.c'],
        LIBS=['gps_static'],
        parse_flags=["-lm"] + rtlibs + usbflags + dbusflags)

test_gpsmm = env.Program('test_gpsmm', ['test_gpsmm.cpp'],
                         LIBS=['gps_static'], parse_flags=["-lm"])
testprogs = [test_bits, test_float, test_geoid, test_libgps, test_matrix, test_mktime, test_packet, test_timespec, test_trig]
if env['socket_export']:
    testprogs.append(test_json)
if env["libgpsmm"]:
    testprogs.append(test_gpsmm)

# Python programs
if not env['python']:
    python_built_extensions = []
    python_targets = []
    python_progs = []
else:
    python_progs = ["gpscat", "gpsfake", "gpsprof", "gegps"]
    if env['xgps']:
        python_progs.extend(["xgps", "xgpsspeed"])
    python_modules = Glob('gps/*.py')

    # Build Python binding
    #
    python_extensions = {
        "gps" + os.sep + "packet": ["gpspacket.c", "packet.c", "isgps.c",
                                        "driver_rtcm2.c", "strl.c", "hex.c", "crc24q.c"],
        "gps" + os.sep + "clienthelpers": ["gpsclient.c", "geoid.c", "gpsdclient.c", "strl.c"]
    }

    python_env = env.Clone()
    vars = sysconfig.get_config_vars('CC', 'CXX', 'OPT', 'BASECFLAGS', 'CCSHARED', 'LDSHARED', 'SO', 'INCLUDEPY', 'LDFLAGS')
    for i in range(len(vars)):
        if vars[i] is None:
            vars[i] = []
    (cc, cxx, opt, basecflags, ccshared, ldshared, so_ext, includepy, ldflags) = vars
    # FIXME: build of python wrappers doesn't pickup flags set for coveraging, manually add them here
    if env['coveraging']:
        basecflags += ' -coverage'
        ldflags += ' -coverage'
        ldshared += ' -coverage'
    # in case CC/CXX was set to the scan-build wrapper,
    # ensure that we build the python modules with scan-build, too
    if env['CC'] is None or env['CC'].find('scan-build') < 0:
        python_env['CC'] = cc
        # As we seem to be changing compilers we must assume that the
        # CCFLAGS are incompatible with the new compiler. If we should
        # use other flags, the variable or the variable for this
        # should be predefined.
        if cc.split()[0] != env['CC']:
            python_env['CCFLAGS'] = ''
    else:
        python_env['CC'] = ' '.join([env['CC']] + cc.split()[1:])
    if env['CXX'] is None or env['CXX'].find('scan-build') < 0:
        python_env['CXX'] = cxx
        # As we seem to be changing compilers we must assume that the
        # CCFLAGS or CXXFLAGS are incompatible with the new
        # compiler. If we should use other flags, the variable or the
        # variable for this should be predefined.
        if cxx.split()[0] != env['CXX']:
            python_env['CCFLAGS'] = ''
            python_env['CXXFLAGS'] = ''
    else:
        python_env['CXX'] = ' '.join([env['CXX']] + cxx.split()[1:])

    ldshared = ldshared.replace('-fPIE', '')
    ldshared = ldshared.replace('-pie', '')
    python_env.Replace(SHLINKFLAGS=[],
                       LDFLAGS=ldflags,
                       LINK=ldshared,
                       SHLIBPREFIX="",
                       SHLIBSUFFIX=so_ext,
                       CPPPATH=[includepy],
                       CPPFLAGS=opt,
                       CFLAGS=basecflags,
                       CXXFLAGS=basecflags)

    python_objects = {}
    python_compiled_libs = {}
    for ext, sources in python_extensions.iteritems():
        python_objects[ext] = []
        for src in sources:
            python_objects[ext].append(
                python_env.NoCache(
                    python_env.SharedObject(
                        src.split(".")[0] + '-py_' + '_'.join(['%s' % (x) for x in sys.version_info]) + so_ext, src
                    )
                )
            )
        python_compiled_libs[ext] = python_env.SharedLibrary(ext, python_objects[ext])
    python_egg_info_source = """Metadata-Version: 1.0
Name: gps
Version: %s
Summary: Python libraries for the gpsd service daemon
Home-page: %s
Author: the GPSD project
Author-email: %s
License: BSD
Description: The gpsd service daemon can monitor one or more GPS devices connected to a host computer, making all data on the location and movements of the sensors available to be queried on TCP port 2947.
Platform: UNKNOWN
""" % (gpsd_version, website, devmail)
    python_egg_info = python_env.Textfile(target="gps-%s.egg-info" % (gpsd_version, ), source=python_egg_info_source)
    python_built_extensions = python_compiled_libs.values()
    python_targets = python_built_extensions + [python_egg_info]

env.Command(target="packet_names.h", source="packet_states.h", action="""
    rm -f $TARGET &&\
    sed -e '/^ *\([A-Z][A-Z0-9_]*\),/s//   \"\\1\",/' <$SOURCE >$TARGET &&\
    chmod a-w $TARGET""")

# timebase.h - always built in order to include current GPS week


def timebase_h(target, source, env):
    from leapsecond import make_leapsecond_include
    f = open(target[0].abspath, 'w')
    f.write(make_leapsecond_include(source[0].abspath))
    f.close()
timebase = env.Command(target="timebase.h",
                       source=["leapseconds.cache"], action=timebase_h)
env.AlwaysBuild(timebase)

env.Textfile(target="gpsd_config.h", source=confdefs)
env.Textfile(target="gpsd.h", source=[File("gpsd.h-head"), File("gpsd_config.h"), File("gpsd.h-tail")])

env.Command(target="gps_maskdump.c", source=["maskaudit.py", "gps.h", "gpsd.h"], action='''
    rm -f $TARGET &&\
        $PYTHON $SOURCE -c $SRCDIR >$TARGET &&\
        chmod a-w $TARGET''')

env.Command(target="ais_json.i", source="jsongen.py", action='''\
    rm -f $TARGET &&\
    $PYTHON $SOURCE --ais --target=parser >$TARGET &&\
    chmod a-w $TARGET''')

# generate revision.h
if 'dev' in gpsd_version:
    (st, rev) = _getstatusoutput('git describe --tags')
    if st != 0:
        from datetime import datetime
        rev = datetime.now().isoformat()[:-4]
else:
    rev = gpsd_version
revision = '#define REVISION "%s"\n' % (rev.strip(),)
env.Textfile(target="revision.h", source=[revision])

generated_sources = ['packet_names.h', 'timebase.h', 'gpsd.h', "ais_json.i",
                     'gps_maskdump.c', 'revision.h', 'gpsd.php',
                     'gpsd_config.h']

# leapseconds.cache is a local cache for information on leapseconds issued
# by the U.S. Naval observatory. It gets kept in the repository so we can
# build without Internet access.
from leapsecond import conditional_leapsecond_fetch


def leapseconds_cache_rebuild(target, source, env):
    if not env["leapfetch"]:
        sys.stdout.write("Leapsecond fetch suppressed by leapfetch=no.\n")
    elif not conditional_leapsecond_fetch(target[0].abspath, timeout=15):
        sys.stdout.write("try building with leapfetch=no.\n")
if 'dev' in gpsd_version or not os.path.exists('leapseconds.cache'):
    leapseconds_cache = env.Command(target="leapseconds.cache",
                                source="leapsecond.py",
                                action=leapseconds_cache_rebuild)
    env.Clean(leapseconds_cache, "leapsecond.pyc")
    env.NoClean(leapseconds_cache)
    env.Precious(leapseconds_cache)
    env.AlwaysBuild(leapseconds_cache)

if env['systemd']:
    udevcommand = 'TAG+="systemd", ENV{SYSTEMD_WANTS}="gpsdctl@%k.service"'
else:
    udevcommand = 'RUN+="%s/gpsd.hotplug"' % (env['udevdir'], )


# Instantiate some file templates.  We'd like to use the Substfile builtin
# but it doesn't seem to work in scons 1.20
def substituter(target, source, env):
    substmap = (
        ('@VERSION@',    gpsd_version),
        ('@prefix@',     env['prefix']),
        ('@libdir@',     env['libdir']),
        ('@udevcommand@',    udevcommand),
        ('@PYTHON@',     sys.executable),
        ('@DATE@',       time.asctime()),
        ('@MASTER@',     'DO NOT HAND_HACK! THIS FILE IS GENERATED'),
        ('@SITENAME@',   sitename),
        ('@SITESEARCH@', sitesearch),
        ('@WEBSITE@',    website),
        ('@MAINPAGE@',   mainpage),
        ('@WEBUPLOAD@',  webupload),
        ('@CGIUPLOAD@',  cgiupload),
        ('@SCPUPLOAD@',  scpupload),
        ('@MAILMAN@',    mailman),
        ('@ADMIN@',      admin),
        ('@DOWNLOAD@',   download),
        ('@BUGTRACKER@', bugtracker),
        ('@BROWSEREPO@', browserepo),
        ('@CLONEREPO@',  clonerepo),
        ('@GITREPO@',    gitrepo),
        ('@WEBFORM@',    webform),
        ('@FORMSERVER@', formserver),
        ('@USERMAIL@',   usermail),
        ('@DEVMAIL@',    devmail),
        ('@ANNOUNCE@',   annmail),
        ('@IRCCHAN@',    ircchan),
        ('@LIBGPSVERSION@', libgps_version),
        ('@TIPLINK@',    tiplink),
        ('@TIPWIDGET@',  tipwidget),
        )
    sfp = open(str(source[0]))
    content = sfp.read()
    sfp.close()
    for (s, t) in substmap:
        content = content.replace(s, t)
    m = re.search("@[A-Z]+@", content)
    if m and m.group(0) not in map(lambda x: x[0], substmap):
        print >>sys.stderr, "Unknown subst token %s in %s." % (m.group(0), sfp.name)
    tfp = open(str(target[0]), "w")
    tfp.write(content)
    tfp.close()

templated = glob.glob("*.in") + glob.glob("*/*.in") + glob.glob("*/*/*.in")

# ignore files in subfolder called 'debian' - the Debian packaging
# tools will handle them.
templated = [x for x in templated if not x.startswith('debian/')]


for fn in templated:
    builder = env.Command(source=fn, target=fn[:-3], action=substituter)
    env.AddPostAction(builder, 'chmod -w $TARGET')
    if fn.endswith(".py.in"):
        env.AddPostAction(builder, 'chmod +x $TARGET')

# Documentation

base_manpages = {
    "gpsd.8": "gpsd.xml",
    "gpsd_json.5": "gpsd_json.xml",
    "gps.1": "gps.xml",
    "cgps.1": "gps.xml",
    "gpsinit.8": "gpsinit.xml",
    "lcdgps.1": "gps.xml",
    "libgps.3": "libgps.xml",
    "libgpsmm.3": "libgpsmm.xml",
    "libQgpsmm.3": "libgpsmm.xml",
    "gpsmon.1": "gpsmon.xml",
    "gpsctl.1": "gpsctl.xml",
    "gpsdctl.8": "gpsdctl.xml",
    "gpspipe.1": "gpspipe.xml",
    "gps2udp.1": "gps2udp.xml",
    "gpsdecode.1": "gpsdecode.xml",
    "srec.5": "srec.xml",
    "ntpshmmon.1": "ntpshmmon.xml",
    }
python_manpages = {
    "gpsprof.1": "gpsprof.xml",
    "gpsfake.1": "gpsfake.xml",
    "gpscat.1": "gpscat.xml",
    "gegps.1": "gps.xml",
    }
if env['xgps']:
    python_manpages.update({
        "xgpsspeed.1": "gps.xml",
        "xgps.1": "gps.xml",
    })

manpage_targets = []
if manbuilder:
    for (man, xml) in base_manpages.items() + python_manpages.items():
        manpage_targets.append(env.Man(source=xml, target=man))

## Where it all comes together

build = env.Alias('build',
                  [libraries, binaries, python_targets,
                   "gpsd.php", manpage_targets,
                   "libgps.pc", "libgpsd.pc", "gpsd.rules"])
env.Default(*build)

if qt_env:
    build_qt = qt_env.Alias('build', [compiled_qgpsmmlib])
    qt_env.Default(*build_qt)

if env['python']:
    build_python = python_env.Alias('build', python_targets)
    python_env.Default(*build_python)

## Installation and deinstallation

# Not here because too distro-specific: udev rules, desktop files, init scripts

# It's deliberate that we don't install gpsd.h. It's full of internals that
# third-party client programs should not see.
headerinstall = [env.Install(installdir('includedir'), x) for x in ("libgpsmm.h", "gps.h")]

binaryinstall = []
binaryinstall.append(env.Install(installdir('sbindir'), [gpsd, gpsdctl]))
binaryinstall.append(env.Install(installdir('bindir'), [gpsdecode, gpsctl, gpspipe, gps2udp,
                                                         gpxlogger, lcdgps, ntpshmmon]))
if env["ncurses"]:
    binaryinstall.append(env.Install(installdir('bindir'), [cgps, gpsmon]))
binaryinstall.append(LibraryInstall(env, installdir('libdir'), compiled_gpslib, libgps_version))
# Work arount a minor bug in InstallSharedLib() link handling
env.AddPreAction(binaryinstall, 'rm -f %s/libgps.*' % (installdir('libdir'), ))

if qt_env:
    binaryinstall.append(LibraryInstall(qt_env, installdir('libdir'), compiled_qgpsmmlib, libgps_version))

if not env['debug'] and not env['profiling'] and not env['nostrip'] and not sys.platform.startswith('darwin'):
    env.AddPostAction(binaryinstall, '$STRIP $TARGET')

if not env['python']:
    python_install = []
else:
    python_lib_dir = env['python_libdir']
    python_module_dir = python_lib_dir + os.sep + 'gps'
    python_extensions_install = python_env.Install(DESTDIR + python_module_dir,
                                                    python_built_extensions)
    if not env['debug'] and not env['profiling'] and not env['nostrip'] and not sys.platform.startswith('darwin'):
        python_env.AddPostAction(python_extensions_install, '$STRIP $TARGET')

    python_modules_install = python_env.Install(DESTDIR + python_module_dir,
                                                python_modules)

    python_progs_install = python_env.Install(installdir('bindir'), python_progs)

    python_egg_info_install = python_env.Install(DESTDIR + python_lib_dir,
                                                 python_egg_info)
    python_install = [python_extensions_install,
                        python_modules_install,
                        python_progs_install,
                        python_egg_info_install]

pc_install = [env.Install(installdir('pkgconfig'), x) for x in ("libgps.pc", "libgpsd.pc")]
if qt_env:
    pc_install.append(qt_env.Install(installdir('pkgconfig'), 'Qgpsmm.pc'))
    pc_install.append(qt_env.Install(installdir('libdir'), 'libQgpsmm.prl'))


maninstall = []
for manpage in base_manpages.keys() + python_manpages.keys():
    if not manbuilder and not os.path.exists(manpage):
        continue
    section = manpage.split(".")[1]
    dest = os.path.join(installdir('mandir'), "man"+section, manpage)
    maninstall.append(env.InstallAs(source=manpage, target=dest))
install = env.Alias('install', binaryinstall + maninstall + python_install + pc_install + headerinstall)


def Uninstall(nodes):
    deletes = []
    for node in nodes:
        if node.__class__ == install[0].__class__:
            deletes.append(Uninstall(node.sources))
        else:
            deletes.append(Delete(str(node)))
    return deletes
uninstall = env.Command('uninstall', '', Flatten(Uninstall(Alias("install"))) or "")
env.AlwaysBuild(uninstall)
env.Precious(uninstall)

# Target selection for '.' is badly broken. This is a general scons problem,
# not a glitch in this particular recipe. Avoid triggering the bug.


def error_action(target, source, env):
    from SCons.Errors import UserError
    raise UserError, "Target selection for '.' is broken."
AlwaysBuild(Alias(".", [], error_action))

# Utility productions


def Utility(target, source, action):
    target = env.Command(target=target, source=source, action=action)
    env.AlwaysBuild(target)
    env.Precious(target)
    return target

# Putting in all these -U flags speeds up cppcheck and allows it to look
# at configurations we actually care about.
Utility("cppcheck", ["gpsd.h", "packet_names.h"],
        "cppcheck -U__UNUSED__ -UUSE_QT -U__COVERITY__ -U__future__ -ULIMITED_MAX_CLIENTS -ULIMITED_MAX_DEVICES -UAF_UNSPEC -UINADDR_ANY -UFIXED_PORT_SPEED -UFIXED_STOP_BITS -U_WIN32 -U__CYGWIN__ -UPATH_MAX -UHAVE_STRLCAT -UHAVE_STRLCPY -UIPTOS_LOWDELAY -UIPV6_TCLASS -UTCP_NODELAY -UTIOCMIWAIT --template gcc --enable=all --inline-suppr --suppress='*:driver_proto.c' --force $SRCDIR")

# Check with clang analyzer
Utility("scan-build", ["gpsd.h", "packet_names.h"],
        "scan-build scons")

# Sanity-check Python code.
if len(python_progs) > 0:
    pylint = Utility("pylint", ["jsongen.py", "maskaudit.py", python_built_extensions],
        ['''pylint --rcfile=/dev/null --dummy-variables-rgx='^_' --msg-template="{path}:{line}: [{msg_id}({symbol}), {obj}] {msg}" --reports=n --disable=F0001,C0103,C0111,C1001,C0301,C0302,C0322,C0324,C0323,C0321,C0330,R0201,R0801,R0902,R0903,R0904,R0911,R0912,R0913,R0914,R0915,W0110,W0201,W0121,W0123,W0232,W0234,W0401,W0403,W0141,W0142,W0603,W0614,W0640,W0621,W1504,E1101,E1102,E1103,F0401 gps/*.py *.py ''' + " ".join(python_progs)])

# Additional Python readablity style checks
if len(python_progs) > 0:
    pep8 = Utility("pep8", ["jsongen.py", "maskaudit.py", python_built_extensions],
        ['''pep8 --ignore=E501,W602,E122,E241,E401 {0} gps/[a-zA-Z]*.py *.py'''.format(" ".join(python_progs))])

# Additional Python readablity style checks
if len(python_progs) > 0:
    flake8 = Utility("flake8", ["jsongen.py", "maskaudit.py", python_built_extensions],
        ['''flake8 --ignore=E501,W602,E122,E241,E401 {0} gps/[a-zA-Z]*.py *.py'''.format(" ".join(python_progs))])


# Check the documentation for bogons, too
Utility("xmllint", glob.glob("*.xml"),
    "for xml in $SOURCES; do xmllint --nonet --noout --valid $$xml; done")

# Use deheader to remove headers not required.  If the statistics line
# ends with other than '0 removed' there's work to be done.
Utility("deheader", generated_sources, [
    'deheader -x cpp -x contrib -x gpspacket.c -x gpsclient.c -x monitor_proto.c -i gpsd_config.h -i gpsd.h -m "MORECFLAGS=\'-Werror -Wfatal-errors -DDEBUG -DPPS_ENABLE\' scons -Q"',
        ])

# Perform all local code-sanity checks (but not the Coverity scan).
audit = env.Alias('audit',
                  ['scan-build',
                   'cppcheck',
                   'pylint',
                   'xmllint',
                   'valgrind-audit',
                   ])

#
# Regression tests begin here
#
# Note that the *-makeregress targets re-create the *.log.chk source
# files from the *.log source files.

# Unit-test the bitfield extractor
bits_regress = Utility('bits-regress', [test_bits], [
    '$SRCDIR/test_bits --quiet'
    ])

# Unit-test the bitfield extractor
matrix_regress = Utility('matrix-regress', [test_matrix], [
    '$SRCDIR/test_matrix --quiet'
    ])

# Check that all Python modules compile properly
if env['python']:
    def check_compile(target, source, env):
        for pyfile in source:
            'cp %s tmp.py' % (pyfile)
            '%s -tt -m py_compile tmp.py' % (sys.executable, )
            'rm -f tmp.py tmp.pyc'
    python_compilation_regress = Utility('python-compilation-regress',
            Glob('*.py') + python_modules + python_progs + ['SConstruct'], check_compile)
else:
    python_compilation_regress = None

# using regress-drivers requires socket_export being enabled.
if not env['socket_export'] or not env['python']:
    announce("GPS regression tests suppressed because socket_export or python is off.")
    gps_regress = None
else:
    # Regression-test the daemon. But first:
    # (1) Clear GPSD's SHM segment in case a previous abort didn't.  This
    # prevents spurious error messages.
    # (2) Dump the platform and its delay parameters.
    # The ":;" in this production and the next one forestalls an attempt by
    # SCons to install up to date versions of gpsfake and gpsctl if it can
    # find older versions of them in a directory on your $PATH.
    gps_regress = Utility("gps-regress", [gpsd, gpsctl, python_built_extensions],
            ':; $SRCDIR/gpsfake -T; $SRCDIR/regress-driver $REGRESSOPTS test/daemon/*.log')

    # Build the regression tests for the daemon.
    # Note: You'll have to do this whenever the default leap second
    # changes in timebase.h.  The problem is in the SiRF tests;
    # that driver relies on the default until it gets the current
    # offset from subframe data.
    Utility('gps-makeregress', [gpsd, gpsctl, python_built_extensions],
        ':; $SRCDIR/gpsfake -T; $SRCDIR/regress-driver -b $REGRESSOPTS test/daemon/*.log')

# To build an individual test for a load named foo.log, put it in
# test/daemon and do this:
#    regress-driver -b test/daemon/foo.log

# Regression-test the RTCM decoder.
if not env["rtcm104v2"]:
    announce("RTCM2 regression tests suppressed because rtcm104v2 is off.")
    rtcm_regress = None
else:
    rtcm_regress = Utility('rtcm-regress', [gpsdecode], [
        '@echo "Testing RTCM decoding..."',
        '@for f in $SRCDIR/test/*.rtcm2; do '
            'echo "\tTesting $${f}..."; '
            'TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
            '$SRCDIR/gpsdecode -u -j <$${f} >$${TMPFILE}; '
            'diff -ub $${f}.chk $${TMPFILE}; '
            'rm -f $${TMPFILE}; '
        'done;',
        '@echo "Testing idempotency of JSON dump/decode for RTCM2"',
        '@TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
        '$SRCDIR/gpsdecode -u -e -j <test/synthetic-rtcm2.json >$${TMPFILE}; '
            'grep -v "^#" test/synthetic-rtcm2.json | diff -ub - $${TMPFILE}; '
            'rm -f $${TMPFILE}; ',
        ])

# Rebuild the RTCM regression tests.
Utility('rtcm-makeregress', [gpsdecode], [
    'for f in $SRCDIR/test/*.rtcm2; do '
        '$SRCDIR/gpsdecode -j <$${f} >$${f}.chk; '
    'done'
        ])

# Regression-test the AIVDM decoder.
if not env["aivdm"]:
    announce("AIVDM regression tests suppressed because aivdm is off.")
    aivdm_regress = None
else:
    aivdm_regress = Utility('aivdm-regress', [gpsdecode], [
        '@echo "Testing AIVDM decoding w/ CSV format..."',
        '@for f in $SRCDIR/test/*.aivdm; do '
            'echo "\tTesting $${f}..."; '
            'TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
            '$SRCDIR/gpsdecode -u -c <$${f} >$${TMPFILE}; '
            'diff -ub $${f}.chk $${TMPFILE} || echo "Test FAILED!"; '
            'rm -f $${TMPFILE}; '
        'done;',
        '@echo "Testing AIVDM decoding w/ JSON unscaled format..."',
        '@for f in $SRCDIR/test/*.aivdm; do '
            'echo "\tTesting $${f}..."; '
            'TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
            '$SRCDIR/gpsdecode -u -j <$${f} >$${TMPFILE}; '
            'diff -ub $${f}.ju.chk $${TMPFILE} || echo "Test FAILED!"; '
            'rm -f $${TMPFILE}; '
        'done;',
        '@echo "Testing AIVDM decoding w/ JSON scaled format..."',
        '@for f in $SRCDIR/test/*.aivdm; do '
            'echo "\tTesting $${f}..."; '
            'TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
            '$SRCDIR/gpsdecode -j <$${f} >$${TMPFILE}; '
            'diff -ub $${f}.js.chk $${TMPFILE} || echo "Test FAILED!"; '
            'rm -f $${TMPFILE}; '
        'done;',
        '@echo "Testing idempotency of unscaled JSON dump/decode for AIS"',
        '@TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
        '$SRCDIR/gpsdecode -u -e -j <$SRCDIR/test/sample.aivdm.ju.chk >$${TMPFILE}; '
            'grep -v "^#" $SRCDIR/test/sample.aivdm.ju.chk | diff -ub - $${TMPFILE}; '
            'rm -f $${TMPFILE}; ',
        # Parse the unscaled json reference, dump it as scaled json,
        # and finally compare it with the scaled json reference
        '@echo "Testing idempotency of scaled JSON dump/decode for AIS"',
        '@TMPFILE=`mktemp -t gpsd-test-XXXXXXXXXXXXXX.chk`; '
        '$SRCDIR/gpsdecode -e -j <$SRCDIR/test/sample.aivdm.ju.chk >$${TMPFILE}; '
            'grep -v "^#" $SRCDIR/test/sample.aivdm.js.chk | diff -ub - $${TMPFILE}; '
            'rm -f $${TMPFILE}; ',
        ])

# Rebuild the AIVDM regression tests.
Utility('aivdm-makeregress', [gpsdecode], [
    'for f in $SRCDIR/test/*.aivdm; do '
        '$SRCDIR/gpsdecode -u -c <$${f} > $${f}.chk; '
        '$SRCDIR/gpsdecode -u -j <$${f} > $${f}.ju.chk; '
        '$SRCDIR/gpsdecode -j  <$${f} > $${f}.js.chk; '
    'done',
        ])

# Regression-test the packet getter.
packet_regress = Utility('packet-regress', [test_packet], [
    '@echo "Testing detection of invalid packets..."',
    '$SRCDIR/test_packet | diff -u $SRCDIR/test/packet.test.chk -',
    ])

# Rebuild the packet-getter regression test
Utility('packet-makeregress', [test_packet], [
    '$SRCDIR/test_packet >$SRCDIR/test/packet.test.chk',
    ])

# Rebuild the geoid test
Utility('geoid-makeregress', [test_geoid], [
    '$SRCDIR/test_geoid 37.371192 122.014965 >$SRCDIR/test/geoid.test.chk'])

# Regression-test the geoid tester.
geoid_regress = Utility('geoid-regress', [test_geoid], [
    '@echo "Testing the geoid model..."',
    '$SRCDIR/test_geoid 37.371192 122.014965 | diff -u $SRCDIR/test/geoid.test.chk -',
    ])

# Regression-test the Maidenhead Locator
if not env['python']:
    maidenhead_locator_regress = None
else:
    maidenhead_locator_regress = Utility('maidenhead-locator-regress', [python_built_extensions], [
        '@echo "Testing the Maidenhead Locator conversion..."',
        '$SRCDIR/test_maidenhead.py >/dev/null',
        ])

# Regression-test the calendar functions
time_regress = Utility('time-regress', [test_mktime], [
    '$SRCDIR/test_mktime'
    ])

# Regression test the unpacking code in libgps
if not env['python']:
    unpack_regress = None
else:
    unpack_regress = Utility('unpack-regress', [test_libgps], [
        '@echo "Testing the client-library sentence decoder..."',
        '$SRCDIR/regress-driver -c $SRCDIR/test/clientlib/*.log',
        ])

# Build the regression test for the sentence unpacker
Utility('unpack-makeregress', [test_libgps], [
    '@echo "Rebuilding the client sentence-unpacker tests..."',
    '$SRCDIR/regress-driver -c -b $SRCDIR/test/clientlib/*.log'
    ])

# Unit-test the JSON parsing
if not env['socket_export']:
    json_regress = None
else:
    json_regress = Utility('json-regress', [test_json], [
        '$SRCDIR/test_json'
        ])

# Unit-test timespec math
timespec_regress = Utility('timespec-regress', [test_timespec], [
    '$SRCDIR/test_timespec'
    ])

# consistency-check the driver methods
method_regress = Utility('packet-regress', [test_packet], [
    '@echo "Consistency-checking driver methods..."',
    '$SRCDIR/test_packet -c >/dev/null',
    ])

# Run a valgrind audit on the daemon  - not in normal tests
valgrind_audit = Utility('valgrind-audit',
    ['$SRCDIR/valgrind-audit.py', python_built_extensions, gpsd],
    './valgrind-audit.py'
    )

# Run test builds on remote machines
flocktest = Utility("flocktest", [], "cd devtools; ./flocktest " + gitrepo)


# Run all normal regression tests
describe = Utility('describe', [],
                   ['@echo "Run normal regression tests for %s..."' % (rev.strip(),)])
testclean = Utility('testclean', [],
                    'rm -f test_bits test_geoid test_json test_libgps test_matrix test_mktime test_packet')
check = env.Alias('check', [
    describe,
    python_compilation_regress,
    method_regress,
    bits_regress,
    matrix_regress,
    gps_regress,
    rtcm_regress,
    aivdm_regress,
    packet_regress,
    geoid_regress,
    maidenhead_locator_regress,
    time_regress,
    unpack_regress,
    json_regress,
    testclean,
    test_timespec,
    timespec_regress,
    ])

env.Alias('testregress', check)

# Remove all shared-memory segments.  Normally only needs to be run
# when a segment size changes.
Utility('shmclean', [], ["ipcrm  -M 0x4e545030;"
                         "ipcrm  -M 0x4e545031;"
                         "ipcrm  -M 0x4e545032;"
                         "ipcrm  -M 0x4e545033;"
                         "ipcrm  -M 0x4e545034;"
                         "ipcrm  -M 0x4e545035;"
                         "ipcrm  -M 0x4e545036;"
                         "ipcrm  -M 0x47505345;"
                         ])

# The website directory
#
# None of these productions are fired by default.
# The content they handle is the GPSD website, not included in release tarballs.

# asciidoc documents
if env.WhereIs('asciidoc'):
    txtfiles = ['AIVDM', 'NMEA',
                'protocol-evolution',
                'protocol-transition',
                'gpsd-time-service-howto',
                'time-service-intro',
                'client-howto']
    asciidocs = ["www/" + stem + ".html" for stem in txtfiles] \
                + ["www/installation.html"]
    for stem in txtfiles:
        env.Command('www/%s.html' % stem, 'www/%s.txt' % stem,
                    ['asciidoc -b html5 -a toc -o www/%s.html www/%s.txt' % (stem, stem)])
    env.Command("www/installation.html",
                "INSTALL",
                ["asciidoc -o www/installation.html INSTALL"])
else:
    announce("Part of the website build requires asciidoc, not installed.")
    asciidocs = []

htmlpages = Split('''
    www/gps2udp.html
    www/gpscat.html
    www/gpsctl.html
    www/gpsdecode.html
    www/gpsd.html
    www/gpsd_json.html
    www/gpsfake.html
    www/gps.html
    www/gpsmon.html
    www/gpspipe.html
    www/gpsprof.html
    www/hardware.html
    www/installation.html
    www/internals.html
    www/libgps.html
    www/libgpsmm.html
    www/ntpshmmon.html
    www/performance/performance.html
    www/replacing-nmea.html
    www/srec.html
    www/writing-a-driver.html
    ''')

webpages = htmlpages + asciidocs + map(lambda f: f[:-3], glob.glob("www/*.in"))

www = env.Alias('www', webpages)

# Paste 'scons --quiet validation-list' to a batch validator such as
# http://htmlhelp.com/tools/validator/batch.html.en


def validation_list(target, source, env):
    for page in glob.glob("www/*.html"):
        if '-head' not in page:
            fp = open(page)
            if "Valid HTML" in fp.read():
                print os.path.join(website, os.path.basename(page))
            fp.close()
Utility("validation-list", [www], validation_list)

# How to update the website
upload_web = Utility("website", [www],
                     ['rsync --exclude="*.in" -avz www/ ' + webupload,
                      'scp README TODO NEWS ' + webupload,
                      'chmod ug+w,a+x www/gps_report.cgi',
                      'scp www/gps_report.cgi ' + cgiupload + "gps_report.cgi"])

# When the URL declarations change, so must the generated web pages
for fn in glob.glob("www/*.in"):
    env.Depends(fn[:-3], "SConstruct")

if htmlbuilder:
    # Manual pages
    for xml in glob.glob("*.xml"):
        env.HTML('www/%s.html' % xml[:-4], xml)

    # DocBook documents
    for stem in ['writing-a-driver', 'performance/performance', 'replacing-nmea']:
        env.HTML('www/%s.html' % stem, 'www/%s.xml' % stem)

    # The internals manual.
    # Doesn't capture dependencies on the subpages
    env.HTML('www/internals.html', '$SRCDIR/doc/internals.xml')

# The hardware page
env.Command('www/hardware.html', ['gpscap.py',
                                  'www/hardware-head.html',
                                  'gpscap.ini',
                                  'www/hardware-tail.html'],
            ['(cat www/hardware-head.html; $PYTHON gpscap.py; cat www/hardware-tail.html) >www/hardware.html'])

# The diagram editor dia is required in order to edit the diagram masters
# Utility("www/cycle.svg", ["www/cycle.dia"], ["dia -e www/cycle.svg www/cycle.dia"])

# Experimenting with pydoc.  Not yet fired by any other productions.
# scons www/ dies with this

## if env['python']:
##     env.Alias('pydoc', "www/pydoc/index.html")
##
##     # We need to run epydoc with the Python version we built the modules for.
##     # So we define our own epydoc instead of using /usr/bin/epydoc
##     EPYDOC = "python -c 'from epydoc.cli import cli; cli()'"
##     env.Command('www/pydoc/index.html', python_progs + glob.glob("*.py")  + glob.glob("gps/*.py"), [
##         'mkdir -p www/pydoc',
##         EPYDOC + " -v --html --graph all -n GPSD $SOURCES -o www/pydoc",
##             ])

# Productions for setting up and performing udev tests.
#
# Requires root. Do "udev-install", then "tail -f /var/log/syslog" in
# another window, then run 'scons udev-test', then plug and unplug the
# GPS ad libitum.  All is well when you get fix reports each time a GPS
# is plugged in.
#
# In case you are a systemd user you might also need to watch the
# journalctl output. Instead of the hotplug script the gpsdctl@.service
# unit will handle hotplugging together with the udev rules.
#
# Note that a udev event can be triggered with an invocation like:
# udevadm trigger --sysname-match=ttyUSB0 --action add

if env['systemd']:
    systemdinstall_target = [env.Install(DESTDIR + systemd_dir, "systemd/%s" % (x,)) for x in ("gpsdctl@.service", "gpsd.service", "gpsd.socket")]
    systemd_install = env.Alias('systemd_install', systemdinstall_target)
    systemd_uninstall = env.Command('systemd_uninstall', '', Flatten(Uninstall(Alias("systemd_install"))) or "")

    env.AlwaysBuild(systemd_uninstall)
    env.Precious(systemd_uninstall)


if env['systemd']:
    hotplug_wrapper_install = []
else:
    hotplug_wrapper_install = [
        'cp $SRCDIR/gpsd.hotplug ' + DESTDIR + env['udevdir'],
        'chmod a+x ' + DESTDIR + env['udevdir'] + '/gpsd.hotplug'
    ]

udev_install = Utility('udev-install', 'install', [
    'mkdir -p ' + DESTDIR + env['udevdir'] + '/rules.d',
    'cp $SRCDIR/gpsd.rules ' + DESTDIR + env['udevdir'] + '/rules.d/25-gpsd.rules',
    ] + hotplug_wrapper_install)

if env['systemd']:
    systemctl_daemon_reload = Utility('systemctl-daemon-reload', '', ['systemctl daemon-reload || true'])
    env.AlwaysBuild(systemctl_daemon_reload)
    env.Precious(systemctl_daemon_reload)
    env.Requires(udev_install, systemd_install)
    env.Requires(systemctl_daemon_reload, systemd_install)
    env.Requires(udev_install, systemctl_daemon_reload)


Utility('udev-uninstall', '', [
    'rm -f %s/gpsd.hotplug' % env['udevdir'],
    'rm -f %s/rules.d/25-gpsd.rules' % env['udevdir'],
        ])

Utility('udev-test', '', [
    '$SRCDIR/gpsd -N -n -F /var/run/gpsd.sock -D 5',
        ])

# Cleanup

# Ordinary cleanup
clean = env.Clean(
    build,
    (
        map(
            glob.glob,
            (
                "*.[oa]", "*.[1358]", "*.os", "*.os.*",
                "*.gcno", "*.pyc", "gps/*.pyc", "TAGS",
                "config.log", "lib*.so*", "lib*.a",
                "gps-*egg-info", "contrib/ppscheck", "*.gcda"
            )
        ) +
        testprogs +
        generated_sources +
        base_manpages.keys() +
        webpages +
        map(lambda f: f[:-3], templated)
    )
)

# Nuke scons state files
sconsclean = Utility("sconsclean", '', ["rm -fr .sconf_temp .scons-option-cache config.log"])

# Tags for Emacs and vi
misc_sources = ['cgps.c', 'gpsctl.c', 'gpsdctl.c', 'gpspipe.c',
                'gps2udp.c', 'gpsdecode.c', 'gpxlogger.c', 'ntpshmmon.c']
sources = libgpsd_sources + libgps_sources \
          + gpsd_sources + gpsmon_sources + misc_sources
env.Command('TAGS', sources, ['etags ' + " ".join(sources)])

# Release machinery begins here
#
# We need to be in the actual project repo (i.e. not doing a -Y build)
# for these productions to work.

if os.path.exists("gpsd.c") and os.path.exists(".gitignore"):
    distfiles = _getoutput(r"git ls-files | grep -v '^www/'").split()
    if ".gitignore" in distfiles:
        distfiles.remove(".gitignore")
    distfiles += generated_sources
    distfiles += base_manpages.keys() + python_manpages.keys()
    distfiles.remove("gpsd.h")
    if "packaging/rpm/gpsd.spec" not in distfiles:
        distfiles.append("packaging/rpm/gpsd.spec")

    # How to build a zip file.
    zip = env.Command('zip', distfiles, [
        '@zip -r gpsd-${VERSION}.zip $SOURCES',
        '@ls -l gpsd-${VERSION}.zip',
        ])
    env.Clean(zip, ["gpsd-${VERSION}.zip", "packaging/rpm/gpsd.spec"])

    # How to build a tarball.
    tarball = env.Command('tarball', distfiles, [
        '@tar --transform "s:^:gpsd-${VERSION}/:" -czf gpsd-${VERSION}.tar.gz $SOURCES',
        '@ls -l gpsd-${VERSION}.tar.gz',
        ])
    env.Clean(tarball, ["gpsd-${VERSION}.tar.gz", "packaging/rpm/gpsd.spec"])

    # Make RPM from the specfile in packaging
    Utility('dist-rpm', tarball, 'rpmbuild -ta gpsd-${VERSION}.tar.gz')

    # Make sure build-from-tarball works.
    testbuild = Utility('testbuild', [tarball], [
        'tar -xzvf gpsd-${VERSION}.tar.gz',
        'cd gpsd-${VERSION}; scons',
        'rm -fr gpsd-${VERSION}',
        ])

    releasecheck = env.Alias('releasecheck', [
        testbuild,
        check,
        audit,
        flocktest,
        ])

    # This is how to ship a release to the hosting site.
    # The chmod copes with the fact that scp will give a
    # replacement the permissions of the *original*...
    upload_release = Utility('upload-release', [tarball], [
            'gpg -b gpsd-${VERSION}.tar.gz',
            'chmod ug=rw,o=r gpsd-${VERSION}.tar.gz gpsd-${VERSION}.tar.gz.sig',
            'scp gpsd-${VERSION}.tar.gz gpsd-${VERSION}.tar.gz.sig ' + scpupload,
            ])

    # How to tag a release
    tag_release = Utility('tag-release', [], [
        'git tag -s -m "Tagged for external release ${VERSION}" release-${VERSION}'
        ])
    upload_tags = Utility('upload-tags', [], ['git push --tags'])

    # Local release preparation. This production will require Internet access,
    # but it doesn't do any uploads or public repo mods.
    #
    # Note that tag_release has to fire early, otherwise the value of REVISION
    # won't be right when revision.h is generated for the tarball.
    releaseprep = env.Alias("releaseprep",
                            [Utility("distclean", [], ["rm -f revision.h"]),
                             tag_release,
                             tarball])
    # Undo local release preparation
    Utility("undoprep", [], ['rm -f gpsd-${VERSION}.tar.gz;',
                             'git tag -d release-${VERSION};'])

    # All a buildup to this.
    env.Alias("release", [releaseprep,
                          upload_release,
                          upload_tags,
                          upload_web])

    # Experimental release mechanics using shipper
    # This will ship a freecode metadata update
    Utility("ship", [tarball, "control"], ['shipper version=%s | sh -e -x' % gpsd_version])

# The following sets edit modes for GNU EMACS
# Local Variables:
# mode:python
# End:
