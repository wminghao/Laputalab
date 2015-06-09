#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "unit.h"

bool loadOBJ(
	const char * path, 
	std::vector<myvec3> & out_vertices
);

#endif