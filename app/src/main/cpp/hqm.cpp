#include "SDL.h"
#include <jni.h>
#include "hqm.h"

#define HQM_CLASS               "io/humanteq/hqsdk_core_legacy/HQSdk"
#define LIST_CLASS              "java/util/ArrayList"
#define GROUP_RESPONSE_CLASS    "io/humanteq/hqsdk_core_legacy/models/GroupResponse"

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
void hqm_init(char *key_string, int enable_debug, int enable_background_tasks) {
    JNIEnv *env = (JNIEnv *) SDL_AndroidGetJNIEnv();
    jobject activity = (jobject) SDL_AndroidGetActivity();
    jclass activity_class = env->GetObjectClass(activity);
    jclass hqsdk_class = env->FindClass(HQM_CLASS);

    // enabling debug
    jmethodID hqsdk_enable_debug = env->GetStaticMethodID(hqsdk_class, "enableDebug",
                                                             "(Z)V");
    env->CallStaticVoidMethod(hqsdk_class, hqsdk_enable_debug,
                                 (jboolean) (enable_debug == JNI_TRUE));

    // initing sdk
    jmethodID sdk_init_method = env->GetStaticMethodID(hqsdk_class, "init",
                                                          "(Landroid/content/Context;Ljava/lang/String;ZZ)V");
    env->CallStaticVoidMethod(hqsdk_class, sdk_init_method, activity,
                                 env->NewStringUTF(key_string),
                                 (jboolean) (enable_background_tasks == JNI_FALSE),
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
void hqm_collect_apps() {
    JNIEnv *env = (JNIEnv *) SDL_AndroidGetJNIEnv();
    jobject activity = (jobject) SDL_AndroidGetActivity();
    jclass activity_class = env->GetObjectClass(activity);

    jclass hqsdk_class = env->FindClass(HQM_CLASS);

    // collect installed apps
    jmethodID collect_apps_method = env->GetStaticMethodID(hqsdk_class, "collectApps",
                                                              "(Landroid/content/Context;)V");
    env->CallStaticVoidMethod(hqsdk_class, collect_apps_method, activity);

    env->DeleteLocalRef(activity);
    env->DeleteLocalRef(activity_class);
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
    JNIEnv *env = (JNIEnv *) SDL_AndroidGetJNIEnv();

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
    JNIEnv *env = (JNIEnv *) SDL_AndroidGetJNIEnv();

    jclass hqsdk_class = env->FindClass(HQM_CLASS);
    jclass list_class = env->FindClass(LIST_CLASS);
    jclass gr_class = env->FindClass(GROUP_RESPONSE_CLASS);

    // get user groups
    jmethodID user_groups_method = env->GetStaticMethodID(hqsdk_class,
                                                             "getUserGroupsSync",
                                                             "()Ljava/util/List;");
    jobject groups = env->CallStaticObjectMethod(hqsdk_class, user_groups_method);

    if(groups == NULL) return (struct _UserGroupData) {.length = 0, .userGroups = NULL};


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

    UserGroup userGroups[listItemsCount];

    for (int i = 0; i < listItemsCount; i++) {
        // get UserGroup item by index
        jobject groupObject = env->CallObjectMethod(groups, getMethodID, i);

        if (groupObject != NULL) {
            jobject idObject = env->GetObjectField(groupObject, segIdFieldID);
            jobject nameObject = env->GetObjectField(groupObject, segNameFieldID);

            // extract data from java GroupResponse object
            const char *strId = _getString(env, idObject);
            const char *strName = _getString(env, nameObject);

            userGroups[i] = (struct _UserGroup) {strId, strName};

            env->DeleteLocalRef(groupObject);
            env->DeleteLocalRef(idObject);
            env->DeleteLocalRef(nameObject);
        } else {
            userGroups[i] = (struct _UserGroup) {"\0", "\0"};
        }
    }

    env->DeleteLocalRef(hqsdk_class);
    env->DeleteLocalRef(list_class);
    env->DeleteLocalRef(gr_class);
    env->DeleteLocalRef(groups);

    UserGroupData userGroupData = {.length = listItemsCount, .userGroups = userGroups};
    return userGroupData;
}