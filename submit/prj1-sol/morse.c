#include "morse.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

typedef struct {
  char c;
  const char *code;
} TextMorse;

//<https://en.wikipedia.org/wiki/Morse_code#/media/File:International_Morse_Code.svg>
static const TextMorse charCodes[] = {
  { 'A', ".-" },
  { 'B', "-..." },
  { 'C', "_._." },
  { 'D', "-.." },
  { 'E', "." },
  { 'F', "..-." },
  { 'G', "--." },
  { 'H', "...." },
  { 'I', ".." },
  { 'J', ".---" },
  { 'K', "-.-" },
  { 'L', ".-.." },
  { 'M', "--" },
  { 'N', "-." },
  { 'O', "---" },
  { 'P', ".--." },
  { 'Q', "--.-" },
  { 'R', ".-." },
  { 'S', "..." },
  { 'T', "-" },
  { 'U', "..-" },
  { 'V', "...-" },
  { 'W', ".--" },
  { 'X', "-..-" },
  { 'Y', "-.--" },
  { 'Z', "--.." },

  { '1', ".----" },
  { '2', "..---" },
  { '3', "...--" },
  { '4', "....-" },
  { '5', "....." },
  { '6', "-...." },
  { '7', "--..." },
  { '8', "---.." },
  { '9', "----." },
  { '0', "-----" },

  { '\0', ".-.-." }, //AR Prosign indicating End-of-message
                     //<https://en.wikipedia.org/wiki/Prosigns_for_Morse_code>
};


/** Return NUL-terminated code string for char c. Returns NULL if
 *  there is no code for c.
 */
static const char *
charToCode(Byte c) {
  for (int i = 0; i < sizeof(charCodes)/sizeof(charCodes[0]); i++) {
    if (charCodes[i].c == c) return charCodes[i].code;
  }
  return NULL;
}


/** Return char for code. Returns < 0 if code is invalid.
 */
static int
codeToChar(const char *code) {
  for (int i = 0; i < sizeof(charCodes)/sizeof(charCodes[0]); i++) {
    if (strcmp(charCodes[i].code, code) == 0) return charCodes[i].c;
  }
  return -1;
}

/** Given an array of Bytes, a bit index is the offset of a bit
 *  in the array (with MSB having offset 0).
 *
 *  Given a bytes[] array and some bitOffset, and assuming that
 *  BITS_PER_BYTE is 8, then (bitOffset >> 3) represents the index of
 *  the byte within bytes[] and (bitOffset & 0x7) gives the bit-index
 *  within the byte (MSB represented by bit-index 0) and .
 *
 *  For example, given array a[] = {0xB1, 0xC7} which is
 *  { 0b1011_0001, 0b1100_0111 } we have the following:
 *
 *     Bit-Offset   Value
 *        0           1
 *        1           0
 *        2           1
 *        3           1
 *        4           0
 *        5           0
 *        6           0
 *        7           1
 *        8           1
 *        9           1
 *       10           0
 *       11           0
 *       12           0
 *       13           1
 *       14           1
 *       15           1
 *
 */


/** Return mask for a Byte with bit at bitIndex set to 1, all other
 *  bits set to 0.  Note that bitIndex == 0 represents the MSB,
 *  bitIndex == 1 represents the next significant bit and so on.
 */
static inline unsigned
byteBitMask(unsigned bitIndex)
{
  //TODO
  unsigned ret = 0;
//    unsigned constMSB = 0b10000000;
//    unsigned constONE = 0b00000001;
    unsigned constONE = 1;

//    0b00000001
//    0b00000010
//    0b00000100
//    0b00001000
//    0b00010000
//    0b00100000
//    0b01000000
//    0b10000000

//    BITS_PER_BYTE
//  ret = ret | constMSB;
//  ret = ret >> bitIndex;

  ret = constONE << (BITS_PER_BYTE -1 - bitIndex);
  return ret;
}

/** Given a power-of-2 powerOf2, return log2(powerOf2) */
static inline unsigned
getLog2PowerOf2(unsigned powerOf2)
{
    unsigned test = 1;
  //TODO
  for(int i = 0; i < BITS_PER_BYTE+2; i++) {
      if(test == powerOf2) {
          return i;
      }
      test = test << 1;
  }
  return 0; // should be found, if not then return 0
}

/** Given a bitOffset return the bitIndex part of the bitOffset. */
static inline unsigned
getBitIndex(unsigned bitOffset)
{
  float div = (float)bitOffset / (float)BITS_PER_BYTE;
  int floor = floorf(div);
  int ret = bitOffset - floor*BITS_PER_BYTE;
  return ret;
}


/*
 *  Given a bytes[] array and some bitOffset, and assuming that
 *  BITS_PER_BYTE is 8, then (bitOffset >> 3) represents the index of
 *  the byte within bytes[]
 */
/** Given a bitOffset return the byte offset part of the bitOffset ???????  */
static inline unsigned
getOffset(unsigned bitOffset)
{
    float div = (float)bitOffset / (float)BITS_PER_BYTE;
    int floor = floorf(div);
    return floor;
}

/** Return bit at offset bitOffset in array[]; i.e., return
 *  (bits(array))[bitOffset]
 */
static inline int
getBitAtOffset(const Byte array[], unsigned bitOffset)
{
    unsigned ret = 0;
    unsigned byteIndex = getOffset(bitOffset);
    Byte byte = array[byteIndex];
    unsigned bitIndex = getBitIndex(bitOffset);
    unsigned mask = byteBitMask(bitIndex);
    ret = byte & mask;
    return ret>0;
}

/** Set bit selected by bitOffset in array to the parameter bit. */
static inline void
setBitAtOffset(Byte array[], unsigned bitOffset, unsigned bit)
{
    unsigned byteIndex = getOffset(bitOffset);
    Byte byte = array[byteIndex];
    unsigned bitIndex = getBitIndex(bitOffset);
    unsigned mask = byteBitMask(bitIndex);
    if (bit==1) {
        array[byteIndex] = (byte | mask);
    } else {
        //       mask = (~mask);
        unsigned inverted_mask = UINT_MAX ^ mask;
        array[byteIndex] = byte & inverted_mask;
        //       fprintf(stderr, "byte: %s \n", array[byteIndex]);
    }
    return;
}

/** Set count bits in array[] starting at bitOffset to bit.  Return
 *  bit-offset one beyond last bit set.
 */
static inline unsigned
setBitsAtOffset(Byte array[], unsigned bitOffset, unsigned bit, unsigned count)
{
    for(int i = 0; i < count; i++) {
        setBitAtOffset(array,bitOffset + i, bit);
    }
    return bitOffset + count + 1;
}
static inline TextMorse getMorse(Byte byte) {
    int i = 0;
    if (byte=='0') {
        i = 35;
    } else if (isdigit(byte)) {
        i = 26 + (byte - '1');
    } else {
        i = byte - 'A';
    }
    TextMorse tm = charCodes[i];
    return tm;
}
static inline int getSize (char * s) {
    char * t; // first copy the pointer to not change the original
    int size = 0;

    for (t = s; *t != '\0'; t++) {
        size++;
    }

    return size;
}

/** Convert text[nText] into a binary encoding of morse code in
 *  morse[].  It is assumed that array morse[] is initially all zero
 *  and is large enough to represent the morse code for all characters
 *  in text[].  The result in morse[] should be terminated by the
 *  morse prosign AR.  Any sequence of non-alphanumeric characters in
 *  text[] should be treated as a *single* inter-word space.  Leading
 *  non alphanumeric characters in text are ignored.
 *
 *  Returns count of number of bytes used within morse[].
 */
int
textToMorse(const Byte text[], unsigned nText, Byte morse[])
{
    unsigned textIndex = 0;
    unsigned morseBitOffset = 0;
    // read past non alpha data to start
    while (! isalnum(text[textIndex])) {
        textIndex++;
    }
    /// loop all text data
    while ( textIndex<nText ) {
        // get byte to work with and set index for next loop
        Byte byte = text[textIndex];
        // get the char* we need to write    is
        TextMorse tm = getMorse(byte);
        char* cp = tm.code;
        // get the size of the char*
        int sz = getSize(cp);
        // looop for each byte in char*
        for (int i = 0 ; i< sz; i++) {
            // set the bits in the byte array (writing the morse codes to the arry)
            if (cp[i] == '-') {
                setBitsAtOffset(morse, morseBitOffset, 1, 3);
                morseBitOffset+=3;
            } else {
                setBitAtOffset(morse, morseBitOffset , 1);
                morseBitOffset+=1;
            }
            // this adds one 0 bit for between dots or dashs
            setBitAtOffset(morse, morseBitOffset++, 0);
        }
        // between characters we need 3 000's but have added one already so we add two more 0's
        setBitsAtOffset(morse, morseBitOffset, 0, 2);
        morseBitOffset+=2;
        // we have added three 0's because we are betrween charasters but maybe we have a word boundery
        // then need to skip whitespaces and add four more 0's

        // PEEK AHEAD, if term then break...
        if (text[textIndex+1]=='\0' || text[textIndex+1]=='\n') {
            break;
        }
        // PEEK AHEAD, and if space char add extra 4 ...
        int j = 0;
        while (textIndex<nText && (! isalnum(text[textIndex+1]))) {
            j++;
            textIndex++;
        }
        if (j) {
//            textIndex+=j;
            setBitsAtOffset(morse, morseBitOffset, 0, 4);
            morseBitOffset+=4;
        }
        textIndex++;    // ready for next char
    }
    for (int i = 0; i < 2; i++) {
        setBitAtOffset(morse, morseBitOffset++, 1); // .
        setBitAtOffset(morse, morseBitOffset++, 0);
        setBitsAtOffset(morse, morseBitOffset, 1, 3); // -
        morseBitOffset += 3;
        setBitAtOffset(morse, morseBitOffset++, 0);
    }
    setBitAtOffset(morse, morseBitOffset, 1); // .
    float  ret = ceil((float)morseBitOffset/(float)BITS_PER_BYTE);
    return ret; //RETURNS # of bytes
}

/** Return count of run of identical bits starting at bitOffset
 *  in bytes[nBytes].
 *
 *  question:
 *  does bitoffset refer to the offset in just the byte?
 *  where Byte byte = bytes[nBytes]
 *  and bits would be a number from 0 to 7 if byte is 8 bits
 *
 */
static inline unsigned
runLength(const Byte bytes[], unsigned nBytes, unsigned bitOffset)
{
  //TODO
    unsigned first = getBitAtOffset(bytes, bitOffset);
    for(unsigned i = 1; i < BITS_PER_BYTE*nBytes; i++) {
        unsigned next = getBitAtOffset(bytes, bitOffset + i);
        if(first != next) {
            return i;
        }
    }
  return BITS_PER_BYTE*nBytes;
}


/** Convert AR-prosign terminated binary Morse encoding in
 *  morse[nMorse] into text in text[].  It is assumed that array
 *  text[] is large enough to represent the decoding of the code in
 *  morse[].  Leading zero bits in morse[] are ignored. Encodings
 *  representing word separators are output as a space ' ' character.
 *
 *  Returns count of number of bytes used within text[], < 0 on error.
 */
int
morseToText(const Byte morse[], unsigned nMorse, Byte text[])
{
    int bitOffset = 0;
    int textIndex = 0;
    int clean = 0;
    for(int i = 0; i < BITS_PER_BYTE; i++) {
        char morseChars[5];
	if(!clean) {
	    for(int n = 0; n < 5; n++) {
                morseChars[n] = 0;
            }
	    clean = 1;
	}
        if(runLength(morse, 1, bitOffset) == 3 && getBitAtOffset(morse, bitOffset) == 1) {
            morseChars[i] = '-';
            bitOffset+=3;
        } if (runLength(morse, 1, bitOffset) == 1 && getBitAtOffset(morse, bitOffset) == 1){
            morseChars[i] = '.';
            bitOffset++;
        } if (runLength(morse, 1, bitOffset) == 1 && getBitAtOffset(morse, bitOffset) == 0) {
            bitOffset++;
        } if (runLength(morse, 1, bitOffset) == 3 && getBitAtOffset(morse, bitOffset) == 0) {
            bitOffset+=3;
            int Char = codeToChar(morseChars);
            text[textIndex] = Char;
            textIndex++;
            for(int n = 0; n < 5; n++) {
                morseChars[n] = 0;
            }
            i = -1;
        } if (runLength(morse, 1, bitOffset) == 7 && getBitAtOffset(morse, bitOffset) == 0) {
            bitOffset += 7;
            int Char = codeToChar(morseChars);
            text[textIndex] = Char;
            textIndex++;
            text[textIndex] = ' ';
            textIndex++;
            for(int n = 0; n < 5; n++) {
                morseChars[n] = 0;
            }
            i = -1;
        } if (strcmp(morseChars, ".-.-.") == 0) {
            text[textIndex] = '\0';
            textIndex++;
            return textIndex; // fix
        }
    }
    return textIndex;
}