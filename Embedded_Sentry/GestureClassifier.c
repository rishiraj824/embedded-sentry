/*
 * GestureClassifier.c
 *
 * Created: 5/10/2020 1:16:35 AM
 * 
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "GestureClassifier.h"

#define ERROR_GAP 1000
const int ARRAY_LIMIT = 20;
// routine for comparing two sequences 
/*

	For the directions defined as,
	enum directions { X_POS = 1, X_NEG = -1, Y_POS = 2, Y_NEG = -2 , Z_POS = 3, Z_NEG = -3 };
	
	eg. 
	As shown in the video Y_POS then X_POS.

	Gesture Recording 1 - {1 -2 -1 2 0 0 0 0 0 0}
	Gesture Recording 2 - {-2 -2 -3 2 3 0 0 0 0 0}

	return {-2 2 0 0 0 0 0 0 0 0}

*/
int lcs(int *a,int *b,int n,int m, int *lcss){
	int l[n+1][m+1],i,j; for(i=0;i<=n;i++){
		for(j=0;j<=m;j++){
			if(i==0 || j==0){
				l[i][j]=0;
			}
			else if(a[i-1]==b[j-1]){
				l[i][j]=l[i-1][j-1]+1;
			} else l[i][j]=max(l[i-1][j],l[i][j-1]);

		}
	}

	int index=l[n][m];
	int p=index-1;
	
	i=n;j=m;

	while(p>=0){
		if(a[i-1]==b[j-1]){
			lcss[p]=a[i-1];
			i--;
			j--;
			p--;
		}
		else if(l[i-1][j]>l[i][j-1]){
			i--;
		}
		else{
			j--;
		}
	}
	for(int k=0;k<index;k++){
		printf("%d ",lcss[k]);
	}
	return lcss;
}

// routine to return maximum of two integers
int max(int a,int b){
	return (a>b)? a:b;
}

// routine to return whether the test gesture matched the recorded gesture 
bool isGesture(int *gesture_one_directions, int *gesture_two_directions, int *test_directions) {
	// initialize two arrays of size of the gesture to find common sequence
	int * common_gesture = malloc((sizeof(gesture_one_directions)/sizeof(gesture_one_directions[0])) * sizeof(gesture_one_directions));
	int * test_common = malloc((sizeof(gesture_one_directions)/sizeof(gesture_one_directions[0])) * sizeof(gesture_one_directions));
	
	// find the common sequence between the two gestures
	lcs(gesture_one_directions, gesture_two_directions, sizeof(gesture_one_directions)/sizeof(gesture_one_directions[0]), sizeof(gesture_two_directions)/sizeof(gesture_two_directions[0]), common_gesture);
	
	// find the common sequence between any gesture and the test gesture
	lcs(test_directions, gesture_one_directions, sizeof(test_directions)/sizeof(test_directions[0]), sizeof(gesture_one_directions)/sizeof(gesture_one_directions[0]), test_common);

	
	bool classified = true;
	
	// check if both the sequences match
	for (int i=0;i<sizeof(common_gesture)/sizeof(common_gesture[0]);i++){
		if(common_gesture[i] != test_common[i]){
			classified = false;
		}
	}
	return classified;
}

// routine to detect whether there was a change in direction along any of the two values at time (t - 1) and (t)
bool differenceInDirection(int x, int y){
	if (x>y && x-y>ERROR_GAP) {
			return true;
	}
	else if(y>x && y-x>ERROR_GAP) {
		return true;
	}
	return false;
}
