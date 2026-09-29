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

#pragma once
#ifdef GAME_DEADLOCK
#include "appframework/iappsystem.h"
#else
#include "appframework/IAppSystem.h"
#endif

class DumperApplication : public CTier0AppSystem<IAppSystem>
{
	virtual void Destructor() {};
#ifndef _WIN32
	virtual void Destructor2() {};
#endif
	virtual void PreShutdown() {};
#ifdef GAME_DEADLOCK
	// Deadworks' sourcesdk has the whole enum; 2 is the kBuildTypeRelease of the others
	virtual AppSystemBuildType_t GetBuildType() { return APP_SYSTEM_BUILD_RETAIL; };
#else
	virtual BuildType_t	GetBuildType() { return kBuildTypeRelease; };
#endif
	virtual void Reconnect(CreateInterfaceFn factory, const char* interfaceName) {};


	virtual int AddSystem(IAppSystem* pAppSystem, const char* interfaceName, bool errorOut) { return 0; };
	virtual int AddSystem(const char* unk, const char* interfaceName, bool errorOut) { return 0; };
	virtual int AddSystem(IAppSystem* pAppSystem, const char* interfaceName) { return 0; };
	virtual void RemoveSystem(IAppSystem* pAppSystem) { };
	virtual int AddSystems(int count, void** pAppSystems) { return 0; };
	virtual void* FindSystem(const char* interfaceName) { return nullptr; };
	virtual void* GetGameInfo() {
		printf("called getgameinfo\n");
		return nullptr;
	};
	virtual unsigned int unk1() { return -1; };
	virtual int GetUILanguage(int languageType) { return 0; };
	virtual int GetAudioLanguage(int languageType) { return 0; };
	virtual bool IsInToolsMode() { return false; };
	virtual bool unk2() { return false; };
	virtual bool unk3() { return false; };
	virtual bool unk4() { return false; };
	virtual void* unk5() { return nullptr; };
	virtual void* unk6() { return nullptr; };
	virtual void* unk7() { return nullptr; };
	virtual void* unk8() { return nullptr; };
	virtual void* unk9() { return nullptr; };
	virtual void* unk10(void* a) { return a; };
	virtual void* unk11() { return nullptr; };
	virtual void* AddSystemDontLoadStartupManifests(const char* a, const char* b) { return nullptr; };
	virtual void* unk12() { return nullptr; };
	virtual void* unk13() { return nullptr; };
};