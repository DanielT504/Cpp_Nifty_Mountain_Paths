#include <iostream>
#include <allegro5/allegro.h>
#include <cmath>
#include <fstream>
#include <string>
#include <cstdlib>
#include "C:/AP/APMATRIX.h"
#include "C:/AP/APVECTOR.h"

//dimensions for data apmatrix
#define ROWS 480
#define COLUMNS 844

using namespace std;

//display dimensions necessary to look nice
const int SCREEN_H = 480;
const int SCREEN_W = 842;
//used in almost every function
int col = 0, row = 0;

bool MapDataDrawer(const char *filename, apmatrix<short> &val);
short findMin(apmatrix<short> &val);
short findMax(apmatrix<short> &val);
void drawMap(int minvalue, float scale, apmatrix<short> &val);
//At this point, functions divert from the provided sketch
//to follow a slightly altered greedy path algorithm
void firstRowPath(int &whichrow, long &tempElevDif, apmatrix<short> &val, apvector<int> &tempRowVals);
void lastRowPath(int &whichrow, long &tempElevDif, apmatrix<short> &val, apvector<int> &tempRowVals);
void middleRowPaths(int &whichrow, long &tempElevDif, apmatrix<short> &val, apvector<int> &tempRowVals);
bool goTop(int whichrow, apmatrix<short> &val);
bool goMiddle(int whichrow, apmatrix<short> &val);
bool goBottom(int whichrow, apmatrix<short> &val);
void anyElevTies(int &whichrow, long &tempElevDif, apmatrix<short> &val, apvector<int> &tempRowVals);
bool all3equal(int whichrow, apmatrix<short> &val);
//lowest path algorithm
void lowestElevPath(int &whichrow, apmatrix<short> &val, apvector<int> &tempRowVals, apvector<int> &bestPathRows);
void firstRowPathLow(int &whichrow, int &tempElev, apmatrix<short> &val, apvector<int> &tempRowVals);
void lastRowPathLow(int &whichrow, int &tempElev, apmatrix<short> &val, apvector<int> &tempRowVals);
void middleRowPathsLow(int &whichrow, int &tempElev, apmatrix<short> &val, apvector<int> &tempRowVals);
bool goTopLow(int whichrow, apmatrix<short> &val);
bool goMiddleLow(int whichrow, apmatrix<short> &val);
bool goBottomLow(int whichrow, apmatrix<short> &val);
void anyElevTiesLow(int &whichrow, int &tempElev, apmatrix<short> &val, apvector<int> &tempRowVals);
bool all3equalLow(int whichrow, apmatrix<short> &val);

int main(int argc, char *argv[]) {
    //declaring variables
    apmatrix<short> val(COLUMNS,ROWS);
    apvector<int> tempRowVals(844), bestPathRows(844);
    int whichrow;
    float scale;
    const char filename[21] = "Colorado_844x480.txt";
    long tempElevDif = 0, elevDif = 100000;

    //randomizing seed
    srand(time(NULL));

    //generic allegro setup
    al_init();

	ALLEGRO_DISPLAY *display = nullptr;
	display = al_create_display(SCREEN_W, SCREEN_H);

	if (!display) {
    	cout << "Failed to initialize display";

       	return 1;
	}

    //making sure the file is accessible
	if (!MapDataDrawer(filename, val)){
        cout << "Error, data file not retrieved";
    }
    else {
        //producing the ratio from the range of data to a gray scale range
        scale = (float(findMax(val)) - float(findMin(val))) / 256;

        //loading in case the computer is slow
        cout << "Rendering image..." << endl;

        //outputting the topographical map
        drawMap(findMin(val), scale, val);

        //producing a path starting from each row
        for (row = 0; row < 479; row++){
            //painting the first pixel of each row red, because that's where each path starts
            al_draw_pixel(1, row, al_map_rgb(255, 0, 0));

            //setting the current row to it's starting point
            whichrow = row;

            //within each path, paint a pixel for every column
            for (col = 0; col < 843; col++){
                //a path only has two options if it's in the top row, so it is given its own procedure
                if (whichrow == 0){
                    firstRowPath(whichrow, tempElevDif, val, tempRowVals);
                }
                //a path only has two options if it's in the bottom row, so it is given its own procedure
                else if (whichrow == 479){
                    lastRowPath(whichrow, tempElevDif, val, tempRowVals);
                }
                //procedure to draw all the paths in the middle rows
                else {
                    middleRowPaths(whichrow, tempElevDif, val, tempRowVals);
                }
            }
            //reevaluating the total elevation difference if a new smaller one is found
            if (tempElevDif < elevDif){
                elevDif = tempElevDif;

                //setting the best possible path to the one with the newly found smaller elevation change
                bestPathRows = tempRowVals;

                //adding 1 to each row value to match the coordinate system on the display
                for (int x = 0; x < bestPathRows.length(); x++){
                    bestPathRows[x] += 1;
                }
            }
            //resetting the elevation change recorder
            tempElevDif = 0;
        }

        //for each column, draw its according green pixel from the best path, to draw the greedy path
        for (col = 0; col < 843; col++){
            al_draw_pixel(col + 1, bestPathRows[col + 1], al_map_rgb(0, 255, 0));
        }

        //outputting summary
        cout << "\nUsing the greedy path algorithm, the path with one of\nthe least total elevation changes (highlighted in green)\n";
        cout << "would have a total elevation change of " << elevDif << " meters,\nand would begin at row " << bestPathRows[0] + 1 << ".\n" << endl;

        //separate algorithm to determine the path with the lowest elevation total
        lowestElevPath(whichrow, val, tempRowVals, bestPathRows);

        //standard in allegro
        al_flip_display();

        //keeping the display up until closed
        while (1){
            al_rest(1);
        }

        return 0;
    }
}

//function to read in data for text file
bool MapDataDrawer(const char *filename, apmatrix<short> &val){
    //declaring extra variable
    int a;

    //opening file to read in data
    ifstream datafile(filename);

    //using an extra variable to make sure that the function
    //doesn't end until after the data has been read in
    if(!datafile){
        a = 0;
    }
    else {
        a = 1;
    }

    //reading in the data from the file, and sorting it into columns and rows
    for (row = 0; row < 480; row++){
        for (col = 0; col < 844; col++){
            datafile >> val[col][row];
        }
    }

    //returning whether or not the file was accessed
    return a;
}

//function to find the minimum value in all the data
short findMin(apmatrix<short> &val){
    //setting the current minimum value to an obviously impossible minimum
    int minvalue = 10000;

    //going through each column of each row
    for (int b = 0; b < 480; b++){
        for (int a = 0; a < 844; a++){
            //comparing each value against the current minimum value
            if (val[a][b] < minvalue){
                minvalue = val[a][b];
            }
        }
    }

    //returning the smallest value contained in the data
    return minvalue;
}

//function to find the maximum value in all the data
short findMax(apmatrix<short> &val){
    //setting the current maximum value to an obviously impossible maximum
    int maxvalue = 0;

    //going through each column of each row
    for (int b = 0; b < 480; b++){
        for (int a = 0; a < 844; a++){
            //comparing each value against the current maximum value
            if (val[a][b] > maxvalue){
                maxvalue = val[a][b];
            }
        }
    }

    //returning the largest value contained in the data
    return maxvalue;
}

//function to draw the topographical map
void drawMap(int minvalue, float scale, apmatrix<short> &val){
    //declaring a variable for the current gray value
    int shade;

    //going through each column of each row
    for (row = 0; row < 480; row++){
        for (col = 0; col < 844; col++){
            //setting the ratio of gray values to the ratio of the data range
            shade = (val[col][row] - minvalue) / scale;

            //drawing out the according gray value for each pixel of the map
            al_draw_pixel(col + 1, row + 1, al_map_rgb(shade, shade, shade));
        }
    }
}

//procedure if the current row is the first row (can only go straight or down)
void firstRowPath(int &whichrow, long &tempElevDif, apmatrix<short> &val, apvector<int> &tempRowVals){
    //if going straight is the most efficient option
    if (abs(val[col][whichrow] - val[col + 1][whichrow])
        < abs(val[col][whichrow] - val[col + 1][whichrow + 1])){

        //drawing the next pixel directly ahead
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));

        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;
    }
    //if going down is the most efficient option
    else if (abs(val[col][whichrow] - val[col + 1][whichrow])
            > abs(val[col][whichrow] - val[col + 1][whichrow + 1])){

        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow + 1]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //moving down one
        whichrow++;

        //drawing the next pixel, down one
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));
    }
    //if the first and second row are equally efficient options
    else {
        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow + 1]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing between going straight and going down one
        if (rand() % 2 + 1 == 1){
            whichrow++;
        }

        //drawing the next pixel, either straight or down one
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));
    }
}

//procedure if the current row is the last row (can only go straight or up)
void lastRowPath(int &whichrow, long &tempElevDif, apmatrix<short> &val, apvector<int> &tempRowVals){
    //if going straight is the most efficient option
    if (abs(val[col][whichrow] - val[col + 1][whichrow])
        < abs(val[col][whichrow] - val[col + 1][whichrow - 1])){

        //drawing the next pixel directly ahead
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));

        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;
    }
    //if going up is the most efficient option
    else if (abs(val[col][whichrow] - val[col + 1][whichrow])
             > abs(val[col][whichrow] - val[col + 1][whichrow - 1])){

        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow - 1]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //moving up one
        whichrow--;

        //drawing the next pixel, up one
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));
    }
    //if the last and second last row are equally efficient options
    else {
        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing between going straight and going up one
        if (rand() % 2 + 1 == 1){
            whichrow--;
        }

        //drawing the next pixel, either straight or up one
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));
    }
}

//procedure to draw out the possible paths from each starting row
void middleRowPaths(int &whichrow, long &tempElevDif, apmatrix<short> &val, apvector<int> &tempRowVals){
    //if going up is the most efficient option
    if (goTop(whichrow, val)){
        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow - 1]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //moving up one
        whichrow--;

        //drawing the next pixel, up one
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));
    }
    //if going straight is the most efficient option
    else if (goMiddle(whichrow, val)){
        //drawing the next pixel, directly ahead
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));

        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;
    }
    //if going down is the most efficient option
    else if (goBottom(whichrow, val)){
        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow + 1]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //moving down one
        whichrow++;

        //drawing the next pixel, down one
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));
    }
    //if two or more of the options have equal elevation differences
    else {
        anyElevTies(whichrow, tempElevDif, val, tempRowVals);
    }
}

//condition where the top choice has the smallest elevation change
bool goTop(int whichrow, apmatrix<short> &val){
    //returns true if going up is the most efficient option
    if (abs(val[col][whichrow] - val[col + 1][whichrow - 1])
        < abs(val[col][whichrow] - val[col + 1][whichrow + 1])
        && abs(val[col][whichrow] - val[col + 1][whichrow - 1])
        < abs(val[col][whichrow] - val[col + 1][whichrow])){

        return 1;
    }
    else{
        return 0;
    }
}

//condition where the middle choice has the smallest elevation change
bool goMiddle(int whichrow, apmatrix<short> &val){
    //returns true if going straight is the most efficient option
    if (abs(val[col][whichrow] - val[col + 1][whichrow])
        < abs(val[col][whichrow] - val[col + 1][whichrow + 1])
        && abs(val[col][whichrow] - val[col + 1][whichrow])
        < abs(val[col][whichrow] - val[col + 1][whichrow - 1])){

        return 1;
    }
    else{
        return 0;
    }
}

//condition where the bottom choice has the smallest elevation change
bool goBottom(int whichrow, apmatrix<short> &val){
    //returns true if going down is the most efficient option
    if (abs(val[col][whichrow] - val[col + 1][whichrow + 1])
        < abs(val[col][whichrow] - val[col + 1][whichrow])
        && abs(val[col][whichrow] - val[col + 1][whichrow + 1])
        < abs(val[col][whichrow] - val[col + 1][whichrow - 1])){

        return 1;
    }
    else{
        return 0;
    }
}

//procedure if 2 or more of the options have the same elevation difference
void anyElevTies(int &whichrow, long &tempElevDif, apmatrix<short> &val, apvector<int> &tempRowVals){
    //if all three options have the same elevation difference
    if (all3equal(whichrow, val)){
        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing to go up, straight, or down
        int a = rand() % 3 + 1;

        if (a == 1){
            whichrow++;
        }
        else if (a == 2){
            whichrow--;
        }

        //drawing next pixel, either up one, directly ahead, or down one
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));
    }
    //if the middle and bottom row are equally efficient options
    else if (abs(val[col][whichrow] - val[col + 1][whichrow + 1])
             == abs(val[col][whichrow] - val[col + 1][whichrow])){

        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing between going straight and going down
        if (rand() % 2 + 1 == 1){
            whichrow++;
        }

        //drawing the next pixel, either directly ahead, or down one
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));
    }
    //if the top and bottom row are equally efficient options
    else if (abs(val[col][whichrow] - val[col + 1][whichrow + 1])
             == abs(val[col][whichrow] - val[col + 1][whichrow - 1])){

        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow + 1]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing between going up and going down
        int b = rand() % 2 + 1;

        if (b == 1){
            whichrow++;
        }
        else {
            whichrow--;
        }

        //drawing next pixel, either up one, or down one
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));
    }
    //if the middle and top row are equally efficient options
    else if (abs(val[col][whichrow] - val[col + 1][whichrow])
             == abs(val[col][whichrow] - val[col + 1][whichrow - 1])){

        //adding the elevation difference to the total elevation change
        tempElevDif += abs(val[col][whichrow] - val[col + 1][whichrow]);
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing between going up and going straight
        if (rand() % 2 + 1 == 1){
            whichrow--;
        }

        //drawing the next pixel, either up one, or directly ahead
        al_draw_pixel(col + 1, whichrow + 1, al_map_rgb(255, 0, 0));
    }
}

//condition where all three options have the same elevation difference
bool all3equal(int whichrow, apmatrix<short> &val){
    //returns true if no option is more efficient than either of the other two
    if (abs(val[col][whichrow] - val[col + 1][whichrow + 1])
        == abs(val[col][whichrow] - val[col + 1][whichrow])
        && abs(val[col][whichrow] - val[col + 1][whichrow - 1])
        == abs(val[col][whichrow] - val[col + 1][whichrow])){

        return 1;
    }
    else{
        return 0;
    }
}

void lowestElevPath(int &whichrow, apmatrix<short> &val, apvector<int> &tempRowVals, apvector<int> &bestPathRows){
            //declaring variables
            long elev = 2000000;
            int tempElev = 0;

            //producing a path starting from each row
            for (row = 0; row < 479; row++){
                //setting the current row to it's starting point
                whichrow = row;

                //within each path, paint a pixel for every column
                for (col = 0; col < 843; col++){
                    //a path only has two options if it's in the top row, so it is given its own procedure
                    if (whichrow == 0){
                        firstRowPathLow(whichrow, tempElev, val, tempRowVals);
                    }
                    //a path only has two options if it's in the bottom row, so it is given its own procedure
                    else if (whichrow == 479){
                        lastRowPathLow(whichrow, tempElev, val, tempRowVals);
                    }
                    //procedure to draw all the paths in the middle rows
                    else {
                        middleRowPathsLow(whichrow, tempElev, val, tempRowVals);
                    }
                }
                //reevaluating the total elevation if a new smaller one is found
                if (tempElev < elev){
                    elev = tempElev;

                    //setting the best possible path to the one with the newly found smaller total elevation
                    bestPathRows = tempRowVals;

                    //adding 1 to each row value to match the coordinate system on the display
                    for (int x = 0; x < bestPathRows.length(); x++){
                        bestPathRows[x] += 1;
                    }
                }
                //resetting the total elevation recorder
                tempElev = 0;
            }

            //for each column, draw its according green pixel from the best path, to draw the lowest path
            for (col = 0; col < 843; col++){
                al_draw_pixel(col + 1, bestPathRows[col + 1], al_map_rgb(255, 255, 0));
            }

            cout << "Using another algorithm (the path with the lowest total\nelevation) we can see that the path highlighted in yellow\nbegins at row ";
            cout << bestPathRows[0] << ".\n" << endl;
            cout << "Bear in mind that these paths will always be slightly\naltered due to an element of randomness." << endl;

            //standard in allegro
            al_flip_display();

           //keeping the display up until closed
            while (1){
                al_rest(1);
            }
}

//procedure if the current row is the first row (can only go straight or down)
void firstRowPathLow(int &whichrow, int &tempElev, apmatrix<short> &val, apvector<int> &tempRowVals){
    //if going straight is the lowest option
    if (val[col + 1][whichrow] < val[col + 1][whichrow + 1]){

        //adding the elevation  to the total elevation
        tempElev += val[col + 1][whichrow];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;
    }
    //if going down is the lowest option
    else if (val[col + 1][whichrow] > val[col + 1][whichrow + 1]){

        //adding the elevation to the total elevation
        tempElev += val[col + 1][whichrow + 1];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //moving down one
        whichrow++;
    }
    //if the first and second row are equally low options
    else {
        //adding the elevation to the total elevation
        tempElev += val[col + 1][whichrow + 1];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing between going straight and going down one
        if (rand() % 2 + 1 == 1){
            whichrow++;
        }
    }
}

//procedure if the current row is the last row (can only go straight or up)
void lastRowPathLow(int &whichrow, int &tempElev, apmatrix<short> &val, apvector<int> &tempRowVals){
    //if going straight is the lowest option
    if (val[col + 1][whichrow] < val[col + 1][whichrow - 1]){

        //adding the elevation to the total elevation
        tempElev += val[col + 1][whichrow];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;
    }
    //if going up is the lowest option
    else if (val[col + 1][whichrow] > val[col + 1][whichrow - 1]){

        //adding the elevation to the total elevation
        tempElev += val[col + 1][whichrow - 1];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //moving up one
        whichrow--;
    }
    //if the last and second last row are equally low options
    else {
        //adding the elevation to the total elevation
        tempElev += val[col + 1][whichrow];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing between going straight and going up one
        if (rand() % 2 + 1 == 1){
            whichrow--;
        }
    }
}

//procedure to draw out the possible paths from each starting row
void middleRowPathsLow(int &whichrow, int &tempElev, apmatrix<short> &val, apvector<int> &tempRowVals){
    //if going up is the lowest option
    if (goTopLow(whichrow, val)){
        //adding the elevation to the total elevation
        tempElev += val[col + 1][whichrow - 1];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //moving up one
        whichrow--;
    }
    //if going straight is the lowest option
    else if (goMiddleLow(whichrow, val)){
        //adding the elevation to the total elevation
        tempElev += val[col + 1][whichrow];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;
    }
    //if going down is the lowest option
    else if (goBottomLow(whichrow, val)){
        //adding the elevation to the total elevation
        tempElev += val[col + 1][whichrow + 1];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //moving down one
        whichrow++;
    }
    //if two or more of the options are equally low
    else {
        anyElevTiesLow(whichrow, tempElev, val, tempRowVals);
    }
}

//condition where the top choice is the lowest
bool goTopLow(int whichrow, apmatrix<short> &val){
    //returns true if going up is the lowest
    if (val[col + 1][whichrow - 1] < val[col + 1][whichrow + 1]
        && val[col + 1][whichrow - 1] < val[col + 1][whichrow]){

        return 1;
    }
    else{
        return 0;
    }
}

//condition where the middle choice is the lowest
bool goMiddleLow(int whichrow, apmatrix<short> &val){
    //returns true if going straight is the lowest
    if (val[col + 1][whichrow] < val[col + 1][whichrow + 1]
        && val[col + 1][whichrow] < val[col + 1][whichrow - 1]){

        return 1;
    }
    else{
        return 0;
    }
}

//condition where the bottom choice is the lowest
bool goBottomLow(int whichrow, apmatrix<short> &val){
    //returns true if going down is the lowest
    if (val[col + 1][whichrow + 1] < val[col + 1][whichrow]
        && val[col + 1][whichrow + 1] < val[col + 1][whichrow - 1]){

        return 1;
    }
    else{
        return 0;
    }
}

//procedure if 2 or more of the options are equally low
void anyElevTiesLow(int &whichrow, int &tempElev, apmatrix<short> &val, apvector<int> &tempRowVals){
    //if all three options are equally low
    if (all3equalLow(whichrow, val)){
        //adding the elevation to the total elevation
        tempElev += val[col + 1][whichrow];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing to go up, straight, or down
        int a = rand() % 3 + 1;

        if (a == 1){
            whichrow++;
        }
        else if (a == 2){
            whichrow--;
        }
    }
    //if the middle and bottom row are equally low options
    else if (val[col + 1][whichrow + 1] == val[col + 1][whichrow]){

        //adding the elevation to the total elevation
        tempElev += val[col + 1][whichrow];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing between going straight and going down
        if (rand() % 2 + 1 == 1){
            whichrow++;
        }
    }
    //if the top and bottom row are equally low options
    else if (val[col + 1][whichrow + 1] == val[col + 1][whichrow - 1]){

        //adding the elevation to the total elevation
        tempElev += val[col + 1][whichrow + 1];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing between going up and going down
        int b = rand() % 2 + 1;

        if (b == 1){
            whichrow++;
        }
        else {
            whichrow--;
        }
    }
    //if the middle and top row are equally low options
    else if (val[col + 1][whichrow] == val[col + 1][whichrow - 1]){

        //adding the elevation difference to the total elevation change
        tempElev += val[col + 1][whichrow];
        //adding this particular pixel to the array containing the current path
        tempRowVals[col] = whichrow;

        //randomly choosing between going up and going straight
        if (rand() % 2 + 1 == 1){
            whichrow--;
        }
    }
}

//condition where all three options are equally low
bool all3equalLow(int whichrow, apmatrix<short> &val){
    //returns true if no option is lower than either of the other two
    if (val[col + 1][whichrow + 1] == val[col + 1][whichrow]
        && val[col + 1][whichrow - 1] == val[col + 1][whichrow]){

        return 1;
    }
    else{
        return 0;
    }
}

