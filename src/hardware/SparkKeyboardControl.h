/*
 * SparkKeyboardControl.h
 *
 *  Created on: 12.02.2024
 *      Author: steffen
 */

#ifndef SPARKKEYBOARDCONTROL_H_
#define SPARKKEYBOARDCONTROL_H_

#include <vector>
#include "SparkTypes.h"

using namespace std;

class SparkKeyboardControl {
public:
	SparkKeyboardControl();
	virtual ~SparkKeyboardControl();

	void init();

	KeyboardMapping &getNextKeyboard();
	KeyboardMapping &getPreviousKeyboard();
	KeyboardMapping &getCurrentKeyboard();


private:
	vector<KeyboardMapping> keyboards;
	int currentKeyboardIndex = 0;

};

#endif /* SPARKKEYBOARDCONTROL_H_ */
