// Alvaro Jover Alvarez @vorixo
// Jordi Amoros Moreno @byFlowee
/* 
We are the original authors, we've seen our code on various repositories
due to a code sharing we did while ago
*/

// FILTRO MEDIANO + GAUSSIANO

#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <sys/time.h>
#include "EasyBMP.h"

using namespace std;

// Constantes para filtro gaussiano
const double PI = 3.141592654;

// Tamanyo de imagen
int WIDTH;
int HEIGHT;

// String es numero
bool is_digits(const std::string &str)
{
	return str.find_first_not_of("0123456789") == string::npos;
}

// Aplicar Gaussian Filter unidireccional
float Gaussian1D(int x, int sigma) {
	return exp(-x * x / (2 * sigma * sigma)) / sigma / sqrt(2 * PI);
}

int main() {


	// Variable imagen
	BMP Image;
	BMP Mediana;
	BMP gausshori,gaussverti;
	BMP Media;

	// Input para imagen
	string str;
	cout << "Introduce el nombre de la imagen: ";
	getline(cin, str);
	const char *nombreimagen = str.c_str();

	// Leemos la imagen
	if (Image.ReadFromFile(nombreimagen)) {

		// Variables no relevantes
		string toInt = "";
		bool fallo = false;

		// Datos de la imagen
		WIDTH = Image.TellWidth();
		HEIGHT = Image.TellHeight();
		Mediana.SetSize(WIDTH, HEIGHT);
		Mediana.SetBitDepth(Image.TellBitDepth()); // cambiar bitdepth al de input
		gausshori.SetSize(WIDTH, HEIGHT);
		gausshori.SetBitDepth(Image.TellBitDepth());
		gaussverti.SetSize(WIDTH, HEIGHT);
		gaussverti.SetBitDepth(Image.TellBitDepth());
		Media.SetSize(WIDTH, HEIGHT);
		Media.SetBitDepth(Image.TellBitDepth());

		cout << "Dimensiones imagen: " << WIDTH << " x " << HEIGHT << endl;

		// Variables de dependencia del algoritmo
		int dim = 0;

		// -----------------------------------------------------
		cout << "Introduce dimension de la malla (impar): ";

		//Pasar la entrada a int
		getline(cin, toInt);

		if (is_digits(toInt)) {
			istringstream ss(toInt);
			ss >> dim;

			if (dim % 2 == 0) {
				fallo = true;
			}
		}

		// Manejo de errores (cortamos la ejecucion)
		if (fallo) {
			cout << "Error: Introduce un numero impar." << endl;
			getchar();
			return 0;
		}

		// Numero de valores que contendra la malla
		int dimord = dim*dim;
		int half = dim/2;

		// -----------------------------------------------------
		cout << "Fuerza del filtro [1-100] (Median Filter): ";
		int fuerza = 0;

		// Pasar la entrada a int
		getline(cin, toInt);

		if (is_digits(toInt)) {
			istringstream ss(toInt);
			ss >> fuerza;
		}

		// Manejo de errores (cortamos la ejecucion)
		if (fuerza > 100 || fuerza < 1) {
			cout << "Error: Valor " << fuerza << " fuera de rango para fuerza." << endl;
			getchar();
			return 0;
		}

		// SIGMA PARA GAUSSIANO
		cout << "Valor de sigma [1-50] (Gaussian Filter): ";
		int sigma = 1; // CAMBIAR PARA DIFERENTES VALORES DE SIGMA

		// Pasar la entrada a int
		getline(cin, toInt);

		if (is_digits(toInt)) {
			istringstream ss(toInt);
			ss >> sigma;
		}

		// Manejo de errores (cortamos la ejecucion)
		if (sigma > 50 || sigma < 1) {
			cout << "Error: Valor " << sigma << " fuera de rango para sigma." << endl;
			getchar();
			return 0;
		}

		// -----------------------------------------------------
		// --  EMPEZAMOS OPERACIONES DE CPU SIN PARALELIZAR   --
		// -----------------------------------------------------

		// Variables t
		clock_t start, stop;
		float elapsedTime = 0;

		// Mediana
		unsigned char *neighboursred = new unsigned char[dimord];
		unsigned char *neighboursblue = new unsigned char[dimord];
		unsigned char *neighboursgreen = new unsigned char[dimord];
		unsigned char ordenacion;

		// Gaussiano
		float *gaussian_filter = new float[dimord];
		float acumuladorgauss = 0;
		unsigned count;

		start = clock();

		//////////////////////////////
		//    FILTRO MEDIANA        //
		//////////////////////////////

		// For indicativo de la fuerza del algoritmo
		for (int i = 0; i < fuerza; i++) {

			// Recorremos la imagen
			for (int y = 0; y < WIDTH; y++) {
				for (int x = 0; x < HEIGHT; x++) {

					// Guardamos los valores de la imagen en una malla de tamanyo dim*dim
					count = 0;

					for (int z = 0; z < dim && (y + z - half) < WIDTH; z++) {
						for (int k = 0; k < dim && (x + k - half) < HEIGHT; k++) {

							if (y + z - half >= 0 && x + k - half >= 0) {
								neighboursred[count] = Image(y + z - half, x + k - half)->Red;
								neighboursblue[count] = Image(y + z - half, x + k - half)->Blue;
								neighboursgreen[count] = Image(y + z - half, x + k - half)->Green;
								count++;
							}
						}
					}

					// Ordenamos para extraer la mediana
					for (int k = 0; k < count; k++) {
						for (int l = k; l < count; l++) {
							if (neighboursred[k] > neighboursred[l]) {
								ordenacion = neighboursred[k];
								neighboursred[k] = neighboursred[l];
								neighboursred[l] = ordenacion;
							}
							if (neighboursblue[k] > neighboursblue[l]) {
								ordenacion = neighboursblue[k];
								neighboursblue[k] = neighboursblue[l];
								neighboursblue[l] = ordenacion;
							}
							if (neighboursgreen[k] > neighboursgreen[l]) {
								ordenacion = neighboursgreen[k];
								neighboursgreen[k] = neighboursgreen[l];
								neighboursgreen[l] = ordenacion;
							}
						}
					}

					// Mediana
					Mediana(y,x)->Red = neighboursred[count / 2];
					Mediana(y,x)->Blue = neighboursblue[count / 2];
					Mediana(y,x)->Green = neighboursgreen[count / 2];
				}
			}

		}

		// Median filter aplicado
		// Descomentar para ver
		// Mediana.WriteToFile("Mediana.bmp");
		cout << "Mediana completa, iniciando Gaussiano..." << endl;

		//////////////////////////////
		//    FILTRO GAUSIANO       //
		//////////////////////////////
		for (int i = 0; i < dimord; i++) {
			gaussian_filter[i] = Gaussian1D(abs(i - dimord / 2), sigma);
			acumuladorgauss += gaussian_filter[i];
		}

		for (int i = 0; i < dimord; i++) {
			gaussian_filter[i] /= acumuladorgauss;
		}

		// Utilizaremos valores flotantes para el gaussiano
		float rojo, azul, verde;

		for (int y = 0; y < HEIGHT; y++)
			for (int x = 0; x < WIDTH; x++) {
				rojo = 0;
				azul = 0;
				verde = 0;

				for (int k = -dimord / 2; k <= dimord / 2; k++)
					if ((x + k >= 0) && (x + k < WIDTH)) {

						rojo += gaussian_filter[k + dimord / 2] * Image(x + k, y)->Red;
						verde += gaussian_filter[k + dimord / 2] * Image(x + k, y)->Green;
						azul += gaussian_filter[k + dimord / 2] * Image(x + k, y)->Blue;
					}
				gausshori(x, y)->Red = (ebmpBYTE)rojo;
				gausshori(x, y)->Green = (ebmpBYTE)verde;
				gausshori(x, y)->Blue = (ebmpBYTE)azul;

			}

		for (int y = 0; y < WIDTH; y++)
			for (int x = 0; x < HEIGHT; x++) {
				rojo = 0;
				azul = 0;
				verde = 0;

				for (int k = -dimord / 2; k <= dimord / 2; k++)
					if ((x + k >= 0) && (x + k < HEIGHT)) {

						rojo += gaussian_filter[k + dimord / 2] * gausshori(y,x+k )->Red;
						verde += gaussian_filter[k + dimord / 2] * gausshori(y,x+k)->Green;
						azul += gaussian_filter[k + dimord / 2] * gausshori(y,x+k)->Blue;
					}
				gaussverti(y, x)->Red = (ebmpBYTE)rojo;
				gaussverti(y, x)->Green = (ebmpBYTE)verde;
				gaussverti(y, x)->Blue = (ebmpBYTE)azul;

			}

		// Descomentar para ver
		// gaussverti.WriteToFile("Gaussiano.bmp");
		cout << "Gaussiano completo, iniciando Media..." << endl;

		//////////////////////////////
		//      FILTRO MEDIA        //
		//////////////////////////////
		for (int y = 0; y < WIDTH; y++) {
			for (int x = 0; x < HEIGHT; x++) {
				Media(y, x)->Red = (gaussverti(y, x)->Red + Mediana(y, x)->Red) / 2;
				Media(y, x)->Blue = (gaussverti(y, x)->Blue + Mediana(y, x)->Blue) / 2;
				Media(y, x)->Green = (gaussverti(y, x)->Green + Mediana(y, x)->Green) / 2;
			}
		}

		Media.WriteToFile("Media.bmp");

		// Borramos memoria
		delete[] gaussian_filter;
		delete[] neighboursblue;
		delete[] neighboursgreen;
		delete[] neighboursred;

		stop = clock();

		elapsedTime = (float)(stop - start) / (float)CLOCKS_PER_SEC * 1000.0f;


		cout << "Tiempo de ejecucion para " << dim <<
			" de tamano de malla y " << fuerza << " de fuerza en la mediana a la resolucion dada: "
			<< elapsedTime/1000 << " segundos" << endl;

		cout << endl << "Pulsa intro para finalizar...";
	}

	else {
		cerr << endl << "CONSEJO: Asegurate que el archivo de entrada tiene el formato correcto (BMP),"
			<< " esta en el directorio correcto (../Sample) y esta escrito correctamente." << endl;
	}

	getchar();

	return 0;

}
