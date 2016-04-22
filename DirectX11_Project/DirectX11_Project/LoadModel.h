#pragma once
#include <vector>
#include <iostream>
#include<fstream>
#include "SharedDefines.h"
using namespace std;
class LoadMesh
{
private:

public:
	LoadMesh();
	~LoadMesh();
	bool LoadObj(const char* path, std::vector<float3>&v_ForModel, std::vector<float2>&t_ForModel, std::vector<float3>&n_ForModel, std::vector<unsigned int>& v_indices, std::vector<unsigned int>& t_indices, std::vector<unsigned int>& n_indices );


};