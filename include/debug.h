#ifndef DEBUG_h
#define DEBUG_h

#include <Arduino.h>

#define DEBUG_LEVEL_NONE 0
#define DEBUG_LEVEL_ERROR 1
#define DEBUG_LEVEL_WARNING 2
#define DEBUG_LEVEL_INFO 3
#define DEBUG_LEVEL_DEBUG 4
#define DEBUG_LEVEL_DEBUG2 5

class Debug {
  private:
    int speed;
    int debugLevel;

  public:
    Debug();
    void start(int speed, int debugLevel);

    void print(int debugLevel, const __FlashStringHelper *ifsh);
    void print(int debugLevel, const String &s);
    void print(int debugLevel, const char str[]);
    void print(int debugLevel, char c);
    void print(int debugLevel, unsigned char b, int base);
    void print(int debugLevel, int num);
    void print(int debugLevel, int n, int base);
    void print(int debugLevel, unsigned int n, int base);
    void print(int debugLevel, long n, int base);
    void print(int debugLevel, unsigned long n, int base);
    void print(int debugLevel, long long n, int base);
    void print(int debugLevel, unsigned long long n, int base);
    void print(int debugLevel, double n, int digits);    
    #ifdef ESP32
      void print(int debugLevel, const Printable& x);
      void print(int debugLevel, struct tm * timeinfo, const char * format);
    #endif

    void println(int debugLevel, const __FlashStringHelper *ifsh);
    void println(int debugLevel, const String &s);
    void println(int debugLevel, const char c[]);
    void println(int debugLevel, char c);
    void println(int debugLevel, int num);
    void println(int debugLevel, unsigned char b, int base);
    void println(int debugLevel, int num, int base);
    void println(int debugLevel, unsigned int num, int base);
    void println(int debugLevel, long num, int base);
    void println(int debugLevel, unsigned long num, int base);
    void println(int debugLevel, long long num, int base);
    void println(int debugLevel, unsigned long long num, int base);
    void println(int debugLevel, double num, int digits);
    
    #ifdef ESP32
      void println(int debugLevel, const Printable& x);
      void println(int debugLevel, struct tm * timeinfo, const char * format);
    #endif

};

#endif
