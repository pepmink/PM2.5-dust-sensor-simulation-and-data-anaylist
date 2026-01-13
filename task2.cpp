#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAX_LINE 256
#define MIN_VALID 3.0
#define MAX_VALID 550.5

typedef struct {
    int id;
    char time[20];
    float value;
} DustData;

typedef struct {
    int id;
    char time[20];
    float value;
    int aqi;
    char pollution[20];
} AQIData;

// AQI Table lookup
void get_aqi_info(float concentration, int *aqi, char *pollution) {
    if (concentration >= 0 && concentration <= 12.0) {
        *aqi = 50;
        strcpy(pollution, "Good");
    } else if (concentration <= 35.4) {
        *aqi = 100;
        strcpy(pollution, "Moderate");
    } else if (concentration <= 55.4) {
        *aqi = 150;
        strcpy(pollution, "Unhealthy_S");
    } else if (concentration <= 150.4) {
        *aqi = 200;
        strcpy(pollution, "Unhealthy");
    } else if (concentration <= 250.4) {
        *aqi = 300;
        strcpy(pollution, "Very_Unhealthy");
    } else {
        *aqi = 500;
        strcpy(pollution, "Hazardous");
    }
}

// Extract hour from timestamp "YYYY:MM:DD hh:mm:ss"
int extract_hour(const char *time) {
    int hour;
    sscanf(time + 11, "%d", &hour);
    return hour;
}

// Compare function for qsort
int compare_float(const void *a, const void *b) {
    float fa = *(const float*)a;
    float fb = *(const float*)b;
    return (fa > fb) - (fa < fb);
}

// Task 2.1: Filter outliers
void filter_outliers(const char *input_file) {
    FILE *fin = fopen(input_file, "r");
    if (!fin) {
        printf("Error: Cannot open file %s\n", input_file);
        return;
    }

    FILE *fvalid = fopen("dust_valid.csv", "w");
    FILE *foutlier = fopen("dust_outlier.csv", "w");
    
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fin); // Skip header
    fprintf(fvalid, "id,time,value\n");
    fprintf(foutlier, "id,time,value\n");
    int outlier_count = 0;
    DustData data;
    
    // First pass: count outliers
    while (fgets(line, MAX_LINE, fin)) {
        sscanf(line, "%d,%19[^,],%f", &data.id, data.time, &data.value);
        if (data.value < MIN_VALID || data.value > MAX_VALID) {
            outlier_count++;
        }
    }
    
    // Write outlier count
    fprintf(foutlier, "number of outliers: %d\n", outlier_count);
    fprintf(foutlier, "id,time,value\n");
    
    // Second pass: write data
    rewind(fin);
    fgets(line, MAX_LINE, fin); // Skip header again
    
    while (fgets(line, MAX_LINE, fin)) {
        sscanf(line, "%d,%19[^,],%f", &data.id, data.time, &data.value);
        if (data.value < MIN_VALID || data.value > MAX_VALID) {
            fprintf(foutlier, "%d,%s,%.1f\n", data.id, data.time, data.value);
        } else {
            fprintf(fvalid, "%d,%s,%.1f\n", data.id, data.time, data.value);
        }
    }
    
    fclose(fin);
    fclose(fvalid);
    fclose(foutlier);
    
    printf("Task 2.1 completed: %d outliers filtered\n", outlier_count);
}

// Task 2.2: Calculate AQI
void calculate_aqi() {
    FILE *fin = fopen("dust_valid.csv", "r");
    if (!fin) {
        printf("Error: Cannot open dust_valid.csv\n");
        return;
    }
    
    // Read all valid data
    DustData *data = (DustData *)malloc(10000 * sizeof(DustData));
    int count = 0;
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fin); // Skip header
    
    while (fgets(line, MAX_LINE, fin) && count < 10000) {
        sscanf(line, "%d,%19[^,],%f", &data[count].id, data[count].time, &data[count].value);
        count++;
    }
    fclose(fin);
    
    // Calculate hourly average and AQI
    FILE *fout = fopen("dust_aqi.csv", "w");
    fprintf(fout, "id,time,value,aqi,pollution\n");
    
    // Group by sensor and hour
    for (int sensor_id = 1; sensor_id <= 10; sensor_id++) {
        for (int hour = 0; hour < 24; hour++) {
            float sum = 0;
            int cnt = 0;
            char sample_time[20] = "";
            
            for (int i = 0; i < count; i++) {
                if (data[i].id == sensor_id && extract_hour(data[i].time) == hour) {
                    sum += data[i].value;
                    cnt++;
                    if (sample_time[0] == '\0') {
                        strcpy(sample_time, data[i].time);
                    }
                }
            }
            
            if (cnt > 0) {
                float avg = sum / cnt;
                int aqi;
                char pollution[20];
                get_aqi_info(avg, &aqi, pollution);
                fprintf(fout, "%d,%s,%.1f,%d,%s\n", sensor_id, sample_time, avg, aqi, pollution);
            }
        }
    }
    
    fclose(fout);
    free(data);
    printf("Task 2.2 completed: AQI calculated\n");
}

// Task 2.3: Summary statistics
void summary_statistics() {
    FILE *fin = fopen("dust_valid.csv", "r");
    if (!fin) {
        printf("Error: Cannot open dust_valid.csv\n");
        return;
    }
    
    DustData *data = (DustData *)malloc(10000 * sizeof(DustData));
    int count = 0;
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fin);
    
    while (fgets(line, MAX_LINE, fin) && count < 10000) {
        sscanf(line, "%d,%19[^,],%f", &data[count].id, data[count].time, &data[count].value);
        count++;
    }
    fclose(fin);
    
    FILE *fout = fopen("dust_summary.csv", "w");
    fprintf(fout, "id,parameter,value,time\n");
    
    // Process each sensor
    for (int sensor_id = 1; sensor_id <= 10; sensor_id++) {
        float *values = (float *)malloc(count * sizeof(float));
        int sensor_count = 0;
        float sum = 0;
        float max_val = -999999;
        float min_val = 999999;
        char max_time[20] = "";
        char min_time[20] = "";
        char start_time[20] = "";
        char end_time[20] = "";
        
        // Collect data for this sensor
        for (int i = 0; i < count; i++) {
            if (data[i].id == sensor_id) {
                values[sensor_count++] = data[i].value;
                sum += data[i].value;
                
                if (start_time[0] == '\0') strcpy(start_time, data[i].time);
                strcpy(end_time, data[i].time);
                
                if (data[i].value > max_val) {
                    max_val = data[i].value;
                    strcpy(max_time, data[i].time);
                }
                if (data[i].value < min_val) {
                    min_val = data[i].value;
                    strcpy(min_time, data[i].time);
                }
            }
        }
        
        if (sensor_count > 0) {
            float mean = sum / sensor_count;
            
            // Calculate median
            qsort(values, sensor_count, sizeof(float), compare_float);
            float median = (sensor_count % 2 == 0) ? 
                (values[sensor_count/2 - 1] + values[sensor_count/2]) / 2.0 :
                values[sensor_count/2];
            
            char time_range[50];
            sprintf(time_range, "%s-%s", start_time, end_time);
            
            fprintf(fout, "%d,max,%.1f,%s\n", sensor_id, max_val, max_time);
            fprintf(fout, "%d,min,%.1f,%s\n", sensor_id, min_val, min_time);
            fprintf(fout, "%d,mean,%.1f,%s\n", sensor_id, mean, time_range);
            fprintf(fout, "%d,median,%.1f,%s\n", sensor_id, median, time_range);
        }
        
        free(values);
    }
    
    fclose(fout);
    free(data);
    printf("Task 2.3 completed: Summary statistics calculated\n");
}

// Task 2.4: Pollution duration statistics
void pollution_statistics() {
    FILE *fin = fopen("dust_aqi.csv", "r");
    if (!fin) {
        printf("Error: Cannot open dust_aqi.csv\n");
        return;
    }
    
    AQIData *data = (AQIData *)malloc(10000 * sizeof(AQIData));
    int count = 0;
    char line[MAX_LINE];
    fgets(line, MAX_LINE, fin);
    
    while (fgets(line, MAX_LINE, fin) && count < 10000) {
        sscanf(line, "%d,%19[^,],%f,%d,%19s", 
               &data[count].id, data[count].time, &data[count].value, 
               &data[count].aqi, data[count].pollution);
        count++;
    }
    fclose(fin);
    
    FILE *fout = fopen("dust_statistics.csv", "w");
    fprintf(fout, "id,pollution,duration\n");
    
    const char *pollution_levels[] = {"Good", "Moderate", "Unhealthy_S", "Unhealthy", "Very_Unhealthy", "Hazardous"};
    
    for (int sensor_id = 1; sensor_id <= 10; sensor_id++) {
        for (int p = 0; p < 6; p++) {
            int hours = 0;
            for (int i = 0; i < count; i++) {
                if (data[i].id == sensor_id && strcmp(data[i].pollution, pollution_levels[p]) == 0) {
                    hours++;
                }
            }
            if (hours > 0) {
                fprintf(fout, "%d,%s,%d\n", sensor_id, pollution_levels[p], hours);
            }
        }
    }
    
    fclose(fout);
    free(data);
    printf("Task 2.4 completed: Pollution statistics calculated\n");
}

int main(int argc, char *argv[]) {
    char filename[256] = "dust_sensor.csv";
    
    if (argc >= 2) {
        strcpy(filename, argv[1]);
    }
    
    printf("Processing file: %s\n\n", filename);
    
    // Task 2.1: Filter outliers
    filter_outliers(filename);
    
    // Task 2.2: Calculate AQI
    calculate_aqi();
    
    // Task 2.3: Summary statistics
    summary_statistics();
    
    // Task 2.4: Pollution statistics
    pollution_statistics();
    
    printf("\nAll tasks completed successfully!\n");
    printf("Output files:\n");
    printf("  - dust_valid.csv\n");
    printf("  - dust_outlier.csv\n");
    printf("  - dust_aqi.csv\n");
    printf("  - dust_summary.csv\n");
    printf("  - dust_statistics.csv\n");
    
    return 0;
}
