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

// opengl
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

//engine
#include "Util.h"
#include "plane.h"
#include "Mesh.hpp"
#include "Shader.hpp"
#include "IRenderable.hpp"
#include "interpolation.h"

// other
#include <limits>
#include "loguru.hpp"

// Source sdk
#include "vfilesys.hpp"

#undef min
#undef max

inline glm::vec3 get_normal(const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
	return glm::normalize(glm::cross(A - C, B - C));
}

// Append a vector 3 to a float array.
inline void push_vec3(const glm::vec3& P, std::vector<float>* vec) {
	vec->push_back(P.x); vec->push_back(P.y); vec->push_back(P.z);
}

// Push boundaries
inline void boundary_extend(glm::vec3* NWU, glm::vec3* SEL, const glm::vec3& eNWU, const glm::vec3& eSEL) {
	NWU->z = glm::max(NWU->z, eNWU.z);
	NWU->y = glm::max(NWU->y, eNWU.y);
	NWU->x = glm::max(NWU->x, eNWU.x);
	SEL->z = glm::min(SEL->z, eSEL.z);
	SEL->y = glm::min(SEL->y, eSEL.y);
	SEL->x = glm::min(SEL->x, eSEL.x);
}

constexpr
unsigned int hash(const char* str, int h = 0){
	return !str[h] ? 5381 : (hash(str, h + 1) * 33) ^ str[h];
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
};

class dispinfo: public IRenderable{
public:
	unsigned int power;
	glm::vec3 startposition;

	std::vector<std::vector<glm::vec3>> normals;
	std::vector<std::vector<float>> distances;

	side* m_source_side = NULL;
	Mesh* m_mesh;

	dispinfo(kv::DataBlock* dataSrc, side* src_side) {
		this->m_source_side = src_side;

		kv::DataBlock* kv_normals = dataSrc->GetFirstByName("normals");
		kv::DataBlock* kv_distances = dataSrc->GetFirstByName("distances");

		this->power = std::stoi(dataSrc->Values["power"]);
		vmf_parse::Vector3fS(dataSrc->Values["startposition"], &this->startposition);

		int i_target = glm::pow(2, this->power) + 1;

		for (int x = 0; x < i_target; x++) {
			this->normals.push_back(std::vector<glm::vec3>());
			this->distances.push_back(std::vector<float>());

			// Read normals
			std::vector<float> list;
			for (auto && v : split(kv_normals->Values["row" + std::to_string(x)])) 
				list.push_back(::atof(v.c_str()));

			for (int xx = 0; xx < i_target; xx++) {
				this->normals[x].push_back(
					glm::vec3(
						list[xx * 3 + 0],
						list[xx * 3 + 1],
						list[xx * 3 + 2])
				);
			}

			// Read distances
			for (auto && v : split(kv_distances->Values["row" + std::to_string(x)]))
				this->distances[x].push_back(std::stof(v.c_str()));
		}
	}

	// internal draw method
	void IRenderable::_Draw(Shader* shader) { 
		this->m_mesh->Draw();
	}

	// Compute GL Mesh
	void IRenderable::_Init() {
		if (this->m_source_side->m_vertices.size() != 4) {
			LOG_F(WARNING, "Displacement matched to: %u vertices.", this->m_source_side->m_vertices.size());
			return;
		}

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
		std::vector<float> meshData;

		std::vector<glm::vec3> finalPoints;
		std::vector<glm::vec3> finalNormals;

		for (int row = 0; row < points; row++) {
			for (int col = 0; col < points; col++) {
				//Generate original base points

				float dx = (float)col / (float)(points - 1); //Time values for linear interpolation
				float dy = (float)row / (float)(points - 1);

				glm::vec3 LWR = lerp(*SW, *SE, dx);
				glm::vec3 UPR = lerp(*NW, *NE, dx);
				glm::vec3 P = lerp(LWR, UPR, dy); // Original point location

				glm::vec3 offset = this->normals[col][row] * this->distances[col][row]; // Calculate offset
				P = P + offset; //Add offset to P

				finalPoints.push_back(P);
			}
		}

		for (int row = 0; row < points; row++) {
			for (int col = 0; col < points; col++) {
				std::vector<glm::vec3*> kernalpts = { NULL, NULL, NULL, NULL };

				if(row + 1 < points)	kernalpts[0] = &finalPoints[((row + 1) * points) + (col + 0)];
				if(col - 1 > 0)			kernalpts[1] = &finalPoints[((row + 0) * points) + (col - 1)];
				
				if(row - 1 > 0)			kernalpts[2] = &finalPoints[((row - 1) * points) + (col + 0)];
				if(col + 1 < points)	kernalpts[3] = &finalPoints[((row + 0) * points) + (col + 1)];

				glm::vec3* A = &finalPoints[((row + 0) * points) + (col + 0)];
				glm::vec3 cNorm = glm::vec3(1, 0, 0);

				for (int t = 0; t < 1; t++) {
					glm::vec3* B = kernalpts[(t + 0) % 4];
					glm::vec3* C = kernalpts[(t + 1) % 4];

					if ((B != NULL) && (C != NULL)) {
						glm::vec3 v0 = *A - *C;
						glm::vec3 v1 = *B - *C;
						glm::vec3 n = glm::cross(v0, v1);
						cNorm += glm::normalize(n);
					}
				}

				finalNormals.push_back(glm::normalize(cNorm));
			}
		}

		int i_condition = 0;
		for (int row = 0; row < points - 1; row++) {
			for (int col = 0; col < points - 1; col++) {
				// Gather point pointers
				// hehe :(
				glm::vec3* SW	=	&finalPoints	[((row + 0) * points) + (col + 0)];
				glm::vec3* SW_N =	&finalNormals	[((row + 0) * points) + (col + 0)];
				glm::vec3* SE	=	&finalPoints	[((row + 0) * points) + (col + 1)];
				glm::vec3* SE_N =	&finalNormals	[((row + 0) * points) + (col + 1)];
				glm::vec3* NW	=	&finalPoints	[((row + 1) * points) + (col + 0)];
				glm::vec3* NW_N =	&finalNormals	[((row + 1) * points) + (col + 0)];
				glm::vec3* NE	=	&finalPoints	[((row + 1) * points) + (col + 1)];
				glm::vec3* NE_N =	&finalNormals	[((row + 1) * points) + (col + 1)];

#pragma region lots of triangles
				// Insert triangles.
				if (i_condition++ % 2 == 0) { //Condition 0
					glm::vec3 n1 = get_normal(*SW, *NW, *NE);
					push_vec3(*NE, &meshData);
					push_vec3(n1,  &meshData);

					push_vec3(*NW, &meshData);
					push_vec3(n1,  &meshData);

					push_vec3(*SW, &meshData);
					push_vec3(n1,  &meshData);

					glm::vec3 n2 = get_normal(*SW, *NE, *SE);
					push_vec3(*SE, &meshData);
					push_vec3(n2,  &meshData);

					push_vec3(*NE, &meshData);
					push_vec3(n2,  &meshData);

					push_vec3(*SW, &meshData);
					push_vec3(n2,  &meshData);
				}
				else { //Condition 1
					glm::vec3 n1 = get_normal(*SW, *NW, *SE);
					push_vec3(*SE, &meshData);
					push_vec3(n1,  &meshData);

					push_vec3(*NW, &meshData);
					push_vec3(n1,  &meshData);

					push_vec3(*SW, &meshData);
					push_vec3(n1,  &meshData);

					glm::vec3 n2 = get_normal(*NW, *NE, *SE);
					push_vec3(*SE, &meshData);
					push_vec3(n2,  &meshData);

					push_vec3(*NE, &meshData);
					push_vec3(n2,  &meshData);

					push_vec3(*NW, &meshData);
					push_vec3(n2,  &meshData);
				}
#pragma endregion
			}
			i_condition++;
		}

		this->m_mesh = new Mesh(meshData, MeshMode::POS_XYZ_NORMAL_XYZ);
	}
};

side* side::create(kv::DataBlock* dataSrc) {
	side* s = new side();
	s->m_ID = ::atof(dataSrc->Values["id"].c_str());
	s->m_texture = material::get(dataSrc->Values["material"]);

	if (!vmf_parse::plane(dataSrc->Values["plane"], &s->m_plane)) return s;

	kv::DataBlock* kv_dispInfo = dataSrc->GetFirstByName("dispinfo");
	if (kv_dispInfo != NULL) s->m_dispinfo = new dispinfo(kv_dispInfo, s);
	return s;
}

class vmf;

class editorvalues {
public:
	std::vector<unsigned int> m_visgroups;
	glm::vec3 m_editorcolor;

	editorvalues(){}
	editorvalues(kv::DataBlock* dataSrc) {
		if (dataSrc == NULL) return;

		for (auto && vgroup : kv::getList(dataSrc->Values, "visgroupid")) {
			unsigned int vgroupid = std::stoi(vgroup);
			this->m_visgroups.push_back(vgroupid);
		}

#ifdef VMF_READ_SOLID_COLORS
		if (vmf_parse::Vector3f(dataSrc->Values["color"], &this->m_editorcolor))
			this->m_editorcolor = this->m_editorcolor / 255.0f;
		else
			this->m_editorcolor = glm::vec3(1, 0, 0);
#endif
	}
};

class solid: public IRenderable {
public:
	std::vector<side*> m_sides;
	editorvalues m_editorvalues;
	glm::vec3 NWU;
	glm::vec3 SEL;

	Mesh* m_mesh;

	solid(kv::DataBlock* dataSrc) {
		// Read editor values
		this->m_editorvalues = editorvalues(dataSrc->GetFirstByName("editor"));

		// Read solids
		for (auto && s : dataSrc->GetAllByName("side")) {
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
					x =  glm::round(glm::min(x,  p.x));
					y =  glm::round(glm::max(y,  p.y));
					z =  glm::round(glm::max(z,  p.z));
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

	void IRenderable::_Draw(Shader* shader) {
		// Loop through solid faces, if there is a displacement draw it
		bool dispDrawn = false;
		for (auto&& s: this->m_sides) {
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

	void IRenderable::_Init() {
		std::vector<float> verts;
		for (auto && s : this->m_sides) {
			if (s->m_dispinfo != NULL) continue;
			if (s->m_vertices.size() < 3) continue;
			if (!s->m_texture->draw) continue;

			for (int j = 0; j < s->m_vertices.size() - 2; j++) {
				glm::vec3* c = &s->m_vertices[0];
				glm::vec3* b = &s->m_vertices[j + 1];
				glm::vec3* a = &s->m_vertices[j + 2];

				push_vec3(*a, &verts);
				push_vec3(s->m_plane.normal, &verts);

				push_vec3(*b, &verts);
				push_vec3(s->m_plane.normal, &verts);

				push_vec3(*c, &verts);
				push_vec3(s->m_plane.normal, &verts);
			}
		}

		this->m_mesh = new Mesh(verts, MeshMode::POS_XYZ_NORMAL_XYZ);
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
		if ((dataSrc->GetFirstByName("solid") == NULL) && (dataSrc->Values.count("origin") == 0))
			throw std::exception(("origin could not be resolved for entity ID: " + dataSrc->Values["id"]).c_str());

		this->m_classname = dataSrc->Values["classname"];
		this->m_id = (int)::atof(dataSrc->Values["id"].c_str());
		this->m_keyvalues = dataSrc->Values;
		this->m_editorvalues = editorvalues(dataSrc->GetFirstByName("editor"));
		
		if (dataSrc->GetFirstByName("solid") == NULL) {
			vmf_parse::Vector3f(dataSrc->Values["origin"], &this->m_origin);
			this->m_origin = glm::vec3(-this->m_origin.x, this->m_origin.z, this->m_origin.y);
		}
		else {
			// Process brush entities
			for (auto&& s: dataSrc->GetAllByName("solid")) {
				this->m_internal_solids.push_back(solid(s));
			}

			// Calculate origin
			glm::vec3 NWU = this->m_internal_solids[0].NWU;
			glm::vec3 SEL = this->m_internal_solids[0].SEL;
			for (auto&& i: this->m_internal_solids)
				boundary_extend(&NWU, &SEL, i.NWU, i.SEL);

			this->m_origin = (NWU + SEL) * 0.5f;
		}
	}
};

struct BoundingBox {
	glm::vec3 NWU;
	glm::vec3 SEL;

	BoundingBox() {}
	BoundingBox(glm::vec3 _nwu, glm::vec3 _sel) : NWU(_nwu), SEL(_sel) {}

	// Create an inverted bounding box to push bounds to
	static BoundingBox inverted_maxs(){
		return BoundingBox(glm::vec3(-99999.9f, -99999.9f, -99999.9f), glm::vec3(99999.9f, 99999.9f, 99999.9f));
	}
};

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
	
	static std::map<std::string, Mesh*> s_model_dict;

	static vmf* from_file(const std::string& path) {
		LOG_SCOPE_FUNCTION(1);
		vmf* v = new vmf();

		std::ifstream ifs(path);
		if (!ifs) throw std::exception("VMF File read error.");

		std::string file_str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

		LOG_F(1, "loading filedata");
		kv::FileData file_kv(file_str);

		LOG_F(1, "Processing visgroups");
		// Process visgroup list
		for (auto && vg : file_kv.headNode->GetFirstByName("visgroups")->GetAllByName("visgroup")) {
			v->m_visgroups.insert({ vg->Values["name"], std::stoi(vg->Values["visgroupid"]) });
		}

		LOG_F(1, "Processing solids");
		// Solids
		for (auto && kv_solid : file_kv.headNode->GetFirstByName("world")->GetAllByName("solid")) {
			v->m_solids.push_back(solid(kv_solid));
		}

		LOG_F(1, "Processing entities");
		// Entities
		for (auto && kv_entity : file_kv.headNode->GetAllByName("entity")) {
			try {
				entity ent = entity(kv_entity);
				v->m_entities.push_back(ent);
			} catch (std::exception e) {
				LOG_F(WARNING, "Entity exception: %s", e.what());
			}
		}

		LOG_F(1, "VMF loaded");
		return v;
	}

	void DrawWorld(Shader* shader) {
		glm::mat4 model = glm::mat4();
		//shader->setMatrix("model", model);

		// Draw solids
		for (auto && solid : this->m_solids) {
			//if (solid.NWU.y < this->m_render_h_max || solid.NWU.y > this->m_render_h_min) continue;

			glm::vec2 orgin = glm::vec2(solid.NWU.x + solid.SEL.x, solid.NWU.z + solid.SEL.z) / 2.0f;
			//shader->setVec2("origin", glm::vec2(orgin.x, orgin.y));
			solid.Draw(shader);
		}

		model = glm::mat4();
		//shader->setMatrix("model", model);
		// Draw 
	}

	void DrawEntities(Shader* shader, glm::mat4 transformMatrix, glm::mat4 matrixFinalApplyTransform) {
		glm::mat4 model = glm::mat4();
		shader->setMatrix("model", model);

		// Draw props
		for (auto && ent : this->m_entities) {
			// Check if we can actually draw this object currently :)
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
				shader->setVec2("origin", glm::vec2(ent.m_origin.x, ent.m_origin.z));

				if(vmf::s_model_dict.count(kv::tryGetStringValue(ent.m_keyvalues, "model", "error.mdl")))
					vmf::s_model_dict[kv::tryGetStringValue(ent.m_keyvalues, "model", "error.mdl")]->Draw();
			}
			else {
				model = glm::mat4();
				shader->setMatrix("model", model);

				for (auto && s : ent.m_internal_solids) {
					if (s.NWU.y > this->m_render_h_min || s.NWU.y < this->m_render_h_max) continue;
					shader->setVec2("origin", glm::vec2(ent.m_origin.x, ent.m_origin.z));
					s.Draw(shader);
				}
			}
		}

		// Resets 
		model = glm::mat4();
		shader->setMatrix("model", model);
	}

	// Calculate boundaries of a visgroup
	BoundingBox getVisgroupBounds(const std::string& visgroup) {
		BoundingBox bounds = BoundingBox::inverted_maxs();
		if (!this->m_visgroups.count(visgroup)) return bounds;

		unsigned int vgroup = this->m_visgroups[visgroup];
		for (auto && iSolid : this->m_solids) boundary_extend(&bounds.NWU, &bounds.SEL, iSolid.NWU, iSolid.SEL);

		return bounds;
	}

	// Calculate spawn points average location, based on the highest(0 high) priority group
	glm::vec3* calculateSpawnAVG_PMIN(const std::string& classname) {
		std::vector<entity*> spawns = this->get_entities_by_classname(classname);

		if (spawns.size() <= 0) return NULL;

		//Find lowest priority (highest)
		int lowest = kv::tryGetValue<int>(spawns[0]->m_keyvalues, "priority", 0);
		for (auto&& s: spawns) {
			int l = kv::tryGetValue<int>(s->m_keyvalues, "priority", 0);
			lowest = l < lowest? l: lowest;
		}

		//Collect all spawns with that priority
		glm::vec3* location = new glm::vec3();
		int c = 0;
		for (auto&& s: spawns) {
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
std::map<std::string, Mesh*> vmf::s_model_dict;
std::map<std::string, material*> material::m_index;