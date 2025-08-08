
#include "xxh3.h"

#include "DebugDrawManager.hpp"
#include "SM/DebugDrawer.hpp"

using namespace SM;

constexpr Vec3 UP = {0.0f, 0.0f, 1.0f};
constexpr float ArrowHeadLength = 0.5f;
constexpr float TransformArrowHeadLength = 0.25f;
constexpr float ArrowheadAngle = glm::radians(25.0f);

static uint8 GetSphereSizeLevel(float radius) {
	if ( radius <= 0.25f )
		return 0;
	else if ( radius <= 1.0f )
		return 1;
	else
		return 2;
}

static void GenerateArrowHeadLines(const Vec3& arrowDir, Vec3* pArrHeadLines) {
	Vec3 dirNorm = glm::normalize(arrowDir);
	Vec3 up = UP;
	if ( glm::abs(glm::dot(dirNorm, up)) > 0.99f )
		up = Vec3(0.0f, 1.0f, 0.0f);
	Vec3 right = glm::normalize(glm::cross(dirNorm, up));
	Vec3 oUp = glm::normalize(glm::cross(right, dirNorm));

	pArrHeadLines[0] = glm::normalize(-dirNorm * cos(ArrowheadAngle) + right * sin(ArrowheadAngle));
	pArrHeadLines[1] = glm::normalize(-dirNorm * cos(ArrowheadAngle) - right * sin(ArrowheadAngle));
	pArrHeadLines[2] = glm::normalize(-dirNorm * cos(ArrowheadAngle) + oUp * sin(ArrowheadAngle));
	pArrHeadLines[3] = glm::normalize(-dirNorm * cos(ArrowheadAngle) - oUp * sin(ArrowheadAngle));
}

static void DrawArrow(DebugDrawer* pDrawer, const Vec3& begin, const Vec3& end, u8Vec3 color, float headLineLength) {
	Vec3 arrowDir = end - begin;
	float length = glm::length(arrowDir);

	Vec3 headLines[4];
	GenerateArrowHeadLines(arrowDir, headLines);

	pDrawer->drawLine(begin, end, color);
	for ( const Vec3& dir : headLines )
		pDrawer->drawLine(end, end + dir * min(headLineLength, length), color);
}



DebugDrawManager* g_debugDrawManager = nullptr;

DebugDrawManager::DebugDrawManager() {
	for ( uint8 i = 0; i < std::size(m_arrBaseSphereLevels); ++i )
		m_arrBaseSphereLevels[i] = IcoSphere(i);

	LPSTR cmdLine = GetCommandLineA();
	if ( std::string_view(cmdLine).find("-debugDraw") != std::string::npos )
		m_bEnabled = true;
	g_debugDrawManager = this;
}

DebugDrawManager::~DebugDrawManager() {
	g_debugDrawManager = nullptr;
}

void DebugDrawManager::render() {
	if ( !m_bEnabled )
		return;

	DebugDrawer* pDrawer = DebugDrawer::Get();

	std::scoped_lock lock0(m_mutex);
	std::scoped_lock lock1(pDrawer->getLock());

	// Draw arrows
	for ( const auto& [k, arrow] : m_mapArrows )
		DrawArrow(pDrawer, arrow.begin, arrow.end, arrow.color, ArrowHeadLength);

	// Draw spheres
	for ( const auto& [k, sphere] : m_mapSpheres ) {
		for ( const IcoSphere::Line& line : sphere.shape.getLines() )
			pDrawer->drawLine(sphere.position + line.begin, sphere.position + line.end, sphere.color);
	}

	// Draw transforms
	for ( const auto& [k, transform] : m_mapTransforms ) {
		Vec3 x = transform.rotation * Vec3(transform.scale.x, 0.0f, 0.0f);
		Vec3 y = transform.rotation * Vec3(0.0f, transform.scale.y, 0.0f);
		Vec3 z = transform.rotation * Vec3(0.0f, 0.0f, transform.scale.z);

		DrawArrow(pDrawer, transform.origin, transform.origin + x, {0xFF, 0x00, 0x00}, TransformArrowHeadLength);
		DrawArrow(pDrawer, transform.origin, transform.origin + y, {0x00, 0xFF, 0x00}, TransformArrowHeadLength);
		DrawArrow(pDrawer, transform.origin, transform.origin + z, {0x00, 0x00, 0xFF}, TransformArrowHeadLength);
	}
}

void DebugDrawManager::addArrow(const std::string_view& name, const Vec3& begin, const Vec3& end, u8Vec3 color) {
	if ( !m_bEnabled )
		return;
	uint32 hash = XXH32(name.data(), name.size(), 0);
	std::scoped_lock lock(m_mutex);
	auto it = m_mapArrows.find(hash);
	if ( it == m_mapArrows.end() )
		return (void)m_mapArrows.emplace(hash, DebugArrow(std::string(name), begin, end, color));

	DebugArrow& elem = it->second;
	elem.begin = begin;
	elem.end = end;
	elem.color = color;
}

void DebugDrawManager::addSphere(const std::string_view& name, const Vec3& position, float radius, u8Vec3 color) {
	if ( !m_bEnabled )
		return;
	uint32 hash = XXH32(name.data(), name.size(), 0);
	std::scoped_lock lock(m_mutex);
	auto it = m_mapSpheres.find(hash);
	if ( it == m_mapSpheres.end() )
		return (void)m_mapSpheres.emplace(
			hash, DebugSphere(std::string(name), position, radius, color, IcoSphere(GetSphereSizeLevel(radius), Vec3(radius)))
		);

	DebugSphere& elem = it->second;
	elem.position = position;
	elem.color = color;
	if ( radius != elem.radius ) {
		elem.radius = radius;
		elem.shape = IcoSphere(GetSphereSizeLevel(radius), Vec3(radius));
	}
}

void DebugDrawManager::addTransform(const std::string_view& name, const Vec3& origin, const Quat& rotation, const Vec3& scale) {
	if ( !m_bEnabled )
		return;
	uint32 hash = XXH32(name.data(), name.size(), 0);
	std::scoped_lock lock(m_mutex);
	auto it = m_mapTransforms.find(hash);
	if ( it == m_mapTransforms.end() )
		return (void)m_mapTransforms.emplace(hash, DebugTransform(std::string(name), origin, rotation, scale));

	DebugTransform& elem = it->second;
	elem.origin = origin;
	elem.rotation = rotation;
	elem.scale = scale;
}

void DebugDrawManager::clear(const std::string_view& name) {
	std::scoped_lock lock(m_mutex);
	if ( name.empty() ) {
		m_mapArrows.clear();
		m_mapSpheres.clear();
		m_mapTransforms.clear();
		return;
	}
	{
		auto it = m_mapArrows.begin();
		while ( it != m_mapArrows.end() ) {
			if ( it->second.name.starts_with(name) )
				it = m_mapArrows.erase(it);
			else
				++it;
		}
	}
	{
		auto it = m_mapSpheres.begin();
		while ( it != m_mapSpheres.end() ) {
			if ( it->second.name.starts_with(name) )
				it = m_mapSpheres.erase(it);
			else
				++it;
		}
	}
	{
		auto it = m_mapTransforms.begin();
		while ( it != m_mapTransforms.end() ) {
			if ( it->second.name.starts_with(name) )
				it = m_mapTransforms.erase(it);
			else
				++it;
		}
	}
}

void DebugDrawManager::removeArrow(const std::string_view& name) {
	uint32 hash = XXH32(name.data(), name.size(), 0);
	std::scoped_lock lock(m_mutex);
	m_mapArrows.erase(hash);
}

void DebugDrawManager::removeSphere(const std::string_view& name) {
	uint32 hash = XXH32(name.data(), name.size(), 0);
	std::scoped_lock lock(m_mutex);
	m_mapSpheres.erase(hash);
}

void DebugDrawManager::removeTransform(const std::string_view& name) {
	uint32 hash = XXH32(name.data(), name.size(), 0);
	std::scoped_lock lock(m_mutex);
	m_mapTransforms.erase(hash);
}
