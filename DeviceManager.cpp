/*
 * DeviceManager.cpp
 *
 The device manager keeps a list of all devices which are installed into the system.
 Anything that needs either a tick handler, a canbus handler, or to communicate to other
 devices on the system must be registered with the manager. The manager then handles
 making sure the tick handler is set up (if needed), the canbus handler is set up (if need),
 and that a device can send information to other devices by either type (BMS, motor ctrl, etc)
 or by device ID.

 The device class itself has the handlers defined so the tick and canbus handling code
 need only call these existing functions but the manager interface needs to
 expose a way to register them with the system.

 Copyright (c) 2013 Collin Kidder, Michael Neuweiler, Charles Galpin

 Permission is hereby granted, free of charge, to any person obtaining
 a copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#include "DeviceManager.h"

DeviceManager::DeviceManager() {
    throttle = NULL;
    brake = NULL;
    motorController = NULL;
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++)
        devices[i] = NULL;
}

/*
 * Add the specified device to the list of registered devices
 */
void DeviceManager::addDevice(Device *device) {
    if (findDevice(device) == -1) {
        int8_t i = findDevice(NULL);
        if (i != -1) {
            devices[i] = device;
        } else {
            Logger::error("unable to register device, max number of devices reached.");
        }
    }
    /*
    switch (device->getType()) {
    case DEVICE_THROTTLE:
    	throttle = (Throttle *) device;
    	break;
    case DEVICE_BRAKE:
    	brake = (Throttle *) device;
    	break;
    case DEVICE_MOTORCTRL:
    	motorController = (MotorController *) device;
    	break;
    }
    */
}

/*
 * Remove the specified device from the list of registered devices
 */
void DeviceManager::removeDevice(Device *device) {
    int8_t i = findDevice(NULL);
    if (i != -1)
        devices[i] = NULL;
    switch (device->getType()) {
    case DEVICE_THROTTLE:
        throttle = NULL;
        break;
    case DEVICE_BRAKE:
        brake = NULL;
        break;
    case DEVICE_MOTORCTRL:
        motorController = NULL;
        break;
    }
}

/*Add a new tick handler to the specified device. It should
 //technically be possible to register for multiple intervals
 //and be called for all of them but support for that is not
 //immediately necessary
 */
//void DeviceManager::addTickObserver(TickObserver *observer, uint32_t frequency) {
//}

/*Add a new filter that sends frames through to the device. There definitely has
 to be support for multiple filters per device right from the beginning.
 Mask, id, ext form the filter. canbus sets whether to attach to
 CAN0 or CAN1.
 */
//void addCanObserver(CanObserver *observer, uint32_t id, uint32_t mask, bool extended) {
//}

/*
 Send an inter-device message. Devtype has to be filled out but could be DEVICE_ANY.
 If devId is anything other than INVALID (0xFFFF) then the message will be targetted to only
 one device. Otherwise it will broadcast to any device that matches the device type (or all
 devices in the case of DEVICE_ANY).
 DeviceManager.h has a list of standard message types but you're allowed to send
 whatever you want. The standard message types are to enforce standard messages for easy
 intercommunication.
 */
void DeviceManager::sendMessage(DeviceType devType, DeviceId devId, uint32_t msgType, void* message)
{
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++)
    {
        if (devices[i] && devices[i]->isEnabled()) //does this object exist and is it enabled?
        {
            if (devType == DEVICE_ANY || devType == devices[i]->getType())
            {
                if (devId == INVALID || devId == devices[i]->getId())
                {
                    Logger::debug("Sending msg to device with ID %X", devices[i]->getId());
                    devices[i]->handleMessage(msgType, message);
                }
            }
        }
    }
}

void DeviceManager::setParameter(DeviceType deviceType, DeviceId deviceId, uint32_t msgType, char *key, char *value) {
    char *params[] = { key, value };
    sendMessage(deviceType, deviceId, msgType, params);
}

void DeviceManager::setParameter(DeviceType deviceType, DeviceId deviceId, uint32_t msgType, char *key, uint32_t value) {
    char buffer[15];
    sprintf(buffer, "%lu", value);
    setParameter(deviceType, deviceId, msgType, key, buffer);
}

uint8_t DeviceManager::getNumThrottles() {
    return countDeviceType(DEVICE_THROTTLE);
}

uint8_t DeviceManager::getNumControllers() {
    return countDeviceType(DEVICE_MOTORCTRL);
}

uint8_t DeviceManager::getNumDisplays() {
    return countDeviceType(DEVICE_DISPLAY);
}

Throttle *DeviceManager::getAccelerator() {
    //try to find one if nothing registered. Cache it if we find one
    if (!throttle) throttle = (Throttle *)getDeviceByType(DEVICE_THROTTLE);

    //if there is no throttle then instantiate a dummy throttle
    //so down range code doesn't puke
    if (!throttle)
    {
        //Logger::debug("getAccelerator() called but there is no registered accelerator!");
        return 0; //NULL!
    }
    return throttle;
}

Throttle *DeviceManager::getBrake() {

    if (!brake) brake = (Throttle *)getDeviceByType(DEVICE_BRAKE);

    if (!brake)
    {
        //Logger::debug("getBrake() called but there is no registered brake!");
        return 0; //NULL!
    }
    return brake;
}

MotorController *DeviceManager::getMotorController() {
    if (!motorController) motorController = (MotorController *)getDeviceByType(DEVICE_MOTORCTRL);

    if (!motorController)
    {
        //Logger::debug("getMotorController() called but there is no registered motor controller!");
        return 0; //NULL!
    }
    return motorController;
}

/*
Allows one to request a reference to a device with the given ID. This lets code specifically request a certain
device. Normally this would be a bad idea because it sort of breaks the OOP design philosophy of polymorphism
but sometimes you can't help it.
*/
Device *DeviceManager::getDeviceByID(DeviceId id)
{
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++)
    {
        if (devices[i])
        {
            if (devices[i]->getId() == id) return devices[i];
        }
    }
    //Logger::debug("getDeviceByID - No device with ID: %X", (int)id);
    return 0; //NULL!
}

/*
The more object oriented version of the above function. Allows one to find the first device that matches
a given type.
*/
Device *DeviceManager::getDeviceByType(DeviceType type)
{
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++)
    {
        if (devices[i] && devices[i]->isEnabled())
        {
            if (devices[i]->getType() == type) return devices[i];
        }
    }
    //Logger::debug("getDeviceByType - No devices of type: %X", (int)type);
    return 0; //NULL!
}

/*
 * Find the position of a device in the devices array
 * /retval the position of the device or -1 if not found.
 */
int8_t DeviceManager::findDevice(Device *device) {
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (device == devices[i])
            return i;
    }
    return -1;
}

/*
 * Count the number of registered devices of a certain type.
 */
uint8_t DeviceManager::countDeviceType(DeviceType deviceType) {
    uint8_t count = 0;
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i]->getType() == deviceType)
            count++;
    }
    return count;
}

void DeviceManager::printDeviceList() {
    Logger::console("\n  ENABLED devices: (DISABLE=0xFFFF to disable where FFFF is device number)\n");
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i] && devices[i]->isEnabled()) {
            Logger::console("     %X     %s", devices[i]->getId(), devices[i]->getCommonName());
        }
    }

    Logger::console("\n  DISABLED devices: (ENABLE=0xFFFF to enable where FFFF is device number)\n");
    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {
        if (devices[i] && !devices[i]->isEnabled()) {
            Logger::console("     %X     %s", devices[i]->getId(), devices[i]->getCommonName());
        }
    }
}


void DeviceManager::updateWifi() {

    sendMessage(DEVICE_WIFI, ICHIP2128, MSG_CONFIG_CHANGE, NULL);  //Load all our other parameters first

    char param [2][30];  //A two element array containing id and enable state
    char *paramPtr[2] = { &param[0][0], &param[1][0] }; //A two element array of pointers, pointing to the addresses of row 1 and row 2 of array.
    //paramPtr[0] then contains address of param row 0 element 0
    //paramPtr[1] then contains address of param row 1 element 0.


    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) { //Find all devices that are enabled and load into array
        if (devices[i] && devices[i]->isEnabled())
        {
            sprintf(paramPtr[0],"x%X",devices[i]->getId());
            sprintf(paramPtr[1],"255");
            //   Logger::console(" Device: %s value %s", paramPtr[0], paramPtr[1]);

            sendMessage(DEVICE_WIFI, ICHIP2128, MSG_SET_PARAM,  paramPtr);	//Send the array to ichip by id (ie 1031)  255 indicates enabled
        }
    }

    for (int i = 0; i < CFG_DEV_MGR_MAX_DEVICES; i++) {    //Find all devices that are NOT enabled and load into array
        if (devices[i] && !devices[i]->isEnabled())
        {
            sprintf(paramPtr[0],"x%X",devices[i]->getId());
            sprintf(paramPtr[1],"0");
            // Logger::console(" Device: %s value %s", paramPtr[0], paramPtr[1]);
            sendMessage(DEVICE_WIFI, ICHIP2128, MSG_SET_PARAM,  paramPtr);        //Send array to ichip by id (ie 1002) 0 indicates disabled
        }
    }


}

//Create a permanent instance of the device manager useable from anywhere.
DeviceManager deviceManager;

