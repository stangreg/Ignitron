# SparkHardwareManager Refactoring

## Overview

This document outlines the refactoring of hardware initialization logic in the Ignitron firmware. The primary goal was to move hardware-specific functionality out of `SparkDataControl` into a dedicated `SparkHardwareManager` class. This improves modularity, maintainability, and separation of concerns.

## Key Changes

1. Created a `SparkHardwareManager` singleton class that:
   - Manages BLE connectivity (both client and server modes)
   - Handles keyboard initialization and access
   - Encapsulates hardware-specific initialization based on operation mode

2. Updated `SparkDataControl` to:
   - Use `SparkHardwareManager` for hardware-related operations
   - Removed direct hardware access, replacing with calls to the hardware manager
   - Deprecated hardware-specific static members in favor of manager-provided instances

3. BLE Notification Handling:
   - The BLE notification callback remains in `SparkDataControl` for now
   - `SparkHardwareManager` uses the callback function directly

## Usage Pattern

Instead of directly accessing hardware components in `SparkDataControl`, classes should now use the hardware manager:

```cpp
// Before
if (sparkDataControl.bleKeyboard.isConnected()) {
    // Do something with keyboard
}

// After
SparkBLEKeyboard* keyboard = SparkHardwareManager::getInstance().getBleKeyboard();
if (keyboard->isConnected()) {
    // Do something with keyboard
}
```

## Future Improvements

Further refactoring could involve:

1. Moving more hardware-specific logic out of `SparkDataControl`
2. Creating specialized managers for different hardware components (display, LEDs, etc.)
3. Implementing dependency injection for better testability
4. Further reducing static dependencies between classes
5. Moving the BLE notification callback to `SparkHardwareManager` with proper registration

## Benefits

1. Better separation of concerns - hardware management is now cleanly separated from data management
2. Improved testability - hardware components can be mocked more easily
3. Easier maintenance - changes to hardware functionality are isolated to a single class
4. Clearer dependencies - the relationship between hardware and data components is more explicit

## References

- `SparkHardwareManager.h/.cpp` - New hardware management class
- `SparkDataControl.h/.cpp` - Updated to use the hardware manager
