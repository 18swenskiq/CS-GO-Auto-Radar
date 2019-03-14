#pragma once
#include <iostream>
#include <fstream>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <sstream>
#include <vector>
#include <map>
#include <regex>

#include "Util.h"
#include "vdf.hpp"
#include "plane.h"
#include "Mesh.hpp"
#include "convexPolytope.h"
#include "fuzzy_select.h"
#include "interpolation.h"

#include <algorithm>

#include <chrono>

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


namespace vmf {

	int current_line = 0;
	int line_count = 0;
	void progress_callback() {
		current_line++;

		if (current_line == line_count - 1)
			std::cout << "Line " << current_line << "/" << line_count << "\n";
		if(((current_line % 10000) == 0))
			std::cout << "Line " << current_line << "/" << line_count << "\r";
	}

	enum team {
		terrorist,
		counter_terrorist
	};

	struct BoundingBox {
		glm::vec3 NWU;
		glm::vec3 SEL;
	};

	struct DispInfo {
		int power;
		glm::vec3 startposition;

		std::vector<std::vector<glm::vec3>> normals;
		std::vector<std::vector<float>> distances;

		// OpenGL generated mesh
		Mesh* glMesh;
	};

	struct Side {
		int ID;
		std::string texture;
		Plane plane;

		DispInfo* displacement = NULL;
	};

	struct Solid {
		int fileorder_id;
		int ID;
		bool containsDisplacements = false;
		std::vector<Side> faces;
		glm::vec3 color;
		glm::vec3 origin;
		bool hidden = false;

		std::vector<unsigned short> visgroupids;

		BoundingBox bounds;

		Mesh* mesh;
	};

	struct BuyZone {
		int teamNum;
		Mesh* mesh;
	};

	struct Entity {
		int ID;
		std::string classname;
		glm::vec3 origin;

		std::map<std::string, std::string> keyValues;
		std::vector<Solid> internal_solids;

		bool hidden = false;
	};

	class vmf {
	public:
		kv::FileData internal;
		std::vector<Mesh> meshes;
		std::vector<Solid> solids;
		std::vector<Entity> entities;

		std::map<unsigned short, std::string> visgroups;

		vmf(std::string path)
		{
			std::ifstream ifs(path);
			if (!ifs) {
				std::cout << "Could not open file... " << path << std::endl;
				throw std::exception("File read error");
			}

			std::cout << "Initializing VMF read\n";
			std::ifstream _ifs(path);

			// new lines will be skipped unless we stop it from happening:    
			_ifs.unsetf(std::ios_base::skipws);

			// count the newlines with an algorithm specialized for counting:
			line_count = std::count(
				std::istream_iterator<char>(_ifs),
				std::istream_iterator<char>(),
				'\n');

			_ifs.close();

			std::cout << "Reading raw VMF\n";
			std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

			std::cout << "Processing VMF data\n";
			kv::FileData data(str, &progress_callback);

			this->internal = data;

#pragma region Solids
			std::cout << "Processing solids\n";

			//Process solids list
			std::vector<kv::DataBlock> SolidList = data.headNode.GetFirstByName("world")->GetAllByName("solid");
			int total = SolidList.size();

			for (int i = 0; i < SolidList.size(); i++){
				std::cout << "Solid " << i + 1 << "/" << total << "\r";

				kv::DataBlock cBlock = SolidList[i];

				Solid solid;
				bool valid = true;

				std::vector<kv::DataBlock> Sides = cBlock.GetAllByName("side");
				for (int j = 0; j < Sides.size(); j++)
				{
					kv::DataBlock cSide = Sides[j];

					Side side;
					side.ID = ::atof(cSide.Values["id"].c_str());
					side.texture = cSide.Values["material"];

					Plane plane;
					if (!vmf_parse::plane(cSide.Values["plane"], &plane))
					{
						valid = false; break;
					}

					side.plane = plane;

					// Deal with displacement info. Oh no

#pragma region displacements

					DispInfo* dispInfo = new DispInfo;

					kv::DataBlock* dblockInfo = cSide.GetFirstByName("dispinfo");

					if (dblockInfo != NULL){
						solid.containsDisplacements = true; // Mark we have displacements here

						kv::DataBlock* dblockNormals = dblockInfo->GetFirstByName("normals");
						kv::DataBlock* dblockDistances = dblockInfo->GetFirstByName("distances");
						dispInfo->power = std::stoi(dblockInfo->Values["power"]);
						vmf_parse::Vector3fS(dblockInfo->Values["startposition"], &dispInfo->startposition);

						int i_target = glm::pow(2, dispInfo->power) + 1;

						for (int x = 0; x < i_target; x++) { //Row
							dispInfo->normals.push_back(std::vector<glm::vec3>()); //Create row container
							dispInfo->distances.push_back(std::vector<float>()); //Create distances container

							//Parse in the normals
							std::vector<std::string> values = split(dblockNormals->Values["row" + std::to_string(x)]);
							std::vector<float> list;
							for (auto && v : values) list.push_back(::atof(v.c_str()));

							//Parse in the distances
							std::vector<std::string> _values = split(dblockDistances->Values["row" + std::to_string(x)]);
							for (auto && v : _values) dispInfo->distances[x].push_back(std::stof(v.c_str()));

							for (int xx = 0; xx < i_target; xx++) { //Column
								dispInfo->normals[x].push_back(
									glm::vec3(list[xx * 3 + 0], 
										list[xx * 3 + 1], 
										list[xx * 3 + 2]));
							}
						}

						side.displacement = dispInfo;
					}
#pragma endregion

					solid.faces.push_back(side);
				}

				kv::DataBlock* editorValues = cBlock.GetFirstByName("editor");

				//Gather up the visgroups
				int viscount = -1;
				while (editorValues->Values.count("visgroupid" + (++viscount > 0 ? std::to_string(viscount) : ""))) 
					solid.visgroupids.push_back(std::stoi(editorValues->Values["visgroupid" + (viscount > 0 ? std::to_string(viscount) : "")]));

				glm::vec3 color;
				if (vmf_parse::Vector3f(editorValues->Values["color"], &color))
					solid.color = glm::vec3(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
				else
					solid.color = glm::vec3(1, 0, 0);

				this->solids.push_back(solid);
			}

			std::cout << "\n";

#pragma endregion Solids
#pragma region Entities
			std::cout << "Processing entites\n";

			//Process entities list
			std::vector<kv::DataBlock> EntitiesList = data.headNode.GetAllByName("entity");
			for (auto && block : EntitiesList) {

				//if (block.Values["classname"] != "prop_static") continue; //Skip anything else than prop static for now

				//Check wether origin can be resolved for entity
				if ((block.GetFirstByName("solid") == NULL) && (block.Values.count("origin") == 0)) {
					std::cout << "Origin could not be resolved for entity with ID " << block.Values["id"]; continue;
				}

				Entity ent;
				ent.classname = block.Values["classname"];
				ent.ID = (int)::atof(block.Values["id"].c_str());
				ent.keyValues = block.Values;

				glm::vec3 loc = glm::vec3();
				if (block.Values.count("origin")) {							//Start with hammer origin
					vmf_parse::Vector3f(block.Values["origin"], &loc);
					ent.origin = glm::vec3(loc.x, loc.y, loc.z);
				}
				else if (block.GetFirstByName("solid") != NULL) {			//Try to process it from solid
					//Get all solids
					std::vector<kv::DataBlock> _solids = block.GetAllByName("solid");
					//std::vector<Solid> _solids_ent;
					for (int i = 0; i < _solids.size(); i++)
					{
						kv::DataBlock cBlock = _solids[i];

						Solid solid;
						bool valid = true;

						std::vector<kv::DataBlock> Sides = cBlock.GetAllByName("side");
						for (int j = 0; j < Sides.size(); j++)
						{
							kv::DataBlock cSide = Sides[j];

							Side side;
							side.ID = ::atof(cSide.Values["id"].c_str());
							side.texture = cSide.Values["material"];

							Plane plane;
							if (!vmf_parse::plane(cSide.Values["plane"], &plane))
							{
								valid = false; break;
							}

							side.plane = plane;

							solid.faces.push_back(side);
						}

						kv::DataBlock* editorValues = block.GetFirstByName("editor");

						//Gather up the visgroups
						int viscount = -1;
						while (editorValues->Values.count("visgroupid" + (++viscount > 0 ? std::to_string(viscount) : "")))
							solid.visgroupids.push_back(std::stoi(editorValues->Values["visgroupid" + (viscount > 0 ? std::to_string(viscount) : "")]));

						glm::vec3 color;
						if (vmf_parse::Vector3f(editorValues->Values["color"], &color))
							solid.color = glm::vec3(color.x / 255.0f, color.y / 255.0f, color.z / 255.0f);
						else
							solid.color = glm::vec3(1, 0, 0);

						ent.internal_solids.push_back(solid);
					}

					//Process convex polytopes & calculate origin

					std::vector<Polytope> polytopes;

					for (auto && iSolid : ent.internal_solids) {
						std::vector<Plane> planes;
						for (auto f : iSolid.faces) planes.push_back(f.plane);

						polytopes.push_back(Polytope(planes, false));
					}

					glm::vec3 NWU = polytopes[0].NWU;
					glm::vec3 SEL = polytopes[0].SEL;

					for (auto && iPoly : polytopes) {
						if (iPoly.NWU.z > NWU.z) NWU.z = iPoly.NWU.z;
						if (iPoly.NWU.y > NWU.y) NWU.y = iPoly.NWU.y;
						if (iPoly.NWU.x > NWU.x) NWU.x = iPoly.NWU.x;

						if (iPoly.SEL.z < SEL.z) SEL.z = iPoly.SEL.z;
						if (iPoly.SEL.y < SEL.y) SEL.y = iPoly.SEL.y;
						if (iPoly.SEL.x < SEL.x) SEL.x = iPoly.SEL.x;
					}

					ent.origin = (NWU + SEL) * 0.5f;
				}

				this->entities.push_back(ent);
			}
#pragma endregion

			std::cout << "Processing visgroups\n";

			//Process Visgroups
			std::vector<kv::DataBlock> VisList = data.headNode.GetFirstByName("visgroups")->GetAllByName("visgroup");
			for (auto v : VisList) {
				this->visgroups.insert({ std::stoi(v.Values["visgroupid"]), v.Values["name"] });

				std::cout << "Visgroup {" << std::stoi(v.Values["visgroupid"]) << "} = '" << v.Values["name"] << "'\n";
			}
		}

		std::vector<Solid*> getSolidsInVisGroup(std::string visgroup) {
			std::vector<Solid*> list;
			for (auto && v : this->solids) {
				for (auto && vid : v.visgroupids) {
					if (this->visgroups[vid] == visgroup) {
						list.push_back(&v);
					}
				}
			}

			return list;
		}

		std::vector<Solid*> getAllBrushesInVisGroup(std::string visgroup) {
			std::vector<Solid*> list;

			// All solids
			for (auto && v : this->solids) {
				for (auto && vid : v.visgroupids) {
					if (this->visgroups[vid] == visgroup) {
						list.push_back(&v);
					}
				}
			}

			// All entity brush solids
			for (auto && e : this->entities) {
				for (auto && es : e.internal_solids) {
					for (auto && esvid : es.visgroupids) {
						if (this->visgroups[esvid] == visgroup) {
							list.push_back(&es);
						}
					}
				}
			}

			return list;
		}

		std::vector<Solid*> getAllRenderBrushes() {
			std::vector<Solid*> list;
			for (auto && s : this->solids)
				list.push_back(&s);

			for (auto && ent : this->entities) {
				if (ent.classname == "func_detail" || ent.classname == "func_brush") {
					for (auto && s : ent.internal_solids) {
						list.push_back(&s);
					}
				}
			}
			
			return list;
		}

		std::vector<Solid*> getAllBrushesByClassNameAppend(std::string classname) {

		}

		std::vector<Solid*> getAllBrushesByClassName(std::string classname) {
			std::vector<Solid*> list;
			for (auto && ent : this->entities) {
				if (ent.classname == classname) {
					for (auto && s : ent.internal_solids) {
						list.push_back(&s);
					}
				}
			}
			return list;
		}

		/* Gets a list of entities with matching classname */
		std::vector<Entity*> findEntitiesByClassName(std::string classname) {
			std::vector<Entity*> list;
			for (auto && ent : this->entities) {
				if (ent.classname == classname) {
					list.push_back(&ent);
				}
			}
			return list;
		}

		glm::vec3* calculateSpawnLocation(team _team) {

			std::vector<Entity*> spawns = this->findEntitiesByClassName(_team == team::terrorist ? "info_player_terrorist" : "info_player_counterterrorist");

			if (spawns.size() <= 0) return NULL;

			//Find lowest priority (highest)
			int lowest = kv::tryGetValue<int>(spawns[0]->keyValues, "priority", 0);
			for (auto && s : spawns) {
				int l = kv::tryGetValue<int>(s->keyValues, "priority", 0);
				lowest = l < lowest ? l : lowest;
			}

			//Collect all spawns with that priority
			glm::vec3* location = new glm::vec3();
			int c = 0;
			for (auto && s : spawns) {
				if (kv::tryGetValue<int>(s->keyValues, "priority", 0) == lowest) {
					*location += s->origin; c++;
				}
			}

			//avg
			*location = *location / (float)c;
			return location;
		}

		void ComputeGLMeshes() {
			auto start = std::chrono::high_resolution_clock::now();

			std::cout << "Processing solid meshes... ";
			for (int i = 0; i < this->solids.size(); i++) {
				std::vector<Plane> sidePlanes;
				for (int j = 0; j < this->solids[i].faces.size(); j++)
					sidePlanes.push_back(this->solids[i].faces[j].plane);

				Polytope p = Polytope(sidePlanes);
				this->solids[i].mesh = p.GeneratedMesh;
				this->solids[i].origin = (p.NWU + p.SEL) * 0.5f;
				this->solids[i].bounds.NWU = p.NWU;
				this->solids[i].bounds.SEL = p.SEL;
			}
			std::cout << "done\n";

			std::cout << "Processing entity solid meshes... ";
			for (auto && ent : this->entities) {
				for (auto && _solid : ent.internal_solids) {
					std::vector<Plane> sidePlanes;
					for (auto f : _solid.faces)
						sidePlanes.push_back(f.plane);

					Polytope p = Polytope(sidePlanes);
					_solid.mesh = p.GeneratedMesh;
					_solid.origin = (p.NWU + p.SEL) * 0.5f;
					_solid.bounds.NWU = p.NWU;
					_solid.bounds.SEL = p.SEL;
				}
			}
			std::cout << "done\n";

			auto elapsed = std::chrono::high_resolution_clock::now() - start;
			long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

			std::cout << "GL mesh computation: " << milliseconds << "ms" << std::endl;
		}

		void ComputeDisplacements() {
			auto start = std::chrono::high_resolution_clock::now();

			std::cout << "Computing displacements...\n";

			for (auto && v :this->solids) {
				if (v.containsDisplacements) {

					std::vector<Plane> planes;
					for (auto && face : v.faces) planes.push_back(face.plane);

					Polytope polyTope = Polytope(planes, true, false); // Generate polytope so we can look at the individual ngon points

					for (auto && side :v.faces) {
						if (side.displacement != NULL) {
							DispInfo* info = side.displacement;

							BrushPolygon* bpoly = NULL;

							std::map<float, BrushPolygon*> normalCorrelations;

							//Sort planes by their similarity to the face's normal direction
							for (auto && fuck : polyTope.ngons)
								normalCorrelations.insert({ glm::distance(fuck.plane.normal, side.plane.normal), &fuck });

							bpoly = normalCorrelations.begin()->second;

							if (bpoly->vertices.size() != 4) {
								std::cout << "Displacement info matched to face with {" << bpoly->vertices.size() << "} vertices!!!\n"; continue;
							}

							// Match the 'starting point' of dispinfo
							std::map<float, glm::vec3*> distancesToStart;
							for (auto && point : bpoly->vertices)
								distancesToStart.insert({ glm::distance(info->startposition, point), &point });

							// The corners of the displacement
							glm::vec3* SW = distancesToStart.begin()->second;

							// Find what point in the vector it was
							int pos = 0;
							for (auto && point : bpoly->vertices)
								if (&point == SW) break; else pos++;

							// Get the rest of the points, in clockwise order (they should already be sorted by polytope generation)
							glm::vec3* NW = &bpoly->vertices[(pos + 1) % 4];
							glm::vec3* NE = &bpoly->vertices[(pos + 2) % 4];
							glm::vec3* SE = &bpoly->vertices[(pos + 3) % 4];

							int points = glm::pow(2, info->power) + 1; // calculate the point count (5, 9, 17)

							// Initialize list for floats
							std::vector<float> meshPoints;

							std::vector<glm::vec3> finalPoints;

							glm::vec3* NWU = &v.bounds.NWU;
							glm::vec3* SEL = &v.bounds.SEL;

							for (int row = 0; row < points; row++) {
								for (int col = 0; col < points; col++) {
									//Generate original base points

									float dx = (float)col / (float)(points-1); //Time values for linear interpolation
									float dy = (float)row / (float)(points-1);

									glm::vec3 LWR = lerp(*SW, *SE, dx);
									glm::vec3 UPR = lerp(*NW, *NE, dx);
									glm::vec3 P = lerp(LWR, UPR, dy); // Original point location

									glm::vec3 offset = info->normals[col][row] * info->distances[col][row]; // Calculate offset
									P = P + offset; //Add offset to P

									finalPoints.push_back(P);

									//Recompute bounds while we are at it
									NWU->x = glm::max(-P.x, NWU->x);
									NWU->y = glm::max(P.z, NWU->y);
									NWU->z = glm::max(P.y, NWU->z);

									SEL->x = glm::min(-P.x, SEL->x);
									SEL->y = glm::min(P.z, SEL->y);
									SEL->z = glm::min(P.y, SEL->z);								

									continue;

									/* TESTING TRIANGLES */
									meshPoints.push_back(-P.x);
									meshPoints.push_back(P.z);
									meshPoints.push_back(P.y);
									meshPoints.push_back(0);
									meshPoints.push_back(0);
									meshPoints.push_back(1);

									meshPoints.push_back(-P.x);
									meshPoints.push_back(P.z);
									meshPoints.push_back(P.y + 8.0f);
									meshPoints.push_back(0);
									meshPoints.push_back(0);
									meshPoints.push_back(1);


									meshPoints.push_back(-P.x + 8.0f);
									meshPoints.push_back(P.z);
									meshPoints.push_back(P.y + 8.0f);
									meshPoints.push_back(0);
									meshPoints.push_back(0);
									meshPoints.push_back(1);
								}
							}

							int i_condition = 0;
							for (int row = 0; row < points - 1; row++) {
								for (int col = 0; col < points - 1; col++) {

									// Gather point pointers
									// hehe :(
									glm::vec3* SW = &finalPoints[((row + 0) * points) + (col + 0)];
									glm::vec3* SE = &finalPoints[((row + 0) * points) + (col + 1)];

									glm::vec3* NW = &finalPoints[((row + 1) * points) + (col + 0)];
									glm::vec3* NE = &finalPoints[((row + 1) * points) + (col + 1)];
									
									if (i_condition++ % 2 == 0) {//Condition 0
										meshPoints.push_back(-SW->x);
										meshPoints.push_back(SW->z);
										meshPoints.push_back(SW->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);
										meshPoints.push_back(-NW->x);
										meshPoints.push_back(NW->z);
										meshPoints.push_back(NW->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);
										meshPoints.push_back(-NE->x);
										meshPoints.push_back(NE->z);
										meshPoints.push_back(NE->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);

										meshPoints.push_back(-SW->x);
										meshPoints.push_back(SW->z);
										meshPoints.push_back(SW->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);
										meshPoints.push_back(-NE->x);
										meshPoints.push_back(NE->z);
										meshPoints.push_back(NE->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);
										meshPoints.push_back(-SE->x);
										meshPoints.push_back(SE->z);
										meshPoints.push_back(SE->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);
									}
									else { //Condition 1
										meshPoints.push_back(-SW->x);
										meshPoints.push_back(SW->z);
										meshPoints.push_back(SW->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);
										meshPoints.push_back(-NW->x);
										meshPoints.push_back(NW->z);
										meshPoints.push_back(NW->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);
										meshPoints.push_back(-SE->x);
										meshPoints.push_back(SE->z);
										meshPoints.push_back(SE->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);

										meshPoints.push_back(-NW->x);
										meshPoints.push_back(NW->z);
										meshPoints.push_back(NW->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);
										meshPoints.push_back(-NE->x);
										meshPoints.push_back(NE->z);
										meshPoints.push_back(NE->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);
										meshPoints.push_back(-SE->x);
										meshPoints.push_back(SE->z);
										meshPoints.push_back(SE->y);
										meshPoints.push_back(0);
										meshPoints.push_back(0);
										meshPoints.push_back(1);
									}
								}
								i_condition++;
							}

							Mesh* _glMesh = new Mesh(meshPoints);
							info->glMesh = _glMesh;
						}
					}
				}
			}

			auto elapsed = std::chrono::high_resolution_clock::now() - start;
			long long milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

			std::cout << "Displacement computation: " << milliseconds << "ms" << std::endl;
		}

		void clean() {
			for (int i = 0; i < this->solids.size(); i++) {
				delete this->solids[i].mesh;
				this->solids[i].mesh = NULL;
			}

			for (auto i : this->entities) {
				for (auto m : i.internal_solids) {
					delete m.mesh;
					m.mesh = NULL;
				}
			}
		}

		~vmf() {
			
		}
	};

	BoundingBox getSolidListBounds(std::vector<Solid*> list) {
		if (list.size() <= 0) return BoundingBox();

		BoundingBox bounds;
		bounds.NWU = list[0]->bounds.NWU;
		bounds.SEL = list[0]->bounds.SEL;

		for (auto && iSolid : list) {
			if (iSolid->bounds.NWU.z > bounds.NWU.z) bounds.NWU.z = iSolid->bounds.NWU.z;
			if (iSolid->bounds.NWU.y > bounds.NWU.y) bounds.NWU.y = iSolid->bounds.NWU.y;
			if (iSolid->bounds.NWU.x > bounds.NWU.x) bounds.NWU.x = iSolid->bounds.NWU.x;

			if (iSolid->bounds.SEL.z < bounds.SEL.z) bounds.SEL.z = iSolid->bounds.SEL.z;
			if (iSolid->bounds.SEL.y < bounds.SEL.y) bounds.SEL.y = iSolid->bounds.SEL.y;
			if (iSolid->bounds.SEL.x < bounds.SEL.x) bounds.SEL.x = iSolid->bounds.SEL.x;
		}

		return bounds;
	}
}