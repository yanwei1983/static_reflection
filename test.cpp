#include <cstdint>
#include <string>

#include "static_reflection.h"
#include "tinyxml2/tinyxml2.h"
#include  <functional>

struct ElfHash_tag {} constexpr ELF_HASH_TAG;
struct NameHash_tag {} constexpr NAME_HASH_TAG;

typedef struct ActionFlowOut
{
	int iOutCnt;
	int aiOutID[100];
}ACTIONFLOWOUT;

typedef struct ActionFlowLCast
{
    int iWaitTimeQianYao;
    int iWaitTimeCast;
    int iWaitTimeFinish;
    int iWaitTimeMoveQianYao;
    ACTIONFLOWOUT stOnStartOut;
    ACTIONFLOWOUT stOnQianYaoOut;
    ACTIONFLOWOUT stOnCastOut;
    ACTIONFLOWOUT stOnEndOut;
}ACTIONFLOWLCAST;

void a(tinyxml2::XMLElement*, int*)
{

}

constexpr auto FUNCCC = [](tinyxml2::XMLElement*, int* field)->void
{
	*field = 1;
};






DEFINE_STRUCT_SCHEMA(ActionFlowLCast,
                     DEFINE_STRUCT_FIELD_BIND("breakTime", 
							BIND_FIELD_TAG(iWaitTimeQianYao, ELF_HASH_TAG),
							BIND_FIELD(iWaitTimeMoveQianYao)),
                     DEFINE_STRUCT_FIELD_TAG(iWaitTimeCast, "castTime", ELF_HASH_TAG),
         			 DEFINE_STRUCT_FIELD_TAG(iWaitTimeFinish, "endTime", FUNCCC),
                     
                     DEFINE_STRUCT_FIELD(stOnStartOut, "onStart"),
                     DEFINE_STRUCT_FIELD(stOnQianYaoOut, "onBreak"),
                     DEFINE_STRUCT_FIELD(stOnCastOut, "onCast"),
                     DEFINE_STRUCT_FIELD(stOnEndOut, "onEnd"));


typedef union
{
    ACTIONFLOWLCAST stCast;

} ActionFlowNodeOneUnion;





template<class T>
void xmlElement_to_struct(tinyxml2::XMLElement* pE, T& refStruct);

template<class FieldType >
void xml_value_to_field(tinyxml2::XMLElement* pVarE, FieldType* field)
{
	xmlElement_to_struct(pVarE, *field);
}

template<>
void xml_value_to_field(tinyxml2::XMLElement* pVarE, uint64_t* field)
{
	pVarE->QueryAttribute("val", (int64_t*)field);
}

template<>
void xml_value_to_field(tinyxml2::XMLElement* pVarE, unsigned* field)
{
	pVarE->QueryAttribute("val", field);
}

template<>
void xml_value_to_field(tinyxml2::XMLElement* pVarE, int* field)
{
	pVarE->QueryAttribute("val", field);
}


void xml_value_to_field(tinyxml2::XMLElement* pVarE, int* field, ElfHash_tag tag)
{
	pVarE->QueryAttribute("val", field);
}


void xml_value_to_field(tinyxml2::XMLElement* pVarE, int* field, NameHash_tag tag)
{
	pVarE->QueryAttribute("val", field);
}

void xml_value_to_field(tinyxml2::XMLElement* pVarE, int* field, std::function<void(tinyxml2::XMLElement*, int*)> after_func)
{
	pVarE->QueryAttribute("val", field);
	after_func(pVarE, field);
}



template<>
void xml_value_to_field(tinyxml2::XMLElement* pVarE, int64_t* field)
{
	pVarE->QueryAttribute("val", field);
}

template<>
void xml_value_to_field(tinyxml2::XMLElement* pVarE, bool* field)
{
	pVarE->QueryAttribute("val", field);
}

template<>
void xml_value_to_field(tinyxml2::XMLElement* pVarE, double* field)
{
	pVarE->QueryAttribute("val", field);
}

template<>
void xml_value_to_field(tinyxml2::XMLElement* pVarE, float* field)
{
	pVarE->QueryAttribute("val", field);
}

template<>
void xml_value_to_field(tinyxml2::XMLElement* pVarE, std::string* field)
{
	const char* pVal = pVarE->Attribute("val");
	if (pVal)
	{
		*field = pVal;
	}
}

template<>
void xml_value_to_field(tinyxml2::XMLElement* pVarE, ActionFlowOut* field)
{
	field->iOutCnt = 0;
}


struct ForEachXMLLambda
{
    tinyxml2::XMLElement* pVarE;
	const char* field_name;
	std::size_t field_name_hash;
    template<typename FieldInfo, typename Field>
    bool operator()(FieldInfo&& this_field_info, Field&& this_field) const
    {
		printf("test:%s\n", std::get<0>(this_field_info) );
		if (field_name_hash != std::get<1>(this_field_info))
			return false;
		printf("vist:%s\n", std::get<0>(this_field_info));
		xml_value_to_field(pVarE, &this_field);
		return true;
    }

	template<typename FieldInfo, typename Field, typename Tag>
	bool operator()(FieldInfo&& this_field_info, Field&& this_field, Tag&& tag) const
	{
		printf("test:%s\n", std::get<0>(this_field_info));
		if (field_name_hash != std::get<1>(this_field_info))
			return false;
		printf("vist:%s\n", std::get<0>(this_field_info));
		xml_value_to_field(pVarE, &this_field, std::forward<Tag>(tag) );
		return true;
	}
};


template<class T>
void xmlElement_to_struct(tinyxml2::XMLElement* pE, T& refStruct)
{
	tinyxml2::XMLElement* pVarE = pE->FirstChildElement();
	while (pVarE != NULL)
	{
		const char* pStrName = pVarE->Attribute("name");
		if (pStrName != NULL)
		{
			std::string field_name = pStrName;
			
			FindInField(refStruct, ForEachXMLLambda{ pVarE, field_name.c_str(), hash::MurmurHash3::shash(field_name.c_str(), field_name.size(), 0) });
		}


		pVarE = pVarE->NextSiblingElement();
	}

}


int main(int argc, char *argv[] )
{
	const char* pstrFileName = argv[1];

	tinyxml2::XMLDocument doc;
	if(doc.LoadFile(pstrFileName) != tinyxml2::XMLError::XML_SUCCESS)
	{
		return -1;
	}

	auto pRootE = doc.FirstChildElement("Root");
	if(pRootE == NULL)
	{
		return -1;
	}

	auto pNodesE = pRootE->FirstChildElement("Nodes");
	if(pNodesE == NULL)
	{
		return -1;
	}

	auto pNodeE = pNodesE->FirstChildElement();
	while(pNodeE)
	{

		std::string struct_name = pNodeE->Name();
		//maybe find in map / global
		ActionFlowNodeOneUnion testNode;
		xmlElement_to_struct(pNodeE, testNode.stCast);


		pNodeE = pNodeE->NextSiblingElement();
	}
}
