#%% [markdown]
# # Developing Video IP
# 
# This notebook introduces how to develop Video IP within Vivado HLS and interact with it from within PYNQ

#%%
from pynq import Overlay, Xlnk
from pynq.overlays.base import BaseOverlay

allocator = Xlnk()
ol = Overlay("fpga_ip.bit")


#%%
from pynq.lib.video import *

hdmi_in = ol.video.hdmi_in
hdmi_out = ol.video.hdmi_out

hdmi_in.configure(PIXEL_RGBA)
hdmi_out.configure(hdmi_in.mode, PIXEL_RGBA)


#%%
hdmi_in.start()
hdmi_out.start()


#%%
from pynq import MMIO
filter_reference = MMIO(0x83C30000,0x10000)


#%%
stored_usernames = ['Tharusha', 'Hardik', 'Ramon']
stored_passwords = ['1234', 'srilanka>india', 'imgonnadiesoon']

valid_login = 0
retry = 1
while ((valid_login != 1) & (retry == 1)):
    username = input('Username: ')
    password = input ('Password: ')
    for i in range (0, len(stored_usernames)):  
        if ((username == stored_usernames[i]) & (password == stored_passwords[i])):
            print ("Welcome, ", username)
            valid_login = 1
    if valid_login == 0:
        print ("Invalid username & password")
        retry_check = input ('Retry? [y/n] ')
        if retry_check == 'y':
            retry = 1
        else:
            print("Using default case")
            retry = 0
            username = 'default'


#%%

from IPython.display import Image
from PIL import Image
from pynq.xlnk import ContiguousArray
import numpy as np
import scipy.misc
mmu = Xlnk()
#raster =Image.open('out_image.png')


render_image = 0
image_hit = 10

#generate images for user
image_dim = 250
out_image_1 = mmu.cma_array(shape = (image_dim,image_dim,4),dtype= np.uint8)
out_image_2 = mmu.cma_array(shape = (image_dim,image_dim,4),dtype= np.uint8)
out_image_3 = mmu.cma_array(shape = (image_dim,image_dim,4),dtype= np.uint8)
out_image_4 = mmu.cma_array(shape = (image_dim,image_dim,4),dtype= np.uint8)
out_image_5 = mmu.cma_array(shape = (image_dim,image_dim,4),dtype= np.uint8)
for i in range (1,6):
    image_name = username + '_' + str(i)+'.png'
    try:
        print(image_name)
        ima=scipy.misc.imread(name= image_name, mode='RGBA')
        print('read')
        print(i)
        if i == 1:
            for j in range (0,image_dim):
                for k in range (0,image_dim):
                    out_image_1[j][k] = ima[j][k]
        elif i == 2:
            for j in range (0,image_dim):
                for k in range (0,image_dim):
                    out_image_2[j][k] = ima[j][k]
        elif i == 3:
            for j in range (0,image_dim):
                for k in range (0,image_dim):
                    out_image_3[j][k] = ima[j][k]
        elif i == 4:
            for j in range (0,image_dim):
                for k in range (0,image_dim):
                    out_image_4[j][k] = ima[j][k]
        elif i == 5:
            for j in range (0,image_dim):
                for k in range (0,image_dim):
                    out_image_5[j][k] = ima[j][k]
    except FileNotFoundError:
        continue
    
output_adr_list = [ out_image_1.physical_address, out_image_2.physical_address, out_image_3.physical_address, out_image_4.physical_address, out_image_5.physical_address]
output_adr = output_adr_list[0]


#%%
import time

start = time.time()
prev_cipher_out = 10
hits = 0

for _ in range(300):
    in_frame = hdmi_in.readframe()
    out_frame = hdmi_out.newframe()
    filter_reference.write(0x18, in_frame.physical_address)
    filter_reference.write(0x20, out_frame.physical_address)
    filter_reference.write(0x28, output_adr)
    filter_reference.write(0x30, 1280)
    filter_reference.write(0x38, 720)
    filter_reference.write(0x00, 0x01)
    filter_reference.write(0x40, render_image)    #render image
    cipher_out = filter_reference.read(0x10)
    

    if render_image == 0:
        if cipher_out == prev_cipher_out & cipher_out != 10:
            hits = hits + 1
        else:
            hits = 0
        
        if hits >= 30 & cipher_out <= 5: 
            output_adr = output_adr_list[cipher_out-1]
            render_image = 1
        
    prev_cipher_out = cipher_out  
    
    while (filter_reference.read(0) & 0x4) == 0:
        pass
    hdmi_out.writeframe(out_frame)

end = time.time()

print(f"300 frames took {end-start} seconds at {300/(end-start)} fps")


#%%
hdmi_in.close()
hdmi_out.close()


