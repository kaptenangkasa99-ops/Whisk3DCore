#ifndef W3D_RUNTIME_TRANSFORMS_H
#define W3D_RUNTIME_TRANSFORMS_H

#include <stddef.h>

class Matrix4;
class Object;

// Runtime-only packed transform snapshot. The authoring Object tree remains the
// source of truth; rebuild after simulation so script/animation edits are visible.
void W3dRuntimeTransformsBuild(Object* root);
void W3dRuntimeTransformsActivate(bool active);
const Matrix4* W3dRuntimeTransformLocal(const Object* object);
const Matrix4* W3dRuntimeTransformWorld(const Object* object);
unsigned W3dRuntimeTransformParent(const Object* object);
int W3dRuntimeTransformsCount();
size_t W3dRuntimeTransformsMemoryBytes();

class W3dRuntimeTransformsScope {
public:
    explicit W3dRuntimeTransformsScope(Object* root);
    ~W3dRuntimeTransformsScope();

private:
    W3dRuntimeTransformsScope(const W3dRuntimeTransformsScope&);
    W3dRuntimeTransformsScope& operator=(const W3dRuntimeTransformsScope&);
};

#endif
