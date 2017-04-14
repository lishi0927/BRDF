#ifndef DIFFUSE_LIGHT_H
#define DIFFUSE_LIGHT_H

#include <gl/glew.h>
#include <glm/glm.hpp>

struct DirectionalLight
{
	glm::vec3 Color;
	float AmbientIntensity;
	glm::vec3 Direction;
	float DiffuseIntensity;
};

struct diffuseshaderuniform
{
	GLuint shader;
	GLuint m_WVPLocation;
	GLuint m_WorldMatrixLocation;
	GLuint m_samplerLocation;

	struct {
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint Direction;
		GLuint DiffuseIntensity;
	} m_dirLightLocation;

	bool Init()
	{
		m_WVPLocation = glGetUniformLocation(shader, "gWVP");
		m_WorldMatrixLocation = glGetUniformLocation(shader, "gWorld");
		m_samplerLocation = glGetUniformLocation(shader, "gSampler");
		m_dirLightLocation.Color = glGetUniformLocation(shader, "gDirectionalLight.Color");
		m_dirLightLocation.AmbientIntensity = glGetUniformLocation(shader, "gDirectionalLight.AmbientIntensity");
		m_dirLightLocation.Direction = glGetUniformLocation(shader, "gDirectionalLight.Direction");
		m_dirLightLocation.DiffuseIntensity = glGetUniformLocation(shader, "gDirectionalLight.DiffuseIntensity");

		if (m_dirLightLocation.AmbientIntensity == 0xFFFFFFFF ||
			m_WVPLocation == 0xFFFFFFFF ||
			m_WorldMatrixLocation == 0xFFFFFFFF ||
			m_samplerLocation == 0xFFFFFFFF ||
			m_dirLightLocation.Color == 0xFFFFFFFF ||
			m_dirLightLocation.DiffuseIntensity == 0xFFFFFFFF ||
			m_dirLightLocation.Direction == 0xFFFFFFFF) {
			return false;
		}

		return true;
	}

	void Enable()
	{
		glUseProgram(shader);
	}

	void SetWVP(const glm::mat4 &WVP)
	{
		glUniformMatrix4fv(m_WVPLocation, 1, GL_FALSE, &WVP[0][0]);
	}

	void SetWorldMatrix(const glm::mat4 &WorldMatrix)
	{
		glUniformMatrix4fv(m_WorldMatrixLocation, 1, GL_FALSE, &WorldMatrix[0][0]);
	}

	void SetTextureUnit(unsigned int TextureUnit)
	{
		glUniform1i(m_samplerLocation, TextureUnit);
	}

	void SetDirectionalLight(const DirectionalLight& Light)
	{
		glUniform3f(m_dirLightLocation.Color, Light.Color.x, Light.Color.y, Light.Color.z);
		glUniform1f(m_dirLightLocation.AmbientIntensity, Light.AmbientIntensity);
		glm::vec3 Direction = Light.Direction;
		Direction = glm::normalize(Direction);
		glUniform3f(m_dirLightLocation.Direction, Direction.x, Direction.y, Direction.z);
		glUniform1f(m_dirLightLocation.DiffuseIntensity, Light.DiffuseIntensity);
	}

}Diffuseshader;

#endif
