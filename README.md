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
