#pragma once

namespace utils::pattern
{
    typedef struct PS_PatternByte
    {
        uint8_t m_value;
        bool    m_is_wildcard;
    } PS_PatternByte;

    typedef struct PS_Pattern
    {
        size_t         m_amount;  // amount of bytes in array
        PS_PatternByte** m_bytes; // array of bytes
    } PS_Pattern;

    typedef	unsigned long	ulong_t;


    // find pattern by IDA-style pattern
    // example: "AA B CC ? DD" (double question marks and single bytes are supported)
    // returns NULL on failure
    uintptr_t ps_find_idastyle(const char* pattern, uintptr_t start, size_t size);

    // find pattern by code-style pattern
    // example: "\xAA\xBB\xCC\x00\xDD", "xxx?x"
    // returns NULL on failure
    uintptr_t ps_find_codestyle(const char* pattern, const char* mask, uintptr_t start, size_t size);
}