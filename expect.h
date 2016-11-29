#include <assert.h>

#ifndef _EXPECT_H_
#define _EXPECT_H_

#define expect(EXPECTED_CONDITION, EXPECTED_ERR_MESSAGE) assert(EXPECTED_CONDITION && EXPECTED_ERR_MESSAGE)

#endif
