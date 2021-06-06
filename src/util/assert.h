#ifndef SNPSERVER_ASSERT_H
#define SNPSERVER_ASSERT_H

#define ASSERT(cond)    \
	if (!(cond)) {      \
        result = false; \
		fprintf(stderr, "ERROR @ %s:%d: (%s) failed", __FILE__, __LINE__, #cond); \
		goto error; \
	}

#endif //SNPSERVER_ASSERT_H
