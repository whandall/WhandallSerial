#ifndef _WHANDALLSERIAL_H_
#define _WHANDALLSERIAL_H_
#include <Arduino.h>

/*! \mainpage WhandallSerial
 *
 * \section intro_sec Einführung
 *
 * Die Verarbeitung seriellen Inputs bereitet vielen Arduino Anfängern Probleme.
 *
 * Die niedrige Geschwindigkeit mit der die Zeichen eintreffen und das Single-Thread Umfeld des Arduinos
 * erfordert eine für viele ungewohnte nicht-blockierende Programmierweise um nicht den gesamten Sketch zu blockieren.
 *
 * Serielle Kommunikation wird so häufig benötigt und oftmals auch auf mehreren Schnittstellen gleichzeitig,
 * daß sich die Erstellung einer Klasse in Form einer Arduino Library regelrecht aufdrängt.
 * 
 *
 * \section hist_sec History
 *
 * - 2017 Erweiterung auf Nextion Format und damit binäre Daten
 * - 2017 erste Version CR/LF - ASCII
 *
 * \section install_sec Installation
 *
 * Standard Arduino Library Installation
 *  
 */
 
 /**
  * Enumeration der verwendeten ASCII Zeichen und der Bits für die Optionen.
  */
enum {
	/// LineFeed
	cbLF = 10,
	/// CarriageReturn
	cbCR = 13,
	/// Nextion Delimiter, 3 hintereinander
	cbNX = 0xFF,
	
	/// LineFeed ignorieren
	optIgnoreLF = 0x40,
	/// Leerzeichen am Zeilenanfang unterdrücken
	optSkipWS = 0x20,
	/// Aktion auch für Leerzeilen ausführen
	optEmptyToo = 0x10,
	/// Nextion Format (Delimiter: 3 x 0xFF) verwenden
	optTripleFF = 0x08,
	/// Delimiter im Puffer belassen (also mit übergeben)
	optKeepDlm = 0x04,
	/// Debug Ausgaben auf Serial
	optDebug = 0x02,
	/// detailiertere Debug Ausgaben aus Serial
	optDebugDetail = 0x01,
};

 /**
  * Typ der Funktion, die nach Empfang einer vollständigen Zeile aufgerufen wird
  */
typedef void (*lineProcess)(const char* iLine);

 /**
  * Hier sind die Variablen und Funktionen der seriellen Anbindung versammelt.
	*	
	*	Im Konstruktor wird der zu lesende Stream und die Verarbeitungsroutine definiert.
	* - SSerial blue(Serial1, blueHandler);
	*
	*	In der Funktion *begin()* wird die maximale Nachrichtenlänge und eventuelle Optionen festgelegt.
  * - blue.begin(40, optEmptyToo + optIgnoreLF);
  * 
	*	Intern wird mit einem in begin() dynamisch angelegten Puffer gearbeitet,
	* der zwei Bytes länger ist als die angeforderte Größe.
	* Ein Byte wird vom abschließenden '\0' belegt (bei maximaler netto Datenlänge),
	*	das andere befindet sich vor dem Puffer und trägt die aktuelle Länge
  *	um einfach Daten mit eingebetteten '\0' bearbeiten zu können.
	* Die schon zur Verfügung stehende Länge könnte natürlich auch im Normalfall nützlich sein.
	*
	* Innerhalb der globalen loop() Funktion muss die Funktion loop() des Objekts aufgerufen werden
  *	um das Pollen der Schnittstelle zu ermöglichen.
	* Aus dem Kontext dieser Funktion wird die Verarbeitungsroutine aufgerufen, sobald eine Zeile vollständig empfangen wurde.
	* - blue.loop();
	*	
  */
class SSerial {
	// erleichtert eine Ableitung ohne diese Library ändern zu müssen 
protected:
	/// eine Referenz auf das Stream Objekt das gepollt werden soll
	Stream& serial;
	/// alle Betriebs-Optionen in einer Variablen vereinigt 
	uint8_t options;
	/// Maximallänge einer Nachricht, die Länge des allocierten Puffers ist um zwei größer
	uint8_t bSize;
	/// Index der Stelle im Puffer, an der das nächste Zeichen abgelegt werden soll
	uint8_t bIndex;
	/// Zeiger auf den Benutzerpuffer, also ein Byte hinter der Länge
	uint8_t* buffer;
	/// Zeiger auf die Verarbeitungsroutine
	lineProcess	handler;
public:
	/// Konstruktor, setzt nur die Schnittstelle und die Verarbeitungsroutine
	SSerial(Stream& inSer, lineProcess iHandler) : serial(inSer), handler(iHandler) {}
	/// Destruktor, gibt den allocierten Speicher zurück, dazu muss der Zeiger wieder korrigiert werden.
	~SSerial() { delete [] --buffer; }
	/// Setup Funktion des Objekts, definiert maximale Nachrichtenlänge und eventuelle Optionen 
	void begin(uint8_t maxMessage, uint8_t inOpts = optIgnoreLF);
	/// verarbeite eventuelle Eingaben auf serial, wenn eine Nachricht vollständig ist, rufe handler auf
	void loop();
	/// Zugriff auf den internen Puffer
	uint8_t* getBuffer() { return buffer; }
	/// Position des nächsten empfangenen Zeichens, also hinter den empfangenen Bereich, gleichzeitig Länge des bisher eingegangen Teils der Nachrich
	uint8_t getIndex() { return bIndex; }
	/// Maximalgröße einer Nachricht
	uint8_t getSize() { return bSize; }
	/// Hilfsfunktion zur Ausgabe eines Bytes in HEX
	void static blankAndHex(uint8_t inB);
};

#endif