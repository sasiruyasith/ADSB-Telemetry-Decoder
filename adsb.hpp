#pragma once
#include <cstdint>
#include <string>

// Our custom box to hold GPS coordinates
struct Position {
    double latitude;
    double longitude;
};

// The ADS-B Alphabet Lookup Table 
// (Added 'static' to prevent linking errors)
static const char ADSB_ALPHABET[64] = {
    '#', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '#', '#', '#', '#', '#',
    '_', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#', '#',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '#', '#', '#', '#', '#', '#'
};

// --- The Tools ---

// Returns the Downlink Format / Type Code
int getTypeCode(uint8_t* msg);

// Extracts the 8-character flight callsign
std::string getCallsign(uint8_t* msg);

// Decodes the altitude in feet
int getAltitude(uint8_t* msg);

// Performs CPR decoding to get Lat/Lon
Position getCoordinates(uint8_t* msg, double receiver_lat, double receiver_lon);

// Validates the message using the 24-bit generator polynomial
bool checkCRC(uint8_t* msg);