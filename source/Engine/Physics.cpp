#include "Physics.h"

#include "Entity.h"

#include <unordered_set>

#include <Jolt/Physics/Constraints/SwingTwistConstraint.h>

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

void MyContactListener::beforeSimulation()
{
	previousContacts = std::move(currentContacts); // Move for efficiency
	currentContacts.clear(); // Reset for the new step
}

void MyContactListener::afterSimulation()
{
	std::vector<Physics::PendingBodyEnterPair> adds;
	std::vector<Physics::PendingBodyEnterPair> removals;



	// Find added contacts: in current but not in previous
	for (const auto& pair : currentContacts)
	{
		if (previousContacts.find(pair) == previousContacts.end())
		{
			Physics::PendingBodyEnterPair p;
			p.entity = pair.first;  // Sensor entity
			p.target = pair.second; // Target entity
			adds.push_back(p);
		}
	}

	// Find removed contacts: in previous but not in current
	for (const auto& pair : previousContacts)
	{
		if (currentContacts.find(pair) == currentContacts.end())
		{
			Physics::PendingBodyEnterPair p;
			p.entity = pair.first;  // Sensor entity
			p.target = pair.second; // Target entity
			removals.push_back(p);
		}
	}

	auto newAdds = Physics::gAdds;
	auto newRemovs = Physics::gRemovals;

	for (const auto& p : adds)
	{
		newAdds.push_back(p);
	}
	for (const auto& p : removals)
	{
		newRemovs.push_back(p);
	}

	Physics::gAdds = newAdds;
	Physics::gRemovals = newRemovs;
	

}

void MyContactListener::OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	auto data1 = Physics::GetBodyData(&inBody1);
	auto data2 = Physics::GetBodyData(&inBody2);
	Entity* entity1 = data1 ? data1->OwnerEntity : nullptr;
	Entity* entity2 = data2 ? data2->OwnerEntity : nullptr;

	if (entity1 == nullptr || entity2 == nullptr) return;

	Physics::physicsMainLock.lock();

	// Add pair if body1 is a sensor
	if (inBody1.IsSensor())
	{
		currentContacts.insert({ entity1, entity2 });
	}
	// Add pair if body2 is a sensor
	if (inBody2.IsSensor())
	{
		currentContacts.insert({ entity2, entity1 });
	}

	Physics::physicsMainLock.unlock();
}

void MyContactListener::OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	auto data1 = Physics::GetBodyData(&inBody1);
	auto data2 = Physics::GetBodyData(&inBody2);
	Entity* entity1 = data1 ? data1->OwnerEntity : nullptr;
	Entity* entity2 = data2 ? data2->OwnerEntity : nullptr;

	if (entity1 == nullptr || entity2 == nullptr) return;

	Physics::physicsMainLock.lock();

	// Add pair if body1 is a sensor
	if (inBody1.IsSensor())
	{
		currentContacts.insert({ entity1, entity2 });
	}
	// Add pair if body2 is a sensor
	if (inBody2.IsSensor())
	{
		currentContacts.insert({ entity2, entity1 });
	}

	Physics::physicsMainLock.unlock();
}

void MyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
	// Leave empty; removals are handled in afterSimulation()
}

void Physics::DestroyBody(Body* body)
{
	if (body == nullptr)
		return;

	physicsMainLock.lock();

	bodyIdMap.erase(body->GetID());

	bodyInterface->RemoveBody(body->GetID());

	bodyInterface->DestroyBody(body->GetID());

	// Retrieve and delete collision properties if present.
	auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
	body->SetUserData(0);

	if (props)
	{
		delete props;
	}

	// Remove body from existingBodies
	auto it = std::find(existingBodies.begin(), existingBodies.end(), body);
	if (it != existingBodies.end())
		existingBodies.erase(it);

	physicsMainLock.unlock();
}

void Physics::DestroyConstraint(Constraint* constraint)
{

	if (constraint == nullptr) return;

	physicsMainLock.lock();

	physics_system->RemoveConstraint(constraint);

	physicsMainLock.unlock();


}

void Physics::SetMotionType(Body* body, JPH::EMotionType type)
{
	bodyInterface->SetMotionType(body->GetID(), type, JPH::EActivation::Activate);
}

void Physics::Init()
{

	RegisterDefaultAllocator();
	Factory::sInstance = new Factory();

	RegisterTypes();

	tempMemAllocator = new TempAllocatorImpl(30 * 1024 * 1024);

	int numThreads = 2;

#ifndef __EMSCRIPTEN_PTHREADS__

	numThreads = ThreadPool::GetNumThreadsForPhysics();

#endif




	threadPool = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, numThreads);

	const uint cMaxBodies = 65536;

	const uint cNumBodyMutexes = 0;

	const uint cMaxBodyPairs = 65536;

	const uint cMaxContactConstraints = 20240;

	object_vs_broadphase_layer_filter = new ObjectVsBroadPhaseLayerFilterImpl();

	object_vs_object_layer_filter = new ObjectLayerPairFilterImpl();

	broad_phase_layer_interface = new BPLayerInterfaceImpl();

	physics_system = new PhysicsSystem();

	physics_system->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, *broad_phase_layer_interface, *object_vs_broadphase_layer_filter, *object_vs_object_layer_filter);


	contact_listener = new MyContactListener();
	physics_system->SetContactListener(contact_listener);

	bodyInterface = &physics_system->GetBodyInterface();

#ifdef JPH_DEBUG_RENDERER
	debugRenderer = new MyDebugRenderer();
#endif

}

void Physics::UpdatePendingBodyExitsEnters()
{

	std::unordered_set<PendingBodyEnterPair> processedAdds;
	std::unordered_set<PendingBodyEnterPair> processedRemovals;

	for (auto& pair : gRemovals)
	{

		auto result = processedRemovals.find(pair);
		if (result != processedRemovals.end()) 
		{
			Logger::Log("found duplicate exit pair");
			continue;
		}

		if (pair.entity == nullptr || pair.target == nullptr || pair.target == nullptr)
			continue;

		//in case if one of them was deleted in previous frame
		if (Level::Current->DeletedLevelObjectAdresses.find(pair.entity) != Level::Current->DeletedLevelObjectAdresses.end())
		{
			continue;
		}
		if (Level::Current->DeletedLevelObjectAdresses.find(pair.target) != Level::Current->DeletedLevelObjectAdresses.end())
		{
			continue;
		}

		//pair.target->OnBodyExited(pair.entity->LeadBody, pair.entity);	
		pair.entity->OnBodyExited(pair.target->LeadBody, pair.target);
		processedRemovals.insert(pair);
	}
	gRemovals.clear();

	for (auto& pair : gAdds)
	{

		auto result = processedAdds.find(pair);
		{
			if (result != processedAdds.end()) 
			{
				Logger::Log("found duplicate enter pair");
				continue;
			}
		}

		if (pair.entity == nullptr || pair.target == nullptr || pair.target == nullptr)
			continue;

		//in case if one of them was deleted in previous frame
		if (Level::Current->DeletedLevelObjectAdresses.find(pair.entity) != Level::Current->DeletedLevelObjectAdresses.end())
		{
			continue;
		}
		if (Level::Current->DeletedLevelObjectAdresses.find(pair.target) != Level::Current->DeletedLevelObjectAdresses.end())
		{
			continue;
		}

		//pair.target->OnBodyEntered(pair.entity->LeadBody, pair.entity);
		pair.entity->OnBodyEntered(pair.target->LeadBody, pair.target);
		processedAdds.insert(pair);
	}
	gAdds.clear();

}

void Physics::DrawConstraint(Constraint* constraint)
{

	if (constraint == nullptr) return;
#ifdef JPH_DEBUG_RENDERER
	constraint->DrawConstraint(debugRenderer);
	constraint->DrawConstraintLimits(debugRenderer);
	constraint->DrawConstraintReferenceFrame(debugRenderer);
#endif

}




Body* Physics::CreateHitBoxBody(Entity* owner, string hitboxName, vec3 PositionOffset, quat RotationOffset, vec3 Size, float Mass, BodyType group, BodyType mask)
{
	
	// 1) Base box, centered at its own origin
	auto box_settings = JPH::BoxShapeSettings();
	box_settings.SetEmbedded();
	box_settings.mHalfExtent = ToPhysics(Size) * 0.5f;

	// 2) Rotate and translate the box in shape-local space
	auto geo_settings = JPH::RotatedTranslatedShapeSettings(
		ToPhysics(PositionOffset),   // translate
		ToPhysics(RotationOffset),   // rotate
		&box_settings                // child shape
	);
	geo_settings.SetEmbedded();

	// 3) Create the final shape directly from geo_settings
	JPH::Shape::ShapeResult sr = geo_settings.Create();
	if (sr.HasError())
		Logger::Log(sr.GetError().c_str());
	JPH::Ref<JPH::Shape> final_shape = sr.Get();

	// 4) Use the entity’s (bone’s) world-space position as the body’s position
	JPH::RVec3 world_com = Vec3(0, 0, 0);// ToPhysics(owner->GetPosition());

	// 5) Build the body
	JPH::BodyCreationSettings bcs(
		final_shape,
		world_com,
		JPH::Quat::sIdentity(),      // shape rotation is baked in
		JPH::EMotionType::Kinematic,
		Layers::MOVING
	);

	bcs.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
	bcs.mMassPropertiesOverride.mMass = Mass;
	bcs.mFriction = 0.9f;

	bcs.mAngularDamping = 10;

	BodyData* props = new BodyData{ group, mask, true, owner, hitboxName };
	bcs.mUserData = reinterpret_cast<uintptr_t>(props);

	// 6) Create and register the body
	JPH::Body* body = bodyInterface->CreateBody(bcs);
	AddBody(body);

	return body;

}

Constraint* Physics::CreateRagdollConstraint(Body* parent,
	Body* child,
	float    twistMinAngle,
	float    twistMaxAngle,
	float    swingHalfConeAngle,
	JPH::QuatArg childSpaceConstraintRotation)
{
	
	twistMinAngle /= 180.0/pi<double>();
	twistMaxAngle /= 180.0 / pi<double>();
	swingHalfConeAngle /= 180.0 / pi<double>();

	SwingTwistConstraintSettings settings;
	settings.mSpace = EConstraintSpace::LocalToBodyCOM;

	// --- 1) grab the child's entity/world position (not its COM)
	//     (assuming GetWorldTransform() returns a JPH::RMat44)
	RMat44  childEntityX = child->GetWorldTransform();
	Vec3    worldPivot = childEntityX.GetTranslation();

	// --- 2) compute child‐space pivot (in child‐COM local coords)
	RMat44  childCOMX = child->GetCenterOfMassTransform();
	RMat44  invChildCOM = childCOMX.Inversed();
	Vec4    local2_v4 = invChildCOM * Vec4(worldPivot, 1.0f);
	settings.mPosition2 = Vec3(
		local2_v4.GetX(), local2_v4.GetY(), local2_v4.GetZ()
	);

	// --- 3) compute parent‐space pivot (in parent‐COM local coords)
	RMat44  parentCOMX = parent->GetCenterOfMassTransform();
	RMat44  invParentCOM = parentCOMX.Inversed();
	Vec4    local1_v4 = invParentCOM * Vec4(worldPivot, 1.0f);
	settings.mPosition1 = Vec3(
		local1_v4.GetX(), local1_v4.GetY(), local1_v4.GetZ()
	);

	// --- 4) build twist & plane axes from childSpaceConstraintRotation
	Vec3 csTwistAxis = childSpaceConstraintRotation * Vec3(0, 1, 0);
	Vec3 csPlaneAxis = childSpaceConstraintRotation * Vec3(1, 0, 0);

	// world axes:
	Vec3 worldTwist = (child->GetRotation() * csTwistAxis).Normalized();
	Vec3 worldPlane = (child->GetRotation() * csPlaneAxis).Normalized();

	// back into parent‐local:
	Vec3 parentTwist = parent->GetRotation().Conjugated() * worldTwist;
	Vec3 parentPlane = parent->GetRotation().Conjugated() * worldPlane;

	settings.mTwistAxis1 = parentTwist;
	settings.mTwistAxis2 = csTwistAxis;
	settings.mPlaneAxis1 = parentPlane;
	settings.mPlaneAxis2 = csPlaneAxis;

	// --- 5) set rotational limits (translation is auto‐locked)
	settings.mTwistMinAngle = twistMinAngle;
	settings.mTwistMaxAngle = twistMaxAngle;
	settings.mNormalHalfConeAngle = swingHalfConeAngle;
	settings.mPlaneHalfConeAngle = swingHalfConeAngle;

	// --- 6) create & register
	Ref<TwoBodyConstraint> c = settings.Create(*parent, *child);
	physics_system->AddConstraint(c);


	return c;

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

void Physics::AddImpulse(const Body* body, vec3 impulse)
{

	if (body == nullptr) return;

	bodyInterface->AddImpulse(body->GetID(), ToPhysics(impulse));

}

void Physics::AddImpulseAtLocation(const Body* body, vec3 impulse, vec3 point)
{

	if (body == nullptr) return;

	bodyInterface->AddImpulse(body->GetID(), ToPhysics(impulse), ToPhysics(point));

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
	auto sphere_shape_settings = JPH::SphereShapeSettings();
	sphere_shape_settings.SetEmbedded();
	sphere_shape_settings.mRadius = radius;
	JPH::Shape::ShapeResult shape_result = sphere_shape_settings.Create();
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


	hit.hasHit = hit.hasHit && hit.entity != nullptr;

	if (hit.hasHit)
	{
		if (hit.entity->Destroyed)
			hit.hasHit = false;

	}

	return hit;
}

Physics::HitResult Physics::CylinderTrace(const vec3 start, const vec3 end, float radius, float halfHeight, const BodyType mask, const vector<Body*> ignoreList)
{
	HitResult hit;
	// Initialize default values
	hit.fraction = 1.0f;
	hit.position = end;
	hit.normal = vec3(0, 0, 0);
	hit.hitbody = nullptr;
	hit.hasHit = false;
	hit.shapePosition = end;
	hit.entity = nullptr;

	// Convert to Jolt coordinates
	JPH::Vec3 startLoc = ToPhysics(start);
	JPH::Vec3 endLoc = ToPhysics(end);
	JPH::Vec3 direction = endLoc - startLoc;

	// Handle zero-length trace
	if (direction.IsNearZero()) {
		return hit;
	}

	// Create cylinder shape
	auto cylinder_shape_settings = JPH::CylinderShapeSettings(halfHeight, radius);
	cylinder_shape_settings.SetEmbedded();
	JPH::Shape::ShapeResult shape_result = cylinder_shape_settings.Create();
	JPH::ShapeRefC cylinder_shape = shape_result.Get();

	if (shape_result.HasError()) {
		Logger::Log(shape_result.GetError().c_str());
		return hit;
	}

	// Align cylinder axis with trace direction
	JPH::Vec3 normalizedDirection = direction.Normalized();
	JPH::Quat rotation = JPH::Quat::sFromTo(JPH::Vec3::sAxisY(), normalizedDirection);
	JPH::RMat44 start_transform = JPH::RMat44::sRotationTranslation(rotation, startLoc);

	// Set up shape cast
	JPH::RShapeCast shape_cast(cylinder_shape, JPH::Vec3::sReplicate(1.0f), start_transform, direction);

	// Configure collision filter
	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;

	// Perform shape cast
	// physicsMainLock.lock(); // Uncomment if thread safety required
	ClosestHitShapeCastCollector collector;
	physics_system->GetNarrowPhaseQuery().CastShape(shape_cast, JPH::ShapeCastSettings(), JPH::Vec3::sZero(), collector, {}, {}, filter);

	if (collector.HadHit()) {
		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), collector.GetHit().mBodyID2);
		const JPH::Body* body = &body_lock.GetBody();
		if (body) {
			// Process hit shape
			const JPH::Shape* root_shape = body->GetShape();
			JPH::SubShapeID remainder;
			const JPH::Shape* hit_shape = root_shape->GetLeafShape(collector.GetHit().mSubShapeID2, remainder);
			if (hit_shape) {
				int hitSurfaceId = hit_shape->GetUserData();
				if (hitSurfaceId) {
					hit.surfaceName = FindSurfacyById(hitSurfaceId);
				}
			}

			// Extract hit information
			hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(collector.GetHit().mSubShapeID2, collector.GetHit().mContactPointOn2));
			hit.fraction = collector.GetHit().mFraction;
			hit.shapePosition = mix(start, end, hit.fraction);
			hit.position = FromPhysics(collector.GetHit().mContactPointOn2);

			auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
			hit.entity = props->OwnerEntity;
			hit.hitbody = body;
			hit.hasHit = true;
			hit.hitboxName = props->hitboxName;
		}
	}
	// physicsMainLock.unlock(); // Uncomment if thread safety required


	// Validate hit entity
	hit.hasHit = hit.hasHit && hit.entity != nullptr;
	if (hit.hasHit && hit.entity->Destroyed) {
		hit.hasHit = false;
	}

	return hit;
}

Physics::HitResult Physics::ShapeTrace(const Shape* shape, vec3 start, vec3 end, vec3 scale, const BodyType mask, const vector<Body*> ignoreList)
{
	HitResult hit;
	hit.fraction = 1.0f;
	hit.position = end;
	hit.normal = vec3(0, 0, 0);
	hit.hitbody = nullptr;
	hit.hasHit = false;
	hit.shapePosition = end;
	hit.entity = nullptr;

	JPH::Vec3 startLoc = ToPhysics(start);
	JPH::Vec3 endLoc = ToPhysics(end);
	JPH::Vec3 direction = endLoc - startLoc;

	if (direction.IsNearZero()) {
		return hit;
	}

	JPH::ShapeRefC cylinder_shape = shape;

	JPH::Vec3 normalizedDirection = direction.Normalized();
	JPH::Quat rotation = JPH::Quat::sFromTo(JPH::Vec3::sAxisY(), normalizedDirection);
	JPH::RMat44 start_transform = JPH::RMat44::sRotationTranslation(rotation, startLoc);

	JPH::Vec3 joltScale(scale.x, scale.y, scale.z); // Apply custom scaling
	JPH::RShapeCast shape_cast(cylinder_shape, joltScale, start_transform, direction);

	TraceBodyFilter filter;
	filter.mask = mask;
	filter.ignoreList = ignoreList;

	ClosestHitShapeCastCollector collector;
	physics_system->GetNarrowPhaseQuery().CastShape(shape_cast, JPH::ShapeCastSettings(), JPH::Vec3::sZero(), collector, {}, {}, filter);

	if (collector.HadHit()) {
		JPH::BodyLockRead body_lock(physics_system->GetBodyLockInterface(), collector.GetHit().mBodyID2);
		const JPH::Body* body = &body_lock.GetBody();
		if (body) {
			const JPH::Shape* root_shape = body->GetShape();
			JPH::SubShapeID remainder;
			const JPH::Shape* hit_shape = root_shape->GetLeafShape(collector.GetHit().mSubShapeID2, remainder);
			if (hit_shape) {
				int hitSurfaceId = hit_shape->GetUserData();
				if (hitSurfaceId) {
					hit.surfaceName = FindSurfacyById(hitSurfaceId);
				}
			}

			hit.normal = FromPhysics(body->GetWorldSpaceSurfaceNormal(collector.GetHit().mSubShapeID2, collector.GetHit().mContactPointOn2));
			hit.fraction = collector.GetHit().mFraction;
			hit.shapePosition = mix(start, end, hit.fraction);
			hit.position = FromPhysics(collector.GetHit().mContactPointOn2);

			auto* props = reinterpret_cast<BodyData*>(body->GetUserData());
			hit.entity = props->OwnerEntity;
			hit.hitbody = body;
			hit.hasHit = hit.entity != nullptr && !hit.entity->Destroyed;
			hit.hitboxName = props->hitboxName;
		}
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
	auto capsule_shape_settings = JPH::CapsuleShapeSettings();
	capsule_shape_settings.SetEmbedded();
	capsule_shape_settings.mRadius = Radius;
	capsule_shape_settings.mHalfHeightOfCylinder = cylinder_half_height;

	JPH::Shape::ShapeResult shape_result = capsule_shape_settings.Create();
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
	BodyData* properties = new BodyData{ group, mask,false, owner };
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
	auto cylinder_shape_settings = JPH::CylinderShapeSettings(
		cylinder_half_height,
		Radius
	);
	cylinder_shape_settings.SetEmbedded();

	JPH::Shape::ShapeResult shape_result = cylinder_shape_settings.Create();
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
	BodyData* properties = new BodyData{ group, mask, false, owner };
	body_settings.mUserData = reinterpret_cast<uintptr_t>(properties);

	// Create and add body to world
	JPH::Body* cylinder_body = bodyInterface->CreateBody(body_settings);
	AddBody(cylinder_body);

	return cylinder_body;
}

static int EnsureJoint(
	JPH::Skeleton& skel,
	std::unordered_map<std::string, int>& indexOf,
	const std::string& name,
	int parent_idx)
{
	auto it = indexOf.find(name);
	if (it != indexOf.end()) return it->second;

	int idx = (int)skel.GetJointCount();
	skel.AddJoint(name.c_str(), parent_idx);
	indexOf[name] = idx;
	return idx;
}

static Ref<Shape> MakeHitboxShape(const Physics::HitboxData& hb)
{
	// 1) box in its local origin
	BoxShapeSettings box;
	box.mHalfExtent = ToPhysics(hb.size * 0.5f);
	box.SetEmbedded();

	// 2) offset/rotate in shape space
	RotatedTranslatedShapeSettings rts(
		ToPhysics(hb.position),
		ToPhysics(hb.rotation),
		&box
	);
	rts.SetEmbedded();

	auto sr = rts.Create();
	JPH_ASSERT(!sr.HasError());
	return sr.Get();
}

static void ComputeSwingTwistAxesLocalToBodies(
	const Body& parent, const Body& child,
	const Quat& childSpaceConstraintRotation,
	Vec3& twistAxis1, Vec3& planeAxis1,
	Vec3& twistAxis2, Vec3& planeAxis2)
{
	// child-space canonical axes
	Vec3 csTwist = childSpaceConstraintRotation * Vec3(0, 1, 0);
	Vec3 csPlane = childSpaceConstraintRotation * Vec3(1, 0, 0);

	// world from child
	Vec3 wTwist = (child.GetRotation() * csTwist).Normalized();
	Vec3 wPlane = (child.GetRotation() * csPlane).Normalized();

	// parent local (1) and child local (2)
	twistAxis1 = parent.GetRotation().Conjugated() * wTwist; // parent local
	planeAxis1 = parent.GetRotation().Conjugated() * wPlane;
	twistAxis2 = csTwist;                                     // child local by definition
	planeAxis2 = csPlane;
}

static void ComputeLocalPivotsAtJointPosition(
	const Body& parent, const Body& child,
	Vec3& p1_local, Vec3& p2_local)
{
	// use child's current (bind) joint location as pivot
	RMat44 childX = child.GetWorldTransform();
	Vec3   worldPivot = childX.GetTranslation();

	// to child COM local:
	RMat44 invChildCOM = child.GetCenterOfMassTransform().Inversed();
	p2_local = Vec3(invChildCOM * Vec4(worldPivot, 1.0f));

	// to parent COM local:
	RMat44 invParentCOM = parent.GetCenterOfMassTransform().Inversed();
	p1_local = Vec3(invParentCOM * Vec4(worldPivot, 1.0f));
}

// -----------------------------------------------------------------------------------

Physics::RagdollHandle* Physics::CreateRagdollFromHitboxes(
	Entity* owner,
	const std::vector<HitboxData>& hitboxes,
	const std::unordered_map<std::string, glm::mat4>& restPoseModelSpace,
	JPH::ObjectLayer layer,
	uint32_t groupFilterID)
{
	if (hitboxes.empty())
		return nullptr;

	// -------- 0) Build a parent-before-child ordering (topological sort on names) --------
	// Map bone name -> list of children
	std::unordered_map<std::string, std::vector<std::string>> children;
	std::unordered_map<std::string, int> indeg;
	std::unordered_map<std::string, HitboxData> byName;

	for (auto& hb : hitboxes) {
		byName[hb.boneName] = hb;
		indeg.try_emplace(hb.boneName, 0);
	}
	for (auto& hb : hitboxes) {
		if (!hb.parentBone.empty() && byName.count(hb.parentBone)) {
			children[hb.parentBone].push_back(hb.boneName);
			indeg[hb.boneName]++;
		}
	}

	std::vector<std::string> order;
	order.reserve(hitboxes.size());
	std::queue<std::string> q;
	for (auto& kv : indeg) if (kv.second == 0) q.push(kv.first);
	while (!q.empty()) {
		auto n = q.front(); q.pop();
		order.push_back(n);
		for (auto& c : children[n]) {
			if (--indeg[c] == 0) q.push(c);
		}
	}
	if (order.size() != hitboxes.size()) {
		// cycle or missing parents — fallback to input order
		order.clear();
		for (auto& hb : hitboxes) order.push_back(hb.boneName);
	}

	// -------- 1) Build Skeleton with parents BEFORE children --------
	JPH::Ref<JPH::Skeleton> skeleton = new JPH::Skeleton();
	skeleton->GetJoints().resize(order.size());

	// name -> index and parent index resolution
	std::unordered_map<std::string, int> nameToIndex;
	nameToIndex.reserve(order.size());
	for (int i = 0; i < (int)order.size(); ++i)
		nameToIndex[order[i]] = i;

	for (int i = 0; i < (int)order.size(); ++i) {
		const HitboxData& hb = byName[order[i]];
		JPH::Skeleton::Joint& j = skeleton->GetJoints()[i];
		j.mName = hb.boneName;
		j.mParentJointIndex = -1;
		if (!hb.parentBone.empty()) {
			auto pit = nameToIndex.find(hb.parentBone);
			if (pit != nameToIndex.end()) j.mParentJointIndex = pit->second;
		}
	}

	// -------- 2) Ragdoll settings with one Part per joint --------
	JPH::Ref<JPH::RagdollSettings> settings = new JPH::RagdollSettings();
	settings->mSkeleton = skeleton;
	settings->mParts.resize(skeleton->GetJointCount());

	// Helper to fetch model-space (skeleton space) rest transforms
	auto getRestModel = [&](const std::string& bone) -> JPH::Mat44 {
		auto it = restPoseModelSpace.find(bone);
		if (it == restPoseModelSpace.end())
			return JPH::Mat44::sIdentity();
		return ToPhysics(it->second);
		};

	// -------- 3) Fill each Part (shape, mass, initial pose, user data, etc.) --------
	for (int i = 0; i < (int)order.size(); ++i)
	{
		const HitboxData& hb = byName[order[i]];
		JPH::RagdollSettings::Part& part = settings->mParts[i]; // derives BodyCreationSettings

		// Shape: same approach as your CreateHitBoxBody (offset box in the body's local space)
		JPH::BoxShapeSettings box_ss(ToPhysics(hb.size) * 0.5f);
		box_ss.SetEmbedded();

		JPH::RotatedTranslatedShapeSettings rts_ss(
			ToPhysics(hb.position),
			ToPhysics(hb.rotation),
			&box_ss
		);
		rts_ss.SetEmbedded();

		// Create final shape now and set it on the part
		JPH::Shape::ShapeResult sr = rts_ss.Create();
		if (sr.HasError()) {
			Logger::Log(sr.GetError().c_str());
			return nullptr;
		}
		JPH::RefConst<JPH::Shape> final_shape = sr.Get();
		part.SetShape(final_shape);

		// Initial transform of this body in MODEL space (not world!)
		// We use the rest pose (model space) as starting transform.
		JPH::Mat44 m_model = getRestModel(hb.boneName);
		JPH::Vec3  pos_m = m_model.GetTranslation();
		JPH::Quat  rot_m = m_model.GetRotation().GetQuaternion();

		part.mPosition = JPH::RVec3(pos_m);     // Body in "skeleton/model" space
		part.mRotation = rot_m;
		part.mObjectLayer = layer;
		part.mMotionType = JPH::EMotionType::Dynamic;

		// Mass & damping etc.
		part.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
		part.mMassPropertiesOverride.mMass = hb.mass > 0.0f ? hb.mass : 1.0f;
		part.mFriction = 0.9f;
		part.mAngularDamping = 10.0f;

		// User data (like your CreateHitBoxBody)
		BodyData* props = new BodyData{ BodyType::HitBox, BodyType::None, true, owner, hb.boneName };
		part.mUserData = reinterpret_cast<uintptr_t>(props);

		// -------- Constraint to parent (if any) in LocalToBodyCOM --------
		if (!hb.parentBone.empty())
		{
			// Child & parent model-space COM transforms
			JPH::Mat44 childCOM = JPH::Mat44::sRotationTranslation(rot_m, pos_m);

			const int parentIdx = skeleton->GetJoints()[i].mParentJointIndex;
			JPH::Mat44 parentCOM = JPH::Mat44::sIdentity();
			JPH::Quat  parentRot = JPH::Quat::sIdentity();
			if (parentIdx >= 0) {
				const std::string& pname = skeleton->GetJoints()[parentIdx].mName.c_str();
				parentCOM = getRestModel(pname);
				parentRot = parentCOM.GetRotation().GetQuaternion();
			}

			// Joint pivot in model space = child joint position (rest)
			const JPH::Vec3 jointPosModel = pos_m;

			// Local pivots (relative to each COM)
			JPH::RMat44 invParent = JPH::RMat44(parentCOM).Inversed();
			JPH::RMat44 invChild = JPH::RMat44(childCOM).Inversed();

			JPH::Vec3 pivot1_local = (invParent * JPH::RVec3(jointPosModel));
			JPH::Vec3 pivot2_local = (invChild * JPH::RVec3(jointPosModel));

			// Axes:
			// child-space axes come from hb.constraintRotation (child local frame)
			const JPH::Quat childConstraintRot = ToPhysics(hb.rotation) * ToPhysics(hb.constraintRotation);
			// if you already baked rotation offset in shape, you may use just hb.constraintRotation

			const JPH::Vec3 childTwistAxis = (childConstraintRot * JPH::Vec3(0, 1, 0)).Normalized();
			const JPH::Vec3 childPlaneAxis = (childConstraintRot * JPH::Vec3(1, 0, 0)).Normalized();

			// Convert those axes to MODEL space using child rest rotation, then into parent's local
			const JPH::Vec3 modelTwistAxis = (rot_m * childTwistAxis).Normalized();
			const JPH::Vec3 modelPlaneAxis = (rot_m * childPlaneAxis).Normalized();

			const JPH::Vec3 parentTwistAxis = (parentRot.Conjugated() * modelTwistAxis).Normalized();
			const JPH::Vec3 parentPlaneAxis = (parentRot.Conjugated() * modelPlaneAxis).Normalized();

			// Build constraint settings
			JPH::Ref<JPH::SwingTwistConstraintSettings> cs = new JPH::SwingTwistConstraintSettings();
			cs->mSpace = JPH::EConstraintSpace::LocalToBodyCOM;

			// Local pivots
			cs->mPosition1 = pivot1_local;   // in parent COM local space
			cs->mPosition2 = pivot2_local;   // in child COM local space

			// Local axes
			cs->mTwistAxis1 = parentTwistAxis;  // in parent local
			cs->mTwistAxis2 = childTwistAxis;   // in child local
			cs->mPlaneAxis1 = parentPlaneAxis;  // in parent local
			cs->mPlaneAxis2 = childPlaneAxis;   // in child local

			// Limits (degrees -> radians)
			cs->mTwistMinAngle = MathHelper::ToRadians(hb.twistParameters.x);
			cs->mTwistMaxAngle = MathHelper::ToRadians(hb.twistParameters.y);
			cs->mNormalHalfConeAngle = MathHelper::ToRadians(hb.twistParameters.z);
			cs->mPlaneHalfConeAngle = MathHelper::ToRadians(hb.twistParameters.z);

			part.mToParent = cs; // Ref<TwoBodyConstraintSettings>
		}
	}

	// -------- 4) Collision filtering & stabilization --------
	// (passing nullptr matrices lets Jolt figure out overlaps using current shapes)
	settings->DisableParentChildCollisions(nullptr, 0.0f);
	settings->Stabilize();

	// -------- 5) Create ragdoll & add to physics --------
	JPH::CollisionGroup::GroupID group_id = static_cast<JPH::CollisionGroup::GroupID>(groupFilterID);
	JPH::Ragdoll* ragdoll = settings->CreateRagdoll(group_id, reinterpret_cast<uint64_t>(owner), physics_system);
	if (!ragdoll)
		return nullptr;

	ragdoll->AddToPhysicsSystem(JPH::EActivation::Activate);

	// -------- 6) Wrap into handle, fill maps --------
	Physics::RagdollHandle* handle = new Physics::RagdollHandle();
	handle->ragdoll = ragdoll;
	handle->groupFilterID = groupFilterID;
	handle->skeleton = skeleton;


	for (int i = 0; i < ragdoll->GetBodyCount(); i++)
	{

		Body* body = GetBodyFromId(ragdoll->GetBodyID(i));

		handle->boneToBody[Physics::GetBodyData(body)->hitboxName] = body;
	}
		

	// bone -> constraint mapping (constraint c corresponds to body c+1)
	
	const uint ccount = ragdoll->GetConstraintCount();
	for (uint c = 0; c < ccount; ++c) {
		JPH::TwoBodyConstraint* con = ragdoll->GetConstraint(c);
		if (!con) continue;
		const uint childIdx = c + 1;
		if (childIdx < order.size()) {
			const std::string& boneName = order[childIdx];
			handle->constraintOf[boneName] = con; // requires handle->constraintOf map
		}
	}


	return handle;
}



// -----------------------------------------------------------------------------------
// Convenience API impl

void Physics::DestroyRagdoll(RagdollHandle*& h)
{
	if (!h) return;
	if (h->ragdoll)
	{
		h->ragdoll->RemoveFromPhysicsSystem();
		h->ragdoll = nullptr;
	}
	delete h;
	h = nullptr;
}

JPH::Body* Physics::GetRagdollBody(RagdollHandle* h, const std::string& bone)
{
	if (!h || !h->ragdoll)
		return nullptr;

	auto it = h->boneToBody.find(bone);
	if (it == h->boneToBody.end())
		return nullptr;



	return it->second;
}


JPH::TwoBodyConstraint* Physics::GetRagdollConstraint(RagdollHandle* h, const std::string& childBone)
{
	if (!h) return nullptr;
	auto it = h->constraintOf.find(childBone);
	return (it != h->constraintOf.end()) ? it->second : nullptr;
}



void Physics::SetRagdollSimulated(RagdollHandle* h, bool enable_dynamic)
{
	if (!h || !h->ragdoll) return;
	for (uint i = 0; i < h->ragdoll->GetBodyCount(); ++i)
	{
		Body* b = GetBodyFromId(h->ragdoll->GetBodyID(i));
		bodyInterface->SetMotionType(b->GetID(),
			enable_dynamic ? EMotionType::Dynamic : EMotionType::Kinematic,
			EActivation::Activate);
	}
}


void Physics::SetRagdollPose(RagdollHandle* h, const std::unordered_map<std::string, mat4>& bonePose)
{
	if (!h || !h->ragdoll) return;
	auto pose = BuildSkeletonPoseFromMap(h, bonePose);
	h->ragdoll->SetPose(pose);
}

void Physics::DriveRagdollToPoseUsingKinematics(RagdollHandle* h, const std::unordered_map<std::string, mat4>& bonePose, float dt)
{
	if (!h || !h->ragdoll) return;
	auto pose = BuildSkeletonPoseFromMap(h, bonePose);
	h->ragdoll->DriveToPoseUsingKinematics(pose, dt);
}

void Physics::DriveRagdollToPoseUsingMotors(RagdollHandle* h, const std::unordered_map<std::string, mat4>& bonePose)
{
	if (!h || !h->ragdoll) return;
	auto pose = BuildSkeletonPoseFromMap(h, bonePose);
	h->ragdoll->DriveToPoseUsingMotors(pose);
}

JPH::SkeletonPose Physics::BuildSkeletonPoseFromMap(RagdollHandle* h, const std::unordered_map<std::string, glm::mat4>& bonePose)
{
	{
		JPH::SkeletonPose pose;
		pose.SetSkeleton(h->skeleton);

		const JPH::Skeleton* skel = h->skeleton;

		for (auto& [boneName, mm] : bonePose)
		{
			/*
			auto it = h->boneToBodyIndex.find(boneName);
			if (it == h->boneToBodyIndex.end())
				continue;

			auto matModel = ToPhysics(mm);

			int boneIdx = it->second;
			int parentIdx = skel->GetJoint(boneIdx).mParentJointIndex;

			JPH::Mat44 matLocal;

			if (parentIdx >= 0)
			{
				auto parentIt = bonePose.find(skel->GetJoint(parentIdx).mName.c_str());
				if (parentIt != bonePose.end())
				{
					JPH::Mat44 parentModel = ToPhysics(parentIt->second);
					matLocal = parentModel.Inversed() * matModel;
				}
				else
				{
					// fallback: assume given matrix is already local
					matLocal = matModel;
				}
			}
			else
			{
				// root: treat as local
				matLocal = matModel;
			}

			// Decompose matLocal into rotation + translation
			JPH::Vec3 translation = matLocal.GetTranslation();
			JPH::Quat rotation = matLocal.GetRotation().GetQuaternion();

			JPH::SkeletonPose::JointState& js = pose.GetJoint(boneIdx);
			js.mTranslation = translation;
			js.mRotation = rotation;

			*/

		}

		return pose;
	}
}
