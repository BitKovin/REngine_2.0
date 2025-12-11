#pragma once

#include "renderable.h"
#include "render.h"
#include "math/vector.h"
#include "math/aabb.h"
#include <set>
#include <map>

// Forward declarations to avoid circular includes
class TargetableInstance;
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
void CustomGizmos_DrawArrow( Renderer& renderer, const Vector3& start, const Vector3& end, const VolumeTest& volume );
void CustomGizmos_DrawBox( Renderer& renderer, const AABB& aabb, const VolumeTest& volume );
void CustomGizmos_DrawSphere( Renderer& renderer, const Vector3& origin, float radius, const VolumeTest& volume );

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
