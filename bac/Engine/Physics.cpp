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
