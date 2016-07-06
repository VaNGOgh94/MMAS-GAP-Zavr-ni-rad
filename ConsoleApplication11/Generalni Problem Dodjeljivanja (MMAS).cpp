#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <limits.h>
#include <random>
using namespace std;

int M;
int* *d;
long double* *feromoni;
long double tauMAX;
long double tauMIN;
long double param_a;
int iter_max;
long double param_RO;

int brojAgenata;
int brojPoslova;

struct Dodjela {
	int agent;
	int posao;
	//int cijena;
};

class Agent {
public:
	int radnoVrijeme;
	int *cijenePoslova;
}*agenti;

class Posao {
public:
	int trajanje;
}*poslovi;

class Mrav {
	private:
		//struktura posla samo sadrži opis posla, no kada pojedini mrav pokušava naæi rješenje potrebno mu je više informacija
		//sa kojima æe to realizirati pa unutar klase mrava postoji proširena struktura za posao
		struct PosaoZaRjesenje {
			Posao *posao;
			int rbrPosla;//buduæi da se polje "proširenih" poslova miješa (zbog nasumiènog redoslijeda) moramo pamtiti o kojem se poslu radi (drugi naèin bi bio da unutar same strukture "Posao" pamtimo redni broj posla)
			bool posaoDodjeljen;//ova bool varijabla indicira jel posaoDodjeljen - korišteno u prvim verzijama algoritma, sada je nepotrebno no možda zatreba
			PosaoZaRjesenje(): 
				posao(NULL), 
				posaoDodjeljen(false){
			};
		};
		//isto vrijedi i za agenta
		struct AgentZaRjesenje {
			Agent *agent;
			int preostaloVrijeme;
			int rbrAgenta;
			AgentZaRjesenje() :
				agent(NULL){
			};
		};
		/*
		int PronadiPosaoNajkracegTrajanja(PosaoZaRjesenje *posloviZaRjesenje) {
			int min = 0;
			//int vrij
				for (int i = 1; i<brojPoslova; i++) {
					if (!posloviZaRjesenje[i].posaoDodjeljen)
						if (poslovi[i].trajanje<poslovi[min].trajanje)
							min = i;
				}
			return min;
		}
		*/
		/*
		int DajNasumicniPosao(PosaoZaRjesenje *posloviZaRjesenje) {//metoda služi da se dobije "random" NEDODIJELJENI posao kojeg æemo dodijeliti negdje
			int ind = 0;
			do {
				ind = rand() % brojPoslova;
			}while (posloviZaRjesenje[ind].posaoDodjeljen);
			return ind;
		}
		*/
		//metoda miješa poslove (dakle simulira "nasumièni odabir")
		void PromjesajPoslove(PosaoZaRjesenje *posloviZaRjesenje){
			int j;
			PosaoZaRjesenje pom;
			for(int i=0; i<brojPoslova-1; i++){
				j = rand() % brojPoslova;
				pom = posloviZaRjesenje[i];
				posloviZaRjesenje[i] = posloviZaRjesenje[j];
				posloviZaRjesenje[j] = pom;
			}
		}
		//metoda evidentira dodjelu, i smješta agenta udesno na njegovo mjesto (ovisno o preostalom kapacitetu)
		//ovo je najisplativiji naèin pomoæu kojega "tražimo" odnosno "dobivamo" skup agenata koji imaju kapaciteta za tekuæi posao
		void EvidentirajDodjelu(PosaoZaRjesenje tekuciPosao, int iteracija, AgentZaRjesenje *agentiZaRjesenje, int odabraniAgent) {
			rjesenje[iteracija].posao = tekuciPosao.rbrPosla;
			rjesenje[iteracija].agent = agentiZaRjesenje[odabraniAgent].rbrAgenta;
			int k = odabraniAgent;
			agentiZaRjesenje[k].preostaloVrijeme -= tekuciPosao.posao->trajanje;
			this->trosakRjesenja += agentiZaRjesenje[k].agent->cijenePoslova[tekuciPosao.rbrPosla];
			//POSTAVI AGENTA NA NJEGOVO MJESTO (šaljemo ga desno jer mu se kapacitet smanjio, polje treba biti sortirano od onih sa najviše kapaciteta prema onima sa najmanje)
			if (k == brojAgenata - 1)
				return;//ako se veæ nalazi na zadnjem (najdesnijem) mjestu nema potrebe da ga šaljemo desno (ne bi ni mogli, algoritam premještanja bi se rušio)

			//vlastiti algoritam premještanja, radi po uzoru na sortiranje umetanjem
			int t = k + 1;
			AgentZaRjesenje pom = agentiZaRjesenje[k];
			while (t < brojAgenata && agentiZaRjesenje[t].preostaloVrijeme > pom.preostaloVrijeme)
				agentiZaRjesenje[t - 1] = agentiZaRjesenje[t++];
			agentiZaRjesenje[t - 1] = pom;


			//ako imamo polje:
			//0.  1.  2.  3.  4.  5.
			//29  21  20  15  12  10  
			//29  21  11  15  12  10 //npr. agent na indeksu 2 dobije posao koji mu uzme 9 radnih sati pa mu ostane samo 11

			//iter1
			//t = 2 + 1 = 3
			//pom = a[2] (11)
			//t<brojAgenata [3<6 = TRUE]
			//&& a[3] (15) > pom(11)  [15>11 = TRUE]
			//a[t-1] = a[t] -> a[2] = a[3]:
			//0.  1.  2.  3.  4.  5.
			//29  21  15  15  12  10

			//iter2
			//t++ -> t = 4
			//pom = a[2] (11) (pom ostaje isti kroz cijelo premještanje)
			//t<brojAgenata [4<6 = TRUE]
			//	&& a[4] (12) > pom(11)  [12>11 = TRUE]
			//a[t-1] = a[t] -> a[3] = a[4]:
			//0.  1.  2.  3.  4.  5.
			//29  21  15  12  12  10

			//iter3
			//t++ -> t = 5
			//pom = a[2] (11) (pom ostaje isti kroz cijelo premještanje)
			//t<brojAgenata [5<6 = TRUE]
			//	&& a[5] (10) > pom(11)  [10>11 = FALSE]
			//PREKID PETLJE
			//a[t-1] = pom -> a[4] = pom(11)
			//0.  1.  2.  3.  4.  5.
			//29  21  15  12  11  10
		}
		
	public:
		//static poslovi i agenti - da ne bi svaki put vezao redne brojeve i slièno, napravim to jednom a kasnije samo kopiram po potrebi (memcpy)
		static PosaoZaRjesenje *inicijalniPoslovi;
		static AgentZaRjesenje *inicijalniAgenti;
		static void PostaviStatickaPolja() {
			inicijalniPoslovi = new PosaoZaRjesenje[brojPoslova];
			for (int i = 0; i < brojPoslova; i++) {
				inicijalniPoslovi[i].posao = &poslovi[i];
				inicijalniPoslovi[i].rbrPosla = i;
			}

			inicijalniAgenti = new AgentZaRjesenje[brojAgenata];
			for (int i = 0; i < brojAgenata; i++) {
				inicijalniAgenti[i].agent = &agenti[i];
				inicijalniAgenti[i].rbrAgenta = i;
				inicijalniAgenti[i].preostaloVrijeme = inicijalniAgenti[i].agent->radnoVrijeme;
			}
		}

		Dodjela *rjesenje;
		bool nesiplativoRjesenje;
		int trosakRjesenja;
		PosaoZaRjesenje *posloviZaRjesenje;
		AgentZaRjesenje *agentiZaRjesenje;
		long double *granice;
		

		long double IzracunajSumuFeromona(int agent, bool *posaoDodjeljen) {
			long double suma = 0;
			for (int i = 0; i<brojPoslova; i++)
				if (posaoDodjeljen[i]) continue;
				else suma += feromoni[agent][i];
				return suma;
		}

		void GenerirajRjesenje() {
			//priprema algoritma
			this->nesiplativoRjesenje = false;
			this->trosakRjesenja = 0;

			memcpy(posloviZaRjesenje, this->inicijalniPoslovi, sizeof(PosaoZaRjesenje)*brojPoslova);//kopiramo poslove iz statièkih polja
			PromjesajPoslove(posloviZaRjesenje);//miješamo poslove
			memcpy(agentiZaRjesenje, this->inicijalniAgenti, sizeof(AgentZaRjesenje)*brojAgenata);//kopiramo agente iz statièkih polja
			
			long double nasumicniBroj;
			mt19937 gen(1701);//generator brojeva

			for (int i = 0; i < brojPoslova; i++) {//poèetak algoritma
				int j = 0;//j nam služi kao brojaè i za granicu agenata
				granice[j] = 0;
				//granica = 0;
				while (j < brojAgenata && agentiZaRjesenje[j].preostaloVrijeme >= posloviZaRjesenje[i].posao->trajanje) {
					//vjerojatnosti[j] = feromoni[agentiZaRjesenje[j++].rbrAgenta][i];
					granice[j+1] = granice[j] + feromoni[agentiZaRjesenje[j].rbrAgenta][i];
					//granica += feromoni[agentiZaRjesenje[j].rbrAgenta][i];
					j++;
				}
				
				if (j == 0) {//ukoliko se nijednom agentu ne može dodijeliti ovaj posao (jer nemaju kapaciteta):
					EvidentirajDodjelu(posloviZaRjesenje[i], i, agentiZaRjesenje, rand()%brojAgenata);
					this->nesiplativoRjesenje = true;
				}

				uniform_real_distribution<> distr(0, granice[j]);//postavljamo granice generiranja broja (od 0 do zadnje granice)
				nasumicniBroj = distr(gen);//generiramo
				
				for (int k = 0; k < j; k++) {
					if (nasumicniBroj <= granice[k+1]) {//granice[0] je prazno pa stoga gledamo k+1 od granica
						EvidentirajDodjelu(posloviZaRjesenje[i], i, agentiZaRjesenje, k);
						break;
					}
				}
			}
		}

		Mrav() {
			rjesenje = new Dodjela[brojPoslova];
			granice = new long double[brojAgenata + 1];
			posloviZaRjesenje = new PosaoZaRjesenje[brojPoslova];
			agentiZaRjesenje = new AgentZaRjesenje[brojAgenata];
		};

		~Mrav() {
			delete [] rjesenje; 
			delete[] posloviZaRjesenje;
			delete[] agentiZaRjesenje;
			delete[] granice;
		}
};

Mrav::PosaoZaRjesenje *Mrav::inicijalniPoslovi;//deklaracija statièkih atributa (pokazivaèa) klase Mrav
Mrav::AgentZaRjesenje *Mrav::inicijalniAgenti;
Mrav *mravi;

void IspisiStanjeFeromona() {
	cout << endl;
	cout << "    ";
	for (int i = 0; i<brojPoslova; i++)
		cout << "P" << i + 1 << "   ";
	cout << endl << endl;;
	for (int i = 0; i<brojAgenata; i++) {
		cout << "A" << i + 1 << "  ";
		for (int j = 0; j<brojPoslova; j++)
			cout << feromoni[i][j] << "  ";
		cout << endl;
	}
	cout << endl;
}

void SortirajAgente() {//sortira agente od onog sa najmanjim radnim vremenom prema onom sa najveæim radnim vremenom (bubble sort)
	for (int i = brojAgenata; i>0; i--) {
		for (int j = 1; j<i; j++) {
			if (agenti[j - 1].radnoVrijeme<agenti[j].radnoVrijeme) {
				Agent pom = agenti[j - 1];
				agenti[j - 1] = agenti[j];
				agenti[j] = pom;
			}
		}
	}
}

int PronadiNajboljegMrava() {
	int max = 0;
	for (int i = 1; i<M; i++)
		if (mravi[i].trosakRjesenja < mravi[max].trosakRjesenja) {
			max = i;
		}
	return max;
}

void SimulirajHlapljenjeFeromona() {
	for (int i = 0; i < brojAgenata; i++) {
		for (int j = 0; j < brojPoslova; j++) {
			if (feromoni[i][j] * (1 - param_RO) < tauMIN)
				feromoni[i][j] = tauMIN;
			else
				feromoni[i][j] *= (1 - param_RO);
		}
	}
}

void NagradiNajboljeg(int najboljiMrav) {
	Dodjela *rjesenje = mravi[najboljiMrav].rjesenje;
	if (mravi[najboljiMrav].nesiplativoRjesenje) {
		for (int i = 0; i < brojPoslova; i++) {
			if (feromoni[rjesenje[i].agent][rjesenje[i].posao] + 0.01 > tauMAX)
				feromoni[rjesenje[i].agent][rjesenje[i].posao] = tauMAX;
			else
				feromoni[rjesenje[i].agent][rjesenje[i].posao] += 0.01;
		}
	}

	else {
		for (int i = 0; i < brojPoslova; i++) {
			if (feromoni[rjesenje[i].agent][rjesenje[i].posao] + 0.05 > tauMAX)
				feromoni[rjesenje[i].agent][rjesenje[i].posao] = tauMAX;
			else
				feromoni[rjesenje[i].agent][rjesenje[i].posao] += 0.05;
		}
	}
}

Dodjela *najboljeRjesenjeDosad;
int najmanjiTrosakDosad;

void init() {
	/*
		tauMAX = 0.7;
		tauMIN = 0.22;
	*/
	param_RO = 0.02;
	param_a = 1;

	brojAgenata = 2;
	agenti = new Agent[brojAgenata];
	agenti[0].radnoVrijeme = 45;
	agenti[1].radnoVrijeme = 33;


	brojPoslova = 6;
	poslovi = new Posao[brojPoslova];
	poslovi[0].trajanje = 6;
	poslovi[1].trajanje = 3;
	poslovi[2].trajanje = 4;
	poslovi[3].trajanje = 8;
	poslovi[4].trajanje = 9;
	poslovi[5].trajanje = 11;

	agenti[0].cijenePoslova = new int[brojPoslova];
	agenti[0].cijenePoslova[0] = 2;
	agenti[0].cijenePoslova[1] = 4;
	agenti[0].cijenePoslova[2] = 3;
	agenti[0].cijenePoslova[3] = 8;
	agenti[0].cijenePoslova[4] = 5;
	agenti[0].cijenePoslova[5] = 18;

	agenti[1].cijenePoslova = new int[brojPoslova];
	agenti[1].cijenePoslova[0] = 8;
	agenti[1].cijenePoslova[1] = 14;
	agenti[1].cijenePoslova[2] = 3;
	agenti[1].cijenePoslova[3] = 22;
	agenti[1].cijenePoslova[4] = 4;
	agenti[1].cijenePoslova[5] = 6;
	feromoni = new long double*[brojAgenata];//alociramo pokazivaèe za redove
	for (int i = 0; i<brojAgenata; i++) {
		feromoni[i] = new long double[brojPoslova];//alociramo redove pomoæu pokazivaèa
		for (int j = 0; j<brojPoslova; j++)//odmah prilikom alokacije
			feromoni[i][j] = 0.99;//iniciramo sve vrijednosti matrice na tauMAX
	}
	IspisiStanjeFeromona();


	M = 10;
	mravi = new Mrav[M];

	SortirajAgente();//pomaže pri ubrzanju algoritma kasnije
	//N^k¢i (skup agenata kojima se može dodijeliti trenutni posao) je najlakše odrediti
	//ako je polje agenata sortirano prema preostalom kapacitetu, stoga se odmah nakon uèitavanja agenata to polje sortira

	Mrav::PostaviStatickaPolja();//nakon što su agenti sortirani, možemo postaviti statièka "inicijalna (proširena) polja"
	najboljeRjesenjeDosad = new Dodjela[brojPoslova];
	//poèetna iteracija (koja služi za postavljanje tauMAX i shodno tome tauMIN)
	for (int i = 0; i < M; i++) {
		mravi[i].GenerirajRjesenje();
	}
	IspisiStanjeFeromona();
	int najboljiMravDosad = PronadiNajboljegMrava();
	memcpy(najboljeRjesenjeDosad, mravi[najboljiMravDosad].rjesenje, sizeof(Dodjela)*brojPoslova);
	najmanjiTrosakDosad = mravi[najboljiMravDosad].trosakRjesenja;
	
	tauMAX = 1 / (param_RO * najmanjiTrosakDosad);
	tauMIN = 1 / (tauMAX*param_a);
	IspisiStanjeFeromona();
	for (int i = 0; i < brojAgenata; i++) {
		for (int j = 0; j < brojPoslova; j++) {
			feromoni[i][j] = tauMAX;
		}
	}

	cout << "NAJMANI TROSAK U INIT = " << najmanjiTrosakDosad << endl;
	cout << "tauMAX = " << tauMAX << endl;
	cout << "tauMIN = " << tauMIN << endl;
	cout << "param_RO = " << param_RO << endl;
	cout << "param_a = " << param_a << endl;

	IspisiStanjeFeromona();
}

int main() {
	
	srand(time(0) % 32768);
	rand();
	init();
	cout << "Koliko iteracija želite: ";
	cin >> iter_max;

	int najboljiMravIteracije;
	for (int iter = 1; iter<iter_max; iter++) {
		for (int m = 0; m<M; m++) {
			mravi[m].GenerirajRjesenje();
		}
		
		najboljiMravIteracije = PronadiNajboljegMrava();
		cout << "Najbolje rješenje iteracije: " << mravi[najboljiMravIteracije].trosakRjesenja<<endl;

		if (mravi[najboljiMravIteracije].trosakRjesenja < najmanjiTrosakDosad) {//ako je rješenje iteracije bolje od najboljeg dosada:
			memcpy(najboljeRjesenjeDosad, mravi[najboljiMravIteracije].rjesenje, sizeof(Dodjela)*brojPoslova);//zapisujemo to rješenje
			najmanjiTrosakDosad = mravi[najboljiMravIteracije].trosakRjesenja;//bilježimo nejgov rezultat
			tauMAX = 1 / (param_RO*najmanjiTrosakDosad);//podešavamo tauMAX i tauMIN
			tauMIN = 1 / (tauMAX*param_a);
			cout << "Imamo novo najbolje dosadašnje rješenje iteracije: " << najmanjiTrosakDosad <<endl;
		}
		SimulirajHlapljenjeFeromona();
		NagradiNajboljeg(najboljiMravIteracije);
		IspisiStanjeFeromona();
	}
	cout << "------------------------------" << endl;
	cout << "Zadnje rješenje algoritma iznosi: " << najmanjiTrosakDosad << endl;
	cout << endl << endl;

	delete[] najboljeRjesenjeDosad;
	for (int i = 0; i < brojAgenata; i++) {
		delete[] agenti[i].cijenePoslova;
		delete[] feromoni[i];
	}
	delete[] feromoni;
	delete[] poslovi;
	delete[] mravi;
	delete[] Mrav::inicijalniAgenti;
	delete[] Mrav::inicijalniPoslovi;
	system("pause");
	return 0;
}
