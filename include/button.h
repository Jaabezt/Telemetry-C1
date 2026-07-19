#ifndef BUTTON_H
#define BUTTON_H

void initButton();

// Call every loop() iteration. Debounces the button and toggles
// logging (via startLogging()/stopLogging()) on a clean press.
void updateButton();

#endif