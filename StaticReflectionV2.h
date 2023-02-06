#ifndef STATICREFLECTIONV2_H
#define STATICREFLECTIONV2_H

#include <tuple>
#include <type_traits>
#include <utility>

#include "StaticHash.h"
#include "TupleHelper.h"

namespace static_reflection_v2
{

    enum class FieldType
    {
        MemberPtr,
        MemberPtrInfoTag,
        MemberPtrInfoFunc,
        FuncInfo
    };

    template<class T, FieldType field_type>
    struct FieldInfo
    {
        static inline constexpr FieldType this_field_type = field_type;

        const char* field_name;
        size_t      field_name_hash;
    };

    template<class T, class member_ptr, FieldType field_type = FieldType::MemberPtr>
    struct MemberPtrInfo : public FieldInfo<T, field_type>
    {
        member_ptr ptr;
    };

    template<class T, class member_ptr, class Tag>
    struct MemberPtrInfoTag : public MemberPtrInfo<T, member_ptr, FieldType::MemberPtrInfoTag>
    {
        Tag tag;
    };

    template<class T, class member_ptr, class Func>
    struct MemberPtrInfoFunc : public MemberPtrInfo<T, member_ptr, FieldType::MemberPtrInfoFunc>
    {
        Func func;
    };

    template<class T, class member_func_ptr>
    struct FuncInfo : public FieldInfo<T, FieldType::FuncInfo>
    {
        member_func_ptr ptr;
    };

    template<class T, class MemberTuple, class FuncTuple>
    struct ClassInfo
    {
        const char* class_name;
        MemberTuple member_info_tuple;
        FuncTuple   func_info_tuple;
    };

    template<class T, class C>
    constexpr auto make_member_ptr(const char* field_name, size_t field_name_hash, C T::*field_ptr)
    {
        return MemberPtrInfo<T, decltype(field_ptr)>{
            {field_name, field_name_hash},
            field_ptr
        };
    }

    template<class T, class C, class Tag>
    constexpr auto make_member_ptr_tag(const char* field_name, size_t field_name_hash, C T::*field_ptr, Tag&& tag)
    {
        return MemberPtrInfoTag<T, decltype(field_ptr), Tag>{
            {{field_name, field_name_hash}, field_ptr},
            std::move(tag)
        };
    }

    template<class T, class C, class Func>
    constexpr auto make_member_ptr_func(const char* field_name, size_t field_name_hash, C T::*field_ptr, Func&& func)
    {
        return MemberPtrInfoFunc<T, decltype(field_ptr), Func>{
            {{field_name, field_name_hash}, field_ptr},
            std::forward<Func>(func)
        };
    }

    template<class T, class C>
    constexpr auto make_func_info(const char* field_name, size_t field_name_hash, C T::*func_ptr)
    {
        return FuncInfo<T, decltype(func_ptr)>{
            {field_name, field_name_hash},
            func_ptr
        };
    }

    template<class T, class MemberTuple, class FuncTuple>
    constexpr auto make_class_info(const char* class_name, MemberTuple&& member_tuple, FuncTuple&& func_tuple)
    {
        return ClassInfo<T, MemberTuple, FuncTuple>{class_name, std::forward<MemberTuple>(member_tuple), std::forward<FuncTuple>(func_tuple)};
    }

    template<class T, class MemberTuple>
    constexpr auto make_class_info(const char* class_name, MemberTuple&& member_tuple)
    {
        return ClassInfo<T, MemberTuple, std::tuple<>>{class_name, std::forward<MemberTuple>(member_tuple)};
    }

    template<class FieldInfo>
    constexpr bool is_member_ptr()
    {
        return std::decay_t<FieldInfo>::this_field_type == FieldType::MemberPtr;
    }

    template<class FieldInfo>
    constexpr bool is_member_ptr_tag()
    {
        return std::decay_t<FieldInfo>::this_field_type == FieldType::MemberPtrInfoTag;
    }

    template<class FieldInfo>
    constexpr bool is_member_ptr_func()
    {
        return std::decay_t<FieldInfo>::this_field_type == FieldType::MemberPtrInfoFunc;
    }

    template<class FieldInfo>
    constexpr bool is_member_func()
    {
        return std::decay_t<FieldInfo>::this_field_type == FieldType::FuncInfo;
    }
}; // namespace static_reflection_v2

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct MetaClass
{
    static inline constexpr void getMetaInfo() {}
};

#define DEFINE_MEMBER(...)   std::make_tuple(__VA_ARGS__)
#define DEFINE_FUNCTION(...) std::make_tuple(__VA_ARGS__)

#define DEFINE_META(ClassT, ...)                                                                                       \
    template<>                                                                                                         \
    struct MetaClass<ClassT>                                                                                           \
    {                                                                                                                  \
        using _ThisClass = ClassT;                                                                                     \
        template<typename E = void>                                                                                    \
        static inline constexpr auto getMetaInfo()                                                                     \
        {                                                                                                              \
            return static_reflection_v2::make_class_info<_ThisClass>(#ClassT, __VA_ARGS__);                            \
        }                                                                                                              \
    };                                                                                                                 \
    template<auto N>                                                                                                   \
    const auto& get(const ClassT& f)                                                                                   \
    {                                                                                                                  \
        constexpr auto member_info = static_reflection_v2::getClassMemberInfo<ClassT, N>();                            \
        return f.*member_info.ptr;                                                                                     \
    }                                                                                                                  \
    namespace std                                                                                                      \
    {                                                                                                                  \
        template<>                                                                                                     \
        struct tuple_size<ClassT> : integral_constant<std::size_t, static_reflection_v2::getClassMemberSize<ClassT>()> \
        {                                                                                                              \
        };                                                                                                             \
        template<std::size_t N>                                                                                        \
        struct tuple_element<N, ClassT>                                                                                \
        {                                                                                                              \
            using type = decltype(get<N>(std::declval<ClassT&>()));                                                    \
        };                                                                                                             \
    } // namespace std

#define META_MEMBER(ClassField) static_reflection_v2::make_member_ptr(#ClassField, #ClassField##_HASH, &_ThisClass::ClassField)
#define META_MEMBER_TAG(ClassField, Tag) \
    static_reflection_v2::make_member_ptr_tag(#ClassField, #ClassField##_HASH, &_ThisClass::ClassField, Tag{})
#define META_MEMBER_FUNC(ClassField, Func) \
    static_reflection_v2::make_member_ptr_func(#ClassField, #ClassField##_HASH, &_ThisClass::ClassField, Func)

#define META_MEMBER_NAME(ClassField, FieldName) static_reflection_v2::make_member_ptr(FieldName, FieldName##_HASH, &_ThisClass::ClassField)
#define META_MEMBER_NAME_TAG(ClassField, FieldName, Tag) \
    static_reflection_v2::make_member_ptr_tag(FieldName, FieldName##_HASH, &_ThisClass::ClassField, Tag{})
#define META_MEMBER_NAME_FUNC(ClassField, FieldName, Func) \
    static_reflection_v2::make_member_ptr_func(FieldName, FieldName##_HASH, &_ThisClass::ClassField, Func)

#define META_FUNCTION(ClassField) static_reflection_v2::make_func_info(#ClassField, #ClassField##_HASH, &_ThisClass::ClassField)
#define META_FUNCTION_NAME(ClassField, FieldName) static_reflection_v2::make_func_info(FieldName, FieldName##_HASH, &_ThisClass::ClassField)

template<typename T>
using not_have_meta_info = std::is_same<std::invoke_result_t<decltype(&MetaClass<std::decay_t<T>>::getMetaInfo)>, void>;

template<typename T>
using have_meta_info = std::negation<not_have_meta_info<T>>;

namespace static_reflection_v2
{
    template<class T>
    static constexpr auto getClassMetaInfo()
    {
        using ThisMetaClass       = MetaClass<std::decay_t<T>>;
        constexpr auto meta_class = ThisMetaClass::getMetaInfo();
        return meta_class;
    }

    template<class T, auto N>
    static constexpr auto getClassMemberInfo()
    {
        constexpr auto meta_class = getClassMetaInfo<T>();
        return std::get<N>(meta_class.member_info_tuple);
    }

    template<class T, auto N>
    static constexpr auto getClassMemberName()
    {
        constexpr auto member_info = getClassMemberInfo<T, N>();
        return member_info.field_name;
    }

    template<class T, auto N>
    static constexpr auto getClassMemberNameHash()
    {
        constexpr auto member_info = getClassMemberInfo<T, N>();
        return member_info.field_name_hash;
    }

    template<class T, auto N>
    static constexpr auto getClassMemberPtr()
    {
        constexpr auto member_info = getClassMemberInfo<T, N>();
        return member_info.ptr;
    }

    template<class T>
    static constexpr auto getClassMemberSize()
    {
        constexpr auto meta_class = getClassMetaInfo<T>();
        return std::tuple_size_v<decltype(meta_class.member_info_tuple)>;
    }

    template<class T, auto N>
    static auto& getClassMemberValueRef(T& t)
    {
        constexpr auto member_ptr = getClassMemberPtr<T, N>();
        auto&          ref        = t.*member_ptr;
        return ref;
    }

    template<class T, auto N>
    static const auto& getClassMemberValueRef(const T& t)
    {
        constexpr auto member_ptr = getClassMemberPtr<T, N>();
        const auto&    ref        = t.*member_ptr;
        return ref;
    }

    template<class T>
    static constexpr size_t getClassMemberIndex(size_t field_hash)
    {
        constexpr auto meta_class = getClassMetaInfo<T>();
        size_t         index      = 0;
        find_if_tuple(meta_class.member_info_tuple,
                      [field_hash, &index](const auto& field_info, size_t idx) constexpr -> bool
                      {
                          if(field_info.field_name_hash != field_hash)
                              return false;

                          if constexpr(is_member_ptr<decltype(field_info)>())
                          {
                              index = idx;
                              return true;
                          }
                      });

        return index;
    }
#define GET_CLASS_MEMBER_INDEX(ClassT, FieldName) static_reflection_v2::getClassMemberIndex<ClassT>(FieldName##_HASH)

    static size_t make_string_hash(const std::string& str)
    {
        return hash::MurmurHash3::shash(str.c_str(), str.size(), 0);
    }

} // end namespace static_reflection_v2

namespace static_reflection_v2
{
    template<typename T, typename Fn>
    inline constexpr void ForEachField(T&& value, Fn&& fn)
    {
        constexpr auto meta_class = getClassMetaInfo<T>();
        static_assert(getClassMemberSize<T>() != 0,
                      "MetaClass<T>() for type T should be specialized to return "
                      "FieldSchema tuples, like ((&T::field, field_name), ...)");
        for_each_tuple(meta_class.member_info_tuple,
                       [&fn, &value](const auto& field_info) constexpr
                       {
                           if constexpr(is_member_ptr<decltype(field_info)>())
                           {
                               fn(field_info, value.*(field_info.ptr));
                           }
                           else if constexpr(is_member_ptr_tag<decltype(field_info)>())
                           {
                               fn(field_info, value.*(field_info.ptr), field_info.tag);
                           }
                           else if constexpr(is_member_ptr_func<decltype(field_info)>())
                           {
                               field_info.func(field_info, value.*(field_info.ptr));
                           }
                       });
    }

    template<typename T, typename Fn>
    inline constexpr void FindInField(T&& value, size_t field_hash, Fn&& fn)
    {
        constexpr auto meta_class = getClassMetaInfo<T>();
        static_assert(getClassMemberSize<T>() != 0,
                      "MetaClass<T>() for type T should be specialized to return "
                      "FieldSchema tuples, like ((&T::field, field_name), ...)");
        find_if_tuple_index(meta_class.member_info_tuple,
                      [&fn, &value, field_hash](const auto& field_info, size_t idx) constexpr -> bool
                      {
                          if(field_info.field_name_hash != field_hash)
                              return false;

                          if constexpr(is_member_ptr<decltype(field_info)>())
                          {
                              return fn(field_info, value.*(field_info.ptr));
                          }
                          else if constexpr(is_member_ptr_tag<decltype(field_info)>())
                          {
                              return fn(field_info, value.*(field_info.ptr), field_info.tag);
                          }
                          else if constexpr(is_member_ptr_func<decltype(field_info)>())
                          {
                              return fn(field_info, value.*(field_info.ptr), field_info.func);
                          }

                          return false;
                      });
    }

} // namespace static_reflection_v2

#endif /* STATICREFLECTIONV2_H */
