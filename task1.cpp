#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#define default_num_sensors 1
#define default_sampling_time 60
#define default_interval 24 
// Transfer the command line argument to integer(The number in the command line in termianl is just a character string and not an integer)
// Using the algorithem new result = result * 10 + (str[i] - '0') to convert string to integer
int Atoi(const char *str){
    int result = 0;
    for (int i = 0; str[i] != '\0'; i++){
        if (str[i] < '0' || str[i] > '9'){
            printf("Error: Invalid character\n");
            return -1;
        }
        result = result * 10 + (str[i] - '0');
    }    
    return result;
}

//Time processing
void time_processing(int sampling_time, int interval, int num_sensors){

    FILE *file = fopen("dust_sensor.csv", "w");
    if (file == NULL){
        printf("Error: Could not open file\n");
        return;
    }

    fprintf(file, "id,time,value\n");

    time_t start_time = time(NULL);  // Start from current time
    
    srand(time(NULL)); // Seed the random number generator

    int total_samples = (interval * 3600) / sampling_time; //Total number of samples in the interval
    for (int i = 0; i <= total_samples; i++){
        // Calculate current timestamp
        time_t current_time = start_time + (i * sampling_time);
        struct tm *tm_info = localtime(&current_time);
        
        // Generate data for all sensors at this timestamp
        for (int sensor_id = 1; sensor_id <= num_sensors; sensor_id++){
            float value = (rand() % 5000) / 10.0; //Generate a random float value between 0.0 and 500.0
            fprintf(file, "%d,%04d:%02d:%02d %02d:%02d:%02d,%.1f\n", 
                    sensor_id,
                    tm_info->tm_year + 1900,
                    tm_info->tm_mon + 1,
                    tm_info->tm_mday,
                    tm_info->tm_hour,
                    tm_info->tm_min,
                    tm_info->tm_sec,
                    value);
        }
    }
    
    fclose(file);
    printf("Data generated successfully in dust_sensor.csv\n");
}



int main(int argc,char* argv[]){

    int num_sensors = default_num_sensors; //Default number of sensors
    int sampling_time = default_sampling_time; //Default sampling time in seconds
    int interval = default_interval; //Default interval in hours

    //Command-Line Argument Processing
    for (int i = 1; i < argc ; i++){ // i = 0 is the program name
        if (strcmp(argv[i], "-n") == 0){
           
            // Check if there is a value after -n
            if (i + 1 < argc){
                int value = Atoi(argv[i + 1]);
                if (value <= 0){
                    printf("Error: Invalid number of sensors\n");
                    return 1;
                }
                num_sensors = value;
                i++;
            }
            else{
                printf("Error: Missing value for -n\n");
                return 1;
            }
        }

        //Check if there is a value after -st
        else if (strcmp(argv[i], "-st") == 0){
            if(i + 1 < argc){
                int value = Atoi(argv[i + 1]);
                if (value < 10){
                    printf("Error: Invalid sampling time\n");
                    return 1;
                }
                sampling_time = value;
                i++;
            }
            else{
                printf("Error: Missing value for -st\n");
                return 1;
            }
        }

        //Check if there is a value after -si
        else if (strcmp(argv[i], "-si") == 0){
            if(i + 1 < argc){
                int value = Atoi(argv[i + 1]);
                if (value < 1){
                    printf("Error: Invalid interval\n");
                    return 1;
                }
                interval = value;
                i++;
            }
            else{
                printf("Error: Missing value for -si\n");
                return 1;
            }
        }
    }
    
    // Generate simulation data
    time_processing(sampling_time, interval, num_sensors);
    
    return 0;
}