#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	float lat;
	float lon;
} pos_t;

typedef struct {
	int a,b;
	bool mono;
	int cout;
	int score;
} rue_t;

int n_intersections, n_rues, temps_restant, n_vehicules, origine;
int main() {
	FILE* fp = fopen("paris_54000.txt", "r");
	fscanf(fp, "%d %d %d %d %d",
			&n_intersections,
			&n_rues,
			&temps_restant,
			&n_vehicules,
			&origine);

	pos_t *inter = malloc(sizeof(pos_t) * n_intersections);
	bzero(pos_inter, sizeof(pos_t) * n_intersections);

	for(int i=0; i<n_intersections; ++i) {
		fscanf(fp, "%f %f", &pos_inter[i].lat, &pos_inter[i].lon);
	}

	rue_t rues = malloc(sizeof(rue_t)*n_rues);
	bzero(rues, sizeof(rue_t)*n_rues);

	for(int i=0; i<n_rues; ++i) {
		int mono;
		fscanf(fp, "%d %d %d %d %d", &rues[i].a, &rues[i].b, &mono, &rues[i].cout, &rues[i].score);
		rues[i].mono = mono == 1;
	}

	fclose(fp);
}
