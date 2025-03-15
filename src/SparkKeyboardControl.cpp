/*
 * SparkKeyboardControl.cpp
 *
 *  Created on: 12.02.2024
 *      Author: steffen
 */

#include "SparkKeyboardControl.h"

SparkKeyboardControl::SparkKeyboardControl() {
    init();
}

SparkKeyboardControl::~SparkKeyboardControl() {
}

void SparkKeyboardControl::init() {

    // Standard keyboard definition (1-6, A-D)
    KeyboardMapping mapping1;
    mapping1.mappingName = "Standard";
    mapping1.keyboardShortPress = {
        {1, '1', 0, 0, "1"},
        {2, '2', 0, 0, "2"},
        {3, '3', 0, 0, "3"},
        {4, '4', 0, 0, "4"},
        {5, '5', 0, 0, "5"},
        {6, '6', 0, 0, "6"}};
    mapping1.keyboardLongPress = {
        {11, 'A', 0, 0, "A"},
        {12, 'B', 0, 0, "B"},
        {13, 'C', 0, 0, "C"},
        {14, 'D', 0, 0, "D"}};

    // Special keyboard for YouTube
    KeyboardMapping mapping2;
    mapping2.mappingName = "YouTube";
    mapping2.keyboardShortPress = {
        {1,
         0xD8, // KEY_LEFT_ARROW
         0x81, // KEY_LEFT_SHIFT
         0,
         "<"},
        {2,
         0xD8, // KEY_LEFT_ARROW
         0,
         0,
         "<"},
        {3,
         ' ',
         0,
         0,
         "P"},
        {4,
         0xD7, // KEY_RIGHT_ARROW
         0,
         0,
         ">"},
        {5,
         0xD8, // KEY_LEFT_ARROW
         0,
         2, // x3
         "<"},
        {6,
         0xD7, // KEY_RIGHT_ARROW
         0,
         2, // x3
         ">"}};

    // Mapping for long press
    mapping2.keyboardLongPress = {
        {11,
         '0',
         0,
         0, // x5
         "B"},
        {12,
         0xD7, // KEY_RIGHT_ARROW
         0,
         5, // x6
         "F"},
        {13,
         '<', // speed down
         0,
         0,
         "-"},
        {14,
         '>', // speed up
         0,
         0,
         "+"}};

    // Adding default definitions for changing to previous and next keyboard
    keyboardKeyDefinition prevKeyboard =
        {15,
         0xD9, // KEY_DOWN_ARROW
         0,
         0,
         "K-"};

    keyboardKeyDefinition nextKeyboard =
        {16,
         0xDA, // KEY_UP_ARROW
         0,
         0,
         "K+"};

    mapping1.keyboardLongPress.push_back(prevKeyboard);
    mapping1.keyboardLongPress.push_back(nextKeyboard);

    mapping2.keyboardLongPress.push_back(prevKeyboard);
    mapping2.keyboardLongPress.push_back(nextKeyboard);

    keyboards.push_back(mapping1);
    keyboards.push_back(mapping2);
}

KeyboardMapping &SparkKeyboardControl::getNextKeyboard() {

    currentKeyboardIndex++;
    if (currentKeyboardIndex == keyboards.size()) {
        currentKeyboardIndex = 0;
    }
    return keyboards.at(currentKeyboardIndex);
}

KeyboardMapping &SparkKeyboardControl::getPreviousKeyboard() {

    currentKeyboardIndex--;
    if (currentKeyboardIndex < 0) {
        currentKeyboardIndex = keyboards.size() - 1;
    }
    return keyboards.at(currentKeyboardIndex);
}

KeyboardMapping &SparkKeyboardControl::getCurrentKeyboard() {
    return keyboards.at(currentKeyboardIndex);
}
