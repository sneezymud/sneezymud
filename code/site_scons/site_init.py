from SCons.Script.SConscript import SConsEnvironment

def get_option(name, default):
    arg = ARGUMENTS.get(name, os.environ.get(name, None))
    return default if arg is None else arg

def get_bool_option(name, default):
    arg = get_option(name, None)
    if arg is None:
        return default
    low = arg.lower()
    if low in ('1', 'y', 'yes', 'on', 'true'):
        return True
    if low in ('0', 'n', 'no', 'off', 'false'):
        return False
    raise ValueError("Cannot convert '%s=%s' to bool" % (name, arg))

def PhonyTargets(env = None, **kw):
    env = env or DefaultEnvironment()
    for target, action in kw.items():
        env.AlwaysBuild(env.Alias(target, [], action))
