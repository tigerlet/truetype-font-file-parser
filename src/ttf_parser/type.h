#ifndef TYPE_H
#define TYPE_H

#include <fstream>
#include <QString>

namespace ttf_dll {

#define DEL(p)  do { delete (p); (p) = nullptr; } while(0)
#define DEL_A(p) do { delete[] (p); (p) = nullptr; } while(0)
#define DEL_T(p, t) do { delete[] ((t*)p); (p) = nullptr; } while(0)

#define DLL_API

typedef signed char         Int8;
typedef signed short        Int16;
typedef signed int          Int32;
typedef signed long long    Int64;
typedef unsigned char       UInt8;
typedef unsigned short      UInt16;
typedef unsigned int        UInt32;
typedef unsigned long long  UInt64;

typedef UInt8     Byte;
typedef Int8      Char;
typedef UInt16    UShort;
typedef Int16     Short;
typedef UInt32    ULong;
typedef Int32     Long;
typedef UInt32    Fixed;
typedef Short     FWord;
typedef UShort    UFword;
typedef UInt16    F2Dot14;
typedef Int64     LongDateTime;
typedef UInt16    GlyphId;
typedef UInt16    Offset;

inline float F2Dot14ToFloat(F2Dot14 val) {
  Short mantissa = static_cast<Short>(val) >> 14;
  float fraction = static_cast<float>(val & 16383) / 16384;
  return mantissa + fraction;
}

inline ULong TableRecordTagToULong(const char *tag) {
  return *reinterpret_cast<const ULong*>(tag);
}

}

#endif
