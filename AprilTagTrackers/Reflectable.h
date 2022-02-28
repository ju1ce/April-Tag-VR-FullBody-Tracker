#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

// Define your own macro to implement this one, like in the example below
// arg_field will also be the stringized name, appended to the prefix, and needs to reference a real field
#define REFLECTABLE_VISITOR(arg_visitor_prefix, arg_type, arg_field, ...)                           \
    const FieldVisitorImpl_t<std::remove_reference<arg_type>::type> arg_visitor_prefix##arg_field = \
        RegisterField<std::remove_reference<arg_type>::type>(arg_visitor_prefix##arg_field, #arg_field, arg_field, ##__VA_ARGS__)

// #define R_FIELD(arg_type, arg_name, ...) \
// public: \
//     REFLECTABLE_VISITOR(visitor_, arg_type, arg_name,##__VA_ARGS__); \
// private: \
//     arg_type arg_name

namespace Reflect
{

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
class FieldReferenceWithAccessor : protected FieldReference<FieldType>
{
public:
    explicit FieldReferenceWithAccessor(FieldType& field)
        : FieldReference<FieldType>(field) {}
    const FieldType& get() const { return this->field; }
    void set(FieldType value) const { this->field = value; }
};

/// Validating function used by FieldReferenceWithValidatingAccessor
template <typename FieldType>
using Validator = void (*)(FieldType& value);

/// Optionally implement for FieldVisitorImpl to keep field reference with validating setter
template <typename FieldType>
class FieldReferenceWithValidatingAccessor : public FieldReferenceWithAccessor<FieldType>
{
public:
    FieldReferenceWithValidatingAccessor(FieldType& field, Validator<FieldType> validator)
        : FieldReferenceWithAccessor<FieldType>(field), validator(validator) {}
    void set(FieldType value) const
    {
        this->field = std::move(value);
        if (validator) validator(this->field);
    }

protected:
    /// If the field is set without the setter above, call this with the field value.
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

    /// Enabled if validator function is an option
    template <typename FieldType,
              std::enable_if_t<std::is_base_of<
                  FieldReferenceWithValidatingAccessor<FieldType>,
                  FieldVisitorImpl<FieldType>>::value, int> = 0>
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
                  FieldReferenceWithValidatingAccessor<FieldType>,
                  FieldVisitorImpl<FieldType>>::value, int> = 0>
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