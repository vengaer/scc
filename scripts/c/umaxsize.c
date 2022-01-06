#include <stdint.h>
#include <stdio.h>

int main(int argc, char **argv) {
    FILE *fp = stdout;
    if(argc > 1) {
        fp = fopen(argv[1], "w");
        if(!fp) {
            perror("fopen");
            return 1;
        }
    }
    fprintf(fp, "uintmax_t_size := %zu\n", sizeof(uintmax_t));
    if(argc > 1) {
        fclose(fp);
    }
    return 0;
}
