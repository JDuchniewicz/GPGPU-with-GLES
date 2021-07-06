#include "shaders.h"
//#include <errno.h>
//#include <unistd.h>

// the outbound buffers have to be freed by the caller
int gpgpu_load_shaders(EVertexShader vertType, EFragmentShader fragType, GLchar** vSource, GLchar** fSource)
{
    int ret = 0, length = 0;
    GLchar* vBuffer = NULL;
    GLchar* fBuffer = NULL;
    FILE* fp = NULL;
    if (vertType == NONE || fragType == NONE)
        ERR("Both shader types have to be specified");

    // vertex
//    printf("%s\n", vFileNames[vertType].filename);
//    char cwd[128];
//    getcwd(cwd, sizeof(cwd));
//    printf("%s\n", cwd);
    fp = fopen(vFileNames[vertType].filename, "r");
//    printf("%d\n", errno);
    if (!fp)
        ERR("Could not open Vertex shader source");

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    vBuffer = malloc(length * sizeof(GLchar));
    fread(vBuffer, sizeof(GLchar), length, fp);
    fclose(fp);

    // fragment
    fp = fopen(fFileNames[fragType].filename, "r");
    if (!fp)
        ERR("Could not open Fragment shader source");

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    fBuffer = malloc(length * sizeof(GLchar));
    fread(fBuffer, sizeof(GLchar), length, fp);

    *vSource = vBuffer;
    *fSource = fBuffer;
bail:
    fclose(fp);
    return ret;
}
