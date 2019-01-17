// Static Reflection by BOT Man, 2018

#ifndef STATIC_REFLECTION_H_
#define STATIC_REFLECTION_H_

#include <tuple>
#include <type_traits>


#if (defined(_MSVC_LANG) && _MSVC_LANG < 201402L) || ((!defined(_MSVC_LANG)) && __cplusplus < 201402L)
namespace std
{
	template< bool B, class T = void >
	using enable_if_t = typename enable_if<B,T>::type;

	template< class T >
	using decay_t = typename decay<T>::type;

	template <typename T, T... ints>
	struct integer_sequence
	{ };

	template <typename T, T N, typename = void>
	struct make_integer_sequence_impl
	{
		template <typename>
		struct tmp;

		template <T... Prev>
		struct tmp<integer_sequence<T, Prev...>>
		{
			using type = integer_sequence<T, Prev..., N-1>;
		};

		using type = typename tmp<typename make_integer_sequence_impl<T, N-1>::type>::type;
	};

	template <typename T, T N>
	struct make_integer_sequence_impl<T, N, typename std::enable_if<N==0>::type>
	{ using type = integer_sequence<T>; };

	template <typename T, T N>
	using make_integer_sequence = typename make_integer_sequence_impl<T, N>::type;


	template<size_t... _Vals>
	using index_sequence = integer_sequence<size_t, _Vals...>;

	template<size_t _Size>
	using make_index_sequence = make_integer_sequence<size_t, _Size>;


	// TEMPLATE CLASS _Cat_base
	template<bool _Val>
	struct _Cat_base
		: integral_constant<bool, _Val>
	{	// base class for type predicates
	};

	template<class _Ty>
	struct is_null_pointer
		: _Cat_base<is_same<typename remove_cv<_Ty>::type, nullptr_t>::value>
	{	// determine whether _Ty is nullptr_t
	};
}
#endif //#if __cplusplus != 201402L


namespace hash
{
	constexpr uint32_t djb2a(const char *s, uint32_t h = 5381)
	{
		return !*s ? h : djb2a(s + 1, 33 * h ^ (uint8_t)*s);
	}

	constexpr uint32_t fnv1a(const char *s, uint32_t h = 0x811C9DC5) 
	{
		return !*s ? h : fnv1a(s + 1, (h ^ (uint8_t)*s) * 0x01000193);
	}

	constexpr uint32_t CRC32_TABLE[] =
	{
		0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC, 0x76DC4190, 0x6B6B51F4, 0x4DB26158, 0x5005713C,
		0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C, 0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C };
	constexpr uint32_t crc32(const char *s, uint32_t h = ~0) {
#define CRC4(c, h) (CRC32_TABLE[((h) & 0xF) ^ (c)] ^ ((h) >> 4))
		return !*s ? ~h : crc32(s + 1, CRC4((uint8_t)*s >> 4, CRC4((uint8_t)*s & 0xF, h)));
#undef CRC4
	}

	namespace MurmurHash3 
	{
		constexpr uint32_t rotl(uint32_t x, int8_t r) 
		{
			return (x << r) | (x >> (32 - r));
		}
		constexpr uint32_t kmix(uint32_t k) 
		{
			return rotl(k * 0xCC9E2D51, 15) * 0x1B873593;
		}
		constexpr uint32_t hmix(uint32_t h, uint32_t k) 
		{
			return rotl(h ^ kmix(k), 13) * 5 + 0xE6546B64;
		}
		constexpr uint32_t shlxor(uint32_t x, int8_t l) 
		{
			return (x >> l) ^ x;
		}
		constexpr uint32_t fmix(uint32_t h) 
		{
			return shlxor(shlxor(shlxor(h, 16) * 0x85EBCA6B, 13) * 0xC2B2AE35, 16);
		}
		constexpr uint32_t body(const char *s, size_t n, uint32_t h) 
		{
			return n < 4 ? h : body(s + 4, n - 4, hmix(h, s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24)));
		}
		constexpr uint32_t tail(const char *s, size_t n, uint32_t h) 
		{
			return h ^ kmix(n == 3 ? s[0] | (s[1] << 8) | (s[2] << 16) : n == 2 ? s[0] | (s[1] << 8) : n == 1 ? s[0] : 0);
		}
		constexpr uint32_t shash(const char *s, size_t n, uint32_t seed) 
		{
			return fmix(tail(s + (n & ~3), n & 3, body(s, n, seed)) ^ n);
		}
	} // namespace MurmurHash3



};

constexpr uint32_t operator"" _H(const char *s, size_t size) 
{
	return hash::MurmurHash3::shash(s, size, 0);
}

namespace detail {

	template <typename T>
	struct is_field_pointer : std::false_type {};

	template <typename C, typename T>
	struct is_field_pointer<T C::*> : std::true_type {};



	template <typename Lambda, typename Tuple>
	inline constexpr void ForEachStructSchemaImpl(Tuple&& tuple,
		Lambda&& lambda,
		std::index_sequence<>)
	{
	}

	template <typename Lambda, typename Tuple, std::size_t first, std::size_t... is>
	inline constexpr void ForEachStructSchemaImpl(Tuple&& tuple,
		Lambda&& lambda,
		std::index_sequence<first, is...>)
	{
		lambda(std::get<first>(std::forward<Tuple>(tuple)));
		ForEachStructSchemaImpl(std::forward<Tuple>(tuple), std::forward<Lambda>(lambda), std::index_sequence<is...>{});
	}

	template <typename Lambda, typename Tuple>
	inline constexpr void ForEachStructSchema(Tuple&& tuple, Lambda&& lambda)
	{
		ForEachStructSchemaImpl(
			std::forward<Tuple>(tuple), std::forward<Lambda>(lambda),
			std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
	}


	template<typename Value, typename Func>
	struct ForEachStructSchemaLambda
	{
		Value& value;
		const Func& fn;


		template<typename Info, typename Tuple>
		void invoke_one_impl(Info&& info, Tuple&& t, std::index_sequence<>) const
		{
			//invoke(FieldName, MemberPtr);
			fn(std::forward<Info>(info), value.*(std::get<0>(std::forward<Tuple>(t))) );
		}
		template<typename Info, typename Tuple, std::size_t... Index>
		void invoke_one_impl(Info&& info, Tuple&& t, std::index_sequence<Index...>) const
		{
			//invoke(FieldName, MemberPtr,Tag);
			//invoke(FieldName. MemberPtr,Func);
			fn(std::forward<Info>(info), value.*(std::get<0>(std::forward<Tuple>(t))), std::get<Index + 1>(std::forward<Tuple>(t))...);
		}

		template<typename Info, typename Tuple>
		void invoke_one(Info&& info, Tuple&& t) const
		{
			constexpr auto size = std::tuple_size<typename std::decay<Tuple>::type>::value;
			invoke_one_impl(std::forward<Info>(info), std::forward<Tuple>(t), std::make_index_sequence<size - 1>{});
		}

		template<typename Info, typename Tuple>
		void invoke_impl(Info&& info, Tuple&& t, std::index_sequence<>) const {}
		template<typename Info, typename Tuple, std::size_t first, std::size_t... is>
		void invoke_impl(Info&& info, Tuple&& t, std::index_sequence<first, is...>) const
		{
			//invoke(FieldName, std::tuple<MemberPtr>);
			//invoke(FieldName, std::tuple<MemberPtr,Tag>);
			//invoke(FieldName. std::tuple<MemberPtr,Func>);
			invoke_one(std::forward<Info>(info), std::get<first + 1>(std::forward<Tuple>(t)));
			invoke_impl(std::forward<Info>(info), std::forward<Tuple>(t), std::index_sequence<is...>{});
		}

		template<typename Info, typename Tuple>
		void operator()(Tuple&& field_schema) const
		{
			using FieldSchema = std::decay_t<decltype(field_schema)>;
			constexpr auto size_tuple = std::tuple_size<FieldSchema>::value;
			static_assert(size_tuple >= 2, "FieldSchema<T>() must have 1 field");
			//tuple like std::tuple<FieldName, std::tuple<MemberPtr>, std::tuple<MemberPtr,Tag> , std::tuple<MemberPtr,Func> >
			invoke_impl(std::get<0>(std::forward<Tuple>(field_schema)), std::forward<Tuple>(field_schema), std::make_index_sequence<size_tuple-1>{});
		}




	};


	template <typename Lambda, typename Tuple>
	inline constexpr bool FindInStructSchemaImpl(Tuple&& tuple,
		Lambda&& lambda,
		std::index_sequence<>)
	{
		return false;
	}

	template <typename Lambda, typename Tuple, std::size_t first, std::size_t... is>
	inline constexpr bool FindInStructSchemaImpl(Tuple&& tuple,
		Lambda&& lambda,
		std::index_sequence<first, is...>)
	{
		//if any lambda return true, return turel
		return lambda(std::get<first>(std::forward<Tuple>(tuple))) || FindInStructSchemaImpl(std::forward<Tuple>(tuple), std::forward<Lambda>(lambda), std::index_sequence<is...>{});
	}

	template <typename Lambda, typename Tuple>
	inline constexpr bool FindInStructSchema(Tuple&& tuple, Lambda&& lambda)
	{
		return FindInStructSchemaImpl(
			std::forward<Tuple>(tuple), std::forward<Lambda>(lambda),
			std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
	}

	template<typename Value, typename Func>
	struct FindInStructSchemaLambda
	{
		Value& value;
		const Func& fn;


		template<typename Info, typename Tuple>
		bool invoke_one_impl(Info&& info, Tuple&& t, std::index_sequence<>) const
		{
			//invoke(FieldName, MemberPtr);
			return fn(std::forward<Info>(info), value.*(std::get<0>(std::forward<Tuple>(t))));
		}

		template<typename Info, typename Tuple, std::size_t... Index>
		bool invoke_one_impl(Info&& info, Tuple&& t, std::index_sequence<Index...>) const
		{
			//invoke(FieldName, MemberPtr,Tag);
			//invoke(FieldName. MemberPtr,Func);
			return fn(std::forward<Info>(info), value.*(std::get<0>(std::forward<Tuple>(t))), std::get<Index + 1>(std::forward<Tuple>(t))...);
		}

		template<typename Info, typename Tuple>
		bool invoke_one(Info&& info, Tuple&& t) const
		{
			constexpr auto size = std::tuple_size<typename std::decay<Tuple>::type>::value;
			return invoke_one_impl(std::forward<Info>(info), std::forward<Tuple>(t), std::make_index_sequence<size - 1>{});
		}

		template<typename Info, typename Tuple>
		bool invoke_impl(Info&& info, Tuple&& t, std::index_sequence<>) const { return false;}
		template<typename Info, typename Tuple, std::size_t first, std::size_t... is>
		bool invoke_impl(Info&& info, Tuple&& t, std::index_sequence<first, is...>) const
		{
			//invoke(FieldName, std::tuple<MemberPtr>);
			//invoke(FieldName, std::tuple<MemberPtr,Tag>);
			//invoke(FieldName. std::tuple<MemberPtr,Func>);
			bool bInvoke = invoke_one(std::forward<Info>(info), std::get<first + 1>(std::forward<Tuple>(t)));

			//first invoke all, then all result ||;
			return invoke_impl(std::forward<Info>(info), std::forward<Tuple>(t), std::index_sequence<is...>{}) || bInvoke;
		}

		template<typename Tuple>
		bool operator()(Tuple&& field_schema) const
		{
			using FieldSchema = std::decay_t<decltype(field_schema)>;
			constexpr auto size_tuple = std::tuple_size<FieldSchema>::value;
			static_assert(size_tuple >= 2, "FieldSchema<T>() must have 1 field");
			//tuple like std::tuple<FieldName, std::tuple<MemberPtr>, std::tuple<MemberPtr,Tag> , std::tuple<MemberPtr,Func> >
			return invoke_impl(std::get<0>(std::forward<Tuple>(field_schema)), std::forward<Tuple>(field_schema), std::make_index_sequence<size_tuple-1>{});
		}




	};




}  // namespace detail

template <typename T>
struct StructSchema
{
	static inline constexpr void GetSchema()
	{
	}
};

#define DEFINE_STRUCT_SCHEMA(ThisStruct, ...)    		\
template <>                                       		\
struct StructSchema<ThisStruct>				  			\
{														\
	using _Struct = ThisStruct;							\
    template<typename E = void> 						\
	static inline constexpr auto GetSchema() ->decltype(std::make_tuple(__VA_ARGS__))			\
	{													\
		return std::make_tuple(__VA_ARGS__); 			\
	}													\
};

#define MAKE_FIELD_INFO(FieldName)\
  std::make_tuple(FieldName,FieldName##_H)

#define BIND_FIELD(StructField) \
  std::make_tuple(&_Struct::StructField)

#define BIND_FIELD_TAG(StructField, Tag) \
  std::make_tuple(&_Struct::StructField, Tag)

#define BIND_FIELD_FUNC(StructField, Func) \
  std::make_tuple(&_Struct::StructField, Func)

#define DEFINE_STRUCT_FIELD_BIND(FieldName, ...) \
  std::make_tuple(MAKE_FIELD_INFO(FieldName), __VA_ARGS__ )

#define DEFINE_STRUCT_FIELD(StructField, FieldName) \
  std::make_tuple(MAKE_FIELD_INFO(FieldName), BIND_FIELD(StructField) )

#define DEFINE_STRUCT_FIELD_TAG(StructField, FieldName, Tag) \
  std::make_tuple(MAKE_FIELD_INFO(FieldName), BIND_FIELD_TAG(StructField, Tag) )

#define DEFINE_STRUCT_FIELD_FUNC(StructField, FieldName, Func) \
  std::make_tuple(MAKE_FIELD_INFO(FieldName), BIND_FIELD_FUNC(StructField, Func) )


//every Field Will MakeTuple Like
//std::tuple<FieldName, std::tuple<MemberPtr>, std::tuple<MemberPtr,Tag> , std::tuple<MemberPtr,Func> >



template <typename T, typename Fn>
inline constexpr void ForEachField(T&& value, Fn&& fn)
{
	constexpr auto struct_schema = StructSchema<std::decay_t<T>>::GetSchema();
	static_assert(std::tuple_size<decltype(struct_schema)>::value != 0,
		"StructSchema<T>() for type T should be specialized to return "
		"FieldSchema tuples, like ((&T::field, field_name), ...)");
	detail::ForEachStructSchemaLambda<T, Fn> lambda{ std::forward<T>(value), std::forward<Fn>(fn) };
	detail::ForEachStructSchema(struct_schema, std::move(lambda));
}

template <typename T, typename Fn>
inline constexpr void FindInField(T&& value, Fn&& fn)
{
	constexpr auto struct_schema = StructSchema<std::decay_t<T>>::GetSchema();
	static_assert(std::tuple_size<decltype(struct_schema)>::value != 0,
		"StructSchema<T>() for type T should be specialized to return "
		"FieldSchema tuples, like ((&T::field, field_name), ...)");
	detail::FindInStructSchemaLambda<T, Fn> lambda{ std::forward<T>(value), std::forward<Fn>(fn) };
	detail::FindInStructSchema(struct_schema, std::move(lambda));
}

#endif  // STATIC_REFLECTION_H_
