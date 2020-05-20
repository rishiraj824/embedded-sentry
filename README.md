#### Gesture Recognition Application using Microcontroller SAMD21 and Sensor MPU6050

This project uses Longest Common Subsequence to match two sequences of directions and compare them finally.

Directions are in enum types defined via - 
```
// states to store the directions +-(x,y,z)
enum directions { X_POS = 1, X_NEG = -1, Y_POS = 2, Y_NEG = -2 , Z_POS = 3, Z_NEG = -3 };
```

A change in direction is sort of detected when there is an acceleration change of the magnitude of 1500. 
Details on the connections mentioned in the file `./Embedded_Sentry/main.c`
