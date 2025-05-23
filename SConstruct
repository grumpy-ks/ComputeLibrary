# -*- coding: utf-8 -*-

# Copyright (c) 2016-2025 Arm Limited.
#
# SPDX-License-Identifier: MIT
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import SCons
import json
import os
import subprocess

def version_at_least(version, required):

    version_list = version.split('.')
    required_list = required.split('.')
    end = min(len(version_list), len(required_list))
    for i in range(0, end):
        if int(version_list[i]) < int(required_list[i]):
            return False
        elif int(version_list[i]) > int(required_list[i]):
            return True

    return True

def read_build_config_json(build_config):
    build_config_contents = {}
    custom_types = []
    custom_layouts = []
    if os.path.isfile(build_config):
        with open(build_config) as f:
            try:
                build_config_contents = json.load(f)
            except:
                print("Warning: Build configuration file is of invalid JSON format!")
    else:
        try:
            build_config_contents = json.loads(build_config)
        except:
            print("Warning: Build configuration string is of invalid JSON format!")
    if build_config_contents:
        custom_types = build_config_contents.get("data_types", [])
        custom_layouts = build_config_contents.get("data_layouts", [])
    return custom_types, custom_layouts

def update_data_type_layout_flags(env, data_types, data_layouts):
    # Manage data-types
    if env['multi_isa']:
        if  any(i in data_types for i in ['all', 'fp16']):
            env.Append(CXXFLAGS = ['-DENABLE_FP16_KERNELS', '-DARM_COMPUTE_ENABLE_FP16'])
    else:
            if not 'v8a' in env['arch'] and not 'v7a' in env['arch'] and not 'armv8r64' in env['arch'] and not 'x86' in env['arch']:
                if  any(i in data_types for i in ['all', 'fp16']):
                    env.Append(CXXFLAGS = ['-DENABLE_FP16_KERNELS','-DARM_COMPUTE_ENABLE_FP16'])

    if any(i in data_types for i in ['all', 'fp32']):
        env.Append(CXXFLAGS = ['-DENABLE_FP32_KERNELS'])
    if any(i in data_types for i in ['all', 'qasymm8']):
        env.Append(CXXFLAGS = ['-DENABLE_QASYMM8_KERNELS'])
    if any(i in data_types for i in ['all', 'qasymm8_signed']):
        env.Append(CXXFLAGS = ['-DENABLE_QASYMM8_SIGNED_KERNELS'])
    if any(i in data_types for i in ['all', 'qsymm16']):
        env.Append(CXXFLAGS = ['-DENABLE_QSYMM16_KERNELS'])
    if any(i in data_types for i in ['all', 'integer']):
        env.Append(CXXFLAGS = ['-DENABLE_INTEGER_KERNELS'])
    # Manage data-layouts
    if any(i in data_layouts for i in ['all', 'nhwc']):
        env.Append(CXXFLAGS = ['-DENABLE_NHWC_KERNELS'])
    if any(i in data_layouts for i in ['all', 'nchw']):
        env.Append(CXXFLAGS = ['-DENABLE_NCHW_KERNELS'])

    return env


vars = Variables("scons")
vars.AddVariables(
    BoolVariable("debug", "Debug", False),
    BoolVariable("asserts", "Enable asserts (this flag is forced to 1 for debug=1)", False),
    BoolVariable("logging", "Enable Logging", False),
    EnumVariable("arch", "Target Architecture. The x86_32 and x86_64 targets can only be used with neon=0 and opencl=1.", "armv7a",
                  allowed_values=("armv7a", "armv7a-hf", "arm64-v8a", "arm64-v8.2-a", "arm64-v8.2-a-sve", "arm64-v8.2-a-sve2", "x86_32", "x86_64",
                                  "armv8a", "armv8.2-a", "armv8.2-a-sve", "armv8.6-a", "armv8.6-a-sve", "armv8.6-a-sve2", "armv8.6-a-sve2-sme2", "armv8r64", "x86")),
    EnumVariable("estate", "Execution State", "auto", allowed_values=("auto", "32", "64")),
    EnumVariable("os", "Target OS. With bare metal selected, only Arm® Neon™ (not OpenCL) can be used, static libraries get built and Neon™'s multi-threading support is disabled.", "linux", allowed_values=("linux", "android", "tizen", "macos", "bare_metal", "openbsd","windows", "qnx")),
    EnumVariable("build", "Either build directly on your device (native) or cross compile from your desktop machine (cross-compile). In both cases make sure the compiler is available in your path.", "cross_compile", allowed_values=("native", "cross_compile", "embed_only")),
    BoolVariable("examples", "Build example programs", True),
    BoolVariable("gemm_tuner", "Build gemm_tuner programs", True),
    BoolVariable("Werror", "Enable/disable the -Werror compilation flag", True),
    BoolVariable("multi_isa", "Build Multi ISA binary version of library.", False),
    BoolVariable("standalone", "Builds the tests as standalone executables, links statically with libgcc, libstdc++ and libarm_compute", False),
    BoolVariable("opencl", "Enable OpenCL support", True),
    BoolVariable("neon", "Enable Arm® Neon™ support", False),
    BoolVariable("embed_kernels", "Enable if you want the OpenCL kernels to be built in the library's binaries instead of being read from separate '.cl' / '.cs' files. If embed_kernels is set to 0 then the application can set the path to the folder containing the OpenCL kernel files by calling CLKernelLibrary::init(). By default the path is set to './cl_kernels'.", True),
    BoolVariable("compress_kernels", "Compress embedded OpenCL kernels in library binary using zlib. Useful for reducing the binary size. embed_kernels should be enabled", False),
    BoolVariable("set_soname", "If enabled the library will contain a SONAME and SHLIBVERSION and some symlinks will automatically be created between the objects. (requires SCons 2.4 or above)", False),
    BoolVariable("openmp", "Enable OpenMP backend. Only works when building with g++ and not clang++", False),
    BoolVariable("cppthreads", "Enable C++11 threads backend", True),
    PathVariable("build_dir", "Specify sub-folder for the build", ".", PathVariable.PathAccept),
    PathVariable("install_dir", "Specify sub-folder for the install", "", PathVariable.PathAccept),
    BoolVariable("exceptions", "Enable/disable C++ exception support", True),
    PathVariable("linker_script", "Use an external linker script", "", PathVariable.PathAccept),
    PathVariable("external_tests_dir", """Add examples, benchmarks and tests to the tests suite from an external path. In order to use this option, the external tests directory must have the following structure:
    EXTERNAL_TESTS_DIR:
    └── tests
        ├── benchmark
        │   ├── CL
        │   ├── datasets
        │   ├── fixtures
        │   └── Neon
        └── validation
            ├── CL
            ├── datasets
            ├── fixtures
            └── Neon\n""", "", PathVariable.PathAccept),
    BoolVariable("fixed_format_kernels", "Enable fixed format kernels for GEMM", False),
    BoolVariable("mapfile", "Generate a map file", False),
    ListVariable("custom_options", "Custom options that can be used to turn on/off features", "none", ["disable_mmla_fp"]),
    ListVariable("data_type_support", "Enable a list of data types to support", "all", ["qasymm8", "qasymm8_signed", "qsymm16", "fp16", "fp32", "integer"]),
    ListVariable("data_layout_support", "Enable a list of data layout to support", "all", ["nhwc", "nchw"]),
    ("toolchain_prefix", "Override the toolchain prefix; used by all toolchain components: compilers, linker, assembler etc. If unspecified, use default(auto) prefixes; if passed an empty string '' prefixes would be disabled", "auto"),
    ("compiler_prefix", "Override the compiler prefix; used by just compilers (CC,CXX); further overrides toolchain_prefix for compilers; this is for when the compiler prefixes are different from that of the linkers, archivers etc. If unspecified, this is the same as toolchain_prefix; if passed an empty string '' prefixes would be disabled", "auto"),
    BoolVariable("address_sanitizer", "Enable AddressSanitizer", False),
    BoolVariable("undefined_sanitizer", "Enable UndefinedBehaviorSanitizer", False),
    BoolVariable("thread_sanitizer", "Enable ThreadSanitizer", False),
    ("extra_cxx_flags", "Extra build flags applied to C++ files", ""),
    ("extra_cc_flags", "Extra build flags appliled to C and C++ files", ""),
    ("extra_link_flags", "Extra LD flags to be appended to the build command", ""),
    ("compiler_cache", "Command to prefix to the C and C++ compiler (e.g ccache)", ""),
    ("specs_file", "Specs file to use (e.g. rdimon.specs)", ""),
    ("build_config", "Operator/Data-type/Data-layout configuration to use for tailored ComputeLibrary builds. Can be a JSON file or a JSON formatted string", ""),
    BoolVariable("test_static_dynamic_linking", "Test that Compute Library built as a static library can be linked into another shared library", False)
)

if version_at_least(SCons.__version__, "4.0"):
    vars.Add(BoolVariable("export_compile_commands", "Export compile_commands.json file.", False))


env = Environment(variables=vars, ENV = os.environ)


build_path = env['build_dir']
# If build_dir is a relative path then add a #build/ prefix:
if not env['build_dir'].startswith('/'):
    SConsignFile('build/%s/.scons' % build_path)
    build_path = "#build/%s" % build_path
else:
    SConsignFile('%s/.scons' % build_path)

install_path = env['install_dir']
#If the install_dir is a relative path then assume it's from inside build_dir
if not env['install_dir'].startswith('/') and install_path != "":
    install_path = "%s/%s" % (build_path, install_path)

env.Append(LIBPATH = [build_path, os.path.join(build_path, "")])
Export('env')
Export('vars')

def install_lib( lib ):
    # If there is no install folder, then there is nothing to do:
    if install_path == "":
        return lib
    if env['os'] == 'qnx':
        return env.Install( "%s/aarch64le/usr/lib/" % install_path, lib)
    return env.Install( "%s/lib/" % install_path, lib)

def install_bin( bin ):
    # If there is no install folder, then there is nothing to do:
    if install_path == "":
        return bin
    if env['os'] == 'qnx':
        return env.Install( "%s/aarch64le/usr/bin/" % install_path, bin)
    return env.Install( "%s/bin/" % install_path, bin)
def install_include( inc ):
    if install_path == "":
        return inc
    if env['os'] == 'qnx':
        return env.Install( "%s/usr/include/" % install_path, inc)
    return env.Install( "%s/include/" % install_path, inc)

Export('install_lib')
Export('install_bin')

Help(vars.GenerateHelpText(env))

# Export compile_commands.json file
if env.get("export_compile_commands", False):
    env.Tool("compilation_db")
    env.CompilationDatabase("%s/compile_commands.json" % build_path)

if 'armv7a' in env['arch'] and env['os'] == 'android':
    print("WARNING: armv7a on Android is no longer maintained")

if env['linker_script'] and env['os'] != 'bare_metal':
    print("Linker script is only supported for bare_metal builds")
    Exit(1)

if env['build'] == "embed_only":
    SConscript('./SConscript', variant_dir=build_path, duplicate=0)
    Return()

if env['neon'] and 'x86' in env['arch']:
    print("Cannot compile Arm® Neon™ for x86")
    Exit(1)

if env['set_soname'] and not version_at_least(SCons.__version__, "2.4"):
    print("Setting the library's SONAME / SHLIBVERSION requires SCons 2.4 or above")
    print("Update your version of SCons or use set_soname=0")
    Exit(1)

if env['os'] == 'bare_metal':
    if env['cppthreads'] or env['openmp']:
         print("ERROR: OpenMP and C++11 threads not supported in bare_metal. Use cppthreads=0 openmp=0")
         Exit(1)

if env['opencl'] and env['embed_kernels'] and env['compress_kernels'] and env['os'] not in ['android']:
    print("Compressed kernels are supported only for android builds")
    Exit(1)

if not env['exceptions']:
    if env['opencl']:
         print("ERROR: OpenCL is not supported when building without exceptions. Use opencl=0")
         Exit(1)

    env.Append(CPPDEFINES = ['ARM_COMPUTE_EXCEPTIONS_DISABLED'])
    env.Append(CXXFLAGS = ['-fno-exceptions'])

env.Append(CXXFLAGS = ['-DARCH_ARM',
         '-Wextra','-Wdisabled-optimization','-Wformat=2',
         '-Winit-self','-Wstrict-overflow=2','-Wswitch-default',
         '-Woverloaded-virtual', '-Wformat-security',
         '-Wctor-dtor-privacy','-Wsign-promo','-Weffc++','-Wno-overlength-strings'])

if not 'windows' in env['os']:
    env.Append(CXXFLAGS = ['-Wall','-std=c++14', '-pedantic' ])


cpp_tool = {'linux': 'g++', 'android' : 'clang++',
             'tizen': 'g++', 'macos':'clang++',
             'bare_metal':'g++', 'openbsd':'g++','windows':'clang-cl', 'qnx':'qcc -Vgcc_ntoaarch64le'}

c_tool = {'linux':'gcc', 'android': 'clang', 'tizen':'gcc',
          'macos':'clang','bare_metal':'gcc',
          'openbsd':'gcc','windows':'clang-cl', 'qnx':'qcc -Vgcc_ntoaarch64le'}

default_cpp_compiler = cpp_tool[env['os']]
default_c_compiler = c_tool[env['os']]
cpp_compiler = os.environ.get('CXX', default_cpp_compiler)
c_compiler = os.environ.get('CC', default_c_compiler)

if env['os'] == 'android' and ( 'clang++' not in cpp_compiler or 'clang' not in c_compiler ):
    print( "WARNING: Only clang is officially supported to build the Compute Library for Android")

if 'clang++' in cpp_compiler:
    env.Append(CXXFLAGS = ['-Wno-vla-extension'])
elif 'armclang' in cpp_compiler:
    pass
elif not 'windows' in env['os']:
        env.Append(CXXFLAGS = ['-Wlogical-op','-Wnoexcept','-Wstrict-null-sentinel','-Wno-misleading-indentation'])

if cpp_compiler == 'g++':
    # Don't strip comments that could include markers
    env.Append(CXXFLAGS = ['-C'])

if env['cppthreads']:
    env.Append(CPPDEFINES = [('ARM_COMPUTE_CPP_SCHEDULER', 1)])

if env['openmp']:
    env.Append(CPPDEFINES = [('ARM_COMPUTE_OPENMP_SCHEDULER', 1)])
    if not 'windows' in env['os']:
        env.Append(CXXFLAGS = ['-fopenmp'])
        env.Append(LINKFLAGS = ['-fopenmp'])
    else:
        env.Append(CXXFLAGS = ['-openmp'])
        env.Append(LINKFLAGS = ['libomp.lib'])

# Validate and define state
if env['estate'] == 'auto':
    if 'v7a' in env['arch']:
        env['estate'] = '32'
    else:
        env['estate'] = '64'

# Map legacy arch
if 'arm64' in env['arch']:
    env['estate'] = '64'

if 'v7a' in env['arch'] and env['estate'] == '64':
    print("ERROR: armv7a architecture has only 32-bit execution state")
    Exit(1)

env.Append(CPPDEFINES = ['ENABLE_NEON', 'ARM_COMPUTE_ENABLE_NEON'])

if 'sve' in env['arch']:
    env.Append(CPPDEFINES = ['ENABLE_SVE', 'ARM_COMPUTE_ENABLE_SVE'])
    if 'sve2' in env['arch']:
        env.Append(CPPDEFINES = ['ARM_COMPUTE_ENABLE_SVE2'])

if 'sme' in env['arch']:
    env.Append(CPPDEFINES = ['ENABLE_SME', 'ARM_COMPUTE_ENABLE_SME'])
    if 'sme2' in env['arch']:
       env.Append(CPPDEFINES = ['ARM_COMPUTE_ENABLE_SME2'])

# Add architecture specific flags
if env['multi_isa']:
    # assert arch version is v8
    if 'v8' not in env['arch']:
        print("Currently Multi ISA binary is only supported for arm v8 family")
        Exit(1)

    if 'v8a' in env['arch']:
        env.Append(CCFLAGS = ['-march=armv8-a'])
    else:
        if 'v8.6-a' in env['arch']:
            if "disable_mmla_fp" not in env['custom_options']:
                env.Append(CPPDEFINES = ['ARM_COMPUTE_ENABLE_SVEF32MM'])

        env.Append(CXXFLAGS = ['-march=armv8.2-a+fp16']) # explicitly enable fp16 extension otherwise __ARM_FEATURE_FP16_VECTOR_ARITHMETIC is undefined

else: # NONE "multi_isa" builds

    if 'v7a' in env['arch']:
        env.Append(CCFLAGS = ['-march=armv7-a', '-mthumb', '-mfpu=neon'])
        if (env['os'] == 'android' or env['os'] == 'tizen') and not 'hf' in env['arch']:
            env.Append(CCFLAGS = ['-mfloat-abi=softfp'])
        else:
            env.Append(CCFLAGS = ['-mfloat-abi=hard'])
    elif 'v8.6-a' in env['arch']:
        if 'armv8.6-a-sve2' in env['arch']:
            env.Append(CCFLAGS = ['-march=armv8.6-a+sve2'])
        elif 'armv8.6-a-sve' == env['arch']:
            env.Append(CCFLAGS = ['-march=armv8.6-a+sve'])
        elif 'armv8.6-a' == env['arch']:
            env.Append(CXXFLAGS = ['-march=armv8.6-a+fp16'])

        env.Append(CPPDEFINES = ['ARM_COMPUTE_ENABLE_I8MM', 'ARM_COMPUTE_ENABLE_BF16','ARM_COMPUTE_ENABLE_FP16'])
        if "disable_mmla_fp" not in env['custom_options']:
            env.Append(CPPDEFINES = ['ARM_COMPUTE_ENABLE_SVEF32MM'])
    elif 'v8' in env['arch']:
        # Preserve the V8 archs for non-multi-ISA variants
        if 'sve2' in env['arch']:
            env.Append(CXXFLAGS = ['-march=armv8.2-a+sve2+fp16+dotprod'])
        elif 'sve' in env['arch']:
            env.Append(CXXFLAGS = ['-march=armv8.2-a+sve+fp16+dotprod'])
        elif 'armv8r64' in env['arch']:
            env.Append(CCFLAGS = ['-march=armv8.4-a'])
        elif 'v8.' in env['arch']:
            env.Append(CXXFLAGS = ['-march=armv8.2-a+fp16']) # explicitly enable fp16 extension otherwise __ARM_FEATURE_FP16_VECTOR_ARITHMETIC is undefined
        else:
            env.Append(CCFLAGS = ['-march=armv8-a'])

        if 'v8.' in env['arch']:
            env.Append(CPPDEFINES = ['ARM_COMPUTE_ENABLE_FP16'])

    elif 'x86' in env['arch']:
        if env['estate'] == '32':
            env.Append(CCFLAGS = ['-m32'])
            env.Append(LINKFLAGS = ['-m32'])
        else:
            env.Append(CCFLAGS = ['-fPIC'])
            env.Append(CCFLAGS = ['-m64'])
            env.Append(LINKFLAGS = ['-m64'])

# Define toolchain
# The reason why we distinguish toolchain_prefix from compiler_prefix is for cases where the linkers/archivers use a
# different prefix than the compilers. An example is the NDK r20 toolchain
auto_toolchain_prefix = ""
if 'x86' not in env['arch']:
    if env['estate'] == '32':
        if env['os'] == 'linux':
            auto_toolchain_prefix = "arm-linux-gnueabihf-" if 'v7' in env['arch'] else "armv8l-linux-gnueabihf-"
        elif env['os'] == 'bare_metal':
            auto_toolchain_prefix = "arm-eabi-"
        elif env['os'] == 'android':
            auto_toolchain_prefix = "arm-linux-androideabi-"
        elif env['os'] == 'tizen':
            auto_toolchain_prefix = "armv7l-tizen-linux-gnueabi-"
    elif env['estate'] == '64' and 'v8' in env['arch']:
        if env['os'] == 'linux':
            auto_toolchain_prefix = "aarch64-linux-gnu-"
        elif env['os'] == 'bare_metal':
            auto_toolchain_prefix = "aarch64-elf-"
        elif env['os'] == 'android':
            auto_toolchain_prefix = "aarch64-linux-android-"
        elif env['os'] == 'tizen':
            auto_toolchain_prefix = "aarch64-tizen-linux-gnu-"

if env['build'] == 'native' or env["toolchain_prefix"] == "":
    toolchain_prefix = ""
elif env["toolchain_prefix"] == "auto":
    toolchain_prefix = auto_toolchain_prefix
else:
    toolchain_prefix = env["toolchain_prefix"]

if env['build'] == 'native' or env["compiler_prefix"] == "":
    compiler_prefix = ""
elif env["compiler_prefix"] == "auto":
    compiler_prefix = toolchain_prefix
else:
    compiler_prefix = env["compiler_prefix"]

env['CC'] = env['compiler_cache']+ " " + compiler_prefix + c_compiler
env['CXX'] = env['compiler_cache']+ " " + compiler_prefix + cpp_compiler
env['LD'] = toolchain_prefix + "ld"
env['AS'] = toolchain_prefix + "as"

if env['os'] == 'windows':
    env['AR'] = "llvm-lib"
    env['RANLIB'] = "llvm-ranlib"
else:
    env['AR'] = toolchain_prefix + "ar"

env['RANLIB'] = toolchain_prefix + "ranlib"

print("Using compilers:")
print("CC", env['CC'])
print("CXX", env['CXX'])

if not GetOption("help"):
    try:
        if env['os'] == 'windows':
            compiler_ver = subprocess.check_output("clang++ -dumpversion").decode().strip()
        elif env['os'] == 'qnx':
            compiler_ver = '8'
        else:
            compiler_ver = subprocess.check_output(env['CXX'].split() + ["-dumpversion"]).decode().strip()
    except OSError:
        print("ERROR: Compiler '%s' not found" % env['CXX'])
        Exit(1)

    if 'armclang' in cpp_compiler:
        pass
    elif 'clang++' not in cpp_compiler:
        if env['arch'] == 'arm64-v8.2-a' and not version_at_least(compiler_ver, '6.2.1'):
            print("GCC 6.2.1 or newer is required to compile armv8.2-a code")
            Exit(1)
        elif env['arch'] == 'arm64-v8a' and not version_at_least(compiler_ver, '4.9'):
            print("GCC 4.9 or newer is required to compile Arm® Neon™ code for AArch64")
            Exit(1)

        if version_at_least(compiler_ver, '6.1'):
            env.Append(CXXFLAGS = ['-Wno-ignored-attributes'])

        if compiler_ver == '4.8.3':
            env.Append(CXXFLAGS = ['-Wno-array-bounds'])

        if not version_at_least(compiler_ver, '7.0.0') and env['os'] == 'bare_metal':
            env.Append(LINKFLAGS = ['-fstack-protector-strong'])

    # Add Android NDK toolchain specific flags
    if 'clang++' in cpp_compiler and env['os'] == 'android':
        # For NDK >= r21, clang 9 or above is used
        if version_at_least(compiler_ver, '9.0.0'):
            env['ndk_above_r21'] = True

            if env['openmp']:
                env.Append(LINKFLAGS = ['-static-openmp'])

        # For NDK >= r23, clang 12 or above is used. This condition detects NDK < r23
        if not version_at_least(compiler_ver, '12.0.0'):
            # System assembler is deprecated and integrated assembler is preferred since r23.
            # However integrated assembler has always been suppressed for NDK < r23.
            # Thus for backward compatibility, we include this flag only for NDK < r23
            env.Append(CCFLAGS = ['-no-integrated-as'])

data_types = []
data_layouts = []

# Set correct data types / layouts to build
if env['build_config']:
    data_types, data_layouts = read_build_config_json(env['build_config'])
else:
    data_types = env['data_type_support']
    data_layouts = env['data_layout_support']

env = update_data_type_layout_flags(env, data_types, data_layouts)

if env['standalone']:
    if not 'windows' in env['os']:
        env.Append(CCFLAGS = ['-fPIC'])
        env.Append(LINKFLAGS = ['-static-libgcc','-static-libstdc++'])

if env['Werror']:
    env.Append(CXXFLAGS = ['-Werror'])

if env['os'] == 'android':
    env.Append(CPPDEFINES = ['ANDROID'])
    env.Append(LINKFLAGS = ['-pie', '-static-libstdc++', '-ldl'])
elif env['os'] == 'bare_metal':
    env.Append(LINKFLAGS = ['-static'])
    env.Append(CCFLAGS = ['-fPIC'])
    if env['specs_file'] == "":
        env.Append(LINKFLAGS = ['-specs=rdimon.specs'])
    env.Append(CPPDEFINES = ['NO_MULTI_THREADING'])
    env.Append(CPPDEFINES = ['BARE_METAL'])
if env['os'] == 'linux' and env['arch'] == 'armv7a':
    env.Append(CXXFLAGS = [ '-Wno-psabi' ])
if env['os'] == 'windows':
    env.Append(CXXFLAGS = [ '/std:c++14','/EHa'])
    env.Append(CXXFLAGS = [ '-Wno-c++98-compat', '-Wno-covered-switch-default','-Wno-c++98-compat-pedantic'])
    env.Append(CXXFLAGS = [ '-Wno-shorten-64-to-32', '-Wno-sign-conversion','-Wno-documentation'])
    env.Append(CXXFLAGS = [ '-Wno-extra-semi-stmt', '-Wno-float-equal','-Wno-implicit-int-conversion'])
    env.Append(CXXFLAGS = [ '-Wno-documentation-pedantic', '-Wno-extra-semi','-Wno-shadow-field-in-constructor'])
    env.Append(CXXFLAGS = [ '-Wno-float-conversion', '-Wno-switch-enum','-Wno-comma'])
    env.Append(CXXFLAGS = [ '-Wno-implicit-float-conversion', '-Wno-deprecated-declarations','-Wno-old-style-cast'])
    env.Append(CXXFLAGS = [ '-Wno-zero-as-null-pointer-constant', '-Wno-inconsistent-missing-destructor-override'])
    env.Append(CXXFLAGS = [ '-Wno-asm-operand-widths'])


if env['specs_file'] != "":
    env.Append(LINKFLAGS = ['-specs='+env['specs_file']])

if env['neon']:
    env.Append(CPPDEFINES = ['ARM_COMPUTE_CPU_ENABLED'])

if env['opencl']:
    env.Append(CPPDEFINES = ['ARM_COMPUTE_OPENCL_ENABLED'])
    if env['os'] in ['bare_metal'] or env['standalone']:
        print("Cannot link OpenCL statically, which is required for bare metal / standalone builds")
        Exit(1)

if env["os"] not in ["windows","android", "bare_metal", "qnx"] and (env['opencl'] or env['cppthreads']):
    env.Append(LIBS = ['pthread'])

if env['os'] == 'openbsd':
    env.Append(LIBS = ['c'])
    env.Append(CCFLAGS = ['-fPIC'])

if env['os'] == 'qnx':
    env.Append(LIBS = ['c++'])
    env.Append(LIBS = ['m'])
    env.Append(LIBS = ['regex'])

if env['opencl']:
    if env['embed_kernels']:
        env.Append(CPPDEFINES = ['EMBEDDED_KERNELS'])
    if env['compress_kernels']:
        env.Append(CPPDEFINES = ['ARM_COMPUTE_COMPRESSED_KERNELS'])
        env.Append(LIBS = ['z'])

if env['debug']:
    env['asserts'] = True
    if not 'windows' in env['os']:
        env.Append(CCFLAGS = ['-O0','-g','-gdwarf-2'])
    else:
        env.Append(CCFLAGS = ['-Z7','-MTd','-fms-compatibility'])
        env.Append(CXXFLAGS = ['-fdelayed-template-parsing'])
        env.Append(LINKFLAGS = ['-DEBUG'])

    env.Append(CPPDEFINES = ['ARM_COMPUTE_DEBUG_ENABLED'])
else:
    if not 'windows' in env['os']:
        env.Append(CCFLAGS = ['-O3'])
    else:
        # on windows we use clang-cl which does not support the option -O3
        if not version_at_least(compiler_ver, '17.0.0'):
            # Disable optimizations in clang 17 or later because the compiler crashes with -O2
            env.Append(CCFLAGS = ['-O2'])

if env['asserts']:
    env.Append(CPPDEFINES = ['ARM_COMPUTE_ASSERTS_ENABLED'])
    if not 'windows' in env['os']:
        env.Append(CCFLAGS = ['-fstack-protector-strong'])

if env['logging']:
    env.Append(CPPDEFINES = ['ARM_COMPUTE_LOGGING_ENABLED'])

if env['address_sanitizer']:
    if 'android' in env['os']:
        env.Append(CCFLAGS = ['-fsanitize=hwaddress'])
        env.Append(LINKFLAGS = ['-fsanitize=hwaddress'])
    else:
        env.Append(CCFLAGS = ['-fsanitize=address'])
        env.Append(LINKFLAGS = ['-fsanitize=address'])

if env['undefined_sanitizer']:
    env.Append(CCFLAGS = ['-fsanitize=undefined'])
    env.Append(LINKFLAGS = ['-fsanitize=undefined'])

if env['thread_sanitizer']:
    env.Append(CCFLAGS = ['-fsanitize=thread'])
    env.Append(LINKFLAGS = ['-fsanitize=thread'])

env.Append(CPPPATH = ['#/include', "#"])
env.Append(CXXFLAGS = env['extra_cxx_flags'])
env.Append(CCFLAGS = env['extra_cc_flags'])
env.Append(LINKFLAGS = env['extra_link_flags'])

if env['multi_isa'] and env['os'] == 'macos':
    # We add this flag to prevent additional vectorization done
    # by the compiler as those vectorizations could be SVE related
    # and be done outside of sve/sve2 specific files. This is only
    # a workaround needed for macOS but can be needed for other
    # OSes in the future. Part of the problem likely stems from hpp
    # files containing implementations instead of just declarations.
    # Those implementations are optimized by the compiler whereever
    # they are included (probably first) and depending on the file
    # they might get sve/sve2 architectural feature flags during the
    # compilation.
    env.Append(CCFLAGS = ['-fno-vectorize'])

Default( install_include("arm_compute"))
Default( install_include("support"))
Default( install_include("utils"))
for dirname in os.listdir("./include"):
    Default( install_include("include/%s" % dirname))

Export('version_at_least')

SConscript('./SConscript', variant_dir=build_path, duplicate=0)

if env['examples'] and env['build_config']:
    print("WARNING: Building examples for selected operators not supported. Use examples=0")
    Return()

if env['examples'] and env['exceptions']:
    if env['os'] == 'bare_metal' and env['arch'] == 'armv7a':
        print("WARNING: Building examples for bare metal and armv7a is not supported. Use examples=0")
        Return()
    SConscript('./examples/SConscript', variant_dir='%s/examples' % build_path, duplicate=0)

if env['exceptions']:
    if env['build_config']:
        print("WARNING: Building tests for selected operators not supported")
        Return()
    if env['os'] == 'bare_metal' and env['arch'] == 'armv7a':
        print("WARNING: Building tests for bare metal and armv7a is not supported")
        Return()
    SConscript('./tests/SConscript', variant_dir='%s/tests' % build_path, duplicate=0)

# Unknown variables are not allowed
# Note: we must delay the call of UnknownVariables until after
# we have applied the Variables object to the construction environment
unknown = vars.UnknownVariables()
if unknown:
    print("Unknown variables: %s" % " ".join(unknown.keys()))
    Exit(1)
