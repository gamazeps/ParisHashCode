#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int main() {
	FILE* fp = fopen("doodle.txt", "r");
	int w,h;
	fscanf(fp, "%d %d", &h, &w);
	char *values = malloc(h*w);
	for(int i=0; i < (w*h); ) {
		char c;
		fread(&c, 1, 1, fp);
		if(c == '#') {
			values[i] = 1;
			++i;
		}
		if(c == '.')
			++i;
	}
}
