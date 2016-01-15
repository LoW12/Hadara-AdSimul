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

- cd bin/
- cmake ..
- make 

How to run
==============

Hadara-AdSimul-Red:
- ./Hadara_AdSimul_Gen [Name] [Path to the cvars file] [Number of iteration to be performed]

Hadara-AdSimul-Gen: 
- ./Hadara_AdSimul_Red [Path to the WF-Net]

Benchmark
==============

Under folder "testSet1" you cand find a set of one hundred generalised sound WF-nets with size
ranging from 281 nodes (146 transitions) to 13967 nodes (7230 transitions).

The plot below shown the reduction time of WF-net of this experimental set abtained on a personal 
laptop featuring an Intel core i7-3740QM @ 2.70GHz processor (using only a single core).

![](https://raw.githubusercontent.com/LoW12/Hadara-AdSimul/master/testSet1plot.png)


Contacts
========

- BRIDE Hadrien (hadrien.bride@femto-st.fr).

License
=======

Hadara-AdSimul is available under the CeCILL license. See the LICENSE file for more info.


