/****************************************************************************
* Copyright 2014-2015 Trefilov Dmitrij                                      *
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
****************************************************************************/
#include "Variant.h"
#include <stdexcept>
#include "Object.h"

namespace metacpp
{
namespace detail
{

    VariantData::VariantData()
        : m_type(eFieldVoid)
    {
    }

    VariantData::~VariantData()
    {
    }

    VariantData::VariantData(bool v)
        : m_type(eFieldBool)
    {
        m_storage.m_bool = v;
    }

    VariantData::VariantData(int32_t v)
        : m_type(eFieldInt)
    {
        m_storage.m_int = v;
    }

    VariantData::VariantData(uint32_t v)
        : m_type(eFieldUint)
    {
        m_storage.m_uint = v;
    }

    VariantData::VariantData(const int64_t &v)
        : m_type(eFieldInt64)
    {
        m_storage.m_int64 = v;
    }

    VariantData::VariantData(const uint64_t &v)
        : m_type(eFieldUint64)
    {
        m_storage.m_uint64 = v;
    }

    VariantData::VariantData(const float &v)
        : m_type(eFieldFloat)
    {
        m_storage.m_float = v;
    }

    VariantData::VariantData(const double &v)
        : m_type(eFieldDouble)
    {
        m_storage.m_double = v;
    }

    VariantData::VariantData(const String &v)
        : m_type(eFieldString), m_string(v)
    {
    }

    VariantData::VariantData(const DateTime &v)
        : m_type(eFieldDateTime), m_datetime(v)
    {
    }

    VariantData::VariantData(Object *o)
        : m_type(eFieldObject)
    {
        m_object.reset(o);
    }

    VariantData::VariantData(const Array<Variant> &a)
        : m_type(eFieldArray)
    {
        m_array = a;
    }

    EFieldType VariantData::type() const
    {
        return m_type;
    }

    void *VariantData::buffer()
    {
        switch (m_type)
        {
        case eFieldBool: return &m_storage.m_bool;
        case eFieldInt: return &m_storage.m_int;
        case eFieldUint: return &m_storage.m_uint;
        case eFieldInt64: return &m_storage.m_int64;
        case eFieldUint64: return &m_storage.m_uint64;
        case eFieldFloat: return &m_storage.m_float;
        case eFieldDouble: return &m_storage.m_double;
        case eFieldObject: return m_object.get();
        case eFieldDateTime: return &m_datetime;
        case eFieldArray: return &m_array;
        case eFieldString: return &m_string;
        default:
            throw std::runtime_error("Unknown variant type");
        }
    }

    Object *VariantData::extractObject()
    {
        if (m_type != eFieldObject)
            throw std::runtime_error("Not an object variant");
        return m_object.extract();
    }

    SharedDataBase *VariantData::clone() const
    {
        throw std::runtime_error("VariantData is not clonable");
    }

    template<>
    bool VariantData::value<bool>() const
    {
        switch (m_type)
        {
        case eFieldBool:
            return m_storage.m_bool;
        case eFieldInt:
            return m_storage.m_int != 0;
        case eFieldUint:
            return m_storage.m_uint != 0;
        case eFieldInt64:
            return m_storage.m_int64 != 0;
        case eFieldUint64:
            return m_storage.m_uint64 != 0;
        case eFieldFloat:
            return m_storage.m_float != 0;
        case eFieldDouble:
            return m_storage.m_double != 0;
        default:
            throw std::invalid_argument("Variant is not convertible to bool");
        }
    }

    template<typename T>
    T VariantData::arithmetic_convert() const
    {
        switch (m_type)
        {
        case eFieldBool:
            return static_cast<T>(m_storage.m_bool);
        case eFieldInt:
            return static_cast<T>(m_storage.m_int);
        case eFieldUint:
            return static_cast<T>(m_storage.m_uint);
        case eFieldInt64:
            return static_cast<T>(m_storage.m_int64);
        case eFieldUint64:
            return static_cast<T>(m_storage.m_uint64);
        case eFieldFloat:
            return static_cast<T>(m_storage.m_float);
        case eFieldDouble:
            return static_cast<T>(m_storage.m_double);
        default:
            throw std::invalid_argument("Variant does not contain arithmetic type");
        }
    }

    template<> int8_t VariantData::value<int8_t>() const { return arithmetic_convert<int8_t>(); }
    template<> uint8_t VariantData::value<uint8_t>() const { return arithmetic_convert<uint8_t>(); }
    template<> int16_t VariantData::value<int16_t>() const { return arithmetic_convert<int16_t>(); }
    template<> uint16_t VariantData::value<uint16_t>() const { return arithmetic_convert<uint16_t>(); }
    template<> int32_t VariantData::value<int32_t>() const { return arithmetic_convert<int32_t>(); }
    template<> uint32_t VariantData::value<uint32_t>() const { return arithmetic_convert<uint32_t>(); }
    template<> int64_t VariantData::value<int64_t>() const { return arithmetic_convert<int64_t>(); }
    template<> uint64_t VariantData::value<uint64_t>() const { return arithmetic_convert<uint64_t>(); }
    template<> float VariantData::value<float>() const { return arithmetic_convert<float>(); }
    template<> double VariantData::value<double>() const { return arithmetic_convert<double>(); }
    template<> long double VariantData::value<long double>() const { return arithmetic_convert<long double>(); }

    template<>
    String VariantData::value<String>() const
    {
        switch (m_type)
        {
        case eFieldBool:
            return String::fromValue(m_storage.m_bool);
        case eFieldInt:
            return String::fromValue(m_storage.m_int);
        case eFieldUint:
            return String::fromValue(m_storage.m_uint);
        case eFieldInt64:
            return String::fromValue(m_storage.m_int64);
        case eFieldUint64:
            return String::fromValue(m_storage.m_uint64);
        case eFieldFloat:
            return String::fromValue(m_storage.m_float);
        case eFieldDouble:
            return String::fromValue(m_storage.m_double);
        case eFieldString:
            return m_string;
        case eFieldDateTime:
            return String::fromValue(m_datetime);
        default:
            throw std::invalid_argument("Variant is not convertible to String");
        }
    }

    template<>
    DateTime VariantData::value<DateTime>() const
    {
        switch (m_type)
        {
        case eFieldString:
            return DateTime::fromString(m_string.data());
        case eFieldDateTime:
            return m_datetime;
        default:
            throw std::invalid_argument("Variant is not convertible to DateTime");
        }
    }

    template<>
    Object *VariantData::value<Object *>() const
    {
        switch (m_type)
        {
        case eFieldObject:
            return m_object.get();
        default:
            throw std::invalid_argument("Variant is not of Object type");
        }
    }

    template<>
    VariantArray VariantData::value<VariantArray>() const
    {
        switch (m_type)
        {
        case eFieldArray:
            return m_array;
        default:
            throw std::invalid_argument("Variant is not of Array type");
        }
    }

    template<>
    void VariantData::value<void>() const
    {
        switch (m_type)
        {
        case eFieldVoid:
            return;
        default:
            throw std::invalid_argument("Variant is not void typed");
        }
    }
} // namespace detail

bool Variant::valid() const
{
    detail::VariantData *data = this->data();
    return data && eFieldVoid != data->type();
}

bool Variant::isIntegral() const
{
    detail::VariantData *data = this->data();
    if (!data) return false;
    switch (data->type())
    {
    case eFieldBool:
    case eFieldInt:
    case eFieldUint:
    case eFieldInt64:
    case eFieldUint64:
        return true;
    default:
        return false;
    }
}

bool Variant::isFloatingPoint() const
{
    detail::VariantData *data = this->data();
    if (!data) return false;
    switch (data->type())
    {
    case eFieldFloat:
    case eFieldDouble:
        return true;
    default:
        return false;
    }
}

bool Variant::isArithmetic() const
{
    return isIntegral() || isFloatingPoint();
}

bool Variant::isString() const
{
    detail::VariantData *data = this->data();
    if (!data) return false;
    return data->type() == eFieldString;
}

bool Variant::isDateTime() const
{
    detail::VariantData *data = this->data();
    if (!data) return false;
    return data->type() == eFieldDateTime;
}

bool Variant::isObject() const
{
    detail::VariantData *data = this->data();
    if (!data) return false;
    return data->type() == eFieldObject;
}

bool Variant::isArray() const
{
    detail::VariantData *data = this->data();
    if (!data) return false;
    return data->type() == eFieldArray;
}

const void *Variant::buffer() const
{
    detail::VariantData *data = this->getData();
    return data->buffer();
}

Object *Variant::extractObject()
{
    // NOTE: detach is not needed since we require invalidation of all instances
    // holding the same object
    detail::VariantData *data = this->getData();
    return data->extractObject();
}

detail::VariantData *Variant::getData() const
{
    detail::VariantData *data = this->data();
    if (!data)
        throw std::runtime_error("Variant is invalid");
    return data;
}

Variant::Variant(void)
{
}

Variant::~Variant()
{
}

Variant::Variant(bool v)
    : SharedDataPointer(new detail::VariantData(v))
{

}

Variant::Variant(int32_t v)
    : SharedDataPointer(new detail::VariantData(v))
{

}

Variant::Variant(uint32_t v)
    : SharedDataPointer(new detail::VariantData(v))
{

}

Variant::Variant(const int64_t &v)
    : SharedDataPointer(new detail::VariantData(v))
{

}

Variant::Variant(const uint64_t &v)
    : SharedDataPointer(new detail::VariantData(v))
{

}

Variant::Variant(const float &v)
    : SharedDataPointer(new detail::VariantData(v))
{

}

Variant::Variant(const double &v)
    : SharedDataPointer(new detail::VariantData(v))
{

}

Variant::Variant(const char *v)
    : SharedDataPointer(new detail::VariantData(String(v)))
{

}

Variant::Variant(const String& v)
    : SharedDataPointer(new detail::VariantData(v))
{

}
Variant::Variant(const DateTime &v)
    : SharedDataPointer(new detail::VariantData(v))
{

}

Variant::Variant(Object *o)
    : SharedDataPointer(new detail::VariantData(o))
{

}

Variant::Variant(const Array<Variant> &a)
    : SharedDataPointer(new detail::VariantData(a))
{

}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &stream, const Variant &v)
{
    return stream << variant_cast<String>(v);
}

std::basic_ostream<char16_t> &operator<<(std::basic_ostream<char16_t> &stream, const Variant &v)
{
    return stream << variant_cast<String>(v);
}

} // namespace metacpp
