#include <stdio.h>
#include "PVRShell.h"
#include "EglContext.h"

class GPGPUwithGLESShell : public pvr::Shell
{
	EglContext _context;
	// The vertex and fragment shader OpenGL handles
	uint32_t _vertexShader, _fragShader;

	// The program object containing the 2 shader objects
	uint32_t _program;

	// VBO handle
	uint32_t _vbo;

public:
	// following function must be override
	virtual pvr::Result initApplication();
	virtual pvr::Result initView();
	virtual pvr::Result releaseView();
	virtual pvr::Result quitApplication();
	virtual pvr::Result renderFrame();

};

int main()
{
    return 0;
}
