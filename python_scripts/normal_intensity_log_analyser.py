import sys 
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
'''
This is a Python script that will analyse the intensity normal profiles 
Various functions are included to read the CSV files generated from the C++ 
code and visualisations using simple 3D scatter plots 

Usage: 
Call the program with the CSV filename as the first argument. 

(c) Rashed Karim, 2017 

This will read a CSV file which is output from the C++ programs 
Ensure that the first row contains a header 
'''
def Read_CSV_File(filename): 
    return np.genfromtxt (filename, delimiter=",", skip_header=1)
    
'''
Standardises the intensity by computing its z-score from a distribution with mean and standard deviation 
'''
def Standardise_Intensity(intensities, mean, std):
    intensities_z = (intensities - mean) / std
    return intensities_z

'''
Can be called to plot points in 3D showing the standardised intensity for normals normal direction, cell-id and standardised intensity 
'''
def Show3DPlot(x,y,z): 
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.scatter(x, y, z)
    ax.set_xlabel('Normal direction')
    ax.set_ylabel('Cell-id')
    ax.set_zlabel('Standardises intensity')

    plt.show()


def Show2DPlot(x,y,t=0):

    fig_2d = plt.figure()
    ax = fig_2d.add_subplot(111)
    if len(t) > 0: 
        ax.scatter(x,y,c=t)
    else: 
        ax.scatter(x,y)
    ax.set_xlabel('Cell-id')
    ax.set_ylabel('Standardised intensity')
    
    plt.show()

'''
Main program 
'''

try:   
   args = sys.argv[1:] 
   for arg in args:
    if arg.endswith(".csv"):
        csv_fn = arg  

except IndexError:  
   print("Needs the CSV filename as first argument")
   sys.exit()

bp = 293 
bp_std = 70

csv = Read_CSV_File(csv_fn)
intensity_z = Standardise_Intensity(csv[:,5], bp, bp_std)
#Show3DPlot(csv[:,1], csv[:,0], intensity_z) 

Show2DPlot(csv[:,0], intensity_z, csv[:,1])
