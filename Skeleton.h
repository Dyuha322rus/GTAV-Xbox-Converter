#pragma once

#include "Material.h"
#include <list>

class CVertex
{
public:
	struct
	{
		bool position;
		bool normal;
		bool color;
		unsigned char numTexCoords;
	} format;
	unsigned int vertId;
	bool isAssociated;
	bool localOffset;
	int boneId;
	int meshId;
	Vector3 position;
	Vector3 normal;
	Vector2 texcoords[8];
	union
	{
		RwRGBA color;
		DWORD intColor;
	};

	CVertex()
	{
		format.position = false;
		format.normal = false;
		format.color = false;
		position = Vector3(0.0f, 0.0f, 0.0f);
		normal = Vector3(0.0f, 0.0f, 0.0f);
		format.numTexCoords = 0;
		for(int i = 0; i < 8; i++)
			texcoords[i] = Vector2(0.0f, 0.0f);
		color.red = 255;
		color.green = 255;
		color.blue = 255;
		color.alpha = 255;
		boneId = 0;
		vertId = 0;
		isAssociated = false;
		localOffset = false;
		meshId = 0;
	}

	void SetPosition(Vector3 &pos)
	{
		position = pos;
		format.position = true;
	}

	void SetNormal(Vector3 &nrm)
	{
		normal = nrm;
		format.normal = true;
	}

	void SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		format.color = true;
		color.red = r;
		color.green = g;
		color.blue = b;
		color.alpha = a;
	}

	void SetColor(unsigned int clr)
	{
		format.color = true;
		color = *(RwRGBA *)&clr;
	}

	void SetTexCoord(unsigned char num, Vector2 &tex)
	{
		texcoords[num] = tex;
		if((num + 1) > format.numTexCoords)
			format.numTexCoords = num + 1;
	}

	void SetBone(int bone)
	{
		boneId = bone;
	}

	bool IsEqual(CVertex &v, std::vector<CMaterial> &materials)
	{
		if(position == v.position && normal == v.normal)
		{
			if(materials.size() > 0 && materials[meshId]._StreamData.textured && materials[v.meshId]._StreamData.textured)
			{
				if(texcoords[0] == v.texcoords[0])
					return true;
				else
					return false;
			}
			return true;
		}
		return false;
	}
};

class CSceletonTriangle
{
public:
	unsigned int meshId;
	CVertex vertices[3];
	struct
	{
		bool position;
		bool normal;
		bool color;
		unsigned char numTexCoords;
	} format;

	CSceletonTriangle()
	{
		format.color = false;
		format.position = false;
		format.normal = false;
		meshId = 0;
		format.numTexCoords = 0;
	}

	CSceletonTriangle(CVertex &vertA, CVertex &vertB, CVertex &vertC, unsigned int mesh)
	{
		vertices[0] = vertA;
		vertices[1] = vertC;
		vertices[2] = vertB;
		for(int i = 0; i < 3; i++)
		{
			if(vertices[i].format.color)
				format.color = true;
			if(vertices[i].format.position)
				format.position = true;
			if(vertices[i].format.normal)
				format.normal = true;
			if(vertices[i].format.numTexCoords > format.numTexCoords)
				format.numTexCoords = vertices[i].format.numTexCoords;
		}
		meshId = mesh;
	}

	void Set(CVertex &vertA, CVertex &vertB, CVertex &vertC, unsigned int mesh)
	{
		vertices[0] = vertA;
		vertices[1] = vertC;
		vertices[2] = vertB;
		for(int i = 0; i < 3; i++)
		{
			if(vertices[i].format.color)
				format.color = true;
			if(vertices[i].format.position)
				format.position = true;
			if(vertices[i].format.normal)
				format.normal = true;
			if(vertices[i].format.numTexCoords > format.numTexCoords)
				format.numTexCoords = vertices[i].format.numTexCoords;
			vertices[i].meshId = mesh;
		}
		meshId = mesh;
	}

	void SetVertex(unsigned int id, CVertex &vert)
	{
		vertices[id] = vert; 
		if(vert.format.color)
			format.color = true;
		if(vert.format.position)
			format.position = true;
		if(vert.format.normal)
			format.normal = true;
		if(vert.format.numTexCoords > format.numTexCoords)
			format.numTexCoords = vert.format.numTexCoords;
	}

	void SetMesh(unsigned int mesh)
	{
		meshId = mesh;
	}
};

class CSkeletonBone
{
public:
	int boneId;
	std::vector<CSceletonTriangle> triangles;
	struct
	{
		bool position;
		bool normal;
		bool color;
		unsigned int numTexCoords;
	} format;

	void AddTriangle(CVertex &vertA, CVertex &vertB, CVertex &vertC, unsigned int mesh)
	{
		CSceletonTriangle triangle;
		triangle.Set(vertA, vertB, vertC, mesh);
		format.position = triangle.format.position;
		format.normal = triangle.format.normal;
		format.color = triangle.format.color;
		format.numTexCoords = triangle.format.numTexCoords;
		triangles.push_back(triangle);
	}
};