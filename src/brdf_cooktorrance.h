#ifndef BRDF_COOKTORRANCE_H
#define BRDF_COOKTORRANCE_H

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <stdio.h>
#include <cmath>

#define SNPRINTF _snprintf_s
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

#define ToRadian(x) (float)(((x) * M_PI / 180.0f))
#define ToDegree(x) (float)(((x) * 180.0f / M_PI))

struct BaseLight
{
	glm::vec3 Color;
	float AmbientIntensity;
	float DiffuseIntensity;

	BaseLight()
	{
		Color = glm::vec3(0.0f, 0.0f, 0.0f);
		AmbientIntensity = 0.0f;
		DiffuseIntensity = 0.0f;
	}
};

struct DirectionalLight : public BaseLight
{
	glm::vec3 Direction;

	DirectionalLight()
	{
		Direction = glm::vec3(0.0f, 0.0f, 0.0f);
	}
};

struct PointLight : public BaseLight
{
	glm::vec3 Position;

	struct
	{
		float Constant;
		float Linear;
		float Exp;
	} Attenuation;

	PointLight()
	{
		Position = glm::vec3(0.0f, 0.0f, 0.0f);
		Attenuation.Constant = 1.0f;
		Attenuation.Linear = 0.0f;
		Attenuation.Exp = 0.0f;
	}
};


struct CookTorranceShaderUniform {
	static const unsigned int MAX_POINT_LIGHTS = 2;
	static const unsigned int MAX_SPOT_LIGHTS = 2;

	GLuint shader;
	GLuint m_WVPLocation;
	GLuint m_WorldMatrixLocation;
	GLuint m_samplerLocation;
	GLuint m_eyeWorldPosLocation;
	GLuint m_numPointLightsLocation;
	GLuint m_roughnessLocation;
	GLuint m_FresnelF0Location;
	GLuint m_gKLocation;

	struct {
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Direction;
	} m_dirLightLocation;

	struct {
		GLuint Color;
		GLuint AmbientIntensity;
		GLuint DiffuseIntensity;
		GLuint Position;
		struct {
			GLuint Constant;
			GLuint Linear;
			GLuint Exp;
		} Atten;
	} m_pointLightsLocation[MAX_POINT_LIGHTS];


	bool Init()
	{
		m_WVPLocation = glGetUniformLocation(shader, "gWVP");
		m_WorldMatrixLocation = glGetUniformLocation(shader, "gWorld");
		m_samplerLocation = glGetUniformLocation(shader, "gSampler");
		m_eyeWorldPosLocation = glGetUniformLocation(shader, "gEyeWorldPos");
		m_dirLightLocation.Color = glGetUniformLocation(shader, "gDirectionalLight.Base.Color");
		m_dirLightLocation.AmbientIntensity = glGetUniformLocation(shader, "gDirectionalLight.Base.AmbientIntensity");
		m_dirLightLocation.Direction = glGetUniformLocation(shader, "gDirectionalLight.Direction");
		m_dirLightLocation.DiffuseIntensity = glGetUniformLocation(shader, "gDirectionalLight.Base.DiffuseIntensity");
		m_numPointLightsLocation = glGetUniformLocation(shader, "gNumPointLights");
		m_roughnessLocation = glGetUniformLocation(shader, "gRoughnessvalue");
		m_FresnelF0Location = glGetUniformLocation(shader, "gFresnelF0");
		m_gKLocation = glGetUniformLocation(shader, "gK");

		if (m_dirLightLocation.AmbientIntensity == 0xFFFFFFFF ||
			m_WVPLocation == 0xFFFFFFFF ||
			m_WorldMatrixLocation == 0xFFFFFFFF ||
			m_samplerLocation == 0xFFFFFFFF ||
			m_eyeWorldPosLocation == 0xFFFFFFFF ||
			m_dirLightLocation.Color == 0xFFFFFFFF ||
			m_dirLightLocation.DiffuseIntensity == 0xFFFFFFFF ||
			m_dirLightLocation.Direction == 0xFFFFFFFF ||
//			m_matSpecularIntensityLocation == 0xFFFFFFFF ||
//			m_matSpecularPowerLocation == 0xFFFFFFFF ||
			m_numPointLightsLocation == 0xFFFFFFFF ||
			m_roughnessLocation == 0xFFFFFFFF ||
			m_FresnelF0Location == 0xFFFFFFFF ||
			m_gKLocation == 0xFFFFFFFFF) {
			return false;
		}

		for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(m_pointLightsLocation); i++) {
			char Name[128];
			memset(Name, 0, sizeof(Name));
			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.Color", i);
			m_pointLightsLocation[i].Color = glGetUniformLocation(shader, Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.AmbientIntensity", i);
			m_pointLightsLocation[i].AmbientIntensity = glGetUniformLocation(shader, Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Position", i);
			m_pointLightsLocation[i].Position = glGetUniformLocation(shader, Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Base.DiffuseIntensity", i);
			m_pointLightsLocation[i].DiffuseIntensity = glGetUniformLocation(shader, Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Constant", i);
			m_pointLightsLocation[i].Atten.Constant = glGetUniformLocation(shader, Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Linear", i);
			m_pointLightsLocation[i].Atten.Linear = glGetUniformLocation(shader, Name);

			SNPRINTF(Name, sizeof(Name), "gPointLights[%d].Atten.Exp", i);
			m_pointLightsLocation[i].Atten.Exp = glGetUniformLocation(shader, Name);

			if (m_pointLightsLocation[i].Color == 0xFFFFFFFF ||
				m_pointLightsLocation[i].AmbientIntensity == 0xFFFFFFFF ||
				m_pointLightsLocation[i].Position == 0xFFFFFFFF ||
				m_pointLightsLocation[i].DiffuseIntensity == 0xFFFFFFFF ||
				m_pointLightsLocation[i].Atten.Constant == 0xFFFFFFFF ||
				m_pointLightsLocation[i].Atten.Linear == 0xFFFFFFFF ||
				m_pointLightsLocation[i].Atten.Exp == 0xFFFFFFFF) {
				return false;
			}
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

	void SetEyeWorldPos(const glm::vec3& EyeWorldPos)
	{
		glUniform3f(m_eyeWorldPosLocation, EyeWorldPos.x, EyeWorldPos.y, EyeWorldPos.z);
	}

	void SetRoughness(float roughness)
	{
		glUniform1f(m_roughnessLocation, roughness);
	}

	void SetFresnel(float F0)
	{
		glUniform1f(m_FresnelF0Location, F0);
	}

	void SetGK(float gk)
	{
		glUniform1f(m_gKLocation, gk);
	}

	void SetPointLights(unsigned int NumLights, const PointLight* pLights)
	{
		glUniform1i(m_numPointLightsLocation, NumLights);

		for (unsigned int i = 0; i < NumLights; i++) {
			glUniform3f(m_pointLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
			glUniform1f(m_pointLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
			glUniform1f(m_pointLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
			glUniform3f(m_pointLightsLocation[i].Position, pLights[i].Position.x, pLights[i].Position.y, pLights[i].Position.z);
			glUniform1f(m_pointLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
			glUniform1f(m_pointLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
			glUniform1f(m_pointLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
		}
	}


}CookShader;


DirectionalLight m_directionalLight;


void LightInit()
{
	m_directionalLight.Color = glm::vec3(1.0f, 1.0f, 1.0f);
	m_directionalLight.AmbientIntensity = 0.0f;
	m_directionalLight.DiffuseIntensity = 0.0f;
	m_directionalLight.Direction = glm::vec3(1.0f, -1.0, 0.0);
}


#endif
