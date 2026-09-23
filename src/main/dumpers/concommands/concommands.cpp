/**
 * =============================================================================
 * DumpSource2
 * Copyright (C) 2024 ValveResourceFormat Contributors
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


// Yes this is shit, no we can't make it better.
#define _ALLOW_KEYWORD_MACROS 1
#define private public
#include <icvar.h>
#undef private
#undef _ALLOW_KEYWORD_MACROS

#include "concommands.h"
#include "interfaces.h"
#include "globalvariables.h"
#include <algorithm>
#include <fstream>
#include <vector>
#include <iostream>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Dumpers::ConCommands
{

#define MINMAXVALUEPRINT(typeName) \
	stream << " " << value->typeName; 												\
																														\
	bool hasMinValue = cvar.HasMin();													\
	bool hasMaxValue = cvar.HasMax();													\
																														\
	stream << " (";																						\
																														\
	if (hasMinValue)																					\
		stream << "min: " << conVarData->MinValue()->typeName;	\
																														\
	if (hasMaxValue)																					\
	{																													\
		if (hasMinValue)																				\
			stream << ", ";																				\
		stream << "max: " << conVarData->MaxValue()->typeName;		\
	}																													\
																														\
	if (hasMinValue || hasMaxValue)														\
		stream << ", ";																					\
																														\
	WriteFlags(cvar.GetFlags(), stream);											\
																														\
	stream << ")";

#define FCVAR_MISSING1	(1ull<<30)
#define FCVAR_MISSING2	(1ull<<31)

std::vector<std::pair<uint64_t, const char*>> g_flagMap{
	{FCVAR_LINKED_CONCOMMAND, "linked_concommand"},
	{FCVAR_DEVELOPMENTONLY, "developmentonly"},
	{FCVAR_GAMEDLL, "gamedll"},
	{FCVAR_CLIENTDLL, "clientdll"},
	{FCVAR_HIDDEN, "hidden"},
	{FCVAR_PROTECTED, "protected"},
	{FCVAR_SPONLY, "sponly"},
	{FCVAR_ARCHIVE, "archive"},
	{FCVAR_NOTIFY, "notify"},
	{FCVAR_USERINFO, "userinfo"},
	{FCVAR_REFERENCE, "reference"},
	{FCVAR_UNLOGGED, "unlogged"},
	{FCVAR_INITIAL_SETVALUE, "initial_setvalue"},
	{FCVAR_REPLICATED, "replicated"},
	{FCVAR_CHEAT, "cheat"},
	{FCVAR_PER_USER, "per_user"},
	{FCVAR_DEMO, "demo"},
	{FCVAR_DONTRECORD, "dontrecord"},
	{FCVAR_PERFORMING_CALLBACKS, "performing_Callbacks"},
	{FCVAR_RELEASE, "release"},
	{FCVAR_MENUBAR_ITEM, "menubar_item"},
	{FCVAR_COMMANDLINE_ENFORCED, "commandline_enforced"},
	{FCVAR_NOT_CONNECTED, "notconnected"},
	{FCVAR_VCONSOLE_FUZZY_MATCHING, "vconsole_fuzzy_matching"},
	{FCVAR_SERVER_CAN_EXECUTE, "server_can_execute"},
	{FCVAR_CLIENT_CAN_EXECUTE, "client_can_execute"},
	{FCVAR_SERVER_CANNOT_QUERY, "server_cannot_query"},
	{FCVAR_VCONSOLE_SET_FOCUS, "vconsole_set_focus"},
	{FCVAR_CLIENTCMD_CAN_EXECUTE, "clientcmd_can_execute"},
	{FCVAR_EXECUTE_PER_TICK, "execute_per_tick"},
	{FCVAR_MISSING1, "missing1"},
	{FCVAR_MISSING2, "missing2"},
	{FCVAR_DEFENSIVE, "defensive"}
};

void WriteFlags(uint64_t flags, std::ofstream& stream)
{
	bool found = false;
	for (const auto& [value, name] : g_flagMap)
	{
		if (flags & value)
		{
			stream << (found ? " " : "") << name;
			found = true;
		}
	}
}
json SerializeFlags(uint64_t flags)
{
	json flagsArray = json::array();
	for (const auto& [value, name] : g_flagMap)
	{
		if (flags & value)
			flagsArray.push_back(name);
	}

	return flagsArray;
}

const char* GetTypeName(EConVarType type)
{
	switch (type)
	{
	case EConVarType_Bool: return "bool";
	case EConVarType_Int16: return "int16";
	case EConVarType_UInt16: return "uint16";
	case EConVarType_Int32: return "int32";
	case EConVarType_UInt32: return "uint32";
	case EConVarType_Int64: return "int64";
	case EConVarType_UInt64: return "uint64";
	case EConVarType_Float32: return "float32";
	case EConVarType_Float64: return "float64";
	case EConVarType_String: return "string";
	case EConVarType_Color: return "color";
	case EConVarType_Vector2: return "vector2";
	case EConVarType_Vector3: return "vector3";
	case EConVarType_Vector4: return "vector4";
	case EConVarType_Qangle: return "qangle";
	default: return "unknown";
	}
}

// Floats keep the shortest form that reads back the same, as in convars.txt,
// instead of the nearest double (0.1 rather than 0.10000000149011612).
double ShortestDouble(float value)
{
	return std::stod(fmt::format("{}", value));
}

json SerializeValue(EConVarType type, const CVValue_t* value)
{
	switch (type)
	{
	case EConVarType_Bool: return value->m_bValue;
	case EConVarType_Int16: return value->m_i16Value;
	case EConVarType_UInt16: return value->m_u16Value;
	case EConVarType_Int32: return value->m_i32Value;
	case EConVarType_UInt32: return value->m_u32Value;
	case EConVarType_Int64: return value->m_i64Value;
	case EConVarType_UInt64: return value->m_u64Value;
	case EConVarType_Float32: return ShortestDouble(value->m_fl32Value);
	case EConVarType_Float64: return value->m_fl64Value;
	case EConVarType_String: return value->m_StringValue.m_pString ? value->m_StringValue.m_pString : "";
	case EConVarType_Color: return json::array({ value->m_clrValue.r(), value->m_clrValue.g(), value->m_clrValue.b(), value->m_clrValue.a() });
	case EConVarType_Vector2: return json::array({ ShortestDouble(value->m_vec2Value.x), ShortestDouble(value->m_vec2Value.y) });
	case EConVarType_Vector3: return json::array({ ShortestDouble(value->m_vec3Value.x), ShortestDouble(value->m_vec3Value.y), ShortestDouble(value->m_vec3Value.z) });
	case EConVarType_Vector4: return json::array({ ShortestDouble(value->m_vec4Value.x), ShortestDouble(value->m_vec4Value.y), ShortestDouble(value->m_vec4Value.z), ShortestDouble(value->m_vec4Value.w) });
	case EConVarType_Qangle: return json::array({ ShortestDouble(value->m_angValue.x), ShortestDouble(value->m_angValue.y), ShortestDouble(value->m_angValue.z) });
	default: return nullptr;
	}
}

json SerializeConVar(ConVarRefAbstract cvar)
{
	auto conVarData = cvar.GetConVarData();
	auto type = cvar.GetType();

	json j;
	j["name"] = cvar.GetName();
	j["type"] = GetTypeName(type);

	if (conVarData->HasDefaultValue())
		j["default"] = SerializeValue(type, conVarData->DefaultValue());

	if (cvar.HasMin())
		j["min"] = SerializeValue(type, conVarData->MinValue());

	if (cvar.HasMax())
		j["max"] = SerializeValue(type, conVarData->MaxValue());

	j["flags"] = SerializeFlags(cvar.GetFlags());

	if (cvar.HasHelpText())
		j["description"] = cvar.GetHelpText();

	return j;
}

json SerializeConCommand(ConCommandRef command)
{
	json j;
	j["name"] = command.GetName();
	j["flags"] = SerializeFlags(command.GetFlags());

	if (command.HasHelpText())
		j["description"] = command.GetHelpText();

	return j;
}

void WriteValueLine(ConVarRefAbstract cvar, std::ofstream& stream)
{
	auto conVarData = cvar.GetConVarData();
	auto value = cvar.GetConVarData()->DefaultValue();

	switch (cvar.GetType())
	{
	case EConVarType_Bool:
	{
		stream << " " << (value->m_bValue ? "true" : "false") << " (";
		WriteFlags(cvar.GetFlags(), stream);
		stream << ")";
		break;
	}
	case EConVarType_Int16:
	{
		MINMAXVALUEPRINT(m_i16Value);
		break;
	}
	case EConVarType_Int32:
	{
		MINMAXVALUEPRINT(m_i32Value);
		break;
	}
	case EConVarType_UInt32:
	{
		MINMAXVALUEPRINT(m_u32Value);
		break;
	}
	case EConVarType_Int64:
	{
		MINMAXVALUEPRINT(m_i64Value);
		break;
	}
	case EConVarType_UInt64:
	{
		MINMAXVALUEPRINT(m_u64Value);
		break;
	}
	case EConVarType_Float32:
	{
		MINMAXVALUEPRINT(m_fl32Value);
		break;
	}
	case EConVarType_Float64:
	{
		MINMAXVALUEPRINT(m_fl64Value);
		break;
	}
	case EConVarType_String:
	{
		stream << " \"" << (value->m_StringValue.m_pString ? value->m_StringValue.m_pString : "") << "\"" << " (";
		WriteFlags(cvar.GetFlags(), stream);
		stream << ")";
		break;
	}
	case EConVarType_Color:
	{
		stream << " [" << value->m_clrValue.r() << ", " << value->m_clrValue.g() << ", " << value->m_clrValue.b() << ", " << value->m_clrValue.a() << "]" << " (";
		WriteFlags(cvar.GetFlags(), stream);
		stream << ")";
		break;
	}
	case EConVarType_Vector2:
	{
		stream << " [" << value->m_vec2Value.x << ", " << value->m_vec2Value.y << "]" << " (";
		WriteFlags(cvar.GetFlags(), stream);
		stream << ")";
		break;
	}
	case EConVarType_Vector3:
	{
		stream << " [" << value->m_vec3Value.x << ", " << value->m_vec3Value.y << ", " << value->m_vec3Value.z << "]" << " (";
		WriteFlags(cvar.GetFlags(), stream);
		stream << ")";
		break;
	}
	case EConVarType_Vector4:
	{
		stream << " [" << value->m_vec4Value.x << ", " << value->m_vec4Value.y << ", " << value->m_vec4Value.z << ", " << value->m_vec4Value.w << "]" << " (";
		WriteFlags(cvar.GetFlags(), stream);
		stream << ")";
		break;
	}
	case EConVarType_Qangle:
	{
		stream << " [" << value->m_angValue.x << ", " << value->m_angValue.y << ", " << value->m_angValue.z << "]" << " (";
		WriteFlags(cvar.GetFlags(), stream);
		stream << ")";
		break;
	}
	default:
		stream << " UNKNOWN VALUE TYPE";
		break;
	}
}

void FixNewlineTabbing(std::string& str)
{
	auto it = str.begin();
	while ((it = std::find(it, str.end(), '\n')) != str.end())
	{
		if (it + 1 == str.end() || *(it + 1) != '\t')
			it = str.insert(it + 1, '\t') + 1;
		else
			it++;
	}

	// trim end of string
	if (str.back() == '\t')
		str.pop_back();

	if (str.back() == '\n')
		str.pop_back();
}

std::string EscapeDescription(std::string str)
{
	for (auto it = str.begin(); it != str.end(); it++) {
		if (*it == '\n')
		{
			*it = '\\';
			it = str.insert(it + 1, 'n');
		}
		else if (*it == '\t')
		{
			*it = '\\';
			it = str.insert(it + 1, 't');
		}
	}

	return str;
}


void DumpConVars(json& convarsArray)
{
	spdlog::info("Dumping convars");
	std::vector<ConVarRefAbstract> convars;
	// there's always gonna be a lot of convars, let's save some reallocations
	convars.reserve(1000);

	for (ConVarRefAbstract ref(ConVarRef((uint16)0)); ref.IsValidRef(); ref = ConVarRefAbstract(ConVarRef(ref.GetAccessIndex() + 1)))
	{
		convars.push_back(ref);
	}

	std::sort(convars.begin(), convars.end(), [](const ConVarRefAbstract a, const ConVarRefAbstract b) {
		return strcmp(a.GetName(), b.GetName()) < 0;
	});

	std::ofstream output(Globals::outputPath / "convars.txt");

	for (const auto cvar : convars)
	{
		std::string helpString = "<no description>";
		if (cvar.HasHelpText())
		{
			helpString = cvar.GetHelpText();
			Globals::stringsIgnoreStream << EscapeDescription(helpString) << "\n";
			FixNewlineTabbing(helpString);
		}

		output << cvar.GetName();
		WriteValueLine(cvar, output);
		output << "\n\t" << helpString;
		output << "\n" << std::endl;

		Globals::stringsIgnoreStream << cvar.GetName() << "\n";

		convarsArray.push_back(SerializeConVar(cvar));
	}

	output.close();
}

void DumpCommands(json& commandsArray)
{
	spdlog::info("Dumping commands");
	std::vector<ConCommandRef> commands;
	// there's always gonna be a lot of commands, let's save some reallocations
	commands.reserve(1000);
	ConCommandData* data = Interfaces::cvar->GetConCommandData(ConCommandRef());
	for (ConCommandRef ref = ConCommandRef((uint16)0); ref.GetRawData() != data; ref = ConCommandRef(ref.GetAccessIndex() + 1))
	{
		commands.push_back(ref);
	}

	std::sort(commands.begin(), commands.end(), [](const ConCommandRef a, const ConCommandRef b) {
		return strcmp(a.GetName(), b.GetName()) < 0;
	});

	std::ofstream output(Globals::outputPath / "commands.txt");

	for (const auto command : commands)
	{
		std::string helpString = "<no description>";
		if (command.HasHelpText())
		{
			helpString = command.GetHelpText();
			Globals::stringsIgnoreStream << EscapeDescription(helpString) << "\n";
			FixNewlineTabbing(helpString);
		}

		output << command.GetName() << " (";
		WriteFlags(command.GetFlags(), output);
		output << ")\n\t" << helpString;
		output << "\n" << std::endl;

		Globals::stringsIgnoreStream << command.GetName() << "\n";

		commandsArray.push_back(SerializeConCommand(command));
	}

	output.close();
}

void Dump()
{
	spdlog::debug("Removing default value from cl_color");
	// cl_color has a random default value on each start.
	if (ConVarRefAbstract cvar("cl_color"); cvar.IsValidRef())
	{
		cvar.GetConVarData()->RemoveDefaultValue();
	}

	json convarsArray = json::array();
	json commandsArray = json::array();

	DumpConVars(convarsArray);
	DumpCommands(commandsArray);

	nlohmann::ordered_json root;
	root["generator"] = "https://github.com/ValveResourceFormat/DumpSource2";

	if (!Globals::sourceRevision.empty())
		root["revision"] = std::stoi(Globals::sourceRevision);

	if (!Globals::versionDate.empty())
		root["version_date"] = Globals::versionDate;

	if (!Globals::versionTime.empty())
		root["version_time"] = Globals::versionTime;

	root["convars"] = convarsArray;
	root["commands"] = commandsArray;

	std::ofstream output(Globals::outputPath / "concommands.json");
	output << root.dump(-1);
	output.close();

	spdlog::info("Wrote concommands.json ({} convars, {} commands)", convarsArray.size(), commandsArray.size());
}

} // namespace Dumpers::ConCommands