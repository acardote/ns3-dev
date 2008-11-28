import os
import os.path
import Params
import Object
import sys
import pproc as subprocess
import shlex
import ccroot

# these are set from the main wscript file
APPNAME=None
VERSION=None
#
# The last part of the path name to use to find the regression traces tarball.
# path will be APPNAME + '-' + VERSION + REGRESSION_SUFFIX + TRACEBALL_SUFFIX,
# e.g., ns-3-dev-ref-traces.tar.bz2
#
TRACEBALL_SUFFIX = ".tar.bz2"



def get_command_template(*arguments):
    if Params.g_options.valgrind:
        if Params.g_options.command_template:
            Params.fatal("Options --command-template and --valgrind are conflicting")
        cmd = "valgrind --leak-check=full %s"
    else:
        cmd = Params.g_options.command_template or '%s'
    for arg in arguments:
        cmd = cmd + " " + arg
    return cmd



def find_program(program_name, env):
    launch_dir = os.path.abspath(Params.g_cwd_launch)
    found_programs = []
    for obj in Object.g_allobjs:
        if not getattr(obj, 'is_ns3_program', False):
            continue

        ## filter out programs not in the subtree starting at the launch dir
        if not (obj.path.abspath().startswith(launch_dir)
                or obj.path.abspath(env).startswith(launch_dir)):
            continue
        
        found_programs.append(obj.target)
        if obj.target == program_name:
            return obj
    raise ValueError("program '%s' not found; available programs are: %r"
                     % (program_name, found_programs))

def get_proc_env(os_env=None):
    env = Params.g_build.env_of_name('default')
    if sys.platform == 'linux2':
        pathvar = 'LD_LIBRARY_PATH'
    elif sys.platform == 'darwin':
        pathvar = 'DYLD_LIBRARY_PATH'
    elif sys.platform == 'win32':
        pathvar = 'PATH'
    elif sys.platform == 'cygwin':
        pathvar = 'PATH'
    elif sys.platform.startswith('freebsd'):
        pathvar = 'LD_LIBRARY_PATH'
    else:
        Params.warning(("Don't know how to configure "
                        "dynamic library path for the platform %r;"
                        " assuming it's LD_LIBRARY_PATH.") % (sys.platform,))
        pathvar = 'LD_LIBRARY_PATH'        

    proc_env = dict(os.environ)
    if os_env is not None:
        proc_env.update(os_env)

    if pathvar is not None:
        if pathvar in proc_env:
            proc_env[pathvar] = os.pathsep.join(list(env['NS3_MODULE_PATH']) + [proc_env[pathvar]])
        else:
            proc_env[pathvar] = os.pathsep.join(list(env['NS3_MODULE_PATH']))

    pymoddir = Params.g_build.m_curdirnode.find_dir('bindings/python').abspath(env)
    if 'PYTHONPATH' in proc_env:
        proc_env['PYTHONPATH'] = os.pathsep.join([pymoddir] + [proc_env['PYTHONPATH']])
    else:
        proc_env['PYTHONPATH'] = pymoddir

    return proc_env

def run_argv(argv, os_env=None):
    proc_env = get_proc_env(os_env)
    #env = Params.g_build.env_of_name('default')
    retval = subprocess.Popen(argv, env=proc_env).wait()
    if retval:
        Params.fatal("Command %s exited with code %i" % (argv, retval))
    return retval

def get_run_program(program_string, command_template=None):
    """
    Return the program name and argv of the process that would be executed by
    run_program(program_string, command_template).
    """
    #print "get_run_program_argv(program_string=%r, command_template=%r)" % (program_string, command_template)
    env = Params.g_build.env_of_name('default')

    if command_template in (None, '%s'):
        argv = shlex.split(program_string)
        program_name = argv[0]

        try:
            program_obj = find_program(program_name, env)
        except ValueError, ex:
            Params.fatal(str(ex))

        try:
            program_node = program_obj.path.find_build(ccroot.get_target_name(program_obj))
        except AttributeError:
            Params.fatal("%s does not appear to be a program" % (program_name,))

        execvec = [program_node.abspath(env)] + argv[1:]

    else:

        program_name = program_string
        try:
            program_obj = find_program(program_name, env)
        except ValueError, ex:
            Params.fatal(str(ex))
        try:
            program_node = program_obj.path.find_build(ccroot.get_target_name(program_obj))
        except AttributeError:
            Params.fatal("%s does not appear to be a program" % (program_name,))

        execvec = shlex.split(command_template % (program_node.abspath(env),))
    return program_name, execvec

def run_program(program_string, command_template=None):
    """
    if command_template is not None, then program_string == program
    name and argv is given by command_template with %s replaced by the
    full path to the program.  Else, program_string is interpreted as
    a shell command with first name being the program name.
    """
    dummy_program_name, execvec = get_run_program(program_string, command_template)
    former_cwd = os.getcwd()
    if (Params.g_options.cwd_launch):
        os.chdir(Params.g_options.cwd_launch)
    else:
        os.chdir(Params.g_cwd_launch)
    try:
        retval = run_argv(execvec)
    finally:
        os.chdir(former_cwd)

    return retval



def run_python_program(program_string):
    env = Params.g_build.env_of_name('default')
    execvec = shlex.split(program_string)

    former_cwd = os.getcwd()
    if (Params.g_options.cwd_launch):
        os.chdir(Params.g_options.cwd_launch)
    else:
        os.chdir(Params.g_cwd_launch)
    try:
        retval = run_argv([env['PYTHON']] + execvec)
    finally:
        os.chdir(former_cwd)

    return retval


