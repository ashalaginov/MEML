# MEML
Resource-aware MQTT-based Machine Learning forNetwork Attacks Detection on IoT Edge Devices


## Setup 
![IoT Ecosystem setup](setup.png)


## Neuro-Fuzzy-Simple

Simple implementation of Neuro-Fuzzy algorithm in C++. The program demonstrates how Neuro-Fuzzy architecture works: 
starting from assigning linguistic terms, building fuzzy patches and ending up with generating classification model.

## Fuzzy Patches generation

For each linguistic term we use simple statistical approach without prior knowledge about the dataset. 
For 3-terms fuzzy set "three-sigma rule of thumb" is used to generate 3 linguistic terms: {Low, Medium, High}. For 5-terms fuzzy set corresponding 5 standard deviations are used.
Finally, simple rectangular fuzzy patches are generated.

## Requirements:

- g++ (tested on v. 4.7.3 and higher)
- STL containers for data operations
- OpenMP for parallel execution (v. 3.1 and higher)
- Doxygen-friendly

## Misc


## Original Paper
You can find more information about the practical experiments and datasets in the following conference paper:
@inproceedings{shalaginov2019meml,
  title={MEML: Resource-aware MQTT-based Machine Learning for Network Attacks Detection on IoT Edge Devices},
  author={Shalaginov, Andrii and Semeniuta, Oleksandr and Alazab, Mamoun},
  booktitle={Proceedings of the 12th IEEE/ACM International Conference on Utility and Cloud Computing Companion},
  pages={123--128},
  year={2019}
}
