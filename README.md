# Embedded-Sentry
Embedded Challenge Spring 2020 (Term Project)

## Project Objective

Using Arduino Uno and an accelerometer (MPU-6050), detect and record a sequence of hand motions. Save this sequence as the key sequence that will be used when attempting to check whether a successful unlock is completed. Attempts will then be made to replicate the key sequence within sufficient tolerances to unlock the resource. A successful unlock is indicated by a visual indication, such as an LED or displays on screen (serial monitor). 

## Project Status

Currently the project objective has been achieved. However, there remains inaccuracies in motion detection using the accelerometer. It is noticed that the accelerometer would have random moments where it detects the opposite of the intended motion which adds to the inaccuracy. Another cause for the inacurracy is the sensitivity of the accelerometer and human error; it is noticed that sometimes the slightest quick (and unintended) motions of the hand that is holding the accelerometer is captured and recorded. Currently, these inaccuracies are unsolved; however, movements that are captured are displayed, and if it is incorrect or unintended, the user can reset and start over their movement sequence. 

## Library Dependencies 

Search for and install Adafruit MPU6050  (in Library Manager of Arduino IDE)
