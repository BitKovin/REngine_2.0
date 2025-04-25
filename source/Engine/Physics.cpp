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

vector<Body*> Physics::existingBodies = vector<Body*>();
recursive_mutex Physics::physicsMainLock = recursive_mutex();

bool Physics::DebugDraw = false;
MyDebugRenderer* Physics::debugRenderer = nullptr;
BodyType Physics::DebugDrawMask = BodyType::GroupAll & ~BodyType::CharacterCapsule;

std::vector<SubShapeIDPair> Physics::gRemovals;

void MyContactListener::OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{
	auto data1 = Physics::GetBodyData(&inBody1);
	auto data2 = Physics::GetBodyData(&inBody2);

	Entity* entity1 = data1->OwnerEntity;
	Entity* entity2 = data2->OwnerEntity;

	if (entity1 == nullptr || entity2 == nullptr) return;


	entity1->OnBodyEntered(entity2->LeadBody,entity2);
	entity2->OnBodyEntered(entity1->LeadBody, entity1);
}

void MyContactListener::OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
{

}

void MyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
    
	Physics::gRemovals.push_back(inSubShapePair);

}

void Physics::UpdatePendingBodyExits()
{
	for (auto& pair : gRemovals)
	{
		std::array<BodyID, 2> ids = { pair.GetBody1ID(), pair.GetBody2ID() };
		BodyLockMultiRead lock(physics_system->GetBodyLockInterface(), ids.data(), 2);

		const Body* body1 = lock.GetBody(0);
		const Body* body2 = lock.GetBody(1);

		if (body1 == nullptr || body2 == nullptr) return;

		auto data1 = Physics::GetBodyData(body1);
		auto data2 = Physics::GetBodyData(body2);
		Entity* e1 = data1->OwnerEntity, * e2 = data2->OwnerEntity;
		if (!e1 || !e2) return;
		e1->OnBodyExited(e2->LeadBody, e2);
		e2->OnBodyExited(e1->LeadBody, e1);

		
	}
	gRemovals.clear();
}
