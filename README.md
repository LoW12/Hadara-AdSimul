Hadara-AdSimul
==============

![](https://raw.githubusercontent.com/LoW12/Hadara-AdSimul/master/logo.png)

An open source tool suite for analysing Workflow Nets.

Hadara-AdSimul-Gen
==================

A tool able to generate large generalised sound Workflow Nets.

Hadara-AdSimul-Red
==================

A tool able to reduce large generalised sound Workflow Nets while preserving generalised soundness.

How to compile
==============
Compilation example : 

- Edit z3's include and bin folder in CMakeLists.txt (z3 v4.3.2.6 available https://github.com/Z3Prover/z3)
- cd bin/
- cmake ..
- make 

How to run
==============

Hadara-AdSimul-Gen:
- ./Hadara_AdSimul_Gen [Name] [Path to the cvars file] [Number of iteration to be performed]

Hadara-AdSimul-Red: 
- ./Hadara_AdSimul_Red [Path to the WF-Net]

Benchmark
==============

Under folder "DataSet1" you can find a set of 48 generalised sound WF-nets with size
ranging from 371 nodes (165 transitions) to 17815 nodes (9233 transitions).

Under folder "DataSet2" you can find a set of 59 generalised sound WF-nets with size
ranging from 1836 nodes (1000 transitions) to 147640 nodes (75782 transitions).

The plots below shows the reduction time of WF-net of the two data set obtained on a personal 
laptop featuring an Intel core i7-3740QM @ 2.70GHz processor (using only a single core).

![](https://raw.githubusercontent.com/LoW12/Hadara-AdSimul/master/DataSet1.png)
![](https://raw.githubusercontent.com/LoW12/Hadara-AdSimul/master/DataSet2.png)

Contacts
========

- BRIDE Hadrien (hadrien.bride@femto-st.fr).

License
=======

Hadara-AdSimul is available under the CeCILL license. See the LICENSE file for more info.


