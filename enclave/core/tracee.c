// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

#include <openenclave/enclave.h>
#include <openenclave/internal/trace.h>

OE_WEAK bool oe_is_logging_available = false;

OE_WEAK oe_result_t oe_log(oe_log_level_t level, const char* fmt, ...)
{
    OE_UNUSED(level);
    OE_UNUSED(fmt);

    // Is oe_abort better?
    oe_abort();
    return OE_FAILURE;
}

OE_WEAK oe_log_level_t oe_get_current_logging_level(void)
{
    return OE_LOG_LEVEL_NONE;
}
