#pragma once

#include "renderable.h"
#include "render.h"
#include "math/vector.h"
#include "math/aabb.h"
#include "stringio.h"
#include <functional>
#include <set>
#include <map>

// Forward declarations to avoid circular includes
class TargetableInstance;
class Targetable;
class EntityKeyValues;

// User-editable function: decide whether to draw a gizmo for an entity and how.
// Default implementation does nothing. Modify this function to implement custom gizmos.
// Params:
//   entity - the entity key-values; use entity.getKeyValue("keyname") to check values
//   instance - the TargetableInstance for the entity; use instance.world_position(), instance.localToWorld()
//   renderer, volume - to add Renderables
// Return true if a gizmo was drawn for this entity.
bool CustomGizmos_DrawIfMatch( const EntityKeyValues& entity, const TargetableInstance& instance, Renderer& renderer, const VolumeTest& volume );

// Convenience helper draw functions that create RenderablePointVector objects and add them to the renderer.
void CustomGizmos_DrawArrow( Renderer& renderer, const Vector3& start, const Vector3& end, const VolumeTest& volume, const Colour4b& colour );
void CustomGizmos_DrawBox( Renderer& renderer, const AABB& aabb, const VolumeTest& volume, const Colour4b& colour );
void CustomGizmos_DrawSphere( Renderer& renderer, const Vector3& origin, float radius, const VolumeTest& volume, const Colour4b& colour );

class RenderableCustomGizmos : public Renderable
{
public:
    typedef std::set<TargetableInstance*> TargetableInstances;
    TargetableInstances m_instances;
    // per-instance drawables to guarantee lifetime until the renderer consumes
    // them (the render pipeline stores pointers to the OpenGLRenderable objects)
    struct GizmoDrawData {
        GizmoDrawData() : m_lines( GL_LINES ), m_loopX( GL_LINE_LOOP ), m_loopY( GL_LINE_LOOP ), m_loopZ( GL_LINE_LOOP ) {}
        void clear(){ m_lines.clear(); m_loopX.clear(); m_loopY.clear(); m_loopZ.clear(); }
        mutable RenderablePointVector m_lines;
        mutable RenderablePointVector m_loopX;
        mutable RenderablePointVector m_loopY;
        mutable RenderablePointVector m_loopZ;
    };
    mutable std::map<TargetableInstance*, GizmoDrawData> m_drawData;

    void attach( TargetableInstance& instance );
    void detach( TargetableInstance& instance );
    void renderSolid( Renderer& renderer, const VolumeTest& volume ) const override;
    void renderWireframe( Renderer& renderer, const VolumeTest& volume ) const override;
};

typedef Static<RenderableCustomGizmos> StaticRenderableCustomGizmos;

// ─────────────────────────────────────────────────────────────
// MAIN STRUCTURE
// ─────────────────────────────────────────────────────────────

// The CUSTOM_GIZMO_BEGIN and CUSTOM_GIZMO_END macros have been removed.
// Implement your custom gizmo drawing logic by defining the function:
//
//   bool CustomGizmos_DrawIfMatch(const EntityKeyValues& entity, const TargetableInstance& instance, Renderer& renderer, const VolumeTest& volume)
//
// Inside the function, prefer using `const Vector3 gizmo_origin = instance.world_position();`
// and the helper functions: `CustomGizmos_GetClassname`, `CustomGizmos_IsClass`,
// `CustomGizmos_ForEachTarget` etc. The convenience drawing macros still exist.

#define CUSTOM_GIZMO_IF_CLASS(name)                                    \
    if ( CustomGizmos_IsClass( entity, (name) ) )

#define CUSTOM_GIZMO_ELSE_IF_CLASS(name)                               \
    else if ( CustomGizmos_IsClass( entity, (name) ) )

// ─────────────────────────────────────────────────────────────
// TARGET LOOP HELPERS
// ─────────────────────────────────────────────────────────────

#define CUSTOM_GIZMO_FOR_TARGETS(propName)                             \
    {                                                                   \
        CustomGizmos_ForEachTarget( entity, (propName), [&]( Targetable* __t ) { \
            if ( __t ) {

#define CUSTOM_GIZMO_END_FOR_TARGETS } ); }

// ─────────────────────────────────────────────────────────────
// DRAWING HELPERS
// ─────────────────────────────────────────────────────────────

#define CUSTOM_GIZMO_ARROW_TO_TARGET(origin, target, colour)           \
    CustomGizmos_DrawArrowToTarget(renderer, (origin), (target), volume, (colour))

#define CUSTOM_GIZMO_SPHERE(origin, r, col)                            \
    CustomGizmos_DrawSphere(renderer, (origin), r, volume, col)

#define CUSTOM_GIZMO_BOX_FROM_INSTANCE(col)                            \
    CustomGizmos_DrawBoxFromInstance(renderer, instance, volume, col)

#define CUSTOM_GIZMO_BOX_CUSTOM(origin, sizeVec, colour)                         \
    do {                                                                          \
        CustomGizmos_DrawBoxCustomSize(renderer, (origin), (sizeVec), volume, (colour)); \
    } while(0)


#define CUSTOM_GIZMO_COLOUR(r,g,b,a) Colour4b(r,g,b,a)

// Read a Vector3 attribute from the entity's keyvalues. Returns default
// value Vector3(16,16,16) if key is missing or not parseable.
// Note: helper function takes `entity` and `key` and returns Vector3.
// Parse a vector from a given C string; declared here to avoid requiring a
// full EntityKeyValues type in this header.
Vector3 CustomGizmos_ParseVectorParameter( const char* s, const Vector3& fallback = Vector3( 16.0f, 16.0f, 16.0f ) );

// Parse a float entity parameter; returns fallback when missing or unparsable
float CustomGizmos_ParseFloatParameter( const char* s, float fallback = 0.0f );

// Parse a Colour parameter in the form "r g b" or "r g b a" where values
// are either 0..1 floats or 0..255 ints. Values are mapped to Colour4b
Colour4b CustomGizmos_ParseColourParameter( const char* s, const Colour4b& fallback = Colour4b( 255, 255, 255, 255 ) );

// Convenience helpers to make the custom gizmo code less macro-heavy and more
// explicit. These helper functions are implemented in the .cpp file and are
// intended to be used by custom gizmo authors who prefer explicit code in
// place of the macros above.

// Return the classname string (entity.getKeyValue("classname"))
const char* CustomGizmos_GetClassname( const EntityKeyValues& entity );

// Return whether the provided entity matches classname `name` (handles nullptr)
bool CustomGizmos_IsClass( const EntityKeyValues& entity, const char* name );

// For each target in the property `propName` call the callback with the
// Targetable*; the callback will be invoked for each targetable registered for
// the property. Implemented in the cpp using getTargetables.
void CustomGizmos_ForEachTarget( const EntityKeyValues& entity, const char* propName, const std::function<void(Targetable*)>& fn );

// Arrow helper that takes a Targetable* (as returned by the targetables list).
void CustomGizmos_DrawArrowToTarget( Renderer& renderer, const Vector3& origin, Targetable* target, const VolumeTest& volume, const Colour4b& colour );


// Draw a custom sized box at the given origin using the 3 component size vector
void CustomGizmos_DrawBoxCustomSize( Renderer& renderer, const Vector3& origin, const Vector3& sizeVec, const VolumeTest& volume, const Colour4b& colour );

// Macro wraps the parsing helper and calls entity.getKeyValue at use site; this keeps
// the header free of any direct EntityKeyValues method calls in inline bodies.
#define ENTITY_PARAMETER_VECTOR(key) ( CustomGizmos_ParseVectorParameter( (entity).getKeyValue((key)) ) )
#define ENTITY_PARAMETER_FLOAT(key) ( CustomGizmos_ParseFloatParameter( (entity).getKeyValue((key)) ) )
#define ENTITY_PARAMETER_COLOUR(key) ( CustomGizmos_ParseColourParameter( (entity).getKeyValue((key)) ) )

// ------------------------------------------------------------------
// Explicit helper usage (recommended if you prefer clear code over macros)
// ------------------------------------------------------------------
// Example:
// const Vector3 gizmo_origin = instance.world_position();
// const char* classname = CustomGizmos_GetClassname(entity);
// if ( CustomGizmos_IsClass(entity, "example") ) {
//     CustomGizmos_ForEachTarget(entity, "another", [&]( Targetable* t ){
//         if ( t ) {
//             CustomGizmos_DrawArrowToTarget(renderer, gizmo_origin, t, volume, CUSTOM_GIZMO_COLOUR(255,0,0,255));
//         }
//     });
// }
