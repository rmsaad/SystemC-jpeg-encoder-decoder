#include "idct.h"

//inverse discrete cosine transform
void idct :: calculate_idct(void){
  unsigned u, v, x, y;
  double temp;

  for(x = 0; x < 8; x++)
    for(y = 0; y < 8; y++){
      temp = 0.0;
      for(u = 0; u < 8; u++)
	for(v = 0; v < 8; v++){
	  if((u == 0)&&(v == 0)) 
	    temp += 0.25 * (0.5 * input_data[u][v] * fcosine[x][u].read() * fcosine[y][v].read());
	  else if (((u == 0) && (v != 0)) || ((u != 0) && (v == 0)))
	    temp += 0.25 * ((1/sqrt(2)) * input_data[u][v] * fcosine[x][u].read() * fcosine[y][v].read());
	  else 
	    temp += 0.25 * (1  * input_data[u][v] * fcosine[x][u].read() * fcosine[y][v].read());
	}
      // do the level shift right here
      temp += 128;
      
      out64[x][y].write(temp);
    }


  printf(".");
}
