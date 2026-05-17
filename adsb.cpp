#include "adsb.hpp"
#include <cmath> 

// Extract the Type Code (Top 5 bits of msg[4])
int getTypeCode(uint8_t* msg) {
    return msg[4] >> 3;
}

// Extract Callsign (Type 1-4)
std::string getCallsign(uint8_t* msg) {
    std::string callsign = "";
    uint64_t payload = ((uint64_t)msg[5] << 40) | ((uint64_t)msg[6] << 32) | 
                       ((uint64_t)msg[7] << 24) | ((uint64_t)msg[8] << 16) | 
                       ((uint64_t)msg[9] << 8)  | (uint64_t)msg[10];

    for (int i = 0; i < 8; i++) {
        int shift_amount = 42 - (i * 6); 
        uint8_t char_value = (payload >> shift_amount) & 0x3F; 
        callsign += ADSB_ALPHABET[char_value];
    }
    return callsign;
}

// Extract Altitude (Type 9-18 and 20-22)
int getAltitude(uint8_t* msg) {
    uint16_t raw_alt = ((uint16_t)msg[5] << 4) | (msg[6] >> 4);
    uint16_t real_alt = (raw_alt >> 5) << 4 | (raw_alt & 0x0F);
    return (real_alt * 25) - 1000;
}

// Extract GPS Coordinates with Dynamic Even/Odd CPR Handling
Position getCoordinates(uint8_t* msg, double receiver_lat, double receiver_lon) {
    uint64_t payload = ((uint64_t)msg[4] << 48) | ((uint64_t)msg[5] << 40) | 
                       ((uint64_t)msg[6] << 32) | ((uint64_t)msg[7] << 24) | 
                       ((uint64_t)msg[8] << 16) | ((uint64_t)msg[9] << 8)  | 
                       (uint64_t)msg[10];

    double lat_fraction = ((payload >> 17) & 0x1FFFF) / 131072.0;
    double lon_fraction = (payload & 0x1FFFF) / 131072.0;
    
    // CPR Format bit is Bit 34 of the message frame (Bit 22 of payload)
    int cpr_format = (payload >> 34) & 1; 
    
    // Dynamic latitude zone size based on format flag
    double zone_size = (cpr_format == 0) ? (360.0 / 60.0) : (360.0 / 59.0);
    
    // Latitude Math
    double base_index = floor(receiver_lat / zone_size);
    double j = base_index + floor(0.5 + (fmod(receiver_lat, zone_size) / zone_size) - lat_fraction);
    double global_lat = zone_size * (j + lat_fraction);

    // Longitude Math
    double pi = 3.14159265358979323846;
    double lat_rad = global_lat * (pi / 180.0);
    
    double cos_val = cos(lat_rad);
    double nl_sub = 1.0 - (1.0 - cos(pi / 30.0)) / (cos_val * cos_val);
    
    double nl = 1;
    if (nl_sub > -1.0 && nl_sub < 1.0) {
        nl = floor((2 * pi) / acos(nl_sub));
    }
    
    if (cpr_format == 1) {
        nl = nl - 1;
    }
    if (nl < 1) nl = 1; 

    double lon_zone_size = 360.0 / nl;
    double base_lon_index = floor(receiver_lon / lon_zone_size);
    double m = base_lon_index + floor(0.5 + (fmod(receiver_lon, lon_zone_size) / lon_zone_size) - lon_fraction);
    double global_lon = lon_zone_size * (m + lon_fraction);

    Position final_pos;
    final_pos.latitude = global_lat;
    final_pos.longitude = global_lon;
    return final_pos;
}

// Validate the message using the 24-bit generator polynomial
bool checkCRC(uint8_t* msg) {
    uint32_t generator = 0xFFF409;
    
    uint8_t temp[14];
    for(int i=0; i<14; i++) temp[i] = msg[i];

    for (int i = 0; i < 88; i++) {
        int byte_idx = i / 8;
        int bit_idx = 7 - (i % 8);

        if (temp[byte_idx] & (1 << bit_idx)) {
            temp[byte_idx] ^= (uint8_t)(generator >> 16);
            temp[byte_idx + 1] ^= (uint8_t)(generator >> 8);
            temp[byte_idx + 2] ^= (uint8_t)(generator);
        }
    }

    if (temp[11] == 0 && temp[12] == 0 && temp[13] == 0) {
        return true;
    }

    return false;
}