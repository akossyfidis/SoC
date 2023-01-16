#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>


void led_modify_speed(char* filepath, long speed);
void led_modify_dir(char* filepath, long dir);
void led_modify_pattern(char* filepath, long pattern);

int main(int argc, char** argv)
{
    if(argc != 4)
    {
        perror("Usage : ./led pattern speed direction (0 or 1)");
        exit(EXIT_FAILURE);
    }

    char * filepath_speed = "/proc/ensea/speed";
    char * filepath_dir = "/proc/ensea/dir";
    char * filepath_pattern = "/dev/ensea_leds";

    led_modify_pattern(filepath_pattern, atol(argv[1]));
    led_modify_speed(filepath_speed, atol(argv[2]));
    led_modify_dir(filepath_dir, atol(argv[3]));
    
    return EXIT_SUCCESS;
}

void led_modify_speed(char* filepath, long speed)
{
    long buf;

    int fd = open(filepath, O_RDWR);
    if(fd < 0)
    {
        perror("Cannot open file");
        return;
    }

    ssize_t r = read(fd, &buf, 4);

    printf("Read : %ld, Buf : %ld\n", r, buf);

    ssize_t w = write(fd, &speed, 4);

    printf("Written : %ld\n", w);

    close(fd);
    
    return;
}

void led_modify_dir(char* filepath, long dir)
{
    long buf;

    int fd = open(filepath, O_RDWR);
    if(fd < 0)
    {
        perror("Cannot open file");
        return;
    }

    ssize_t r = read(fd, &buf, 4);

    printf("Read : %ld, Buf : %ld\n", r, buf);

    ssize_t w = write(fd, &dir, 4);

    printf("Written : %ld\n", w);

    close(fd);
    
    return;
}

void led_modify_pattern(char* filepath, long pattern)
{
    unsigned long buf = 0;

    int fd = open(filepath, O_RDWR);
    if(fd < 0)
    {
        perror("Cannot open file");
        return;
    }

    ssize_t r = read(fd, &buf, sizeof(buf));

    printf("Read : %ld, Buf : %ld\n", r, buf);

    ssize_t w = write(fd, &pattern, 4);

    printf("Written : %ld\n", w);

    close(fd);
    
    return;
}