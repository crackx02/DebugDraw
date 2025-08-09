
#include <thread>
#include <set>

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#include "MinHook.h"
#include "lua.hpp"

#include "Util.hpp"
#include "Types.hpp"
#include "DebugDrawManager.hpp"
#include "Lua_DebugDraw.hpp"
#include "SM/Console.hpp"
#include "SM/RenderStateManager.hpp"

using namespace SM;

constexpr uintptr Offset_InitializeConsole = 0x02d7a80;
constexpr uintptr Offset_RegisterDebugDraw = 0x02d7a80;
constexpr uintptr Offset_DebugDrawer_Render = 0x09ef890;
constexpr uintptr Offset_PlayState_Cleanup = 0x042dab0;



// State //

static struct {
	bool bMhInitialized = false;
	DebugDrawManager debugDrawManager;
	std::mutex setInjectedLuaStatesMutex;
	std::set<lua_State*> setInjectedLuaStates;
} g_State;



// Hooks //

static int(*O_luaL_loadstring)(lua_State*, const char*);
static int H_luaL_loadstring(lua_State* L, const char* str) {
	int res = O_luaL_loadstring(L, str);
	if ( res == 0 ) {
		bool inject = false;
		{
			std::scoped_lock lock(g_State.setInjectedLuaStatesMutex);
			inject = !g_State.setInjectedLuaStates.contains(L);
		}

		if ( inject ) {
			// Need to do this as axolot removed debugDraw from the terrain env even though the documentation claims it exists
			std::string_view sv(str);
			if ( sv.find("unsafe_env") != std::string::npos && sv.find("debugDraw") == std::string::npos ) {
				lua_pcall(L, 0, -1, 0);
				res = O_luaL_loadstring(L, "unsafe_env.sm.debugDraw = sm.debugDraw");
				std::scoped_lock lock(g_State.setInjectedLuaStatesMutex);
				g_State.setInjectedLuaStates.insert(L);
			}
		}
	}
	return res;
}

static void(*O_PlayState_Cleanup)(void*) = nullptr;
static void H_PlayState_Cleanup(void* self) {
	{
		std::scoped_lock lock(g_State.setInjectedLuaStatesMutex);
		g_State.setInjectedLuaStates.clear();
	}
	g_debugDrawManager->clear();
	O_PlayState_Cleanup(self);
}

static void(*O_DebugDrawer_Render)(DebugDrawer*, float, void*, void*) = nullptr;
static void H_DebugDrawer_Render(DebugDrawer* self, float p2, void* p3, void* p4) {
	g_debugDrawManager->render();
	O_DebugDrawer_Render(self, p2, p3, p4);
}

static void(*O_luaL_Register)(lua_State* L, const char* libname, const luaL_Reg* lib) = nullptr;
static void H_luaL_Register(lua_State* L, const char* libname, const luaL_Reg* lib) {
	if ( libname != nullptr && strcmp(libname, "sm.debugDraw") == 0 ) {
		// The render terrain env Lua state is recreated when hopping between different worlds, need to re-inject
		std::scoped_lock lock(g_State.setInjectedLuaStatesMutex);
		g_State.setInjectedLuaStates.erase(L);
		Lua_DebugDraw::Register(L);
	} else
		O_luaL_Register(L, libname, lib);
}

static void(*O_InitializeConsole)(void*, void*) = nullptr;
static void H_InitializeConsole(void* pContraption, void* ptr) {
	if ( pContraption != nullptr )
		O_InitializeConsole(pContraption, ptr);
	
	SM_LOG("Initializing");

	ResolveClassOffset(RenderStateManager);

	if ( MH_CreateHookApi(L"lua51.dll", "luaL_register", (LPVOID)H_luaL_Register, (LPVOID*)&O_luaL_Register) != MH_OK )
		SM_ERROR("Failed to hook luaL_register!");

	if ( MH_CreateHookApi(L"lua51.dll", "luaL_loadstring", (LPVOID)H_luaL_loadstring, (LPVOID*)&O_luaL_loadstring) != MH_OK )
		SM_ERROR("Failed to hook luaL_loadstring!");

	if ( MakeHook(DebugDrawer_Render) != MH_OK )
		SM_ERROR("Failed to hook DebugDrawer::render!");

	if ( MakeHook(PlayState_Cleanup) != MH_OK )
		SM_ERROR("Failed to hook PlayState::cleanup!");

	if ( MH_EnableHook(MH_ALL_HOOKS) != MH_OK ) {
		SM_ERROR("Failed to enable hooks!");
		if ( pContraption == nullptr )
			*(bool*)ptr = false;
		return;
	}

	SM_LOG("Initialized");
}



// DLL Process //

static bool Attach() {
	if ( MH_Initialize() != MH_OK ) {
		MessageBoxA(nullptr, "Failed to initialize MinHook", "DebugDraw ERROR", MB_OK);
		return false;
	}
	g_State.bMhInitialized = true;
	
	ResolveClassOffset(Console);

	// Initialize only once the console exists, that way we can properly log stuff
	if ( Console::Get() == nullptr ) {
		if ( MakeHook(InitializeConsole) != MH_OK || EnableHook(InitializeConsole) != MH_OK ) {
			MessageBoxA(nullptr, "Failed to hook InitializeConsole", "DebugDraw ERROR", MB_OK);
			return false;
		}
	} else {
		SM_WARN("DebugDraw mod was injected after game startup! This is experimental and may not work properly.");
		bool res = true;
		H_InitializeConsole(nullptr, &res);
		return res;
	}

	return true;
}

static void Detach() {
	if ( g_State.bMhInitialized ) {
		g_State.bMhInitialized = false;
		MH_Uninitialize();
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH: {
			return Attach();
			break;
		}
		case DLL_PROCESS_DETACH:
			Detach();
	}
	return TRUE;
}
