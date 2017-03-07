#ifndef __CPPSTREAM_H__
#define __CPPSTREAM_H__

// Write by Kingxf


#include <Arduino.h>
#include <utility>

enum endline_type { endl };
enum flush_type   { flush_it };

template <typename T> static inline Print& operator<<(Print& stream, T&& params) {
  stream.print(std::forward<T>(params));
  return stream;
}

static inline Print& operator<<(Print& stream, const String& str) {
  stream.print(str.c_str());
  return stream;
}

static inline Print& operator<<(Print& stream, endline_type) {
  stream.println();
  return stream;
}

static inline Print& operator<<(Print& stream, flush_type) {
  static_cast<Stream&>(stream).flush();
  return stream;
}



#endif

