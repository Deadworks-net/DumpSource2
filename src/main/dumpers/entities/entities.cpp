/**
 * =============================================================================
 * DumpSource2
 * Copyright (C) 2026 ValveResourceFormat Contributors
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "entities.h"
#include "globalvariables.h"
#include "modules.h"
#include "utils/module.h"
#include "datamap.h"
#include "entity2/entityclass.h"
#include "schemasystem/schematypes.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <map>
#include <vector>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Dumpers::Entities
{

std::vector<std::pair<int, const char*>> g_flagMap{
	{FTYPEDESC_GLOBAL, "global"},
	{FTYPEDESC_SAVE, "save"},
	{FTYPEDESC_KEY, "key"},
	{FTYPEDESC_INPUT, "input"},
	{FTYPEDESC_OUTPUT, "output"},
	{FTYPEDESC_PTR, "ptr"},
	{FTYPEDESC_PROCEDURAL_KEYFIELD, "procedural_keyfield"},
	{FTYPEDESC_ENUM, "enum"},
	{FTYPEDESC_REMOVED_KEYFIELD, "removed_keyfield"},
	{FTYPEDESC_WAS_INPUT, "was_input"},
	{FTYPEDESC_WAS_OUTPUT, "was_output"}
};

#define FIELD_TYPE_NAME(type) \
	case type:                \
		return #type;

const char* GetFieldTypeName(fieldtype_t type)
{
	switch (type)
	{
		FIELD_TYPE_NAME(FIELD_VOID)
		FIELD_TYPE_NAME(FIELD_FLOAT32)
		FIELD_TYPE_NAME(FIELD_STRING)
		FIELD_TYPE_NAME(FIELD_VECTOR)
		FIELD_TYPE_NAME(FIELD_QUATERNION)
		FIELD_TYPE_NAME(FIELD_INT32)
		FIELD_TYPE_NAME(FIELD_BOOLEAN)
		FIELD_TYPE_NAME(FIELD_INT16)
		FIELD_TYPE_NAME(FIELD_CHARACTER)
		FIELD_TYPE_NAME(FIELD_COLOR32)
		FIELD_TYPE_NAME(FIELD_EMBEDDED)
		FIELD_TYPE_NAME(FIELD_CUSTOM)
		FIELD_TYPE_NAME(FIELD_CLASSPTR)
		FIELD_TYPE_NAME(FIELD_EHANDLE)
		FIELD_TYPE_NAME(FIELD_POSITION_VECTOR)
		FIELD_TYPE_NAME(FIELD_TIME)
		FIELD_TYPE_NAME(FIELD_TICK)
		FIELD_TYPE_NAME(FIELD_SOUNDNAME)
		FIELD_TYPE_NAME(FIELD_INPUT)
		FIELD_TYPE_NAME(FIELD_FUNCTION)
		FIELD_TYPE_NAME(FIELD_VMATRIX)
		FIELD_TYPE_NAME(FIELD_VMATRIX_WORLDSPACE)
		FIELD_TYPE_NAME(FIELD_MATRIX3X4_WORLDSPACE)
		FIELD_TYPE_NAME(FIELD_INTERVAL)
		FIELD_TYPE_NAME(FIELD_UNUSED)
		FIELD_TYPE_NAME(FIELD_VECTOR2D)
		FIELD_TYPE_NAME(FIELD_INT64)
		FIELD_TYPE_NAME(FIELD_VECTOR4D)
		FIELD_TYPE_NAME(FIELD_RESOURCE)
		FIELD_TYPE_NAME(FIELD_TYPEUNKNOWN)
		FIELD_TYPE_NAME(FIELD_CSTRING)
		FIELD_TYPE_NAME(FIELD_HSCRIPT)
		FIELD_TYPE_NAME(FIELD_VARIANT)
		FIELD_TYPE_NAME(FIELD_UINT64)
		FIELD_TYPE_NAME(FIELD_FLOAT64)
		FIELD_TYPE_NAME(FIELD_POSITIVEINTEGER_OR_NULL)
		FIELD_TYPE_NAME(FIELD_HSCRIPT_NEW_INSTANCE)
		FIELD_TYPE_NAME(FIELD_UINT32)
		FIELD_TYPE_NAME(FIELD_UTLSTRINGTOKEN)
		FIELD_TYPE_NAME(FIELD_QANGLE)
		FIELD_TYPE_NAME(FIELD_NETWORK_ORIGIN_CELL_QUANTIZED_VECTOR)
		FIELD_TYPE_NAME(FIELD_HMATERIAL)
		FIELD_TYPE_NAME(FIELD_HMODEL)
		FIELD_TYPE_NAME(FIELD_NETWORK_QUANTIZED_VECTOR)
		FIELD_TYPE_NAME(FIELD_NETWORK_QUANTIZED_FLOAT)
		FIELD_TYPE_NAME(FIELD_DIRECTION_VECTOR_WORLDSPACE)
		FIELD_TYPE_NAME(FIELD_QANGLE_WORLDSPACE)
		FIELD_TYPE_NAME(FIELD_QUATERNION_WORLDSPACE)
		FIELD_TYPE_NAME(FIELD_HSCRIPT_LIGHTBINDING)
		FIELD_TYPE_NAME(FIELD_V8_VALUE)
		FIELD_TYPE_NAME(FIELD_V8_OBJECT)
		FIELD_TYPE_NAME(FIELD_V8_ARRAY)
		FIELD_TYPE_NAME(FIELD_V8_CALLBACK_INFO)
		FIELD_TYPE_NAME(FIELD_UTLSTRING)
		FIELD_TYPE_NAME(FIELD_NETWORK_ORIGIN_CELL_QUANTIZED_POSITION_VECTOR)
		FIELD_TYPE_NAME(FIELD_HRENDERTEXTURE)
		FIELD_TYPE_NAME(FIELD_HPARTICLESYSTEMDEFINITION)
		FIELD_TYPE_NAME(FIELD_UINT8)
		FIELD_TYPE_NAME(FIELD_UINT16)
		FIELD_TYPE_NAME(FIELD_CTRANSFORM)
		FIELD_TYPE_NAME(FIELD_CTRANSFORM_WORLDSPACE)
		FIELD_TYPE_NAME(FIELD_HPOSTPROCESSING)
		FIELD_TYPE_NAME(FIELD_MATRIX3X4)
		FIELD_TYPE_NAME(FIELD_SHIM)
		FIELD_TYPE_NAME(FIELD_CMOTIONTRANSFORM)
		FIELD_TYPE_NAME(FIELD_CMOTIONTRANSFORM_WORLDSPACE)
		FIELD_TYPE_NAME(FIELD_ATTACHMENT_HANDLE)
		FIELD_TYPE_NAME(FIELD_AMMO_INDEX)
		FIELD_TYPE_NAME(FIELD_CONDITION_ID)
		FIELD_TYPE_NAME(FIELD_AI_SCHEDULE_BITS)
		FIELD_TYPE_NAME(FIELD_MODIFIER_HANDLE)
		FIELD_TYPE_NAME(FIELD_ROTATION_VECTOR)
		FIELD_TYPE_NAME(FIELD_ROTATION_VECTOR_WORLDSPACE)
		FIELD_TYPE_NAME(FIELD_HVDATA)
		FIELD_TYPE_NAME(FIELD_SCALE32)
		FIELD_TYPE_NAME(FIELD_STRING_AND_TOKEN)
		FIELD_TYPE_NAME(FIELD_ENGINE_TIME)
		FIELD_TYPE_NAME(FIELD_ENGINE_TICK)
		FIELD_TYPE_NAME(FIELD_WORLD_GROUP_ID)
		FIELD_TYPE_NAME(FIELD_GLOBALSYMBOL)
		FIELD_TYPE_NAME(FIELD_HNMGRAPHDEFINITION)
		default:
			return "FIELD_UNKNOWN";
	}
}

#undef FIELD_TYPE_NAME

bool IsInModule(const CModule& module, const void* ptr)
{
	return ptr >= module.m_base && ptr < (uint8_t*)module.m_base + module.m_size;
}

bool IsModuleString(const CModule& module, const char* str)
{
	for (auto it = str; IsInModule(module, it); it++)
	{
		if (!*it)
			return it != str;

		if (!isprint((unsigned char)*it))
			return false;
	}

	return false;
}

// Whatever else points at two strings, only a real class info has a C++ class
// name matching its own schema binding or datamap.
bool IsEntityClassInfo(const CModule& module, const CEntityClassInfo* info)
{
	if (!IsModuleString(module, info->m_pszClassname) || !IsModuleString(module, info->m_pszCPPClassname))
		return false;

	auto schemaBinding = info->m_pSchemaBinding;
	if (IsInModule(module, schemaBinding) && IsModuleString(module, schemaBinding->m_pszName) && !strcmp(schemaBinding->m_pszName, info->m_pszCPPClassname))
		return true;

	auto dataMap = info->m_pDataDescMap;
	return IsInModule(module, dataMap) && IsModuleString(module, dataMap->dataClassName) && !strcmp(dataMap->dataClassName, info->m_pszCPPClassname);
}

// Entity classes are only linked into the entity system once a game starts, but
// their class infos are statics that exist as soon as the module is loaded.
std::vector<const CEntityClassInfo*> FindEntityClasses(CModule& module)
{
	std::vector<const CEntityClassInfo*> classes;
	auto dataSection = module.GetSection(".data");

	if (!dataSection)
	{
		spdlog::error("Failed to find .data section in {}", module.m_pszModule);
		return classes;
	}

	// Statics without an initializer live past the section's raw data, so scan to the end of the module.
	auto end = (uint8_t*)module.m_base + module.m_size - sizeof(CEntityClassInfo);
	for (auto ptr = (uint8_t*)dataSection->m_pBase; ptr <= end; ptr += alignof(CEntityClassInfo))
	{
		auto info = reinterpret_cast<const CEntityClassInfo*>(ptr);
		if (IsEntityClassInfo(module, info))
			classes.push_back(info);
	}

	std::sort(classes.begin(), classes.end(), [](const CEntityClassInfo* a, const CEntityClassInfo* b) {
		return strcmp(a->m_pszClassname, b->m_pszClassname) < 0;
	});

	return classes;
}

// Keyvalues also come from embedded datamaps, such as the scene node's parentAttachmentName.
void CollectDataMaps(const datamap_t* dataMap, std::map<std::string, const datamap_t*>& dataMaps)
{
	for (; dataMap; dataMap = dataMap->baseMap)
	{
		auto [it, inserted] = dataMaps.emplace(dataMap->dataClassName, dataMap);

		if (!inserted)
		{
			if (it->second != dataMap)
				spdlog::warn("Skipping a second datamap named {}", dataMap->dataClassName);

			return;
		}

		for (int i = 0; i < dataMap->dataNumFields; i++)
		{
			const auto& field = dataMap->dataDesc[i];
			if (field.fieldType == FIELD_EMBEDDED && !(field.flags & FTYPEDESC_ENUM))
				CollectDataMaps(field.td, dataMaps);
		}
	}
}

json SerializeField(const typedescription_t& field)
{
	json j;
	j["name"] = field.fieldName;
	j["type"] = GetFieldTypeName(field.fieldType);

	// Procedural keyfields are parsed by code instead of being written to a member.
	if (!(field.flags & FTYPEDESC_PROCEDURAL_KEYFIELD))
		j["offset"] = field.fieldOffset;

	j["size"] = field.fieldSize;

	json flags = json::array();
	for (const auto& [value, name] : g_flagMap)
	{
		if (field.flags & value)
			flags.push_back(name);
	}

	if (flags.size())
		j["flags"] = std::move(flags);

	if (field.externalName)
		j["external_name"] = field.externalName;

	if (field.flags & FTYPEDESC_ENUM)
	{
		if (field.enumName)
			j["enum"] = field.enumName;
	}
	else if (field.fieldType == FIELD_EMBEDDED && field.td)
	{
		j["embedded"] = field.td->dataClassName;
	}

	return j;
}

json SerializeDataMap(const datamap_t* dataMap)
{
	json j;
	j["name"] = dataMap->dataClassName;

	if (dataMap->baseMap)
		j["base"] = dataMap->baseMap->dataClassName;

	json fields = json::array();
	for (int i = 0; i < dataMap->dataNumFields; i++)
	{
		const auto& field = dataMap->dataDesc[i];
		if (field.fieldName)
			fields.push_back(SerializeField(field));
	}

	if (fields.size())
		j["fields"] = std::move(fields);

	return j;
}

void Dump()
{
	auto server = std::find_if(Modules::allModules.begin(), Modules::allModules.end(), [](const CModule& module) {
		return !strcmp(module.m_pszModule, "server");
	});

	if (server == Modules::allModules.end())
	{
		spdlog::warn("Skipping entities as the server module is not loaded");
		return;
	}

	spdlog::info("Dumping entity classes");

	std::map<std::string, const datamap_t*> dataMaps;
	json classesArray = json::array();

	for (auto info : FindEntityClasses(*server))
	{
		json classObj;
		classObj["name"] = info->m_pszClassname;
		classObj["cpp_class"] = info->m_pszCPPClassname;

		if (IsModuleString(*server, info->m_pszDescription))
			classObj["description"] = info->m_pszDescription;

		if (info->m_pDataDescMap)
		{
			classObj["datamap"] = info->m_pDataDescMap->dataClassName;
			CollectDataMaps(info->m_pDataDescMap, dataMaps);
		}

		classesArray.push_back(std::move(classObj));
	}

	json dataMapsArray = json::array();
	for (const auto& [name, dataMap] : dataMaps)
		dataMapsArray.push_back(SerializeDataMap(dataMap));

	nlohmann::ordered_json root;
	root["generator"] = "https://github.com/ValveResourceFormat/DumpSource2";

	if (!Globals::sourceRevision.empty())
		root["revision"] = std::stoi(Globals::sourceRevision);

	if (!Globals::versionDate.empty())
		root["version_date"] = Globals::versionDate;

	if (!Globals::versionTime.empty())
		root["version_time"] = Globals::versionTime;

	root["classes"] = classesArray;
	root["datamaps"] = dataMapsArray;

	std::ofstream output(Globals::outputPath / "entities.json");
	output << root.dump(-1);
	output.close();

	spdlog::info("Wrote entities.json ({} classes, {} datamaps)", classesArray.size(), dataMapsArray.size());
}

} // namespace Dumpers::Entities