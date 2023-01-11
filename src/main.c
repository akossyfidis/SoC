#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>


int main()
{
    char * filepath = "/dev/mem";
        
    printf("Hello World !\n");
    
    map(filepath);

    return EXIT_SUCCESS;
}

void map(char* filepath)
{
    int fd = open(filepath, O_RDWR);
    if(fd < 0)
    {
        perror("Cannot open file");
        return EXIT_FAILURE;
    }

    char* data = mmap (NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0xFF203000);
    
    if(data == MAP_FAILED)
    {
        printf("Shared Mapping Failed\n");
        return EXIT_FAILURE;
    }   

    printf("%s\n", data);

    char* status = munmap(0xFF203000, 4);  
    if(status == MAP_FAILED)
    {
        perror("Unmap Failed\n");
        return EXIT_FAILURE;
    }

    printf("%s\n", data);

    close(fd);
    
    return EXIT_SUCCESS;
}