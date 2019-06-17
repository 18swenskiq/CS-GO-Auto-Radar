#pragma once
#include <iostream>
#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <vector>
#include <unordered_set>

#include "plane.h"
#include "Mesh.hpp"

struct BrushPolygon {
	Plane plane;
	std::vector<glm::vec3> vertices;

	BrushPolygon(Plane p) :
		plane(p) {}
};

namespace ray
{
	bool IntersectNgon(glm::vec3 orig, glm::vec3 dir,
		BrushPolygon polygon,
		float* t)
	{
		if (polygon.vertices.size() < 3)
			return false;

		glm::vec3 N = polygon.plane.normal;

		//Find P

		//Check parralel
		float NdotRayDir = glm::dot(N, dir);
		if (glm::abs(NdotRayDir) < 1e-3)
			return false;

		//Compute T
		*t = -(glm::dot(N, orig) + polygon.plane.offset) / NdotRayDir;
		//Check if triangle is behind the ray
		if (*t < 0) return false;

		glm::vec3 P = orig + ((*t) * dir);

		glm::vec3 C, edge, vp;

		//Inside out testing
		for (int i = 0; i < polygon.vertices.size(); i++)
		{
			glm::vec3 v0 = polygon.vertices[i];
			glm::vec3 v1 = i + 1 == polygon.vertices.size() ? polygon.vertices[0] : polygon.vertices[i + 1];

			edge = v1 - v0;
			vp = P - v0;
			C = glm::cross(edge, vp);
			if (glm::dot(N, C) < 0) {
				return false;
			}
		}

		return true;
	}


	bool IntersectTriangle(glm::vec3 orig, glm::vec3 dir,
		glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 norm,
		float* t)
	{
		glm::vec3 N = norm;

		//Find P

		//Check parralel
		float NdotRayDir = glm::dot(N, dir);
		if (glm::abs(NdotRayDir) < 1e-4)
			return false;

		float d = glm::dot(N, v0);

		//Compute T
		*t = -(glm::dot(N, orig) + d) / NdotRayDir;
		//Check if triangle is behind the ray
		if (*t < 0) return false;

		glm::vec3 P = orig + ((*t) * dir);

		glm::vec3 C, edge, vp;

		//Inside out testing
		edge = v1 - v0;
		vp = P - v0;
		C = glm::cross(edge, vp);
		if (glm::dot(N, C) < 0) return false;

		edge = v2 - v1;
		vp = P - v1;
		C = glm::cross(edge, vp);
		if (glm::dot(N, C) < 0) return false;

		edge = v0 - v2;
		vp = P - v2;
		C = glm::cross(edge, vp);
		if (glm::dot(N, C) < 0) return false;

		return true;
	}

	std::vector<float> IntersectMesh(glm::vec3 orig, glm::vec3 dir, Mesh* mesh)
	{
		std::vector<float> ret;
		for (int i = 0; i < mesh->vertices.size() / 18; i++)
		{
			glm::vec3 v0 = glm::vec3(mesh->vertices[i * 18 + 0], mesh->vertices[i * 18 + 1], mesh->vertices[i * 18 + 2]);
			glm::vec3 n0 = glm::vec3(mesh->vertices[i * 18 + 3], mesh->vertices[i * 18 + 4], mesh->vertices[i * 18 + 5]);
			glm::vec3 v1 = glm::vec3(mesh->vertices[i * 18 + 6], mesh->vertices[i * 18 + 7], mesh->vertices[i * 18 + 8]);
			glm::vec3 n1 = glm::vec3(mesh->vertices[i * 18 + 9], mesh->vertices[i * 18 + 10], mesh->vertices[i * 18 + 11]);
			glm::vec3 v2 = glm::vec3(mesh->vertices[i * 18 + 12], mesh->vertices[i * 18 + 13], mesh->vertices[i * 18 + 14]);
			glm::vec3 n2 = glm::vec3(mesh->vertices[i * 18 + 15], mesh->vertices[i * 18 + 16], mesh->vertices[i * 18 + 17]);

			float dist;
			if (ray::IntersectTriangle(orig, dir, v0, v1, v2, n0, &dist))
				if (dist > 0)
					ret.push_back(dist);
		}

		return ret;
	}
}

class Polytope {
public:
	Mesh* GeneratedMesh;
	Mesh* ngonMesh;
	std::vector<BrushPolygon> ngons;
	std::vector<float> meshData;
	std::map<int, Mesh*> meshes;

	std::vector<std::vector<int>> triangles;

	glm::vec3 NWU;
	glm::vec3 SEL;

	Polytope(std::vector<Plane> planes, bool gen_gl_mesh = true, bool dbg = false)
	{
		std::vector<glm::vec3> intersecting;

		//Set up polygon structure
		for (int i = 0; i < planes.size(); i++) {
			this->ngons.push_back(BrushPolygon(planes[i]));
		}

		// Do plane intersections
		for (int i = 0; i < planes.size(); i++) {
			for (int j = 0; j < planes.size(); j++) {
				for (int k = 0; k < planes.size(); k++) {
					if (i == j || i == k || j == k) continue; //Skip invalid checks

					glm::vec3 p(0, 0, 0);

					if (!Plane::FinalThreePlaneIntersection(planes[i], planes[j], planes[k], &p)) { continue; };

					bool valid = true;
					//Check if point is outside object
					for (int m = 0; m < planes.size(); m++)
					{
						if (Plane::EvalPointPolarity(planes[m], p) < -0.01f)
						{
							valid = false;
							break;
						}
					}

					if (!valid) continue;

					intersecting.push_back(p);

					//Add verts
					this->ngons[i].vertices.push_back(p);
					this->ngons[j].vertices.push_back(p);
					this->ngons[k].vertices.push_back(p);
				}
			}
		}

		std::vector<float> generatedMesh;

		float x = this->ngons[0].vertices[0].x;
		float _x = this->ngons[0].vertices[0].x;
		float y = this->ngons[0].vertices[0].y;
		float _y = this->ngons[0].vertices[0].y;
		float z = this->ngons[0].vertices[0].z;
		float _z = this->ngons[0].vertices[0].z;

		//Remove polygon vertex duplicates
		for (int i = 0; i < this->ngons.size(); i++) {

			std::vector<glm::vec3> newlist;

			//Cleanup and find bounds

			for (int j = 0; j < this->ngons[i].vertices.size(); j++)
			{
				bool found = false;

				for (int k = 0; k < newlist.size(); k++)
				{
					if (glm::distance(newlist[k], this->ngons[i].vertices[j]) < 0.5f) //Throw out dupe points (within half a unit)
					{
						found = true; break;
					}
				}

				if (found) continue;

				newlist.push_back(this->ngons[i].vertices[j]);

				x = this->ngons[i].vertices[j].x > x ? this->ngons[i].vertices[j].x : x;
				_x = this->ngons[i].vertices[j].x < _x ? this->ngons[i].vertices[j].x : _x;

				y = this->ngons[i].vertices[j].y > y ? this->ngons[i].vertices[j].y : y;
				_y = this->ngons[i].vertices[j].y < _y ? this->ngons[i].vertices[j].y : _y;

				z = this->ngons[i].vertices[j].z > z ? this->ngons[i].vertices[j].z : z;
				_z = this->ngons[i].vertices[j].z < _z ? this->ngons[i].vertices[j].z : _z;
			}

			this->ngons[i].vertices = newlist;

			//Reorder vertices
			if (this->ngons[i].vertices.size() < 3)
				continue;

			std::vector<glm::vec3> points = Plane::OrderCoplanarClockWise(this->ngons[i].plane, this->ngons[i].vertices);
			this->ngons[i].vertices = points;
			for (int j = 0; j < points.size() - 2; j++) {
				glm::vec3 a = points[0];
				glm::vec3 b = points[j + 1];
				glm::vec3 c = points[j + 2];

				generatedMesh.push_back(-a.x);
				generatedMesh.push_back(a.z);
				generatedMesh.push_back(a.y);

				generatedMesh.push_back(this->ngons[i].plane.normal.x);
				generatedMesh.push_back(-this->ngons[i].plane.normal.z);
				generatedMesh.push_back(-this->ngons[i].plane.normal.y);


				generatedMesh.push_back(-b.x);
				generatedMesh.push_back(b.z);
				generatedMesh.push_back(b.y);

				generatedMesh.push_back(this->ngons[i].plane.normal.x);
				generatedMesh.push_back(-this->ngons[i].plane.normal.z);
				generatedMesh.push_back(-this->ngons[i].plane.normal.y);


				generatedMesh.push_back(-c.x);
				generatedMesh.push_back(c.z);
				generatedMesh.push_back(c.y);

				generatedMesh.push_back(this->ngons[i].plane.normal.x);
				generatedMesh.push_back(-this->ngons[i].plane.normal.z);
				generatedMesh.push_back(-this->ngons[i].plane.normal.y);
			}
		}

		NWU = glm::vec3(-_x, z, y);
		SEL = glm::vec3(-x, _z, _y);

		this->meshData = generatedMesh;

		if (gen_gl_mesh)
		{
			Mesh* m = new Mesh(generatedMesh, MeshMode::POS_XYZ_NORMAL_XYZ);
			this->GeneratedMesh = m;
		}
	}
};