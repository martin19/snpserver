#ifndef SNPSERVER_ASSERT_H
#define SNPSERVER_ASSERT_H

#include "loguru.h"

#define ASSERT(cond)    \
	if (!(cond)) {      \
        result = false; \
		LOG_F(ERROR, "ERROR (%s) failed", #cond); \
		goto error; \
	}

#endif //SNPSERVER_ASSERT_H
