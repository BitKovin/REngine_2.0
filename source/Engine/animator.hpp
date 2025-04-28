#ifndef ANIMATOR_HPP
#define ANIMATOR_HPP
#include "skinned_model.hpp"
#include "MathHelper.hpp"
#include <unordered_map>
#include <string>

namespace roj
{
class Animator
{

public : 

    bool Loop = false;

    float m_currTime{ 0.0f };

    bool m_playing = false;

    MathHelper::Transform oldRootBoneTransform;
    MathHelper::Transform rootBoneTransform;

    vec3 totalRootMotionPosition = vec3();
    vec3 totalRootMotionRotation = vec3();

private:

    std::vector<glm::mat4> m_boneMatrices;
    
    SkinnedModel m_model;


    bool m_loopEnabled = false;
private:
    int getKeyTransformIdx(std::vector<float>& timestamps);
    float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
    glm::mat4 interpolatePosition(FrameBoneTransform& boneTransform);
    glm::mat4 interpolateRotation(FrameBoneTransform& boneTransform);
    glm::mat4 interpolateScaling(FrameBoneTransform& boneTransform);
    void calcBoneTransform(BoneNode& node, glm::mat4 offset, bool stopAfterRoot);

    void ApplyNodePose(BoneNode& node, glm::mat4 offset, std::unordered_map<std::string, mat4>& pose);

public:

    Animation* m_currAnim{ nullptr };

    string currentAnimationName = "";

    Animator() = default;
	Animator(SkinnedModel* model);
    void play();
    void set(const std::string& name);

    bool UpdatePose = true;

    std::vector<std::string> get();
    std::vector<glm::mat4>& getBoneMatrices();

    std::unordered_map<string, glm::mat4> currentPose;

    std::unordered_map<std::string, mat4> GetBonePoseArray();

    void ApplyBonePoseArray(std::unordered_map<std::string, mat4> pose);


    void PopulateBonePoseArray(BoneNode& node, glm::mat4 offset, std::unordered_map<std::string, mat4>& outVector);

    void UpdateAnimationPose();

    void update(float dt);

    void updateRootMotion();

    void reset();
    
};
}
#endif //-ANIMATOR_HPP
