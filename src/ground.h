#ifndef GROUND_H
#define GROUND_H

#include<glm/glm.hpp>
#include <gl/glew.h>

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

struct Vertex
{
	glm::vec3 m_pos;
	glm::vec2 m_tex;
	glm::vec3 m_normal;

	Vertex(){}

	Vertex(const glm::vec3 pos, const glm::vec2& tex, const glm::vec3& normal)
	{
		m_pos = pos;
		m_tex = tex;
		m_normal = normal;
	}

	Vertex(glm::vec3 pos, glm::vec2 tex)
	{
		m_pos = pos;
		m_tex = tex;
		m_normal = glm::vec3(0.0f, 0.0f, 0.0f);
	}
};

void CalcNormals(const unsigned int* pIndices, unsigned int IndexCount,
	Vertex* pVertices, unsigned int VertexCount)
{
	// Accumulate each triangle normal into each of the triangle vertices
	for (unsigned int i = 0; i < IndexCount; i += 3) {
		unsigned int Index0 = pIndices[i];
		unsigned int Index1 = pIndices[i + 1];
		unsigned int Index2 = pIndices[i + 2];
		glm::vec3 v1 = pVertices[Index1].m_pos - pVertices[Index0].m_pos;
		glm::vec3 v2 = pVertices[Index2].m_pos - pVertices[Index0].m_pos;
		glm::vec3 Normal = glm::cross(v1, v2);
		Normal = glm::normalize(Normal);

		pVertices[Index0].m_normal += Normal;
		pVertices[Index1].m_normal += Normal;
		pVertices[Index2].m_normal += Normal;
	}

	// Normalize all the vertex normals
	for (unsigned int i = 0; i < VertexCount; i++) {
		pVertices[i].m_normal = glm::normalize(pVertices[i].m_normal);
	}
}


void CreateIndexBuffer(const unsigned int* pIndices, unsigned int SizeInBytes, GLuint& m_IBO)
{
	glGenBuffers(1, &m_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, SizeInBytes, pIndices, GL_STATIC_DRAW);
}

void CreateVertexBuffer(const unsigned int* pIndices, unsigned int IndexCount, GLuint& m_VBO)
{
	Vertex Vertices[4] = { Vertex(glm::vec3(-1.0f, -1.0f, 0.5773f), glm::vec2(0.0f, 0.0f)),
		Vertex(glm::vec3(0.0f, -1.0f, -1.15475f), glm::vec2(0.5f, 0.0f)),
		Vertex(glm::vec3(1.0f, -1.0f, 0.5773f),  glm::vec2(1.0f, 0.0f)),
		Vertex(glm::vec3(0.0f, 1.0f, 0.0f),      glm::vec2(0.5f, 1.0f)) };

	unsigned int VertexCount = ARRAY_SIZE_IN_ELEMENTS(Vertices);

	CalcNormals(pIndices, IndexCount, Vertices, VertexCount);

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

#endif