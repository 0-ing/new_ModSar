#!/usr/bin/env python
 
"""
/*
 * Copyright (C) 2015   Jeremy Chen jeremy_cz@yahoo.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
"""

import os
import bc_core as bcc
import argparse
import sys
import pprint

parser = argparse.ArgumentParser()
parser.add_argument('-v', '--verbose', help='run with verbose', action='store_true')
parser.add_argument('-d', '--debug', help='build debug version', action='store_true')
parser.add_argument('-c', '--clean', help='clean', action='store_true')
parser.add_argument('-b', '--clean_build', help='always go along with -c: -cb means clean followed by build', action='store_true')
parser.add_argument('-l', '--list', help='list all packages', action='store_true')
parser.add_argument('-e', '--exclusive', help='packages not specified for build', action='store_true')
parser.add_argument('-t', '--target_arch', help='specify target arch. Use -l for details', default=None)
parser.add_argument('-r', '--build_variant', help='specify a group to build. Use -l for details', default=None)
parser.add_argument('-a', '--dep', help='build with dependency', action='store_true')
parser.add_argument('-j', '--jobs', help='Number of jobs for make', type=int, default=0)
parser.add_argument('-g', '--cmake_generator', help='specify a generator for cmake. Use -i for details', default=None)
parser.add_argument('-D', '--extra_make_var', help='specify extra (c)make variables separated by ","', default=None)
parser.add_argument('-i', '--info', help='show information', action='store_true')
parser.add_argument('-p', '--plot', help='plot graphic diagram', action='store_true')
parser.add_argument('packages', help='packages to be built; separated by ","', nargs='?')

def show_info():
    print('================================================================')
    print('            Build Central version 1.0.1')
    print('Supported architectures: ' + ', '.join(build_config['TARGET_LIST']))
    print('     Supported variants: ' + ','.join(build_config['BUILD_VARIANTS'][target_arch]['VARIANTS']))
    print('  Building architecture: ' + target_arch)
    print('       Building variant: ' + build_variant)
    print('             Stage path: ' + os.path.normpath(bcc.get_stage_path(build_config, target_arch, build_variant)))
    print('            Output path: ' + os.path.normpath(build_config['OUTPUT_DIR']))
    print('================================================================')

def show_generator():
    print('=======================CMake Generator==========================')
    pprint.pprint(bcc.cmake_generator)
    print('================================================================')

args = parser.parse_args()

if args.cmake_generator and not args.cmake_generator in bcc.cmake_generator:
    print('Generator %s is not supported!'%(args.cmake_generator))
    show_generator();
    exit(-1)

build_config = bcc.load_build_config(None, None)
if build_config['ret'] != 'ok':
    print(build_config['ret'])
    exit(-1)

if args.extra_make_var:
    build_config['extra_make_var'] = args.extra_make_var.split(',')

target_arch = args.target_arch
host_arch = build_config['os_type']
if not target_arch:
    target_arch = build_config['DEFAULT_TARGET']
if target_arch == build_config['HOST']:
    target_arch = host_arch
if not target_arch in build_config['TARGET_LIST']:
    print('Error: invalid target arch: %s'%(args.target_arch))
    exit(-1)

if not target_arch in build_config['BUILD_VARIANTS']:
    print('Error: arch %s is not defined in BUILD_VARIANTS tag!'%(target_arch))
    exit(-1)

def sort_package_list(pkg_list):
    tmp_list = []
    all_target_found = False 
    for pkgs in pkg_list:
        for pkg in pkgs:
            if pkg != bcc.build_all_target:
                tmp_list.append(pkg)
            else:
                all_target_found = True 
    tmp_set = set(tmp_list)
    tmp_list = list(tmp_set)
    tmp_list.sort()
    if all_target_found:
        tmp_list.append(bcc.build_all_target)
    return tmp_list

build_variant = args.build_variant
if not build_variant:
    build_variant = build_config['BUILD_VARIANTS'][target_arch]['DEFAULT_VARIANT']
if not build_variant in build_config['BUILD_VARIANTS'][target_arch]['VARIANTS']:
    print('Error: variant %s is invalid!'%(build_variant))
    print('Available variant for arch %s: %s'%(target_arch,
                             ','.join(build_config['BUILD_VARIANTS'][target_arch]['VARIANTS'])))
    exit(-1)

tools_variant = build_config['BUILD_VARIANTS'][host_arch]['DEFAULT_VARIANT']

package_graphs = build_config['BUILD_VARIANTS'][target_arch]['VARIANTS'][build_variant]['GRAPHS']
sorted_packages = sort_package_list(package_graphs)
if host_arch in build_config['BUILD_VARIANTS']:
    tools_graphs = build_config['BUILD_VARIANTS'][host_arch]['VARIANTS'][tools_variant]['GRAPHS']
else:
    tools_graphs = None

if args.plot:
    bcc.draw_graphic(package_graphs)
    exit(0)

# -l without -a
if args.list and not args.dep:
    show_info()
    print('=========================Packages===============================')
    print('| %-16s| %-20s| %s'%('Package Name', 'Description', 'Source Dir'))
    for pkg in sorted_packages:
        pkg_cfg = build_config['PACKAGES'][target_arch].get(pkg, None)
        path = ''
        if pkg_cfg:
            label = pkg_cfg.get('Label', '')
            path = os.path.normpath(os.path.join(build_config['proj_root'], pkg_cfg['Path']))
        else:
            label = 'Stand for All Packages'

        print('| %-16s| %-20s| %s'%(pkg, label, path))
    exit(0)

arg_package_list = []
if args.packages:
    arg_package_list = args.packages.split(',')
else:
    arg_package_list += bcc.guess_current_package(os.getcwd(), build_config, target_arch)

for pkg in arg_package_list:
    if not pkg in sorted_packages:
        if args.packages:
            print('==== Invalid package %s for %s! Please select packages from the following: ===='%(pkg, target_arch))
            for pkg in sorted_packages:
                print(pkg)
            exit(-1)
        else:
            print('==== Work directory matches package %s but not configured in building list. Skipping...'%(pkg))
            arg_package_list.remove(pkg)

package_list = []
if args.exclusive:
    if arg_package_list:
        if bcc.build_all_target in arg_package_list:
            print('Error! -e option should specify package %s!'%(bcc.build_all_target))
            exit(-1)
        for pkg in ordered_packages:
            if not pkg in arg_package_list and pkg != bcc.build_all_target:
                package_list.append(pkg)
    else:
        print('Error! -e option should specify a package to build!')
        exit(-1)
else:
    if arg_package_list:
        package_list = arg_package_list
package_list = list(set(package_list))

if len(package_list) == 1 and package_list[0] == bcc.build_all_target:
    args.dep = True

# -la
if args.list and args.dep:
    show_info()
    for pkg in package_list:
        print(pkg)
        dep_list = []
        bcc.generate_build_order(package_graphs, pkg, dep_list)
        dep_list.reverse()

        tools = []
        if tools_graphs:
            tools = bcc.get_tools(build_config, target_arch, dep_list)
            tools_dep_list = []
            for tool in tools:
                bcc.generate_build_order(tools_graphs, tool, tools_dep_list)
            tools_dep_list.reverse()

        if target_arch == host_arch:
            dep_list = [pkg for pkg in dep_list if not pkg in tools_dep_list]

        for tool in tools_dep_list:
            print('    ' + tool + '(host)')
        for pkg in dep_list:
            print('    ' + pkg)
    exit(0)

dep_list = []
for pkg in package_list:
    bcc.generate_build_order(package_graphs, pkg, dep_list)
dep_list.reverse()
if args.dep:
    package_build_list = dep_list
else:
    package_build_list = [pkg for pkg in dep_list if pkg in package_list]

tools_build_list = []
if args.dep:
    if tools_graphs:
        tools = bcc.get_tools(build_config, target_arch, package_build_list)
        for tool in tools:
            bcc.generate_build_order(tools_graphs, tool, tools_build_list)
        tools_build_list.reverse()

if args.info:
    retry_list = []
    for pkg in package_build_list:
        installed = bcc.get_install_list(target_arch, pkg, build_config, build_variant)
        if installed['info'] == 'retry':
            retry_list.append(pkg)
        elif installed['info'] == 'ok':
            print('> ' + pkg)
            for i in installed['files']:
                print(i)
    if len(retry_list):
        print('==== Warning! Please build the following packages before installing list is available. ====')
        for pkg in retry_list:
            print('  ' + pkg)

    show_info()
    show_generator()
    exit(0)

def do_print(line):
    print(line),

not_build = False
if args.clean and not args.clean_build:
    not_build = True

clean_type = None
if args.clean:
    clean_type = 'uninstall_clean'

ret = None
failure_package = ''
if tools_build_list:
    ret = bcc.do_build_packages(tools_build_list,
                               host_arch,
                               tools_variant,
                               args.debug,
                               args.verbose,
                               clean_type,
                               not_build,
                               args.jobs,
                               args.cmake_generator,
                               build_config,
                               do_print)
    if ret['info'] != 'ok':
        failure_package = ret['package'] + '(host)'

if target_arch == host_arch:
    package_build_list = [pkg for pkg in package_build_list if not pkg in tools_build_list]

if ret is None or ret['info'] == 'ok':
    ret = bcc.do_build_packages(package_build_list,
                               target_arch,
                               build_variant,
                               args.debug,
                               args.verbose,
                               clean_type,
                               not_build,
                               args.jobs,
                               args.cmake_generator,
                               build_config,
                               do_print)
    if ret['info'] != 'ok':
        failure_package = ret['package']

print('')
show_info()
print('==== The following packages are specified: ====')
for pkg in package_list:
    print('  ' + pkg)

print('\n==== Build status: ====')
build_list = [pkg + '(host)' for pkg in tools_build_list] + package_build_list
cur_symbol = '>'
for pkg in build_list:
    if ret['info'] != 'ok' and failure_package == pkg:
        cur_symbol = '?'
    print(cur_symbol + ' ' + pkg)
    if cur_symbol == '?':
        cur_symbol = ' '

if ret['info'] == 'ok':
    print('==== Success! ====')
else:
    print('==== Failure! ====')
    print('\n' + ret['info'])

print('\nLog file: ' + os.path.join(build_config['LOG_DIR'], target_arch, 'log'))
print('Output dir: ' + build_config['OUTPUT_DIR'] + '\n')

if ret['info'] == 'ok':
    exit(0)
else:
    exit(-1)
