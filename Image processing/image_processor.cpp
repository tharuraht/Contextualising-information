#include <ap_fixed.h>
#include <ap_int.h>
#include <stdint.h>
#include <assert.h>

typedef ap_uint<8> pixel_type;
typedef ap_int<8> pixel_type_s;
typedef ap_uint<96> u96b;
typedef ap_uint<32> word_32;
typedef ap_ufixed<8,0, AP_RND, AP_SAT> comp_type;
typedef ap_fixed<10,2, AP_RND, AP_SAT> coeff_type;
typedef ap_uint<11> screen;
typedef ap_uint<14> thresh_t;
typedef ap_uint<3> sys_output;
typedef ap_uint<24> pixel_val;
typedef ap_uint<17> block_t;

struct pixel_data {
	pixel_type blue;
	pixel_type green;
	pixel_type red;
};

unsigned int process_pixels (pixel_val current, bool greyscale);

int gray_filter(volatile uint32_t* in_data, volatile uint32_t* out_data, volatile uint32_t* in_image, int w, int h, bool render_image) {
#pragma HLS INTERFACE s_axilite port=render_image
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE s_axilite port=w
#pragma HLS INTERFACE s_axilite port=h

#pragma HLS INTERFACE m_axi depth=2073600 port=in_data offset=slave // This will NOT work for resolutions higher than 1080p
#pragma HLS INTERFACE m_axi depth=2073600 port=out_data offset=slave
#pragma HLS INTERFACE m_axi depth=62500 port=in_image offset=slave


	screen srch_st_width = w/2 - 175;
	screen srch_end_width = w/2 + 175;
	screen srch_st_height = h*0.75 - 175;
	screen srch_end_height = h*0.75 + 175;
	thresh_t h_lines = 0;
	thresh_t v_lines = 0;
	block_t block_0 = 0;
	block_t block_1 = 0;
	block_t block_2 = 0;
	pixel_val output;
	pixel_val white_p = 16777215;
	//pixel_val black_p = 0;
	pixel_val pink_p = 254 | (127 << 8) | (155 << 16);
	thresh_t threshold = 15300;

	for (screen i = 0; i < h; ++i) {
		for (screen j = 0; j < w; ++j) {
			//each individual pixel

			#pragma HLS PIPELINE II=1
			#pragma HLS LOOP_FLATTEN off

			pixel_val current = *in_data++;
			output = process_pixels(current, 1);

			//convert to binary image
			//range is 0 - 2^24 - 1 = 16777215

			if (output < white_p/2) {
				output = 0;
			}
			else {
				output = white_p;
			}


			//search the square
			if ((i>=srch_st_height) && (i<=srch_end_height) && (j>=srch_st_width) && (j<=srch_end_width)) {
				//check horizontnal lines height: 15
				if ((i<=srch_st_height + 15) || (i>=srch_end_height - 15)) {
					if (output == 0)
						h_lines++;
				}

				//read bars
				if ((j<=srch_st_width + 15) || (j>=srch_end_width - 15)) {
					if (output == 0)
						v_lines++;
				}


				//reading the cipher
				if ((i>=srch_st_height + 45) && (i<=srch_end_height - 45) && (j>=srch_st_width + 45) && (j<=srch_end_width - 45)) {
					//block #2
					if ((j<=srch_st_width + 120) && (output == 0)) {
						block_2++;
					}
					//block #1
					if ((j>=srch_st_width + 135) && (j<=srch_st_width + 210) && (output == 0)) {
						block_1++;
					}
					//block #0
					if ((j>=srch_st_width + 225) && (j<=srch_st_width + 300) && (output == 0)) {
						block_0++;
					}
				}

			}
			//i is height
			//j is width

			//write to image--------------------------------------------------------------------------
			//output of image
			if ((j <250) && (i < 250) && (render_image)) {
				pixel_val output_image = *in_image++;
				output = process_pixels(output_image,0);
			}
			//print search square
			else if (((i == srch_st_height) || (i == srch_end_height)) && ((j>=srch_st_width) && (j<=srch_end_width))) {
				output = pink_p;
			}
			else if (((j == srch_st_width) || (j == srch_end_width)) && (i >= srch_st_height) && (i<= srch_end_height)) {
				output = pink_p;
			}
			else {
				output = current;
			}


			*out_data++ = output;

		}
	}

	//check 350x350 area
	//outline segment being searched
	//check for cipher existence
	//assume correct cipher orientation


	if (block_2 >= threshold) {
		block_2 = 1;
		//std::cout<<"block2"<<std::endl;
	}
	else {
		block_2 = 0;
	}

	if (block_1 >= threshold) {
		block_1 = 1;
		//std::cout<<"block1"<<std::endl;
	}
	else {
		block_1 = 0;
	}

	if (block_0 >= threshold) {
		block_0 = 1;
		//std::cout<<"block0"<<std::endl;
	}
	else {
		block_0 = 0;
	}

	if ((v_lines >= 8400) && (h_lines >= 8400)) {
		sys_output out_index = (block_2 * 4) + (block_1 * 2) + block_0;
		return out_index;
	}
	else {
		return 10;
	}
}

unsigned int process_pixels (pixel_val current, bool greyscale) {

	unsigned char in_r = current & 0xFF;
	unsigned char in_b = (current >> 8) & 0xFF;
	unsigned char in_g = (current >> 16) & 0xFF;

	//convert to greyscale
	if (greyscale) {
		float Y = 0.2126f*in_r  + 0.7152f*in_g  + 0.0722f*in_b ;
		in_r = int(Y);
		in_b = int(Y);
		in_g = int(Y);
	}

	pixel_val output = in_r | (in_b << 8) | (in_g << 16);
	return output;
}
