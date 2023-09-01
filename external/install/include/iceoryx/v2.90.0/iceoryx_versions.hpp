#ifndef __ICEORYXVERSIONS__
#define __ICEORYXVERSIONS__

#define ICEORYX_VERSION_MAJOR    2
#define ICEORYX_VERSION_MINOR    90
#define ICEORYX_VERSION_PATCH    0
#define ICEORYX_VERSION_TWEAK    0

#define ICEORYX_LATEST_RELEASE_VERSION    "2.90.0"
#define ICEORYX_BUILDDATE                 "2023-08-17T09:32:35Z"
#define ICEORYX_SHA1                      "414784e169ba58666a9b4c0be087ea3e043ae839"

#include "iox/logging.hpp"

#define ICEORYX_PRINT_BUILDINFO()     IOX_LOG(INFO) << "Built: " << ICEORYX_BUILDDATE;


#endif
