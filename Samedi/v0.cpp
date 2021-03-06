#include <stdio.h>
#include <math.h>
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
	int temps_arrive;
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

/*
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
};*/

pos_t interesting[] = {
	//Place d'italie
	{ 48.8314830, 2.3556920},
	//Lecourbe
	{ 48.8409670, 2.2958170},
	//Assas
	{ 48.8468840, 2.3318660 },
	//Trocadero
	{ 48.8638670, 2.2888090 },
	//Clichy
	{ 48.8835876, 2.3271952},
	//Riquet
	{ 48.8888890, 2.3736950 },
	//Nation
	{ 48.8483940, 2.3959100 },
	//Chatelet
	{ 48.8550160, 2.3552420 }
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

	return sqrt(d);
}

float compute_coef_simp(int id, int dest1, int dest2, int rue) {
	double coef=1;

	double d1,d2;

	d1 = disti(id, dest1);
	d2 = disti(id, dest2);
	if( d1 < 0.001 && !voitures[id].temps_arrive) {
		voitures[id].temps_arrive = temps_restant;
	}
	if(!voitures[id].temps_arrive) {
		coef *= exp( (d1-d2 ) * 8000);
	} /*else {
		if( (d1) > 0.02) {
	  		coef *= exp( (d1-d2 ) * 1000);
		}
	}*/
	if(!rues[rue].parcourue)
	  coef += (54000 - temps_restant) ;
	return coef;
}

float compute_coef_depth(int id, int src, int dest, int rue, int level=8, std::list<int> browsed=std::list<int>()) {
	double coef_max=0.2;

	for(auto& t: dests[dest]) {
		double coef = compute_coef_simp(id, dest, t.a, t.rue);
		bool stop = false;
		//loop
		for(auto& v: browsed) {
			if(t.a == v) {
				stop = true;
			}
		}
		if(stop)
			continue;

		browsed.push_back(t.a);
		if(level)
			coef += compute_coef_depth(id, dest, t.a, t.rue, level-1, browsed);

		browsed.pop_back();
		if(coef > coef_max)
			coef_max = coef;
	}
	return coef_max*0.7 + compute_coef_simp(id, voitures[id].position, dest, rue);
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

double own_score(int id, int a, dest_t d) {
	return 1;
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
	double max_np=-999999999;
	int max_np_id=-1;
	int max_np_dest=-1;
	int max_np_cout=-1;
	int max_np_rue=-1;

	int r = rand()%d.size();
	int i = 0;
	int dest_rand = 0;
	int cout_rand = -1;
	int rue_rand = 0;

	int dest_min = 0;
	int score_min = -1;
	int cout_min = -1;
	int rue_min = 0;

	std::list<double> scores;

	dest_t max_np_d;
	for(auto& t: d) {
		//Random part
		if(r == i) {
			dest_rand = t.a;
			cout_rand = t.cout;
			rue_rand = t.rue;
		}
		++i;

		if(score_min == -1 || t.score < score_min) {
			score_min = t.score;
			dest_min = t.a;
			cout_min = t.cout;
			rue_min = t.rue;
		}

		//Pour les voitures impaires, au debut, on privilegie les longues routes (periph)
		double coef = 1.0;

		coef = compute_coef_depth(id, voitures[id].position, t.a, t.rue);
		scores.push_back(coef);
		if(t.a == back)
			continue;

		if(rues[t.rue].parcourue < 8) {
			coef /= 1 + (rues[t.rue].parcourue+1);
			if((coef*own_score(id, source, t)) > max_np) {
				max_np = own_score(id, source, t)*coef;
				max_np_id = t.a;

				max_np_d = t;
			}
		}
	}
	if(max_np_id == -1) {
		int ran = rand()%10;
#if 0
		double sum = 0;
		for(auto v: scores)
			sum+=v;

		double rand_v = rand();
		rand_v /= RAND_MAX;
		rand_v *= sum;

		double tmp = 0;
		auto u = d.begin();
		auto v = scores.begin();
		while(tmp<rand_v) {
			tmp += *v;
			if(tmp>=rand_v)
				break;
			++u;
			++v;
		}
		voiture_goto(id, u->a, u->cout, u->rue);
#endif
		/*
		if(ran < 3)
			voiture_goto(id, dest_min, cout_min, rue_min);
		else*/
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
