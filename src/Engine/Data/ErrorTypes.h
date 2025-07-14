#pragma once

// Return values from main Tree functions
namespace Nebulite{
enum ERROR_TYPE{
    // Critical Errors first with negative value
    CRITICAL_GENERAL = -1000,
    CRITICAL_CUSTOM_ASSERT,
    CRITICAL_FUNCTION_NOT_IMPLEMENTED,
    CRITICAL_INVALID_FILE,
    CRITICAL_INVALID_ARGC_ARGV_PARSING,
    CRITICAL_FUNCTIONCALL_INVALID,
    // Non-critical errors positive
    NONE = 0,
    CUSTOM_ERROR,   // used for functioncall error
    TOO_MANY_ARGS,
    TOO_FEW_ARGS,
    UNKNOWN_ARG,
    FEATURE_NOT_IMPLEMENTED
};
}
