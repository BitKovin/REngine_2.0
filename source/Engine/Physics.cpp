#include "Physics.h"

#include "Entity.h"

TempAllocatorImpl* Physics::tempMemAllocator = nullptr;

JobSystemThreadPool* Physics::threadPool = nullptr;

BPLayerInterfaceImpl* Physics::broad_phase_layer_interface = nullptr;

ObjectVsBroadPhaseLayerFilterImpl* Physics::object_vs_broadphase_layer_filter = nullptr;

ObjectLayerPairFilterImpl* Physics::object_vs_object_layer_filter = nullptr;

PhysicsSystem* Physics::physics_system = nullptr;

MyContactListener* Physics::contact_listener = nullptr;

BodyInterface* Physics::bodyInterface = nullptr;

unordered_map<BodyID, Body*> Physics::bodyIdMap;

vector<Body*> Physics::existingBodies = vector<Body*>();

std::unordered_map<std::string, uint64_t> Physics::SurfaceIds = std::unordered_map<std::string, uint64_t>();
std::unordered_map<uint64_t, std::string> Physics::SurfaceNames = std::unordered_map<uint64_t, std::string>();
uint64_t Physics::nextSurfaceId = 1;

recursive_mutex Physics::physicsMainLock = recursive_mutex();

bool Physics::DebugDraw = false;
#ifdef JPH_DEBUG_RENDERER
MyDebugRenderer* Physics::debugRenderer = nullptr;
#endif
BodyType Physics::DebugDrawMask = BodyType::GroupAll & ~BodyType::CharacterCapsule;

std::vector<Physics::PendingBodyEnterPair> Physics::gRemovals;
std::vector<Physics::PendingBodyEnterPair> Physics::gAdds;

void MyContactListener::OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{

	if (inBody1.IsSensor() == false && inBody2.IsSensor() == false) return;

	auto data1 = Physics::GetBodyData(&inBody1);
	auto data2 = Physics::GetBodyData(&inBody2);

	Entity* entity1 = data1->OwnerEntity;
	Entity* entity2 = data2->OwnerEntity;

	if (entity1 == nullptr || entity2 == nullptr) return;

	Physics::PendingBodyEnterPair pair1;
	pair1.target = entity2;
	pair1.entity = entity1;

	Physics::PendingBodyEnterPair pair2;
	pair2.target = entity1;
	pair2.entity = entity2;


	Physics::physicsMainLock.lock();
	Physics::gAdds.push_back(pair1);
	Physics::gAdds.push_back(pair2);
	Physics::physicsMainLock.unlock();

}

void MyContactListener::OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{

}

void MyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
    
	const Body* body1 = Physics::GetBodyFromId(inSubShapePair.GetBody1ID());
	const Body* body2 = Physics::GetBodyFromId(inSubShapePair.GetBody2ID());

	if (body1 == nullptr || body2 == nullptr) return;

	if (body1->IsSensor() == false && body2->IsSensor() == false) return;

	auto data1 = Physics::GetBodyData(body1);
	auto data2 = Physics::GetBodyData(body2);
	Entity* e1 = data1->OwnerEntity, * e2 = data2->OwnerEntity;
	if (!e1 || !e2) return;

	Physics::PendingBodyEnterPair pair1;
	pair1.target = e2;
	pair1.entity = e1;

	Physics::PendingBodyEnterPair pair2;
	pair2.target = e1;
	pair2.entity = e2;


	Physics::physicsMainLock.lock();
	Physics::gRemovals.push_back(pair1);
	Physics::gRemovals.push_back(pair2);
	Physics::physicsMainLock.unlock();

}

void Physics::UpdatePendingBodyExitsEnters()
{
	for (auto& pair : gRemovals)
	{
		pair.target->OnBodyExited(pair.entity->LeadBody, pair.entity);	
	}
	gRemovals.clear();

	for (auto& pair : gAdds)
	{
		pair.target->OnBodyEntered(pair.entity->LeadBody, pair.entity);
	}
	gAdds.clear();

}

uint64_t Physics::FindSurfaceId(string surfaceName)
{
	Physics::physicsMainLock.lock();

	auto result = SurfaceIds.find(surfaceName);

	if (result != SurfaceIds.end())
	{
		Physics::physicsMainLock.unlock();
		return result->second;
	}

	int newId = nextSurfaceId;
	nextSurfaceId++;
	SurfaceIds[surfaceName] = newId;
	SurfaceNames[newId] = surfaceName;
	Physics::physicsMainLock.unlock();

	return newId;
}

string Physics::FindSurfacyById(uint64_t id)
{
	std::lock_guard<std::recursive_mutex> guard(Physics::physicsMainLock);

	auto it = SurfaceNames.find(id);
	if (it != SurfaceNames.end())
		return it->second;

	// Not found — choose what makes sense: empty, error, or throw
	return std::string();  // empty = “unknown”
}

RefConst<Shape> Physics::CreateMeshShape(const std::vector<vec3>& vertices, const std::vector<uint32_t>& indices,string surfaceType)
{
	// Validate that the number of indices is a multiple of 3 (required for triangles)
	if (indices.size() % 3 != 0)
	{
		printf("Error: Number of indices (%zu) must be a multiple of 3 to form complete triangles.\n", indices.size());
		return RefConst<Shape>(); // Return empty shape on error
	}

	// Convert vertices to Jolt's Float3 format
	Array<Float3> joltVertices;
	joltVertices.reserve(vertices.size()); // Reserve space to avoid reallocations
	for (const auto& v : vertices)
	{
		joltVertices.push_back(Float3(v.x, v.y, v.z));
	}

	// Convert indices to Jolt's IndexedTriangle format
	Array<IndexedTriangle> joltTriangles;
	joltTriangles.reserve(indices.size() / 3); // Reserve space for the number of triangles
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		IndexedTriangle tri;
		tri.mIdx[0] = indices[i + 0];     // First vertex index of the triangle
		tri.mIdx[1] = indices[i + 1]; // Second vertex index
		tri.mIdx[2] = indices[i + 2]; // Third vertex index
		joltTriangles.push_back(tri);
	}

	// Create MeshShapeSettings with the converted data
	MeshShapeSettings shapeSettings(joltVertices, joltTriangles);
	shapeSettings.mUserData = FindSurfaceId(surfaceType);


	// Attempt to create the shape
	Shape::ShapeResult result = shapeSettings.Create();
	if (result.HasError())
	{
		printf("Error creating mesh shape: %s\n", result.GetError().c_str());
		return RefConst<Shape>(); // Return empty shape on error
	}


	// Return the successfully created shape
	return result.Get();
}

Physics::HitResult Physics::LineTrace(const vec3 start, const vec3 end, const BodyType mask, const vector<Body*> ignoreList)
{
	HitResult hit;
	hit.fraction = 1.0f;
	hit.position = end;
	hit.normal = vec3(0, 0, 0);
	hit.hitbody = nullptr;
	hit.hasHit = false;
	hit.shapePosition = end;
	hit.entity = nullptr;
	hit.hitboxName = "";
	hit.surfaceName = "";

	// Convert start and end from your own vector type to Jolt's coordinate system.
	JPH::Vec3 startLoc = ToPhysics(start);
	JPH::Vec3 endLoc = ToPhysics(end);

	// Set up the ray from startLoc to endLoc.
	JPH::RRayCast ray;
	ray.mOrigin = startLoc;
	JPH::Vec3 ray_dir = endLoc - startLoc;
	ray.mDirection = ray_dir;

	// Prepare a result structure for the ray cast.
	JPH::RayCastResult result;

	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;

	//physicsMainLock.lock();

	// Cast the ray using the narrow phase query.
	bool hasHit = physics_system->GetNarrowPhaseQuery().CastRay(ray, result, {}, {}, filter);

	if (hasHit)
	{


		// Lock the body using the BodyLockInterface to safely access it.
		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), result.mBodyID);

		// Retrieve the hit body's surface normal.
		// The method GetWorldSpaceSurfaceNormal requires the sub-shape ID and the hit position.
		const JPH::Body* body = &body_lock.GetBody();

		if (body)
		{
			const JPH::Shape* root_shape = body->GetShape();
			JPH::SubShapeID remainder;
			const JPH::Shape* hit_shape = root_shape->GetLeafShape(result.mSubShapeID2, remainder);

			if (hit_shape)
			{
				int hitSurfaceId = hit_shape->GetUserData();

				if (hitSurfaceId)
				{
					hit.surfaceName = FindSurfacyById(hitSurfaceId);
				}

			}

			// Calculate fraction of the hit along the ray.
			hit.fraction = result.mFraction;

			hit.shapePosition = mix(start, end, hit.fraction);

			// Compute the hit position in physics space and convert it back to your coordinate system.
			hit.position = FromPhysics(ray.GetPointOnRay(result.mFraction));


			hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(result.mSubShapeID2, ray.GetPointOnRay(result.mFraction)));


			// Record the hit body and shape.
			hit.hitbody = body;

			auto* props = reinterpret_cast<BodyData*>(body->GetUserData());

			hit.entity = props->OwnerEntity;
			hit.hitboxName = props->hitboxName;
		}
	}




	hit.hasHit = hasHit && hit.entity != nullptr;

	if (hit.hasHit)
	{
		if (hit.entity->Destroyed)
			hit.hasHit = false;

	}

	return hit;
}

Physics::HitResult Physics::SphereTrace(const vec3 start, const vec3 end, float radius, const BodyType mask, const vector<Body*> ignoreList)
{
	HitResult hit;


	hit.fraction = 1.0f;
	hit.position = end;
	hit.normal = vec3(0, 0, 0);
	hit.hitbody = nullptr;
	hit.hasHit = false;
	hit.shapePosition = end;
	hit.entity = nullptr;

	// Convert start and end from your own vector type to Jolt's coordinate system.
	JPH::Vec3 startLoc = ToPhysics(start);
	JPH::Vec3 endLoc = ToPhysics(end);

	// Create a sphere shape for the trace.
	auto sphere_shape_settings = new JPH::SphereShapeSettings();
	sphere_shape_settings->SetEmbedded();
	sphere_shape_settings->mRadius = radius;
	JPH::Shape::ShapeResult shape_result = sphere_shape_settings->Create();
	JPH::ShapeRefC sphere_shape = shape_result.Get();

	if (shape_result.HasError())
	{
		Logger::Log(shape_result.GetError().c_str());
		hit.hasHit = false;
		hit.fraction = 1.0f;
		hit.position = end;
		hit.shapePosition = end;
		hit.normal = vec3(0, 0, 0);
		hit.hitbody = nullptr;
		hit.hitboxName = "";
		delete sphere_shape_settings;
		return hit;
	}

	// Set up the shape cast from startLoc to endLoc.
	JPH::Vec3 direction = endLoc - startLoc;
	JPH::RMat44 start_transform = JPH::RMat44::sTranslation(startLoc);
	JPH::RShapeCast shape_cast(sphere_shape, JPH::Vec3::sReplicate(1.0f), start_transform, direction);



	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;

	//physicsMainLock.lock();

	// Cast the shape using the narrow phase query.
	ClosestHitShapeCastCollector collector;
	physics_system->GetNarrowPhaseQuery().CastShape(shape_cast, JPH::ShapeCastSettings(), JPH::Vec3::sZero(), collector, {}, {}, filter);
	if (collector.HadHit())
	{


		// Lock the body using the BodyLockInterface to safely access it.
		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), collector.GetHit().mBodyID2);

		// Retrieve the hit body's surface normal.
		const JPH::Body* body = &body_lock.GetBody();
		if (body)
		{

			const JPH::Shape* root_shape = body->GetShape();
			JPH::SubShapeID remainder;
			const JPH::Shape* hit_shape = root_shape->GetLeafShape(collector.GetHit().mSubShapeID2, remainder);

			if (hit_shape)
			{
				int hitSurfaceId = hit_shape->GetUserData();

				if (hitSurfaceId)
				{
					hit.surfaceName = FindSurfacyById(hitSurfaceId);
				}

			}

			hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(collector.GetHit().mSubShapeID2, collector.GetHit().mContactPointOn2));

			// Calculate fraction of the hit along the path.
			hit.fraction = collector.GetHit().mFraction;

			hit.shapePosition = mix(start, end, hit.fraction);

			// Compute the hit position in physics space (point on the hit body) and convert it back to your coordinate system.
			hit.position = FromPhysics(collector.GetHit().mContactPointOn2);

			auto* props = reinterpret_cast<BodyData*>(body->GetUserData());

			hit.entity = props->OwnerEntity;

			// Record the hit body.
			hit.hitbody = body;
			hit.hasHit = true;
			hit.hitboxName = props->hitboxName;
		}
	}

	//physicsMainLock.unlock();

	// Clean up the shape settings.
	delete sphere_shape_settings;

	hit.hasHit = hit.hasHit && hit.entity != nullptr;

	if (hit.hasHit)
	{
		if (hit.entity->Destroyed)
			hit.hasHit = false;

	}

	return hit;
}



bool TraceBodyFilter::ShouldCollideLocked(const Body& inBody) const
{

	// Check if the body is in the ignore list.
	for (Body* ignored : ignoreList)
	{
		if (ignored == &inBody)
			return false;
	}

	// Retrieve collision properties from the body's user data.
	// It is assumed that user data points to a CollisionProperties struct.
	auto* properties = reinterpret_cast<BodyData*>(inBody.GetUserData());
	if (properties)
	{
		if (properties->OwnerEntity)
		{
			if (properties->OwnerEntity->Destroyed)
			{
				return false;
			}
		}

		if (inBody.IsSensor() && properties->group != BodyType::Liquid)
			return false;

		// Check if the body's group is included in our filter's mask.
		// If the bitwise AND of mask and the body's group is zero, they don't match.
		if ((static_cast<uint32_t>(mask) & static_cast<uint32_t>(properties->group)) == 0)
			return false;
	}

	// Accept the collision if no condition rejects it.
	return true;
}

uint64_t Physics::GetShapeDataIdFromName(string name)
{
	return 0;
}

Body* Physics::CreateCharacterBody(Entity* owner, vec3 Position, float Radius, float Height, float Mass,
	BodyType group,
	BodyType mask)
{
	// Calculate cylinder portion height (total capsule height = cylinder_height + 2 * radius)
	float cylinder_half_height = (Height - 2.0f * Radius) / 2.0f;
	if (cylinder_half_height < 0.0f) {
		Logger::Log("Capsule height is too small for given radius, using minimum height");
		cylinder_half_height = 0.0f;
	}

	// Create capsule shape
	auto capsule_shape_settings = new JPH::CapsuleShapeSettings();
	capsule_shape_settings->SetEmbedded();
	capsule_shape_settings->mRadius = Radius;
	capsule_shape_settings->mHalfHeightOfCylinder = cylinder_half_height;

	JPH::Shape::ShapeResult shape_result = capsule_shape_settings->Create();
	JPH::Shape* capsule_shape = shape_result.Get();

	if (shape_result.HasError())
		Logger::Log(shape_result.GetError().c_str());

	// Configure body settings
	JPH::BodyCreationSettings body_settings(
		capsule_shape,
		ToPhysics(Position),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Dynamic,  // Dynamic body type
		Layers::MOVING             // Use moving layer
	);

	body_settings.mMotionQuality = EMotionQuality::LinearCast;

	// Lock rotation in all axes
	body_settings.mAllowedDOFs = JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ;  // Allow only translation

	// Set mass properties
	body_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
	body_settings.mMassPropertiesOverride.mMass = Mass;
	body_settings.mFriction = 0.0f;  // Match box friction
	body_settings.mRestitution = 0.0f; // No bounciness

	// Allocate and attach collision properties to the body via the user data field:
	BodyData* properties = new BodyData{ group, mask, owner };
	body_settings.mUserData = reinterpret_cast<uintptr_t>(properties);

	// Create and add body to world
	JPH::Body* character_body = bodyInterface->CreateBody(body_settings);
	
	AddBody(character_body);

	return character_body;
}

Body* Physics::CreateCharacterCylinderBody(Entity* owner, vec3 Position, float Radius, float Height, float Mass,
	BodyType group, BodyType mask)
{
	// Calculate cylinder half height (total height = 2 * half_height)
	float cylinder_half_height = Height * 0.5f;
	if (cylinder_half_height <= 0.0f) {
		Logger::Log("Cylinder height must be positive, using minimal value");
		cylinder_half_height = 0.01f;
	}

	// Create cylinder shape
	auto cylinder_shape_settings = new JPH::CylinderShapeSettings(
		cylinder_half_height,
		Radius
	);
	cylinder_shape_settings->SetEmbedded();

	JPH::Shape::ShapeResult shape_result = cylinder_shape_settings->Create();
	JPH::Shape* cylinder_shape = shape_result.Get();

	if (shape_result.HasError())
		Logger::Log(shape_result.GetError().c_str());

	// Configure body settings (identical to capsule version)
	JPH::BodyCreationSettings body_settings(
		cylinder_shape,
		ToPhysics(Position),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Dynamic,  // Dynamic body type
		Layers::MOVING              // Use moving layer
	);

	body_settings.mMotionQuality = EMotionQuality::LinearCast;
	body_settings.mAllowedDOFs = JPH::EAllowedDOFs::TranslationX |
		JPH::EAllowedDOFs::TranslationY |
		JPH::EAllowedDOFs::TranslationZ;  // Lock rotation
	body_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
	body_settings.mMassPropertiesOverride.mMass = Mass;
	body_settings.mFriction = 0.0f;
	body_settings.mRestitution = 0.0f;

	// Attach collision properties
	BodyData* properties = new BodyData{ group, mask, owner };
	body_settings.mUserData = reinterpret_cast<uintptr_t>(properties);

	// Create and add body to world
	JPH::Body* cylinder_body = bodyInterface->CreateBody(body_settings);
	AddBody(cylinder_body);

	return cylinder_body;
}