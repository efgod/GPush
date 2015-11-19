#include "client_log.h"

#ifdef TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif


namespace gim {

#if (defined TARGET_OS_IPHONE) || (defined TARGET_IPHONE_SIMULATOR)  
    void logprint(LogLevel level, const char* logbuf){
        return;
    }
#endif

}
