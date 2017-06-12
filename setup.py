from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from os.path import expanduser, dirname, join
from glob import glob
from itertools import chain
from subprocess import check_output, CalledProcessError
import sys
import setuptools

__version__ = '1.3.1beta'


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


def get_torch_include_lib():
    # Try includes from torch
    try:
        path = check_output("which th", shell=True).decode()
        if "not found" not in path:
            rootdir = dirname(dirname(path))
            return (join(rootdir, "include"), join(rootdir, "lib"))
    except CalledProcessError:
        pass

    # Default to using the torch7 default install dir
    return expanduser("~/torch/install/include"), expanduser("~/torch/install/lib")


torch_incdir, torch_libdir = get_torch_include_lib()
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
            "include",
            "replayer",
            ".",
            torch_incdir,
        ],
        # TODO Search for ZSTD and define this if it exists
        define_macros=[('WITH_ZSTD', None)],
        library_dirs=[torch_libdir],
        libraries=['TH', 'zstd', 'zmq'],
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
