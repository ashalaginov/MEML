/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: ashalaginov 
 *
 * Created on August 30, 2019, 12:07 PM
 */

#include <cstdlib>
#include <math.h>
#include <chrono>
#include <stdio.h>
#include <signal.h>
#include <stdint.h>

#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdlib>

//Boost
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>  
#include <boost/optional.hpp>
#include <boost/numeric/ublas/vector.hpp>


//MQTT
#include <mosquittopp.h>

using namespace std;

//function definition
void toTerminal(std::vector<boost::numeric::ublas::vector<double> > & Input);
uint32_t micros();


#define mqtt_host "localhost"
#define mqtt_port 1883

static int run = 1;

void handle_signal(int s) {
    run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result) {
    printf("connect callback, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
    bool match = 0;
    printf("got message '%.*s' for topic '%s'\n", message->payloadlen, (char*) message->payload, message->topic);

    mosquitto_topic_matches_sub("/devices/wb-adc/controls/+", message->topic, &match);
    if (match) {
        printf("got message for ADC topic\n");
    }

}

/******************************************************************
ArduinoANN - An artificial neural network for the Arduino
All basic settings can be controlled via the Network Configuration
section.
See robotics.hobbizine.com/arduinoann.html for details.
 ******************************************************************/


/******************************************************************
   Network Configuration - customized per network
 ******************************************************************/

const int PatternCount = 100; //normal | neptune
const int Epochs = 1000; //2147483647
const int InputNodes = 9;
const int HiddenNodes = 1;
const int OutputNodes = 1;
const float LearningRate = 0.3;
const float Momentum = 0.9;
const float InitialWeightMax = 0.5;
const float Success = 0.01;


/******************************************************************
   End Network Configuration
 ******************************************************************/


int i, j, p, q, r;
int ReportEvery1000;
int RandomizedIndex[PatternCount];
long TrainingCycle;
float Rando;
float Error;
float Accum;


float Hidden[HiddenNodes];
float Output[OutputNodes];
float HiddenWeights[InputNodes + 1][HiddenNodes];
float OutputWeights[HiddenNodes + 1][OutputNodes];
float HiddenDelta[HiddenNodes];
float OutputDelta[OutputNodes];
float ChangeHiddenWeights[InputNodes + 1][HiddenNodes];
float ChangeOutputWeights[HiddenNodes + 1][OutputNodes];

/*
 * 
 */
int main(int argc, char** argv) {
    //Read and push tain data?d
    FILE *pFileTrain; //pointer to train and test files
    boost::numeric::ublas::vector<double> inputDataAttributes(InputNodes);
    std::vector<boost::numeric::ublas::vector<double> > Input;
    std::vector<int> ClassTmp;
    std::vector<std::vector<int> > Target;

    unsigned int fileStatus;
    double tmp1, tmp; //temporary variables
    int cl;
    unsigned int size = 0; //temporary variable

    // Short alias for this namespace
    namespace pt = boost::property_tree;

    // Create a root
    pt::ptree root;

    //Initialization of the Train data from file
    std::string path = "NSL_9ft_raw_processed.csv"; //9 features - 2 classes

    if ((pFileTrain = fopen(path.c_str(), "rt")) == NULL)
        puts("Error while opening input train file!");

    //Parsing train file content into data structure
    while (!feof(pFileTrain)) {

        //sample_type sample;
        for (int i = 0; i < InputNodes; i++) {
            fileStatus = fscanf(pFileTrain, "%lf ", &tmp1);
            inputDataAttributes(i) = tmp1;
            //printf("%lf ", tmp1);

        }

        fileStatus = fscanf(pFileTrain, "%d ", &cl);
        ClassTmp.push_back(cl);
        Target.push_back(ClassTmp);
        // printf("cl %fl \n", floor(cl));
        ClassTmp.clear();

        fileStatus = fscanf(pFileTrain, "\n");
        Input.push_back(inputDataAttributes);
        size++;

        if (size > PatternCount)
            break;
    }
    fclose(pFileTrain);


    //Initiatization
    ReportEvery1000 = 1;
    for (p = 0; p < PatternCount; p++) {
        RandomizedIndex[p] = p;
    }

    //ANN training
    for (unsigned int t = 0; t < 1; t++) {

        /******************************************************************
          Initialize HiddenWeights and ChangeHiddenWeights
         ******************************************************************/

        for (i = 0; i < HiddenNodes; i++) {
            for (j = 0; j <= InputNodes; j++) {
                ChangeHiddenWeights[j][i] = 0.0;
                Rando = float(rand() % 100) / 100;
                HiddenWeights[j][i] = 2.0 * (Rando - 0.5) * InitialWeightMax;
            }
        }
        /******************************************************************
          Initialize OutputWeights and ChangeOutputWeights
         ******************************************************************/

        for (i = 0; i < OutputNodes; i++) {
            for (j = 0; j <= HiddenNodes; j++) {
                ChangeOutputWeights[j][i] = 0.0;
                Rando = float(rand() % 100) / 100;
                OutputWeights[j][i] = 2.0 * (Rando - 0.5) * InitialWeightMax;
            }
        }
        //Serial.println("Initial/Untrained Outputs: ");
        toTerminal(Input);
        /******************************************************************
          Begin training
         ******************************************************************/

        for (TrainingCycle = 1; TrainingCycle < Epochs; TrainingCycle++) {

            /******************************************************************
              Randomize order of training patterns
             ******************************************************************/
            for (p = 0; p < PatternCount; p++) {
                //q = random(PatternCount);
                q = rand() % PatternCount;

                r = RandomizedIndex[p];
                RandomizedIndex[p] = RandomizedIndex[q];
                RandomizedIndex[q] = r;
            }
            Error = 0.0;
            /******************************************************************
              Cycle through each training pattern in the randomized order
             ******************************************************************/
            for (q = 0; q < PatternCount; q++) {
                p = RandomizedIndex[q];

                /******************************************************************
                  Compute hidden layer activations
                 ******************************************************************/

                for (i = 0; i < HiddenNodes; i++) {
                    Accum = HiddenWeights[InputNodes][i];
                    for (j = 0; j < InputNodes; j++) {
                        Accum += Input[p][j] * HiddenWeights[j][i];
                    }
                    Hidden[i] = 1.0 / (1.0 + exp(-Accum));
                }

                /******************************************************************
                  Compute output layer activations and calculate errors
                 ******************************************************************/
                for (i = 0; i < OutputNodes; i++) {
                    Accum = OutputWeights[HiddenNodes][i];
                    for (j = 0; j < HiddenNodes; j++) {
                        Accum += Hidden[j] * OutputWeights[j][i];
                    }
                    Output[i] = 1.0 / (1.0 + exp(-Accum));
                    OutputDelta[i] = (Target[p][i] - Output[i]) * Output[i] * (1.0 - Output[i]);
                    Error += 0.5 * (Target[p][i] - Output[i]) * (Target[p][i] - Output[i]);
                }

                /******************************************************************
                  Backpropagate errors to hidden layer
                 ******************************************************************/

                for (i = 0; i < HiddenNodes; i++) {
                    Accum = 0.0;
                    for (j = 0; j < OutputNodes; j++) {
                        Accum += OutputWeights[i][j] * OutputDelta[j];
                    }
                    HiddenDelta[i] = Accum * Hidden[i] * (1.0 - Hidden[i]);
                }


                /******************************************************************
                  Update Inner-->Hidden Weights
                 ******************************************************************/
                for (i = 0; i < HiddenNodes; i++) {
                    ChangeHiddenWeights[InputNodes][i] = LearningRate * HiddenDelta[i] + Momentum * ChangeHiddenWeights[InputNodes][i];
                    HiddenWeights[InputNodes][i] += ChangeHiddenWeights[InputNodes][i];
                    for (j = 0; j < InputNodes; j++) {
                        ChangeHiddenWeights[j][i] = LearningRate * Input[p][j] * HiddenDelta[i] + Momentum * ChangeHiddenWeights[j][i];
                        HiddenWeights[j][i] += ChangeHiddenWeights[j][i];
                    }
                }

                /******************************************************************
                  Update Hidden-->Output Weights
                 ******************************************************************/
                for (i = 0; i < OutputNodes; i++) {
                    ChangeOutputWeights[HiddenNodes][i] = LearningRate * OutputDelta[i] + Momentum * ChangeOutputWeights[HiddenNodes][i];
                    OutputWeights[HiddenNodes][i] += ChangeOutputWeights[HiddenNodes][i];
                    for (j = 0; j < HiddenNodes; j++) {
                        ChangeOutputWeights[j][i] = LearningRate * Hidden[j] * OutputDelta[i] + Momentum * ChangeOutputWeights[j][i];
                        OutputWeights[j][i] += ChangeOutputWeights[j][i];
                    }
                }
            }

            /******************************************************************
              Every 1000 cycles send data to terminal for display
             ******************************************************************/
            ReportEvery1000 = ReportEvery1000 - 1;
            if (ReportEvery1000 == 0) {

                printf("\n");
                printf("TrainingCycle: ");
                printf("%ld", TrainingCycle);
                printf("  Error = ");
                printf("%.5f", Error);

                toTerminal(Input);

                if (TrainingCycle == 1) {
                    ReportEvery1000 = 9;
                } else {
                    ReportEvery1000 = 10;
                }
            }


            /******************************************************************
              If error rate is less than pre-determined threshold then end
             ******************************************************************/

            if (Error < Success) break;
        }

        printf("\n");
        printf("TrainingCycle: ");
        printf("%ld", TrainingCycle);
        printf("  Error = ");
        printf("%.5f", Error);

        toTerminal(Input);

        printf("\n");
        printf("ANN Trained! ");
        printf("\n");
        ReportEvery1000 = 1;

    }



    /******************************************************************
     JSON
     ******************************************************************/
    //float HiddenWeights[InputNodes + 1][HiddenNodes];
    pt::ptree matrix_node;
    for (int i = 0; i <= InputNodes; i++) {
        pt::ptree row;
        for (int j = 0; j < HiddenNodes; j++) {
            // Create an unnamed value
            pt::ptree cell;
            //round_nplaces(HiddenWeights[i][j],3);

            std::stringstream stream;
            stream << std::fixed << std::setprecision(2) << HiddenWeights[i][j];
            std::string s = stream.str();

            cell.put_value(s);
            // Add the value to our row
            row.push_back(std::make_pair("", cell));
        }
        // Add the row to our matrix
        matrix_node.push_back(std::make_pair("", row));
    }
    // Add the node to the root
    root.add_child("HiddenWeights", matrix_node);

    //float OutputWeights[HiddenNodes + 1][OutputNodes];
    matrix_node.clear();

    for (int i = 0; i <= HiddenNodes; i++) {
        pt::ptree row;
        for (int j = 0; j < OutputNodes; j++) {
            // Create an unnamed value
            pt::ptree cell;
            //round_nplaces(HiddenWeights[i][j],3);

            std::stringstream stream;
            stream << std::fixed << std::setprecision(2) << OutputWeights[i][j];
            std::string s = stream.str();

            cell.put_value(s);
            // Add the value to our row
            row.push_back(std::make_pair("", cell));
        }
        // Add the row to our matrix
        matrix_node.push_back(std::make_pair("", row));
    }
    // Add the node to the root
    root.add_child("OutputWeights", matrix_node);
    printf("Final JSON Model:\n");
    pt::write_json(std::cout, root, false);

    return 0;
}

void toTerminal(std::vector<boost::numeric::ublas::vector<double> > & Input) {
    u_int32_t tss1 = micros();

    for (p = 0; p < PatternCount; p++) {
        /*
          Serial.println();
          Serial.print ("  Training Pattern: ");
          Serial.println (p);
          Serial.print ("  Input ");
          for ( i = 0 ; i < InputNodes ; i++ ) {
          Serial.print (Input[p][i], 2);
          Serial.print (" ");
          }
          Serial.print ("  Target ");
          for ( i = 0 ; i < OutputNodes ; i++ ) {
          Serial.print (Target[p][i], 2);
          Serial.print (" ");
          }
         */
        u_int32_t ts1 = micros();
        /******************************************************************
          Compute hidden layer activations
         ******************************************************************/

        for (i = 0; i < HiddenNodes; i++) {
            Accum = HiddenWeights[InputNodes][i];
            for (j = 0; j < InputNodes; j++) {
                Accum += Input[p][j] * HiddenWeights[j][i];
            }
            Hidden[i] = 1.0 / (1.0 + exp(-Accum));
        }

        /******************************************************************
          Compute output layer activations and calculate errors
         ******************************************************************/

        for (i = 0; i < OutputNodes; i++) {
            Accum = OutputWeights[HiddenNodes][i];
            for (j = 0; j < HiddenNodes; j++) {
                Accum += Hidden[j] * OutputWeights[j][i];
            }
            Output[i] = 1.0 / (1.0 + exp(-Accum));
        }
        uint32_t ts2 = micros();
        /*
            Serial.print ("  Output ");
            for ( i = 0 ; i < OutputNodes ; i++ ) {
              Serial.print (Output[i], 5);
              Serial.print (" ");
            }
            Serial.print ("  Prediction time, microseconds:");
            Serial.println(ts2 - ts1);
         */
    }
    u_int32_t tss2 = micros();
    //Serial.print("  Epoch time, microseconds:");
    printf("  Epoch time, microseconds:");
    printf("%d", tss2 - tss1);
    //Serial.println(tss2 - tss1);
    /*
        boost::property_tree::ptree pt;
        pt.put("Test", "string");
        pt.put("Test2.inner0", "string2");
        pt.put("Test2.inner1", "string3");
        pt.put("Test2.inner2", 1234);

        std::stringstream ss;
        boost::property_tree::json_parser::write_json(ss, pt);

        std::cout << ss.str() << std::endl;
     */
}


// Get time stamp in microseconds.

uint32_t micros() {
    uint32_t us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::
            now().time_since_epoch()).count();
    return us;
}

