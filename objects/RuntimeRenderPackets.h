#ifndef W3D_RUNTIME_RENDER_PACKETS_H
#define W3D_RUNTIME_RENDER_PACKETS_H

#include <stddef.h>
#include "math/Matrix4.h"

class Mesh;
class Material;

struct W3dRuntimeRenderPacketInfo {
    Mesh* mesh;
    Material* material;
    Matrix4 world;
    unsigned long long sortKey;
    size_t submissionOrder;
    size_t materialGroup;
    int indexStart;
    int indexCount;
    bool translucent;
    float depth;

    W3dRuntimeRenderPacketInfo()
        : mesh(0), material(0), sortKey(0), submissionOrder(0), materialGroup(0),
          indexStart(0), indexCount(0), translucent(false), depth(0.0f) {}
};

void W3dRuntimeRenderPacketsBegin();
void W3dRuntimeRenderPacketsSort();
void W3dRuntimeRenderPacketsFlush();
bool W3dRuntimeRenderPacketsCapture(Mesh* mesh);
bool W3dRuntimeRenderPacketsSetCapture(bool enabled);
bool W3dRuntimeRenderPacketsSetEnabled(bool enabled);
bool W3dRuntimeRenderPacketsEnabled();
const W3dRuntimeRenderPacketInfo* W3dRuntimeRenderPacketsLast(size_t* count);
size_t W3dRuntimeRenderPacketsMemoryBytes();

#endif
