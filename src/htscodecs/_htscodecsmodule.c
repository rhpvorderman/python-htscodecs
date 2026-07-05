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
#define Py_LIMITED_API  0x030B00F0
#include "Python.h"
#include <stdlib.h>

#include "htscodecs/arith_dynamic.h"
#include "htscodecs/fqzcomp_qual.h"
#include "htscodecs/htscodecs.h"
#include "htscodecs/pack.h"
#include "htscodecs/rANS_static4x16.h"
#include "htscodecs/rANS_static.h"
#include "htscodecs/rle.h"
#include "htscodecs/tokenise_name3.h"
#include "htscodecs/varint.h"

#define DEFAULT_LEVEL 5
#define DEFAULT_USE_ARITH 0

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

PyDoc_STRVAR(rans_compress_4x8__doc__,
"rans_compress_4x8($module, data, /, order=0)\n"
"--\n"
"\n"
"Compress data using the rANS 4x8 codec.\n"
"\n"
"  data\n"
"    bytes or any object that supports the buffer protocol.\n"
"  order\n"
"    integer, can be 0 or 1 for Order-0 and Order-1 entropy models."
"\n"
"Returns a bytes object.");

#define rans_compress_4x8_method (METH_VARARGS | METH_KEYWORDS)

static PyObject *
rans_compress_4x8(PyObject *module, PyObject *args, PyObject *kwargs)
{
   Py_buffer data = {NULL, NULL}; 
   int order = 0;
   static char *const keywords[] =  {"", "order", NULL};
   static const char *format = "y*|i:_htscodecs.rans_compress_4x16";
   int ret = PyArg_ParseTupleAndKeywords(args, kwargs, format, keywords, 
                                         &data, &order);
   if (!ret) {
      return NULL;
   }
   if (order < 0 || order > 1) {
      PyErr_Format(
         PyExc_ValueError,
         "Order must be either 0 or 1, got: %d.", order
      );
      PyBuffer_Release(&data);
      return NULL;
   }
   unsigned int out_size = 0;
   unsigned char *out = rans_compress(data.buf, data.len, &out_size, order);
   PyBuffer_Release(&data);
   if (out == NULL) {
      PyErr_Format(
         PyExc_RuntimeError, 
         "Unable to run rans_compress_4x8. order: %d", order
      );
      return NULL;
   }
   PyObject *result = PyBytes_FromStringAndSize((char *)out, out_size);
   free(out);
   return result;
}

PyDoc_STRVAR(rans_uncompress_4x8__doc__,
"rans_uncompress_4x8($module, data, /)\n"
"--\n"
"\n"
"Decompress data using the rANS 4x8 codec.\n"
"\n"
"  data\n"
"    bytes or any object that supports the buffer protocol.\n"
"\n"
"Returns a bytes object.");

#define rans_uncompress_4x8_method METH_O

static PyObject *
rans_uncompress_4x8(PyObject *module, PyObject *data_obj)
{
   Py_buffer data = {NULL, NULL}; 
   int ret = PyObject_GetBuffer(data_obj, &data, PyBUF_SIMPLE | PyBUF_READ);
   if (ret == -1) {
      return NULL;
   }
   unsigned int out_size = 0;
   /*  */
   unsigned char *out = rans_uncompress(data.buf, data.len, &out_size);
   PyBuffer_Release(&data);
   if (out == NULL) {
      PyErr_SetString(PyExc_RuntimeError, "Error while decompressing data.");
      return NULL;
   }
   PyObject *result = PyBytes_FromStringAndSize((char *)out, out_size);
   free(out);
   return result;
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
   unsigned int out_size = 0;
   unsigned char *out = rans_compress_4x16(
      data.buf, data.len, &out_size, flags);
   PyBuffer_Release(&data);
   if (out == NULL) {
      PyErr_Format(
         PyExc_RuntimeError, 
         "Unable to run rans_compress_4x16. flags: %d", flags
      );
      return NULL;
   }
   PyObject *result = PyBytes_FromStringAndSize((char *)out, out_size);
   free(out);
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
   PyBuffer_Release(&data);
   if (out == NULL) {
      PyErr_SetString(PyExc_RuntimeError, "Error while decompressing data.");
      return NULL;
   }
   PyObject *result = PyBytes_FromStringAndSize((char *)out, out_size);
   free(out);
   return result;
}


PyDoc_STRVAR(arith_compress_flags__doc__,
"arith_compress_flags($module, data, /, flags=0)\n"
"--\n"
"\n"
"Compress data using adaptive arithmetic coding.\n"
"\n"
"  data\n"
"    bytes or any object that supports the buffer protocol.\n"
"  flags\n"
"    integer with bit flags set."
"\n"
"Returns a bytes object.");

#define arith_compress_flags_method (METH_VARARGS | METH_KEYWORDS)

static PyObject *
arith_compress_flags(PyObject *module, PyObject *args, PyObject *kwargs)
{
   Py_buffer data = {NULL, NULL}; 
   int flags = 0;
   static char *const keywords[] =  {"", "flags", NULL};
   static const char *format = "y*|i:_htscodecs.arith_compress";
   int ret = PyArg_ParseTupleAndKeywords(args, kwargs, format, keywords, 
                                         &data, &flags);
   if (!ret) {
      return NULL;
   }
   unsigned int out_size = 0;
   unsigned char *out = arith_compress(
      data.buf, data.len, &out_size, flags);
   PyBuffer_Release(&data);
   if (out == NULL) {
      PyErr_Format(
         PyExc_RuntimeError, 
         "Unable to run arith_compress. flags: %d", flags
      );
      return NULL;
   }
   PyObject *result = PyBytes_FromStringAndSize((char *)out, out_size);
   free(out);
   return result;
}

PyDoc_STRVAR(arith_uncompress__doc__,
"arith_uncompress($module, data, /)\n"
"--\n"
"\n"
"Decompress data using adaptive arithmetic coding.\n"
"\n"
"  data\n"
"    bytes or any object that supports the buffer protocol.\n"
"\n"
"Returns a bytes object.");

#define arith_uncompress_method METH_O

static PyObject *
py_arith_uncompress(PyObject *module, PyObject *data_obj)
{
   Py_buffer data = {NULL, NULL}; 
   int ret = PyObject_GetBuffer(data_obj, &data, PyBUF_SIMPLE | PyBUF_READ);
   if (ret == -1) {
      return NULL;
   }
   unsigned int out_size = 0;
   /* Size calculation is complex and dependent on stripe. So let the 
      function allocate its own memory. */
   unsigned char *out = arith_uncompress(data.buf, data.len, &out_size);
   PyBuffer_Release(&data);
   if (out == NULL) {
      PyErr_SetString(PyExc_RuntimeError, "Error while decompressing data.");
      return NULL;
   }
   PyObject *result = PyBytes_FromStringAndSize((char *)out, out_size);
   free(out);
   return result;
}


PyDoc_STRVAR(tok3_encode_names_block__doc__,
"tok3_encode_names_block($module, names_block, /, level=DEFAULT_LEVEL,\n"
"                        use_arith=DEFAULT_USE_ARITH)\n"
"--\n"
"\n"
"Compress data using the tok3 encode names codec.\n"
"\n"
"This converts the names to a stream of tokens which is then compressed \n"
"using the rANS 4x16 codec."
"\n"
"  names_block\n"
"    ASCII block of strings with individual names separated by newlines or \\0 bytes\n"
"  level\n"
"    The level to compress from 1 to 9\n"
"  use_arith\n"
"     Set to True to use adaptive arithmetic coding instead of rANS 4x16.\n"
"\n"
"Returns a bytes object with the compressed data.");

#define tok3_encode_names_block_method (METH_VARARGS | METH_KEYWORDS)

static PyObject *
tok3_encode_names_block(PyObject *module, PyObject *args, PyObject *kwargs)
{
   PyObject *names_block;
   int level=DEFAULT_LEVEL;
   int use_arith=DEFAULT_USE_ARITH;
   static char *const keywords[] =  {"", "level", "use_arith", NULL};
   static const char *format = "U|ip:_htscodecs.tok3_encode_nams_block";
   int ret = PyArg_ParseTupleAndKeywords(args, kwargs, format, keywords, 
                                         &names_block, &level, &use_arith);
   if (!ret) {
      return NULL;
   }
   Py_ssize_t string_length = PyUnicode_GetLength(names_block);
   Py_ssize_t ascii_length = 0;
   const char *names = PyUnicode_AsUTF8AndSize(names_block, &ascii_length);
   if (ascii_length != string_length) {
      PyErr_SetString(
         PyExc_ValueError, 
         "Only ASCII names can be encoded."
      );
      return NULL;
   }
   if (level < 1 || level > 9) {
      PyErr_Format(
         PyExc_ValueError,
         "level must be a value between 1 and 9, got: %d", 
         level
      );
      return NULL;
   }

   int out_size = 0;
   unsigned char *out = tok3_encode_names(names, ascii_length, 
      level, use_arith, &out_size, NULL );
   if (out == NULL) {
      PyErr_Format(
         PyExc_RuntimeError, 
         "Unable to run tok3_encode_names. level: %d, use_arith: %d", 
         level, use_arith
      );
      return NULL;
   }
   PyObject *result = PyBytes_FromStringAndSize((char *)out, out_size);
   free(out);
   return result;
}


static PyMethodDef htscodecs_methods[] = {
   {"htscodecs_version", (PyCFunction)py_htscodecs_version, 
    htscodecs_version_method, htscodecs_version__doc__},
   {"rans_compress_4x8", (PyCFunction)rans_compress_4x8, 
    rans_compress_4x8_method, rans_compress_4x8__doc__},
   {"rans_uncompress_4x8", (PyCFunction)rans_uncompress_4x8, 
    rans_uncompress_4x8_method, rans_uncompress_4x8__doc__},
   {"rans_compress_4x16_flags", (PyCFunction)rans_compress_4x16_flags, 
    rans_compress_4x16_flags_method, rans_compress_4x16_flags__doc__},
   {"rans_uncompress_4x16", (PyCFunction)py_rans_uncompress_4x16, 
    rans_uncompress_4x16_method, rans_uncompress_4x16__doc__},
   {"arith_compress_flags", (PyCFunction)arith_compress_flags, 
    arith_compress_flags_method, arith_compress_flags__doc__},
   {"arith_uncompress", (PyCFunction)py_arith_uncompress, 
    arith_uncompress_method, arith_uncompress__doc__},
   {"tok3_encode_names_block", (PyCFunction)tok3_encode_names_block,
    tok3_encode_names_block_method, tok3_encode_names_block__doc__},
   {NULL,}
};

static struct _htscodecs_state {
} HtsCodecsState;

static int htscodecs_exec(PyObject *module)
{
   PyModule_AddIntConstant(module, "RANS_FLAG_X32", RANS_ORDER_X32);
   PyModule_AddIntConstant(module, "RANS_FLAG_STRIPE", RANS_ORDER_STRIPE);
   PyModule_AddIntConstant(module, "RANS_FLAG_NOSZ", RANS_ORDER_NOSZ);
   PyModule_AddIntConstant(module, "RANS_FLAG_CAT", RANS_ORDER_CAT);
   PyModule_AddIntConstant(module, "RANS_FLAG_RLE", RANS_ORDER_RLE);
   PyModule_AddIntConstant(module, "RANS_FLAG_PACK", RANS_ORDER_PACK); 
   PyModule_AddIntMacro(module, DEFAULT_LEVEL);
   PyModule_AddIntMacro(module, DEFAULT_USE_ARITH);
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