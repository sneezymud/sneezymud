# hey emacs, this is -*- python -*-
from SCons.Script.SConscript import SConsEnvironment
import os.path;

CXXFLAGS = [
    '-g',             # debugger
    '-DLINUX',        # compiling on Linux
    '-DFAST',         # compiling on a fast processor machine
    '-O2',            # optimize code
    '-Wall',                  # all rec. warnings shown (includes all above)
    '-Wpointer-arith',        # looks for things that depend on sizeof()
    '-Wcast-qual',            # Warns about using casting to remove const
    '-Wcast-align',           # a pointer cast changes alignment
    '-Wwrite-strings',        # looks for formal const char* sent to char*
    '-Wconversion',           # bad conversions
    '-Woverloaded-virtual',   # virtual without a declaration in base class
    '-Werror',                # turns a warning into an error
    ]

CPPPATH = [
    '/usr/local/include',
    '#/code/cmd',
    '#/code/disc',
    '#/code/game',
    '#/code/misc',
    '#/code/obj',
    '#/code/spec',
    '#/code/sys',
    '#/code/task'
    ]

LDFLAGS = [
    '-lstdc++',
    '/usr/lib/libcrypt.a', # enable for insight
    '-g', # enable core debugging
    '-lmysqlclient',
    '-lares',
    '-lboost_regex',
    '-lboost_program_options',
    '-lboost_date_time',
    '-lcurl',
    '-ltcmalloc',
#    '-lprofiler',
    ]

LIBPATH = [
    '/usr/local/lib',
    '/mud/build/google-perftools/lib'
    ]

# build environment
env=Environment(CXXFLAGS=CXXFLAGS,
                LINKFLAGS=LDFLAGS,
                CPPPATH=CPPPATH,
                LIBPATH=LIBPATH,
                tools=['default', 'cxxtest'],

                #prettified output
                SHCXXCOMSTR = "\033[35mCompiling\033[0m $SOURCE",
                CXXCOMSTR = "\033[35mCompiling\033[0m $SOURCE",
                SHLINKCOMSTR = "\033[31mLinking\033[0m $TARGET",
                LINKCOMSTR = "\033[31mLinking\033[0m $TARGET",
                INSTALLSTR = "\033[33mInstalling\033[0m '$SOURCE' as '$TARGET'",
                )

env['ENV']['LD_LIBRARY_PATH']='/mud/build/google-perftools/lib'

Export('env')

# getting some problems with this
CacheDir('/mud/build/cache')

# build the sneezy exe by default, see the help below for other targets
Default('sneezy')

# go into code and start building
env.SConscript('code/SConscript',
               build_dir='objs',
               duplicate=0,
               exports='env')


# etags
def PhonyTargets(env = None, **kw):
    if not env: env = DefaultEnvironment()
    for target,action in kw.items():
        env.AlwaysBuild(env.Alias(target, [], action))

PhonyTargets(env, tags='@etags code/*/*')


# progress indicator
AddOption('--no-progress', dest='no-progress',
          action="store_true", default=False)

if not GetOption('no-progress'):
    Progress(['\033[32mChecking dependencies\033[0m -\r',
              '\033[32mChecking dependencies\033[0m \\\r',
              '\033[32mChecking dependencies\033[0m |\r',
              '\033[32mChecking dependencies\033[0m /\r'], interval=5)

# command line help
Help("""
Targets:
sneezy:         build the main binary (output: sneezy.2).
lowtools:       build the low tools (output: lowtools/).
objs/*/*.os:    build a specific file (eg: objs/game/game_hilo.os).
tags:           rebuild the emacs tag file. 
check:          build and run unit tests
'-c'            to clean objects and executables.
'--no-progress' to mute the "Checking dependencies" output.
""")





        
    
