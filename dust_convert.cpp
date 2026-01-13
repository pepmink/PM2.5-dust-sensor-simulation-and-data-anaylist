#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#define MAX_LINE 256
#define START_BYTE 0xAA
#define STOP_BYTE 0xFF
#define PACKET_SIZE 16

// Convert timestamp "YYYY:MM:DD hh:mm:ss" to Unix timestamp
uint32_t timestamp_to_unix(const char *timestamp) {
    struct tm tm_info = {0};
    int year, month, day, hour, min, sec;
    
    sscanf(timestamp, "%d:%d:%d %d:%d:%d", 
           &year, &month, &day, &hour, &min, &sec);
    
    tm_info.tm_year = year - 1900;
    tm_info.tm_mon = month - 1;
    tm_info.tm_mday = day;
    tm_info.tm_hour = hour;
    tm_info.tm_min = min;
    tm_info.tm_sec = sec;
    tm_info.tm_isdst = -1;
    
    return (uint32_t)mktime(&tm_info);
}

// Get pollution code (ASCII character)
uint8_t get_pollution_code(const char *pollution) {
    if (strcmp(pollution, "Good") == 0) return 'G';
    if (strcmp(pollution, "Moderate") == 0) return 'M';
    if (strcmp(pollution, "Unhealthy_S") == 0) return 'U';
    if (strcmp(pollution, "Unhealthy") == 0) return 'u';
    if (strcmp(pollution, "Very_Unhealthy") == 0) return 'V';
    if (strcmp(pollution, "Hazardous") == 0) return 'H';
    return 'X'; // Unknown
}

// Calculate two's complement checksum
uint8_t calculate_checksum(uint8_t *packet, int start, int end) {
    uint32_t sum = 0;
    for (int i = start; i < end; i++) {
        sum += packet[i];
    }
    // Two's complement: negate and add 1
    return (uint8_t)(~sum + 1);
}

// Write float in little-endian format
void write_float_le(uint8_t *buffer, float value) {
    union {
        float f;
        uint8_t bytes[4];
    } data;
    data.f = value;
    
    // Little-endian: LSB first
    buffer[0] = data.bytes[0];
    buffer[1] = data.bytes[1];
    buffer[2] = data.bytes[2];
    buffer[3] = data.bytes[3];
}

// Write uint32_t in little-endian format
void write_uint32_le(uint8_t *buffer, uint32_t value) {
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
    buffer[2] = (uint8_t)((value >> 16) & 0xFF);
    buffer[3] = (uint8_t)((value >> 24) & 0xFF);
}

// Write uint16_t in little-endian format
void write_uint16_le(uint8_t *buffer, uint16_t value) {
    buffer[0] = (uint8_t)(value & 0xFF);
    buffer[1] = (uint8_t)((value >> 8) & 0xFF);
}

// Create data packet
void create_packet(uint8_t *packet, uint8_t id, const char *timestamp, 
                   float pm25, uint16_t aqi, const char *pollution) {
    int pos = 0;
    
    // Start byte
    packet[pos++] = START_BYTE;
    
    // Packet length (total size)
    packet[pos++] = PACKET_SIZE;
    
    // ID
    packet[pos++] = id;
    
    // Time (Unix timestamp, 4 bytes, little-endian)
    uint32_t unix_time = timestamp_to_unix(timestamp);
    write_uint32_le(&packet[pos], unix_time);
    pos += 4;
    
    // PM2.5 (4 bytes, IEEE 754 float, little-endian)
    write_float_le(&packet[pos], pm25);
    pos += 4;
    
    // AQI (2 bytes, little-endian)
    write_uint16_le(&packet[pos], aqi);
    pos += 2;
    
    // Pollution code (1 byte, ASCII)
    packet[pos++] = get_pollution_code(pollution);
    
    // Checksum (1 byte)
    // Calculate checksum for bytes from packet_length to pollution_code
    packet[pos] = calculate_checksum(packet, 1, pos);
    pos++;
    
    // Stop byte
    packet[pos++] = STOP_BYTE;
}

// Write packet to file in hex format
void write_packet_hex(FILE *fout, uint8_t *packet) {
    for (int i = 0; i < PACKET_SIZE; i++) {
        fprintf(fout, "%02X", packet[i]);
        if (i < PACKET_SIZE - 1) {
            fprintf(fout, " ");
        }
    }
    fprintf(fout, "\n");
}

int main(int argc, char *argv[]) {
    char input_file[256] = "dust_aqi.csv";
    char output_file[256] = "hex_packet.dat";
    
    // Parse command-line arguments
    if (argc >= 2) {
        strcpy(input_file, argv[1]);
    }
    if (argc >= 3) {
        strcpy(output_file, argv[2]);
    }
    
    printf("Converting: %s -> %s\n", input_file, output_file);
    
    // Open input file
    FILE *fin = fopen(input_file, "r");
    if (!fin) {
        printf("Error: Cannot open input file %s\n", input_file);
        return 1;
    }
    
    // Open output file
    FILE *fout = fopen(output_file, "w");
    if (!fout) {
        printf("Error: Cannot create output file %s\n", output_file);
        fclose(fin);
        return 1;
    }
    
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fin); // Skip header
    
    int packet_count = 0;
    uint8_t packet[PACKET_SIZE];
    
    // Process each line
    while (fgets(line, MAX_LINE, fin) && packet_count < 10000) {
        int id, aqi;
        char timestamp[20];
        float pm25;
        char pollution[20];
        
        // Parse CSV line: id,time,value,aqi,pollution
        int parsed = sscanf(line, "%d,%19[^,],%f,%d,%19s", 
                           &id, timestamp, &pm25, &aqi, pollution);
        
        if (parsed == 5 && id > 0) {
            // Create packet
            create_packet(packet, (uint8_t)id, timestamp, pm25, (uint16_t)aqi, pollution);
            
            // Write to output file in hex format
            write_packet_hex(fout, packet);
            
            packet_count++;
        }
    }
    
    fclose(fin);
    fclose(fout);
    
    printf("Conversion completed: %d packets created\n", packet_count);
    printf("Output file: %s\n", output_file);
    
    return 0;
}
