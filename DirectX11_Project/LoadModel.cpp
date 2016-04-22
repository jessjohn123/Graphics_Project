#include "LoadModel.h"

LoadMesh::LoadMesh()
{

}

LoadMesh::~LoadMesh()
{

}

bool LoadMesh::LoadObj(const char * path, std::vector<float3>& v_ForModel, std::vector<float2>& t_ForModel, std::vector<float3>& n_ForModel, std::vector<unsigned int>& v_indices, std::vector<unsigned int>& t_indices, std::vector<unsigned int>& n_indices)
{
	ifstream fin;
	char input, input2;

	fin.open(path, ios_base::in);

	if (fin.fail())
	{
		return false;
	}

	fin.get(input);
	while (!fin.eof())
	{
		fin.get(input);
		if (input == 'v')
		{
			fin.get(input);
			if (input == ' ')
			{
				float3 temp;

				fin >> temp.x >> temp.y >> temp.z;
				temp.x = -temp.x;
				v_ForModel.push_back(temp);
			}

			else if (input == 't')
			{
				float2 temp;
				fin >> temp.x >> temp.y;
				temp.y = 1.0f - temp.y;
				t_ForModel.push_back(temp);
			}

			else if (input == 'n')
			{
				float3 temp;
				fin >> temp.x >> temp.y >> temp.z;
				temp.x = -temp.x;
				n_ForModel.push_back(temp);
			}
		}

		else if (input == 'f')
		{
			float3 v_temp, t_temp, n_temp;

			fin.get(input);
			if (input == ' ')
			{
				fin >> v_temp.x >> input2 >> t_temp.x >> input2 >> n_temp.x
					>> v_temp.y >> input2 >> t_temp.y >> input2 >> n_temp.y
					>> v_temp.z >> input2 >> t_temp.z >> input2 >> n_temp.z;

				v_indices.push_back(v_temp.x-1);
				v_indices.push_back(v_temp.y-1);
				v_indices.push_back(v_temp.z-1);

				t_indices.push_back(t_temp.x-1);
				t_indices.push_back(t_temp.y-1);
				t_indices.push_back(t_temp.z-1);

				n_indices.push_back(n_temp.x-1);
				n_indices.push_back(n_temp.y-1);
				n_indices.push_back(n_temp.z-1);	
			}
		}

		while (input != '\n')
		{
			fin.get(input);
		}
	}

	fin.close();

	return true;
}
