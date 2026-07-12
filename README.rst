python-htscodecs
================

Python bindings for `htscodecs <https://github.com/samtools/htscodecs>`_

Currently implemented codecs:

+ rANS4x8 (``rANS_static.h``)
+ rANS4x16 (``rANS_static4x16.h``)
+ Arithmetic Adaptive Encoder (``arith_dynamic.h``)
+ Tok3 name tokeniser (``tokenise_name3.h``)

Not (yet) implemented codecs:

+ FQZ compression of quality. (``fqzcomp_qual.h``). This is a very complex 
  codec with a complex set of parameters for calling which make sense 
  specifically in the context of CRAM. Therefore it is deemed less valuable
  for inclusion (for now).
+ Bit packing (``pack.h``)
+ Run length encoding (``rle.h``)
+ Variable integers (``varint.h``)

