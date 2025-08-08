#pragma once

#include <vector>

#include "Types.hpp"

class IcoSphere {
	public:
		struct Line {
			Vec3 begin;
			Vec3 end;
		};

		IcoSphere() {};
		IcoSphere(uint8 depth);
		IcoSphere(const IcoSphere& base, const Vec3& scale);

		inline uint8 getDepth() const {return m_depth;};
		inline const std::vector<Line>& getLines() const {return m_vecLines;};

	private:
		uint8 m_depth = 0;
		std::vector<Line> m_vecLines;
};
