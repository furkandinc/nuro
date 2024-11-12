#include "game_logic.h"

#include <iostream>

#include "../engine/runtime/runtime.h"
#include "../engine/entity/entity.h"
#include "../engine/camera/camera.h"
#include "../engine/rendering/material/unlit/unlit_material.h"
#include "../engine/rendering/material/lit/lit_material.h"

Camera* camera = nullptr;
Entity* cube = nullptr;

LitMaterial* sphereMaterial = nullptr;

void awake() {
	// Set default skybox
	// Runtime::setSkybox(Runtime::defaultSkybox);
	// Runtime::defaultSkybox->emission = 6.0f;
	Runtime::clearColor = glm::vec4(0.01f, 0.01f, 0.01f, 1.0f);

	// Create camera
	camera = new Camera();
	Runtime::useCamera(camera);

	// Create all entities
	sphereMaterial = new LitMaterial();
	sphereMaterial->baseColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	sphereMaterial->roughness = 0.16f;
	sphereMaterial->metallic = 0.04f;
	Model* sphereModel = new Model("./user/assets/models/sphere.fbx", { sphereMaterial });
	Entity* sphere = new Entity();
	sphere->model = sphereModel;
	sphere->transform.position = glm::vec3(0.0f, 0.0f, 6.5f);

	Texture* sphereAlbedo = new Texture("./user/assets/textures/mat_albedo.jpg", ALBEDO);
	Texture* sphereRoughness = new Texture("./user/assets/textures/mat_roughness.jpg", ROUGHNESS);
	Texture* sphereMetallic = new Texture("./user/assets/textures/mat_metallic.jpg", METALLIC);
	Texture* sphereNormal = new Texture("./user/assets/textures/mat_normal.jpg", NORMAL);
	LitMaterial* pbrSphereMaterial = new LitMaterial();
	pbrSphereMaterial->setAlbedoMap(sphereAlbedo);
	pbrSphereMaterial->setRoughnessMap(sphereRoughness);
	pbrSphereMaterial->setMetallicMap(sphereMetallic);
	pbrSphereMaterial->setNormalMap(sphereNormal);
	Model* pbrSphereModel = new Model("./user/assets/models/sphere.fbx", { pbrSphereMaterial });
	Entity* pbrSphere = new Entity();
	pbrSphere->model = pbrSphereModel;
	pbrSphere->transform.position = glm::vec3(3.0f, 0.0f, 6.5f);

	Texture* plankAlbedo = new Texture("./user/assets/textures/plank.jpg", ALBEDO);
	LitMaterial* plank = new LitMaterial();
	plank->tiling = glm::vec2(2.0f,  2.0f);
	plank->setAlbedoMap(plankAlbedo);
	plank->roughness = 0.0f;
	plank->metallic = 0.0f;
	Model* cubeModel = new Model("./user/assets/models/cube.obj", { plank, plank });
	cube = new Entity();
	cube->model = cubeModel;
	cube->transform.position = glm::vec3(-3.0f, 1.5f, 6.5f);

	LitMaterial* floorMaterial = new LitMaterial();
	floorMaterial->baseColor = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f);
	floorMaterial->roughness = 0.35f;
	Model* floorModel = new Model("./user/assets/models/cube.obj", { floorMaterial, floorMaterial });
	floorModel->castsShadow = false;
	Entity* floor = new Entity();
	floor->model = floorModel;
	floor->transform.position = glm::vec3(0.0f, -1.0f, 0.0f);
	floor->transform.scale = glm::vec3(150.0f, 0.1f, 150.0f);

	LitMaterial* wallMaterial = new LitMaterial();
	wallMaterial->baseColor = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f);
	wallMaterial->roughness = 0.35f;
	Model* wallModel = new Model("./user/assets/models/cube.obj", { wallMaterial, wallMaterial });
	wallModel->castsShadow = true;
	Entity* wall = new Entity();
	wall->model = wallModel;
	wall->transform.position = glm::vec3(0.0f, -1.0f, 10.0f);
	wall->transform.scale = glm::vec3(7.0f, 5.0f, 0.1f);

	/*Texture* smearedWallAlbedo = new Texture("./user/assets/textures/Smeared Wall_BaseColor.jpg", ALBEDO);
	Texture* smearedWallRoughness = new Texture("./user/assets/textures/Smeared Wall_Roughness.jpg", ROUGHNESS);
	Texture* smearedWallNormal = new Texture("./user/assets/textures/Smeared Wall_Normal.jpg", NORMAL);
	LitMaterial* smearedWallMaterial = new LitMaterial();
	smearedWallMaterial->baseColor = glm::vec4(1.0f, 0.96f, 0.86f, 1.0f);
	smearedWallMaterial->setAlbedoMap(smearedWallAlbedo);
	smearedWallMaterial->roughness = 0.48f;
	smearedWallMaterial->setMetallicMap(smearedWallRoughness);
	smearedWallMaterial->setNormalMap(smearedWallNormal);
	smearedWallMaterial->tiling = glm::vec2(20.0f, 10.0f);
	Model* smearedWallModel = new Model("./user/assets/models/cube.obj", { smearedWallMaterial, smearedWallMaterial });
	Entity* smearedWall = new Entity();
	smearedWall->model = smearedWallModel;
	smearedWall->transform.position = glm::vec3(20.0f, -1.0f, 10.0f);
	smearedWall->transform.scale = glm::vec3(10.0f, 5.0f, 0.1f);

	Texture* mannequinAlbedo = new Texture("./user/assets/textures/mannequin_albedo.jpg", ALBEDO);
	Texture* mannequinRoughness = new Texture("./user/assets/textures/mannequin_roughness.jpg", ROUGHNESS);
	Texture* mannequinMetallic = new Texture("./user/assets/textures/mannequin_metallic.jpg", METALLIC);
	LitMaterial* mannequinMaterial = new LitMaterial();
	mannequinMaterial->setAlbedoMap(mannequinAlbedo);
	mannequinMaterial->setRoughnessMap(mannequinRoughness);
	mannequinMaterial->setMetallicMap(mannequinMetallic);
	mannequinMaterial->baseColor = glm::vec4(0.5f, 0.1f, 0.1f, 1.0f);
	Model* mannequinModel = new Model("./user/assets/models/mannequin.fbx", mannequinMaterial);
	Entity* mannequin = new Entity();
	mannequin = new Entity();
	mannequin->model = mannequinModel;
	mannequin->transform.position = glm::vec3(14.0f, -0.9f, 8.5f);
	mannequin->transform.rotation = glm::vec3(90.0f, 0.0f, 0.0f);
	mannequin->transform.scale = glm::vec3(1.4f);*/
}

void update() {
	// cube->transform.rotation.y += 80.0f * Runtime::deltaTime;
	cube->transform.rotation.y += 20.0f * Runtime::deltaTime;
}