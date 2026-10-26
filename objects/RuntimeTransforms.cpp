#include "RuntimeTransforms.h"
#include "Objects.h"

#include <vector>

namespace {
struct RuntimeTransformStorage {
    std::vector<unsigned> parentIndices;
    std::vector<Matrix4> localTransforms;
    std::vector<Matrix4> worldTransforms;
    std::vector<const Object*> owners;
    bool active;

    RuntimeTransformStorage() : active(false) {}
};

static RuntimeTransformStorage gRuntimeTransforms;

static void AppendSubtree(Object* object, unsigned parentIndex, bool hasParent) {
    if (!object) return;

    RuntimeTransformStorage& storage = gRuntimeTransforms;
    const unsigned index = (unsigned)storage.localTransforms.size();
    Matrix4 identity;
    identity.Identity();
    storage.parentIndices.push_back(hasParent ? parentIndex : (unsigned)-1);
    storage.localTransforms.push_back(identity);
    storage.worldTransforms.push_back(identity);
    storage.owners.push_back(object);
    object->runtimeTransformHandle = index;

    for (size_t i = 0; i < object->Childrens.size(); ++i)
        AppendSubtree(object->Childrens[i], index, true);
}

static const Matrix4* FindTransform(const Object* object,
                                    const std::vector<Matrix4>& transforms) {
    if (!gRuntimeTransforms.active || !object) return NULL;
    const unsigned index = object->runtimeTransformHandle;
    if (index >= transforms.size() || index >= gRuntimeTransforms.owners.size())
        return NULL;
    if (gRuntimeTransforms.owners[index] != object) return NULL;
    return &transforms[index];
}
}

void W3dRuntimeTransformsBuild(Object* root) {
    RuntimeTransformStorage& storage = gRuntimeTransforms;
    storage.active = false;
    storage.parentIndices.clear();
    storage.localTransforms.clear();
    storage.worldTransforms.clear();
    storage.owners.clear();
    if (!root) return;

    AppendSubtree(root, 0, false);
    for (size_t i = 0; i < storage.owners.size(); ++i) {
        const unsigned parentIndex = storage.parentIndices[i];
        const bool hasParent = parentIndex != (unsigned)-1;
        const Matrix4* parentWorld = hasParent ? &storage.worldTransforms[parentIndex] : NULL;
        storage.owners[i]->GetMatrixWithParentWorld(parentWorld, storage.localTransforms[i]);
        if (hasParent)
            storage.worldTransforms[i] = storage.worldTransforms[parentIndex] * storage.localTransforms[i];
        else
            storage.worldTransforms[i] = storage.localTransforms[i];
    }
}

void W3dRuntimeTransformsActivate(bool active) {
    gRuntimeTransforms.active = active;
}

const Matrix4* W3dRuntimeTransformLocal(const Object* object) {
    return FindTransform(object, gRuntimeTransforms.localTransforms);
}

const Matrix4* W3dRuntimeTransformWorld(const Object* object) {
    return FindTransform(object, gRuntimeTransforms.worldTransforms);
}

unsigned W3dRuntimeTransformParent(const Object* object) {
    if (!gRuntimeTransforms.active || !object) return (unsigned)-1;
    const unsigned index = object->runtimeTransformHandle;
    if (index >= gRuntimeTransforms.parentIndices.size() ||
        index >= gRuntimeTransforms.owners.size() ||
        gRuntimeTransforms.owners[index] != object)
        return (unsigned)-1;
    return gRuntimeTransforms.parentIndices[index];
}

int W3dRuntimeTransformsCount() {
    return (int)gRuntimeTransforms.localTransforms.size();
}

size_t W3dRuntimeTransformsMemoryBytes() {
    const RuntimeTransformStorage& storage = gRuntimeTransforms;
    return storage.parentIndices.capacity() * sizeof(unsigned) +
           storage.localTransforms.capacity() * sizeof(Matrix4) +
           storage.worldTransforms.capacity() * sizeof(Matrix4) +
           storage.owners.capacity() * sizeof(const Object*);
}

W3dRuntimeTransformsScope::W3dRuntimeTransformsScope(Object* root) {
    W3dRuntimeTransformsBuild(root);
    W3dRuntimeTransformsActivate(true);
}

W3dRuntimeTransformsScope::~W3dRuntimeTransformsScope() {
    W3dRuntimeTransformsActivate(false);
}
