#include "pch.h"
#include "ObjLoader.h"

float Clamp(float _val, float _max, float _min) {
	if (_min >= _val)
		return _min;
	else if (_max <= _val)
		return _max;
	else
		return _val;

}

DirectX::XMFLOAT3 Lerp(DirectX::XMFLOAT3 _c1, DirectX::XMFLOAT3 _c2, float ratio) {
	DirectX::XMFLOAT3 temp;

	temp.x = (_c2.x - _c1.x) * ratio + _c1.x;
	temp.y = (_c2.y - _c1.y) * ratio + _c1.y;
	temp.z = (_c2.z - _c1.z) * ratio + _c1.z;

	return temp;
}

float Vector_Dot(DirectX::XMFLOAT3 v, DirectX::XMFLOAT3 w)
{
	return  v.x * w.x + v.y * w.y + v.z * w.z;
}

DirectX::XMFLOAT3 Vector_Scalar_Multiply(DirectX::XMFLOAT3 v, float s)
{
	DirectX::XMFLOAT3 x = v;

	x.x = v.x * s;
	x.y = v.y * s;
	x.z = v.z * s;

	return x;
}

bool loadOBJ(const char * path, std::vector<DX11UWA::VertexPositionColor> &out_vertices, std::vector<unsigned int> &out_indices)
{
	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<DirectX::XMFLOAT3> temp_vertices;
	std::vector<DirectX::XMFLOAT2> temp_uvs;
	std::vector<DirectX::XMFLOAT3> temp_normals;

	std::vector<DX11UWA::VertexPositionColor> vertices;
	std::vector<unsigned int> indices;

	// Declaring and Initializing the light
	DX11UWA::VertexPositionColor directionalLight;
	directionalLight.color.x = 1.0f;
	directionalLight.color.y = 1.0f;
	directionalLight.color.z = 0.0f;

	directionalLight.pos.x = 0.0f;
	directionalLight.pos.y = 1.0f;
	directionalLight.pos.z = 0.5f;

	FILE * file = fopen(path, "r");

	if (file == NULL)
	{
		printf("Impossible to open the file !\n");
		return false;
	}

	while (true) {
		char lineHeader[256];

		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);

		if (res == EOF)
			break;		// End of file. Quit the loop.

						// else : parse lineHeader
		if (strcmp(lineHeader, "v") == 0)
		{
			DirectX::XMFLOAT3 vertex;

			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0)
		{
			DirectX::XMFLOAT2 uv;

			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0)
		{
			DirectX::XMFLOAT3 normal;

			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0],
				&vertexIndex[1], &uvIndex[1], &normalIndex[1],
				&vertexIndex[2], &uvIndex[2], &normalIndex[2]);

			if (matches != 9)
			{
				printf("File can't be read by our simple parser: (Try exporting with other options)\n");
				return false;
			}

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++)
	{
		DX11UWA::VertexPositionColor temp;

		unsigned int vertexIndex = vertexIndices[i];
		DirectX::XMFLOAT3 vertex = temp_vertices[vertexIndex - 1];
		temp.pos.x = vertex.x;
		temp.pos.y = vertex.y;
		temp.pos.z = vertex.z;

		unsigned int uvIndex = uvIndices[i];
		DirectX::XMFLOAT2 uv = temp_uvs[uvIndex - 1];
		temp.color.x = uv.x;
		temp.color.y = uv.y;
		temp.color.z = 1.0f;

		unsigned int normalIndex = normalIndices[i];
		DirectX::XMFLOAT3 normal = temp_normals[normalIndex - 1];

		// Apply the Lighting to the model
		//float lightRatio = Clamp(Vector_Dot(Vector_Scalar_Multiply(directionalLight.pos, -1.0f), normal), 1.0f, 0.0f);
		//temp.color = Lerp(temp.color, directionalLight.color, lightRatio);

		vertices.push_back(temp);
		indices.push_back(i);
	}

	out_vertices = vertices;
	out_indices = indices;

	return true;
}

