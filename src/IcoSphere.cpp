
#include <set>

#include "IcoSphere.hpp"

static Vec3 Vec3Slerp(Vec3 a, Vec3 b, float t) {
	a = glm::normalize(a);
	b = glm::normalize(b);

	float dot = glm::clamp(glm::dot(a, b), -1.0f, 1.0f);
	float theta = acos(dot) * t;

	Vec3 relative = glm::normalize(b - a * dot);
	return a * cos(theta) + relative * sin(theta);
}

struct IndexPair {
	uint32 a;
	uint32 b;

	IndexPair(uint32 i1, uint32 i2) {
		a = std::min(i1, i2);
		b = std::max(i1, i2);
	}

	bool operator<(const IndexPair& other) const {
		return (a != other.a ? a < other.a : b < other.b);
	}
};



IcoSphere::IcoSphere(uint8 depth) {
	m_depth = depth;
	float rw = 0.8506507f;
	float rh = 0.525731f;

	std::vector<Vec3> vecVertices = {
		{-rw, 0.0f, rh}, {0.0f, rh, rw},
		{0.0f, -rh, rw}, {rw, 0.0f, rh},
		{rh, -rw, 0.0f}, {-rh, -rw, 0.0f},
		{-rw, 0.0f, -rh}, {0.0f, -rh, -rw},
		{0.0f, rh, -rw}, {rw, 0.0f, -rh},
		{rh, rw, 0.0f}, {-rh, rw, 0.0f}
	};

	std::vector<uint32> vecIndices = {
		0, 1, 2, 0, 2, 5, 0, 5, 6,
		0, 6, 11, 0, 11, 1, 1, 3, 2,
		1, 10, 3, 1, 11, 10, 2, 3, 4,
		2, 4, 5, 3, 10, 9, 3, 9, 4,
		4, 9, 7, 4, 7, 5, 5, 6, 7,
		6, 8, 7, 6, 11, 8, 7, 8, 9,
		8, 10, 9, 8, 11, 10
	};

	if ( depth != 0 ) {
		uint32 triCount = uint32(vecIndices.size() / 3);
		uint32 reservedIndices = uint32(pow(4, depth) * triCount * 3);

		std::vector<Vec3> vecTempVertices;
		std::vector<uint32> vecTempIndices;
		vecIndices.reserve(reservedIndices);
		vecVertices.reserve(reservedIndices / 4);
		vecTempIndices.reserve(reservedIndices);
		vecTempVertices.reserve(reservedIndices / 4);

		for ( uint8 i = 1; i <= depth; ++i ) {
			vecTempVertices.clear();
			vecTempIndices.clear();

			for ( uint32* p = vecIndices.data(); p != vecIndices.data() + vecIndices.size(); p += 3 ) {
				const Vec3& v0 = vecVertices[*(p + 0)];
				const Vec3& v1 = vecVertices[*(p + 1)];
				const Vec3& v2 = vecVertices[*(p + 2)];

				Vec3 arrVertices[6] = {
					v0,
					Vec3Slerp(v0, v1, 0.5f),
					v1,
					Vec3Slerp(v1, v2, 0.5f),
					v2,
					Vec3Slerp(v2, v0, 0.5f)
				};

				uint32 indexBase = uint32(vecTempVertices.size());
				uint32 arrIndices[] = {
					indexBase + 0,
					indexBase + 1,
					indexBase + 5,
					indexBase + 1,
					indexBase + 2,
					indexBase + 3,
					indexBase + 1,
					indexBase + 3,
					indexBase + 5,
					indexBase + 3,
					indexBase + 4,
					indexBase + 5
				};

				vecTempVertices.insert(vecTempVertices.end(), arrVertices, arrVertices + std::size(arrVertices));
				vecTempIndices.insert(vecTempIndices.end(), arrIndices, arrIndices + std::size(arrIndices));
			}

			vecVertices.swap(vecTempVertices);
			vecIndices.swap(vecTempIndices);
		}
	}


	std::set<IndexPair> setUniqueLinePairs;
	for ( uint64 i = 0; i + 2 < vecIndices.size(); i += 3 ) {
		uint32 i0 = vecIndices[i + 0];
		uint32 i1 = vecIndices[i + 1];
		uint32 i2 = vecIndices[i + 2];
		setUniqueLinePairs.emplace(i0, i1);
		setUniqueLinePairs.emplace(i1, i2);
		setUniqueLinePairs.emplace(i2, i0);
	}

	m_vecLines.reserve(setUniqueLinePairs.size());
	for ( const IndexPair& pair : setUniqueLinePairs )
		m_vecLines.emplace_back(vecVertices[pair.a], vecVertices[pair.b]);

	m_vecLines.shrink_to_fit();
}

IcoSphere::IcoSphere(const IcoSphere& base, const Vec3& scale) {
	m_depth = base.getDepth();
	m_vecLines = base.getLines();
	for ( Line& l : m_vecLines ) {
		l.begin *= scale;
		l.end *= scale;
	}
}
