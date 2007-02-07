## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
# Copyright (c) 2006 INRIA
# All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>



import os
import os.path
import shutil
from SCons.Environment import Environment
from SCons.Builder import Builder
from SCons.Action import Action
import SCons

# hack stolen from wengo
# to get an ARGUMENTS defined correctly
try:
    ARGUMENTS = SCons.Script.ARGUMENTS
    COMMAND_LINE_TARGETS = SCons.Script.COMMAND_LINE_TARGETS
except AttributeError:
    from SCons.Script.SConscript import Arguments
    from SCons.Script.SConscript import CommandLineTargets
    ARGUMENTS = Arguments
    COMMAND_LINE_TARGETS = CommandLineTargets

class Ns3Module:
    def __init__(self, name, dir):
        self.sources = []
        self.inst_headers = []
        self.headers = []
        self.extra_dist = []
        self.deps = []
        self.external_deps = []
        self.config = []
        self.name = name
        self.dir = dir
        self.executable = False
        self.library = True
        self.ldflags = []
        self.header_inst_dir = ''
    def set_library(self):
        self.library = True
        self.executable = False
    def set_executable(self):
        self.library = False
        self.executable = True
    def add_config(self, config_fn):
        self.config.append(config_fn)
    def add_extra_dist(self, dist):
        self.extra_dist.append(dist)
    def add_external_dep(self, dep):
        self.external_deps.append(dep)
    def add_dep(self, dep):
        self.deps.append(dep)
    def add_deps(self, deps):
        self.deps.extend(deps)
    def add_source(self, source):
        self.sources.append(source)
    def add_sources(self, sources):
        self.sources.extend(sources)
    def add_header(self, header):
        self.headers.append(header)
    def add_headers(self, headers):
        self.headers.extend(headers)
    def add_inst_header(self, header):
        self.inst_headers.append(header)
    def add_inst_headers(self, headers):
        self.inst_headers.extend(headers)
    def add_ldflags (self, ldflags):
        self.ldflags.extend (ldflags)
    def add_ldflag (self, ldflag):
        self.add_ldflags ([ldflag])
    def set_header_inst_dir (self, inst_dir):
        self.header_inst_dir = inst_dir
        

def MyCopyAction(target, source, env):
    try:
        if len(target) == len(source):
            for i in range(len(target)):
                shutil.copy(source[i].path, target[i].path)
            return 0
        else:
            return 'invalid target/source match'
    except:
        print
        return 'exception'
def MyCopyActionPrint(target, source, env):
    if len(target) == len(source):
        output = ''
        for i in range(len(target)):
            output = output + 'copy \'' + source[i].path + '\' to \'' + target[i].path + '\''
            if i < len(target) - 1:
                output = output + '\n'
        return output
    else:
        return 'error in copy'
def GcxxEmitter(target, source, env):
    if os.path.exists(source[0].path):
        return [target, source]
    else:
        return [[], []]
def MyRmTree(target, source, env):
    shutil.rmtree(env['RM_DIR'])
    return 0
def MyRmTreePrint(target, source, env):
    return ''
def print_cmd_line(s, target, src, env):
    print 'Building ' + (' and '.join([str(x) for x in target])) + '...'

class Ns3BuildVariant:
    def __init__(self):
        self.static = False
        self.gcxx_deps = False
        self.gcxx_root = ''
        self.build_root = ''
        self.env = None

class Ns3:
    def __init__(self):
        self.__modules = []
        self.extra_dist = []
        self.build_dir = 'build'
        self.version = '0.0.1'
        self.name = 'noname'
        self.doxygen_config = ''
    def add(self, module):
        self.__modules.append(module)
    def add_extra_dist(self, dist):
        self.extra_dist.append(dist)
    def __get_module(self, name):
        for module in self.__modules:
            if module.name == name:
                return module
        return None
    def get_mod_output(self, module, variant):
        if module.executable:
            suffix = variant.env.subst(variant.env['PROGSUFFIX'])
            filename = os.path.join(variant.build_root, 'bin', 
                                    module.name + suffix)
        else:
            if variant.static:
                prefix = variant.env['LIBPREFIX']
                suffix = variant.env['LIBSUFFIX']
            else:
                prefix = variant.env['SHLIBPREFIX']
                suffix = variant.env['SHLIBSUFFIX']
            prefix = variant.env.subst(prefix)
            suffix = variant.env.subst(suffix)
            filename = os.path.join(variant.build_root, 'lib', 
                                     prefix + module.name + suffix)
        return filename
    def get_obj_builders(self, variant, module):
        env = variant.env.Copy ()
        objects = []
        hash = {}
        self.get_internal_deps (module, hash)
        for dep in hash.values ():
            if dep.header_inst_dir != '':
                inc_dir = os.path.join(variant.build_root, 'include', 
                                       self.name, dep.header_inst_dir)
                env.Append (CPPPATH = [inc_dir])
                
        if len(module.config) > 0:
            src_config_file = os.path.join(self.build_dir, 'config', module.name + '-config.h')
            tgt_config_file = os.path.join(variant.build_root, 'include', 
                                           self.name, module.name + '-config.h')

        for source in module.sources:
            obj_file = os.path.splitext(source)[0] + '.o'
            tgt = os.path.join(variant.build_root, module.dir, obj_file)
            src = os.path.join(module.dir, source)
            if variant.static:
                obj_builder = env.StaticObject(target = tgt, source = src)
            else:
                obj_builder = env.SharedObject(target = tgt, source = src)
            if len(module.config) > 0:
                config_file = env.MyCopyBuilder(target = [tgt_config_file], 
                                                    source = [src_config_file])
                env.Depends(obj_builder, config_file)
            if variant.gcxx_deps:
                gcno_tgt = os.path.join(variant.build_root, module.dir, 
                                        os.path.splitext(source)[0] + '.gcno')
                gcda_tgt = os.path.join(variant.build_root, module.dir, 
                                        os.path.splitext(source)[0] + '.gcda')
                gcda_src = os.path.join(variant.gcxx_root, module.dir, 
                                        os.path.splitext(source)[0] + '.gcda')
                gcno_src = os.path.join(variant.gcxx_root, module.dir, 
                                        os.path.splitext(source)[0] + '.gcno')
                gcno_builder = env.CopyGcxxBuilder(target = gcno_tgt, source = gcno_src)
                gcda_builder = env.CopyGcxxBuilder(target = gcda_tgt, source = gcda_src)
                env.Depends(obj_builder, gcda_builder)
                env.Depends(obj_builder, gcno_builder)
            objects.append(obj_builder)
        return objects
    def get_internal_deps(self, module, hash):
        for dep_name in module.deps:
            dep = self.__get_module(dep_name)
            hash[dep_name] = dep
            self.get_internal_deps(dep, hash)
    def get_external_deps(self, module):
        hash = {}
        self.get_internal_deps(module, hash)
        ext_hash = {}
        for mod in hash.values():
            for ext_dep in mod.external_deps:
                ext_hash[ext_dep] = 1
        return ext_hash.keys()
    def get_sorted_deps(self, module):
        h = {}
        self.get_internal_deps(module, h)
        modules = []
        for dep in h.keys():
            deps_copy = []
            mod = h[dep]
            deps_copy.extend(mod.deps)
            modules.append([mod, deps_copy])
        sorted = []
        while len(modules) > 0:
            to_remove = []
            for item in modules:
                if len(item[1]) == 0:
                    to_remove.append(item[0].name)
            for item in to_remove:
                for i in modules:
                    if item in i[1]:
                        i[1].remove(item)
            new_modules = []
            for mod in modules:
                found = False
                for i in to_remove:
                    if i == mod[0].name:
                        found = True
                        break
                if not found:
                    new_modules.append(mod)
            modules = new_modules
            sorted.extend(to_remove)
        sorted.reverse()
        # append external deps
        ext_deps = self.get_external_deps(module)
        for dep in ext_deps:
            sorted.append(dep)
        return sorted

    def gen_mod_dep(self, variant):
        build_root = variant.build_root
        cpp_path = os.path.join(variant.build_root, 'include')
        env = variant.env
        env.Append(CPPPATH = [cpp_path])
        header_dir = os.path.join(build_root, 'include', self.name)
        lib_path = os.path.join(build_root, 'lib')
        env.Append (LIBPATH = [lib_path])
        module_builders = []
        for module in self.__modules:
            my_env = env.Copy ();
            objects = self.get_obj_builders(variant, module)
            libs = self.get_sorted_deps(module)
            my_env.Append (LIBS = libs)
            my_env.Append (LINKFLAGS = module.ldflags)

            filename = self.get_mod_output(module, variant)
            if module.executable:
                module_builder = my_env.Program(target=filename, source=objects)
            else:
                if variant.static:
                    module_builder = my_env.StaticLibrary(target=filename, source=objects)
                else:
                    module_builder = my_env.SharedLibrary(target=filename, source=objects)

            for dep_name in module.deps:
                dep = self.__get_module(dep_name)
                my_env.Depends(module_builder, self.get_mod_output(dep, variant))

            for header in module.inst_headers:
                if module.header_inst_dir != '':
                    tgt = os.path.join(header_dir, module.header_inst_dir, header)
                else:
                    tgt = os.path.join(header_dir, header)
                src = os.path.join(module.dir, header)
                #builder = env.Install(target = tgt, source = src)
                header_builder = my_env.MyCopyBuilder(target=tgt, source=src)
                my_env.Depends(module_builder, header_builder)

            module_builders.append(module_builder)
        return module_builders
    def gen_mod_config(self, env):
        config_dir = os.path.join(self.build_dir, 'config')
        for module in self.__modules:
            if len(module.config) > 0:
                config_file = os.path.join(config_dir, module.name + '-config.h')
                config_file_guard = module.name + '_CONFIG_H'
                config_file_guard.upper()
                if not os.path.isfile(config_file):
                    if not os.path.isdir(config_dir):
                        os.makedirs(config_dir)
                    outfile = open(config_file, 'w')
                    outfile.write('#ifndef ' + config_file_guard + '\n')
                    outfile.write('#define ' + config_file_guard + '\n')
                    config = env.Configure()
                    for fn in module.config:
                        output = fn(env, config)
                        for o in output:
                            outfile.write(o)
                            outfile.write('\n')
                    outfile.write('#endif /*' + config_file_guard + '*/\n')
                    config.Finish()
    def generate_dependencies(self):
        inheritenv = (ARGUMENTS.get('inheritenv', 'n') in 'yY1')
        if inheritenv:
            env = Environment(ENV=os.environ)
        else:
            env = Environment()
        self.gen_mod_config(env)
        cc = env['CC']
        cxx = env.subst(env['CXX'])
        common_flags = ARGUMENTS.get('cflags', '').split(' ')
        cxxflags = ARGUMENTS.get('cxxflags', '').split(' ')
        ldflags = ARGUMENTS.get('ldflags', '').split(' ')
        if cc == 'cl' and cxx == 'cl':
            env = Environment(tools=['mingw'])
            cc = env['CC']
            cxx = env.subst(env['CXX'])
        if cc == 'gcc' and cxx == 'g++':
            common_flags.extend(['-g3', '-Wall', '-Werror'])
            debug_flags = []
            opti_flags = ['-O3']
        elif cc == 'cl' and cxx == 'cl':
            env = Environment(ENV=os.environ)
            debug_flags = ['-W1', '-GX', '-EHsc', '-D_DEBUG', '/MDd']
            opti_flags = ['-O2', '-EHsc', '-DNDEBUG', '/MD']
        env.Append(CCFLAGS = common_flags, 
                    CPPDEFINES = ['RUN_SELF_TESTS'], 
                    TARFLAGS = '-c -z', 
                    CPPFLAGS = cxxflags, 
                    LINKFLAGS = ldflags)
        if env['PLATFORM'] == 'posix':
            env.Append(LINKFLAGS = ' -z origin')
            env.Append(RPATH=env.Literal(os.path.join('\\$$ORIGIN', os.pardir, 'lib')))
        verbose = ARGUMENTS.get('verbose', 'n')
        if verbose == 'n':
            env['PRINT_CMD_LINE_FUNC'] = print_cmd_line
        header_builder = Builder(action = Action(MyCopyAction, strfunction=MyCopyActionPrint))
        env.Append(BUILDERS = {'MyCopyBuilder':header_builder})
        gcxx_builder = Builder(action = Action(MyCopyAction, strfunction=MyCopyActionPrint), 
                                emitter = GcxxEmitter)
        env.Append(BUILDERS = {'CopyGcxxBuilder':gcxx_builder})
        variant = Ns3BuildVariant()
        builders = []


        gcov_env = env.Copy()
        gcov_env.Append(CFLAGS = ['-fprofile-arcs', '-ftest-coverage'], 
                         CXXFLAGS = ['-fprofile-arcs', '-ftest-coverage'], 
                         LINKFLAGS = ['-fprofile-arcs'])
        # code coverage analysis
        variant.static = False
        variant.env = gcov_env
        variant.build_root = os.path.join(self.build_dir, 'gcov')
        builders = self.gen_mod_dep(variant)
        for builder in builders:
            gcov_env.Alias('gcov', builder)
        gcov_env.Alias('lcov-report')
        if 'lcov-report' in COMMAND_LINE_TARGETS:
            lcov_report_dir = os.path.join(self.build_dir, 'lcov-report')
            create_dir_command = "rm -rf " + lcov_report_dir
            create_dir_command += " && mkdir " + lcov_report_dir + ";"
            gcov_env.Execute(create_dir_command)
            info_file = os.path.join(lcov_report_dir, self.name + '.info')
            lcov_command = "utils/lcov/lcov -c -d . -o " + info_file
            lcov_command += " --source-dirs=" + os.getcwd()
            lcov_command += ":" + os.path.join(os.getcwd(), 
                                                variant.build_root, 
                                                'include')
            gcov_env.Execute(lcov_command)
            genhtml_command = "utils/lcov/genhtml -o " + lcov_report_dir
            genhtml_command += " " + info_file
            gcov_env.Execute(genhtml_command)



        opt_env = env.Copy()
        opt_env.Append(CFLAGS=opti_flags, 
                        CXXFLAGS=opti_flags, 
                        CPPDEFINES=['NDEBUG'])
        # optimized static support
        variant.static = True
        variant.env = opt_env
        variant.build_root = os.path.join(self.build_dir, 'opt-static')
        builders = self.gen_mod_dep(variant)
        for builder in builders:
            opt_env.Alias('opt-static', builder)


        opt_env = env.Copy()
        opt_env.Append(CFLAGS = opti_flags, 
                        CXXFLAGS = opti_flags, 
                        CPPDEFINES = ['NDEBUG'])
        # optimized shared support
        variant.static = False
        variant.env = opt_env
        variant.build_root = os.path.join(self.build_dir, 'opt-shared')
        builders = self.gen_mod_dep(variant)
        for builder in builders:
            opt_env.Alias('opt-shared', builder)


        arc_env = env.Copy()
        arc_env.Append(CFLAGS=opti_flags, 
                        CXXFLAGS=opti_flags, 
                        CPPDEFINES=['NDEBUG'])
        arc_env.Append(CFLAGS=['-frandom-seed=0', '-fprofile-generate'], 
                        CXXFLAGS=['-frandom-seed=0', '-fprofile-generate'], 
                        LINKFLAGS=['-frandom-seed=0', '-fprofile-generate'])
        # arc profiling
        variant.static = False
        variant.env = arc_env
        variant.build_root = os.path.join(self.build_dir, 'opt-arc')
        builders = self.gen_mod_dep(variant)
        for builder in builders:
            arc_env.Alias('opt-arc', builder)


        arcrebuild_env = env.Copy()
        arcrebuild_env.Append(CFLAGS=opti_flags, 
                               CXXFLAGS=opti_flags, 
                               CPPDEFINES=['NDEBUG'])
        arcrebuild_env.Append(CFLAGS=['-frandom-seed=0', '-fprofile-use'], 
                               CXXFLAGS=['-frandom-seed=0', '-fprofile-use'], 
                               LINKFLAGS=['-frandom-seed=0', '-fprofile-use'])
        # arc rebuild
        variant.static = False
        variant.env = arcrebuild_env
        variant.gcxx_deps = True
        variant.gcxx_root = os.path.join(self.build_dir, 'opt-arc')
        variant.build_root = os.path.join(self.build_dir, 'opt-arc-rebuild')
        builders = self.gen_mod_dep(variant)
        for builder in builders:
            arcrebuild_env.Alias('opt-arc-rebuild', builder)
        variant.gcxx_deps = False




        dbg_env = env.Copy()
        dbg_env.Append(CFLAGS = debug_flags, 
                       CXXFLAGS = debug_flags,
                       CPPDEFINES = ['NS3_DEBUG_ENABLE'])
        # debug static support
        variant.static = True
        variant.env = dbg_env
        variant.build_root = os.path.join(self.build_dir, 'dbg-static')
        builders = self.gen_mod_dep(variant)
        for builder in builders:
            dbg_env.Alias('dbg-static', builder)

        dbg_env = env.Copy()
        dbg_env.Append(CFLAGS=debug_flags, 
                       CXXFLAGS=debug_flags,
                       CPPDEFINES = ['NS3_DEBUG_ENABLE'])
        # debug shared support
        variant.static = False
        variant.env = dbg_env
        variant.build_root = os.path.join(self.build_dir, 'dbg-shared')
        builders = self.gen_mod_dep(variant)
        for builder in builders:
            dbg_env.Alias('dbg-shared', builder)

        env.Alias('dbg', 'dbg-shared')
        env.Alias('opt', 'opt-shared')
        env.Default('dbg')
        env.Alias('all', ['dbg-shared', 'dbg-static', 'opt-shared', 'opt-static'])


        # dist support
        dist_env = env.Copy()
        if dist_env['PLATFORM'] == 'posix':
            dist_list = []
            for module in self.__modules:
                for f in module.sources:
                    dist_list.append(os.path.join(module.dir, f))
                for f in module.headers:
                    dist_list.append(os.path.join(module.dir, f))
                for f in module.inst_headers:
                    dist_list.append(os.path.join(module.dir, f))
                for f in module.extra_dist:
                    dist_list.append(os.path.join(module.dir, f))
            for f in self.extra_dist:
                dist_list.append(f)
            dist_list.append('SConstruct')
            dist_list.append('build.py')

            targets = []
            basename = self.name + '-' + self.version
            for src in dist_list:
                tgt = os.path.join(basename, src)
                targets.append(dist_env.MyCopyBuilder(target=tgt, source=src))
            tar = basename + '.tar.gz'
            zip = basename + '.zip'
            tmp_tar = os.path.join(self.build_dir, tar)
            tmp_zip = os.path.join(self.build_dir, zip)
            dist_tgt = [tar, zip]
            dist_src = [tmp_tar, tmp_zip]
            dist_env.Tar(tmp_tar, targets)
            dist_env.Zip(tmp_zip, targets)
            dist_builder = dist_env.MyCopyBuilder(target=dist_tgt, source=dist_src)
            dist_env.Alias('dist', dist_builder)
            dist_env.Append(RM_DIR=basename)
            dist_env.AddPostAction(dist_builder, dist_env.Action(MyRmTree, 
                                                                 strfunction=MyRmTreePrint))
            dist_builder = dist_env.MyCopyBuilder(target=dist_tgt, source=dist_src)
            dist_env.Alias('fastdist', dist_tgt)

            # distcheck
            distcheck_list = []
            for src in dist_list:
                tgt = os.path.join(self.build_dir, basename, src)
                distcheck_list.append(tgt)
            untar = env.Command(distcheck_list, tar, 
                                ['cd ' + self.build_dir + ' && tar -zxf ../' + tar])
            scons_dir = os.path.join(self.build_dir, basename)
            distcheck_builder = env.Command('x', distcheck_list, 
                                            ['cd ' + scons_dir + ' && scons'])
            env.AlwaysBuild(distcheck_builder)
            env.Alias('distcheck', distcheck_builder)
        if self.doxygen_config != '':
            doxy = env.Command('doc/html/*', self.doxygen_config, 
                               ['doxygen ' + self.doxygen_config])
            env.AlwaysBuild(doxy)
            env.Alias('doc', doxy)
