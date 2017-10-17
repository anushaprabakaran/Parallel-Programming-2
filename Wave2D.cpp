// Wave 2D : Sequential Version
// Two dimensional wave diffusion program using Schroedinger's Wave
// Dissemination
// 
// Name : Anusha Prabakaran
// Stdent ID : 1470730


#include <iostream>
#include "Timer.h"
#include <stdlib.h>   // atoi
#include <iostream>
#include <math.h>     // pow

using namespace std;

int default_size = 100;    // the default system size
int defaultCellWidth = 8;  // default cell width
double c = 1.0;            // wave speed
double dt = 0.1;           // time quantum
double dd = 2.0;           // change in system

// calculating (dt/dd)^2 for the wave formula
double dtByddSquare = pow(dt / dd, 2.0);

int main(int argc, char *argv[]) {
  // verify arguments
  if (argc != 4) {
    cerr << "usage: Wave2D size max_time interval" << endl;
    return -1;
  }

  int size = atoi(argv[1]);                 // 100
  int max_time = atoi(argv[2]);             // 500
  int interval = atoi(argv[3]);             // 10

  if (size < 100 || max_time < 3 || interval < 0) {
    cerr << "usage: Wave2D size max_time interval" << endl;
    cerr << "       where size >= 100 && time >= 3 && interval >= 0" <<endl;
    return -1;
  }

  // create a simulation space
  double z[3][size][size];
  for (int p = 0; p < 3; p++) {
    for (int i = 0; i < size; i++) {
      for (int j = 0; j < size; j++) {
        z[p][i][j] = 0.0;       // no wave
      }
    }
  }

  // start a timer
  Timer time;
  time.start();

  // time = 0;
  // initialize the simulation space: calculate z[0][][]
  int weight = size / default_size;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (i > 40 * weight && i < 60 * weight  &&
        j > 40 * weight && j < 60 * weight) {
        z[0][i][j] = 20.0;
      }
      else {
        z[0][i][j] = 0.0;
      }
    }
  }

  // time = 1
  // calculate z[1][][] 
  if (max_time > 1) {
    for (int i = 0; i < size; i++) {
      for (int j = 0; j < size; j++) {
        // cell is on an edge
        if (i == 0 || i == size - 1 || j == 0 || j == size - 1) {
          z[1][i][j] = 0.0;
          continue;
        }
        // cells not on edge
        z[1][i][j] = z[0][i][j] + ((c * c) / 2.0 * dtByddSquare
          * (z[0][i + 1][j] + z[0][i - 1][j]
            + z[0][i][j + 1] + z[0][i][j - 1]
            - 4.0 * z[0][i][j]));
      }
    }
  }

  // simulate wave diffusion from time = 2
  int curr = 2;                // current
  int prev = 1;                // previous 
  int prevPrev = 0;            // previous of previous
  if (max_time > 2) {
    for (int t = 2; t < max_time; t++) {
      // printing the simulation space every interval steps
      if ((interval != 0) && (t % interval == 0 || t == max_time - 1)) {
        cout << t << endl;    // prints the time of each interval
        for (int i = 0; i < size; i++) {
          for (int j = 0; j < size; j++) {
            //cout << "z[" << prev << "][" << j << "][" << i << "]";
            cout << z[curr][j][i] << " ";
          }
          cout << endl;
        }
        cout << endl;
      }

      for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
          // cell is on an edge
          if (i == 0 || i == size - 1 || j == 0 || j == size - 1) {
            z[curr][i][j] = 0.0;
            continue;
          }
          // cells not on edge
          z[curr][i][j] = (2.0 * z[prev][i][j] - z[prevPrev][i][j])
            + (c * c) * dtByddSquare
            * (z[prev][i + 1][j] + z[prev][i - 1][j]
              + z[prev][i][j + 1] + z[prev][i][j - 1]
              - 4.0 * z[prev][i][j]);
        }
      }

      // rotation - shifting down the values
      int temp = prevPrev;
      prevPrev = prev;
      prev = curr;
      curr = temp;
    } 
  }// end of simulation
  
  // finish the timer
  cerr << "Elapsed time = " << time.lap() << endl;
  return 0;
}


