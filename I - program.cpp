// ConsoleApplication3.cpp: Okre�la punkt wej�cia dla aplikacji konsoli.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <time.h>
#include <cmath>
#include <vector>
#include <windows.h>

LARGE_INTEGER start, stop;
unsigned __int64  start1, stop1;
using namespace std;

using namespace std;

class Dane {
	public:
		int iloscOper;				// Ilosc operacji
		int iloscMasz;				// Ilosc maszyn
		vector <int> techNast;		// Tablica nastepnikow technologicznych
		vector <int> maszNast;		// Tablica nastepnikow maszynowych
		vector <int> maszNastOst;	// Tablica wyliczonych nastepnikow maszynowych
		int *czasPracy;				// Tablica zawierajaca czas pracy
		vector <int> techPoprz;		// Tablica poprzednikow technologicznych
		vector <int> permutacja;	// Tablica zawierajaca permutacje
		vector <int> LP;			// Tablica LP
	    vector <int> PS;			// Tablica PS
	    int **harmonogram;			// Harmonogram
	    queue <int> kolejka;		// Kolejka
	    int iloscKolejka;			// Ilosc operacji obecnych w kolejce (ogolnie)
	    int ostOPer;				// Numer ostatniej operacji w harmonogramie
	    vector <int> sciezkaKryt;	// Tablica zawierajaca sciezke krytyczna
	    vector <int> bloki;			// Tablica zawierajaca wszystkie bloki
	    vector <int> blokiBezPoj;	// Tablica zawierajaca wszystkie bloki poza pojedynczymi

	    int zmPom;					// Zmienna pomocnicza
	    int zapPoz1;
	    int zapPoz2;
	public:


		void wczytajDane();			// Wczytywanie danych z pliku
		void zapisWyniku();			// Zapisanie wyniku do pliku
		void utworzLP();			// Tworzenie tablicy LP
		void utworzPS();			// Tworzenie tablicy PS
		int liczPoczOper( int nr_operacji);		// Wyliczanie poczatku operacji
		int doKolejki();			// Dodawanie do kolejki
		void redPoprzNast( int nr_operacji);	// Zmniejszenie tablicy poprzednikow/nastepnikow
		int liczHarm();				// Obliczanie harmonogramu
		void wyznaczNastMasz();		// Wyznaczanie nastepnikow maszynowych
		void wyznaczSciezBlok();	// Wyznaczanie sciezki krytycznej i blokow
		void usunBlokPoj();			// Usuwanie blokow pojedynczych
		void zamien();				// Zamiana dwoch losowych, sasiadujacych ze soba zadan
		int SA(float T, float lambda);	// Symulowane wyzarzanie
		float liczPrawd(int delta, float temperatura);	// Obliczanie prawdopodobienstwa
		~Dane();					// Destruktor
};

// Destruktor
Dane::~Dane(){


	techNast.clear();
	techPoprz.clear();
	maszNast.clear();
	maszNastOst.clear();
	delete [] czasPracy;
	permutacja.clear();
	LP.clear();
	for(int i=0; i<iloscOper+1; i++){
    	delete [] harmonogram[i];
    }
    delete [] harmonogram;	
}


// Wczytanie danych z pliku
void Dane:: wczytajDane(){
	char nazwaPliku[100];
	cout<<"Podaj nazwe pliku do wczytania:";
    cin>>nazwaPliku;
    ifstream plikWe;           
    plikWe.open(nazwaPliku);  
    // Sprawdzenie, czy wskazany plikWe istnieje
    if(!plikWe){            
         cout << "\nNie mozna otworzyc pliku!\n";
         exit(0);  
    }
    // Wczytanie ilosci operacji
    plikWe >> iloscOper;
    // Wczytanie do tablicy nastepnikow technologicznych
    techNast.resize(iloscOper+1);
    for(int i=1; i<=iloscOper; i++){
    	plikWe>>techNast[i];
    }
    // Wczytanie do tablicy nastepnikow maszynowych
    maszNast.resize(iloscOper+1);
    for(int i=1; i<=iloscOper; i++){
    	plikWe>>maszNast[i];
    }
    // Wczytanie do tablicy czasow wykonywania sie operacji
    czasPracy = new int[iloscOper+1];
    for(int i=1; i<=iloscOper; i++){
    	plikWe>>czasPracy[i];
    }
    // Wczytanie ilosci maszyn
    plikWe >> iloscMasz;
    // Wczytanie permutacji
    permutacja.resize(iloscOper+1+iloscMasz);
    permutacja[0] = 0;
    for(int j=1; j<(iloscOper+1+iloscMasz); j++){
    	plikWe>>permutacja[j]; 	
    }
    // Wyliczenie tablicy poprzednikow technologicznych
    techPoprz.resize(iloscOper+1);
    for(int i=1; i<=iloscOper; i++){
		techPoprz[i] = 0;
	}
    for(int i=1; i<=iloscOper; i++){
    	techPoprz[techNast[i]] = i;
    }
    // Utworzenie tablicy harmonogramu wypelnionej zerami
    harmonogram = new int*[iloscOper+1];
    for(int i=0; i<iloscOper+1; i++){
    	harmonogram[i] = new int [3]; 
    }
	// Utworzenie tablicy nast�pnikow maszynowych
	maszNastOst.resize(iloscOper+1);
	iloscKolejka = 0;
}

// Zapisanie wyniku obliczen do pliku
void Dane::zapisWyniku(){
	char nazwaPliku[100];
	cout<<endl<<"Podaj nazwe pliku, do ktorego zapisac harmonogram."<<endl;
	cin>>nazwaPliku;
	ofstream plikWy;
    plikWy.open(nazwaPliku, ios::app);
    plikWy.seekp(0, ios::end);
    for(int i=1; i<iloscOper+1; i++){
		for(int j=1; j<=2; j++){
			plikWy<<harmonogram[i][j]<<" ";
		}
		plikWy<<endl;
	}
	plikWy<<harmonogram[ostOPer][2];
	cout<<endl<<"Zapis zakonczony sukcesem!"<<endl<<endl;
	plikWy.close();
}

// Utworzenie tablicy z liczba poprzednikow
void Dane::utworzLP(){
	LP.resize(iloscOper+1);
	int iloscPoprzednikow = 0,spr;
	for(int i=1; i<=iloscOper; i++){
		// Sprawdzenie istnienia poprzednika technologicznego na podstawie tablicy poprzednikow
		if(techPoprz[i] != 0){
			iloscPoprzednikow++;
		}
		// Sprawdzenie istnienia poprzednika technologicznego na podstawie permutacji
		spr = 0;
		for(int j=1; j<=(iloscOper+iloscMasz); j++){
			if(permutacja[j] == i){
				spr = 1;
				if(permutacja[j-1] != 0){
					iloscPoprzednikow++;
				}	
				break;		
			}
		}	
		// Wpisanie liczby poprzednikow do tablicy LP
		LP[i] = iloscPoprzednikow;
		iloscPoprzednikow = 0;	
		if( spr == 0){
			LP[i] = 9;
		}	
	}
}


void Dane::utworzPS(){
	PS.resize(iloscOper+1);
	int pozycja, spr;
	// Petla przesuwa sie po tablicy PS
	for(int i=1; i<=iloscOper; i++){
		spr = 0;
		// Petla przesuwa sie po tablicy permutacji
		for(int j=1; j<(iloscOper+1+iloscMasz); j++){
			if( permutacja[j] == i ){
				PS[i] = j;
				spr = 1;
			}
		}
		if(spr == 0 ){
			PS[i] = 0;
		}		
	}
}


/* Funkcja zwraca wiekszy z czasow zakonczenia poprzednika technologicznego
   i poprzednika maszynowego operacji o numerze podanej jako argument */
int Dane::liczPoczOper( int nr_operacji ){
	if( PS[nr_operacji] == 0){
		return -9;
	}
	int tPoprz = techPoprz[nr_operacji];
	int tPoprzCzas = harmonogram[tPoprz][2];
	int mPoprz = permutacja[PS[nr_operacji]-1];
	int mPoprzCzas = harmonogram[mPoprz][2];
	int poczatekCzas = max(tPoprzCzas,mPoprzCzas);
	return poczatekCzas;
}

/* Funkcja przeglada tablice LP wyszukuj�c komorki o zawartosci 0 (czyli 
   operacje bez poprzednikow) i numery tych operacji dodaje do kolejki,
   natomiast w miejsce zera wpisuje 9. 
   Zwracane wartosci:
   					0 - dodano do kolejki operacje
   				    1 - nie dodano zadnej operacji (w tablicy LP nie ma
						 operacji bez poprzednikow)*/
int Dane::doKolejki(){
	int spr = 1;
	for(int i=1; i<=iloscOper; i++){
		if(LP[i] == 0){
			kolejka.push(i);
			LP[i] = 9;
			spr = 0;
		}
	}
	return spr;	
}


void Dane::redPoprzNast( int nr_operacji){
	LP[ techNast[nr_operacji] ]--;
	if(LP[ techNast[nr_operacji] ] == 0){
			kolejka.push(techNast[nr_operacji]);
	}
	LP[ maszNastOst[nr_operacji] ]--;
	if(LP[ maszNastOst[nr_operacji] ] == 0){
			kolejka.push(maszNastOst[nr_operacji]);
	}
}


void Dane::wyznaczNastMasz(){
	for(int i=1; i<=iloscOper; i++){
		if(PS[i] == 0){
			maszNastOst[i] = 0;
		}
		else{
			maszNastOst[i]=permutacja[PS[i]+1];
		}
	}
}

/* Funkcja wylicza harmonogram, oraz zapisuje w zmiennej ostOPer
   nr ostatniej operacji w harmonogramie. Funkcja zwraca:
   	      0 - harmonogram wyliczono poprawnie
   	      1 - nie da sie wyliczyc harmonogramu (bledne dane)*/
int Dane:: liczHarm(){
	//wyzerowuje tablice z harmonogramem
	for(int i=0; i<iloscOper+1; i++){
		for(int j=0; j<=2; j++){
			harmonogram[i][j] = 0;
		}
	}
	utworzLP();
	utworzPS();
	wyznaczNastMasz();
	int operacja, czas_p, czas_k, czas_max = -1;
	doKolejki();
	//while( !doKolejki() ){
		do {
			// Pobranie pierwszego elementu kolejki
			operacja = kolejka.front();
			// Usuniecie pierwszego elementu kolejki
			kolejka.pop();
			iloscKolejka++;
			czas_p = liczPoczOper( operacja );
			czas_k = czas_p + czasPracy[ operacja ];
			harmonogram[ operacja ][1] = czas_p;
			harmonogram[ operacja ][2] = czas_k;
			if(czas_k > czas_max){ 
				czas_max = czas_k;
				ostOPer = operacja;
			}
			
			redPoprzNast( operacja );
			
		} while( !kolejka.empty() );
	//}
	if( iloscKolejka != iloscOper ){
		return 1;
	}
	else{
		return 0;
	}	
}


void Dane:: wyznaczSciezBlok(){
	//////////// Sciezka krytyczna ////////////
	// Utworzenie pustej tablicy
	sciezkaKryt.resize(iloscOper+1);
	for(int i=1; i<=iloscOper; i++){
		sciezkaKryt[i] = 0;
	}
	// Analiza sciezki krytycznej od ostatniej operacji wg harmonogramu
	int aktZad = ostOPer;		// Analizowane zadanie
	int czas1;		// Czas zakonczenia poprzednika technologicznego badanej operacji
	int czas2;		// Czas zakonczeniapoprzednika maszynowego badanej operacji
	int pTech = 0, pMasz = 0, i = 1;
	// Szukanie sciezki krytycznej od ostatniej wykonywanej operacji
	while(1){
		czas1 = 0;   
		czas2 = 0;
		if(aktZad!=0){
				pMasz=permutacja[PS[aktZad]-1];
		}
		else{
			break;
		}	
		if (harmonogram[aktZad][1] == harmonogram[pMasz][2]){
			sciezkaKryt[i] = aktZad; 
			aktZad = pMasz;
		}
        else{
        	// Sprawdzenie poprzednika technologicznego badanej operacji
			pTech = techPoprz[aktZad];
			// Sprawdzenie, czy czas zakonczenia poprzednika techn. badanej
    		// operacji jest rowny czasowi jej rozpoczecia
    		if (harmonogram[aktZad][1] == harmonogram[pTech][2]){
    			sciezkaKryt[i] = aktZad; 
				aktZad = pTech;  	
    		}		
        }
		i++;
	} 
	
	i--;
		
    ////////// Bloki //////////

	bloki.resize((3*i)+1);
    for(int j=0; j<(3*i)+1; j++){
    	bloki[j] = 0;
    }

    bloki[1] = sciezkaKryt[1];
    int k = 2;					// Indeksndeks sciezke krytyczna
    int m = 2;					// Pusta komorka w tablicy bloki
    // Sprawdzenie, czy w sciezce jest wiecej niz jedna operacja
    if( sciezkaKryt[k] == 0 ){
    	bloki[2] = sciezkaKryt[1];
    }
    // Jezeli tak, to...
    else{
    	while( sciezkaKryt[k] != 0 ){
			if( sciezkaKryt[k-1] == techNast[sciezkaKryt[k]] ){ 
				bloki[m] = sciezkaKryt[k-1];
				m++;
				bloki[m] = 0;
				m++;
				bloki[m] = sciezkaKryt[k];
				m++;
				k++;
			}
			else{
				k++;
			}
    	}
    	bloki[m] = sciezkaKryt[k-1];
    	m++; 	
    }
    zmPom = m-1;
}

/* Funkcja przeglada tablice bloki i jest napotyka blok skladajacy sie z tylko
   jednego zadania, to go zeruje. */
void Dane::usunBlokPoj(){
   for(int i=1; i<=zmPom; i++){
   	if(bloki[i] != 0){
   			if( (bloki[i] == bloki[i+1]) ){
   	   			bloki[i] = 0;
   	   			bloki[i+1] = 0;
   	  		 }
   		}
   }

}

/* Funkcja losuje operacje poczatku lub konca bloku, w permutacji zamienia
   odpowiednio wylosowana operacje z nastepna lub poprzednia wykonujaca sie
   na maszynie */
void Dane::zamien(){
	int losPozycja, tmp;
	do{
	  losPozycja = 1 + int((zmPom-1+1) * rand()/(RAND_MAX+1.0));
	} while( bloki[losPozycja] == 0 );
	if( bloki[losPozycja-1] == 0){
		tmp = permutacja [ PS[ bloki[losPozycja] ] - 1 ];
		permutacja[PS[ bloki[losPozycja]] ] = tmp;
		permutacja [ PS[ bloki[losPozycja] ] - 1 ] = bloki[losPozycja];
		zapPoz1 = PS[ bloki[losPozycja] ] - 1;
		zapPoz2 = PS[ bloki[losPozycja]] ;
	}
	else{
		tmp = permutacja [ PS[ bloki[losPozycja] ] + 1 ];
		permutacja[PS[ bloki[losPozycja]] ] = tmp;
		permutacja [ PS[ bloki[losPozycja] ] + 1 ] = bloki[losPozycja];
		zapPoz1 = PS[ bloki[losPozycja] ] + 1;
		zapPoz2 = PS[ bloki[losPozycja]] ;
	}
	//delete [] bloki;
	//delete [] PS;
	//delete [] sciezkaKryt;
	//delete [] LP;
	zmPom = 0;
}

int Dane::SA( float T, float lambda){
	float p = 0;			// Prawdopodobienstwo
	float delta;
	
	int minCzas = harmonogram[ostOPer][2];
	int czasPoprz = minCzas;
	float wylosPrawd = 0;		// Przechowuje wylosowane prawdopodobienstwo
	
	do{
		wyznaczSciezBlok();
		usunBlokPoj();
		zamien();  
		liczHarm();
		if( harmonogram[ostOPer][2] <= minCzas){
			minCzas = harmonogram[ostOPer][2];
		}
		if( harmonogram[ostOPer][2] >= czasPoprz ){
			// Obliczanie delty
			delta = harmonogram[ostOPer][2] - czasPoprz;
			p = liczPrawd(delta, T);
			wylosPrawd=((float) rand() / (RAND_MAX));
			if(wylosPrawd>p){
				int tmp = permutacja[zapPoz1];
				permutacja[zapPoz1] = permutacja[zapPoz2];
				permutacja[zapPoz2] = tmp; 
				liczHarm();
			}
			else{
				czasPoprz = harmonogram[ostOPer][2];
			}			
		}
		else{
			czasPoprz = harmonogram[ostOPer][2];
		}			
		T = lambda * T;	
	} while (T>=1);


	return (minCzas);
}


float Dane::liczPrawd(int delta, float temperatura){
	float p=0;
	p=exp(-delta/temperatura);
	
	return p;
}


int _tmain(int argc, _TCHAR* argv[]){
	int czas;
	srand(time(NULL));
	Dane dane;
	

	dane.wczytajDane();	
	int spr = dane.liczHarm();
	float T=1200;
	float lambda=0.9998;
	if (spr == 0){ 
		//QueryPerformanceCounter(&start);
		czas = dane.SA(T,lambda);
		//QueryPerformanceCounter(&stop);
		cout<<"Najmniejszy czas: "<<czas<<endl;
		//start1 = start.QuadPart; 
    	//stop1 = stop.QuadPart;
    	//cout<<"Czas pracy : "<<(stop1 - start1)/1000000<<endl;
	}
	else{
		cout<<endl<<"Dane nieprawidlowe."<<endl<<"Program zakonczy dzialanie."<<endl;
	}
	//dane.zapisWyniku();

	system("pause");
	return 0;
}
