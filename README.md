# Contextualising-information
First year main project using an FPGA

Included in this repository are some of the files that were created in my main first year group project.

Task and motivation
===================
The task given was to use a PYNQ-Z1 board, featuring an FPGA and ARM processor to perform a form of live image processing, where the input would be an input HDMI stream and the output should be displayed on an output monitor. 
We used this task to design a system that would provide data by scanning a 'cipher' (much like a QR code) and displaying the relevant information on the screen. However this information would change depending on who the user was. For example one of the ciphers would display student information, which would vary depending on the student. 

The report of the project can be found in the included PDF

File description
================
Include in the repository are the following:

fpga_processor.py
-----------------
As described in the report, the ARM processor was run via python on jupyter notebooks, which was run on the board directly. This file was hence primarily used to load and execute the bitstream (.bit) file needed to program the FPGA. 
The file allowed for the use of hardware buttons, from which we used two:
- Change user
- Rescan a new cipher

However, since the processor was powerful enough and ran using python, we also programmed methods to determine the user of device, which where controlled by pressing a hardware button on the board:
- User login: To change user, a button on the board itself was pressed, this in turn paused the processor and requested a username and password from the user (via the jupyter notebook). On successful login, the processor would load the relevant files and display different information on scanning the same cypher.
- Face recognition: Using the OpenCV Haar-Cascade method, the program could identify the user from the camera that was used as the input HDMI stream, and change the user accordingly.

fpga_ip (.bit and .tcl files)
-----------------------------
These were the files that were used by the board to program the FPGA, they were generated from the image_processor source code using the Xilinx VIVADO HLS software. 

image_processor.cpp
-------------------
This file contained the C++ code that was used to program the FPGA, and was where the live image processing was employed. The board would be used to taken in an input stream (of frames) and in each frame would try to detect the presence of a scanner. If successful, it would read the cipher and output the relevant identifier to the processor, which in turn would provide an image that was overlayed onto the corner of the output video stream.

image_processor_tb.cpp
----------------------
This file was created for use in VIVADO HLS, where it was used to test the source code in a purely software-based C++ simualtion (and later RTL simulation), and provided test inputs and teh corresponding output. This saved considerable time as compiling and generating the bitstream files was a considerably time consuming process.