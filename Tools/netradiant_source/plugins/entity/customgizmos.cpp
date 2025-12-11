#include "customgizmos.h"
#include "entity.h"
#include "targetable.h"
#include "renderable.h"
#include "math/aabb.h"
#include "math/vector.h"
#include <cmath>
#include <array>
#include <functional>
#include <cstring>
// For string parsing helpers
#include "stringio.h"

// Current draw data pointer used by the helper functions when invoked from the
// RenderableCustomGizmos::renderSolid function. This points into the
// per-instance DrawData kept by RenderableCustomGizmos and ensures the
// lifetime of OpenGLRenderable objects across the rendering pipeline.
static thread_local RenderableCustomGizmos::GizmoDrawData* g_current_gizmodata = nullptr;

// helper for parsing vector values
Vector3 CustomGizmos_ParseVectorParameter( const char* s, const Vector3& fallback ){
    Vector3 v;
    if ( s != nullptr && string_parse_vector3( s, v ) ){
        return v;
    }
    return fallback;
}

float CustomGizmos_ParseFloatParameter( const char* s, float fallback ){
    if ( s != nullptr ){
        float v;
        if ( string_parse_float( s, v ) ){
            return v;
        }
    }
    return fallback;
}

Colour4b CustomGizmos_ParseColourParameter( const char* s, const Colour4b& fallback ){
    if ( s == nullptr ) return fallback;
    // Try to parse 3 or 4 floats/ints
    Vector3 v;
    auto clamp255 = []( int x ){ if ( x < 0 ) return 0; if ( x > 255 ) return 255; return x; };
    if ( string_parse_vector3( s, v ) ){
        // check if values are in 0..1 range, convert to 0..255
        bool plusMinusOkay = true;
        for ( int i = 0; i < 3; ++i ){
            if ( v[i] < 0.0f || v[i] > 255.0f ){
                plusMinusOkay = false;
                break;
            }
        }
        // Determine if they are 0..1 floats: assume if values <= 1.0
        bool isFloat0to1 = v[0] <= 1.0f && v[1] <= 1.0f && v[2] <= 1.0f;
        int r, g, b, a;
        if ( isFloat0to1 ){
            r = clamp255( static_cast<int>( v[0] * 255.0f ) );
            g = clamp255( static_cast<int>( v[1] * 255.0f ) );
            b = clamp255( static_cast<int>( v[2] * 255.0f ) );
            a = 255;
        }
        else {
            r = clamp255( static_cast<int>( v[0] ) );
            g = clamp255( static_cast<int>( v[1] ) );
            b = clamp255( static_cast<int>( v[2] ) );
            a = 255;
        }
        // see if there is an alpha value after the vector; look for a fourth component
        // we'll use a quick parse where possible
        const char* rest = s;
        // skip first three numbers
        for ( int i = 0; i < 3; ++i ){
            buffer_parse_floating_literal( rest );
            if ( *rest == ' ' ) ++rest;
        }
        if ( *rest != '\0' ){
            float alphaf = buffer_parse_floating_literal( rest );
            if ( alphaf <= 1.0f ){
                a = clamp255( static_cast<int>( alphaf * 255.0f ) );
            }
            else {
                a = clamp255( static_cast<int>( alphaf ) );
            }
        }
        return Colour4b( r, g, b, a );
    }
    // not a three component vector; try parsing as three ints using string_parse_int
    {
        // As a fallback, try to parse three integers
        const char* p = s;
        int r, g, b;
        if ( string_parse_int( p, r ) ){
            // consume separator if present
            if ( *p == ' ' ) ++p;
            if ( string_parse_int( p, g ) ){
                if ( *p == ' ' ) ++p;
                if ( string_parse_int( p, b ) ){
                    return Colour4b( clamp255(r), clamp255(g), clamp255(b), 255 );
                }
            }
        }
    }
    // fallback
    return fallback;
}

// --- Helpers to make explicit the functionality previously provided by macros ---
const char* CustomGizmos_GetClassname( const EntityKeyValues& entity ){
    return entity.getKeyValue("classname");
}

bool CustomGizmos_IsClass( const EntityKeyValues& entity, const char* name ){
    const char* cls = CustomGizmos_GetClassname( entity );
    return ( cls != nullptr && strcmp( cls, name ) == 0 );
}

void CustomGizmos_ForEachTarget( const EntityKeyValues& entity, const char* propName, const std::function<void(Targetable*)>& fn ){
    const char* prop = entity.getKeyValue( propName );
    if ( prop == nullptr || string_empty( prop ) ) return;
    targetables_t* targets = getTargetables( prop );
    if ( targets == nullptr ) return;
    for ( auto* t : *targets ){
        fn( t );
    }
}

void CustomGizmos_DrawArrowToTarget( Renderer& renderer, const Vector3& origin, Targetable* target, const VolumeTest& volume, const Colour4b& colour ){
    if ( target == nullptr ) return;
    CustomGizmos_DrawArrow( renderer, origin, target->world_position(), volume, colour );
}


void CustomGizmos_DrawBoxCustomSize( Renderer& renderer, const Vector3& origin, const Vector3& sizeVec, const VolumeTest& volume, const Colour4b& colour ){
    const Vector3 half = Vector3( sizeVec[0] * 0.5f, sizeVec[1] * 0.5f, sizeVec[2] * 0.5f );
    AABB box( vector3_subtracted(origin, half), vector3_added(origin, half) );
    CustomGizmos_DrawBox( renderer, box, volume, colour );
}


bool CustomGizmos_DrawIfMatch( const EntityKeyValues& entity, const TargetableInstance& instance, Renderer& renderer, const VolumeTest& volume ){
    // Explicit style example (recommended for readability)
    const Vector3 gizmo_origin = instance.world_position();
    if ( CustomGizmos_IsClass( entity, "test" ) ){
        // iterate targets and draw arrows to them
            CustomGizmos_ForEachTarget( entity, "another", [&]( Targetable* t ){
                if ( t ){
                    CUSTOM_GIZMO_ARROW_TO_TARGET( gizmo_origin, t, CUSTOM_GIZMO_COLOUR(255,0,0,255) );
                }
            } );
        return true;
    }

    if ( CustomGizmos_IsClass( entity, "example" ) ){
        // box size can now be provided via entity key-values, e.g. "box size" = "32 32 64"
        const Vector3 boxSize = ENTITY_PARAMETER_VECTOR("box_size");
            CUSTOM_GIZMO_BOX_CUSTOM( gizmo_origin, boxSize, ENTITY_PARAMETER_COLOUR("box_colour") );

        // Optional radius for a sphere
        float r = ENTITY_PARAMETER_FLOAT("radius");
        if ( r > 0.0f ){
                CUSTOM_GIZMO_SPHERE( gizmo_origin, r, ENTITY_PARAMETER_COLOUR("sphere_colour") );
        }
        return true;
    }

    // Fallback: no gizmo drawn
    return false;
}


void CustomGizmos_DrawArrow( Renderer& renderer, const Vector3& start, const Vector3& end, const VolumeTest& volume, const Colour4b& colour ){
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
    linesPtr->push_back( PointVertex( vertex3f_for_vector3( start ), colour ) );
    linesPtr->push_back( PointVertex( vertex3f_for_vector3( end ), colour ) );

    // Arrowhead wings
    // choose an arbitrary vector not parallel to direction
    Vector3 up( 0.0f, 0.0f, 1.0f );
    if ( fabs( vector3_dot( ndir, up ) ) > 0.9f ) up = Vector3( 0.0f, 1.0f, 0.0f );
    Vector3 ort = vector3_normalised( vector3_cross( ndir, up ) );
    Vector3 wing1 = vector3_subtracted( end, vector3_scaled( ndir, 12.0f ) );
    Vector3 wing2 = vector3_added( wing1, vector3_scaled( ort, 6.0f ) );
    Vector3 wing3 = vector3_subtracted( wing1, vector3_scaled( ort, 6.0f ) );

    linesPtr->push_back( PointVertex( vertex3f_for_vector3( end ), colour ) );
    linesPtr->push_back( PointVertex( vertex3f_for_vector3( wing2 ), colour ) );
    linesPtr->push_back( PointVertex( vertex3f_for_vector3( end ), colour ) );
    linesPtr->push_back( PointVertex( vertex3f_for_vector3( wing3 ), colour ) );

    // Add to renderer
    if ( !usingStorage ){
        renderer.addRenderable( *linesPtr, g_matrix4_identity );
    }
}

void CustomGizmos_DrawBox( Renderer& renderer, const AABB& aabb, const VolumeTest& volume, const Colour4b& colour ){
    // AABB expected as (center, half-extents)
    const Vector3 center = aabb.origin;
    const Vector3 half = aabb.extents; // "extents" should be half-size

    // compute min / max explicitly
    const Vector3 mn = vector3_subtracted( center, half );
    const Vector3 mx = vector3_added( center, half );

    // Build corners in a consistent order:
    // 0: (min.x, min.y, min.z)
    // 1: (max.x, min.y, min.z)
    // 2: (max.x, max.y, min.z)
    // 3: (min.x, max.y, min.z)
    // 4: (min.x, min.y, max.z)
    // 5: (max.x, min.y, max.z)
    // 6: (max.x, max.y, max.z)
    // 7: (min.x, max.y, max.z)
    std::array<Vector3,8> points;
    points[0] = Vector3( mn[0], mn[1], mn[2] );
    points[1] = Vector3( mx[0], mn[1], mn[2] );
    points[2] = Vector3( mx[0], mx[1], mn[2] );
    points[3] = Vector3( mn[0], mx[1], mn[2] );
    points[4] = Vector3( mn[0], mn[1], mx[2] );
    points[5] = Vector3( mx[0], mn[1], mx[2] );
    points[6] = Vector3( mx[0], mx[1], mx[2] );
    points[7] = Vector3( mn[0], mx[1], mx[2] );

    // edges defined using the corner indices above
    const int edges[12][2] = {
        // bottom face (z = min)
        {0,1}, {1,2}, {2,3}, {3,0},
        // top face (z = max)
        {4,5}, {5,6}, {6,7}, {7,4},
        // vertical edges
        {0,4}, {1,5}, {2,6}, {3,7}
    };

    // Optional: quick volume test if you have a method for AABB; else skip.
    // If VolumeTest has TestAABB or similar you can early-out here.
    // e.g. if (!volume.TestAABB(aabb)) return;

    bool usingStorage = ( g_current_gizmodata != nullptr );
    RenderablePointVector* linesPtr = nullptr;
    RenderablePointVector localLines( GL_LINES );
    if ( usingStorage ){
        linesPtr = &( g_current_gizmodata->m_lines );
    }
    else{
        linesPtr = &localLines;
    }

    for ( int i = 0; i < 12; ++i ){
        const Vector3 &a = points[ edges[i][0] ];
        const Vector3 &b = points[ edges[i][1] ];
        linesPtr->push_back( PointVertex( vertex3f_for_vector3( a ), colour ) );
        linesPtr->push_back( PointVertex( vertex3f_for_vector3( b ), colour ) );
    }

    if ( !usingStorage ){
        renderer.addRenderable( *linesPtr, g_matrix4_identity );
    }
}


void CustomGizmos_DrawSphere( Renderer& renderer, const Vector3& origin, float radius, const VolumeTest& volume, const Colour4b& colour ){
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

    for ( int i = 0; i < segments; ++i ){
        double a = i * step;

        // Precompute cos/sin
        const float ca = static_cast<float>( cos(a) );
        const float sa = static_cast<float>( sin(a) );

        // X-Y plane (Z constant)
        {
            Vector3 p( origin[0] + radius * ca,
                       origin[1] + radius * sa,
                       origin[2] );
            loopZPtr->push_back( PointVertex( vertex3f_for_vector3( p ), colour ) );
        }

        // X-Z plane (Y constant)
        {
            Vector3 p( origin[0] + radius * ca,
                       origin[1],
                       origin[2] + radius * sa );
            loopYPtr->push_back( PointVertex( vertex3f_for_vector3( p ), colour ) );
        }

        // Y-Z plane (X constant)
        {
            Vector3 p( origin[0],
                       origin[1] + radius * ca,
                       origin[2] + radius * sa );
            loopXPtr->push_back( PointVertex( vertex3f_for_vector3( p ), colour ) );
        }
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
