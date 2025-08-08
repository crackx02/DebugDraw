
#include "Lua_DebugDraw.hpp"
#include "DebugDrawManager.hpp"
#include "SM/DebugDrawer.hpp"
#include "SM/Console.hpp"

static constexpr Vec3 UP = {0.0f, 0.0f, 1.0f};
static constexpr u8Vec3 WHITE = {0xFF, 0xFF, 0xFF};

inline static void CheckArgCount(lua_State* L, int min, int max) {
	int top = lua_gettop(L);
	if ( top < min )
		luaL_error(L, "expected at least %i arguments, got %i", min, top);
	if ( top > max )
		luaL_error(L, "expected at most %i arguments, got %i", max, top);
}

static std::string_view CheckString(lua_State* L, int index, bool optional = false) {
	int t = lua_type(L, index);
	if ( t <= LUA_TNIL && optional )
		return "";
	if ( t != LUA_TSTRING )
		if ( optional )
			luaL_error(L, "expected string or nil, got %s", lua_typename(L, t));
		else
			luaL_error(L, "expected string, got %s", lua_typename(L, t));
	uint64 len = 0;
	const char* str = luaL_checklstring(L, index, &len);
	return {str, len};
}

static Vec3* CheckVec3(lua_State* L, int index, bool optional = false) {
	if ( lua_type(L, index) <= LUA_TNIL && optional )
		return nullptr;
	return (Vec3*)luaL_checkudata(L, index, "Vec3");
}

static u8Vec3 OptColor(lua_State* L, int index, u8Vec3 def) {
	if ( lua_type(L, index) <= LUA_TNIL )
		return def;
	return u8Vec3(*(Vec3*)luaL_checkudata(L, index, "Color") * 255.0f);
}

static Quat* CheckQuat(lua_State* L, int index) {
	return (Quat*)luaL_checkudata(L, index, "Quat");
}

static bool CheckBoolean(lua_State* L, int index) {
	int t = lua_type(L, index);
	if ( t != LUA_TBOOLEAN )
		luaL_error(L, "expected boolean, got %s", lua_typename(L, t));
	return lua_toboolean(L, index);
}



void Lua_DebugDraw::Register(lua_State* L) {
	lua_getglobal(L, "sm");
	SM_ASSERT(lua_istable(L, -1));

	lua_pushstring(L, "debugDraw");
	lua_newtable(L);

	lua_pushstring(L, "addArrow");
	lua_pushcfunction(L, addArrow);
	lua_rawset(L, -3);

	lua_pushstring(L, "addSphere");
	lua_pushcfunction(L, addSphere);
	lua_rawset(L, -3);

	lua_pushstring(L, "addTransform");
	lua_pushcfunction(L, addTransform);
	lua_rawset(L, -3);

	lua_pushstring(L, "clear");
	lua_pushcfunction(L, clear);
	lua_rawset(L, -3);

	lua_pushstring(L, "removeArrow");
	lua_pushcfunction(L, removeArrow);
	lua_rawset(L, -3);

	lua_pushstring(L, "removeSphere");
	lua_pushcfunction(L, removeSphere);
	lua_rawset(L, -3);

	lua_pushstring(L, "removeTransform");
	lua_pushcfunction(L, removeTransform);
	lua_rawset(L, -3);

	lua_pushstring(L, "drawLine");
	lua_pushcfunction(L, drawLine);
	lua_rawset(L, -3);

	lua_pushstring(L, "enabled");
	lua_pushboolean(L, g_debugDrawManager->isEnabled());
	lua_rawset(L, -3);

	lua_pushvalue(L, -2);
	lua_pushvalue(L, -2);
	lua_rawset(L, -5);
	lua_pop(L, -2);
	lua_pop(L, -2);

	SM_LOG("DebugDraw registered");
}

int Lua_DebugDraw::addArrow(lua_State* L) {
	CheckArgCount(L, 2, 4);
	std::string_view name = CheckString(L, 1);
	Vec3* pStartPos = CheckVec3(L, 2);
	Vec3* pEndPos = CheckVec3(L, 3, true);
	g_debugDrawManager->addArrow(
		name, *pStartPos,
		(pEndPos != nullptr ? *pEndPos : *pStartPos + UP),
		OptColor(L, 4, WHITE)
	);
	return 0;
}

int Lua_DebugDraw::addSphere(lua_State* L) {
	CheckArgCount(L, 2, 4);
	std::string_view name = CheckString(L, 1);
	Vec3* pPosition = CheckVec3(L, 2);
	float radius = float(luaL_optnumber(L, 3, 0.125));
	g_debugDrawManager->addSphere(
		name, *pPosition, radius,
		OptColor(L, 4, WHITE)
	);
	return 0;
}

int Lua_DebugDraw::addTransform(lua_State* L) {
	CheckArgCount(L, 3, 4);
	std::string_view name = CheckString(L, 1);
	Vec3* pOrigin = CheckVec3(L, 2);
	Quat* pRotation = CheckQuat(L, 3);
	float scale = float(luaL_optnumber(L, 4, 1.0));
	g_debugDrawManager->addTransform(name, *pOrigin, *pRotation, Vec3(scale));
	return 0;
}

int Lua_DebugDraw::clear(lua_State* L) {
	CheckArgCount(L, 0, 1);
	g_debugDrawManager->clear(CheckString(L, 1, true));
	return 0;
}

int Lua_DebugDraw::removeArrow(lua_State* L) {
	CheckArgCount(L, 1, 1);
	g_debugDrawManager->removeArrow(CheckString(L, 1));
	return 0;
}

int Lua_DebugDraw::removeSphere(lua_State* L) {
	CheckArgCount(L, 1, 1);
	g_debugDrawManager->removeSphere(CheckString(L, 1));
	return 0;
}

int Lua_DebugDraw::removeTransform(lua_State* L) {
	CheckArgCount(L, 1, 1);
	g_debugDrawManager->removeTransform(CheckString(L, 1));
	return 0;
}

int Lua_DebugDraw::drawLine(lua_State* L) {
	CheckArgCount(L, 1, 3);
	Vec3* pBegin = CheckVec3(L, 1);
	Vec3* pEnd = CheckVec3(L, 2, true);
	u8Vec3 color = OptColor(L, 3, {0xFF, 0xFF, 0xFF});
	SM::DebugDrawer* pDrawer = SM::DebugDrawer::Get();
	std::scoped_lock lock(pDrawer->getLock());
	pDrawer->drawLine(*pBegin, (pEnd != nullptr ? *pEnd : *pBegin + Vec3(0.0f, 0.0f, 1.0f)), color);
	return 0;
}
