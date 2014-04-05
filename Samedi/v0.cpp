#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <list>

typedef struct {
	float lat;
	float lon;
} pos_t;

typedef struct {
	int a,b;
	bool mono;
	int cout;
	int score;

	bool parcourue;
} rue_t;

typedef struct {
	int temps_restant;
	int position;
	int provenance;
} voiture_t;

voiture_t *voitures;

int n_intersections, n_rues, temps_restant, n_voitures, origine;
pos_t *inter;
rue_t *rues;

typedef struct {
	int a;
	int cout;
	int score;
} dest_t;
std::list<dest_t> *dests;

double dist(int a, int b) {
	double d = 0;
	double tmp;
	tmp = inter[a].lon - inter[b].lon;
	tmp *= tmp;
	d+=tmp;

	tmp = inter[a].lat - inter[b].lat;
	tmp *= tmp;
	d+=tmp;

	return d;
}

double own_score(int a, dest_t d) {
	return (1.0*d.score)/(1.0*d.cout);
}

std::list<int> *trajet_voitures;

void voiture_goto(int voiture, int dest, int cout) {
	if(voitures[voiture].temps_restant) {
		fprintf(stderr, "FAILED\n");
		exit(1);
	}
	voitures[voiture].temps_restant = cout;
	voitures[voiture].provenance=voitures[voiture].position;
	voitures[voiture].position=dest;
	trajet_voitures[voiture].push_back(dest);
}

void decide_voiture(int source, std::list<dest_t>& d, int back) {
	double max_np=0;
	int max_np_id=0;
	for(auto& t: d) {
		if(t.a == back)
			continue;
		if(!rues[t.a].parcourue) {
			if(own_score(source, t) > max_np) {
				max_np = own_score(source, t);
				max_np_id = t.a;
			}
		}
	}
}

int main() {
	FILE* fp = fopen("paris_54000.txt", "r");
	fscanf(fp, "%d %d %d %d %d",
			&n_intersections,
			&n_rues,
			&temps_restant,
			&n_voitures,
			&origine);
	voitures = new voiture_t[n_voitures];
	bzero(voitures, sizeof(voiture_t) * n_voitures);

	trajet_voitures = new std::list<int>[n_voitures];

	inter = new pos_t[n_intersections];
	bzero(inter, sizeof(pos_t) * n_intersections);

	dests = new std::list<dest_t>[n_intersections];

	for(int i=0; i<n_intersections; ++i) {
		fscanf(fp, "%f %f", &inter[i].lat, &inter[i].lon);
	}

	rues = new rue_t[n_rues];
	bzero(rues, sizeof(rue_t)*n_rues);

	for(int i=0; i<n_rues; ++i) {
		int mono;
		fscanf(fp, "%d %d %d %d %d", &rues[i].a, &rues[i].b, &mono, &rues[i].cout, &rues[i].score);
		rues[i].mono = mono == 1;
		dest_t tmp;
		tmp.a = rues[i].b;
		tmp.cout = rues[i].cout;
		tmp.score = rues[i].score;
		dests[rues[i].a].push_front(tmp);
		if(!rues[i].mono) {
			tmp.a = rues[i].a;
			dests[rues[i].b].push_front(tmp);
		}
	}
	fclose(fp);

	while(--temps_restant) {
		for(int i=0; i<n_voitures; ++i) {
			if(voitures[i].temps_restant) {
				voitures[i].temps_restant--;
				continue;
			}

			auto& d = dests[voitures[i].position]; 
			decide_voiture(voitures[i].position, d, voitures[i].provenance);
		}
	}


	//Now display the result
	printf("%d\n", n_voitures);
	for(int i=0; i<n_voitures; ++i) {
		printf("%d\n", trajet_voitures[i].size());
		for(int t: trajet_voitures[i]) {
			printf("%d\n", t);
		}
	}
}
