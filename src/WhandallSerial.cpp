#include <WhandallSerial.h>

void SSerial::begin(uint8_t maxMessage, uint8_t inOpts) {
	options = inOpts;
	bSize = maxMessage;
	buffer = new uint8_t[bSize + 2];	// one uint8_t for the trailing '\0', one for the prefix len uint8_t
	if (buffer == NULL) {
		bSize = 0;
	} else {
		buffer++;
	}
	bIndex = 0;
}

void SSerial::blankAndHex(uint8_t inB) {
	Serial.write(' ');
	if (inB < 16) {
		Serial.write('0');
	}
	Serial.print(inB, HEX);
}

void SSerial::loop() {
  bool doCheck = false;

  while (serial.available()) {	// allows usage of continue/break
    uint8_t inChar = serial.read();
		if (options & optDebug) {
			if (options & optDebugDetail) {
				blankAndHex(bIndex);
				Serial.write(':');
			}
			blankAndHex(inChar);
			if (options & optDebugDetail) {
				Serial.println();
			}
		}
    if (inChar == cbLF && options & optIgnoreLF) {
      continue;
    } else if (inChar == cbCR && !(options & optTripleFF)) {
			if (options & optKeepDlm) {
				buffer[bIndex++] = inChar;
			}
      // pass up only if non empty, or empty requested
      doCheck = bIndex || (options & optEmptyToo);
    } else if ((options & optTripleFF) && inChar == 0xFF &&
							(bIndex >= (*buffer==0x71 ? 7 : 2)) &&
							buffer[bIndex-1] == 0xFF && buffer[bIndex-2] == 0xFF) {
			if (options & optKeepDlm) {
				// store in buffer, advancing index
				buffer[bIndex++] = inChar;
			} else {
				// index of first 0xFF
				bIndex -= 2;
			}
      // pass up only if non empty, or empty requested
      doCheck = bIndex || (options & optEmptyToo);
    } else {
      // skip whitespace on line start
      if ((bIndex == 0) && isWhitespace(inChar) && (options & optSkipWS)) {
        continue;
      }
      // store in buffer, advancing index
      buffer[bIndex++] = inChar;
      // passup if filled
      doCheck = (bIndex == bSize);
    }
    if (doCheck) {
      buffer[bIndex] = 0;	// make buffer a string
			if ((options & optDebug) && ~(options & optDebugDetail)) {
				Serial.println();
			}
			buffer[-1] = bIndex;
      (*handler)((const char*)buffer);	// pass to handler
      bIndex = 0;			// empty buffer
    }
  }
}