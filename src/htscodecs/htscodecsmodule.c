/*
Copyright (c) 2026, Leiden University Medical Center

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define PY_SSIZE_T_CLEAN


#define Py_LIMITED_API 0x030D0000  /* 3.13 or higher. */
#include "Python.h"

#include "htscodecs/arith_dynamic.h"
#include "htscodecs/fqzcomp_qual.h"
#include "htscodecs/htscodecs.h"
#include "htscodecs/pack.h"
#include "htscodecs/rANS_static4x16.h"
#include "htscodecs/rANS_static.h"
#include "htscodecs/rle.h"
#include "htscodecs/tokenise_name3.h"
#include "htscodecs/varint.h"

PyDoc_STRVAR(htscodecs_version__doc__,
"htscodecs_version($module)\n"
"--\n"
"\n"
"Return the version of the htscodecs C library in use.\n"
);


static PyObject *
py_htscodecs_version(PyObject *Py_UNUSED(module), PyObject *Py_UNUSED(args))
{
   return PyUnicode_FromString(htscodecs_version());
}

static PyMethodDef htscodecs_methods[] = {
   {"htscodecs_version", py_htscodecs_version, METH_NOARGS, 
    htscodecs_version__doc__},
   {NULL,}
};

static struct _htscodecs_state {
} HtsCodecsState;

static int htscodecs_exec(PyObject *module)
{
   return 0;
}

static PyModuleDef_Slot htscodecs_slots [] = {
   {Py_mod_exec, htscodecs_exec},
   {0, NULL}
};

static PyModuleDef htscodecs_module_def = {
   PyModuleDef_HEAD_INIT,
   .m_name = "htscodecs.htscodecs",
   .m_doc = NULL,
   .m_size = sizeof(HtsCodecsState),
   .m_methods = htscodecs_methods,
   .m_slots = htscodecs_slots,
   .m_traverse = NULL,
   .m_clear = NULL,
   .m_free = NULL,
};

PyMODINIT_FUNC
PyInit_htscodecs(void) {
   return PyModuleDef_Init(&htscodecs_module_def);
}