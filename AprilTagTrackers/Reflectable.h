#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// Define your own macro to implement this one, like in the example below
// field_name will also be the stringized name, appended to the prefix, and needs to reference a real field
#define REFLECTABLE_FIELD(a_visitor_visibility, a_visitor_prefix, a_field_visibility, a_field_type, a_field_name, ...) \
    a_visitor_visibility:                                                                                              \
    const FieldVisitorImpl_t<Reflect::remove_constref_t<a_field_type>> a_visitor_prefix##a_field_name =                \
        RegisterField<Reflect::remove_constref_t<a_field_type>>(                                                       \
            a_visitor_prefix##a_field_name,                                                                            \
            #a_field_name,                                                                                             \
            const_cast<Reflect::remove_constref_t<a_field_type>&>(a_field_name),                                       \
            ##__VA_ARGS__);                                                                                            \
    a_field_visibility:                                                                                                \
    a_field_type a_field_name

namespace Reflect
{

template <typename T>
using remove_constref_t = typename std::remove_reference<typename std::remove_const<T>::type>::type;

/// Optionally implement for FieldVisitor to store identifier name.
class FieldIdentifier
{
public:
    explicit FieldIdentifier(std::string name)
        : name(std::move(name)) {}
    virtual ~FieldIdentifier() = default;
    /// Name of the field passed to SRL_FIELD, stringized
    std::string name;
};

/// Optionally implement for FieldVisitorImpl to keep field reference
template <typename FieldType>
class FieldReference
{
public:
    explicit FieldReference(FieldType& field)
        : field(field) {}
    FieldType& field;
};

/// Optionally implement for FieldVisitorImpl to keep field reference behind accessors
template <typename FieldType>
// Protected to hide field reference, and only allow accessor access
class FieldAccessor : protected FieldReference<FieldType>
{
public:
    explicit FieldAccessor(FieldType& field)
        : FieldReference<FieldType>(field) {}
    const FieldType& get() const { return this->field; }
    void set(FieldType value) const { this->field = value; }
};

/// Validating function used by FieldValidateAccessor
template <typename FieldType>
using Validator = void (*)(FieldType& value);

/// Optionally implement for FieldVisitorImpl to keep field reference with validating setter
template <typename FieldType>
class FieldValidateAccessor : public FieldAccessor<FieldType>
{
public:
    FieldValidateAccessor(FieldType& field, Validator<FieldType> validator)
        : FieldAccessor<FieldType>(field), validator(validator) {}
    void set(FieldType value) const
    {
        this->field = std::move(value);
        if (validator) validator(this->field);
    }

protected:
    /// If the field is set directly, call this, if not nullptr, with the field value.
    Validator<FieldType> validator;
};

/// Derive from this class to allow the R_FIELD macro to work.
/// FieldVisitor is the non-typed abstract class to define visitors,
/// FieldVisitorImpl is derived from it, to implement the virtual visitors for templated field type.
template <typename FieldVisitor, template <typename> class FieldVisitorImpl>
class Reflectable
{
public:
    Reflectable() = default;
    virtual ~Reflectable() = default;

protected:
    using FieldVisitor_t = FieldVisitor;
    template <typename FieldType>
    using FieldVisitorImpl_t = FieldVisitorImpl<FieldType>;

    /// Enabled if validator function
    template <typename FieldType,
              std::enable_if_t<std::is_base_of<
                                   FieldValidateAccessor<FieldType>,
                                   FieldVisitorImpl<FieldType>>::value,
                               int> = 0>
    FieldVisitorImpl<FieldType> RegisterField(
        const FieldVisitorImpl<FieldType>& self,
        std::string name,
        FieldType& field_ref,
        Validator<FieldType> validator = nullptr)
    {
        fields.emplace_back(std::cref(self));
        return FieldVisitorImpl<FieldType>(std::move(name), field_ref, validator);
    }

    /// Enabled if no validator function
    template <typename FieldType,
              std::enable_if_t<!std::is_base_of<
                                   FieldValidateAccessor<FieldType>,
                                   FieldVisitorImpl<FieldType>>::value,
                               int> = 0>
    FieldVisitorImpl<FieldType> RegisterField(
        const FieldVisitorImpl<FieldType>& self,
        std::string name,
        FieldType& field_ref)
    {
        fields.emplace_back(std::cref(self));
        return FieldVisitorImpl<FieldType>(std::move(name), field_ref);
    }

    /// Reference to all visitor_ fields for iterating, in order of declaration
    std::vector<std::reference_wrapper<const FieldVisitor>> fields;
};

};
