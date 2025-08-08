#pragma once

#include <string_view>
#include <string>
#include <mutex>

#include "IcoSphere.hpp"
#include "Types.hpp"
#include "NullHash.hpp"

struct DebugArrow {
	std::string name;
	Vec3 begin;
	Vec3 end;
	u8Vec3 color;
};

struct DebugSphere {
	std::string name;
	Vec3 position;
	float radius;
	u8Vec3 color;
	IcoSphere shape;
};

struct DebugTransform {
	std::string name;
	Vec3 origin;
	Quat rotation;
	Vec3 scale;
};

class DebugDrawManager {
	public:
		DebugDrawManager();
		~DebugDrawManager();

		inline bool isEnabled() const {return m_bEnabled;};

		void render();

		void addArrow(const std::string_view& name, const Vec3& begin, const Vec3& end, u8Vec3 color);
		void addSphere(const std::string_view& name, const Vec3& position, float radius, u8Vec3 color);
		void addTransform(const std::string_view& name, const Vec3& origin, const Quat& rotation, const Vec3& scale);

		void clear(const std::string_view& name = "");

		void removeArrow(const std::string_view& name);
		void removeSphere(const std::string_view& name);
		void removeTransform(const std::string_view& name);

	private:
		bool m_bEnabled = false;
		IcoSphere m_arrBaseSphereLevels[3];
		std::mutex m_mutex;
		NullHashMap<uint32, DebugArrow> m_mapArrows;
		NullHashMap<uint32, DebugSphere> m_mapSpheres;
		NullHashMap<uint32, DebugTransform> m_mapTransforms;
};

extern DebugDrawManager* g_debugDrawManager;
