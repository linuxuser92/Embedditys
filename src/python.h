#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Runs Python code in a restricted interpreter namespace.
 * Returns 0 on success and nonzero on failure. The caller owns output/error
 * buffers and must release them with python_free_buffer.
 */
int python_run_sandboxed(const char* source, char** output, char** error);
void python_free_buffer(char* buffer);

#ifdef __cplusplus
}
#endif
