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
	Runtime::setSkybox(Runtime::defaultSkybox);

	// Create camera
	camera = new Camera();
	Runtime::useCamera(camera);

	// Create all entities
	sphereMaterial = new LitMaterial();
	sphereMaterial->baseColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	Model* sphereModel = new Model("./user/assets/models/sphere.fbx", { sphereMaterial });
	Entity* sphere = new Entity();
	sphere->model = sphereModel;
	sphere->transform.position = glm::vec3(0.0f, 0.0f, 3.6f);

	Texture* sphereAlbedo = new Texture("./user/assets/textures/mat_albedo.jpg", ALBEDO);
	Texture* sphereRoughness = new Texture("./user/assets/textures/mat_roughness.jpg", ROUGHNESS);
	Texture* sphereMetallic = new Texture("./user/assets/textures/mat_metallic.jpg", METALLIC);
	LitMaterial* pbrSphereMaterial = new LitMaterial();
	pbrSphereMaterial->setAlbedoMap(sphereAlbedo);
	pbrSphereMaterial->setRoughnessMap(sphereRoughness);
	pbrSphereMaterial->setMetallicMap(sphereMetallic);
	Model* pbrSphereModel = new Model("./user/assets/models/sphere.fbx", { pbrSphereMaterial });
	Entity* pbrSphere = new Entity();
	pbrSphere->model = pbrSphereModel;
	pbrSphere->transform.position = glm::vec3(5.0f, 0.0f, 0.0f);

	Texture* plankAlbedo = new Texture("./user/assets/textures/plank.jpg", ALBEDO);
	LitMaterial* plank = new LitMaterial();
	plank->tiling = glm::vec2(2.0f, 2.0f);
	plank->setAlbedoMap(plankAlbedo);
	plank->roughness = 0.0f;
	plank->metallic = 0.0f;
	Model* cubeModel = new Model("./user/assets/models/cube.obj", { plank, plank });
	cube = new Entity();
	cube->model = cubeModel;
	cube->transform.position = glm::vec3(-3.0f, 1.5f, 0.0f);

	LitMaterial* floorMaterial = new LitMaterial();
	floorMaterial->baseColor = glm::vec4(0.6f, 0.6f, 0.6f, 1.0f);
	floorMaterial->roughness = 0.6f;
	Model* floorModel = new Model("./user/assets/models/cube.obj", { floorMaterial, floorMaterial });
	Entity* floor = new Entity();
	floor->model = floorModel;
	floor->transform.position = glm::vec3(0.0f, -1.0f, 0.0f);
	floor->transform.scale = glm::vec3(25.0f, 0.1f, 25.0f);

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
	mannequin->transform.position = glm::vec3(2.0f, -0.9f, 1.0f);
	mannequin->transform.rotation = glm::vec3(90.0f, 0.0f, 0.0f);
	mannequin->transform.scale = glm::vec3(2.0f);
}

void update() {
	cube->transform.rotation.y += 80.0f * Runtime::deltaTime;

	sphereMaterial->roughness = Runtime::roughness;
	sphereMaterial->metallic = Runtime::metallic;
}