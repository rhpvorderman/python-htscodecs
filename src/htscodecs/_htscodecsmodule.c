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

#define htscodecs_version_method METH_NOARGS

static PyObject *
py_htscodecs_version(PyObject *Py_UNUSED(module), PyObject *Py_UNUSED(args))
{
   return PyUnicode_FromString(htscodecs_version());
}


PyDoc_STRVAR(rans_compress_4x16_flags__doc__,
"rans_compress_4x16($module, data, /, flags=0)\n"
"--\n"
"\n"
"Compress data using the rANS 4x16 codec.\n"
"\n"
"  data\n"
"    bytes or any object that supports the buffer protocol.\n"
"  flags\n"
"    integer with bit flags set."
"\n"
"Returns a bytes object.");

#define rans_compress_4x16_flags_method (METH_VARARGS | METH_KEYWORDS)

static PyObject *
rans_compress_4x16_flags(PyObject *module, PyObject *args, PyObject *kwargs)
{
   Py_buffer data = {NULL, NULL}; 
   int flags = 0;
   static char *const keywords[] =  {"", "flags", NULL};
   static const char *format = "y*|i:_htscodecs.rans_compress_4x16";
   int ret = PyArg_ParseTupleAndKeywords(args, kwargs, format, keywords, 
                                         &data, &flags);
   if (!ret) {
      return NULL;
   }
   unsigned int out_size = rans_compress_bound_4x16(data.len, flags);
   PyObject *result = PyBytes_FromStringAndSize(NULL, out_size);
   if (result == NULL) {
      PyBuffer_Release(&data);
      return PyErr_NoMemory();
   }
   unsigned char *result_buffer = (unsigned char *)PyBytes_AsString(result);
   unsigned char *out_value = rans_compress_to_4x16(
      data.buf, data.len, result_buffer, &out_size, flags);
   PyBuffer_Release(&data);
   if (out_value == NULL) {
      PyErr_Format(
         PyExc_RuntimeError, 
         "Unable to run rans_compress_to_4x16. flags: %d", flags
      );
      return NULL;
   }
   if (_PyBytes_Resize(&result, out_size) == -1) {
      return NULL;
   }
   return result;
}

PyDoc_STRVAR(rans_uncompress_4x16__doc__,
"rans_uncompress_4x16($module, data, /)\n"
"--\n"
"\n"
"Decompress data using the rANS 4x16 codec.\n"
"\n"
"  data\n"
"    bytes or any object that supports the buffer protocol.\n"
"\n"
"Returns a bytes object.");

#define rans_uncompress_4x16_method METH_O

static PyObject *
py_rans_uncompress_4x16(PyObject *module, PyObject *data_obj)
{
   Py_buffer data = {NULL, NULL}; 
   int ret = PyObject_GetBuffer(data_obj, &data, PyBUF_SIMPLE | PyBUF_READ);
   if (ret == -1) {
      return NULL;
   }
   unsigned int out_size = 0;
   /* Size calculation is complex and dependent on stripe. So let the 
      function allocate its own memory. */
   unsigned char *out = rans_uncompress_4x16(data.buf, data.len, &out_size);
   if (out == NULL) {
      PyBuffer_Release(&data);
      PyErr_SetString(PyExc_RuntimeError, "Error while decompressing data.");
      return NULL;
   }
   PyBuffer_Release(&data);
   PyObject *result = PyBytes_FromStringAndSize((char *)out, out_size);
   free(out);
   return result;
}

static PyMethodDef htscodecs_methods[] = {
   {"htscodecs_version", py_htscodecs_version, htscodecs_version_method, 
    htscodecs_version__doc__},
   {"rans_compress_4x16_flags", (PyCFunction)rans_compress_4x16_flags, 
    rans_compress_4x16_flags_method, rans_compress_4x16_flags__doc__},
   {"rans_uncompress_4x16", py_rans_uncompress_4x16, 
    rans_uncompress_4x16_method, rans_uncompress_4x16__doc__},
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
   .m_name = "htscodecs._htscodecs",
   .m_doc = NULL,
   .m_size = sizeof(HtsCodecsState),
   .m_methods = htscodecs_methods,
   .m_slots = htscodecs_slots,
   .m_traverse = NULL,
   .m_clear = NULL,
   .m_free = NULL,
};

PyMODINIT_FUNC
PyInit__htscodecs(void) {
   return PyModuleDef_Init(&htscodecs_module_def);
}