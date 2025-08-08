#pragma once

#include "lua.hpp"

namespace Lua_DebugDraw {
	void Register(lua_State* L);

	int addArrow(lua_State* L);
	int addSphere(lua_State* L);
	int addTransform(lua_State* L);
	int clear(lua_State* L);
	int removeArrow(lua_State* L);
	int removeSphere(lua_State* L);
	int removeTransform(lua_State* L);

	// Extras
	int drawLine(lua_State* L);
}
