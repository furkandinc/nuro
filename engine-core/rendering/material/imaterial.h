#pragma once

#include <cstdint>

#include "../src/core/viewport/viewport.h"
#include "../src/core/rendering/shader/shader.h"

/* !!!!!!!!!!!!!!!!!!!!!!!!!!
   !!					   !!
   !! BAD TEMPORARY CODE!  !!
   !!					   !!
   !!!!!!!!!!!!!!!!!!!!!!!!!! */

class IMaterial
{
public:
	virtual ~IMaterial() {}

	virtual void bind() const = 0;
	virtual uint32_t getId() const = 0;
	virtual Shader* getShader() const = 0;
	virtual uint32_t getShaderId() const = 0;
};