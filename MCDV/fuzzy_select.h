#pragma once
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "plane.h"
#include "convexPolytope.h"

namespace fuzzy_select{


	Plane* find_plane_by_normal(std::vector<Plane*> planes, glm::vec3 normal, float error = 0.01f) {
		for (auto && p : planes) {
			if (glm::distance(p->normal, normal) < error)
				return p;
		}

		return NULL;
	}

	// This just, doesnt work?? 
	BrushPolygon* find_bpoly_by_normal(std::vector<BrushPolygon> polys, glm::vec3 normal, float error = 0.0001f) {
		for (auto && p : polys) {
			if (glm::distance(p.plane.normal, normal) < error)
				return &p;
		}

		return NULL;
	}

}