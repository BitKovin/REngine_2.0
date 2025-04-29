#include "Physics.h"

#include "Entity.hpp"

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
