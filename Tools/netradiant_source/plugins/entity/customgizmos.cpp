#include "customgizmos.h"
#include "entity.h"
#include "targetable.h"
#include "renderable.h"
#include "math/aabb.h"
#include "math/vector.h"
#include <cmath>
#include <array>

// Current draw data pointer used by the helper functions when invoked from the
// RenderableCustomGizmos::renderSolid function. This points into the
// per-instance DrawData kept by RenderableCustomGizmos and ensures the
// lifetime of OpenGLRenderable objects across the rendering pipeline.
static thread_local RenderableCustomGizmos::GizmoDrawData* g_current_gizmodata = nullptr;

// Default implementation of the editable function. Modify as desired.
bool CustomGizmos_DrawIfMatch( const EntityKeyValues& entity, const TargetableInstance& instance, Renderer& renderer, const VolumeTest& volume ){
    // Check name (or use whichever property you like)
    const char* name = entity.getKeyValue( "targetname" );
    if ( name != 0 && string_equal( name, "test" ) ){
        // Resolve the "target" property and find matching targetables
        const char* targetName = entity.getKeyValue( "another" );
        if ( !string_empty( targetName ) ){
            targetables_t* targets = getTargetables( targetName );
            if ( targets != nullptr ){
                // Draw an arrow from this entity to every matching target
                const Vector3 start = instance.world_position();
                for ( targetables_t::iterator it = targets->begin(); it != targets->end(); ++it ){
                    const Targetable* t = *it;
                    const Vector3 end = t->world_position();

                    // use helper to draw arrow (helper already added in the project)
                    // The helper will attempt to append geometry into the current
                    // per-instance draw storage if available; otherwise it will
                    // fall back to directly calling renderer.addRenderable.
                    CustomGizmos_DrawArrow( renderer, start, end, volume );
                }
            }
        }
        return true; // we drew custom gizmo(s)
    }

    return false; // nothing drawn for this entity
}

void CustomGizmos_DrawArrow( Renderer& renderer, const Vector3& start, const Vector3& end, const VolumeTest& volume ){
    if ( !volume.TestLine( segment_for_startend( start, end ) ) ){
        return;
    }
    bool usingStorage = ( g_current_gizmodata != nullptr );
    RenderablePointVector* linesPtr = nullptr;
    RenderablePointVector localLines( GL_LINES );
    if ( usingStorage ){
        linesPtr = &( g_current_gizmodata->m_lines );
    }
    else{
        linesPtr = &localLines;
    }
    const Vector3 dir = vector3_subtracted( end, start );
    const double len = vector3_length( dir );
    if ( len == 0 ) return;
    Vector3 ndir = dir; vector3_normalise( ndir );
    // main line
    linesPtr->push_back( PointVertex( vertex3f_for_vector3( start ) ) );
    linesPtr->push_back( PointVertex( vertex3f_for_vector3( end ) ) );

    // Arrowhead wings
    // choose an arbitrary vector not parallel to direction
    Vector3 up( 0.0f, 0.0f, 1.0f );
    if ( fabs( vector3_dot( ndir, up ) ) > 0.9f ) up = Vector3( 0.0f, 1.0f, 0.0f );
    Vector3 ort = vector3_normalised( vector3_cross( ndir, up ) );
    Vector3 wing1 = vector3_subtracted( end, vector3_scaled( ndir, 12.0f ) );
    Vector3 wing2 = vector3_added( wing1, vector3_scaled( ort, 6.0f ) );
    Vector3 wing3 = vector3_subtracted( wing1, vector3_scaled( ort, 6.0f ) );

    linesPtr->push_back( PointVertex( vertex3f_for_vector3( end ) ) );
    linesPtr->push_back( PointVertex( vertex3f_for_vector3( wing2 ) ) );
    linesPtr->push_back( PointVertex( vertex3f_for_vector3( end ) ) );
    linesPtr->push_back( PointVertex( vertex3f_for_vector3( wing3 ) ) );

    // Add to renderer
    if ( !usingStorage ){
        renderer.addRenderable( *linesPtr, g_matrix4_identity );
    }
}

void CustomGizmos_DrawBox( Renderer& renderer, const AABB& aabb, const VolumeTest& volume ){
    // AABB lines
    bool usingStorage = ( g_current_gizmodata != nullptr );
    RenderablePointVector* linesPtr = nullptr;
    RenderablePointVector localLines( GL_LINES );
    if ( usingStorage ){
        linesPtr = &( g_current_gizmodata->m_lines );
    }
    else{
        linesPtr = &localLines;
    }
    std::array<Vector3, 8> points = aabb_corners( aabb );
    // edges of a cube
    const int edges[24][2] = {
        {0,1},{1,3},{3,2},{2,0},
        {4,5},{5,7},{7,6},{6,4},
        {0,4},{1,5},{2,6},{3,7}
    };
    for ( int i=0;i<12;i++ ){
        linesPtr->push_back( PointVertex( vertex3f_for_vector3( points[ edges[i][0] ] ) ) );
        linesPtr->push_back( PointVertex( vertex3f_for_vector3( points[ edges[i][1] ] ) ) );
    }
    if ( !usingStorage ){
        renderer.addRenderable( *linesPtr, g_matrix4_identity );
    }
}

void CustomGizmos_DrawSphere( Renderer& renderer, const Vector3& origin, float radius, const VolumeTest& volume ){
    if ( radius <= 0 ) return;
    const int segments = 18;
    const double step = 2.0 * c_pi / segments;

    bool usingStorage = ( g_current_gizmodata != nullptr );
    RenderablePointVector* loopXPtr = nullptr;
    RenderablePointVector* loopYPtr = nullptr;
    RenderablePointVector* loopZPtr = nullptr;
    RenderablePointVector localX( GL_LINE_LOOP );
    RenderablePointVector localY( GL_LINE_LOOP );
    RenderablePointVector localZ( GL_LINE_LOOP );
    if ( usingStorage ){
        loopXPtr = &( g_current_gizmodata->m_loopX );
        loopYPtr = &( g_current_gizmodata->m_loopY );
        loopZPtr = &( g_current_gizmodata->m_loopZ );
    }
    else{
        loopXPtr = &localX; loopYPtr = &localY; loopZPtr = &localZ;
    }
    for ( int i=0; i<segments; ++i ){
        double a = i * step;
        // X-Y plane
        loopZPtr->push_back( PointVertex( vertex3f_for_vector3( Vector3( origin[0] + radius * static_cast<float>( cos( a ) ), origin[1] + radius * static_cast<float>( sin( a ) ), origin[2] ) ) ) );
        // X-Z plane
        loopYPtr->push_back( PointVertex( vertex3f_for_vector3( Vector3( origin[0] + radius * static_cast<float>( cos( a ) ), origin[1], origin[2] + radius * static_cast<float>( sin( a ) ) ) ) ) );
        // Y-Z plane
        loopXPtr->push_back( PointVertex( vertex3f_for_vector3( Vector3( origin[0], origin[1] + radius * static_cast<float>( cos( a ) ), origin[2] + radius * static_cast<float>( sin( a ) ) ) ) ) );
    }
    if ( !usingStorage ){
        renderer.addRenderable( *loopXPtr, g_matrix4_identity );
        renderer.addRenderable( *loopYPtr, g_matrix4_identity );
        renderer.addRenderable( *loopZPtr, g_matrix4_identity );
    }
}

// Renderable implementation
void RenderableCustomGizmos::attach( TargetableInstance& instance ){
    const bool inserted = m_instances.insert( &instance ).second;
    ASSERT_MESSAGE( inserted, "cannot attach instance" );
}
void RenderableCustomGizmos::detach( TargetableInstance& instance ){
    const bool erased = m_instances.erase( &instance );
    ASSERT_MESSAGE( erased, "cannot detach instance" );
    // clean up any stored draw data for this instance
    m_drawData.erase( &instance );
}

void RenderableCustomGizmos::renderSolid( Renderer& renderer, const VolumeTest& volume ) const{
    for ( TargetableInstances::const_iterator i = m_instances.begin(); i != m_instances.end(); ++i ){
        TargetableInstance* inst = *i;
        if ( inst->path().top().get().visible() ){
            // prepare per-instance draw buffers
            RenderableCustomGizmos::GizmoDrawData& data = m_drawData[ inst ];
            data.clear();
            // set the current draw storage used by the helper functions
            RenderableCustomGizmos::GizmoDrawData* prev = g_current_gizmodata;
            g_current_gizmodata = &data;
            // call the customizable draw function
            CustomGizmos_DrawIfMatch( inst->getEntity(), *inst, renderer, volume );
            // reset current storage
            g_current_gizmodata = prev;

            // add any generated renderables to the renderer. Note: renderer.addRenderable
            // will store pointers to the provided OpenGLRenderable, so we must ensure
            // the objects we pass in remain alive until rendering completes. Using
            // the per-instance data in this class satisfies that.
            if ( !data.m_lines.empty() ){
                renderer.addRenderable( data.m_lines, g_matrix4_identity );
            }
            if ( !data.m_loopX.empty() ){
                renderer.addRenderable( data.m_loopX, g_matrix4_identity );
            }
            if ( !data.m_loopY.empty() ){
                renderer.addRenderable( data.m_loopY, g_matrix4_identity );
            }
            if ( !data.m_loopZ.empty() ){
                renderer.addRenderable( data.m_loopZ, g_matrix4_identity );
            }
        }
    }
}

void RenderableCustomGizmos::renderWireframe( Renderer& renderer, const VolumeTest& volume ) const{
    renderSolid( renderer, volume );
}
