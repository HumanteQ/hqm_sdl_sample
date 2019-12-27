#ifndef SDL2_HQM_TEST_HQM_H
#define SDL2_HQM_TEST_HQM_H

#define HQM_CLASS               "io/humanteq/hq_core/HQSdk"
#define LIST_CLASS              "java/util/ArrayList"
#define GROUP_RESPONSE_CLASS    "io/humanteq/hq_core/models/UserGroup"

typedef struct _UserGroup {
    char *id;
    char *name;
} UserGroup;

typedef struct _UserGroupData {
    UserGroup *userGroups;
    const int length;
} UserGroupData;

void hqm_init(char *key_string, int enable_debug);

void hqm_start();

void hqm_log(char *event_name, char *event_data);

UserGroupData hqm_get_user_groups();

void hqm_request_user_data(char * email);

void hqm_delete_user_data();

std::string hqm_get_uuid();


#endif //SDL2_HQM_TEST_HQM_H
