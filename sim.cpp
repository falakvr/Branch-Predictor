#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <cstdlib>

#define debug 0

using namespace std;

int SIZE = 32;

int branches, mispredictions, M2, M1, N, K;

unsigned int B_INDEX, G_INDEX, H_INDEX, PC, b_pred_table_size, g_pred_table_size, chooser_table_size;

string bimodal, gshare, hybrid, tracefile, traceline, policy;

char b_prediction, g_prediction, h_prediction, predictor;


int *IntToBin(unsigned int PC, int length);
unsigned int BinToInt(int *, int);
unsigned int HexToInt(char *inAddress);


int main(int argc, char* argv[])
{
	if ((string)argv[1] == "bimodal")
	{
		M2 = atoi(argv[2]);
		tracefile = argv[3];
		if (debug) cout << "M2 is - " << M2 << "\n";
	}	
	else if ((string)argv[1] == "gshare")
	{
		M1 = atoi(argv[2]);
		N = atoi(argv[3]);
		tracefile = argv[4];
		if (debug) cout << "M1 is - " << M1 << " N is - " << N << "\n";
	}
	else if ((string)argv[1] == "hybrid")
	{
		K = atoi(argv[2]);
		M1 = atoi(argv[3]);
		N = atoi(argv[4]);
		M2 = atoi(argv[5]);
		tracefile = argv[6];
	}

	if ((string)argv[1] == "bimodal")
	{

		b_pred_table_size = (unsigned int)pow(2,M2);

		int B_PredictionTable[b_pred_table_size]; //array of counters i.e. prediction table for each PC

		for (int i=0; i < b_pred_table_size; i++)
		{
			B_PredictionTable[i] = 2;
		}

		int *addr = (int *) malloc(sizeof(int)*SIZE);

		int *b_index = (int*) calloc (M2, sizeof(int));

		ifstream myfile (tracefile.c_str());

		while (getline (myfile, traceline))
		{
			std::string str0 = traceline.substr(0,traceline.length() - 2);
			std::string str1 = traceline.substr(traceline.length() - 1, traceline.length());

			char * str2 = strdup(str0.c_str());

			PC = HexToInt(str2);

			branches++;

			addr = IntToBin(PC, SIZE);

			for (int i=2; i <= M2 + 1; i++)
			{
					b_index[i-2] = addr[i];	
			}

			B_INDEX = BinToInt(b_index, M2);

			if (B_PredictionTable[B_INDEX] >= 2) 
				b_prediction = 't';
			else
				b_prediction = 'n';

			if (str1 == "t" && B_PredictionTable[B_INDEX] != 3)
				B_PredictionTable[B_INDEX]++;
			else if (str1 == "n" && B_PredictionTable[B_INDEX] != 0)
				B_PredictionTable[B_INDEX]--;

			if ((b_prediction == 't' && str1 == "n") || (b_prediction == 'n' && str1 == "t"))
				mispredictions++;
		}

		cout << "COMMAND" << endl;
		cout << " " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << endl;
		cout << "OUTPUT" << endl;	

		cout << " number of predictions:	" << branches << "\n";
		cout << " number of mispredictions:	" << mispredictions << "\n";
		cout << " misprediction rate:		" << fixed << setprecision(2) << (float)mispredictions * 100 / branches << "%" << endl;

		cout << "FINAL BIMODAL CONTENTS" << endl;

		for (int i=0; i < b_pred_table_size; i++)
			cout << " " << i << "		" << B_PredictionTable[i] << "\n";
	}

	else if ((string)argv[1] == "gshare")
	{
		g_pred_table_size = (unsigned int)pow(2,M1);

		int G_PredictionTable[g_pred_table_size]; //array of counters i.e. prediction table for each PC

		int GlobalBHR[N]; //Global branch history register array of size n

		for (int i=0; i < g_pred_table_size; i++)
		{
			G_PredictionTable[i] = 2;
		}

		for (int i=0; i < N; i++)
		{
			GlobalBHR[i] = 0;
		}

		int *addr = (int *) malloc(sizeof(int)*SIZE);

		int *g_index = (int*) calloc (M1, sizeof(int));

		int *mpc = (int*) calloc (M1, sizeof(int));

		int *BHRmod = (int*) calloc (N, sizeof(int));

		ifstream myfile (tracefile.c_str());

		while (getline (myfile, traceline))
		{
			std::string str0 = traceline.substr(0,traceline.length() - 2);
			std::string str1 = traceline.substr(traceline.length() - 1, traceline.length());

			char * str2 = strdup(str0.c_str());

			PC = HexToInt(str2);

			//INDEX = PC / 100;

			branches++;

			addr = IntToBin(PC, SIZE);

			//index = IntToBin(INDEX, M1 - 1);

			for (int i=2; i <= M1 + 1; i++)
			{
					mpc[i-2] = addr[i];	
			}

			for (int i = 0; i < N; i++)
			{
				BHRmod[i] = GlobalBHR[i] ^ mpc[M1 - N + i];
			}
			
			for (int i = M1 - N; i < M1; i++)
			{
				mpc[i] = BHRmod[i - (M1 - N)];
			}

			for (int i=0; i < M1; i++)
			{
				g_index[i] = mpc[i];
			}

			G_INDEX = BinToInt(g_index, M1);

			if (G_PredictionTable[G_INDEX] >= 2) 
				g_prediction = 't';
			else
				g_prediction = 'n';

			if (str1 == "t" && G_PredictionTable[G_INDEX] != 3)
				G_PredictionTable[G_INDEX]++;
			else if (str1 == "n" && G_PredictionTable[G_INDEX] != 0)
				G_PredictionTable[G_INDEX]--;

			if ((g_prediction == 't' && str1 == "n") || (g_prediction == 'n' && str1 == "t"))
				mispredictions++;
		
			for (int i = 0; i < N-1; i++)
			{
				GlobalBHR[i] = GlobalBHR[i+1];
			}

			if (str1 == "t")
				(GlobalBHR[N-1] = 1);
			else if (str1 == "n")
				(GlobalBHR[N-1] = 0);
		}

		cout << "COMMAND" << endl;
		cout << " " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] << endl;
		cout << "OUTPUT" << endl;	

		cout << " number of predictions:	" << branches << "\n";
		cout << " number of mispredictions:	" << mispredictions << "\n";
		cout << " misprediction rate:		" << fixed << setprecision(2) << (float)mispredictions * 100 / branches << "%" << endl;

		cout << "FINAL GSHARE CONTENTS" << endl;

		for (int i=0; i < g_pred_table_size; i++)
			cout << "  " << i << "		" << G_PredictionTable[i] << "\n";
	}

	else if ((string)argv[1] == "hybrid")
	{
		chooser_table_size = (unsigned int)pow(2,K);
		b_pred_table_size = (unsigned int)pow(2,M2);
		g_pred_table_size = (unsigned int)pow(2,M1);

		int B_PredictionTable[b_pred_table_size]; //array of counters i.e. prediction table for each PC
		int G_PredictionTable[g_pred_table_size]; //array of counters i.e. prediction table for each PC
		int ChooserTable[chooser_table_size];

		int GlobalBHR[N]; //Global branch history register array of size n
		
		for (int i=0; i < chooser_table_size; i++)
		{
			ChooserTable[i] = 1;
		}

		for (int i=0; i < b_pred_table_size; i++)
		{
			B_PredictionTable[i] = 2;
		}

		for (int i=0; i < g_pred_table_size; i++)
		{
			G_PredictionTable[i] = 2;
		}

		for (int i=0; i < N; i++)
		{
			GlobalBHR[i] = 0;
		}

		int *addr = (int *) malloc(sizeof(int)*SIZE);

		int *b_index = (int*) calloc (M2, sizeof(int));

		int *g_index = (int*) calloc (M1, sizeof(int));

		int *h_index = (int*) calloc (K, sizeof(int));

		int *mpc = (int*) calloc (M1, sizeof(int));

		int *BHRmod = (int*) calloc (N, sizeof(int));

		ifstream myfile (tracefile.c_str());

		while (getline (myfile, traceline))
		{
			std::string str0 = traceline.substr(0,traceline.length() - 2);
			std::string str1 = traceline.substr(traceline.length() - 1, traceline.length());

			char * str2 = strdup(str0.c_str());

			PC = HexToInt(str2);

			branches++;

			addr = IntToBin(PC, SIZE);

			for (int i=2; i <= M2 + 1; i++)
			{
					b_index[i-2] = addr[i];	
			}

			B_INDEX = BinToInt(b_index, M2);

			if (B_PredictionTable[B_INDEX] >= 2) 
				b_prediction = 't';
			else
				b_prediction = 'n';
			
			for (int i=2; i <= M1 + 1; i++)
			{
					mpc[i-2] = addr[i];	
			}

			for (int i = 0; i < N; i++)
			{
				BHRmod[i] = GlobalBHR[i] ^ mpc[M1 - N + i];
			}

			for (int i = M1 - N; i < M1; i++)
			{
				mpc[i] = BHRmod[i - (M1 - N)];
			}

			for (int i=0; i < M1; i++)
			{
				g_index[i] = mpc[i];
			}

			G_INDEX = BinToInt(g_index, M1);

			if (G_PredictionTable[G_INDEX] >= 2) 
				g_prediction = 't';
			else
				g_prediction = 'n';

			for (int i=2; i <= K + 1; i++)
			{
					h_index[i-2] = addr[i];	
			}

			H_INDEX = BinToInt(h_index, K);

			if (ChooserTable[H_INDEX] >= 2) 
			{
				h_prediction = g_prediction;
				predictor = 'g';
			}
			else
			{
				h_prediction = b_prediction;
				predictor = 'b';			
			}

			if (predictor == 'b')
			{
				if (str1 == "t" && B_PredictionTable[B_INDEX] != 3)
				B_PredictionTable[B_INDEX]++;
			else if (str1 == "n" && B_PredictionTable[B_INDEX] != 0)
				B_PredictionTable[B_INDEX]--;
			}
			else if (predictor == 'g')
			{
				if (str1 == "t" && G_PredictionTable[G_INDEX] != 3)
				G_PredictionTable[G_INDEX]++;
			else if (str1 == "n" && G_PredictionTable[G_INDEX] != 0)
				G_PredictionTable[G_INDEX]--;
			}

			for (int i = 0; i < N-1; i++)
			{
				GlobalBHR[i] = GlobalBHR[i+1];
			}

			if (str1 == "t")
				(GlobalBHR[N-1] = 1);
			else if (str1 == "n")
				(GlobalBHR[N-1] = 0);

			if 
			(	
				((b_prediction == 't') && (g_prediction == 't') && (str1 == "t")) || 
				((b_prediction == 'n') && (g_prediction == 'n') && (str1 == "n")) ||
				((b_prediction == 't') && (g_prediction == 't') && (str1 == "n")) ||
				((b_prediction == 'n') && (g_prediction == 'n') && (str1 == "t"))

			)
				policy = "no change";

			else if 
			(
				((b_prediction == 'n') && (g_prediction == 't') && (str1 == "t")) ||
				((b_prediction == 't') && (g_prediction == 'n') && (str1 == "n"))
			)
				policy = "increment";

			else if 
			(
				((b_prediction == 't') && (g_prediction == 'n') && (str1 == "t")) ||
				((b_prediction == 'n') && (g_prediction == 't') && (str1 == "n"))
			)
				policy = "decrement";

			if (	(policy == "increment") && (ChooserTable[H_INDEX] != 3)	)
				ChooserTable[H_INDEX]++;
			else if (	(policy == "decrement") && (ChooserTable[H_INDEX] != 0)	)
				ChooserTable[H_INDEX]--;

			if ((h_prediction == 't' && str1 == "n") || (h_prediction == 'n' && str1 == "t"))
				mispredictions++;
		}

		cout << "COMMAND" << endl;
		cout << " " << argv[0] << " " << argv[1] << " " << argv[2] << " " << argv[3] << " " << argv[4] <<  " " << argv[5] <<  " " << argv[6] << endl;
		cout << "OUTPUT" << endl;	

		cout << " number of predictions:	" << branches << "\n";
		cout << " number of mispredictions:	" << mispredictions << "\n";
		cout << " misprediction rate:		" << fixed << setprecision(2) << (float)mispredictions * 100 / branches << "%" << endl;

		cout << "FINAL CHOOSER CONTENTS" << endl;

		for (int i=0; i < chooser_table_size; i++)
			cout << "  " << i << "		" << ChooserTable[i] << "\n";
	}
}


int * IntToBin(unsigned int address, int length = 32)
{
	int *arr = (int *) malloc(sizeof(int)*length);
 	int i=0, size=length,k;
  	i = 0;
  	while(address != 0)
  	{
  		arr[i++] = address%2;
  		address /= 2;
  	}
  	for(;i<size;i++)
  		arr[i] = 0;
	return arr;
}

unsigned int BinToInt(int *arr1, int length)
{
	unsigned int y =0;
	int i; 
	for (i=0; i<length; i++)
	{
		y += (unsigned int)(pow(2, i)*arr1[i]);
	}
	return y;
}

unsigned int HexToInt(char *inAddress) 
{		
	return (unsigned int) strtol(inAddress, NULL, 16);
};