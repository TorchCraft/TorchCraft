# Copyright (c) 2015-present, Facebook, Inc.
# All rights reserved.
# 
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from os.path import expanduser, dirname, join
from glob import glob
from itertools import chain
from subprocess import check_output, CalledProcessError
import sys
import distutils.unixccompiler

__version__ = '1.4.0'

###############################################################################
# Monkey-patch setuptools to compile in parallel (copied from pytorch)
###############################################################################
original_link = distutils.unixccompiler.UnixCCompiler.link


def parallelCCompile(self, sources, output_dir=None, macros=None,
                     include_dirs=None, debug=0, extra_preargs=None,
                     extra_postargs=None, depends=None):
    # those lines are copied from distutils.ccompiler.CCompiler directly
    macros, objects, extra_postargs, pp_opts, build = self._setup_compile(
        output_dir, macros, include_dirs, sources, depends, extra_postargs)
    cc_args = self._get_cc_args(pp_opts, debug, extra_preargs)

    # compile using a thread pool
    import multiprocessing.pool

    def _single_compile(obj):
        src, ext = build[obj]
        self._compile(obj, src, ext, cc_args, extra_postargs, pp_opts)
    num_jobs = multiprocessing.cpu_count()
    multiprocessing.pool.ThreadPool(num_jobs).map(_single_compile, objects)

    return objects


def patched_link(self, *args, **kwargs):
    _cxx = self.compiler_cxx
    self.compiler_cxx = None
    result = original_link(self, *args, **kwargs)
    self.compiler_cxx = _cxx
    return result


distutils.ccompiler.CCompiler.compile = parallelCCompile
distutils.unixccompiler.UnixCCompiler.link = patched_link


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


sources = list(chain(
    glob('py/*.cpp'),
    glob('replayer/*.cpp'),
    glob('client/*.cpp'),
))
print(sources)

ext_modules = [
    Extension(
        'torchcraft',
        sources,
        include_dirs=[
            # Path to pybind11 headers
            get_pybind_include(),
            get_pybind_include(user=True),
            "include/torchcraft",
            "include",
            "replayer",
            ".",
            "BWEnv/fbs",
        ],
        # TODO Search for ZSTD and define this if it exists
        define_macros=[('WITH_ZSTD', None)],
        libraries=['zstd', 'zmq'],
        language='c++'
    ),
]


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        'unix': [],
    }

    if sys.platform == 'darwin':
        c_opts['unix'] += ['-stdlib=libc++', '-mmacosx-version-min=10.7']

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
            opts.append("-std=c++11")
        elif ct == 'msvc':
            opts.append('/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version())
        for ext in self.extensions:
            ext.extra_compile_args = opts
        build_ext.build_extensions(self)


setup(
    name='torchcraft',
    version=__version__,
    author='Zeming Lin',
    author_email='zlin@fb.com',
    url='',
    description='Torchcraft',
    long_description='',
    ext_modules=ext_modules,
    install_requires=['pybind11>=2.1'],
    cmdclass={'build_ext': BuildExt},
    zip_safe=False,
)
