#include <Arduino.h>
#include "debug.h"

Debug::Debug() {   

};

void Debug::start(int speed, int debugLevel) {
    this->speed = speed;
    this->debugLevel = debugLevel;
    Serial.begin(speed);
}

void Debug::print(int debugLevel, const __FlashStringHelper *ifsh) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(ifsh);
    }
}

void Debug::print(int debugLevel, const String &s) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(s);
    }
}

void Debug::print(int debugLevel, const char str[]) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(str);
    }
}

void Debug::print(int debugLevel, char c) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(c);
    }
}

void Debug::print(int debugLevel, unsigned char b, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(b, base);
    }
}

void Debug::print(int debugLevel, int n, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(n, base);
    }
}

void Debug::print(int debugLevel, int num) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(num);
    }
}

void Debug::print(int debugLevel, unsigned int n, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(n, base);
    }
}

void Debug::print(int debugLevel, long n, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(n, base);
    }
}

void Debug::print(int debugLevel, unsigned long n, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(n, base);
    }
}

void Debug::print(int debugLevel, long long n, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(n, base);
    }
}

void Debug::print(int debugLevel, unsigned long long n, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(n, base);
    }
}

void Debug::print(int debugLevel, double n, int digits) {
   if (debugLevel <= this->debugLevel) {
        Serial.print(n, digits);
    }
}

void Debug::print(int debugLevel, const Printable& x) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(x);
    }
}

void Debug::print(int debugLevel, struct tm * timeinfo, const char * format) {
    if (debugLevel <= this->debugLevel) {
        Serial.print(timeinfo, format);
    }
}

void Debug::println(int debugLevel, const __FlashStringHelper *ifsh) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(ifsh);
    }
}

void Debug::println(int debugLevel, const String &s) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(s);
    }
}

void Debug::println(int debugLevel, const char c[]) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(c);
    }
}

void Debug::println(int debugLevel, char c) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(c);
    }
}

void Debug::println(int debugLevel, int num) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(num);
    }
}

void Debug::println(int debugLevel, unsigned char b, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(b, base);
    }
}

void Debug::println(int debugLevel, int num, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(num, base);
    }
}

void Debug::println(int debugLevel, unsigned int num, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(num, base);
    }
}

void Debug::println(int debugLevel, long num, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(num, base);
    }
}

void Debug::println(int debugLevel, unsigned long num, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(num, base);
    }
}

void Debug::println(int debugLevel, long long num, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(num, base);
    }
}

void Debug::println(int debugLevel, unsigned long long num, int base) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(num, base);
    }
}

void Debug::println(int debugLevel, double num, int digits) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(num, digits);
    }
}

void Debug::println(int debugLevel, const Printable& x) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(x);
    }
}

void Debug::println(int debugLevel, struct tm * timeinfo, const char * format) {
    if (debugLevel <= this->debugLevel) {
        Serial.println(timeinfo, format);
    }
}
