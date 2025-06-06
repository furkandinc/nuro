#include "texture.h"

#include <glad/glad.h>
#include <stb_image.h>

#include <utils/fsutil.h>
#include <utils/console.h>
#include <context/application_context.h>

uint32_t Texture::defaultTextureId = 0;

Texture::Texture() : type(TextureType::EMPTY),
sourcePath(),
width(0),
height(0),
channels(0),
data(nullptr),
_backendId(defaultTextureId)
{
}

Texture::~Texture()
{
	freeIoData();
	deleteBuffers();
}

void Texture::setSource(TextureType _type, const FS::Path& _sourcePath)
{
	// Validate source path
	if (!FS::exists(_sourcePath))
		Console::out::warning("Texture", "Texture source at '" + _sourcePath.string() + "' could not be found");

	type = _type;
	sourcePath = _sourcePath;
}

uint32_t Texture::backendId() const
{
	return _backendId;
}

void Texture::setDefaultTexture(uint32_t textureId)
{
	defaultTextureId = textureId;
}

bool Texture::loadIoData()
{
	// Load image data
	int _width, _height, _channels;
	stbi_set_flip_vertically_on_load(true);
	std::string pathStr = sourcePath.string();
	unsigned char* _data = stbi_load(pathStr.c_str(), &_width, &_height, &_channels, 0);
	if (!_data)
	{
		Console::out::warning("Texture", "Couldn't load data for texture '" + sourcePath.filename().string() + "'");
		return false;
	}

	// Sync loaded data
	width = _width;
	height = _height;
	channels = _channels;
	data = _data;

	return true;
}

void Texture::freeIoData()
{
	// No data loaded
	if (!data) 
		return;

	// Free memory allocated for image data
	stbi_image_free(data);

	return;
}

bool Texture::uploadBuffers()
{
	// Don't dispatch texture if there is no data
	if (!data) 
		return false;

	// Generate texture
	glGenTextures(1, &_backendId);
	glBindTexture(GL_TEXTURE_2D, _backendId);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Anisotropic filtering
	GLfloat maxAniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);

	// Get texture backend format from texture type
	GLenum internalFormat;
	GLenum format;
	switch (type)
	{
	case TextureType::IMAGE:
	{
		switch (channels) {
		case 1:
			internalFormat = GL_RED;
			format = GL_RED;
			break;
		case 2:
			internalFormat = GL_RG;
			format = GL_RG;
			break;
		case 3:
			internalFormat = GL_RGB;
			format = GL_RGB;
			break;
		case 4:
			internalFormat = GL_RGBA;
			format = GL_RGBA;
			break;
		default:
			internalFormat = GL_RGB;
			format = GL_RGB;
			break;
		}
		break;
	}
	case TextureType::ALBEDO:
		internalFormat = GL_SRGB;
		format = GL_RGB;
		break;
	case TextureType::ROUGHNESS:
		internalFormat = GL_RED;
		format = GL_RED;
		break;
	case TextureType::METALLIC:
		internalFormat = GL_RED;
		format = GL_RED;
		break;
	case TextureType::NORMAL:
		internalFormat = GL_RGB;
		format = GL_RGB;
		break;
	case TextureType::OCCLUSION:
		internalFormat = GL_RED;
		format = GL_RED;
		break;
	case TextureType::EMISSIVE:
		internalFormat = GL_RGB;
		format = GL_RGB;
		break;
	case TextureType::HEIGHT:
		internalFormat = GL_RED;
		format = GL_RED;
		break;
	default:
		internalFormat = GL_RGB;
		format = GL_RGB;
		break;
	}

	// Buffer image data to texture
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	// Generate textures mipmap
	glGenerateMipmap(GL_TEXTURE_2D);

	// Undbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void Texture::deleteBuffers()
{
	if (_backendId)
		glDeleteTextures(1, &_backendId);

	_backendId = defaultTextureId;
}