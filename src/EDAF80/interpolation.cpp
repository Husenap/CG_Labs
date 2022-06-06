#include "interpolation.hpp"

glm::vec3
interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	return glm::vec2(1, x) * glm::mat2x2(1, -1, 0, 1) * glm::transpose(glm::mat2x3(p0, p1));
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
	glm::vec3 const& p2, glm::vec3 const& p3,
	float const t, float const x)
{
	return glm::vec4(1, x, x * x, x * x * x) * glm::mat4x4(0, -t, 2 * t, -t, 1, 0, t - 3, 2 - t, 0, t, 3 - 2 * t, t - 2, 0, 0, -t, t) * glm::transpose(glm::mat4x3(p0, p1, p2, p3));
}
