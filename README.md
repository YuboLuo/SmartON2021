# SmartOn: Just-in-Time Active Event Detection on Energy Harvesting Systems
This repository holds the relevant project files for SmartON, an energy harvesting system that performs active event detection by using reinforcement learning.

## TI_CCS
This folder includes files for CCS project. You can create a new empty project in CCS and then copy those files to the project. It should work. This project uses driverlib library so when you create a new empty project in CCS, be sure to choose "MSP430 DriverLib -> Empty Project with DriverLib Source" under the "Project templates and examples". The target board is MSP430Fr5994. This project was developed in CCS Version: 9.1.0.00010 

## Hardware
This folder includes files for designing the PCB board and library files for some components. This project was developed with Eagle. 

## Simulation
This folder includes files for the software simulation.

## Experiments
All experiment-related results, e.g. raw sensor data.

## Reference 
Please cite this paper in the following format for Latex. 

@INPROCEEDINGS {smarton,
author = {Y. Luo and S. Nirjon},
booktitle = {2021 17th International Conference on Distributed Computing in Sensor Systems (DCOSS)},
title = {SmartON: Just-in-Time Active Event Detection on Energy Harvesting Systems},
year = {2021},
volume = {},
issn = {},
pages = {35-44},
keywords = {event detection;power system management;capacitance;energy efficiency;hardware;software;sensor systems},
doi = {10.1109/DCOSS52077.2021.00018},
url = {https://doi.ieeecomputersociety.org/10.1109/DCOSS52077.2021.00018},
publisher = {IEEE Computer Society},
address = {Los Alamitos, CA, USA},
month = {jul}
}
