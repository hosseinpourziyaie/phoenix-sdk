#include <std_include.hpp>
#include "pattern.hpp"

namespace utils::pattern
{
    char* util_trim_pattern_str(const char* str)
    {
        if (str)
        {
            size_t len = strlen(str);
            if (len)
            {
                char* start = (char*)str;
                char* end = (start + len) - 1;

                // move start ptr up
                for (size_t i = 0; i < len; ++i)
                {
                    if (isspace(*start) || *start == '?')
                        ++start;
                }

                // move end ptr down
                for (size_t i = len; len--; )
                {
                    if (isspace(*end) || *end == '?')
                        --end;
                }

                // add null terminator
                *(end + 1) = '\0';

                // shift string bytes, this must be done in case the string is dynamically allocated and the original ptr isn't changed
                size_t new_len = strlen(start);
                if (new_len)
                {
                    return (char*)memmove((char*)str, start, (new_len * sizeof(char)) + 1);
                }
            }
        }

        return 0;
    }

    PS_PatternByte* ps_make_pattern_byte(uint8_t value, bool is_wildcard)
    {
        PS_PatternByte* byte = (PS_PatternByte*)(malloc(sizeof(PS_PatternByte)));
        if (byte)
        {
            byte->m_value = value;
            byte->m_is_wildcard = is_wildcard;

            return byte;
        }

        return 0;
    }

    bool ps_add_pattern_byte(PS_Pattern* pattern, PS_PatternByte* byte)
    {
        if (pattern && byte)
        {
            // save off last amount and increment total
            size_t amount = pattern->m_amount++;

            // make more room if needed and add a new byte
            PS_PatternByte** bytes = (PS_PatternByte**)(realloc(pattern->m_bytes, sizeof(PS_PatternByte*) * ((size_t)amount + 1)));
            if (bytes)
            {
                pattern->m_bytes = bytes;
                pattern->m_bytes[amount] = byte;

                return true;
            }
        }

        return false;
    }

    void ps_free_pattern(PS_Pattern* pattern)
    {
        if (pattern)
        {
            // free each byte
            for (size_t i = 0; i < pattern->m_amount; ++i)
            {
                free(pattern->m_bytes[i]);
            }

            free(pattern->m_bytes);
        }
    }

    bool ps_build_idastyle(PS_Pattern* out_pattern, const char* pattern)
    {
        if (out_pattern && pattern && strlen(pattern))
        {
            // make a copy of the input string
            char* str = strdup(pattern);

            // clean up the input string
            str = util_trim_pattern_str(str);
            if (str)
            {
                bool       valid = true;
                PS_Pattern out = { 0 };

                // iteratively split the input string by space
                for (
                    char* token = strtok(str, " ");
                    token;
                    token = strtok(0, " ")
                    )
                {
                    // make sure tokens aren't too long
                    size_t token_len = strlen(token);
                    if (!token_len || token_len > 2)
                    {
                        ps_free_pattern(&out);
                        valid = false;

                        break;
                    }

                    PS_PatternByte* byte;

                    if (token[0] == L'?') // wildcard
                    {
                        // add a wildcard byte to array
                        byte = ps_make_pattern_byte(0, true);
                        if (!byte)
                        {
                            ps_free_pattern(&out);
                            valid = false;

                            break;
                        }

                        if (!ps_add_pattern_byte(&out, byte))
                        {
                            ps_free_pattern(&out);
                            valid = false;

                            break;
                        }
                    }
                    else // probably a byte
                    {
                        // ensure the current token is a valid byte (should only be 1 or 2 chars and valid hex digit(s))
                        bool is_valid_byte = false;

                        if (token_len == 1 && isxdigit(token[0])) // 1-char
                            is_valid_byte = true;
                        else if (token_len == 2 && isxdigit(token[0]) && isxdigit(token[1])) // 2-char
                            is_valid_byte = true;

                        if (!is_valid_byte)
                        {
                            ps_free_pattern(&out);
                            valid = false;

                            break;
                        }

                        // attempt to convert the string to a base 16 int and make sure it's in bounds of a uint8_t
                        ulong_t val = strtoul(token, 0, 16);
                        if (!val || val == ULONG_MAX || val > UINT8_MAX)
                        {
                            ps_free_pattern(&out);
                            valid = false;

                            break;
                        }

                        // add normal byte to array
                        byte = ps_make_pattern_byte((uint8_t)val, false);
                        if (!byte)
                        {
                            ps_free_pattern(&out);
                            valid = false;

                            break;
                        }

                        if (!ps_add_pattern_byte(&out, byte))
                        {
                            ps_free_pattern(&out);
                            valid = false;

                            break;
                        }
                    }
                }

                free(str);

                if (valid)
                {
                    *out_pattern = out;
                    return true;
                }
            }
        }

        return false;
    }

    bool ps_build_codestyle(PS_Pattern* out_pattern, const char* pattern, const char* mask)
    {
        if (out_pattern && pattern && mask)
        {

        }

        return false;
    }

    uintptr_t ps_find(PS_Pattern* pattern, uintptr_t start, size_t size)
    {
        if (pattern && pattern->m_amount && pattern->m_bytes && start && size)
        {
            uint8_t* scan_start = (uint8_t*)start;
            uint8_t* scan_end = (uint8_t*)(start + size);
            size_t    matched = 0;
            uintptr_t found = 0;

            // iterate over the range of memory
            for (uint8_t* i = scan_start; i < scan_end; ++i)
            {
                PS_PatternByte* byte = pattern->m_bytes[matched];

                if (byte->m_value == *i || byte->m_is_wildcard) // compare byte value. also, wildcards are always a "match"
                {
                    // first match is the "found" location
                    if (!matched)
                        found = (uintptr_t)i;

                    ++matched;
                }
                else // didn't match
                {
                    matched = 0;
                    found = 0;
                }

                // if all matched then return the address to the first match
                if (matched >= pattern->m_amount)
                    return found;
            }
        }

        return 0;
    }

    uintptr_t ps_find_idastyle(const char* pattern, uintptr_t start, size_t size)
    {
        PS_Pattern ptrn;

        if (ps_build_idastyle(&ptrn, pattern))
        {
            return ps_find(&ptrn, start, size);
        }

        return 0;
    }

    uintptr_t ps_find_codestyle(const char* pattern, const char* mask, uintptr_t start, size_t size)
    {
        PS_Pattern ptrn;

        if (ps_build_codestyle(&ptrn, pattern, mask))
        {
            return ps_find(&ptrn, start, size);
        }

        return 0;
    }
}

/* https://github.com/angelfor3v3r/c-pattern-scan */