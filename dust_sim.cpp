#include <stdio.h>
#include <string.h>
// Transfer the command line argument to integer(The number in the command line in termianl is just a character string and not an integer)
// Using the algorithem new result = result * 10 + (str[i] - '0') to convert string to integer
int Atoi(const char *str){
    int result = 0;
    for (int i = 0; str[i] != '\0'; i++){
        if (str[i] < '0' || str[i] > '9'){
            printf("Error: Invalid character)");
            return -1;
        }
        result = result * 10 + (str[i] - '0');
    }    
    return result;
}
int main(int argc,char* argv[]){
    int num_sensors = 5; //Default number of sensors
    int sampling_time = 60; //Default sampling time in seconds
    int interval = 24; //Default interval in hours

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
                if (value <= 0){
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
                if (value <= 0){
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
}