#ifndef JSONTOSTRUCT_H
#define JSONTOSTRUCT_H

#include "StaticHash.h"
#include "StaticReflectionV2.h"
#include "json.hpp"
#include "type_traits_ext.h"

// forward decal
template<class T>
inline void json_to_struct(const nlohmann::json& json, T& refStruct);

template<class FieldType>
inline void json_to_field(const nlohmann::json& json, FieldType* field)
{
    if constexpr(is_invokable<decltype(&nlohmann::json::get<FieldType>), FieldType, nlohmann::json>::value)
    {
        *field = json.get<FieldType>();
    }
    else
    {
        json_to_struct(json, *field);
    }
}

template<class T>
inline void json_to_struct(const nlohmann::json& json, T& refStruct)
{
    for(const auto& [field_name, v]: json.items())
    {
        auto field_name_hash = static_reflection_v2::make_string_hash(field_name);
        static_reflection_v2::FindInField(refStruct, field_name_hash, 
        [&json](auto field_info, auto field) 
        {
            json_to_field(json, &field);
            return true;
        });
    }
}

#endif /* JSONTOSTRUCT_H */
