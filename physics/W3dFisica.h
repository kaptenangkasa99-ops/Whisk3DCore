#ifndef W3DFISICA_H
#define W3DFISICA_H

// ============================================================================
//  W3dFisica — MINIMAL Core physics: velocity + AABB bounce. Nothing else.
//  Designed for simple, lightweight games (2D and 3D) that must run even on
//  a Symbian device: pure C++03, no external engine, zero allocations per frame,
//  and ZERO COST if no object uses velocity (the step exits on the first line).
//
//  WHAT IT DOES (and what it DOES NOT):
//    * Integrates pos += velocity * dt for objects that have velocity.
//    * Bounces an AABB against another object, against a rect, or against the area
//      of another object, separating them so they don't get stuck.
//    * NO gravity, NO mass, NO rotation, NO friction, NO multiple-contact
//      resolution. The game handles that in Lua if needed.
//
//  WHERE IT RUNS: W3dFisicaPaso(dt) is called on BOTH sides with the SAME dt,
//  and BEFORE running the frame's scripts:
//      runtime  -> W3dGameActualizar()  (Whisk3D-Examples/game/w3drun.cpp)
//      editor   -> TickReal()           (main/script/SimJuego.cpp, the Play mode)
//  This ensures the game behaves IDENTICALLY compiled vs in Play mode. The order
//  "move first, then run the script" is intentional: the script sees the new
//  position and resolves the collision (bounce) within the SAME frame, before rendering.
//
//  SPACES (Important). Each object integrates within ITS OWN space, the same one
//  already used by Lua bindings to read/write it:
//    * 2D objects (ui/text/image/rect/container/slice9/button/expand/video):
//      CANVAS in px, centered at (0,0) — same as posPx/tamPx/cajaUI/chocan. Axes:
//      +x to the RIGHT, +y DOWNWARDS (it's a screen, not a mathematical plane).
//      In the object, position is stored as a FRACTION of the canvas; px <-> fraction
//      conversion is handled automatically by the physics system (the adapter below
//      provides canvas size and object size).
//    * 3D objects: ENGINE units, engine axes (Y up), and position is LOCAL
//      (relative to parent), same as posicion()/mover().
//  The Z axis: in 3D, it's the normal depth axis; in 2D, it's the depth in px
//  of the element (integrated as-is, without conversion). A 2D game leaves vz=0.
//
//  2D NOTE: The MOVING BODY (the one with velocity) is read/written RAW
//  (pos * canvas), exactly like posPx/setPosPx. This is deliberate: the physics
//  integrates and separates by writing that raw pos, meant for objects the game
//  moves freely (paddles, balls: direct children of the root, center anchor, free layout),
//  not for anchored/flex widgets.
//  OTHER operands follow the RAW-vs-RESOLVED rule (the same as chocan in BindsJuego):
//    * OBSTACLE in rebotar(obj, otro): if its raw pos is authoritative (determined
//      by the 'raw' adapter below: free layout + center anchor), the RAW box of
//      THIS frame is used (a paddle moved by the script in this frame hits where it
//      is NOW, without lag); if placed by layout, the RESOLVED rect from the last
//      render is used (where it is actually drawn).
//    * AREA (the court in rebotarEn(obj, area)): ALWAYS the layout rect (the same
//      one returned by cajaUI). An area is precisely a widget positioned by layout,
//      so the game doesn't need to recalculate borders: layout lives in .w3dui.
//      (An area is not moved via script: no lag is possible.)
//
//  Collision BOX (used by bounces):
//    * 2D: Defaults to the element's rect (width x height in canvas px), the SAME
//      one compared by chocan(). Zero configuration required.
//    * 3D: Defaults to a cube with side 2*RadioFoco() (mesh bounding) centered
//      on the object. It's an approximation: if insufficient, the game sets the
//      exact box using caja(obj, width, height, depth).
//
//  Data LIFETIME: Object velocity/box data lives in a small list (one slot per
//  moving object). Everything is cleared in W3dFisicaLimpiar(), which calls
//  W3dScriptDescargarTodo() -> stopping the editor or quitting the game leaves
//  the list empty. Nothing is stored inside Object.
//  A SCENE CHANGE does not clear it (there may be bodies that don't belong to any
//  scene): if a scene leaves something moving, the script must stop it upon exit
//  using velocidad(obj, 0, 0) or reset it in its inicio().
// ============================================================================
class Object;

// ---- paso de simulacion ----------------------------------------------------
// integra pos += vel*dt de TODOS los cuerpos con velocidad. Si no hay ninguno
// no hace nada (un juego sin fisica no paga costo).
void W3dFisicaPaso(float dt);
// olvida todos los cuerpos (Stop / fin del juego). La llama W3dScriptDescargarTodo().
void W3dFisicaLimpiar(void);
// olvida UN objeto (por si el juego lo destruye a mano).
void W3dFisicaOlvidar(Object* o);
// true si el objeto tiene un cuerpo (velocidad o caja propia asignada).
bool W3dFisicaTiene(Object* o);
// true si hay algun cuerpo vivo.
bool W3dFisicaHay(void);

// ---- velocidad desde C++ (el editor la usa para su snapshot por frame) -----
// unidades: px de lienzo por segundo en 2D, unidades del motor por segundo en 3D.
void W3dFisicaGetVel(Object* o, float* vx, float* vy, float* vz);
void W3dFisicaSetVel(Object* o, float vx, float vy, float vz);

// ---- 2D adapter ----------------------------------------------------------
// The Core DOES NOT know the game's content in the Elemento2D (lives in main/), so
// that main/ provides four functions (they register BindsJuegoRegistrar, which
// runs both in the editor and in the runtime):
// lienzo(w,h) -> size of the game lienzo in px (UI2D_SizeLienzo)
// size2d(o,anc,alt) -> size of the 2D object in linen px; false if it is not 2D
// cajaui(o,...) -> rect RESUELTO by the layout (anclas/flex) of the last
// render, center + size in linen px; false yes
// object is not released however. It's the same thing that returns
// cajaUI() in lua; is used for the AREAS and for them
// Non-authoritative OBSTACLES (see the top note).
// It can be NULL.
// cruda(o) -> true if there is CRUDA of the object is AUTORITATIVE (hijo
// direct from the root + free layout + ancla center: there
// regla crudo-vs-resuelto de chocan, PosCrudaAutoritativa
// en BindsJuego). Decide the OBSTACLE box
// rebound(). NULL = everything is considered raw (el
// historical behavior).
// SIN adapter registered the physics treats ALL as 3D (the positions of them
// 2D objects fall into fractions): real builds are registered.
typedef void (*W3dFisicaLienzoFn)(float* w, float* h);
typedef bool (*W3dFisicaTam2DFn)(Object* o, float* ancho, float* alto);
typedef bool (*W3dFisicaCajaUIFn)(Object* o, float* cx, float* cy, float* ancho, float* alto);
typedef bool (*W3dFisicaCrudaFn)(Object* o);
void W3dFisicaSetAdaptador2D(W3dFisicaLienzoFn lienzo, W3dFisicaTam2DFn tam2d,
                             W3dFisicaCajaUIFn cajaui, W3dFisicaCrudaFn cruda);

// ---- binds lua -------------------------------------------------------------
// los registra RegistrarAPI de script/W3dScript.cpp (por eso los ve igual el
// Play del editor y el juego compilado). Recibe el lua_State crudo como void*.
void W3dFisicaRegistrarBinds(void* L);

#endif // W3DFISICA_H
