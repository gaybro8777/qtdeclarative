/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QV4VALUE_P_H
#define QV4VALUE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <limits.h>
#include <cmath>

#include <QtCore/QString>
#include "qv4global_p.h"
#include <private/qv4heap_p.h>

QT_BEGIN_NAMESPACE

namespace QV4 {

namespace Heap {
    struct Base;
}

struct Q_QML_PRIVATE_EXPORT Value
{
private:
    /*
        We use 8 bytes for a value and a different variant of NaN boxing. A Double
        NaN (actually -qNaN) is indicated by a number that has the top 13 bits set, and for a
        signalling NaN it is the top 14 bits. The other values are usually set to 0 by the
        processor, and are thus free for us to store other data. We keep pointers in there for
        managed objects, and encode the other types using the free space given to use by the unused
        bits for NaN values. This also works for pointers on 64 bit systems, as they all currently
        only have 48 bits of addressable memory. (Note: we do leave the lower 49 bits available for
        pointers.)

        We xor Doubles with (0xffff8000 << 32). That has the effect that no doubles will
        get encoded with bits 63-49 all set to 0. We then use bit 48 to distinguish between
        managed/undefined (0), or Null/Int/Bool/Empty (1). So, storing a 49 bit pointer will leave
        the top 15 bits 0, which is exactly the 'natural' representation of pointers. If bit 49 is
        set, bit 48 indicates Empty (0) or integer-convertible (1). Then the 3 bit below that are
        used to encode Null/Int/Bool.

        Undefined is encoded as a managed pointer with value 0. This is the same as a nullptr.

        Specific bit-sequences:
        0 = always 0
        1 = always 1
        x = stored value
        a,b,c,d = specific bit values, see notes

        32109876 54321098 76543210 98765432 10987654 32109876 54321098 76543210 |
        66665555 55555544 44444444 33333333 33222222 22221111 11111100 00000000 | JS Value
        ------------------------------------------------------------------------+--------------
        00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 | Undefined
        00000000 0000000x xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx | Managed (heap pointer)
        a0000000 0000bc00 00000000 00000000 00000000 00000000 00000000 00000000 | NaN/Inf
        dddddddd ddddddxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx | double
        00000000 00000010 00000000 00000000 00000000 00000000 00000000 00000000 | empty (non-sparse array hole)
        00000000 00000010 10000000 00000000 00000000 00000000 00000000 00000000 | Null
        00000000 00000011 00000000 00000000 00000000 00000000 00000000 0000000x | Bool
        00000000 00000011 10000000 00000000 xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx | Int

        Notes:
        - a: xor-ed signbit, always 1 for NaN
        - bc, xor-ed values: 11 = inf, 10 = sNaN, 01 = qNaN, 00 = boxed value
        - d: xor-ed bits, where at least one bit is set, so: (val >> (64-14)) > 0
        - Undefined maps to C++ nullptr, so the "default" initialization is the same for both C++
          and JS
        - Managed has the left 15 bits set to 0, so: (val >> (64-15)) == 0
        - empty, Null, Bool, and Int have the left 14 bits set to 0, and bit 49 set to 1,
          so: (val >> (64-15)) == 1
        - Null, Bool, and Int have bit 48 set, indicating integer-convertible
        - xoring _val with NaNEncodeMask will convert to a double in "natural" representation, where
          any non double results in a NaN
        - on 32bit we can use the fact that addresses are 32bits wide, so the tag part (bits 32 to
          63) are zero. No need to shift.
    */

    quint64 _val;

public:
    QML_NEARLY_ALWAYS_INLINE quint64 &rawValueRef() { return _val; }
    QML_NEARLY_ALWAYS_INLINE quint64 rawValue() const { return _val; }
    QML_NEARLY_ALWAYS_INLINE void setRawValue(quint64 raw) { _val = raw; }

#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    static inline int valueOffset() { return 0; }
    static inline int tagOffset() { return 4; }
#else // !Q_LITTLE_ENDIAN
    static inline int valueOffset() { return 4; }
    static inline int tagOffset() { return 0; }
#endif
    QML_NEARLY_ALWAYS_INLINE void setTagValue(quint32 tag, quint32 value) { _val = quint64(tag) << 32 | value; }
    QML_NEARLY_ALWAYS_INLINE quint32 value() const { return _val & quint64(~quint32(0)); }
    QML_NEARLY_ALWAYS_INLINE quint32 tag() const { return _val >> 32; }
    QML_NEARLY_ALWAYS_INLINE void setTag(quint32 tag) { setTagValue(tag, value()); }

#if QT_POINTER_SIZE == 8
    QML_NEARLY_ALWAYS_INLINE Heap::Base *m() const
    {
        Heap::Base *b;
        memcpy(&b, &_val, 8);
        return b;
    }
    QML_NEARLY_ALWAYS_INLINE void setM(Heap::Base *b)
    {
        memcpy(&_val, &b, 8);
    }
#elif QT_POINTER_SIZE == 4
    QML_NEARLY_ALWAYS_INLINE Heap::Base *m() const
    {
        Q_STATIC_ASSERT(sizeof(Heap::Base*) == sizeof(quint32));
        Heap::Base *b;
        quint32 v = value();
        memcpy(&b, &v, 4);
        return b;
    }
    QML_NEARLY_ALWAYS_INLINE void setM(Heap::Base *b)
    {
        quint32 v;
        memcpy(&v, &b, 4);
        setTagValue(Managed_Type_Internal, v);
    }
#else
#  error "unsupported pointer size"
#endif

    QML_NEARLY_ALWAYS_INLINE int int_32() const
    {
        return int(value());
    }
    QML_NEARLY_ALWAYS_INLINE void setInt_32(int i)
    {
        setTagValue(quint32(ValueTypeInternal::Integer), quint32(i));
    }
    QML_NEARLY_ALWAYS_INLINE uint uint_32() const { return value(); }

    QML_NEARLY_ALWAYS_INLINE void setEmpty()
    {
        setTagValue(quint32(ValueTypeInternal::Empty), value());
    }

    QML_NEARLY_ALWAYS_INLINE void setEmpty(int i)
    {
        setTagValue(quint32(ValueTypeInternal::Empty), quint32(i));
    }

    QML_NEARLY_ALWAYS_INLINE void setEmpty(quint32 i)
    {
        setTagValue(quint32(ValueTypeInternal::Empty), i);
    }

    QML_NEARLY_ALWAYS_INLINE quint32 emptyValue()
    {
        Q_ASSERT(isEmpty());
        return quint32(value());
    }

    // ### Fix for 32 bit (easiest solution is to set highest bit to 1 for mananged/undefined/integercompatible
    // and use negative numbers here
    enum QuickType {
        QT_ManagedOrUndefined = 0,
        QT_ManagedOrUndefined1 = 1,
        QT_ManagedOrUndefined2 = 2,
        QT_ManagedOrUndefined3 = 3,
        QT_Empty = 4,
        QT_Null = 5,
        QT_Bool = 6,
        QT_Int = 7
        // all other values are doubles
    };

    enum Type {
        Undefined_Type = 0,
        Managed_Type = 1,
        Empty_Type = 4,
        Null_Type = 5,
        Boolean_Type = 6,
        Integer_Type = 7,
        Double_Type = 8
    };

    inline Type type() const {
        int t = quickType();
        if (t < QT_Empty)
            return _val ? Managed_Type : Undefined_Type;
        if (t > QT_Int)
            return Double_Type;
        return static_cast<Type>(t);
    }

    // Shared between 32-bit and 64-bit encoding
    enum {
        Tag_Shift = 32
    };

    // Used only by 64-bit encoding
    static const quint64 NaNEncodeMask  = 0xfffc000000000000ll;
    enum {
        IsDouble_Shift = 64-14,
        IsManagedOrUndefined_Shift = 64-15,
        IsIntegerConvertible_Shift = 64-15,
        IsIntegerOrBool_Shift = 64-16,
        QuickType_Shift = 64 - 17
    };

    static const quint64 Immediate_Mask_64 = 0x00020000u; // bit 49

    enum class ValueTypeInternal_64 {
        Empty            = Immediate_Mask_64 | 0,
        Null             = Immediate_Mask_64 | 0x08000u,
        Boolean          = Immediate_Mask_64 | 0x10000u,
        Integer          = Immediate_Mask_64 | 0x18000u
    };

    // Used only by 32-bit encoding
    enum Masks {
        SilentNaNBit           =                  0x00040000,
        NotDouble_Mask         =                  0x7ffa0000,
    };
    static const quint64 Immediate_Mask_32 = NotDouble_Mask | 0x00020000u | SilentNaNBit;

    enum class ValueTypeInternal_32 {
        Empty            = Immediate_Mask_32 | 0,
        Null             = Immediate_Mask_32 | 0x08000u,
        Boolean          = Immediate_Mask_32 | 0x10000u,
        Integer          = Immediate_Mask_32 | 0x18000u
    };

    enum {
        Managed_Type_Internal = 0
    };

    using ValueTypeInternal = ValueTypeInternal_64;

    enum {
        NaN_Mask = 0x7ff80000,
    };

    inline quint64 quickType() const { return (_val >> QuickType_Shift); }

    // used internally in property
    inline bool isEmpty() const { return tag() == quint32(ValueTypeInternal::Empty); }
    inline bool isNull() const { return tag() == quint32(ValueTypeInternal::Null); }
    inline bool isBoolean() const { return tag() == quint32(ValueTypeInternal::Boolean); }
    inline bool isInteger() const { return tag() == quint32(ValueTypeInternal::Integer); }
    inline bool isNullOrUndefined() const { return isNull() || isUndefined(); }
    inline bool isNumber() const { return quickType() >= QT_Int; }

    inline bool isUndefined() const { return _val == 0; }
    inline bool isDouble() const { return (_val >> IsDouble_Shift); }
    inline bool isManaged() const { return _val && ((_val >> IsManagedOrUndefined_Shift) == 0); }
    inline bool isManagedOrUndefined() const { return ((_val >> IsManagedOrUndefined_Shift) == 0); }

    inline bool isIntOrBool() const {
        return (_val >> IsIntegerOrBool_Shift) == 3;
    }

    inline bool integerCompatible() const {
        Q_ASSERT(!isEmpty());
        return (_val >> IsIntegerConvertible_Shift) == 1;
    }
    static inline bool integerCompatible(Value a, Value b) {
        return a.integerCompatible() && b.integerCompatible();
    }
    static inline bool bothDouble(Value a, Value b) {
        return a.isDouble() && b.isDouble();
    }
    inline bool isNaN() const { return (tag() & 0x7ffc0000  ) == 0x00040000; }

    QML_NEARLY_ALWAYS_INLINE double doubleValue() const {
        Q_ASSERT(isDouble());
        double d;
        Value v = *this;
        v._val ^= NaNEncodeMask;
        memcpy(&d, &v._val, 8);
        return d;
    }
    QML_NEARLY_ALWAYS_INLINE void setDouble(double d) {
        memcpy(&_val, &d, 8);
        _val ^= NaNEncodeMask;
        Q_ASSERT(isDouble());
    }
    inline bool isString() const;
    inline bool isObject() const;
    inline bool isFunctionObject() const;
    inline bool isInt32() {
        if (tag() == quint32(ValueTypeInternal::Integer))
            return true;
        if (isDouble()) {
            double d = doubleValue();
            int i = (int)d;
            if (i == d) {
                setInt_32(i);
                return true;
            }
        }
        return false;
    }
    double asDouble() const {
        if (tag() == quint32(ValueTypeInternal::Integer))
            return int_32();
        return doubleValue();
    }

    bool booleanValue() const {
        return int_32();
    }
    int integerValue() const {
        return int_32();
    }

    QML_NEARLY_ALWAYS_INLINE String *stringValue() const {
        if (!isString())
            return nullptr;
        return reinterpret_cast<String*>(const_cast<Value *>(this));
    }
    QML_NEARLY_ALWAYS_INLINE Object *objectValue() const {
        if (!isObject())
            return nullptr;
        return reinterpret_cast<Object*>(const_cast<Value *>(this));
    }
    QML_NEARLY_ALWAYS_INLINE Managed *managed() const {
        if (!isManaged())
            return nullptr;
        return reinterpret_cast<Managed*>(const_cast<Value *>(this));
    }
    QML_NEARLY_ALWAYS_INLINE Heap::Base *heapObject() const {
        return isManagedOrUndefined() ? m() : nullptr;
    }

    static inline Value fromHeapObject(Heap::Base *m)
    {
        Value v;
        v.setM(m);
        return v;
    }

    int toUInt16() const;
    inline int toInt32() const;
    inline unsigned int toUInt32() const;

    bool toBoolean() const {
        if (integerCompatible())
            return static_cast<bool>(int_32());

        return toBooleanImpl(*this);
    }
    static bool toBooleanImpl(Value val);
    double toInteger() const;
    inline double toNumber() const;
    static double toNumberImpl(Value v);
    double toNumberImpl() const { return toNumberImpl(*this); }
    QString toQStringNoThrow() const;
    QString toQString() const;
    Heap::String *toString(ExecutionEngine *e) const {
        if (isString())
            return reinterpret_cast<Heap::String *>(m());
        return toString(e, *this);
    }
    static Heap::String *toString(ExecutionEngine *e, Value val);
    Heap::Object *toObject(ExecutionEngine *e) const {
        if (isObject())
            return reinterpret_cast<Heap::Object *>(m());
        return toObject(e, *this);
    }
    static Heap::Object *toObject(ExecutionEngine *e, Value val);

    inline bool isPrimitive() const;
    inline bool tryIntegerConversion() {
        bool b = integerCompatible();
        if (b)
            setTagValue(quint32(ValueTypeInternal::Integer), value());
        return b;
    }

    template <typename T>
    const T *as() const {
        if (!isManaged())
            return 0;

        Q_ASSERT(m()->vtable());
#if !defined(QT_NO_QOBJECT_CHECK)
        static_cast<const T *>(this)->qt_check_for_QMANAGED_macro(static_cast<const T *>(this));
#endif
        const VTable *vt = m()->vtable();
        while (vt) {
            if (vt == T::staticVTable())
                return static_cast<const T *>(this);
            vt = vt->parent;
        }
        return 0;
    }
    template <typename T>
    T *as() {
        if (isManaged())
            return const_cast<T *>(const_cast<const Value *>(this)->as<T>());
        else
            return nullptr;
    }

    template<typename T> inline T *cast() {
        return static_cast<T *>(managed());
    }
    template<typename T> inline const T *cast() const {
        return static_cast<const T *>(managed());
    }

    inline uint asArrayIndex() const;
    inline bool asArrayIndex(uint &idx) const;
#ifndef V4_BOOTSTRAP
    uint asArrayLength(bool *ok) const;
#endif

    ReturnedValue asReturnedValue() const { return _val; }
    static Value fromReturnedValue(ReturnedValue val) { Value v; v._val = val; return v; }

    // Section 9.12
    bool sameValue(Value other) const;

    inline void mark(MarkStack *markStack);

    Value &operator =(const ScopedValue &v);
    Value &operator=(ReturnedValue v) { _val = v; return *this; }
    Value &operator=(Managed *m) {
        if (!m) {
            setM(0);
        } else {
            _val = reinterpret_cast<Value *>(m)->_val;
        }
        return *this;
    }
    Value &operator=(Heap::Base *o) {
        setM(o);
        return *this;
    }

    template<typename T>
    Value &operator=(const Scoped<T> &t);
};
Q_STATIC_ASSERT(std::is_trivial< Value >::value);

inline void Value::mark(MarkStack *markStack)
{
    Heap::Base *o = heapObject();
    if (o)
        o->mark(markStack);
}

inline bool Value::isString() const
{
    Heap::Base *b = heapObject();
    return b && b->vtable()->isString;
}
inline bool Value::isObject() const
{
    Heap::Base *b = heapObject();
    return b && b->vtable()->isObject;
}

inline bool Value::isFunctionObject() const
{
    Heap::Base *b = heapObject();
    return b && b->vtable()->isFunctionObject;
}

inline bool Value::isPrimitive() const
{
    return !isObject();
}

inline double Value::toNumber() const
{
    if (isInteger())
        return int_32();
    if (isDouble())
        return doubleValue();
    return toNumberImpl();
}


#ifndef V4_BOOTSTRAP
inline uint Value::asArrayIndex() const
{
#if QT_POINTER_SIZE == 8
    if (!isNumber())
        return UINT_MAX;
    if (isInteger())
        return int_32() >= 0 ? (uint)int_32() : UINT_MAX;
#else
    if (isInteger() && int_32() >= 0)
        return (uint)int_32();
    if (!isDouble())
        return UINT_MAX;
#endif
    double d = doubleValue();
    uint idx = (uint)d;
    if (idx != d)
        return UINT_MAX;
    return idx;
}

inline bool Value::asArrayIndex(uint &idx) const
{
    if (Q_LIKELY(!isDouble())) {
        if (Q_LIKELY(isInteger() && int_32() >= 0)) {
            idx = (uint)int_32();
            return true;
        }
        return false;
    }
    double d = doubleValue();
    idx = (uint)d;
    return (idx == d && idx != UINT_MAX);
}
#endif

inline
ReturnedValue Heap::Base::asReturnedValue() const
{
    return Value::fromHeapObject(const_cast<Heap::Base *>(this)).asReturnedValue();
}



struct Q_QML_PRIVATE_EXPORT Primitive : public Value
{
    inline static Primitive emptyValue();
    inline static Primitive emptyValue(uint v);
    static inline Primitive fromBoolean(bool b);
    static inline Primitive fromInt32(int i);
    inline static Primitive undefinedValue();
    static inline Primitive nullValue();
    static inline Primitive fromDouble(double d);
    static inline Primitive fromUInt32(uint i);

    using Value::toInt32;
    using Value::toUInt32;

    static double toInteger(double d);
    static int toInt32(double d);
    static unsigned int toUInt32(double d);
};

inline Primitive Primitive::undefinedValue()
{
    Primitive v;
    v.setM(nullptr);
    return v;
}

inline Primitive Primitive::emptyValue()
{
    Primitive v;
    v.setEmpty(0);
    return v;
}

inline Primitive Primitive::emptyValue(uint e)
{
    Primitive v;
    v.setEmpty(e);
    return v;
}

inline Primitive Primitive::nullValue()
{
    Primitive v;
    v.setTagValue(quint32(ValueTypeInternal::Null), 0);
    return v;
}

inline Primitive Primitive::fromBoolean(bool b)
{
    Primitive v;
    v.setTagValue(quint32(ValueTypeInternal::Boolean), b);
    return v;
}

inline Primitive Primitive::fromDouble(double d)
{
    Primitive v;
    v.setDouble(d);
    return v;
}

inline Primitive Primitive::fromInt32(int i)
{
    Primitive v;
    v.setInt_32(i);
    return v;
}

inline Primitive Primitive::fromUInt32(uint i)
{
    Primitive v;
    if (i < INT_MAX) {
        v.setTagValue(quint32(ValueTypeInternal::Integer), i);
    } else {
        v.setDouble(i);
    }
    return v;
}

struct Double {
    quint64 d;

    Double(double dbl) {
        memcpy(&d, &dbl, sizeof(double));
    }

    int sign() const {
        return (d >> 63) ? -1 : 1;
    }

    bool isDenormal() const {
        return static_cast<int>((d << 1) >> 53) == 0;
    }

    int exponent() const {
        return static_cast<int>((d << 1) >> 53) - 1023;
    }

    quint64 significant() const {
        quint64 m = (d << 12) >> 12;
        if (!isDenormal())
            m |= (static_cast<quint64>(1) << 52);
        return m;
    }

    static int toInt32(double d) {
        int i = static_cast<int>(d);
        if (i == d)
                return i;
        return Double(d).toInt32();
    }

    int toInt32() {
        int e = exponent() - 52;
        if (e < 0) {
            if (e <= -53)
                return 0;
            return sign() * static_cast<int>(significant() >> -e);
        } else {
            if (e > 31)
                return 0;
            return sign() * (static_cast<int>(significant()) << e);
        }
    }
};

inline double Primitive::toInteger(double d)
{
    if (std::isnan(d))
        return +0;
    else if (!d || std::isinf(d))
        return d;
    return d >= 0 ? std::floor(d) : std::ceil(d);
}

inline int Primitive::toInt32(double value)
{
    return Double::toInt32(value);
}

inline unsigned int Primitive::toUInt32(double d)
{
    return static_cast<uint>(toInt32(d));
}

struct Encode {
    static ReturnedValue undefined() {
        return Primitive::undefinedValue().rawValue();
    }
    static ReturnedValue null() {
        return Primitive::nullValue().rawValue();
    }

    explicit Encode(bool b) {
        val = Primitive::fromBoolean(b).rawValue();
    }
    explicit Encode(double d) {
        val = Primitive::fromDouble(d).rawValue();
    }
    explicit Encode(int i) {
        val = Primitive::fromInt32(i).rawValue();
    }
    explicit Encode(uint i) {
        val = Primitive::fromUInt32(i).rawValue();
    }
    explicit Encode(ReturnedValue v) {
        val = v;
    }
    Encode(Value v) {
        val = v.rawValue();
    }

    explicit Encode(Heap::Base *o) {
        val = Value::fromHeapObject(o).asReturnedValue();
    }

    explicit Encode(Value *o) {
        Q_ASSERT(o);
        val = o->asReturnedValue();
    }

    static ReturnedValue smallestNumber(double d) {
        if (static_cast<int>(d) == d && !(d == 0. && std::signbit(d)))
            return Encode(static_cast<int>(d));
        else
            return Encode(d);
    }

    operator ReturnedValue() const {
        return val;
    }
    quint64 val;
private:
    explicit Encode(void *);
};

template<typename T>
ReturnedValue value_convert(ExecutionEngine *e, const Value &v);

inline int Value::toInt32() const
{
    if (Q_LIKELY(integerCompatible()))
        return int_32();

    if (Q_LIKELY(isDouble()))
        return Double::toInt32(doubleValue());

    return Double::toInt32(toNumberImpl());
}

inline unsigned int Value::toUInt32() const
{
    return static_cast<unsigned int>(toInt32());
}

inline double Value::toInteger() const
{
    if (integerCompatible())
        return int_32();

    return Primitive::toInteger(isDouble() ? doubleValue() : toNumberImpl());
}


}

QT_END_NAMESPACE

#endif // QV4VALUE_DEF_P_H
