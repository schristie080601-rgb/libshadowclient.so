#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <android/log.h>
#include <sys/system_properties.h>
#include <vector>
#include <string>
#include "KittyMemory/KittyMemory.h"
#include "KittyMemory/MemoryPatch.h"
#include "KittyMemory/KittyUtils.h"
#include <substrate.h>  // Include for MSHookFunction, make sure it's in the template or add it

#define LOG_TAG "下去了's Menu"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Vector3 struct (add if not in KittyUtils)
struct Vector3 {
    float x, y, z;
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 operator*(float scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    Vector3 normalized() const {
        float mag = sqrt(x*x + y*y + z*z);
        if (mag == 0) return *this;
        return *this * (1.0f / mag);
    }
};

std::vector<std::string> featureList;
bool fly = false, longarms = false, pullmod = false, speedboost = false, noclip = false, platforms = false, wallwalk = false; // Added wallwalk

uintptr_t libil2cpp_base = 0;

// Offsets - REPLACE THESE WITH YOUR DUMP'S HEX VALUES
#define PLAYER_INSTANCE_OFFSET 0x003D0E38  // Example for GorillaLocomotion.Player::Instance (static field offset)
#define GRAVITY_MULTIPLIER_OFFSET 0xC4  // Relative to Player instance, gravityMultiplier float
#define USE_GRAVITY_OFFSET 0xC8  // Relative, useGravity bool
#define JUMP_MULTIPLIER_OFFSET 0xB0  // jumpMultiplier float
#define MAX_ARM_LENGTH_OFFSET 0xA8  // maxArmLength float
#define BODY_COLLIDER_OFFSET 0x30  // bodyCollider ptr relative to Player
#define COLLIDER_ENABLED_OFFSET 0x20  // enabled bool relative to Collider ptr
#define PLAYER_UPDATE_OFFSET 0x01A2B3C0  // Method GorillaLocomotion.Player::Update offset
#define VRRIG_LIST_OFFSET 0x003E4F50  // Example for VRRigCache or player list array
#define POSITION_OFFSET 0xC0  // Transform.position Vector3 relative to VRRig
#define OVR_BASE_LIB "libunity.so"  // or "libOVRPlatform.so" for OVRInput
#define OVR_A_BUTTON_OFFSET 0x00ABCDEF  // OVRInput.Get(OVRInput.Button.One, OVRInput.Controller.RTouch) state
#define OVR_B_BUTTON_OFFSET 0x00FEDCBA  // Button.Two
#define VRRIG_SIZE 0x8  // Ptr size for array

// Hook for Player.Update
void (*old_Player_Update)(void *instance);
void Player_Update(void *instance) {
    if (instance == nullptr) return;

    if (libil2cpp_base == 0) libil2cpp_base = KittyMemory::getLibraryAddress("libil2cpp.so");

    // Pull Mod
    if (pullmod) {
        uintptr_t playerInstance = *(uintptr_t*)(libil2cpp_base + PLAYER_INSTANCE_OFFSET);
        if (playerInstance) {
            uintptr_t localRig = *(uintptr_t*)(playerInstance + 0x98);  // Example currentRig offset, replace
            if (localRig) {
                Vector3 localPos = *(Vector3*)(localRig + POSITION_OFFSET);
                for (int i = 0; i < 32; i++) {  // Max players
                    uintptr_t otherRig = *(uintptr_t*)(libil2cpp_base + VRRIG_LIST_OFFSET + i * VRRIG_SIZE);
                    if (otherRig && otherRig != localRig) {
                        Vector3* otherPos = (Vector3*)(otherRig + POSITION_OFFSET);
                        Vector3 dir = localPos - *otherPos;
                        *otherPos = *otherPos + dir.normalized() * 5.0f * Time_deltaTime();  // Assume Time hook or 0.016f
                    }
                }
            }
        }
    }

    // Platforms - Placeholder for spawning
    static float timer = 0;
    timer += 0.016f;
    if (platforms && timer > 0.5f) {
        timer = 0;
        // To implement: Call il2cpp method for GameObject.Instantiate(platformPrefab, pos, rot)
        // Need prefab offset, or hardcode simple cube
        LOGD("Spawning platform - implement Instantiate here");
    }

    if (old_Player_Update) old_Player_Update(instance);
}

// getFeatureList
std::vector<std::string> getFeatureList() {
    featureList.clear();
    featureList.push_back("Category_Movement");
    featureList.push_back("Toggle_Fly");
    featureList.push_back("Toggle_Long Arms");
    featureList.push_back("Toggle_Speed Boost");
    featureList.push_back("Toggle_Noclip");
    featureList.push_back("Toggle_Pull Mod");
    featureList.push_back("Toggle_Wall Walk"); // Added Wall Walk
    featureList.push_back("Category_Player");
    featureList.push_back("Category_Room");
    featureList.push_back("Toggle_Platforms");
    // Removed Category_Op, Category_All, Category_Settings, Category_Credits
    return featureList;
}

// Changes
void Changes(JNIEnv *env, jclass clazz, jint feature, jboolean boolean, jint value) {
    switch (feature) {
        case 0: fly = boolean; break;
        case 1: longarms = boolean; break;
        case 2: speedboost = boolean; break;
        case 3: noclip = boolean; break;
        case 4: pullmod = boolean; break;
        case 5: wallwalk = boolean; break; // Added Wall Walk case
        case 6: platforms = boolean; break; // Shifted Platforms index
    }
}

// hack_thread
void* hack_thread(void*) {
    do {
        libil2cpp_base = KittyMemory::getLibraryAddress("libil2cpp.so");
        sleep(1);
    } while (!libil2cpp_base);

    // Hook Update
    MSHookFunction((void*)(libil2cpp_base + PLAYER_UPDATE_OFFSET), (void*)Player_Update, (void**)&old_Player_Update);

    while (true) {
        uintptr_t playerInstance = *(uintptr_t*)(libil2cpp_base + PLAYER_INSTANCE_OFFSET);
        if (playerInstance) {
            if (fly) {
                float zero = 0.0f;
                KittyMemory::memWrite((void*)(playerInstance + GRAVITY_MULTIPLIER_OFFSET), &zero, sizeof(float));
            } else {
                float normalGravity = 1.2f;  // GTAG normal, adjust
                KittyMemory::memWrite((void*)(playerInstance + GRAVITY_MULTIPLIER_OFFSET), &normalGravity, sizeof(float));
            }

            if (speedboost) {
                float fast = 2.0f; // Example multiplier
                KittyMemory::memWrite((void*)(playerInstance + JUMP_MULTIPLIER_OFFSET), &fast, sizeof(float));
            } else {
                float normal = 1.1f;
                KittyMemory::memWrite((void*)(playerInstance + JUMP_MULTIPLIER_OFFSET), &normal, sizeof(float));
            }

            if (noclip) {
                uintptr_t collider = *(uintptr_t*)(playerInstance + BODY_COLLIDER_OFFSET);
                if (collider) {
                    bool falseVal = false;
                    KittyMemory::memWrite((void*)(collider + COLLIDER_ENABLED_OFFSET), &falseVal, sizeof(bool));
                }
            } else {
                uintptr_t collider = *(uintptr_t*)(playerInstance + BODY_COLLIDER_OFFSET);
                if (collider) {
                    bool trueVal = true;
                    KittyMemory::memWrite((void*)(collider + COLLIDER_ENABLED_OFFSET), &trueVal, sizeof(bool));
                }
            }

            if (longarms) {
                float longVal = 2.5f;
                KittyMemory::memWrite((void*)(playerInstance + MAX_ARM_LENGTH_OFFSET), &longVal, sizeof(float));
            } else {
                float normal = 1.2f;
                KittyMemory::memWrite((void*)(playerInstance + MAX_ARM_LENGTH_OFFSET), &normal, sizeof(float));
            }

            // --- Wall Walk Implementation Placeholder ---
            if (wallwalk) {
                // Implement wall walk logic here.
                // This might involve:
                // 1. Detecting proximity to walls.
                // 2. Modifying player's velocity or position to stick to walls.
                // 3. Potentially adjusting gravity or collision detection.
                LOGD("Wall Walk is ON - Implement game-specific logic here!");
            }
            // ------------------------------------------
        }

        usleep(16000); // ~60 FPS
    }
    return nullptr;
}

// vr_input_thread
void* vr_input_thread(void*) {
    uintptr_t unity_base = KittyMemory::getLibraryAddress(OVR_BASE_LIB);
    if (!unity_base) return nullptr;

    JavaVM* jvm;
    JNIEnv *env = NULL;
    if (JNI_GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        // Attach if not attached
        JNI_GetCreatedJavaVMs(&jvm, 1, NULL);
        jvm->AttachCurrentThread(&env, NULL);
    }

    jclass modClass = env->FindClass("modmenu/FloatingModMenuService");
    jmethodID showID = env->GetStaticMethodID(modClass, "showMenu", "()V");
    jmethodID hideID = env->GetStaticMethodID(modClass, "hideMenu", "()V");

    bool lastA = false, lastB = false;
    while (true) {
        bool currA = *(bool*)(unity_base + OVR_A_BUTTON_OFFSET);
        bool currB = *(bool*)(unity_base + OVR_B_BUTTON_OFFSET);
        if (currA && !lastA) {
            env->CallStaticVoidMethod(modClass, showID);
        }
        if (currB && !lastB) {
            env->CallStaticVoidMethod(modClass, hideID);
        }
        lastA = currA;
        lastB = currB;
        usleep(50000); // 20Hz
    }
    jvm->DetachCurrentThread();
    return nullptr;
}

// JNI startHack
extern "C" JNIEXPORT void JNICALL Java_modmenu_FloatingModMenuService_startHack(JNIEnv *env, jobject thiz) {
pthread_t hThread, iThread;
pthread_create(&hThread, NULL, hack_thread, NULL);
pthread_create(&iThread, NULL, vr_input_thread, NULL);
}
