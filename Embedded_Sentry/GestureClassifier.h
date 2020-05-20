/*
 * lreg.h
 *
 * Created: 5/10/2020 1:56:10 AM
 *  Author:Rishi
 */ 


#ifndef GESTURE_CLASSIFIER_H_
#define GESTURE_CLASSIFIER_H_


bool isGesture(int *gesture_one, int *gesture_two, int *gesture_test);
bool differenceInDirection(int, int);
int max (int, int);
int lcs(int *sequence_one,int *sequence_two,int sequence_one_length,int sequence_two_length, int *output);


#endif /* GESTURE_CLASSIFIER_H_ */