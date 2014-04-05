#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

//Don't touch this
char *values = NULL;

typedef enum {
	NOT_PAINTED,
	PAINTED,
	TO_DEPAINT
} state_t;
state_t *states = NULL;
int w,h;
int score_square(int x, int y, int size) {
	int score = 0;
	for(int i=x-size; i <= x+size; ++i) {
		if(i < 0 || i >=w)
			return -1;
		for(int j=y-size; j <= y+size; ++j) {
			if(j<0 || j>=h)
				return -1;

			volatile int pos = i + j*w;
			if(states[pos] == NOT_PAINTED && values[pos])
				score++;
			if(!values[pos] &&  states[pos] != TO_DEPAINT)
				score -= 15;
		}
	}
	return score;
}

void apply_square(int x, int y, int size) {
	for(int i=x-size; i <= x+size; ++i) {
		if(i < 0 || i >w) {
			fprintf(stderr, "FAILED\n");
			exit(1);
		}
		for(int j=y-size; j <= y+size; ++j) {
			if(i < 0 || i >w) {
				fprintf(stderr, "FAILED\n");
				exit(1);
			}
			int pos = i + j*w;
			if(states[pos] == NOT_PAINTED && values[pos])
				states[pos]=PAINTED;
			if(!values[pos] &&  states[pos] != TO_DEPAINT) {
				states[pos] = TO_DEPAINT;
			}
		}
	}
	printf("PAINTSQ %d %d %d\n", y, x, size);
}

int main() {
	FILE* fp = fopen("doodle.txt", "r");
	fscanf(fp, "%d %d", &h, &w);

	values = malloc(h*w);
	bzero(values, w*h);

	states = malloc(h*w*sizeof(state_t));
	bzero(states, w*h*sizeof(state_t));

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
	fclose(fp);

	for(int size = 5; size>=0; size--) {
		int score_lim = 2 * size * size *2/3;
		for(int i=0; i < w; ++i) {
			for(int j=0; j < h; ++j) {
				int score = score_square(i, j, size);
				if(score > score_lim) {
					apply_square(i, j, size);
				}
			}
		}
	}
	for(int i=0; i < w; ++i) {
		for(int j=0; j < h; ++j) {
			if(states[i+j*w] == TO_DEPAINT) {
				printf("ERASECELL %d %d\n", j, i);
			}
		}
	}
}
