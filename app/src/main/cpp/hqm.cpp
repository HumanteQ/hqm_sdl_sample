#include "SDL.h"
#include <jni.h>
#include <android/log.h>
#include <string>
#include "hqm.h"

const char *_getString(JNIEnv *env, jobject idString) {
    const char *str = env->GetStringUTFChars((jstring) idString, 0);
    env->ReleaseStringUTFChars((jstring) idString, str);

    return str;
}

/**
 * Asynchronously initializes HQM SDK.
 *
 *      hqm_init(
 *           "38e44d7",         // sdk key
 *           1,                 // enable debug mode
 *           1);                // allow background tasks
 *
 * @key_string               - user sdk-key.
 * @enable_debug             - enable/disable debug mode.
 * @enable_background_tasks  - allow sdk jobs to run in background.
 */
void hqm_init(char *key_string, int enable_debug) {
    auto *env = (JNIEnv *) SDL_AndroidGetJNIEnv();
    auto activity = (jobject) SDL_AndroidGetActivity();
    jclass activity_class = env->GetObjectClass(activity);
    jclass hqsdk_class = env->FindClass(HQM_CLASS);

    // initing sdk
    jmethodID sdk_init_method = env->GetStaticMethodID(hqsdk_class, "init",
                                                       "(Landroid/content/Context;Ljava/lang/String;Z)V");
    env->CallStaticVoidMethod(hqsdk_class, sdk_init_method, activity,
                              env->NewStringUTF(key_string),
                              (jboolean) (enable_debug == JNI_TRUE)
    );

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activity_class);
    env->DeleteLocalRef(hqsdk_class);
}

/**
 * Asynchronously schedules installed apps collector.
 * If enable_background_tasks == 0 asynchronously collects installed apps only once per call.
 */
void hqm_start() {
    auto *env = (JNIEnv *) SDL_AndroidGetJNIEnv();
    auto activity = (jobject) SDL_AndroidGetActivity();

    jclass hqsdk_class = env->FindClass(HQM_CLASS);

    // collect installed apps
    jmethodID collect_apps_method = env->GetStaticMethodID(hqsdk_class, "start",
                                                           "(Landroid/content/Context;)V");
    env->CallStaticVoidMethod(hqsdk_class, collect_apps_method, activity);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(hqsdk_class);
}

/**
 * Asynchronously logs custom event.
 *
 *   hqm_log("test", "just a string");
 *   hqm_log("test", "{\"text\": \"sdl_test\", \"event\": \"app_start\"}");
 *
 * @param event_name
 * @param event_data
 */
void hqm_log(char *event_name, char *event_data) {
    auto *env = (JNIEnv *) SDL_AndroidGetJNIEnv();

    jclass hqsdk_class = env->FindClass(HQM_CLASS);

    // log event
    jmethodID log_event_method = env->GetStaticMethodID(hqsdk_class, "logEvent",
                                                        "(Ljava/lang/String;Ljava/lang/String;)V");
    env->CallStaticVoidMethod(hqsdk_class, log_event_method,
                              env->NewStringUTF(event_name),
                              env->NewStringUTF(event_data));

    env->DeleteLocalRef(hqsdk_class);
}

/**
 * Synchronously retrieves predicted user groups.
 *
 *   // Get user groups.
 *   UserGroupData hqm_group_data = hqm_get_user_groups();
 *
 *   if(hqm_group_data.userGroups != NULL) {
 *      // Iterate over user groups.
 *          for (int i = 0; i < hqm_group_data.length; i++, hqm_group_data.userGroups++) {
 *              __android_log_print(ANDROID_LOG_ERROR, "HQM",
 *                                  "id:\"%s\" name:\"%s\"",
 *                                  hqm_group_data.userGroups->id,
 *                                  hqm_group_data.userGroups->name);
 *      }
 *   }
 *
 * @return - a @UserGroupData struct representing a UserGroup list.
 */
UserGroupData hqm_get_user_groups() {
    auto *env = (JNIEnv *) SDL_AndroidGetJNIEnv();

    jclass hqsdk_class = env->FindClass(HQM_CLASS);
    jclass list_class = env->FindClass(LIST_CLASS);
    jclass gr_class = env->FindClass(GROUP_RESPONSE_CLASS);
    auto activity = (jobject) SDL_AndroidGetActivity();

    // get user groups
    jmethodID user_groups_method = env->GetStaticMethodID(hqsdk_class,
                                                          "getUserGroupsSync",
                                                          "(Landroid/content/Context;)Ljava/util/List;");
    jobject groups = env->CallStaticObjectMethod(hqsdk_class, user_groups_method, activity);

    if (groups == nullptr) return (UserGroupData) {.length = 0, .userGroups = nullptr};


    // Get "java.util.List.get(int location)" MethodID
    jmethodID getMethodID = env->GetMethodID(list_class, "get", "(I)Ljava/lang/Object;");

    // Get "int java.util.List.size()" MethodID
    jmethodID sizeMethodID = env->GetMethodID(list_class, "size", "()I");

    // Call "int java.util.List.size()" method and get count of items in the list.
    int listItemsCount = (int) env->CallIntMethod(groups, sizeMethodID);

    // Get segment_id field
    jfieldID segIdFieldID = env->GetFieldID(gr_class, "segment_id", "Ljava/lang/String;");

    // Get segment_name field
    jfieldID segNameFieldID = env->GetFieldID(gr_class, "segment_name",
                                              "Ljava/lang/String;");

    UserGroup *userGroups = static_cast<UserGroup *>(malloc(listItemsCount * sizeof *userGroups));
    for (int i = 0; i < listItemsCount; i++) {
        // get UserGroup item by index
        jobject groupObject = env->CallObjectMethod(groups, getMethodID, i);

        if (groupObject != nullptr) {
            jobject idObject = env->GetObjectField(groupObject, segIdFieldID);
            jobject nameObject = env->GetObjectField(groupObject, segNameFieldID);

            // extract data from java GroupResponse object
            const char *strId = _getString(env, idObject);
            const char *strName = _getString(env, nameObject);

            userGroups[i].id = (char *) malloc(strlen(strId) + 1);
            strncpy(userGroups[i].id, strId, strlen(strId));

            userGroups[i].name = (char *) malloc(strlen(strName) + 1);
            strncpy(userGroups[i].name, strName, strlen(strName));

            env->DeleteLocalRef(groupObject);
            env->DeleteLocalRef(idObject);
            env->DeleteLocalRef(nameObject);
        } else {
            userGroups[i].id = nullptr;
            userGroups[i].name = nullptr;
        }
    }

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(hqsdk_class);
    env->DeleteLocalRef(list_class);
    env->DeleteLocalRef(gr_class);
    env->DeleteLocalRef(groups);

    UserGroupData userGroupData = {.length = listItemsCount, .userGroups = userGroups};
    return userGroupData;
}

/**
 * Request user data.
 */
void hqm_request_user_data(char *email) {
    auto *env = (JNIEnv *) SDL_AndroidGetJNIEnv();

    jclass hqsdk_class = env->FindClass(HQM_CLASS);

    // request user data
    jmethodID request_user_data_method = env->GetStaticMethodID(hqsdk_class, "requestUserData",
                                                                "(Ljava/lang/String;)V");
    env->CallStaticVoidMethod(hqsdk_class, request_user_data_method,
                              env->NewStringUTF(email));

    env->DeleteLocalRef(hqsdk_class);
}

/**
 * Delete user data.
 */
void hqm_delete_user_data() {
    auto *env = (JNIEnv *) SDL_AndroidGetJNIEnv();

    jclass hqsdk_class = env->FindClass(HQM_CLASS);

    // request user data
    jmethodID delete_user_data_method = env->GetStaticMethodID(hqsdk_class, "deleteUserData", "()V");
    env->CallStaticVoidMethod(hqsdk_class, delete_user_data_method);

    env->DeleteLocalRef(hqsdk_class);
}

/**
 * Get user id.
 */
std::string hqm_get_uuid() {
    auto *env = (JNIEnv *) SDL_AndroidGetJNIEnv();

    jclass hqsdk_class = env->FindClass(HQM_CLASS);

    // request user data
    jmethodID get_uuid_method = env->GetStaticMethodID(hqsdk_class, "getUuid", "()Ljava/lang/String;");

    jstring rv = (jstring) env->CallStaticObjectMethod(hqsdk_class, get_uuid_method);

    if (rv == nullptr)
        return "";

    std::string data = env->GetStringUTFChars( rv, 0);

    env->DeleteLocalRef(hqsdk_class);

    return data;
}