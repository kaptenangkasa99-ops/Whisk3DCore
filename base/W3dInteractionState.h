#ifndef W3D_INTERACTIONSTATE_H
#define W3D_INTERACTIONSTATE_H

// Status of INTERACTION of the app, SHARED engine<->editor and therefore lives on the MOTOR.
// Before these globals + their enums colgaban de variables.h (header of the EDITOR), and el
// Core traversal included this header -> inverted the cover (Engine <- UI <- Editor). Now
// the engine declares and defines; the editor will WRITE according to the input, the Core will LEE
// (selection, gizmos). C++03 dialect (compiles with RVCT on Symbian).

// viewport modes (Blender style selector; only with an active MALLA). Edit your Paint
// however it is not implemented: for now the selector only changes InteractionMode.
enum { ObjectMode, EditMode, VertexPaint, WeightPaint, TexturePaint, PoseMode };
// viewport sub-state: navigating, using a transform (G/R/S/extrude...).
enum { editNavegacion, EdgeMove, FaceMove, timelineMove, rotacion, EditScale, translacion };

extern int InteractionMode; // uno de ObjectMode/EditMode/...
extern int estado;          // uno de editNavegacion/...

#endif
