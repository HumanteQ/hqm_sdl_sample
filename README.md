[ ![Download](https://api.bintray.com/packages/humanteq/hqm-sdk/hqm-core-legacy/images/download.svg) ](https://bintray.com/humanteq/hqm-sdk/hqm-core-legacy/_latestVersion)

## HQMonitor SDL Sample App.

#### Integration instructions:

 1. Add [hqm.h](https://raw.githubusercontent.com/HumanteQ/hqm_sdl_sample/master/app/src/main/cpp/hqm.h) and [hqm.cpp](https://raw.githubusercontent.com/HumanteQ/hqm_sdl_sample/master/app/src/main/cpp/hqm.cpp) into your project.
 2. Add HQM repo into `/your_project/build.gradle`:
```clojure

    allprojects {
        repositories {
            google()
            jcenter()
            ...
    
            maven { url 'https://dl.bintray.com/humanteq/hqm-sdk/' }
    
            ...
        }
    }
```
 3. Add HQM dependency into `/your_project/app/build.gradle`:
```clojure

    dependencies {
        implementation fileTree(include: ['*.jar'], dir: 'libs')
        implementation "androidx.appcompat:appcompat:$appcompat_version" 
        ...
    
        implementation 'io.humanteq.hqm:hqm-core-legacy:2.0.0'
    
        ...
    }
```
 4. Initialize SDK:
```c
	#include "hqm.h"

            ...
            
            // Init SDK
            hqm_init(
                    "your_key", // sdk key
                    1);         // enable debug mode
  ```
  
 5. Start SDK: ( After obtaining user consent )
```c  
            // Start SDK
            hqm_start();
  ```
  
 6. Send user-defined event:
```c  
            // Send event as text ...
            hqm_log("test_event", "just a string");
            hqm_log("test_event", "{\"text\": \"sdl_app_startup\", \"event\": \"app_start\"}");
```

7. Request predicted user groups: (HQSdk will need some time, typically 10 - 15 min, to compute user groups)
```c
            // Request predicted user group list
             UserGroupData hqm_group_data = hqm_get_user_groups();
             if (hqm_group_data.userGroups != NULL) {
                 char buf[256] = {0};
                 for (int i = 0; i < hqm_group_data.length; i++, hqm_group_data.userGroups++) {
                     snprintf(buf, sizeof(buf), "id:\"%s\" name:\"%s\"",
                              hqm_group_data.userGroups->id,
                              hqm_group_data.userGroups->name);
                 }
             }
            
            ...
```

Startup script example: [native-lib.cpp](https://github.com/HumanteQ/hqm_sdl_sample/blob/master/app/src/main/cpp/native-lib.cpp#L55)


#### GDPR compliance.
To comply with GDPR, we provide following user data management methods:
1. Request for user data. 
A report with current user data will be sent to the provided email.
```c
            hqm_request_user_data("some@email.org");
```

2. User data deletion request. All current user data will be deleted from Humanteq servers.
```c
            hqm_delete_user_data();
```