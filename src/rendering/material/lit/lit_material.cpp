#include "lit_material.h"

#include "../src/runtime/runtime.h"

LitMaterial::LitMaterial()
{
	shader = ShaderBuilder::get("lit");

	tiling = glm::vec2(1.0f);
	offset = glm::vec2(0.0f);

	albedoMap = nullptr;
	normalMap = nullptr;

	roughness = 0.0f;
	roughnessMap = nullptr;

	metallic = 0.0f;
	metallicMap = nullptr;

	ambientOcclusionMap = nullptr;

	emissionIntensity = 0.0f;
	emissionColor = glm::vec3(1.0f);
	emissionMap = nullptr;

	shader->bind();
	syncStaticUniforms();
	syncLightUniforms();
}

void LitMaterial::bind()
{
	shader->bind();

	// General parameters
	shader->setFloat("configuration.gamma", PostProcessing::configuration.gamma);
	shader->setBool("configuration.solidMode", Runtime::solidMode);

	// Shadow parameters
	shader->setBool("configuration.castShadows", Runtime::shadows);
	Runtime::mainShadowMap->bind(SHADOW_MAP_UNIT);
	shader->setFloat("configuration.shadowMapResolutionWidth", Runtime::mainShadowMap->getResolutionWidth());
	shader->setFloat("configuration.shadowMapResolutionHeight", Runtime::mainShadowMap->getResolutionHeight());

	Runtime::mainShadowDisk->bind(SHADOW_DISK_UNIT);
	shader->setFloat("configuration.shadowDiskWindowSize", Runtime::mainShadowDisk->getWindowSize());
	shader->setFloat("configuration.shadowDiskFilterSize", Runtime::mainShadowDisk->getFilterSize());
	shader->setFloat("configuration.shadowDiskRadius", Runtime::mainShadowDisk->getRadius());

	// World parameters
	shader->setVec3("configuration.cameraPosition", Transformation::prepareWorldPosition(Runtime::getCameraRendering()->transform.position));

	// Set material data
	shader->setVec4("material.baseColor", baseColor);

	shader->setVec2("material.tiling", tiling);
	shader->setVec2("material.offset", offset);

	shader->setFloat("material.emissionIntensity", emissionIntensity);
	shader->setVec3("material.emissionColor", emissionColor);

	shader->setBool("material.enableAlbedoMap", enableAlbedoMap);
	if (enableAlbedoMap) {
		albedoMap->bind(ALBEDO_MAP_UNIT);
	}

	shader->setBool("material.enableNormalMap", enableNormalMap && Runtime::normalMapping);
	if (enableNormalMap && Runtime::normalMapping) {
		normalMap->bind(NORMAL_MAP_UNIT);
	}
	shader->setFloat("material.normalMapIntensity", Runtime::normalMappingIntensity);

	shader->setBool("material.enableRoughnessMap", enableRoughnessMap);
	if (enableRoughnessMap) {
		roughnessMap->bind(ROUGHNESS_MAP_UNIT);
	}
	else {
		shader->setFloat("material.roughness", roughness);
	}

	shader->setBool("material.enableMetallicMap", enableMetallicMap);
	if (enableMetallicMap) {
		metallicMap->bind(METALLIC_MAP_UNIT);
	}
	else {
		shader->setFloat("material.metallic", metallic);
	}

	shader->setBool("material.enableAmbientOcclusionMap", enableAmbientOcclusionMap);
	if (enableAmbientOcclusionMap) {
		ambientOcclusionMap->bind(AMBIENT_OCCLUSION_MAP_UNIT);
	}

	shader->setBool("material.enableEmissionMap", enableEmissionMap);
	if (enableEmissionMap) {
		emissionMap->bind(EMISSION_MAP_UNIT);
	}

	// tmp debug
	shader->setFloat("directionalLights[0].intensity", Runtime::directionalIntensity);
	shader->setFloat("pointLights[0].intensity", Runtime::intensity);
	shader->setFloat("pointLights[0].range", Runtime::range);
	shader->setFloat("pointLights[0].falloff", Runtime::falloff);
}

Shader* LitMaterial::getShader()
{
	return shader;
}

void LitMaterial::setAlbedoMap(Texture* albedoMap)
{
	enableAlbedoMap = true;
	this->albedoMap = albedoMap;
}

void LitMaterial::setNormalMap(Texture* normalMap)
{
	enableNormalMap = true;
	this->normalMap = normalMap;
}

void LitMaterial::setRoughnessMap(Texture* roughnessMap)
{
	enableRoughnessMap = true;
	this->roughnessMap = roughnessMap;
}

void LitMaterial::setMetallicMap(Texture* metallicMap)
{
	enableMetallicMap = true;
	this->metallicMap = metallicMap;
}

void LitMaterial::setAmbientOcclusionMap(Texture* ambientOcclusionMap)
{
	enableAmbientOcclusionMap = true;
	this->ambientOcclusionMap = ambientOcclusionMap;
}

void LitMaterial::setEmissionMap(Texture* emissionMap)
{
	enableEmissionMap = true;
	this->emissionMap = emissionMap;
}

void LitMaterial::syncStaticUniforms()
{
	shader->setFloat("configuration.gamma", PostProcessing::configuration.gamma);

	shader->setBool("configuration.solidMode", Runtime::solidMode);
	shader->setBool("configuration.castShadows", Runtime::shadows);

	shader->setInt("material.albedoMap", ALBEDO_MAP_UNIT);
	shader->setInt("material.normalMap", NORMAL_MAP_UNIT);
	shader->setInt("material.roughnessMap", ROUGHNESS_MAP_UNIT);
	shader->setInt("material.metallicMap", METALLIC_MAP_UNIT);
	shader->setInt("material.ambientOcclusionMap", AMBIENT_OCCLUSION_MAP_UNIT);
	shader->setInt("material.emissionMap", EMISSION_MAP_UNIT);
	shader->setInt("configuration.shadowDisk", SHADOW_DISK_UNIT);
	shader->setInt("configuration.shadowMap", SHADOW_MAP_UNIT);
}

void LitMaterial::syncLightUniforms()
{
	// Lighting parameters
	shader->setInt("configuration.numDirectionalLights", 1);
	shader->setInt("configuration.numPointLights", 1);
	shader->setInt("configuration.numSpotLights", 1);

	shader->setFloat("directionalLights[0].intensity", Runtime::directionalIntensity);
	shader->setVec3("directionalLights[0].direction", Transformation::prepareWorldPosition(Runtime::directionalDirection));
	shader->setVec3("directionalLights[0].color", Runtime::directionalColor);
	shader->setVec3("directionalLights[0].position", Transformation::prepareWorldPosition(Runtime::directionalPosition));

	shader->setVec3("pointLights[0].position", Transformation::prepareWorldPosition(glm::vec3(0.0f, 0.0f, 6.5f)));
	shader->setVec3("pointLights[0].color", glm::vec3(0.0f, 0.78f, 0.95f));
	shader->setFloat("pointLights[0].intensity", 2.0f);
	shader->setFloat("pointLights[0].range", 10.0f);
	shader->setFloat("pointLights[0].falloff", 5.0f);

	shader->setVec3("spotLights[0].position", Transformation::prepareWorldPosition(glm::vec3(12.0f, 1.9f, -4.0f)));
	shader->setVec3("spotLights[0].direction", Transformation::prepareWorldPosition(glm::vec3(-0.4, -0.2f, 1.0f)));
	shader->setVec3("spotLights[0].color", glm::vec3(1.0f, 1.0f, 1.0f));
	shader->setFloat("spotLights[0].intensity", 3.5f);
	shader->setFloat("spotLights[0].range", 25.0f);
	shader->setFloat("spotLights[0].falloff", 10.0f);
	shader->setFloat("spotLights[0].innerCutoff", glm::cos(glm::radians(12.5f)));
	shader->setFloat("spotLights[0].outerCutoff", glm::cos(glm::radians(20.0f)));

	shader->setInt("fog.type", 0);
	/*shader->setInt("fog.type", 3);
	shader->setVec3("fog.color", glm::vec3(1.0f, 1.0f, 1.0f));
	shader->setFloat("fog.data[0]", 0.01);*/
}
