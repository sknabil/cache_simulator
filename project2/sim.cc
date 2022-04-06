#include <iostream>
#include <fstream>
#include <cmath>
#include <string.h>
#include <iomanip>
#include <map>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <bitset>

using namespace std;


// SIM_BRANCH
char Smith_STR[] = "smith";
char Bimodal_STR[] = "bimodal";
char Gshare_STR[] = "gshare";
char Hybrid_STR[] = "hybrid";


uint64_t extract(uint64_t value, int start, int end)
{
    uint64_t mask = (1 << (end - start)) - 1;

    return (value >> start) & mask;
}


void get_prediction_smith(int prediction_bits, int actual_prediction, int *miss_prediction, int *smith_global_counter)
{
	// 1 = taken, 0 = not taken
	int upper_limit = 0, mid_limit = 0;

	if(prediction_bits == 1)
	{
		upper_limit = 1;
		mid_limit = 0;
	}
	else if(prediction_bits == 2)
	{
		upper_limit = 3;
		mid_limit = 1;
	}
	else if(prediction_bits == 3)
	{
		upper_limit = 7;
		mid_limit = 3;
	}
	else if(prediction_bits == 4)
	{
		upper_limit = 15;
		mid_limit = 7;
	}
	else if(prediction_bits == 5)
	{
		upper_limit = 31;
		mid_limit = 15;
	}
	else if(prediction_bits == 6)
	{
		upper_limit = 63;
		mid_limit = 31;
	}
	else {
		printf("Invalid prediction_bits\n");
		exit(1);
	}


	if(actual_prediction == 1){

		if(*smith_global_counter <= mid_limit)
		{
			// miss prediction
			*miss_prediction += 1;
		}

		// update global counter predictor
		if(*smith_global_counter < upper_limit)
		{
			*smith_global_counter += 1;
		}
	}
	else { // actual_prediction == 0
		if(*smith_global_counter > mid_limit)
		{
			// miss prediction
			*miss_prediction += 1;
		}

		// update global counter predictor
		if(*smith_global_counter > 0)
		{
			*smith_global_counter -= 1;
		}
	}

}


void initializeSmithGlobalCounter(int prediction_bits, int *smith_global_counter)
{
	if(prediction_bits == 1)
	{
		*smith_global_counter = 1;
	}
	else if(prediction_bits == 2)
	{
		*smith_global_counter = 2;
	}
	else if(prediction_bits == 3)
	{
		*smith_global_counter = 4;
	}
	else if(prediction_bits == 4)
	{
		*smith_global_counter = 8;
	}
	else if(prediction_bits == 5)
	{
		*smith_global_counter = 16;
	}
	else if(prediction_bits == 6)
	{
		*smith_global_counter = 32;
	}
	else {
		printf("Invalid prediction_bits\n");
		exit(1);
	}

}

void smith_counter_predictor(int counter_bit, char* trace_file)
{

	FILE *fp1;
	char inputLine[30];
	fp1 = fopen(trace_file, "r");

	if(fp1 == NULL)
	{
		perror("Unable to open file!");
		exit(1);
	}

	int total_predictions = 0, total_miss_predictions = 0;
	int smith_global_counter = 0;

	initializeSmithGlobalCounter(counter_bit, &smith_global_counter);

	while (fgets(inputLine, 20, fp1) != NULL){

		string s1 = inputLine;
		boost::trim_right(s1);
		boost::trim_left(s1);

		if (s1.length() == 0)
		{
			//ERROR_LINES++;
			continue;
		}


		char *token1 = strtok(inputLine, " ");
		char *token2 = strtok(NULL, "\n");
	

		if(token1!=NULL && token2 != NULL){
			total_predictions++;

			//cout << "#" << token1 << "#"<<token2<<"@" << endl;

			int actual_prediction = 0;
			if(strcmp(token2, "t") == 0)
			{
				actual_prediction = 1;
			}

			get_prediction_smith(counter_bit, actual_prediction, &total_miss_predictions, &smith_global_counter);

		}
		
	}

	cout << "number of predictions:\t\t" << total_predictions << endl;
	cout << "number of mispredictions:\t" << total_miss_predictions << endl;

	cout << "misprediction rate:\t\t" << std::fixed<<std::setprecision(2) << ((double)total_miss_predictions / total_predictions) * 100 << "%" << endl;
	cout << "FINAL COUNTER CONTENT:\t\t"  << smith_global_counter << endl;

	if(fp1 != NULL) {
		fclose(fp1);
	}
}


int get_prediction_bimodal(int actual_prediction, int *miss_prediction, int *threeBitCounter)
{
	int final_prediction = 0;

	// 1 = taken, 0 = not taken
	int upper_limit = 0, mid_limit = 0;
	upper_limit = 7;
	mid_limit = 3;


	if(actual_prediction == 1){

		if(*threeBitCounter <= mid_limit)
		{
			// miss prediction
			*miss_prediction += 1;

			final_prediction = 0;
		}
		else {
			final_prediction = 1;
		}

		// update global counter predictor
		if(*threeBitCounter < upper_limit)
		{
			*threeBitCounter += 1;
		}
	}
	else { // actual_prediction == 0
		if(*threeBitCounter > mid_limit)
		{
			// miss prediction
			*miss_prediction += 1;

			final_prediction = 1;
		}
		else {
			final_prediction = 0;
		}

		// update global counter predictor
		if(*threeBitCounter > 0)
		{
			*threeBitCounter -= 1;
		}
	}

	return final_prediction;

}

void bimodalBranchPredictor(int number_of_pc_bit, char* trace_file)
{

	int tableSize = pow(2, number_of_pc_bit);
	int predictionTable[tableSize];

	// initialize prediction table
	int x = 0;
	for(x=0; x < tableSize; x++)
	{
		predictionTable[x] = 4;
	}

	FILE *fp1;
	char inputLine[30];
	fp1 = fopen(trace_file, "r");

	if(fp1 == NULL)
	{

		perror("Unable to open file!");
		exit(1);
	}

	//cout << "file opened\n";
	//exit(1);
	int total_predictions = 0, total_miss_predictions = 0;

	while (fgets(inputLine, 20, fp1) != NULL){


		string s1 = inputLine;
		boost::trim_right(s1);
		boost::trim_left(s1);

		if (s1.length() == 0)
		{

			//ERROR_LINES++;
			continue;
		}


		char *token1 = strtok(inputLine, " ");
		char *token2 = strtok(NULL, "\n");


		if(token1!=NULL && token2 != NULL){
			total_predictions++;

			//cout << "#" << token1 << "#"<<token2<<"@" << endl;

			int actual_prediction = 0;
			if(strcmp(token2, "t") == 0)
			{
				actual_prediction = 1;
			}

			uint64_t num = (uint64_t) strtoul(token1, NULL, 16);

			uint64_t mbits = extract(num, 2, number_of_pc_bit+2 );



			int predictionCounter;

			predictionCounter = predictionTable[mbits];



			get_prediction_bimodal(actual_prediction, &total_miss_predictions, &predictionCounter);


			predictionTable[mbits] = predictionCounter;//prediction_row;
			
		}

	}

	cout << "number of predictions:\t\t" << total_predictions << endl;
	cout << "number of mispredictions:\t" << total_miss_predictions << endl;

	cout << "misprediction rate:\t\t" << std::fixed << std::setprecision(2) << (((double)total_miss_predictions / total_predictions) * 100) << "%" << endl;
	cout << "FINAL BIMODAL CONTENTS"  << endl;
	for(x=0; x < tableSize; x++)
	{
		cout <<x<<"\t"<<predictionTable[x] <<endl;
	}


	if(fp1 != NULL) {
		fclose(fp1);
	}
}

int get_prediction_gshare(int actual_prediction, int *miss_prediction, int *threeBitCounter, uint64_t *ghr, int n)
{
	int final_prediction = 0;

	// 1 = taken, 0 = not taken
	int upper_limit = 0, mid_limit = 0;
	upper_limit = 7;
	mid_limit = 3;

	if(actual_prediction == 1){

		if(*threeBitCounter <= mid_limit)
		{
			// miss prediction
			*miss_prediction += 1;

			final_prediction = 0;
		}
		else {
			final_prediction = 1;
		}

		// update global counter predictor
		if(*threeBitCounter < upper_limit)
		{
			*threeBitCounter += 1;
		}

		// Update ghr
		// Shifting one bit right and placing last update in MSB

		*ghr = *ghr >> 1;
      	*ghr = (1 << (n - 1)) | *ghr;
	}
	else { // actual_prediction == 0
		if(*threeBitCounter > mid_limit)
		{
			// miss prediction
			*miss_prediction += 1;

			final_prediction = 1;
		}
		else {
			final_prediction = 0;	
		}

		// update global counter predictor
		if(*threeBitCounter > 0)
		{
			*threeBitCounter -= 1;
		}

		// Update ghr
		//*ghr = (*ghr << 1 );	
		*ghr = *ghr >> 1;
	}

	return final_prediction;
}

void gshareBranchPredictor(int number_of_pc_bit, int n, char* trace_file)
{

	int tableSize = pow(2, number_of_pc_bit);
	int predictionTable[tableSize];

	// initialize prediction table
	int x = 0;
	for(x=0; x < tableSize; x++)
	{
		predictionTable[x] = 4;
	}


	FILE *fp1;
	char inputLine[30];
	fp1 = fopen(trace_file, "r");

	if(fp1 == NULL)
	{

		perror("Unable to open file!");
		exit(1);
	}

	//cout << "file opened\n";
	//exit(1);
	int total_predictions = 0, total_miss_predictions = 0;

	// Global branch history register
	uint64_t ghr = 0;

	while (fgets(inputLine, 20, fp1) != NULL){


		string s1 = inputLine;
		boost::trim_right(s1);
		boost::trim_left(s1);

		if (s1.length() == 0)
		{

			//ERROR_LINES++;
			continue;
		}


		char *token1 = strtok(inputLine, " ");
		char *token2 = strtok(NULL, "\n");


		if(token1!=NULL && token2 != NULL){
			total_predictions++;

			//cout << "#" << token1 << "#"<<token2<<"@" << endl;

			int actual_prediction = 0;
			if(strcmp(token2, "t") == 0)
			{
				actual_prediction = 1;
			
			}

			uint64_t num = (uint64_t) strtoul(token1, NULL, 16);


			uint64_t index = extract(num, 2, number_of_pc_bit+2 );//mbits;//extract(mbits, 0, n);

			
			ghr = extract(ghr, 0, n);

			index = index ^ ghr;


			int predictionCounter;


			predictionCounter = predictionTable[index];

			get_prediction_gshare(actual_prediction, &total_miss_predictions, &predictionCounter, &ghr, n);

			//prediction_row.insert(prediction_row.begin(), predictionCounter);
			predictionTable[index] = predictionCounter;//prediction_row;
					
		}
		
	}

	cout << "number of predictions:\t\t" << total_predictions << endl;
	cout << "number of mispredictions:\t" << total_miss_predictions << endl;

	cout << "misprediction rate:\t\t" << std::fixed<<std::setprecision(2) << ((double)total_miss_predictions / total_predictions) * 100 << "%" << endl;
	cout << "FINAL GSHARE CONTENTS"  << endl;
	for(x=0; x < tableSize; x++)
	{
		cout <<x<<"\t"<<predictionTable[x] <<endl;
	}


	if(fp1 != NULL) {
		fclose(fp1);
	}
}

void hybridBranchPredictor(int k,int m1, int n, int m2, char* trace_file)
{

	// m1, n --> gshare
	// m2 --> bimodal
	int tableSizeGshare = pow(2, m1);
	int predictionTableGshare[tableSizeGshare];

	// initialize prediction table
	int x = 0;
	for(x=0; x < tableSizeGshare; x++)
	{
		predictionTableGshare[x] = 4;
	}

	int tableSizeBimodal = pow(2, m2);
	int predictionTableBimodal[tableSizeBimodal];

	// initialize prediction table
	int y = 0;
	for(y=0; y < tableSizeBimodal; y++)
	{
		predictionTableBimodal[y] = 4;
	}

	// hybrid prediction table
	int tableSizeChooser = pow(2, k);
	int predictionTableChooser[tableSizeChooser];

	// initialize prediction table
	int z = 0;
	for(z=0; z < tableSizeChooser; z++)
	{
		predictionTableChooser[z] = 1;
	}


	FILE *fp1;
	char inputLine[30];
	fp1 = fopen(trace_file, "r");

	if(fp1 == NULL)
	{

		perror("Unable to open file!");
		exit(1);
	}

	//cout << "file opened\n";
	//exit(1);
	int total_predictions = 0, total_miss_predictions = 0;

	int gshare_miss_predictions = 0, bimodal_miss_predictions = 0;

	// Global branch history register
	uint64_t ghr = 0;

	while (fgets(inputLine, 20, fp1) != NULL){


		string s1 = inputLine;
		boost::trim_right(s1);
		boost::trim_left(s1);

		if (s1.length() == 0)
		{
			//ERROR_LINES++;
			continue;
		}


		char *token1 = strtok(inputLine, " ");
		char *token2 = strtok(NULL, "\n");


		if(token1!=NULL && token2 != NULL){
			total_predictions++;

			//cout << "#" << token1 << "#"<<token2<<"@" << endl;

			int actual_prediction = 0;
			if(strcmp(token2, "t") == 0)
			{
				actual_prediction = 1;
			
			}


			uint64_t num = (uint64_t) strtoul(token1, NULL, 16);

			int gsharePrediction = 0, bimodalPrediction = 0;
			/********************************/
			// gshare prediction starts //

			uint64_t indexGshare = extract(num, 2, m1+2 );//mbits;//extract(mbits, 0, n);

			
			ghr = extract(ghr, 0, n);

			indexGshare = indexGshare ^ ghr;


			int predictionCounterGshare;


			predictionCounterGshare = predictionTableGshare[indexGshare];

			gsharePrediction = get_prediction_gshare(actual_prediction, &gshare_miss_predictions, &predictionCounterGshare, &ghr, n);

			//prediction_row.insert(prediction_row.begin(), predictionCounter);
			

			/****************************/
			// bimodal prediction starts //
		
			uint64_t indexBimodal = extract(num, 2, m2+2 );

			int predictionCounterBimodal;

			predictionCounterBimodal = predictionTableBimodal[indexBimodal];



			bimodalPrediction = get_prediction_bimodal(actual_prediction, &bimodal_miss_predictions, &predictionCounterBimodal);



			// Index chooser table
			uint64_t indexChooser = extract(num, 2, k + 2 );

			int predictionCounterChooser;
			predictionCounterChooser = predictionTableChooser[indexChooser];



			/************************/
		
			int final_prediction;
			if(predictionCounterChooser >= 2)
			{
				// use gshare prediction
				final_prediction = gsharePrediction;

				/*****************************/
				// update if gshare chosen
				predictionTableGshare[indexGshare] = predictionCounterGshare;//prediction_row;
				
			}
			else 
			{
				// use bimodal prediction
				final_prediction = bimodalPrediction;
				/*****************************/
				// update if bimodal chosen
				predictionTableBimodal[indexBimodal] = predictionCounterBimodal;//prediction_row;
				// bimodal ends
			}
			

			if(actual_prediction == 1){

				if(final_prediction == 0)
				{
					// miss prediction
					total_miss_predictions += 1;
				}

				
			}
			else { // actual_prediction == 0
				if(final_prediction == 1)
				{
					// miss prediction
					total_miss_predictions += 1;
				}
				
			}


			// update prediction counter chooser entry
			if(actual_prediction == gsharePrediction && actual_prediction != bimodalPrediction)
			{
				// increment
				// update global counter predictor
				if(predictionCounterChooser < 3)
				{
					predictionCounterChooser += 1;
				}
			}
			else if(actual_prediction != gsharePrediction && actual_prediction == bimodalPrediction)
			{
				// decrement
				// update global counter predictor
				if(predictionCounterChooser > 0)
				{
					predictionCounterChooser -= 1;
				}
			}

			// update prediction chooser entry
			predictionTableChooser[indexChooser] = predictionCounterChooser;
			/*************************/	
			
		}
		
	}

	cout << "number of predictions:\t\t" << total_predictions << endl;
	cout << "number of mispredictions:\t" << total_miss_predictions << endl;

	cout << "misprediction rate:\t\t" << std::fixed<<std::setprecision(2) << ((double)total_miss_predictions / total_predictions) * 100 << "%" << endl;
	cout << "FINAL CHOOSER CONTENTS"  << endl;
	for(x=0; x < tableSizeChooser; x++)
	{
		cout <<x<<"\t"<<predictionTableChooser[x] <<endl;
	}

	cout << "FINAL GSHARE CONTENTS"  << endl;
	for(x=0; x < tableSizeGshare; x++)
	{
		cout <<x<<"\t"<<predictionTableGshare[x] <<endl;
	}

	cout << "FINAL BIMODAL CONTENTS"  << endl;
	for(x=0; x < tableSizeBimodal; x++)
	{
		cout <<x<<"\t"<<predictionTableBimodal[x] <<endl;
	}


	if(fp1 != NULL) {
		fclose(fp1);
	}
}

int main(int argc, char *argv[])
{
	char* trace_file;

	char* command  = argv[1];
	if(strcmp(command, Smith_STR) == 0)
	{
		//printf("SMITH");
		int b; // number of counter bits used for prediction

		if(argc <= 3) {
	        printf("%s should be sim smith <B> <trace_file>\n", argv[0]);
	        return -1;
	    }

		b = atoi(argv[2]);
		trace_file = argv[3];

		cout << "COMMAND\n./sim " << command<<" "<< b <<" "<<trace_file << endl;
		cout << "OUTPUT" << endl;
		//cout << "ARGS:" << b << " " << trace_file << endl;

		smith_counter_predictor(b, trace_file);

	}
	else if(strcmp(command, Bimodal_STR) == 0)
	{
		//printf("BI-MODAL\n");
		int m2; // number of PC bits used to index the bimodal table

		if(argc <= 3) {
	        printf("%s should be sim bimodal <M2> <trace_file>\n", argv[0]);
	        return -1;
	    }

	    m2 = atoi(argv[2]);
		trace_file = argv[3];

		cout << "COMMAND\n./sim " << command<<" "<< m2 <<" "<<trace_file << endl;
		cout << "OUTPUT" << endl;

	    bimodalBranchPredictor(m2, trace_file);

	}
	else if(strcmp(command, Gshare_STR) == 0)
	{
		//printf("Gshare\n");
		int m1, n; // number of PC bits used to index the bimodal table

		if(argc <= 4) {
	        printf("%s should be sim gshare <M1> <N> <trace_file>\n", argv[0]);
	        return -1;
	    }

	    m1 = atoi(argv[2]);
	    n = atoi(argv[3]);
		trace_file = argv[4];
		//cout << "ARGS:" << m1<<" "<< n << " " << trace_file << endl;

		cout << "COMMAND\n./sim " << command<<" "<< m1<<" "<< n <<" "<<trace_file << endl;
		cout << "OUTPUT" << endl;

	    gshareBranchPredictor(m1, n, trace_file);
	}
	else if(strcmp(command, Hybrid_STR) == 0)
	{
		//printf("Gshare\n");
		int m1, n, m2, k; // number of PC bits used to index the bimodal table

		if(argc <= 6) {
	        printf("%s should be sim hybrid <K> <M1> <N> <M2> <trace_file>\n", argv[0]);
	        return -1;
	    }

	    k = atoi(argv[2]);
	    m1 = atoi(argv[3]);
	    n = atoi(argv[4]);
	    m2 = atoi(argv[5]);
		trace_file = argv[6];
		//cout << "ARGS:" << m1<<" "<< n << " " << trace_file << endl;

		cout << "COMMAND\n./sim " << command<<" "<< k<<" "<< m1<<" "<< n <<" "<< m2 <<" "<<trace_file << endl;
		cout << "OUTPUT" << endl;

	    hybridBranchPredictor(k, m1, n, m2, trace_file);
	}
	else {
		printf("Invalid command!\n");
		return -1;
	}
	
	return 0;
}