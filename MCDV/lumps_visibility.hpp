#pragma once
#include <vector>
#include <fstream>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "generic.hpp"

/* BSP Visibility file structure */
/*
                 ===== The BSP Tree =====
+---------------------------+---------------------------+																		
|			NODE			|			LEAF			|																		
+---------------------------+---------------------------+																		
+-> Total volume of all		+-> Represents a defined	|																		
|   subnodes and leafs		|   Volume of the map		|																		
|	below it				|							|
|							+-> Convex Polyhedra		|
+-> Has exactly two			|	defined by the planes   |
|   Children: can be		|	of their parent nodes	|
|   either another child	|							|
|   node or leaf			+-> Never overlaps			|
|                           |                           |
+-> References a plane      |                           |
+---------------------------+---------------------------+

		NODE
       /    \
     NODE   LEAF
    /   \
  LEAF  LEAF

Traversing the tree

		NODE		:: We start at headnode (Node 0), and we have our viewpoint as a vec3
       /    \          
	  |		 |			IF viewpoint infront node plane
	  |		 |				goto: Child-1
	  |		 |			ELSE
	  |		 |				goto: Child-2
     NODE   LEAF	
    /   \			:: Repeat this step until we reach a leaf
  LEAF  LEAF

There is multiple, unconnected BSP trees in the map defined in the model array**
The first tree is worldspawn; all the geometry of the map

**Worldspawn doesn't seem to be split
*/

namespace vis
{
#pragma pack(push, 1)
// L5
struct node
{
	int planeNum; //Index into plane array
	int children[2]; //If the reference is +, then its referencing the node array
						// Otherwise if it's -, then we look for a leaf. (-1-child).  Value -100 >> 99

	/* Rough bounding box */
	short mins[3];
	short maxs[3]; 

	/* Map faces contained in this node */
	unsigned short firstFace; //Index into face array.
	unsigned short numFaces;  //Counting both sides

	short area; //If all leaves below this node are in the same area, then this is the area index, if not; -1
		
	short padding; //32 bytelength pad
};

// L10
struct leaf
{
	int contents; //OR of all brushes (not needed?)
	short cluster; //Cluster this leaf is in
	short area : 9;
	short flags : 7;

	/* Rough bounding box */
	short mins[3];
	short maxs[3]; 

	unsigned short firstleafface; //Index into leaffaces
	unsigned short numleaffaces;
	unsigned short firstleafbrush; //Index into leafbrushes
	unsigned short numleafbrushes;
	short leafWaterDataID; //-1 for not in water

	/* VERSION 19 OR LOWER */
	/*
	CompressedLightCube ambientLighting;
	short padding;
	*/
};

//L14
struct model
{
	glm::vec3 mins, maxs; //Bounding box
	glm::vec3 origin;

	int headnode; //References node array
	int firstface, numfaces; //Indexes into face array
	/* Create a virtual pool of faces that the BSP tree will read from */
};

#pragma pack(pop)

//=========================================================================
// Definitions

std::vector<vis::model> readModels(std::ifstream* reader, bsp::lumpHeader lumpinfo)
{
	return bsp::readLumpGeneric<vis::model>(reader, lumpinfo);
}

std::vector<vis::leaf> readLeaves(std::ifstream* reader, bsp::lumpHeader lumpinfo)
{
	return bsp::readLumpGeneric<vis::leaf>(reader, lumpinfo);
}

std::vector<vis::node> readNodes(std::ifstream* reader, bsp::lumpHeader lumpinfo)
{
	return bsp::readLumpGeneric<vis::node>(reader, lumpinfo);
}
}