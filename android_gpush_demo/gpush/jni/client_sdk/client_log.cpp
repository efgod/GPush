//
//  client_log.cpp
//  BangBangXing
//
//  Created by 林宁宁 on 15/7/9.
//  Copyright (c) 2015年 蓝海(福建)信息技术有限公司. All rights reserved.
//

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
