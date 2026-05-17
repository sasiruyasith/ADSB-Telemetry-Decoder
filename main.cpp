#include <iostream>
#include <cmath>
#include <vector>
#include <fstream>
#include <string>
#include <rtl-sdr.h>
#include "adsb.hpp"

// --- ARDUINO COMMUNICATION ---
void sendToArduino(std::string lat, std::string lon) {
    // Ensure this matches your exact Arduino Port
    std::ofstream arduino("\\\\.\\COM3"); 
    if (arduino.is_open()) {
        arduino << lat << "," << lon << "\n";
        arduino.close();
    }
}

// --- RADAR ENGINE ---
bool detectPreamble(const std::vector<double>& magnitudes, int start_index) {
    if (start_index + 10 > magnitudes.size()) return false;

    double p0 = magnitudes[start_index + 0];
    double p2 = magnitudes[start_index + 2];
    double p7 = magnitudes[start_index + 7];
    double p9 = magnitudes[start_index + 9];

    double empty1 = magnitudes[start_index + 1];
    double empty3 = magnitudes[start_index + 3];
    double empty4 = magnitudes[start_index + 4];
    double empty5 = magnitudes[start_index + 5];
    double empty6 = magnitudes[start_index + 6];
    double empty8 = magnitudes[start_index + 8];

    if (p0 < 12 || p2 < 12 || p7 < 12 || p9 < 12) return false;

    if (p0 > empty1 && p0 > empty3 && p0 > empty4 && 
        p2 > empty1 && p2 > empty3 && p2 > empty4 &&
        p7 > empty6 && p7 > empty8 &&
        p9 > empty6 && p9 > empty8) {
        return true; 
    }
    return false;
}

int main() {
    std::cout << "--- Radar Engine Started ---" << std::endl;
    rtlsdr_dev_t *dev = nullptr;

    if (rtlsdr_open(&dev, 0) < 0) {
        std::cout << "Hardware Error: Could not find SDR." << std::endl;
        return 1;
    }

    rtlsdr_set_center_freq(dev, 1090000000); 
    rtlsdr_set_sample_rate(dev, 2000000);    
    rtlsdr_set_tuner_gain_mode(dev, 1);      
    rtlsdr_set_tuner_gain(dev, 496);         
    rtlsdr_reset_buffer(dev);

    const int buffer_size = 262144;
    uint8_t buffer[buffer_size];
    int bytes_read = 0;

    std::cout << "Scanning for verified aircraft... (Ctrl+C to stop)\n" << std::endl;

    while (true) {
        if (rtlsdr_read_sync(dev, buffer, buffer_size, &bytes_read) < 0) break;

        std::vector<double> magnitudes;
        magnitudes.reserve(bytes_read / 2);
        for (int i = 0; i < bytes_read; i += 2) {
            double i_val = buffer[i] - 127.0;
            double q_val = buffer[i+1] - 127.0;
            magnitudes.push_back(std::sqrt((i_val * i_val) + (q_val * q_val)));
        }

        for (int i = 0; i < (int)magnitudes.size() - 250; i++) {
            if (detectPreamble(magnitudes, i)) {
                
                uint8_t current_msg[14] = {0};
                int data_start = i + 16; 

                for (int bit = 0; bit < 112; bit++) {
                    int s_idx = data_start + (bit * 2);
                    if (magnitudes[s_idx] > magnitudes[s_idx + 1]) {
                        current_msg[bit / 8] |= (1 << (7 - (bit % 8)));
                    }
                }

                if (checkCRC(current_msg)) {
                    int df = current_msg[0] >> 3;
                    int type_code = getTypeCode(current_msg);

                    if (df == 17) {
                        // Include high-altitude positioning windows (9-18 and 20-22)
                        if ((type_code >= 9 && type_code <= 18) || (type_code >= 20 && type_code <= 22)) {
                            std::cout << "\n>>> VERIFIED ADS-B (DF17) <<<" << std::endl;
                            std::cout << "ALTITUDE: " << getAltitude(current_msg) << " feet" << std::endl;
                            
                            Position pos = getCoordinates(current_msg, 6.840, 79.940);
                            
                            sendToArduino(std::to_string(pos.latitude), std::to_string(pos.longitude));
                            std::cout << ">> SENT TO LCD: " << pos.latitude << ", " << pos.longitude << std::endl;

                        } 
                        else if (type_code >= 1 && type_code <= 4) {
                            std::cout << "\n>>> VERIFIED ADS-B (DF17) <<<" << std::endl;
                            std::cout << "CALLSIGN: " << getCallsign(current_msg) << std::endl;
                        }
                    }
                }
                i += 240; 
            }
        }
    }

    rtlsdr_close(dev);
    return 0;
}