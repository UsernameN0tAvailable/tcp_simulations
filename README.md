# TCP Simulations

This repository makes use of the development ns3 version found [here](https://github.com/nsnam/ns-3-dev-git). No changes have been made and all the ns3 documentation is also valid for this repository.

## Simulation Scenarios
The implemented simulation scenarios are found in the scratch folder.

### Simulation Scripts

In folder scratch/TcpBulkSend, the following scenario is implemented: 5 terminals make use of TCP to transfer data generated from a Bulk Send application.

In folder scratch/TcpOnOff, the following scenario is implemented: 5 terminals use TCP to transfer data generated from an On/Off application.

### Execution
Setup and Configuration:

````./waf configure````

Execution:
````./waf --run "<SimulationScript> --<parameter1>=<value> --<parameter2>=<value> ..."````

The simulation scripts are the following: TcpBulkSend, TcpOnOff

Multiple parameter can be use at a time, the valid parameters are:

````ber````: Bit Error Ratio during the OFF phase

````per````: Packet Error Ratio during the ON phase

````duration````: simulation duration in seconds

````nSF````: number of TCP connections per terminal

````meanONDuration````: mean On phase duration in seconds

````mode````: Error Model: 1 := BER Error Model, 2 := ON/OFF Error Model, 3 := ON/OFF plus BER Error Model

#### Output

The output of the TCP simulations is the throughput in bits per second through the whole network achieved by TCP:

````<Tcp throughput> ````

### Results
The results plotted in the thesis can be found in the RESULTS folder.
#### Throughput Results
The throughput results can be found in the folder RESULTS/THROUGHPUT. TCP_BULKSEND contains the results obtained with 
the BulkSend Application and the TCP_ON_OFF contains the results obtained with the On/Off Application. Both folders contain
the results of the four modes. MODE_1 contains the results obtained with the BER Error Model, MODE_2 the ON/OFF Error Model 
results with the OFF phase bit error ratio modulation, MODE_3 the results obtained with the ON/OFF plus BER Error Model and
MODE_4 the results obtained with the ON/OFF Error Model with the mean ON phase modulation. The text file names are descriptive, in that 
they represents the parameters used for execution (see above in the Execution section for the description and the use of the parameters).

The content of the text files represents the following: ````<average throughput[bits]>   <standard deviation>````.
The average and the standard deviation of the throughput of four simulations with the same parameters but different execution seeds.
#### Congestion Window Results
The congestion window results can be found in the folder RESULTS/CWND. The CWND folder contains the congestion window data of three files: 
total congestion window of the five TCP connections at one host in the Bulk Send application scenario, total congestion window of the 
five TCP connections at one host in the On/Off application scenario and the congestion window of one single connection in the Bulk Send application
scenario.

The content of the text files represents the following: ````<time stamp [seconds]> <congestion window size [bytes]>````


