/*
 * Minimal Arduino-compatible Print / Printable replacement for pure ESP-IDF builds.
 *
 * Arduino.h is not available when ESPHome is built without the Arduino framework.
 * This header provides the subset of Print / Printable that this component uses,
 * implemented with standard C++ and no Arduino dependencies.
 *
 * Drop-in: include this instead of Arduino.h wherever Print / Printable are needed.
 */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>   // strlen

// Keep the same macro names Arduino uses so existing call-sites need no change.
#ifndef HEX
#define HEX 16
#endif
#ifndef DEC
#define DEC 10
#endif

// ---------------------------------------------------------------------------
// Print — abstract base class; subclasses implement write(uint8_t).
// Provides print() overloads for strings and all integer types.
// ---------------------------------------------------------------------------
class Print {
public:
    virtual ~Print() = default;

    // --- pure virtual byte sink ---
    virtual size_t write(uint8_t c) = 0;

    // --- string helpers ---
    size_t write(const char *s) {
        if (!s) return 0;
        size_t n = 0;
        while (*s) n += write((uint8_t)*s++);
        return n;
    }
    size_t print(const char *s) { return write(s); }

    // --- integer helpers ---
    size_t print(uint8_t  v, int base = DEC) { return _num((unsigned long)v, base); }
    size_t print(uint16_t v, int base = DEC) { return _num((unsigned long)v, base); }
    size_t print(uint32_t v, int base = DEC) { return _num((unsigned long)v, base); }
    size_t print(int      v, int base = DEC) {
        if (base == DEC && v < 0) {
            size_t n = write((uint8_t)'-');
            return n + _num((unsigned long)(-v), base);
        }
        return _num((unsigned long)v, base);
    }
    size_t print(unsigned long v, int base = DEC) { return _num(v, base); }
    size_t print(long          v, int base = DEC) { return print((int)v, base); }

private:
    size_t _num(unsigned long value, int base) {
        // Build digits right-to-left in a local buffer.
        char buf[33];
        char *p = buf + sizeof(buf) - 1;
        *p = '\0';
        if (value == 0) {
            *--p = '0';
        } else {
            while (value > 0) {
                int d = (int)(value % (unsigned long)base);
                *--p = (char)(d < 10 ? ('0' + d) : ('a' + d - 10));
                value /= (unsigned long)base;
            }
        }
        return write(p);
    }
};

// ---------------------------------------------------------------------------
// Printable — interface for objects that can serialise themselves to a Print.
// ---------------------------------------------------------------------------
class Printable {
public:
    virtual ~Printable() = default;
    virtual size_t printTo(Print &p) const = 0;
};
