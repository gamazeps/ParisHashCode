#include <stdio.h>
#include <time.h>
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

	int parcourue;
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
	int rue;
} dest_t;
std::list<dest_t> *dests;

pos_t interesting[] = {
	//Place d'italie
	{ 48.8314830, 2.3556920},
	//Denfert
	{ 48.8338640, 2.3326150},
	//Rue du bac
	{ 48.8559550, 2.3255770 },
	//Etoile
	{ 48.8743062, 2.2947665 },
	//Clichy
	{ 48.8835876, 2.3271952},
	//Stalingrad
	{ 48.8840350, 2.3684600 },
	//Nation
	{ 48.8483940, 2.3959100 },
	//Chatelet
	{ 48.8587060, 2.3474560 }
};

double disti(int voiture, int a) {
	double d = 0;
	double tmp;
	tmp = inter[a].lon - interesting[voiture].lon;
	tmp *= tmp;
	d+=tmp;

	tmp = inter[a].lat - interesting[voiture].lat;
	tmp *= tmp;
	d+=tmp;

	return d;
}

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

void voiture_goto(int voiture, int dest, int cout, int rue) {
	if(voitures[voiture].temps_restant) {
		fprintf(stderr, "FAILED\n");
		exit(1);
	}
	voitures[voiture].temps_restant = cout;
	voitures[voiture].provenance=voitures[voiture].position;
	voitures[voiture].position=dest;
	trajet_voitures[voiture].push_back(dest);
	rues[rue].parcourue++;
}

void decide_voiture(int id, int source, std::list<dest_t>& d, int back) {
	double max_np=0;
	int max_np_id=-1;
	int max_np_dest=-1;
	int max_np_cout=-1;
	int max_np_rue=-1;

	int r = rand()%d.size();
	int i = 0;
	int dest_rand = 0;
	int cout_rand = -1;
	int rue_rand = 0;
	dest_t max_np_d;
	for(auto& t: d) {
		//Random part
		if(r == i) {
			dest_rand = t.a;
			cout_rand = t.cout;
			rue_rand = t.rue;
		}
		++i;

		if(t.a == back)
			continue;
		//Pour les voitures impaires, au debut, on privilegie les longues routes (periph)
		double coef = 1.0;
		/*
		if(temps_restant > 20000 && (id&1)) {
			if(rues[t.rue].score > 300) {
				coef *= 5;
			}
		}
		*/
#if 0
		float good_way = 1.4;
		float bad_way = 0.8;

		float bad_way2 = 0.8;

		float lat_orig = inter[voitures[id].position].lat;
		float lat_dest = inter[t.a].lat;

		float lat_mid = 48.83;

		if( id&1) {
			if(lat_orig > lat_mid) {
				if(lat_dest > lat_orig)
					coef *= bad_way;
				else
					coef *= good_way;
			} else {
				//On est du bon cote
				//Mais trop loin
				if(lat_orig < 48.83)
					coef *= bad_way2;
			}
		} else {
			if(lat_orig < lat_mid) {
				if(lat_dest < lat_orig)
					coef *= bad_way;
				else
					coef *= good_way;
			} else {
				if(lat_orig > 48.89)
					coef *= bad_way2;
			}
		}

		float lon_orig = inter[voitures[id].position].lon;
		float lon_dest = inter[t.a].lon;
		if( (id>>1)&1) {
			if(lon_orig > 2.32) {
				if(lon_dest > lon_orig)
					coef *= bad_way;
				else
					coef *= good_way;
			} else {
				if(lon_orig < 2.29)
					coef *= bad_way2;
			}
		} else {
			if(lon_orig < 2.32) {
				if(lon_dest < lon_orig)
					coef *= bad_way;
				else
					coef *= good_way;
			} else {
				if(lon_orig < 2.35)
					coef *= bad_way2;
			}
		}
#endif
		double d1,d2;
		d1 = disti(id, voitures[id].position);
		d2 = disti(id, voitures[id].position);
		if(d1 > 1500) {
			if( (d2-d1) > 50) {
				coef *= 0.5;
			} else {
				coef *= 2.0;
			}
		}

		if(rues[t.rue].parcourue < 1) {
			coef /= ( 1 << rues[t.rue].parcourue);
			if((coef*own_score(source, t)) > max_np) {
				max_np = own_score(source, t)*coef;
				max_np_id = t.a;

				max_np_d = t;
			}
		}
	}
	if(max_np_id == -1) {
		voiture_goto(id, dest_rand, cout_rand, rue_rand);
	} else {
		voiture_goto(id, max_np_d.a, max_np_d.cout, max_np_d.rue);
	}
}

int main() {
	srand(time(NULL));
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
	for(int i=0; i<n_voitures; ++i) {
		trajet_voitures[i].push_back(origine);
		voitures[i].provenance=-1;
		voitures[i].position=origine;
	}


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
		tmp.rue = i;
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
			decide_voiture(i, voitures[i].position, d, voitures[i].provenance);
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

	int total = 0;
	for(int i=0; i< n_rues; ++i) {
		if(rues[i].parcourue)
			total += rues[i].score;
	}
	fprintf(stderr, "score = %d\n", total);
}
