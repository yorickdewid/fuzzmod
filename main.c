#include <stdio.h>
#include <stdlib.h>

void usage(const char *progname) {
    printf("%s is an network fuzzer\n\n", progname);
    printf("Usage:\n  %s [OPTION]... HOST\n", progname);
}

int main(int argc, char *argv[]) {
    usage(argv[0]);
    return 0;
}
