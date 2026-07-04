# Copyright (c) 2026, Leiden University Medical Center
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import functools
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from setuptools import Extension, setup
from setuptools.command.build_ext import build_ext

HTSCODECS_SOURCE = os.path.join("src", "htscodecs", "htscodecs")

SYSTEM_IS_UNIX = (sys.platform.startswith("linux") or
                  sys.platform.startswith("darwin") or
                  'bsd' in sys.platform)
SYSTEM_IS_WINDOWS = sys.platform.startswith("win")

# Since pip builds in a temp directory by default, setting a fixed file in
# /tmp works during the entire session.
DEFAULT_CACHE_FILE = Path(tempfile.gettempdir()
                          ).absolute() / ".htscodecs_build_cache"
BUILD_CACHE = os.environ.get("PYTHON_HTSCODECS_CACHE")
BUILD_CACHE_FILE = Path(os.environ.get("PYTHON_HTSCODECS_CACHE_FILE",
                                       DEFAULT_CACHE_FILE))


class BuildHtsCodecsExt(build_ext):
    def build_extension(self, ext):
        # Add option to link dynamically for packaging systems such as conda.
        if os.getenv("PYTHON_HTSCODECS_LINK_DYNAMIC") is not None:
            # Check for htscodecs include directories. This is useful when
            # installing in a conda environment.
            possible_prefixes = [sys.exec_prefix, sys.base_exec_prefix]
            for prefix in possible_prefixes:
                if Path(prefix, "include", "htscodecs").exists():
                    ext.include_dirs = [os.path.join(prefix, "include")]
                    ext.library_dirs = [os.path.join(prefix, "lib")]
                    break   # Only one include directory is needed.
                # On windows include is in Library apparently
                elif Path(prefix, "Library", "include", "htscodecs").exists():
                    ext.include_dirs = [os.path.join(prefix, "Library",
                                                     "include")]
                    ext.library_dirs = [os.path.join(prefix, "Library", "lib")]
                    break
            ext.libraries = ["htscodecs"]
        else:
            build_dir = build_htscodecs()
            if SYSTEM_IS_UNIX:
                ext.extra_objects = [
                    os.path.join(build_dir, "htscodecs", ".libs", "libhtscodecs.a")]
            else:
                raise NotImplementedError(
                    f"Unsupported platform: {sys.platform}")
            ext.include_dirs = [os.path.join(build_dir, "htscodecs")]
        super().build_extension(ext)


@functools.lru_cache(maxsize=None)
def build_htscodecs():
    # Check for cache
    if BUILD_CACHE:
        if BUILD_CACHE_FILE.exists():
            cache_path = Path(BUILD_CACHE_FILE.read_text())
            if (cache_path / "htscodecs" / ".libs" / "libhtscodecs.a").exists():
                return str(cache_path)

    # Creating temporary directories
    build_dir = tempfile.mktemp()
    shutil.copytree(HTSCODECS_SOURCE, build_dir)

    if hasattr(os, "sched_getaffinity"):
        cpu_count = len(os.sched_getaffinity(0))
    else:  # sched_getaffinity not available on all platforms
        cpu_count = os.cpu_count() or 1  # os.cpu_count() can return None
    # Build environment is a copy of OS environment to allow user to influence
    # it.
    build_env = os.environ.copy()
    run_args = dict(cwd=build_dir, env=build_env)
    if sys.platform == "darwin":
        subprocess.run(["autoreconf", "-i", build_dir])
        subprocess.run([os.path.join(build_dir, "configure"), "--with-pic", "--disable-shared"], **run_args)
        make_program = "gmake" if shutil.which("gmake") else "make"
        subprocess.run([make_program], **run_args)
    elif sys.platform == "linux":
        subprocess.run(["autoreconf", "-i", build_dir])
        subprocess.run([os.path.join(build_dir, "configure"), "--with-pic", "--disable-shared"], **run_args)
        subprocess.run(["make", "-j", str(cpu_count)], **run_args)
    else:
        raise NotImplementedError(f"Unsupported platform: {sys.platform}")
    if BUILD_CACHE:
        BUILD_CACHE_FILE.write_text(build_dir)
    return build_dir


setup(
    cmdclass={"build_ext": BuildHtsCodecsExt},
    ext_modules=[
        Extension("htsccodecs.htscodecs", ["src/htscodecs/htscodecsmodule.c"]),
    ]
)
