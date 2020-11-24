#pragma once
#include <iostream>
#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <vector>

#include <math.h>
class Plane {
public:
	glm::vec3 normal;
	float offset;

	int textureID = 0;

	//Plane defined by three points
	Plane(glm::vec3 P, glm::vec3 Q, glm::vec3 R) {
		glm::vec3 n = glm::cross(P - Q, P - R); //Calculate normal
												//float d = ((P.x * n.x + P.y * n.y + P.z * n.z) / glm::length(n)); //Calculate distance

		float d = glm::dot(glm::normalize(n), P);

		//Set class attribs
		this->normal = glm::normalize(n);
		this->offset = d;
	}

	//Direct constructor
	Plane(glm::vec3 normal, float offset) {
		this->offset = offset;
		this->normal = normal;
	}

	//Standard constructor (generic floor plane)
	Plane() {
		this->offset = 0.0f;
		this->normal = glm::vec3(0, 0, 1);
	}

	//Destructor
	~Plane() {};

	static bool ThreePlaneIntersection(Plane p1, Plane p2, Plane p3, glm::vec3* p) {
		float det = glm::dot(glm::cross(p1.normal, p2.normal), p3.normal);

		float epsilon = 1e-5f; //Epsilon value for floating point error

		if (det < epsilon && det > -epsilon) { return false; };

		*p = (-(p1.offset * glm::cross(p2.normal, p3.normal)) -
			(p2.offset * glm::cross(p3.normal, p1.normal)) -
			(p3.offset * glm::cross(p1.normal, p2.normal))) / det;

		// a = p1.offset
		// b = glm::cross(p2.normal, p3.normal)
		// c = p2.offset
		// d = glm::cross(p3.normal, p1.normal)
		// e = p3.offset
		// f = glm::cross(p1.normal, p2.normal)
		// g = det


		//*p = (  -(a * b) - (c * d) - (e * f)  ) / g



		return true;
	}

	static bool FinalThreePlaneIntersection(Plane p1, Plane p2, Plane p3, glm::vec3* p) {
		float det = glm::dot(glm::cross(p1.normal, p2.normal), p3.normal);
		float epsilon = 1e-5f; //Epsilon value for floating point error

		if (det < epsilon && det > -epsilon) { return false; };

		*p = -(-p1.offset * glm::cross(p2.normal, p3.normal)
			- p2.offset * glm::cross(p3.normal, p1.normal)
			- p3.offset * glm::cross(p1.normal, p2.normal))
			/ glm::dot(p1.normal, glm::cross(p2.normal, p3.normal));

		return true;

		/*

		p = (-adist(Cross(bnorm, cnorm))
		-bdist(Cross(cnorm, anorm))
		-cdist(Cross(anorm, bnorm)))
		/ Dot(anorm, Cross(bnorm, cnorm))

		*/

	}

	static bool GetTripleIntersection(Plane p1, Plane p2, Plane p3, glm::vec3* p) {
		float determinant = glm::determinant(glm::mat3{ p1.normal.x, p2.normal.x, p3.normal.x,
			p1.normal.y, p2.normal.y, p3.normal.y,
			p1.normal.z, p2.normal.z, p3.normal.z });

		float epsilon = 1e-5f; //Check within small epsilon to prevent infinite values
		if (determinant < epsilon && determinant > -epsilon) return false;

		glm::vec3 point = (glm::cross(p2.normal, p3.normal) * -p1.offset +
			glm::cross(p3.normal, p1.normal) * -p2.offset +
			glm::cross(p1.normal, p2.normal) * -p3.offset) / determinant;

		*p = point;
		return true;

		/*
		Determinant layout
		-- plane normals --
		\ N1  N2  N3
		x
		y
		z
		^components */
	}

	//Detects which side of the plane the point is on (-): negative, (~0): coplanar, (+): positive
	static float EvalPointPolarity(Plane plane, glm::vec3 p1) {
		return glm::dot(p1, plane.normal) - //Evaluate dot & plane normal
			glm::dot(plane.normal * plane.offset, plane.normal); //Compare to known point & plane normal (closest point to 0,0,0 is normal*offset)
	}

	//Determines clockwise-ness of point B in relation to point A with respect to center point C
	static float CompareClockWiseNess(Plane plane, glm::vec3 C, glm::vec3 A, glm::vec3 B) {
		return glm::dot(plane.normal, glm::cross(A - C, B - C));
	}

	static std::vector<glm::vec3> OrderCoplanarClockWise(Plane plane, std::vector<glm::vec3> Points) {
		//Find center point (avarage distribution of points)
		glm::vec3 center(0, 0, 0);
		for (int i = 0; i < Points.size(); i++) {
			center += Points[i];
		}
		center /= Points.size();

		glm::vec3 ref = Points[0] - center;

		std::vector<glm::vec4> angledVecs;


		for (int i = 0; i < Points.size(); i++) {
			glm::vec3 diff = Points[i] - center;
			float ang = atan2(glm::length(glm::cross(diff, ref)), glm::dot(diff, ref));

			float sign = glm::dot(glm::cross(diff, ref), plane.normal) < 0 ? -1.0f : 1.0f;
			ang *= sign;
			
			angledVecs.push_back(glm::vec4(Points[i].x, Points[i].y, Points[i].z, ang));
		}

		while (true)
		{
			bool modified = false;

			for (int i = 0; i < Points.size() - 1; i++)
			{
				int s0 = i; int s1 = i + 1;

				glm::vec4 a = angledVecs[s0]; glm::vec4 b = angledVecs[s1];

				if (a.w > b.w)
				{
					angledVecs[s0] = b; angledVecs[s1] = a;
					modified = true;
				}
			}
			if (!modified) break;
		}

		for (int i = 0; i < Points.size(); i++)
		{
			Points[i] = glm::vec3(angledVecs[i].x, angledVecs[i].y, angledVecs[i].z);
		}

		return Points;
	}

	static void InPlaceOrderCoplanarClockWise(Plane plane, std::vector<glm::vec3>* Points){
		if (Points->size() == 0) return;

		//Find center point (avarage distribution of points)
		glm::vec3 center(0, 0, 0);
		for (int i = 0; i < Points->size(); i++) {
			center += (*Points)[i];
		}
		center /= Points->size();

		glm::vec3 ref = (*Points)[0] - center;

		std::vector<glm::vec4> angledVecs;

		for (int i = 0; i < Points->size(); i++) {
			glm::vec3 diff = (*Points)[i] - center;
			float ang = atan2(glm::length(glm::cross(diff, ref)), glm::dot(diff, ref));

			float sign = glm::dot(glm::cross(diff, ref), plane.normal) < 0 ? -1.0f : 1.0f;
			ang *= sign;

			angledVecs.push_back(glm::vec4((*Points)[i].x, (*Points)[i].y, (*Points)[i].z, ang));
		}

		while (true){
			bool modified = false;

			for (int i = 0; i < Points->size() - 1; i++){
				int s0 = i; int s1 = i + 1;

				glm::vec4 a = angledVecs[s0]; glm::vec4 b = angledVecs[s1];

				if (a.w > b.w){
					angledVecs[s0] = b; angledVecs[s1] = a;
					modified = true;
				}
			}
			if (!modified) break;
		}

		for (int i = 0; i < Points->size(); i++){
			(*Points)[i] = glm::vec3(angledVecs[i].x, angledVecs[i].y, angledVecs[i].z);
		}
	}
};