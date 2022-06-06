#pragma once

#include <cmath>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <imgui.h>

struct VerletObject {
	glm::vec3 currentPosition  = {};
	glm::vec3 previousPosition = {};
	glm::vec3 acceleration     = {};
	float     radius           = 5.f;
	uint32_t  color            = 0xffffffff;

	void updatePosition(float dt) {
		const glm::vec3 velocity = currentPosition - previousPosition;

		previousPosition = currentPosition;

		currentPosition += velocity + acceleration * dt * dt;

		acceleration = {};
	}

	void accelerate(glm::vec3 acc) { acceleration += acc; }
};

namespace std {
template <>
struct hash<glm::ivec3> {
	inline size_t operator()(const glm::ivec3& v) const {
		std::hash<int> int_hasher;
		return int_hasher(v.x) ^ int_hasher(v.y) ^ int_hasher(v.z);
	}
};
}  // namespace std

static float CellSize = 50.f;
struct SpatialPartition {
	std::unordered_map<glm::ivec3, std::vector<int>> cells;

	void clear() { cells.clear(); }

	std::pair<glm::ivec3, glm::ivec3> getRange(const VerletObject& o) const {
		const auto& p = o.currentPosition;
		const float r = o.radius;
		return {{static_cast<int>(std::floor((p.x - r) / CellSize)),
		         static_cast<int>(std::floor((p.y - r) / CellSize)),
				 static_cast<int>(std::floor((p.z - r) / CellSize))},
		        {static_cast<int>(std::floor((p.x + r) / CellSize)),
		         static_cast<int>(std::floor((p.y + r) / CellSize)),
				 static_cast<int>(std::floor((p.z + r) / CellSize))}};
	}

	void insert(const VerletObject& o, int id) {
		const auto minmax = getRange(o);
		const auto min = minmax.first;
		const auto max = minmax.second;
		for (int z = min.z; z <= max.z; ++z) {
			for (int y = min.y; y <= max.y; ++y) {
				for (int x = min.x; x <= max.x; ++x) {
					cells[glm::ivec3(x, y, z)].push_back(id);
				}
			}
		}
	}

	template <typename Fn>
	void apply(const VerletObject& o, Fn fn) const {
		const auto minmax = getRange(o);
		const auto min = minmax.first;
		const auto max = minmax.second;
		for (int z = min.z; z <= max.z; ++z) {
			for (int y = min.y; y <= max.y; ++y) {
				for (int x = min.x; x <= max.x; ++x) {
					auto it = cells.find({ x, y, z });
					if (it != cells.end()) {
						for (int id : it->second) {
							fn(id);
						}
					}
				}
			}
		}
	}
};

class Solver {
	static constexpr glm::vec3 Gravity = {0.0f, -982.f, 0.0f};

public:
	void addObject() {
		VerletObject newObject;
		newObject .currentPosition = { (static_cast<float>(rand()) / RAND_MAX) * 10.f, 0.f , (static_cast<float>(rand()) / RAND_MAX) * 10.f};
		newObject.radius = (std::pow(static_cast<float>(rand()) / RAND_MAX, 4.f)) * 3.f + 2.f;
		newObject.color = IM_COL32(rand() % 255, rand() % 255, rand() % 255, 255);
		objects.push_back(newObject);

		averageRadius = 0.f;
		for (auto& o : objects) {
			averageRadius += o.radius;
		}
		averageRadius /= objects.size();
	}

	void update(float dt) {
		static constexpr int SubSteps = 8;
		const float          subDt    = dt / static_cast<float>(SubSteps);

		numCollisions = 0;

		for (int i = 0; i < SubSteps; ++i) {
			applyGravity();
			applyConstraint();

			partition.clear();
			for (int id = 0; id < objects.size(); ++id) {
				partition.insert(objects[id], id);
			}
			solveCollisions();

			updatePositions(subDt);
		}

		numCollisions /= SubSteps;
	}

	void clear() { objects.clear(); }

	template <typename Fn>
	void apply(Fn fn) const {
		for (const auto& o : objects) {
			fn(o);
		}
	}

	void debug() {
		if (ImGui::Begin("Verlet Debug")) {
			ImGui::DragFloat("Red Gravity", &redGravity);
			ImGui::DragFloat("Map Radius", &mapRadius);
			ImGui::DragFloat("Cell Size", &CellSize);
			ImGui::Text("Number of Objects: %d", objects.size());
			ImGui::Text("Number of Collisions: %d", numCollisions);
			ImGui::Text("Average Radius: %f", averageRadius);
		}
		ImGui::End();
	}

	const float getMapRadius() const { return mapRadius; }

private:
	void updatePositions(float dt) {
		for (auto& o : objects) {
			o.updatePosition(dt);
		}
	}

	void applyGravity() {
		for (auto& o : objects) {
			o.accelerate(Gravity);
		}
	}

	void applyConstraint() {
		static constexpr glm::vec3 center = {0, 0, 0};

		for (auto& o : objects) {
			const auto  toObj = o.currentPosition - center;
			const float dist  = glm::length(toObj);
			if (dist > mapRadius - o.radius) {
				const auto n      = toObj / dist;
				o.currentPosition = center + n * (mapRadius - o.radius);
			}
		}
	}

	void solveCollisions() {
		for (int i = 0; i < objects.size(); ++i) {
			auto& a = objects[i];
			partition.apply(a, [&](int id) {
				if (id <= i) return;
				auto& b = objects[id];

				const auto collisionAxis =
				    a.currentPosition - b.currentPosition;
				const float dist = glm::length(collisionAxis);
				if (dist > 0.f && dist < a.radius + b.radius) {
					const auto  n     = collisionAxis / dist;
					const float delta = (a.radius + b.radius) - dist;
					a.currentPosition += 0.5f * delta * n;
					b.currentPosition -= 0.5f * delta * n;

					++numCollisions;
				}
			});
		}
	}

	std::vector<VerletObject> objects;
	SpatialPartition          partition;

	float redGravity    = 0.f;
	float mapRadius     = 100.f;
	float averageRadius = 0.f;
	int   numCollisions = 0;
};
