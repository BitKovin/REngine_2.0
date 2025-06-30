#include "model_animator.hpp"



#include "glm.h"


int roj::Animator::getKeyTransformIdx(std::vector<float>& timestamps)
{
    if (timestamps.size() < 2)
        return -1;

    float adjustedTime = m_currTime;

    // Non-looping: clamp to last segment if time exceeds last timestamp
    if (!Loop && adjustedTime >= timestamps.back()) {
        return timestamps.size() - 2;
    }

    // Find segment containing current time
    for (int index = 0; index < timestamps.size() - 1; ++index) {
        if (adjustedTime < timestamps[index + 1]) {
            return index;
        }
    }

    return timestamps.size() - 2; // Fallback (shouldn't reach here for looping)
}

float roj::Animator::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    if (m_currTime < lastTimeStamp) return 0.f;
    float midWayLength = m_currTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    return midWayLength / framesDiff;
}

glm::mat4 roj::Animator::interpolatePosition(roj::FrameBoneTransform& boneTransform)
{
    int posIdx = getKeyTransformIdx(boneTransform.positionTimestamps);
    if (posIdx == -1)
        return glm::translate(glm::mat4(1.0f), boneTransform.positions[0]);

    if (Loop && posIdx == boneTransform.positionTimestamps.size() - 1)
    {
        // Wrap-around case: interpolate between last and first keyframe
        float lastTime = boneTransform.positionTimestamps.back();
        float nextTime = boneTransform.positionTimestamps[0] + m_currAnim->duration;
        float scaleFactor = (m_currTime - lastTime) / (nextTime - lastTime);
        glm::vec3 finalPosition = glm::mix(
            boneTransform.positions.back(),
            boneTransform.positions[0],
            scaleFactor
        );
        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    float scaleFactor = getScaleFactor(
        boneTransform.positionTimestamps[posIdx],
        boneTransform.positionTimestamps[posIdx + 1],
        m_currTime
    );
    glm::vec3 finalPosition = glm::mix(
        boneTransform.positions[posIdx],
        boneTransform.positions[posIdx + 1],
        scaleFactor
    );
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 roj::Animator::interpolateRotation(roj::FrameBoneTransform& boneTransform)
{
    int posIdx = getKeyTransformIdx(boneTransform.rotationTimestamps);
    if (posIdx == -1)
        return glm::toMat4(glm::normalize(boneTransform.rotations[0]));

    if (Loop && posIdx == boneTransform.rotationTimestamps.size() - 1)
    {
        float lastTime = boneTransform.rotationTimestamps.back();
        float nextTime = boneTransform.rotationTimestamps[0] + m_currAnim->duration;
        float scaleFactor = (m_currTime - lastTime) / (nextTime - lastTime);
        glm::quat finalRotation = glm::slerp(
            boneTransform.rotations.back(),
            boneTransform.rotations[0],
            scaleFactor
        );
        return glm::toMat4(glm::normalize(finalRotation));
    }

    float scaleFactor = getScaleFactor(
        boneTransform.rotationTimestamps[posIdx],
        boneTransform.rotationTimestamps[posIdx + 1],
        m_currTime
    );
    glm::quat finalRotation = glm::slerp(
        boneTransform.rotations[posIdx],
        boneTransform.rotations[posIdx + 1],
        scaleFactor
    );
    return glm::toMat4(glm::normalize(finalRotation));
}

glm::mat4 roj::Animator::interpolateScaling(roj::FrameBoneTransform& boneTransform)
{
    int posIdx = getKeyTransformIdx(boneTransform.scaleTimestamps);
    if (posIdx == -1)
        return glm::scale(glm::mat4(1.0f), boneTransform.scales[0]);

    if (Loop && posIdx == boneTransform.scaleTimestamps.size() - 1)
    {
        float lastTime = boneTransform.scaleTimestamps.back();
        float nextTime = boneTransform.scaleTimestamps[0] + m_currAnim->duration;
        float scaleFactor = (m_currTime - lastTime) / (nextTime - lastTime);
        glm::vec3 finalScale = glm::mix(
            boneTransform.scales.back(),
            boneTransform.scales[0],
            scaleFactor
        );
        return glm::scale(glm::mat4(1.0f), finalScale);
    }

    float scaleFactor = getScaleFactor(
        boneTransform.scaleTimestamps[posIdx],
        boneTransform.scaleTimestamps[posIdx + 1],
        m_currTime
    );
    glm::vec3 finalScale = glm::mix(
        boneTransform.scales[posIdx],
        boneTransform.scales[posIdx + 1],
        scaleFactor
    );
    return glm::scale(glm::mat4(1.0f), finalScale);
}

void roj::Animator::calcBoneTransform(BoneNode& node, glm::mat4 offset, bool stopAfterRoot)
{
    auto it = m_currAnim->animationFrames.find(node.name);
    if (it != m_currAnim->animationFrames.end())
    {
        auto& boneTransform = it->second;
        glm::mat4 translation = interpolatePosition(boneTransform);
        glm::mat4 rotation = interpolateRotation(boneTransform);
        glm::mat4 scale = interpolateScaling(boneTransform);

        currentPose[node.name] = translation * rotation * scale;

        offset *= currentPose[node.name];
    }
    else
    {
        currentPose[node.name] = node.transform;
        offset *= node.transform;
    }



    auto it2 = m_model->boneInfoMap.find(node.name);
    if (it2 != m_model->boneInfoMap.end())
    {
        auto& boneInfo = it2->second;
        m_boneMatrices[boneInfo.id] = offset * boneInfo.offset;
    }

    if (node.name == "root")
    {
        auto trans = MathHelper::DecomposeMatrix(offset);

        rootBoneTransform = trans;

        if (stopAfterRoot)
            return;

    }

    for (roj::BoneNode& child : node.children)
    {
        calcBoneTransform(child, offset, stopAfterRoot);
    }
}

void roj::Animator::ApplyNodePose(BoneNode& node, glm::mat4 offset, std::unordered_map<std::string, mat4>& pose)
{

    auto poseRes = pose.find(node.name);
    if (poseRes != pose.end())
    {
        currentPose[node.name] = pose[node.name];
    }
    else
    {
        currentPose[node.name] = glm::identity<mat4>();
    }
    
    offset *= currentPose[node.name];

    auto it2 = m_model->boneInfoMap.find(node.name);
    if (it2 != m_model->boneInfoMap.end())
    {
        auto& boneInfo = it2->second;

        m_boneMatrices[boneInfo.id] = offset * boneInfo.offset;
    }

    for (roj::BoneNode& child : node.children)
    {
        ApplyNodePose(child, offset, pose);
    }
}

roj::Animator::Animator(SkinnedModel* model)
    : m_model(model)
{
    m_boneMatrices.resize(model->boneInfoMap.size());

    for (int i = 0; i < m_boneMatrices.size(); i++)
    {
        m_boneMatrices[i] = glm::identity<mat4>();
    }

}

roj::Animator::~Animator()
{

}

void roj::Animator::set(const std::string& name)
{
    auto it = m_model->animations.find(name);
    if (it != m_model->animations.end()) {
        m_currAnim = &it->second;
        m_currTime = 0.0f;
        currentAnimationName = name;
    }
}
std::vector<std::string> roj::Animator::get()
{
    std::vector<std::string> animNames;
    animNames.reserve(m_model->animations.size());
    for (auto& anim : m_model->animations)
    {
        animNames.emplace_back(anim.first);
    }

    return animNames;
}

std::vector<glm::mat4>& roj::Animator::getBoneMatrices()
{
    return m_boneMatrices;
}

std::unordered_map<std::string, mat4> roj::Animator::GetBonePoseArray()
{
    std::unordered_map<std::string, mat4> outVector = std::unordered_map<std::string, mat4>();

    if (m_currAnim) 
    {
        PopulateBonePoseArray(m_currAnim->rootBone, glm::mat4(1.0f), outVector);
    }

    return outVector;
}

void roj::Animator::ApplyBonePoseArray(std::unordered_map<std::string, mat4> pose)
{

    ApplyNodePose(m_model->defaultRoot, glm::identity<mat4>(), pose);
}

void roj::Animator::PopulateBonePoseArray(BoneNode& node, glm::mat4 offset, std::unordered_map<std::string, mat4>& outVector)
{
    
    outVector[node.name] = currentPose[node.name];

    for (roj::BoneNode& child : node.children)
    {
        PopulateBonePoseArray(child, offset, outVector);
    }
}

void roj::Animator::UpdateAnimationPose()
{
    if (m_currAnim)
    {
        calcBoneTransform(m_currAnim->rootBone, glm::identity<mat4>(), false);
    }
    
}

void roj::Animator::update(float dt)
{
    if (m_model == nullptr) return;

    if (m_currAnim && m_playing)
    {
        m_currTime += m_currAnim->ticksPerSec * dt;

        // Simplified time management
        if (Loop) 
        {



            if (m_currTime > m_currAnim->duration)
            {
                m_currTime -= m_currAnim->duration - m_currAnim->frameTime;
            }

        }
        else {
            m_currTime = glm::min(m_currTime, m_currAnim->duration);
        }

        m_playing = Loop || (m_currTime < m_currAnim->duration);
        calcBoneTransform(m_currAnim->rootBone, glm::identity<mat4>(), UpdatePose == false);
        updateRootMotion();
    }
}

void roj::Animator::updateRootMotion()
{

    vec3 motionPos = rootBoneTransform.Position - oldRootBoneTransform.Position;
    vec3 motionRot = MathHelper::ToYawPitchRoll(inverse(oldRootBoneTransform.RotationQuaternion) * rootBoneTransform.RotationQuaternion);


    totalRootMotionPosition += motionPos;
    totalRootMotionRotation += motionRot;

    oldRootBoneTransform = rootBoneTransform;
    

}

void roj::Animator::play()
{
    m_playing = true;

    if (m_currAnim)
    {
        m_currTime = m_currAnim->frameTime;
    }
    else
    {
        m_currTime = 0;
    }


}


void roj::Animator::reset()
{
    m_currTime = 0.0f;
}