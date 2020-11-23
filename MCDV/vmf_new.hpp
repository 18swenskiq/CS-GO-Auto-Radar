// io
#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include "vdf.hpp"

// stl containers
#include <vector>
#include <map>
#include <set>


// directx
#ifdef DXBUILD
#include <d3d11.h>
#include <DirectXMath.h>
#include "DXMathExtensions.h"
#endif


// opengl
#ifdef GLBUILD
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#endif


//engine (directx)
#ifdef DXBUILD
#include "Util.h"
#include "plane.h"
#include "DXMesh.h"
#include "Shader.hpp"
#include "IRenderable.hpp"
#include "interpolation.h"
#endif

//engine (opengl)
#ifdef GLBUILD
#include "Util.h"
#include "plane.h"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "IRenderable.hpp"
#include "interpolation.h"
#endif

// other
#include <limits>

// Source sdk
#include "vfilesys.hpp"

// Define whether we want DX or GL
#include "buildmode.h"

// UINT16 buffer bit definitions ================
// Byte 0
#define TAR_MIBUFFER_PLAYSPACE 0x1
#define TAR_MIBUFFER_OVERLAP 0x2
#define TAR_MIBUFFER_OBJECTIVE_B 0x4
#define TAR_MIBUFFER_OBJECTIVE_X 0x8
#define TAR_MIBUFFER_BUYZONE_CT 0x10
#define TAR_MIBUFFER_BUYZONE_T 0x20
#define TAR_MIBUFFER_BUYZONE_X 0x40
#define TAR_MIBUFFER_COVER0 0x80
// Byte 1
#define TAR_MIBUFFER_COVER1 0x100
#define TAR_MIBUFFER_MODEL 0x200
#define TAR_MIBUFFER_FUNC_DETAIL 0x400
#define TAR_MIBUFFER_NEGATIVE 0x800
#define TAR_MIBUFFER_USER0 0x1000
#define TAR_MIBUFFER_USER1 0x2000
#define TAR_MIBUFFER_USER2 0x4000
#define TAR_MIBUFFER_USER3 0x8000
// ============================================

typedef unsigned int TAR_MIBUFFER_FLAGS;
std::map<unsigned int, TAR_MIBUFFER_FLAGS> g_visgroup_flag_translations;

bool use_verbose = true;
std::string prefix = "";

inline glm::vec3 get_normal(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
	return glm::normalize(glm::cross(A - C, B - C));
}

constexpr
unsigned int hash(const char* str, int h = 0)
{
	return !str[h] ? 5381 : (hash(str, h + 1) * 33) ^ str[h];
}

//Variadic print
void _debug() {
	std::cout << std::endl;
}

template<typename First, typename ... Strings>
void _debug(First arg, const Strings&... rest) {
	std::cout << arg;
	_debug(rest...);
}

template<typename First, typename ... Strings>
void debug(First arg, const Strings&... rest) {
	if (use_verbose) {
		std::cout << prefix;
		_debug(arg, rest...);
	}
}

namespace vmf_parse {
	//Pass Vector3
	bool Vector3f(std::string str, glm::vec3* vec)
	{
		str = sutil::removeChar(str, '(');
		str = sutil::removeChar(str, ')');

		std::vector<std::string> elems = split(str, ' ');
		std::vector<float> pelems;

		for (int i = 0; i < elems.size(); i++) {
			std::string f = sutil::trim(elems[i]);

			//TODO: error check against invalid values here
			float e = ::atof(f.c_str());
			pelems.push_back(e);
		}

		if (pelems.size() == 3) {
			*vec = glm::vec3(pelems[0], pelems[1], pelems[2]);
			return true;
		}

		return false;
	}

	//Parse Vector 3 with square barackets. Thanks again, valve
	bool Vector3fS(std::string str, glm::vec3* vec)
	{
		str = sutil::removeChar(str, '[');
		str = sutil::removeChar(str, ']');

		std::vector<std::string> elems = split(str, ' ');
		std::vector<float> pelems;

		for (int i = 0; i < elems.size(); i++) {
			std::string f = sutil::trim(elems[i]);

			//TODO: error check against invalid values here
			float e = ::atof(f.c_str());
			pelems.push_back(e);
		}

		if (pelems.size() == 3) {
			*vec = glm::vec3(pelems[0], pelems[1], pelems[2]);
			return true;
		}

		return false;
	}

	//Parse plane from standard 3 point notation (ax, ay, az) (bx, by, bz) ...
	bool plane(std::string str, Plane* plane)
	{
		std::vector<std::string> points = split(str, '(');

		if (points.size() != 4) { return false; }

		glm::vec3 A, B, C;

		if (!(Vector3f(points[1], &A) && Vector3f(points[2], &B) && Vector3f(points[3], &C))) {
			return false;
		}

		*plane = Plane(A, B, C);

		return true;
	}
}

class material {
public:
	static std::map<std::string, material*> m_index;

	std::string name;
	bool draw = true;

	material(const std::string& materialname) {
		this->name = materialname;
		if (this->name == "TOOLS/TOOLSSKYBOX" ||
			this->name == "TOOLS/NODRAW")
			this->draw = false;
	}

	static material* get(const std::string& tex) {
		if (material::m_index.count(tex)) return material::m_index[tex];

		material::m_index.insert({ tex, new material(tex) });
		return material::m_index[tex];
	}
};

class dispinfo;

class side {
public:
	int m_ID;
	material* m_texture;
	Plane m_plane;
	dispinfo* m_dispinfo = NULL;
	std::vector<glm::vec3> m_vertices;

	static side* create(kv::DataBlock* dataSrc);

	void howmany() {
		debug(this->m_vertices.size());
	}
};

class dispinfo : public IRenderable{
public:
	
	#ifdef DXBUILD
	DirectX::XMFLOAT3 startposition;
	std::vector<std::vector<DirectX::XMFLOAT3>> normals;
	#endif

	#ifdef GLBUILD
	glm::vec3 startposition;
	std::vector<std::vector<glm::vec3>> normals;
	#endif


	unsigned int power;
	std::vector<std::vector<float>> distances;
	side* m_source_side = NULL;

	dispinfo(kv::DataBlock* dataSrc, side* src_side) {
		this->m_source_side = src_side;

		kv::DataBlock* kv_normals = dataSrc->_GetFirstByName("normals");
		kv::DataBlock* kv_distances = dataSrc->_GetFirstByName("distances");

		this->power = std::stoi(dataSrc->Values["power"]);
		vmf_parse::Vector3fS(dataSrc->Values["startposition"], &this->startposition);

		int i_target = glm::pow(2, this->power) + 1;

		for (int x = 0; x < i_target; x++) {
			#ifdef DXBUILD
			this->normals.push_back(std::vector<DirectX::XMFLOAT3>());
			#endif
			#ifdef GLBUILD
			this->normals.push_back(std::vector<glm::vec3>());
			#endif
			this->distances.push_back(std::vector<float>());

			// Read normals
			std::vector<float> list;
			for (auto && v : split(kv_normals->Values["row" + std::to_string(x)])) 
				list.push_back(::atof(v.c_str()));

			for (int xx = 0; xx < i_target; xx++) {
				#ifdef DXBUILD
				this->normals[x].push_back(DirectX::XMFLOAT3(list[xx * 3 + 0], list[xx * 3 + 1], list[xx * 3 + 2]));
				#endif
				#ifdef GLBUILD
				this->normals[x].push_back(glm::vec3(list[xx * 3 + 0], list[xx * 3 + 1], list[xx * 3 + 2]));
				#endif
			}

			// Read distances
			for (auto && v : split(kv_distances->Values["row" + std::to_string(x)]))
				this->distances[x].push_back(std::stof(v.c_str()));
		}
	}

	// internal draw method
	void IRenderable::_Draw(Shader* shader, std::vector<glm::mat4> transform_stack = {}) { 
		this->m_mesh->Draw();
	}

	// Compute GL Mesh
	void IRenderable::SetupDrawable() {
		if (this->m_source_side->m_vertices.size() != 4) {
			debug("Displacement info matched to face with {", this->m_source_side->m_vertices.size(), "} vertices!!!");
			return;
		}

#ifdef DXBUILD
		// Match 'starting point'
		std::map<float, DirectX::XMFLOAT3*> distancesToStart;
		for (auto && p : this->m_source_side->m_vertices)
			distancesToStart.insert({}); // find distance

		// The corners of the displacement
		DirectX::XMFLOAT3* SW = distancesToStart.begin()->second;

		// Find what point in vector it was
		int pos = 0;
		for (auto&& point : this->m_source_side->m_vertices)
			if (&point == SW) break; else pos++;

		// Get the rest of the points, in clockwise order
		DirectX::XMFLOAT3* NW = &this->m_source_side->m_vertices[(pos + 1) % 4];
		DirectX::XMFLOAT3* NE = &this->m_source_side->m_vertices[(pos + 2) % 4];
		DirectX::XMFLOAT3* SE = &this->m_source_side->m_vertices[(pos + 3) % 4];

		int points = pow(2, this->power) + 1; // was GLM::Pow, will break anything?

#endif

#ifdef GLBUILD
		// Match 'starting point'
		std::map<float, glm::vec3*> distancesToStart;
		for (auto && p : this->m_source_side->m_vertices)
			distancesToStart.insert({ glm::distance(this->startposition, p), &p });

		// The corners of displacement
		glm::vec3* SW = distancesToStart.begin()->second;

		// Find what point in vector it was
		int pos = 0;
		for (auto && point : this->m_source_side->m_vertices)
			if (&point == SW) break; else pos++;

		// Get the rest of the points, in clockwise order (they should already be sorted by polytope generation)
		glm::vec3* NW = &this->m_source_side->m_vertices[(pos + 1) % 4];
		glm::vec3* NE = &this->m_source_side->m_vertices[(pos + 2) % 4];
		glm::vec3* SE = &this->m_source_side->m_vertices[(pos + 3) % 4];

		int points = glm::pow(2, this->power) + 1; // calculate the point count (5, 9, 17)

												   // Initialize list for floats

#endif
		std::vector<float> meshData;

		#ifdef DXBUILD
		std::vector<DirectX::XMFLOAT3> finalPoints;
		std::vector<DirectX::XMFLOAT3> finalNormals;
		#endif

		#ifdef GLBUILD
		std::vector<glm::vec3> finalPoints;
		std::vector<glm::vec3> finalNormals;
		#endif


		for (int row = 0; row < points; row++) {
			for (int col = 0; col < points; col++) {
				//Generate original base points

				float dx = (float)col / (float)(points - 1); //Time values for linear interpolation
				float dy = (float)row / (float)(points - 1);

				#ifdef DXBUILD
				DirectX::XMFLOAT3 LWR = DirectX::XMVectorLerp(DirectX::XMLoadFloat3(*SW), *SE, dx);
				DirectX::XMFLOAT3 UPR = DirectX::XMVectorLerp(*NW, *NE, dx);
				DirectX::XMFLOAT3 P = DirectX::XMVectorLerp(LWR, UPR, dy);

				DirectX::XMFLOAT3 offset = this->normals[col][row] * this->distances[col][row];
				P = P + offset;
				#endif

				#ifdef GLBUILD
				glm::vec3 LWR = lerp(*SW, *SE, dx);
				glm::vec3 UPR = lerp(*NW, *NE, dx);
				glm::vec3 P = lerp(LWR, UPR, dy); // Original point location

				glm::vec3 offset = this->normals[col][row] * this->distances[col][row]; // Calculate offset
				P = P + offset; //Add offset to P
				#endif
				finalPoints.push_back(P);
			}
		}

		for (int row = 0; row < points; row++) {
			for (int col = 0; col < points; col++) {

				#ifdef DXBUILD
				std::vector<DirectX::XMFLOAT3*> kernalpts = { NULL, NULL, NULL, NULL };
				#endif

				#ifdef GLBUILD
				std::vector<glm::vec3*> kernalpts = { NULL, NULL, NULL, NULL };
				#endif


				if(row + 1 < points)	kernalpts[0] = &finalPoints[((row + 1) * points) + (col + 0)];
				if(col - 1 > 0)			kernalpts[1] = &finalPoints[((row + 0) * points) + (col - 1)];
				
				if(row - 1 > 0)			kernalpts[2] = &finalPoints[((row - 1) * points) + (col + 0)];
				if(col + 1 < points)	kernalpts[3] = &finalPoints[((row + 0) * points) + (col + 1)];

				#ifdef DXBUILD
				DirectX::XMFLOAT3* A = &finalPoints[((row + 0) * points) + (col + 0)];
				DirectX::XMFLOAT3 cNorm = DirectX::XMFLOAT3(1, 0, 0);
				#endif


				#ifdef GLBUILD
				glm::vec3* A = &finalPoints[((row + 0) * points) + (col + 0)];
				glm::vec3 cNorm = glm::vec3(1, 0, 0);
				#endif


				for (int t = 0; t < 1; t++) {
					#ifdef DXBUILD
					DirectX::XMFLOAT3* B = kernalpts[(t + 0) % 4];
					DirectX::XMFLOAT3* C = kernalpts[(t + 1) % 4];
					#endif


					#ifdef GLBUILD
					glm::vec3* B = kernalpts[(t + 0) % 4];
					glm::vec3* C = kernalpts[(t + 1) % 4];
					#endif

					if ((B != NULL) && (C != NULL)) {

						#ifdef DXBUILD

						// Subtract A - C
						DirectX::XMFLOAT3 v0 = DXME::SubtractFloat3(*A, *C);
						DirectX::XMFLOAT3 v1 = DXME::SubtractFloat3(*B, *C);
						DirectX::XMFLOAT3 n = DXME::CrossFloat3(v0, v1);
						cNorm = DXME::AddFloat3(cNorm, DXME::NormalizeFloat3(n));
						#endif

						#ifdef GLBUILD
						glm::vec3 v0 = *A - *C;
						glm::vec3 v1 = *B - *C;
						glm::vec3 n = glm::cross(v0, v1);
						cNorm += glm::normalize(n);
						#endif
					}
				}
				#ifdef DXBUILD
				finalNormals.push_back(DXME::NormalizeFloat3(cNorm));
				#endif

				#ifdef GLBUILD
				finalNormals.push_back(glm::normalize(cNorm));
				#endif
			}
		}

		int i_condition = 0;
		for (int row = 0; row < points - 1; row++) {
			for (int col = 0; col < points - 1; col++) {
				// Gather point pointers
				// hehe :(

#ifdef DXBUILD
				DirectX::XMVECTOR* SW = &finalPoints[((row + 0) * points) + (col + 0)];
#endif



#ifdef GLBUILD
				glm::vec3* SW	=	&finalPoints	[((row + 0) * points) + (col + 0)];
				glm::vec3* SW_N =	&finalNormals	[((row + 0) * points) + (col + 0)];
				glm::vec3* SE	=	&finalPoints	[((row + 0) * points) + (col + 1)];
				glm::vec3* SE_N =	&finalNormals	[((row + 0) * points) + (col + 1)];
				glm::vec3* NW	=	&finalPoints	[((row + 1) * points) + (col + 0)];
				glm::vec3* NW_N =	&finalNormals	[((row + 1) * points) + (col + 0)];
				glm::vec3* NE	=	&finalPoints	[((row + 1) * points) + (col + 1)];
				glm::vec3* NE_N =	&finalNormals	[((row + 1) * points) + (col + 1)];
#endif
				

				// Insert triangles.
				if (i_condition++ % 2 == 0) {//Condition 0
					glm::vec3 n1 = get_normal(*SW, *NW, *NE);
					meshData.push_back(-NE->x);
					meshData.push_back(NE->z);
					meshData.push_back(NE->y);
					meshData.push_back(-n1.x);
					meshData.push_back(n1.z);
					meshData.push_back(n1.y);

					meshData.push_back(-NW->x);
					meshData.push_back(NW->z);
					meshData.push_back(NW->y);
					meshData.push_back(-n1.x);
					meshData.push_back(n1.z);
					meshData.push_back(n1.y);

					meshData.push_back(-SW->x);
					meshData.push_back(SW->z);
					meshData.push_back(SW->y);
					meshData.push_back(-n1.x);
					meshData.push_back(n1.z);
					meshData.push_back(n1.y);

					glm::vec3 n2 = get_normal(*SW, *NE, *SE);
					meshData.push_back(-SE->x);
					meshData.push_back(SE->z);
					meshData.push_back(SE->y);
					meshData.push_back(-n2.x);
					meshData.push_back(n2.z);
					meshData.push_back(n2.y);

					meshData.push_back(-NE->x);
					meshData.push_back(NE->z);
					meshData.push_back(NE->y);
					meshData.push_back(-n2.x);
					meshData.push_back(n2.z);
					meshData.push_back(n2.y);

					meshData.push_back(-SW->x); // tri2
					meshData.push_back(SW->z);
					meshData.push_back(SW->y);
					meshData.push_back(-n2.x);
					meshData.push_back(n2.z);
					meshData.push_back(n2.y);
				}
				else { //Condition 1
					glm::vec3 n1 = get_normal(*SW, *NW, *SE);
					meshData.push_back(-SE->x);
					meshData.push_back(SE->z);
					meshData.push_back(SE->y);
					meshData.push_back(-n1.x);
					meshData.push_back(n1.z);
					meshData.push_back(n1.y);

					meshData.push_back(-NW->x);
					meshData.push_back(NW->z);
					meshData.push_back(NW->y);
					meshData.push_back(-n1.x);
					meshData.push_back(n1.z);
					meshData.push_back(n1.y);

					meshData.push_back(-SW->x);
					meshData.push_back(SW->z);
					meshData.push_back(SW->y);
					meshData.push_back(-n1.x);
					meshData.push_back(n1.z);
					meshData.push_back(n1.y);


					glm::vec3 n2 = get_normal(*NW, *NE, *SE);
					meshData.push_back(-SE->x);
					meshData.push_back(SE->z);
					meshData.push_back(SE->y);
					meshData.push_back(-n2.x);
					meshData.push_back(n2.z);
					meshData.push_back(n2.y);

					meshData.push_back(-NE->x);
					meshData.push_back(NE->z);
					meshData.push_back(NE->y);
					meshData.push_back(-n2.x);
					meshData.push_back(n2.z);
					meshData.push_back(n2.y);

					meshData.push_back(-NW->x); //tri2
					meshData.push_back(NW->z);
					meshData.push_back(NW->y);
					meshData.push_back(-n2.x);
					meshData.push_back(n2.z);
					meshData.push_back(n2.y);
				}

			}
			i_condition++;
		}


		#ifdef DXBUILD
		// Might be fucky with pointers
		this->m_mesh = new DXMesh(dxr, meshData, DXMeshMode::POS_XYZ_NORMAL_XYZ);
		#endif
		#ifdef GLBUILD
		this->m_mesh = new Mesh(meshData, MeshMode::POS_XYZ_NORMAL_XYZ);
		#endif
	}
};

side* side::create(kv::DataBlock* dataSrc) {
	side* s = new side();
	s->m_ID = ::atof(dataSrc->Values["id"].c_str());
	s->m_texture = material::get(dataSrc->Values["material"]);

	if (!vmf_parse::plane(dataSrc->Values["plane"], &s->m_plane)) return s;

	kv::DataBlock* kv_dispInfo = dataSrc->_GetFirstByName("dispinfo");
	if (kv_dispInfo != NULL) s->m_dispinfo = new dispinfo(kv_dispInfo, s);
	return s;
}

class vmf;

class editorvalues {
public:
	std::vector<unsigned int> m_visgroups;
	glm::vec3 m_editorcolor;

	TAR_MIBUFFER_FLAGS m_miflags;

	editorvalues(){}
	editorvalues(kv::DataBlock* dataSrc) {
		if (dataSrc == NULL) return;

		for (auto && vgroup : kv::getList(dataSrc->Values, "visgroupid")) {
			unsigned int vgroupid = std::stoi(vgroup);
			this->m_visgroups.push_back(vgroupid);

			if (g_visgroup_flag_translations.count(vgroupid))
				this->m_miflags |= g_visgroup_flag_translations[vgroupid];
		}
	}
};

class solid : public IRenderable {
public:
	std::vector<side*> m_sides;
	editorvalues m_editorvalues;
	glm::vec3 NWU;
	glm::vec3 SEL;

	solid(kv::DataBlock* dataSrc) {
		// Read editor values
		this->m_editorvalues = editorvalues(dataSrc->_GetFirstByName("editor"));

		// Read solids
		for (auto && s : dataSrc->_GetAllByName("side")) {
			m_sides.push_back(side::create(s));
		}

		// Process polytope problem. (still questionable why this is a thing)
		std::vector<glm::vec3> intersecting;

		float x, _x, y, _y, z, _z;
		x = _y = _z = 99999.0f;// std::numeric_limits<float>::max();
		_x = y = z = -99999.0f;// std::numeric_limits<float>::min();

		for (int i = 0; i < m_sides.size(); i++) {
			for (int j = 0; j < m_sides.size(); j++) {
				for (int k = 0; k < m_sides.size(); k++) {
					// skip common planes
					if (i == j || i == k || j == k) continue;

					// Calculate intersection of 3 planes
					// will return false if unable to solve (planes are parralel)
					glm::vec3 p(0, 0, 0);
					if (!Plane::FinalThreePlaneIntersection(
						this->m_sides[i]->m_plane,
						this->m_sides[j]->m_plane,
						this->m_sides[k]->m_plane,
						&p)) continue;

					// Check if we are part of the solid using simple polarity checks
					bool inbounds = true;
					for (auto && m : this->m_sides) {
						if (Plane::EvalPointPolarity(m->m_plane, p) < -0.01f) {
							inbounds = false;
							break;
						}
					} if (!inbounds) continue;

					// Check if there is already a very similar vertex, and skip it
					bool similar = false;
					for (auto && v : intersecting) 
						if (glm::distance(v, p) < 0.5f) {
							similar = true; break;
					}
					if (similar) continue;

					// Add points to all surfaces
					this->m_sides[i]->m_vertices.push_back(p);
					this->m_sides[j]->m_vertices.push_back(p);
					this->m_sides[k]->m_vertices.push_back(p);

					intersecting.push_back(p);

					// Calculate bounds
					_x = glm::round(glm::max(_x, p.x));
					_y = glm::round(glm::min(_y, p.y));
					_z = glm::round(glm::min(_z, p.z));
					x = glm::round(glm::min(x, p.x));
					y = glm::round(glm::max(y, p.y));
					z = glm::round(glm::max(z, p.z));
				}
			}
		}

		for (auto && side : this->m_sides) {
			// Sort out deez rascals
			Plane::InPlaceOrderCoplanarClockWise(side->m_plane, &side->m_vertices);
		}

		// Append bounds data
		this->NWU = glm::vec3(-x, z, y);
		this->SEL = glm::vec3(-_x, _z, _y);
	}

	/* Check if this solid contains any displacement infos. */
	bool containsDisplacements() {
		for (auto && s : this->m_sides) {
			if (s->m_dispinfo != NULL)
				return true;
		}
		return false;
	}

	void IRenderable::_Draw(Shader* shader, std::vector<glm::mat4> transform_stack = {}) {
		bool dispDrawn = false;
		for (auto && s : this->m_sides) {
			if (s->m_dispinfo != NULL) {
				s->m_dispinfo->Draw(shader);
				dispDrawn = true;
			}
		}

		// Only draw solid if thre is no displacement info
		if (!dispDrawn) {
			this->m_mesh->Draw();
		}
	}

	void IRenderable::SetupDrawable() {
		std::vector<float> verts;
		for (auto && s : this->m_sides) {
			if (s->m_dispinfo != NULL) continue;
			if (s->m_vertices.size() < 3) continue;
			if (!s->m_texture->draw) continue;

			for (int j = 0; j < s->m_vertices.size() - 2; j++) {
				glm::vec3* c = &s->m_vertices[0];
				glm::vec3* b = &s->m_vertices[j + 1];
				glm::vec3* a = &s->m_vertices[j + 2];

				verts.push_back(-a->x);
				verts.push_back(a->z);
				verts.push_back(a->y);

				verts.push_back(s->m_plane.normal.x);
				verts.push_back(-s->m_plane.normal.z);
				verts.push_back(-s->m_plane.normal.y);

				verts.push_back(-b->x);
				verts.push_back(b->z);
				verts.push_back(b->y);

				verts.push_back(s->m_plane.normal.x);
				verts.push_back(-s->m_plane.normal.z);
				verts.push_back(-s->m_plane.normal.y);

				verts.push_back(-c->x);
				verts.push_back(c->z);
				verts.push_back(c->y);

				verts.push_back(s->m_plane.normal.x);
				verts.push_back(-s->m_plane.normal.z);
				verts.push_back(-s->m_plane.normal.y);
			}
		}
#ifdef DXBUILD
		this->m_mesh = new DXMesh(dxr, verts, DXMeshMode::POS_XYZ_NORMAL_XYZ);
#endif

#ifdef GLBUILD
		this->m_mesh = new Mesh(verts, MeshMode::POS_XYZ_NORMAL_XYZ);
#endif
	}
};

class entity {
public:
	std::string m_classname;
	int m_id;
	std::map<std::string, std::string> m_keyvalues;
	editorvalues m_editorvalues;
	std::vector<solid> m_internal_solids;
	glm::vec3 m_origin;

	entity (kv::DataBlock* dataSrc) {
		

		if ((dataSrc->_GetFirstByName("solid") == NULL) && (dataSrc->Values.count("origin") == 0))
			throw std::exception(("origin could not be resolved for entity ID: " + dataSrc->Values["id"]).c_str());

		this->m_classname = dataSrc->Values["classname"];
		this->m_id = (int)::atof(dataSrc->Values["id"].c_str());
		this->m_keyvalues = dataSrc->Values;
		this->m_editorvalues = editorvalues(dataSrc->_GetFirstByName("editor"));
		
		if (dataSrc->_GetFirstByName("solid") == NULL) {
			vmf_parse::Vector3f(dataSrc->Values["origin"], &this->m_origin);
			this->m_origin = glm::vec3(-this->m_origin.x, this->m_origin.z, this->m_origin.y);
		}
		else {
			for (auto && s : dataSrc->_GetAllByName("solid")) {
				this->m_internal_solids.push_back(solid(s));
			}

			// Calculate origin
			glm::vec3 NWU = this->m_internal_solids[0].NWU;
			glm::vec3 SEL = this->m_internal_solids[0].SEL;
			for (auto && i : this->m_internal_solids) {
				NWU.z = glm::max(NWU.z, i.NWU.z);
				NWU.y = glm::max(NWU.y, i.NWU.y);
				NWU.x = glm::max(NWU.x, i.NWU.x);
				SEL.z = glm::min(SEL.z, i.SEL.z);
				SEL.y = glm::min(SEL.y, i.SEL.y);
				SEL.x = glm::min(SEL.x, i.SEL.x);
			}

			this->m_origin = (NWU + SEL) * 0.5f;
		}
	}
};

struct BoundingBox {
	glm::vec3 NWU;
	glm::vec3 SEL;
};

bool check_in_whitelist(std::vector<unsigned int>* visgroups_in, std::set<unsigned int> filter) {
	if (filter.count(0xBEEEEEEE)) return true;

	for (auto && vgroup : *visgroups_in)
		if (filter.count(vgroup))
			return true;

	return false;
}

class vmf {
private:
	static vfilesys* s_fileSystem;
	
public:
	// Static setup functions
	static void LinkVFileSystem(vfilesys* sys) {
		vmf::s_fileSystem = sys;
	}

	vmf() {}

	std::vector<solid> m_solids;
	std::vector<entity> m_entities;

	std::map<std::string, unsigned int> m_visgroups;

	std::set<unsigned int> m_whitelist_visgroups;
	std::set<std::string> m_whitelist_classnames;
	float m_render_h_max = 10000.0f;
	float m_render_h_min = -10000.0f;
	
#ifdef DXBUILD
	static std::map<std::string, DXMesh*> s_model_dict;
#endif

#ifdef GLBUILD
	static std::map<std::string, Mesh*> s_model_dict;
#endif

	void LinkVisgroupFlagTranslations(std::map<std::string, TAR_MIBUFFER_FLAGS> map) {
		for (auto && translation : map) {
			if (this->m_visgroups.count(translation.first)) {
				g_visgroup_flag_translations.insert({ this->m_visgroups[translation.first], translation.second });
			}
		}
	}

	static vmf* from_file(const std::string& path, std::map<std::string, TAR_MIBUFFER_FLAGS> translations = {}) {
		vmf* v = new vmf();
		prefix = "vmf [" + path + "] ";
		use_verbose = true;
		debug("Opening");

		std::ifstream ifs(path);
		if (!ifs) throw std::exception("355 VMF File read error.");

		std::string file_str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

		debug("Processing VMF data");
		kv::FileData file_kv(file_str);

		debug("Processing visgroups");
		// Process visgroup list
		for (auto && vg : file_kv.headNode._GetFirstByName("visgroups")->_GetAllByName("visgroup")) {
			v->m_visgroups.insert({ vg->Values["name"], std::stoi(vg->Values["visgroupid"]) });
			std::cout << "'" << vg->Values["name"] << "': " << std::stoi(vg->Values["visgroupid"]) << "\n";
		}
		v->LinkVisgroupFlagTranslations(translations);

		debug("Processing solids");
		// Solids
		for (auto && kv_solid : file_kv.headNode._GetFirstByName("world")->_GetAllByName("solid")) {
			v->m_solids.push_back(solid(kv_solid));
		}

		debug("Processing entities");
		// Entities
		for (auto && kv_entity : file_kv.headNode._GetAllByName("entity")) {
			try {
				entity ent = entity(kv_entity);
				v->m_entities.push_back(ent);
			} catch (std::exception e) {
				debug("374 ENTITY::EXCEPTION ( ", e.what(), ") ");
			}
		}

		debug("Done!");
		return v;
	}

	void InitModelDict() {
		for (auto && i : this->m_entities) {
			switch (hash(i.m_classname.c_str())) {
			case hash("prop_static"):
			case hash("prop_dynamic"):
			case hash("prop_physics"):

				std::string modelName = kv::tryGetStringValue(i.m_keyvalues, "model", "error.mdl");
				std::string baseName = split(modelName, ".")[0];
				if (vmf::s_model_dict.count(modelName)) continue; // Skip already defined models

				vtx_mesh* vtx = vmf::s_fileSystem->get_resource_handle<vtx_mesh>(baseName + ".dx90.vtx");
				vvd_data* vvd = vmf::s_fileSystem->get_resource_handle<vvd_data>(baseName + ".vvd");

				if (vvd == NULL || vtx == NULL) {
					debug( "Failed to load resource: ", baseName, "\n");
					continue;
				}

				// GENERATE MESH TING
				std::vector<float> meshData;
				for (auto && vert : vtx->vertexSequence) {
					meshData.push_back(vvd->verticesLOD0[vert].m_vecPosition.x);
					meshData.push_back(vvd->verticesLOD0[vert].m_vecPosition.y);
					meshData.push_back(vvd->verticesLOD0[vert].m_vecPosition.z);
					meshData.push_back(-vvd->verticesLOD0[vert].m_vecNormal.x);
					meshData.push_back(vvd->verticesLOD0[vert].m_vecNormal.z);
					meshData.push_back(vvd->verticesLOD0[vert].m_vecNormal.y);
				}

				// TODO: Make a DX mesh somehow
				#ifdef GLBUILD
				vmf::s_model_dict.insert({ modelName, new Mesh(meshData, MeshMode::POS_XYZ_NORMAL_XYZ) }); // Add to our list
				#endif
				#ifdef DXBUILD
				vmf::s_model_dict.insert({ modelName, new DXMesh(dxr, meshData, DXMeshMode::POS_XYZ_NORMAL_XYZ) }); // Add to our list
				#endif
				break;
			}
		}
	}

	void SetFilters(std::set<std::string> visgroups, std::set<std::string> classnames){
		this->m_whitelist_visgroups = std::set<unsigned int>{};
		if (visgroups.size() == 0) this->m_whitelist_visgroups.insert(0xBEEEEEEE);
		
		for (auto && vname : visgroups)
			if (this->m_visgroups.count(vname))
				this->m_whitelist_visgroups.insert(this->m_visgroups[vname]);

		this->m_whitelist_classnames = classnames;
	}

	void SetMinMax(float min, float max) {
		this->m_render_h_min = min;
		this->m_render_h_max = max;
	}

	void DrawWorld(Shader* shader, std::vector<glm::mat4> transform_stack = {}, unsigned int infoFlags = 0x00) {
		glm::mat4 model = glm::mat4();
		shader->setMatrix("model", model);
		shader->setUnsigned("Info", infoFlags);

		// Draw solids
		for (auto && solid : this->m_solids) {
			if (solid.NWU.y < this->m_render_h_max || solid.NWU.y > this->m_render_h_min) continue;

			if (check_in_whitelist(&solid.m_editorvalues.m_visgroups, this->m_whitelist_visgroups)) {
				shader->setUnsigned("Info", infoFlags);
				glm::vec2 orgin = glm::vec2(solid.NWU.x + solid.SEL.x, solid.NWU.z + solid.SEL.z) / 2.0f;
				shader->setVec2("origin", glm::vec2(orgin.x, orgin.y));
				solid.Draw(shader);
			}
		}

		model = glm::mat4();
		shader->setMatrix("model", model);
		// Draw 
	}

	void DrawEntities(Shader* shader, std::vector<glm::mat4> transform_stack = {}, unsigned int infoFlags = 0x00) {
		glm::mat4 model = glm::mat4();
		shader->setMatrix("model", model);
		shader->setUnsigned("Info", infoFlags);

		// Draw props
		for (auto && ent : this->m_entities) {
			// Visgroup pre-check
			if (check_in_whitelist(&ent.m_editorvalues.m_visgroups, this->m_whitelist_visgroups)) {
				if (this->m_whitelist_classnames.count(ent.m_classname)) {
					if (ent.m_classname == "prop_static" ||
						ent.m_classname == "prop_dynamic" ||
						ent.m_classname == "prop_physics" ) {
						if (ent.m_origin.y > this->m_render_h_min || ent.m_origin.y < this->m_render_h_max) continue;

						model = glm::mat4();
						model = glm::translate(model, ent.m_origin);
						glm::vec3 rot;
						vmf_parse::Vector3f(kv::tryGetStringValue(ent.m_keyvalues, "angles", "0 0 0"), &rot);
						model = glm::rotate(model, glm::radians(rot.y), glm::vec3(0, 1, 0)); // Yaw 
						model = glm::rotate(model, glm::radians(rot.x), glm::vec3(0, 0, 1)); // ROOOOOLLLLL
						model = glm::rotate(model, -glm::radians(rot.z), glm::vec3(1, 0, 0)); // Pitch 
						model = glm::scale(model, glm::vec3(::atof(kv::tryGetStringValue(ent.m_keyvalues, "uniformscale", "1").c_str())));
						shader->setMatrix("model", model);
						shader->setUnsigned("Info", infoFlags);
						shader->setVec2("origin", glm::vec2(ent.m_origin.x, ent.m_origin.z));

						if(vmf::s_model_dict.count(kv::tryGetStringValue(ent.m_keyvalues, "model", "error.mdl")))
							vmf::s_model_dict[kv::tryGetStringValue(ent.m_keyvalues, "model", "error.mdl")]->Draw();
					}
					else {
						model = glm::mat4();
						shader->setMatrix("model", model);
						shader->setUnsigned("Info", infoFlags);

						for (auto && s : ent.m_internal_solids) {
							if (s.NWU.y > this->m_render_h_min || s.NWU.y < this->m_render_h_max) continue;
							shader->setVec2("origin", glm::vec2(ent.m_origin.x, ent.m_origin.z));
							s.Draw(shader);
						}
					}
				}
			}
		}

		// Resets 
		model = glm::mat4();
		shader->setMatrix("model", model);
		shader->setUnsigned("Info", infoFlags);
	}

	BoundingBox getVisgroupBounds(const std::string& visgroup) {
		BoundingBox bounds;
		if (!this->m_visgroups.count(visgroup)) return bounds;

		unsigned int vgroup = this->m_visgroups[visgroup];

		bounds.NWU = glm::vec3(
			-999999.0f,
			-999999.0f,
			-999999.0f);

		bounds.SEL = glm::vec3(
			999999.0f,
			999999.0f,
			999999.0f);

		for (auto && iSolid : this->m_solids) {
			if (!check_in_whitelist(&iSolid.m_editorvalues.m_visgroups, std::set<unsigned int>{ vgroup })) continue;
			if (iSolid.NWU.z > bounds.NWU.z) bounds.NWU.z = iSolid.NWU.z;
			if (iSolid.NWU.y > bounds.NWU.y) bounds.NWU.y = iSolid.NWU.y;
			if (iSolid.NWU.x > bounds.NWU.x) bounds.NWU.x = iSolid.NWU.x;

			if (iSolid.SEL.z < bounds.SEL.z) bounds.SEL.z = iSolid.SEL.z;
			if (iSolid.SEL.y < bounds.SEL.y) bounds.SEL.y = iSolid.SEL.y;
			if (iSolid.SEL.x < bounds.SEL.x) bounds.SEL.x = iSolid.SEL.x;
		}

		std::cout << "Bounds MAXY: " << bounds.NWU.y << "\n";
		std::cout << "Bounds MINY: " << bounds.SEL.y << "\n";

		return bounds;
	}

	glm::vec3* calculateSpawnAVG_PMIN(const std::string& classname) {
		std::vector<entity*> spawns = this->get_entities_by_classname(classname);

		if (spawns.size() <= 0) return NULL;

		//Find lowest priority (highest)
		int lowest = kv::tryGetValue<int>(spawns[0]->m_keyvalues, "priority", 0);
		for (auto && s : spawns) {
			int l = kv::tryGetValue<int>(s->m_keyvalues, "priority", 0);
			lowest = l < lowest ? l : lowest;
		}

		//Collect all spawns with that priority
		glm::vec3* location = new glm::vec3();
		int c = 0;
		for (auto && s : spawns) {
			if (kv::tryGetValue<int>(s->m_keyvalues, "priority", 0) == lowest) {
				*location += s->m_origin; c++;
			}
		}

		//avg
		*location = *location / (float)c;
		return location;
	}

	std::vector<entity*> get_entities_by_classname(const std::string& classname) {
		std::vector<entity*> ents;
		for (auto && i : this->m_entities) {
			if (i.m_classname == classname) {
				ents.push_back(&i);
			}
		}

		return ents;
	}
};

vfilesys* vmf::s_fileSystem = NULL;
#ifdef DXBUILD
std::map<std::string, DXMesh*> vmf::s_model_dict;
#endif

#ifdef GLBUILD
std::map<std::string, Mesh*> vmf::s_model_dict;
#endif


std::map<std::string, material*> material::m_index;