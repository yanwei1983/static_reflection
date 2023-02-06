#ifndef TUPLEHELPER_H
#define TUPLEHELPER_H

#include <optional>
#include <tuple>
#include <type_traits>

#if(defined(_MSVC_LANG) && _MSVC_LANG < 201402L) || ((!defined(_MSVC_LANG)) && __cplusplus < 201402L)
namespace std
{
    template<bool B, class T = void>
    using enable_if_t = typename enable_if<B, T>::type;

    template<class T>
    using decay_t = typename decay<T>::type;

    template<typename T, T... ints>
    struct integer_sequence
    {
    };

    template<typename T, T N, typename = void>
    struct make_integer_sequence_impl
    {
        template<typename>
        struct tmp;

        template<T... Prev>
        struct tmp<integer_sequence<T, Prev...>>
        {
            using type = integer_sequence<T, Prev..., N - 1>;
        };

        using type = typename tmp<typename make_integer_sequence_impl<T, N - 1>::type>::type;
    };

    template<typename T, T N>
    struct make_integer_sequence_impl<T, N, typename std::enable_if<N == 0>::type>
    {
        using type = integer_sequence<T>;
    };

    template<typename T, T N>
    using make_integer_sequence = typename make_integer_sequence_impl<T, N>::type;

    template<size_t... _Vals>
    using index_sequence = integer_sequence<size_t, _Vals...>;

    template<size_t _Size>
    using make_index_sequence = make_integer_sequence<size_t, _Size>;

    // TEMPLATE CLASS _Cat_base
    template<bool _Val>
    struct _Cat_base : integral_constant<bool, _Val>
    { // base class for type predicates
    };

    template<class _Ty>
    struct is_null_pointer : _Cat_base<is_same<typename remove_cv<_Ty>::type, nullptr_t>::value>
    { // determine whether _Ty is nullptr_t
    };
} // namespace std
#endif // #if __cplusplus != 201402L

//////////////////////////////////////////////////////////////////////////////////////////////////////
template<class tuple_type, class ftype>
constexpr decltype(auto) for_each_tuple(tuple_type&& tuple, ftype&& f)
{
    return []<std::size_t... I>(tuple_type && tuple, ftype && f, std::index_sequence<I...>)
    {
        (f(std::get<I>(tuple)), ...);
        return f;
    }
    (std::forward<tuple_type>(tuple),
     std::forward<ftype>(f),
     std::make_index_sequence<std::tuple_size<std::remove_reference_t<tuple_type>>::value>{});
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
template<class tuple_type, class ftype>
constexpr decltype(auto) for_each_tuple_index(tuple_type&& tuple, ftype&& f)
{
    return []<std::size_t... I>(tuple_type && tuple, ftype && f, std::index_sequence<I...>)
    {
        (f(std::get<I>(tuple), std::integral_constant<std::size_t, I>()), ...);
        return f;
    }
    (std::forward<tuple_type>(tuple),
     std::forward<ftype>(f),
     std::make_index_sequence<std::tuple_size<std::remove_reference_t<tuple_type>>::value>{});
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
template<class tuple_type, class ftype>
constexpr decltype(auto) find_if_tuple(tuple_type&& tuple, ftype&& f)
{
    return []<std::size_t... I>(tuple_type && tuple, ftype && f, std::index_sequence<I...>)->bool
    {
        return (f(std::get<I>(tuple)) || ...);
    }
    (std::forward<tuple_type>(tuple),
     std::forward<ftype>(f),
     std::make_index_sequence<std::tuple_size<std::remove_reference_t<tuple_type>>::value>{});
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
template<class tuple_type, class ftype>
constexpr decltype(auto) find_if_tuple_index(tuple_type&& tuple, ftype&& f)
{
    return []<std::size_t... I>(tuple_type && tuple, ftype && f, std::index_sequence<I...>)->bool
    {
        return (f(std::get<I>(tuple), std::integral_constant<std::size_t, I>()) || ...);
    }
    (std::forward<tuple_type>(tuple),
     std::forward<ftype>(f),
     std::make_index_sequence<std::tuple_size<std::remove_reference_t<tuple_type>>::value>{});
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

template<class... Ts>
struct overloaded_t : Ts...
{
    using Ts::operator()...;
};
template<class... Ts>
overloaded_t(Ts...) -> overloaded_t<Ts...>;

/*
overloaded_t overload
{
    [&fn, &value, &field_name_tuple](auto member_ptr)
    {
        // invoke(fn, FieldName, MemberPtr);
        fn(field_name_tuple, value.*(member_ptr));
    },
    [&fn, &value, &field_name_tuple](auto member_ptr, auto tag)
    {
        // invoke(fn, FieldName, MemberPtr, tag);
        fn(field_name_tuple, value.*(member_ptr), tag);
    }
};

std::apply(overload, member_ptr_tuple);
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////

template<class Tuple>
struct unpack_warp_t
{
    Tuple&& tuple;
    using count   = std::tuple_size<std::remove_reference_t<Tuple>>;
    using indexes = std::make_index_sequence<count{}>;

    template<class F, std::size_t... Is>
    decltype(auto) _unpacker(F&& f, std::index_sequence<Is...>)
    {
        return f(std::get<Is>(std::forward<Tuple>(tuple))...);
    }

    template<class F>
    decltype(auto) unpack(F&& f)
    {
        return _unpacker(std::forward<F>(f), indexes{});
    }
};

template<class Tuple, class F>
decltype(auto) unpack(Tuple&& tuple, F&& f)
{
    unpack_warp_t<Tuple> unpack_warp{std::forward<Tuple>(tuple)};
    return unpack_warp.unpack(std::forward<F>(f));
}

/*
for(const auto& pair_v : map_val)
{
    unpack(pair_v, [](auto& x, auto&y)
    {
    });
}
*/

template<class... Args>
auto make_option_tuple(Args&&... args)
{
    auto tuple = std::make_tuple(std::forward<Args>(args)...);
    return std::optional<decltype(tuple)>(std::move(tuple));
}

#endif /* TUPLEHELPER_H */
