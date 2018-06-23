#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#define _HAS_ITERATOR_DEBUGGING = 0
#define _SCL_SECURE = 0

namespace octree{
	class Node {
	public:
		Node* subnodes = NULL;
		std::vector<glm::vec3*> points = std::vector<glm::vec3*>();
		int resolution = 0;

		glm::vec3 mins = glm::vec3(0,0,0);
		glm::vec3 maxes = glm::vec3(0, 0, 0);
		glm::vec3 midpoint = glm::vec3(0, 0, 0);

		Node() {}

		Node(int resolution, glm::vec3 mins, glm::vec3 maxes) {
			//std::cout << "Octree level {" << resolution << "}" << std::endl;

			this->resolution = resolution;

			if (resolution > 0) {
				this->subnodes = new Node[8]; //Generate subnodes

				//calculate sub distances
				glm::vec3 subdist = (maxes - mins) * 0.5f;
				subdist = glm::abs(subdist); //Make sure its positive

				//Calculate midpoint
				glm::vec3 _midpoint = (mins + maxes) * 0.5f;

				this->midpoint = _midpoint;

				this->subnodes[0] = Node(resolution - 1, _midpoint + glm::vec3(-subdist.x, -subdist.y, -subdist.z), _midpoint);
				this->subnodes[1] = Node(resolution - 1, _midpoint + glm::vec3(subdist.x, -subdist.y, -subdist.z),  _midpoint);
				this->subnodes[2] = Node(resolution - 1, _midpoint + glm::vec3(-subdist.x, subdist.y, -subdist.z),  _midpoint);
				this->subnodes[3] = Node(resolution - 1, _midpoint + glm::vec3(subdist.x, subdist.y, -subdist.z),   _midpoint);

				this->subnodes[4] = Node(resolution - 1, _midpoint + glm::vec3(-subdist.x, -subdist.y, subdist.z), _midpoint);
				this->subnodes[5] = Node(resolution - 1, _midpoint + glm::vec3(subdist.x, -subdist.y,  subdist.z), _midpoint);
				this->subnodes[6] = Node(resolution - 1, _midpoint + glm::vec3(-subdist.x, subdist.y,  subdist.z), _midpoint);
				this->subnodes[7] = Node(resolution - 1, _midpoint + glm::vec3(subdist.x, subdist.y,   subdist.z), _midpoint);
			}
		}

		void insert(glm::vec3* v) {
			if (this->resolution > 0) {
				//Decide what oct to push into
				int index = 0;

				//Index into upper quadrents if above midpoint
				if (v->z > this->midpoint.z)
					index += 4;

				//Index into upper(y) halfs
				if (v->y > this->midpoint.y)
					index += 2;

				//Index into remaining two
				if (v->x > this->midpoint.x)
					index += 1;

				//Dive into the next node
				this->subnodes[index].insert(v);
				return; //We are done in this node
			}
			else { //We are as low as resolution goes
				this->points.push_back(v);
			}
		}

		Node* getNodeByVec(glm::vec3 v) {
			if (this->resolution > 0) {
				//Decide what oct to push into
				int index = 0;

				//Index into upper quadrents if above midpoint
				if (v.z > this->midpoint.z)
					index += 4;

				//Index into upper(y) halfs
				if (v.y > this->midpoint.y)
					index += 2;

				//Index into remaining two
				if (v.x > this->midpoint.x)
					index += 1;

				//If the next node we try has no values
				if(this->subnodes[index].getEntryCount() == 0)
					return this;

				//Dive into the next node
				return this->subnodes[index].getNodeByVec(v);
			}

			//Lowest point
			return this;
		}

		int getEntryCount(int c = 0) {
			if (this->resolution > 0) {
				int temp = 0;
				for (int i = 0; i < 8; i++) {
					temp += this->subnodes[i].getEntryCount();
				}
				c += temp;
				return c;
			}
			
			return this->points.size();
		}

		std::vector<glm::vec3*> getContainedValues(std::vector<glm::vec3*> vals = std::vector<glm::vec3*>()) {
			if (this->resolution > 0) {
				for (int i = 0; i < 8; i++) {
					std::vector<glm::vec3*> temp = this->subnodes[i].getContainedValues(vals);
					for (int x = 0; x < temp.size(); x++) {
						vals.push_back(temp[x]);
					}
				}
				return vals;
			}

			return this->points;
		}
	};

	class Tree {
	public:
		Node head;
		glm::vec3 mins = glm::vec3(0, 0, 0);
		glm::vec3 maxes = glm::vec3(0, 0, 0);

		Tree(std::vector<glm::vec3> data, int resolution) {
			//Find master mins and maxes
			for (int i = 0; i < data.size(); i++) {
				glm::vec3 v0 = data[i];
				if (v0.x < mins.x)
					mins.x = v0.x;
				if (v0.y < mins.y)
					mins.y = v0.y;
				if (v0.z < mins.z)
					mins.z = v0.z;

				if (v0.x > maxes.x)
					maxes.x = v0.x;
				if (v0.y > maxes.y)
					maxes.y = v0.y;
				if (v0.z > maxes.z)
					maxes.z = v0.z;
			}

			//Generate node structure
			this->head = Node(resolution, mins, maxes);
			std::cout << "Octree prepared" << std::endl;

			for (int i = 0; i < data.size(); i++) {
				head.insert(&data[i]);
			}

			std::cout << "Data inserted into tree" << std::endl;
		}
	};
}