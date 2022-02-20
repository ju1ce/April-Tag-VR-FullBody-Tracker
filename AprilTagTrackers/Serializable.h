#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

template<typename IStream, typename OStream>
class Serializer
{
public:
    using IS = IStream;
    using OS = OStream;

    virtual ~Serializer() = default;

protected:
    template<typename FieldData>
    /* virtual */ void SerializeField(OS &os, const FieldData& fd) const
    {
        os << fd.get();
    }

    template<typename FieldData>
    /* virtual */ void DeserializeField(IS &is, const FieldData& fd)
    {
        typename FieldData::FieldType temp;
        is >> temp;
        fd.set(std::move(temp));
    }

    class IFieldBase
    {
    public:
        IFieldBase(const std::string&& name)
            : name(std::move(name)) {}
        virtual ~IFieldBase() = default;
        virtual void Serialize(OS &os) const = 0;
        virtual void Deserialize(IS &is) const = 0;

        const std::string name;
    };

    // TODO: make private
    std::vector<std::unique_ptr<IFieldBase>> all_interfaces;
};

#define SRL_CLASS(arg_name, arg_serializer_type, arg_body) \
class arg_name : public arg_serializer_type \
{ \
public: \
    using Self = arg_name; \
    template<typename FT> \
    class IFieldData : public IFieldBase \
    { \
    public:\
        using FieldType = FT; \
        using Getter = const FT &(Self::*)() const; \
        using Setter = void (Self::*)(const FT &value); \
        IFieldData(const std::string&& name, Self& outer, Getter getter, Setter setter) \
            : IFieldBase(std::move(name)), outer(outer), getter(getter), setter(setter) {} \
        const FT &get() const { return (outer.*getter)(); } \
        void set(const FT &value) const { (outer.*setter)(value); } \
        void Serialize(OS &os) const override { outer.SerializeField(os, *this); } \
        void Deserialize(IS &is) const override { outer.DeserializeField(is, *this); } \
    private: \
        Self& outer; \
        Getter getter = nullptr; \
        Setter setter = nullptr; \
    }; \
private: \
    template<typename T> \
    using _srl_Type = typename std::remove_reference<typename std::remove_pointer<T>::type>::type; \
    template<typename FT> \
    IFieldData<FT> &_srl_register_field(const std::string&& name, \
                                        typename IFieldData<FT>::Getter getter, \
                                        typename IFieldData<FT>::Setter setter) \
    { \
        auto field_ptr = new IFieldData<FT>(std::move(name), *this, getter, setter); \
        auto base_ptr = static_cast<IFieldBase *>(field_ptr); \
        all_interfaces.push_back(std::unique_ptr<IFieldBase>(base_ptr)); \
        return *field_ptr; \
    } \
    arg_body \
}

#define SRL_FIELD_V(arg_type, arg_name, arg_on_update) \
public: \
    const _srl_Type<arg_type> & arg_name() const { return _srl_field_##arg_name; } \
    void arg_name(const _srl_Type<arg_type> & value) \
    { \
        _srl_field_##arg_name = value; \
        _srl_on_update_##arg_name(_srl_field_##arg_name); \
    } \
    IFieldData<_srl_Type<arg_type>> &interface_##arg_name = \
        _srl_register_field<_srl_Type<arg_type>>(#arg_name, &Self::arg_name, &Self::arg_name); \
private: \
    static void _srl_on_update_##arg_name(_srl_Type<arg_type>& value) arg_on_update \
    arg_type _srl_field_##arg_name

#define SRL_FIELD(arg_type, arg_name) SRL_FIELD_V(arg_type, arg_name, {})