#include <ap_fixed.h>
#include <ap_int.h>
#include <cassert>
#include <iostream>
//#include "myfile.h"

//#include "opencv2/opencv.hpp"
#include <hls_opencv.h>

typedef ap_uint<8> pixel_type;
typedef ap_int<8> pixel_type_s;
typedef ap_uint<96> u96b;
typedef ap_uint<32> word_32;
typedef ap_ufixed<8,0, AP_RND, AP_SAT> comp_type;
typedef ap_fixed<10,2, AP_RND, AP_SAT> coeff_type;

struct pixel_data {
	pixel_type blue;
	pixel_type green;
	pixel_type red;
};


int gray_filter(volatile uint32_t* in_data, volatile uint32_t* out_data, volatile uint32_t* in_image, int w, int h, bool render_image);

int main() {


	cv::Mat src_hls = cv::imread("/home/ta2617/1st_yr/cip_5.png", CV_LOAD_IMAGE_UNCHANGED);
	std::cout << "Image type: " << src_hls.type() << ", no. of channels: " << src_hls.channels() << std::endl;
	//src_hls.convertTo(src_hls, CV_8UC3);
	//cv::cvtColor(src_hls, src_hls, CV_BGR2RGBA);


	std::cout<<"VIDEO SIZE  "<<src_hls.size()<<std::endl;
	uchar *data_p = src_hls.data;

	int w = 1920;
	int h = 1080;


	uchar *image = (uchar *)malloc(w*h*4);

	for (int i=0; i<w*h; i++){
		image[4*i + 0] = data_p[3*i + 2]; //R - R
		image[4*i + 1] = data_p[3*i + 1]; // B - B
		image[4*i + 2] = data_p[3*i + 0]; // G - G
		image[4*i + 3] = 0;
	}

	bool dis_out= false;

	//output image
	cv::Mat gen_image = cv::imread("/home/ta2617/pynq_EIE_reference/src/out_image.png", CV_LOAD_IMAGE_UNCHANGED);
	std::cout << "Image type: " << gen_image.type() << ", no. of channels: " << gen_image.channels() << std::endl;


	std::cout<<"OUTPUT SIZE  "<<gen_image.size()<<std::endl;
	uchar *im_data_p = gen_image.data;

	uchar *out_im = (uchar *)malloc(350*350*4);

	std::cout<<"works"<<std::endl;
	for (int i=0; i<350*350; i++){
		out_im[4*i + 0] = im_data_p[3*i + 2]; //R - R
		out_im[4*i + 1] = im_data_p[3*i + 1]; // B - B
		out_im[4*i + 2] = im_data_p[3*i + 0]; // G - G
		out_im[4*i + 3] = 0;
	}


	int output = gray_filter((volatile uint32_t *)image, (volatile uint32_t *)image, (volatile uint32_t *)out_im, w, h, dis_out);

	for (int i=0; i<w*h; i++){
		data_p[3*i + 2] = image[4*i + 0];
		data_p[3*i + 1] = image[4*i + 1];
		data_p[3*i + 0] = image[4*i + 2];
	}

	cv::imwrite("output.jpg",src_hls);
	std::cout<<"output: "<<output<<std::endl;
	free(image);

	return 0;

}
