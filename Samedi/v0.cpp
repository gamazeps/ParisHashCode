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
pos_t *inter;
rue_t *rues;

struct {
	int a;
	int cout;
	int score;
} dest_t;
std::list<dest_t> *dests = 

int main() {
	FILE* fp = fopen("paris_54000.txt", "r");
	fscanf(fp, "%d %d %d %d %d",
			&n_intersections,
			&n_rues,
			&temps_restant,
			&n_vehicules,
			&origine);

	inter = new pos_t[n_intersections];
	bzero(inter, sizeof(pos_t) * n_intersections);

	for(int i=0; i<n_intersections; ++i) {
		fscanf(fp, "%f %f", &inter[i].lat, &inter[i].lon);
	}

	rues = new rue_t[n_rues];
	bzero(rues, sizeof(rue_t)*n_rues);

	for(int i=0; i<n_rues; ++i) {
		int mono;
		fscanf(fp, "%d %d %d %d %d", &rues[i].a, &rues[i].b, &mono, &rues[i].cout, &rues[i].score);
		rues[i].mono = mono == 1;
	}

	fclose(fp);
}
