#include "shader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <utils/fsutil.h>
#include <utils/console.h>

Shader::Shader() : sourcePath(),
data(),
uniforms(),
_backendId(0)
{
}

Shader::~Shader()
{
	freeIoData();
	deleteBuffers();
}

void Shader::setSource(const FS::Path& _sourcePath)
{
	// Validate source path
	if (!FS::exists(_sourcePath))
		Console::out::warning("Shader", "Shader source at '" + _sourcePath.string() + "' could not be found");

	sourcePath = _sourcePath;
}

void Shader::bind() const
{
	glUseProgram(_backendId);
}

uint32_t Shader::backendId() const
{
	return _backendId;
}

void Shader::setBool(const std::string& identifier, bool value)
{
	glUniform1i(getUniformLocation(identifier), (int32_t)value);
}
void Shader::setInt(const std::string& identifier, int32_t value)
{
	glUniform1i(getUniformLocation(identifier), value);
}
void Shader::setFloat(const std::string& identifier, float value)
{
	glUniform1f(getUniformLocation(identifier), value);
}
void Shader::setVec2(const std::string& identifier, glm::vec2 value)
{
	glUniform2f(getUniformLocation(identifier), value.x, value.y);
}
void Shader::setVec3(const std::string& identifier, glm::vec3 value)
{
	glUniform3f(getUniformLocation(identifier), value.x, value.y, value.z);
}
void Shader::setVec4(const std::string& identifier, glm::vec4 value)
{
	glUniform4f(getUniformLocation(identifier), value.x, value.y, value.z, value.w);
}
void Shader::setMatrix3(const std::string& identifier, glm::mat3 value)
{
	glUniformMatrix3fv(getUniformLocation(identifier), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setMatrix4(const std::string& identifier, glm::mat4 value)
{
	glUniformMatrix4fv(getUniformLocation(identifier), 1, GL_FALSE, glm::value_ptr(value));
}

int32_t Shader::getUniformLocation(const std::string& identifier)
{
	// Uniform found in cache, return uniform location
	if (uniforms.find(identifier) != uniforms.end()) return uniforms[identifier];

	// Uniform not found in cache, fetch and save uniform location
	int32_t location = glGetUniformLocation(_backendId, identifier.c_str());
	uniforms[identifier] = location;

	// Return newly fetched uniform location
	return location;
}

bool Shader::shaderCompiled(const char* type, int32_t shader)
{
	// Fetch shader compilation status
	int32_t success;
	char shader_log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	// Shader compilation failed, terminate program
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, shader_log);
		Console::out::warning("Shader", "Compilation of " + std::string(type) + " shader failed!", shader_log);
		return false;
	}

	// Shader compilation successful
	return true;
}

bool Shader::programLinked(int32_t program)
{
	// Fetch shader program linking status
	int32_t success;
	char shader_log[512];
	glGetProgramiv(program, GL_COMPILE_STATUS, &success);

	// Shader program linking failed, terminate program
	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, shader_log);
		Console::out::warning("Shader", "Linking of program failed!", shader_log);
		return false;
	}

	// Shader program linking successful
	return true;
}

bool Shader::loadIoData()
{
	data.vertexSource = FS::readFile(sourcePath / ".vert");
	data.fragmentSource = FS::readFile(sourcePath / ".frag");

	return true;
}

void Shader::freeIoData()
{
	data.vertexSource.clear();
	data.fragmentSource.clear();
}

bool Shader::uploadBuffers()
{
	// Don't dispatch shader if there is no data
	if (data.vertexSource.empty() || data.fragmentSource.empty()) return false;

	// Shader backend ids
	uint32_t vertexShader, fragmentShader;

	// Compile vertex shader source
	const char* vertexSource = data.vertexSource.c_str();
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, nullptr);
	glCompileShader(vertexShader);
	if (!shaderCompiled("vertex", vertexShader)) return false;

	// Compile fragment shader source
	const char* fragmentSource = data.fragmentSource.c_str();
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
	glCompileShader(fragmentShader);
	if (!shaderCompiled("fragment", fragmentShader)) return false;

	// Create and link shader program
	_backendId = glCreateProgram();
	glAttachShader(_backendId, vertexShader);
	glAttachShader(_backendId, fragmentShader);
	glLinkProgram(_backendId);
	if (!programLinked(_backendId)) return false;

	// Delete shader sources
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return true;
}

void Shader::deleteBuffers()
{
	if (_backendId) glDeleteProgram(_backendId);
	_backendId = 0;
}