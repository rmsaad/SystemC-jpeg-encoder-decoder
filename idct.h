#include "systemc.h"
#include <math.h>

#ifndef IDCT_H
#define IDCT_H

struct idct : sc_module{
  sc_out<double> out64[8][8];
  sc_in<double> fcosine[8][8];
  sc_in<double> input_data[8][8];
  sc_in<bool> clk;
  
 
  void calculate_idct(void);

  SC_CTOR(idct){

    //SC_METHOD(read_data);
    //dont_initialize();
    //sensitive << clk.pos();

    SC_METHOD(calculate_idct);
    dont_initialize();
    sensitive << clk.neg();

  }



};

#endif
