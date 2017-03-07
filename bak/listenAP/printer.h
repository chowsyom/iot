#ifndef __PRINTER_H__
#define __PRINTER_H__

// Write by Kingxf

#include <Arduino.h>

class DataPrinter : public Printable {
public:
  inline DataPrinter(const char* str) : data((uint8_t*)str), len(strlen(str)) {}
  inline DataPrinter(const String& str) : data((uint8_t*)str.c_str()), len(str.length()) {}
  inline DataPrinter(const uint8_t* data, size_t len) : data(data), len(len) {}
  virtual size_t printTo(Print& p) const {
    p.write(data, len);
    return len;
  }
private:
  const uint8_t* data;
  size_t len;
};

class HexConvertPrinter : public Printable {
  class InteralStream : public Print {
  public:
    inline InteralStream(Print &rawstream, bool upper, const char* delimiter) : 
        rawstream(rawstream), 
        letter_a(upper ? 'A' : 'a'), 
        delimiter(delimiter), 
        first(true) 
    { }
    virtual size_t write(uint8_t data) {
      if (!first)
        rawstream.print(delimiter);
      else
        first = false;
      rawstream.print(chr(data >> 4));
      rawstream.print(chr(data & 0xf));
      return 2;
    }
  private:
    bool first;
    Print& rawstream;
    char letter_a;
    const char* delimiter;
    inline char chr( uint8_t x ) const { return x <= 9 ? x + '0' : x - 10 + letter_a; }
  };
public:
  inline HexConvertPrinter(const Printable& rawprinter, bool upper = false, const char* delimiter = "") : rawprinter(rawprinter), upper(upper), delimiter(delimiter) { }
  virtual size_t printTo(Print& rawstream) const {
    InteralStream is(rawstream, upper, delimiter);
    return is.print(rawprinter);
  }
private:
  const char* delimiter;
  const Printable& rawprinter;
  bool upper;
};

inline DataPrinter        Printer(const char* str) { return DataPrinter(str); }
inline DataPrinter        Printer(const String& str) { return DataPrinter(str); }
inline DataPrinter        Printer(const uint8_t* data, size_t len) { return DataPrinter(data, len); }
inline HexConvertPrinter  HexPrinter(const Printable& rawprinter, bool upper = false, const char* delimiter = "") { return HexConvertPrinter(rawprinter, upper, delimiter); }

#endif
