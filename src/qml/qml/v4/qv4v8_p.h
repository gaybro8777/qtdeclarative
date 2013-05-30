/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the V4VM module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
// Copyright 2012 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/** \mainpage V8 API Reference Guide
 *
 * V8 is Google's open source JavaScript engine.
 *
 * This set of documents provides reference material generated from the
 * V8 header file, include/v8.h.
 *
 * For other documentation see http://code.google.com/apis/v8/
 */

#ifndef V8_H_
#define V8_H_

#include "qv4global_p.h"
#include "qv4string_p.h"
#include "qv4value_p.h"
#include <QStack>
#include <QSharedData>

namespace QV4 {
struct Value;
struct String;
struct ExecutionEngine;
struct Object;
class MemoryManager;
struct SimpleCallContext;
}


#include <stdint.h>

#define V8EXPORT Q_QML_EXPORT

/**
 * The v8 JavaScript engine.
 */
namespace v8 {

class Context;
class String;
class StringObject;
class Value;
class Utils;
class NumberObject;
class Object;
class Array;
class External;
class Function;
class Date;
class ImplementationUtilities;
class Signature;
class AccessorSignature;
template <class T> struct Handle;
class FunctionTemplate;
class ObjectTemplate;
class AccessorInfo;
class Isolate;
class TryCatch;

V8EXPORT void *gcProtect(void *handle);
V8EXPORT void gcProtect(void *memoryManager, void *handle);
V8EXPORT void gcUnprotect(void *memoryManager, void *handle);

// --- Weak Handles ---

/**
 * A weak reference callback function.
 *
 * This callback should either explicitly invoke Dispose on |object| if
 * V8 wrapper is not needed anymore, or 'revive' it by invocation of MakeWeak.
 *
 * \param object the weak global object to be reclaimed by the garbage collector
 * \param parameter the value passed in when making the weak global object
 */
//typedef void (*WeakReferenceCallback)(Persistent<Value> object,
//                                      void* parameter);


// --- Handles ---

#define TYPE_CHECK(T, S)                                       \
  while (false) {                                              \
    *(static_cast<T* volatile*>(0)) = static_cast<S*>(0);      \
  }

Q_QML_EXPORT quint64 qv8_get_value(const QV4::Value &v);

/**
 * An object reference managed by the v8 garbage collector.
 *
 * All objects returned from v8 have to be tracked by the garbage
 * collector so that it knows that the objects are still alive.  Also,
 * because the garbage collector may move objects, it is unsafe to
 * point directly to an object.  Instead, all objects are stored in
 * handles which are known by the garbage collector and updated
 * whenever an object moves.  Handles should always be passed by value
 * (except in cases like out-parameters) and they should never be
 * allocated on the heap.
 *
 * There are two types of handles: local and persistent handles.
 * Local handles are light-weight and transient and typically used in
 * local operations.  They are managed by HandleScopes.  Persistent
 * handles can be used when storing objects across several independent
 * operations and have to be explicitly deallocated when they're no
 * longer used.
 *
 * It is safe to extract the object stored in the handle by
 * dereferencing the handle (for instance, to extract the Object* from
 * a Handle<Object>); the value will still be governed by a handle
 * behind the scenes and the same rules apply to these values as to
 * their handles.
 */

template <typename T>
struct Handle;

template <typename T>
struct HandleOperations
{
    static void init(Handle<T> *handle)
    {
#if QT_POINTER_SIZE == 8
        handle->val = quint64(Handle<T>::_Deleted_Type) << Handle<T>::Tag_Shift;
#else
        handle->tag = Handle<T>::_Deleted_Type;
        handle->int_32 = 0;
#endif
    }
    static void init(Handle<T> *handle, T *other)
    {
        handle->val = *reinterpret_cast<quint64 *>(other);
    }

    static void init(Handle<T> *handle, const QV4::Value &v)
    {
        handle->val = qv8_get_value(v);
    }

    static void ref(Handle<T> *)
    {
    }

    static void deref(Handle<T> *)
    {
    }

    static void *protect(Handle<T> *handle)
    {
        return gcProtect(handle);
    }

    static void protect(void *memoryManager, Handle<T> *handle)
    {
        gcProtect(memoryManager, handle);
    }

    static void unProtect(void *memoryManager, Handle<T> *handle)
    {
        gcUnprotect(memoryManager, handle);
    }

    static bool isEmpty(const Handle<T> *handle)
    {
        return handle->tag == Handle<T>::_Deleted_Type;
    }

    static T *get(const Handle<T> *handle)
    {
        return const_cast<T*>(reinterpret_cast<const T*>(handle));
    }
};

#define DEFINE_REFCOUNTED_HANDLE_OPERATIONS(Type) \
    template <> \
    struct HandleOperations<Type> \
    { \
        static void init(Handle<Type> *handle) \
        { \
            handle->object = 0; \
        } \
        static void init(Handle<Type> *handle, Type *obj) \
        { \
            handle->object = obj; \
        } \
    \
        static void ref(Handle<Type> *handle) \
        { \
            if (handle->object) \
                handle->object->ref.ref(); \
        } \
    \
        static void deref(Handle<Type> *handle) \
        { \
            if (handle->object && !handle->object->ref.deref()) { \
                delete handle->object; \
                handle->object = 0; \
            } \
        } \
        static void *protect(Handle<Type> *) { return 0; } \
        static void protect(void *, Handle<Type> *) {} \
        static void unProtect(void *, Handle<Type> *) {} \
        static bool isEmpty(const Handle<Type> *handle) \
        { \
            return handle->object == 0; \
        } \
        static Type *get(const Handle<Type> *handle) \
        { \
        return handle->object; \
        } \
     \
    };

template <typename T>
struct Handle {
    Handle()
    {
        HandleOperations<T>::init(this);
    }
    template <typename Other>
    Handle(const Handle<Other> &that)
        : val(that.val)
    {
        HandleOperations<T>::ref(this);
    }

    explicit Handle(T *obj)
    {
        HandleOperations<T>::init(this, obj);
        HandleOperations<T>::ref(this);
    }

    Handle(const QV4::Value &v) {
        HandleOperations<T>::init(this, v);
    }

    Handle(const Handle<T> &other)
        : val(other.val)
    {
        HandleOperations<T>::ref(this);
    }
    Handle<T> &operator=(const Handle<T> &other)
    {
        if (this == &other)
            return *this;
        HandleOperations<T>::deref(this);
        this->val = other.val;
        HandleOperations<T>::ref(this);
        return *this;
    }
    ~Handle()
    {
        HandleOperations<T>::deref(this);
    }

    bool IsEmpty() const { return HandleOperations<T>::isEmpty(this); }

    T *operator->() const { return HandleOperations<T>::get(this); }

    T *get() const { return HandleOperations<T>::get(this); }

    template <typename Source>
    static Handle<T> Cast(Handle<Source> that)
    {
        return that.template As<T>();
    }

    template <typename Target>
    Handle<Target> As()
    {
        return Handle<Target>(*this);
    }

    void Clear()
    {
        val = 0;
    }

    template <class S> inline bool operator==(Handle<S> that) const {
        return val == that.val;
    }
    template <class S> inline bool operator!=(Handle<S> that) const {
        return val != that.val;
    }

    enum Masks {
        NotDouble_Mask = 0xfffc0000,
        Type_Mask = 0xffff8000,
        Immediate_Mask = NotDouble_Mask | 0x00008000,
        Tag_Shift = 32
    };

    enum ValueType {
        Undefined_Type = Immediate_Mask | 0x00000,
        Null_Type = Immediate_Mask | 0x10000,
        Boolean_Type = Immediate_Mask | 0x20000,
        Integer_Type = Immediate_Mask | 0x30000,
        Object_Type = NotDouble_Mask | 0x00000,
        String_Type = NotDouble_Mask | 0x10000,
        Deleted_Type = NotDouble_Mask | 0x30000
    };

    enum ImmediateFlags {
        ConvertibleToInt = Immediate_Mask | 0x1
    };

    enum ValueTypeInternal {
        _Undefined_Type = Undefined_Type,
        _Deleted_Type = Deleted_Type,
        _Null_Type = Null_Type | ConvertibleToInt,
        _Boolean_Type = Boolean_Type | ConvertibleToInt,
        _Integer_Type = Integer_Type | ConvertibleToInt,
        _Object_Type = Object_Type,
        _String_Type = String_Type
    };

    union {
        T *object;
        quint64 val;
        double dbl;
        struct {
#if Q_BYTE_ORDER != Q_LITTLE_ENDIAN
            uint tag;
#endif
            union {
                uint uint_32;
                int int_32;
#if QT_POINTER_SIZE == 4
                T *o;
#endif
            };
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
            uint tag;
#endif
        };
    };
};

// --- Special objects ---



// --- Value ---


/**
 * The superclass of all JavaScript values and objects.
 */
class V8EXPORT Value {
 public:
  /**
   * Returns true if this value is the undefined value.  See ECMA-262
   * 4.3.10.
   */
  bool IsUndefined() const;

  /**
   * Returns true if this value is the null value.  See ECMA-262
   * 4.3.11.
   */
  bool IsNull() const;

   /**
   * Returns true if this value is true.
   */
  bool IsTrue() const;

  /**
   * Returns true if this value is false.
   */
  bool IsFalse() const;

  /**
   * Returns true if this value is an instance of the String type.
   * See ECMA-262 8.4.
   */
  bool IsString() const;

  /**
   * Returns true if this value is a function.
   */
  bool IsFunction() const;

  /**
   * Returns true if this value is an array.
   */
  bool IsArray() const;

  /**
   * Returns true if this value is an object.
   */
  bool IsObject() const;

  /**
   * Returns true if this value is boolean.
   */
  bool IsBoolean() const;

  /**
   * Returns true if this value is a number.
   */
  bool IsNumber() const;

  /**
   * Returns true if this value is external.
   */
  bool IsExternal() const;

  /**
   * Returns true if this value is a 32-bit signed integer.
   */
  bool IsInt32() const;

  /**
   * Returns true if this value is a 32-bit unsigned integer.
   */
  bool IsUint32() const;

  /**
   * Returns true if this value is a Date.
   */
  bool IsDate() const;

  /**
   * Returns true if this value is a Boolean object.
   */
  bool IsBooleanObject() const;

  /**
   * Returns true if this value is a Number object.
   */
  bool IsNumberObject() const;

  /**
   * Returns true if this value is a String object.
   */
  bool IsStringObject() const;

  /**
   * Returns true if this value is a RegExp.
   */
  bool IsRegExp() const;

  /**
   * Returns true if this value is an Error.
   */
  bool IsError() const;

  Handle<String> ToString() const;
  Handle<Object> ToObject() const;

  bool BooleanValue() const;
  double NumberValue() const;
  int64_t IntegerValue() const;
  uint32_t Uint32Value() const;
  int32_t Int32Value() const;

  /** JS == */
  bool Equals(Handle<Value> that) const;
  bool StrictEquals(Handle<Value> that) const;

  static Handle<Value> NewFromInternalValue(quint64 val)
  {
      Handle<Value> res;
      res.val = val;
      return res;
  }

  QV4::Value v4Value() const;
  static Handle<Value> fromV4Value(const QV4::Value &v4Value);

};


/**
 * A JavaScript string value (ECMA-262, 4.3.17).
 */
class V8EXPORT String : public Value {
 public:

  /**
   * An ExternalStringResource is a wrapper around a two-byte string
   * buffer that resides outside V8's heap. Implement an
   * ExternalStringResource to manage the life cycle of the underlying
   * buffer.  Note that the string data must be immutable.
   */
  class V8EXPORT ExternalStringResource {
   public:
    /**
     * Override the destructor to manage the life cycle of the underlying
     * buffer.
     */
    virtual ~ExternalStringResource() {}

    /**
     * The string data from the underlying buffer.
     */
    virtual const uint16_t* data() const = 0;

    /**
     * The length of the string. That is, the number of two-byte characters.
     */
    virtual size_t length() const = 0;

      virtual void Dispose() { delete this; }

   protected:
    ExternalStringResource() {}
  };

  /**
   * Get the ExternalStringResource for an external string.  Returns
   * NULL if IsExternal() doesn't return true.
   */
  ExternalStringResource* GetExternalStringResource() const;

  static String* Cast(v8::Value* obj);

  /**
   * Allocates a new string from either UTF-8 encoded or ASCII data.
   * The second parameter 'length' gives the buffer length.
   * If the data is UTF-8 encoded, the caller must
   * be careful to supply the length parameter.
   * If it is not given, the function calls
   * 'strlen' to determine the buffer length, it might be
   * wrong if 'data' contains a null character.
   */
  static Handle<String> New(const char* data, int length = -1);

  /** Allocates a new string from 16-bit character codes.*/
  static Handle<String> New(const uint16_t* data, int length = -1);

  static Handle<String> New(QV4::String *s);

  /**
   * Creates a new external string using the data defined in the given
   * resource. When the external string is no longer live on V8's heap the
   * resource will be disposed by calling its Dispose method. The caller of
   * this function should not otherwise delete or modify the resource. Neither
   * should the underlying buffer be deallocated or modified except through the
   * destructor of the external string resource.
   */
  static Handle<String> NewExternal(ExternalStringResource* resource);


      QString asQString() const;
      QV4::String *asV4String() const;
};


enum PropertyAttribute {
  None       = 0,
  ReadOnly   = 1 << 0,
  DontEnum   = 1 << 1,
  DontDelete = 1 << 2
};

/**
 * Accessor[Getter|Setter] are used as callback functions when
 * setting|getting a particular property. See Object and ObjectTemplate's
 * method SetAccessor.
 */
typedef Handle<Value> (*AccessorGetter)(Handle<String> property,
                                        const AccessorInfo& info);


typedef void (*AccessorSetter)(Handle<String> property,
                               Handle<Value> value,
                               const AccessorInfo& info);


/**
 * Access control specifications.
 *
 * Some accessors should be accessible across contexts.  These
 * accessors have an explicit access control parameter which specifies
 * the kind of cross-context access that should be allowed.
 *
 * Additionally, for security, accessors can prohibit overwriting by
 * accessors defined in JavaScript.  For objects that have such
 * accessors either locally or in their prototype chain it is not
 * possible to overwrite the accessor by using __defineGetter__ or
 * __defineSetter__ from JavaScript code.
 */
enum AccessControl {
  DEFAULT               = 0,
  ALL_CAN_READ          = 1,
  ALL_CAN_WRITE         = 1 << 1,
  PROHIBITS_OVERWRITING = 1 << 2
};


/**
 * A JavaScript object (ECMA-262, 4.3.3)
 */
class V8EXPORT Object : public Value {
 public:
    bool Set(Handle<Value> key,
             Handle<Value> value,
             PropertyAttribute attribs = None);

  bool Set(uint32_t index,
                    Handle<Value> value);

  Handle<Value> Get(Handle<Value> key);

  Handle<Value> Get(uint32_t index);

  // TODO(1245389): Replace the type-specific versions of these
  // functions with generic ones that accept a Handle<Value> key.
  bool Has(Handle<String> key);

  bool Delete(Handle<String> key);

  bool Has(uint32_t index);

  bool Delete(uint32_t index);

  bool SetAccessor(Handle<String> name,
                   AccessorGetter getter,
                   AccessorSetter setter = 0,
                   Handle<Value> data = Handle<Value>(),
                   AccessControl settings = DEFAULT,
                   PropertyAttribute attribute = None);

  /**
   * Returns an array containing the names of the enumerable properties
   * of this object, including properties from prototype objects.  The
   * array returned by this method contains the same values as would
   * be enumerated by a for-in statement over this object.
   */
  Handle<Array> GetPropertyNames();

  /**
   * This function has the same functionality as GetPropertyNames but
   * the returned array doesn't contain the names of properties from
   * prototype objects.
   */
  Handle<Array> GetOwnPropertyNames();

  /**
   * Get the prototype object.  This does not skip objects marked to
   * be skipped by __proto__ and it does not consult the security
   * handler.
   */
  Handle<Value> GetPrototype();

  /**
   * Set the prototype object.  This does not skip objects marked to
   * be skipped by __proto__ and it does not consult the security
   * handler.
   */
  bool SetPrototype(Handle<Value> prototype);

  /** Gets the value in an internal field. */
  Handle<Value> GetInternalField(int index);
  /** Sets the value in an internal field. */
  void SetInternalField(int index, Handle<Value> value);

  class V8EXPORT ExternalResource { // NOLINT
   public:
    ExternalResource() {}
    virtual ~ExternalResource() {}

    virtual void Dispose() { delete this; }

   private:
    // Disallow copying and assigning.
    ExternalResource(const ExternalResource&);
    void operator=(const ExternalResource&);
  };

  void SetExternalResource(ExternalResource *);
  ExternalResource *GetExternalResource();

  // Testers for local properties.
  bool HasOwnProperty(Handle<String> key);

  /**
   * Returns the identity hash for this object. The current implementation
   * uses a hidden property on the object to store the identity hash.
   *
   * The return value will never be 0. Also, it is not guaranteed to be
   * unique.
   */
  int GetIdentityHash();

  /**
   * Clone this object with a fast but shallow copy.  Values will point
   * to the same values as the original object.
   */
  Handle<Object> Clone();


  /**
   * Checks whether a callback is set by the
   * ObjectTemplate::SetCallAsFunctionHandler method.
   * When an Object is callable this method returns true.
   */
  bool IsCallable();

  /**
   * Call an Object as a function if a callback is set by the
   * ObjectTemplate::SetCallAsFunctionHandler method.
   */
  Handle<Value> CallAsFunction(Handle<Object> recv,
                              int argc,
                              Handle<Value> argv[]);

  /**
   * Call an Object as a constructor if a callback is set by the
   * ObjectTemplate::SetCallAsFunctionHandler method.
   * Note: This method behaves like the Function::NewInstance method.
   */
  Handle<Value> CallAsConstructor(int argc,
                                 Handle<Value> argv[]);

  static Handle<Object> New();
  static Object* Cast(Value* obj);
};


/**
 * An instance of the built-in array constructor (ECMA-262, 15.4.2).
 */
class V8EXPORT Array : public Object {
 public:
  uint32_t Length() const;

  /**
   * Creates a JavaScript array with the given length. If the length
   * is negative the returned array will have length 0.
   */
  static Handle<Array> New(int length = 0);

  static Array* Cast(Value* obj);
};


/**
 * A JavaScript function object (ECMA-262, 15.3).
 */
class V8EXPORT Function : public Object {
 public:
  Handle<Object> NewInstance() const;
  Handle<Object> NewInstance(int argc, Handle<Value> argv[]) const;
  Handle<Value> Call(Handle<Object> recv,
                    int argc,
                    Handle<Value> argv[]);
  Handle<Value> GetName() const;

  static Function* Cast(Value* obj);
};



/**
 * A JavaScript value that wraps a C++ void*.  This type of value is
 * mainly used to associate C++ data structures with JavaScript
 * objects.
 *
 * The Wrap function V8 will return the most optimal Value object wrapping the
 * C++ void*. The type of the value is not guaranteed to be an External object
 * and no assumptions about its type should be made. To access the wrapped
 * value Unwrap should be used, all other operations on that object will lead
 * to unpredictable results.
 */
class V8EXPORT External : public Value {
 public:
  static Handle<Value> Wrap(void* data);
  static void* Unwrap(Handle<Value> obj);

  static Handle<External> New(void* value);
  static External* Cast(Value* obj);
  void* Value() const;
};


// --- Templates ---


/**
 * The superclass of object and function templates.
 */
class V8EXPORT Template : public QSharedData {
 public:
  /** Adds a property to each instance created by this template.*/
  void Set(Handle<String> name, Handle<Value> value,
           PropertyAttribute attributes = None);
  void Set(const char* name, Handle<Value> value);

  struct Property {
      QV4::PersistentValue name;
      QV4::PersistentValue value;
      PropertyAttribute attributes;
  };
  QVector<Property> m_properties;
 };

DEFINE_REFCOUNTED_HANDLE_OPERATIONS(Template)

/**
 * The argument information given to function call callbacks.  This
 * class provides access to information about the context of the call,
 * including the receiver, the number and values of arguments, and
 * the holder of the function.
 */
class V8EXPORT Arguments {
 public:
    Arguments(const QV4::Value *args, int argc, const QV4::Value &thisObject, bool isConstructor,
              const Handle<Value> &data);
  int Length() const;
  Handle<Value> operator[](int i) const;
  Handle<Object> This() const;
  QV4::Value ThisV4() const;
  Handle<Object> Holder() const;
  bool IsConstructCall() const;
  Handle<Value> Data() const;
  Isolate* GetIsolate() const;

private:
  QVector<QV4::PersistentValue> m_args;
  QV4::PersistentValue m_thisObject;
  bool m_isConstructor;
  QV4::PersistentValue m_data;
};


/**
 * The information passed to an accessor callback about the context
 * of the property access.
 */
class V8EXPORT AccessorInfo {
 public:
  AccessorInfo(const QV4::Value &thisObject, const Handle<Value> &data);
  Isolate* GetIsolate() const;
  Handle<Value> Data() const;
  Handle<Object> This() const;
  Handle<Object> Holder() const;
private:
  QV4::PersistentValue m_this;
  QV4::PersistentValue m_data;
};


typedef QV4::Value (*InvocationCallback)(const Arguments& args);
typedef QV4::Value (*NewInvocationCallback)(QV4::SimpleCallContext *);

/**
 * NamedProperty[Getter|Setter] are used as interceptors on object.
 * See ObjectTemplate::SetNamedPropertyHandler.
 */
typedef Handle<Value> (*NamedPropertyGetter)(Handle<String> property,
                                             const AccessorInfo& info);


/**
 * Returns the value if the setter intercepts the request.
 * Otherwise, returns an empty handle.
 */
typedef Handle<Value> (*NamedPropertySetter)(Handle<String> property,
                                             Handle<Value> value,
                                             const AccessorInfo& info);

/**
 * Returns a non-empty handle if the interceptor intercepts the request.
 * The result is an integer encoding property attributes (like v8::None,
 * v8::DontEnum, etc.)
 */
typedef Handle<Value> (*NamedPropertyQuery)(Handle<String> property,
                                              const AccessorInfo& info);


/**
 * Returns a non-empty handle if the deleter intercepts the request.
 * The return value is true if the property could be deleted and false
 * otherwise.
 */
typedef Handle<Value> (*NamedPropertyDeleter)(Handle<String> property,
                                                const AccessorInfo& info);

/**
 * Returns an array containing the names of the properties the named
 * property getter intercepts.
 */
typedef Handle<Array> (*NamedPropertyEnumerator)(const AccessorInfo& info);


/**
 * Returns the value of the property if the getter intercepts the
 * request.  Otherwise, returns an empty handle.
 */
typedef Handle<Value> (*IndexedPropertyGetter)(uint32_t index,
                                               const AccessorInfo& info);


/**
 * Returns the value if the setter intercepts the request.
 * Otherwise, returns an empty handle.
 */
typedef Handle<Value> (*IndexedPropertySetter)(uint32_t index,
                                               Handle<Value> value,
                                               const AccessorInfo& info);


/**
 * Returns a non-empty handle if the interceptor intercepts the request.
 * The result is an integer encoding property attributes.
 */
typedef Handle<Value> (*IndexedPropertyQuery)(uint32_t index,
                                                const AccessorInfo& info);

/**
 * Returns a non-empty handle if the deleter intercepts the request.
 * The return value is true if the property could be deleted and false
 * otherwise.
 */
typedef Handle<Value> (*IndexedPropertyDeleter)(uint32_t index,
                                                  const AccessorInfo& info);

/**
 * Returns an array containing the indices of the properties the
 * indexed property getter intercepts.
 */
typedef Handle<Array> (*IndexedPropertyEnumerator)(const AccessorInfo& info);


/**
 * A FunctionTemplate is used to create functions at runtime. There
 * can only be one function created from a FunctionTemplate in a
 * context.  The lifetime of the created function is equal to the
 * lifetime of the context.  So in case the embedder needs to create
 * temporary functions that can be collected using Scripts is
 * preferred.
 *
 * A FunctionTemplate can have properties, these properties are added to the
 * function object when it is created.
 *
 * A FunctionTemplate has a corresponding instance template which is
 * used to create object instances when the function is used as a
 * constructor. Properties added to the instance template are added to
 * each object instance.
 *
 * A FunctionTemplate can have a prototype template. The prototype template
 * is used to create the prototype object of the function.
 *
 * The following example shows how to use a FunctionTemplate:
 *
 * \code
 *    v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New();
 *    t->Set("func_property", QV4::Value::fromDouble(1));
 *
 *    v8::Local<v8::Template> proto_t = t->PrototypeTemplate();
 *    proto_t->Set("proto_method", v8::FunctionTemplate::New(InvokeCallback));
 *    proto_t->Set("proto_const", QV4::Value::fromDouble(2));
 *
 *    v8::Local<v8::ObjectTemplate> instance_t = t->InstanceTemplate();
 *    instance_t->SetAccessor("instance_accessor", InstanceAccessorCallback);
 *    instance_t->SetNamedPropertyHandler(PropertyHandlerCallback, ...);
 *    instance_t->Set("instance_property", Number::New(3));
 *
 *    v8::Local<v8::Function> function = t->GetFunction();
 *    v8::Local<v8::Object> instance = function->NewInstance();
 * \endcode
 *
 * Let's use "function" as the JS variable name of the function object
 * and "instance" for the instance object created above.  The function
 * and the instance will have the following properties:
 *
 * \code
 *   func_property in function == true;
 *   function.func_property == 1;
 *
 *   function.prototype.proto_method() invokes 'InvokeCallback'
 *   function.prototype.proto_const == 2;
 *
 *   instance instanceof function == true;
 *   instance.instance_accessor calls 'InstanceAccessorCallback'
 *   instance.instance_property == 3;
 * \endcode
 *
 * A FunctionTemplate can inherit from another one by calling the
 * FunctionTemplate::Inherit method.  The following graph illustrates
 * the semantics of inheritance:
 *
 * \code
 *   FunctionTemplate Parent  -> Parent() . prototype -> { }
 *     ^                                                  ^
 *     | Inherit(Parent)                                  | .__proto__
 *     |                                                  |
 *   FunctionTemplate Child   -> Child()  . prototype -> { }
 * \endcode
 *
 * A FunctionTemplate 'Child' inherits from 'Parent', the prototype
 * object of the Child() function has __proto__ pointing to the
 * Parent() function's prototype object. An instance of the Child
 * function has all properties on Parent's instance templates.
 *
 * Let Parent be the FunctionTemplate initialized in the previous
 * section and create a Child FunctionTemplate by:
 *
 * \code
 *   Local<FunctionTemplate> parent = t;
 *   Local<FunctionTemplate> child = FunctionTemplate::New();
 *   child->Inherit(parent);
 *
 *   Local<Function> child_function = child->GetFunction();
 *   Local<Object> child_instance = child_function->NewInstance();
 * \endcode
 *
 * The Child function and Child instance will have the following
 * properties:
 *
 * \code
 *   child_func.prototype.__proto__ == function.prototype;
 *   child_instance.instance_accessor calls 'InstanceAccessorCallback'
 *   child_instance.instance_property == 3;
 * \endcode
 */
class V8EXPORT FunctionTemplate : public Template {
 public:
  /** Creates a function template.*/
  static Handle<FunctionTemplate> New(
      InvocationCallback callback = 0,
      Handle<Value> data = Handle<Value>());
  static Handle<FunctionTemplate> New(
      NewInvocationCallback callback,
      Handle<Value> data = Handle<Value>());
  /** Returns the unique function instance in the current execution context.*/
  Handle<Function> GetFunction();

  /** Get the InstanceTemplate. */
  Handle<ObjectTemplate> InstanceTemplate();

  /**
   * A PrototypeTemplate is the template used to create the prototype object
   * of the function created by this template.
   */
  Handle<ObjectTemplate> PrototypeTemplate();

private:
  FunctionTemplate(InvocationCallback callback, Handle<Value> data);
  FunctionTemplate(NewInvocationCallback callback, Handle<Value> data);
  friend class V4V8Function;
  InvocationCallback m_callback;
  NewInvocationCallback m_newCallback;
  QV4::PersistentValue m_data;
  Handle<ObjectTemplate> m_instanceTemplate;
  Handle<ObjectTemplate> m_prototypeTemplate;
};

DEFINE_REFCOUNTED_HANDLE_OPERATIONS(FunctionTemplate)


/**
 * An ObjectTemplate is used to create objects at runtime.
 *
 * Properties added to an ObjectTemplate are added to each object
 * created from the ObjectTemplate.
 */
class V8EXPORT ObjectTemplate : public Template {
 public:
  /** Creates an ObjectTemplate. */
  static Handle<ObjectTemplate> New();

  /** Creates a new instance of this template.*/
  Handle<Object> NewInstance();

  /**
   * Sets an accessor on the object template.
   *
   * Whenever the property with the given name is accessed on objects
   * created from this ObjectTemplate the getter and setter callbacks
   * are called instead of getting and setting the property directly
   * on the JavaScript object.
   *
   * \param name The name of the property for which an accessor is added.
   * \param getter The callback to invoke when getting the property.
   * \param setter The callback to invoke when setting the property.
   * \param data A piece of data that will be passed to the getter and setter
   *   callbacks whenever they are invoked.
   * \param settings Access control settings for the accessor. This is a bit
   *   field consisting of one of more of
   *   DEFAULT = 0, ALL_CAN_READ = 1, or ALL_CAN_WRITE = 2.
   *   The default is to not allow cross-context access.
   *   ALL_CAN_READ means that all cross-context reads are allowed.
   *   ALL_CAN_WRITE means that all cross-context writes are allowed.
   *   The combination ALL_CAN_READ | ALL_CAN_WRITE can be used to allow all
   *   cross-context access.
   * \param attribute The attributes of the property for which an accessor
   *   is added.
   * \param signature The signature describes valid receivers for the accessor
   *   and is used to perform implicit instance checks against them. If the
   *   receiver is incompatible (i.e. is not an instance of the constructor as
   *   defined by FunctionTemplate::HasInstance()), an implicit TypeError is
   *   thrown and no callback is invoked.
   */
  void SetAccessor(Handle<String> name,
                   AccessorGetter getter,
                   AccessorSetter setter = 0,
                   Handle<Value> data = Handle<Value>(),
                   AccessControl settings = DEFAULT,
                   PropertyAttribute attribute = None);

  /**
   * Sets a named property handler on the object template.
   *
   * Whenever a named property is accessed on objects created from
   * this object template, the provided callback is invoked instead of
   * accessing the property directly on the JavaScript object.
   *
   * \param getter The callback to invoke when getting a property.
   * \param setter The callback to invoke when setting a property.
   * \param query The callback to invoke to check if a property is present,
   *   and if present, get its attributes.
   * \param deleter The callback to invoke when deleting a property.
   * \param enumerator The callback to invoke to enumerate all the named
   *   properties of an object.
   * \param data A piece of data that will be passed to the callbacks
   *   whenever they are invoked.
   */
  void SetNamedPropertyHandler(NamedPropertyGetter getter,
                               NamedPropertySetter setter = 0,
                               NamedPropertyQuery query = 0,
                               NamedPropertyDeleter deleter = 0,
                               NamedPropertyEnumerator enumerator = 0,
                               Handle<Value> data = Handle<Value>());
  void SetFallbackPropertyHandler(NamedPropertyGetter getter,
                                  NamedPropertySetter setter = 0,
                                  NamedPropertyQuery query = 0,
                                  NamedPropertyDeleter deleter = 0,
                                  NamedPropertyEnumerator enumerator = 0,
                                  Handle<Value> data = Handle<Value>());

  /**
   * Sets an indexed property handler on the object template.
   *
   * Whenever an indexed property is accessed on objects created from
   * this object template, the provided callback is invoked instead of
   * accessing the property directly on the JavaScript object.
   *
   * \param getter The callback to invoke when getting a property.
   * \param setter The callback to invoke when setting a property.
   * \param query The callback to invoke to check if an object has a property.
   * \param deleter The callback to invoke when deleting a property.
   * \param enumerator The callback to invoke to enumerate all the indexed
   *   properties of an object.
   * \param data A piece of data that will be passed to the callbacks
   *   whenever they are invoked.
   */
  void SetIndexedPropertyHandler(IndexedPropertyGetter getter,
                                 IndexedPropertySetter setter = 0,
                                 IndexedPropertyQuery query = 0,
                                 IndexedPropertyDeleter deleter = 0,
                                 IndexedPropertyEnumerator enumerator = 0,
                                 Handle<Value> data = Handle<Value>());

  /**
   * Gets the number of internal fields for objects generated from
   * this template.
   */
  int InternalFieldCount();

  /**
   * Sets the number of internal fields for objects generated from
   * this template.
   */
  void SetInternalFieldCount(int value);

  /**
   * Sets whether the object can store an "external resource" object.
   */
  bool HasExternalResource();
  void SetHasExternalResource(bool value);

  struct Accessor {
      QV4::PersistentValue getter;
      QV4::PersistentValue setter;
      QV4::PersistentValue name;
      PropertyAttribute attribute;
  };

  QVector<Accessor> m_accessors;

  NamedPropertyGetter m_namedPropertyGetter;
  NamedPropertySetter m_namedPropertySetter;
  NamedPropertyQuery m_namedPropertyQuery;
  NamedPropertyDeleter m_namedPropertyDeleter;
  NamedPropertyEnumerator m_namedPropertyEnumerator;
  QV4::PersistentValue m_namedPropertyData;

  NamedPropertyGetter m_fallbackPropertyGetter;
  NamedPropertySetter m_fallbackPropertySetter;
  NamedPropertyQuery m_fallbackPropertyQuery;
  NamedPropertyDeleter m_fallbackPropertyDeleter;
  NamedPropertyEnumerator m_fallbackPropertyEnumerator;
  QV4::PersistentValue m_fallbackPropertyData;

  IndexedPropertyGetter m_indexedPropertyGetter;
  IndexedPropertySetter m_indexedPropertySetter;
  IndexedPropertyQuery m_indexedPropertyQuery;
  IndexedPropertyDeleter m_indexedPropertyDeleter;
  IndexedPropertyEnumerator m_indexedPropertyEnumerator;
  QV4::PersistentValue m_indexedPropertyData;

  private:
  ObjectTemplate();
 };

DEFINE_REFCOUNTED_HANDLE_OPERATIONS(ObjectTemplate)

// --- Exceptions ---


/**
 * Schedules an exception to be thrown when returning to JavaScript.  When an
 * exception has been scheduled it is illegal to invoke any JavaScript
 * operation; the caller must return immediately and only after the exception
 * has been handled does it become legal to invoke JavaScript operations.
 */
Handle<Value> V8EXPORT ThrowException(Handle<Value> exception);

/**
 * Create new error objects by calling the corresponding error object
 * constructor with the message.
 */
class V8EXPORT Exception {
 public:
  static Handle<Value> ReferenceError(Handle<String> message);
  static Handle<Value> SyntaxError(Handle<String> message);
  static Handle<Value> TypeError(Handle<String> message);
  static Handle<Value> Error(Handle<String> message);
};


// --- User Object Comparison Callback ---
typedef bool (*UserObjectComparisonCallback)(Handle<Object> lhs,
                                             Handle<Object> rhs);

// --- Garbage Collection Callbacks ---

/**
 * Applications can register callback functions which will be called
 * before and after a garbage collection.  Allocations are not
 * allowed in the callback functions, you therefore cannot manipulate
 * objects (set or delete properties for example) since it is possible
 * such operations will result in the allocation of objects.
 */
enum GCType {
  kGCTypeScavenge = 1 << 0,
  kGCTypeMarkSweepCompact = 1 << 1,
  kGCTypeAll = kGCTypeScavenge | kGCTypeMarkSweepCompact
};

enum GCCallbackFlags {
  kNoGCCallbackFlags = 0,
  kGCCallbackFlagCompacted = 1 << 0
};

typedef void (*GCPrologueCallback)(GCType type, GCCallbackFlags flags);
typedef void (*GCCallback)();



/**
 * Isolate represents an isolated instance of the V8 engine.  V8
 * isolates have completely separate states.  Objects from one isolate
 * must not be used in other isolates.  When V8 is initialized a
 * default isolate is implicitly created and entered.  The embedder
 * can create additional isolates and use them in parallel in multiple
 * threads.  An isolate can be entered by at most one thread at any
 * given time.  The Locker/Unlocker API must be used to synchronize.
 */
class V8EXPORT Isolate {
 public:
    Isolate();
    ~Isolate();

  /**
   * Returns the entered isolate for the current thread or NULL in
   * case there is no current isolate.
   */
  static Isolate* GetCurrent();

  static QV4::ExecutionEngine *GetEngine();
  static void SetEngine(QV4::ExecutionEngine *e);

  private:
      friend class Context;
      QStack<QV4::ExecutionEngine*> m_engines;
};


/**
 * Container class for static utility functions.
 */
class V8EXPORT V8 {
 public:

  /**
   * Enables the host application to receive a notification before a
   * garbage collection.  Allocations are not allowed in the
   * callback function, you therefore cannot manipulate objects (set
   * or delete properties for example) since it is possible such
   * operations will result in the allocation of objects. It is possible
   * to specify the GCType filter for your callback. But it is not possible to
   * register the same callback function two times with different
   * GCType filters.
   */
  static void AddGCPrologueCallback(
      GCPrologueCallback callback, GCType gc_type_filter = kGCTypeAll);

  /**
   * This function removes callback which was installed by
   * AddGCPrologueCallback function.
   */
  static void RemoveGCPrologueCallback(GCPrologueCallback callback);

  /**
   * Allows the host application to declare implicit references between
   * the objects: if |parent| is alive, all |children| are alive too.
   * After each garbage collection, all implicit references
   * are removed.  It is intended to be used in the before-garbage-collection
   * callback function.
   */
//  static void AddImplicitReferences(Persistent<Object> parent,
//                                    Persistent<Value>* children,
//                                    size_t length);

};


/**
 * A sandboxed execution context with its own set of built-in objects
 * and functions.
 */
class V8EXPORT Context {
public:
  /**
   * Returns the context of the calling JavaScript code.  That is the
   * context of the top-most JavaScript frame.  If there are no
   * JavaScript frames an empty handle is returned.
   */
  static Handle<Value> GetCallingScriptData();

private:
  Context() {}
  ~Context() {}
};


}  // namespace v8


#undef V8EXPORT
#undef TYPE_CHECK


#endif  // V8_H_
