#include "systemc.h"
#include "functions.h"
#include "fdct.h"
#include "idct.h"

//#include inverse module

#define NS *1e-9 // use this constant to make sure that the clock signal is in nanoseconds

int sc_main(int argc, char *argv[]) {
	char choice;
	sc_signal<FILE *> sc_input; 	// input file pointer signal
 	sc_signal<FILE *> sc_output;	// output file pointer signal
	sc_signal<double> dct_data[8][8]; // signal to the dc transformed data
	sc_signal<double> cosine_tbl[8][8]; // signal for the cosine table values

	sc_signal<bool> clk1, clk2; 		// clock signal for FDCT (also need a 2nd clock for IDCT)

	FILE *input, *output; // input and output file pointers
	double cosine[8][8]; // the cosine table
	double data[8][8]; // the data read from the signals, which will be zigzagged
	
	//Decompression signals and variables
	sc_signal<double> idct_data_in[8][8];
	sc_signal<double> idct_data_out[8][8];
	sc_signal<double> cosine_tbl2[8][8];

	int data_int[8][8];

	if (argc == 4) {
		if (!(input = fopen(argv[1], "rb"))) // error occurred while trying to open file
			printf("\nSystemC JPEG LAB:\ncannot open file '%s'\n", argv[1]), exit(1);

		if (!(output = fopen(argv[2], "wb"))) // rror occurred while trying to create file
			printf("\nSystemC JPEG LAB:\ncannot create file '%s'\n", argv[2]), exit(1);

		// copy the input and output file pointer onto the respective ports
		sc_input.write(input);
		sc_output.write(output);
		

		choice = *argv[3];
	} else
		fprintf(stderr, "\nSystemC JPEG LAB: insufficient command line arguments\n"
			"usage:  ./sc_jpeg.x [input file] [output file] [(C)ompress or (D)ecompress]\n")
				, exit(1);

        // write the header, read from the input file
	write_read_header(input, output);

	// make the cosine table
	make_cosine_tbl(cosine);

	// call the forward discrete transform module
	// and bind the ports -FDCT
	fdct fdct_mod("fdct");

	fdct_mod.sc_input(sc_input);
        fdct_mod.clk(clk1);

	// copy the cosine table and the quantization table onto the corresponding signals to send to DCT module
	for(unsigned char i = 0; i < 8; i++)
	  for(unsigned char j = 0; j < 8; j++)
	   cosine_tbl[i][j].write(cosine[i][j]);

	for(unsigned char i = 0; i < 8; i++)
	  for(unsigned char j = 0; j < 8; j++){
	    fdct_mod.out64[i][j](dct_data[i][j]);
	    fdct_mod.fcosine[i][j](cosine_tbl[i][j]);
	  }
	   
	//binds ports - idct
	idct idct_mod("idct");
	
	idct_mod.clk(clk2);

	for(unsigned char i = 0; i < 8; i++)
	  for(unsigned char j = 0; j < 8; j++){
	    idct_mod.input_data[i][j](idct_data_in[i][j]);
	    idct_mod.out64[i][j](idct_data_out[i][j]);
	    idct_mod.fcosine[i][j](cosine_tbl[i][j]);
	  }
	
	// because compression and decompression are two different processes, we must use
	// two different clocks, to make sure that when we want to compress, we only compress
	// and dont decompress by mistake
	sc_start(SC_ZERO_TIME); 	// initialize the clock
	if ((choice == 'c') || (choice == 'C')) { // for compression
		while (!(feof(input))) { // cycle the clock for as long as there is something to be read from the input file
			// create the FDCT clock signal
			clk1.write(1);		// convert the clock to high
			sc_start(10, SC_NS);	// cycle the high for 10 nanoseconds
			clk1.write(0);		// start the clock as low
			sc_start(10, SC_NS);	// cycle the low for 10 nanoseconds

			// read back signals from module
		        for(unsigned char i = 0; i < 8; i++)
			  for(unsigned char j = 0; j < 8; j++)
			    data[i][j] = dct_data[i][j].read();

			// zigzag and quantize the outputted data - will write out to file (see functions.h)
			zigzag_quant(data, output);


		}
	} else if ((choice == 'd') || (choice == 'D')) { // for decompression
		while (!(feof(input))) {
			//unzigzag and inverse quatize input file and result will be placed in data
		        unzigzag_iquant(data, input);
			
			//write unzigzag data to ports
			for(unsigned char i = 0; i < 8; i++)
			   for(unsigned char j = 0; j < 8; j++)
			     idct_data_in[i][j].write(data[i][j]);
		
			clk2.write(1);		// convert the clock to high
			sc_start(10, SC_NS);	// cycle the high for 10 nanoseconds
			clk2.write(0);		// start the clock as low
			sc_start(10, SC_NS);	// cycle the low for 10 nanoseconds
			
			//read idct data from ports & write to output file
			for(unsigned char i = 0; i < 8; i++)
			  for(unsigned char j = 0; j < 8; j++){
			     data[i][j] = idct_data_out[i][j].read();
			     data_int[i][j] = (int) data[i][j];
			    
			     // get rid of the weird colors
			     if (data_int[i][j] < 1)
			       data_int[i][j] = 0;
			     printf("%d ", data_int[i][j]);
			     fwrite(&data_int[i][j], 1, 1, output);
			  }
		       
			
		}
		
	}	
	fclose(sc_input.read());
	fclose(sc_output.read());

	return 0;
}
