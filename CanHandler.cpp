/*
 * CanHandler.cpp
 *
 * Devices may register to this handler in order to receive CAN frames (publish/subscribe)
 * and they can also use this class to send messages.
 *
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

#include "CanHandler.h"
#include "sys_io.h"


#define SPI_CS_PIN 5 // CS Pins
#define CAN_INT_PIN = 6;

mcp2515_can CAN(SPI_CS_PIN); // Set CS pin
CanHandler canHandler = CanHandler();

/*
 * Constructor of the can handler
 */
CanHandler::CanHandler()
{

    for (int i = 0; i < CFG_CAN_NUM_OBSERVERS; i++)
    {
        observerData[i].observer = NULL;
    }
    masterID = 0x05;
    busSpeed = 0;
}

/*
 * Initialization of the CAN bus
 */
void CanHandler::setup()
{

    while (CAN_OK != CAN.begin(CAN_500KBPS))
    { // init can bus : baudrate = 500kf
        SERIAL_PORT_MONITOR.println("CAN init fail, retry...");
        delay(100);
    }

    Logger::info("CAN%d init ok. Speed = %i", 0, busSpeed);
}

uint32_t CanHandler::getBusSpeed()
{
    return busSpeed;
}

/*
 * Attach a CanObserver. Can frames which match the id/mask will be forwarded to the observer
 * via the method handleCanFrame(RX_CAN_FRAME).
 *
 *  \param observer - the observer object to register (must implement CanObserver class)
 *  \param id - the id of the can frame to listen to
 *  \param mask - the mask to be applied to the frames
 *  \param extended - set if extended frames must be supported
 */
void CanHandler::attach(CanObserver *observer, uint32_t id, uint32_t mask, bool extended)
{
    int8_t pos = findFreeObserverData();

    if (pos == -1)
    {
        Logger::error("no free space in CanHandler::observerData, increase its size via CFG_CAN_NUM_OBSERVERS");
        return;
    }

    observerData[pos].id = id;
    observerData[pos].mask = mask;
    observerData[pos].extended = extended;
    observerData[pos].observer = observer;

    CAN.init_Filt(pos, extended, id);
    CAN.init_Mask(pos, extended, (unsigned long)mask);

    Logger::debug("attached CanObserver (%X) for id=%X, mask=%X", observer, id, mask);
}

/*
 * Detaches a previously attached observer from this handler.
 *
 * \param observer - observer object to detach
 * \param id - id of the observer to detach (required as one CanObserver may register itself several times)
 * \param mask - mask of the observer to detach (dito)
 */
void CanHandler::detach(CanObserver *observer, uint32_t id, uint32_t mask)
{
    for (int i = 0; i < CFG_CAN_NUM_OBSERVERS; i++)
    {
        if (observerData[i].observer == observer &&
            observerData[i].id == id &&
            observerData[i].mask == mask)
        {
            observerData[i].observer = NULL;
        }
    }
}

/*
 * Logs the content of a received can frame
 *
 * \param frame - the received can frame to log
 */
void CanHandler::logFrame(CAN_FRAME &frame)
{
    if (Logger::isDebug())
    {
        Logger::debug("CAN: dlc=%X fid=%X id=%X ide=%X rtr=%X data=%X,%X,%X,%X,%X,%X,%X,%X",
                      frame.length, frame.fid, frame.id, frame.extended, frame.rtr,
                      frame.data[0], frame.data[1], frame.data[2], frame.data[3],
                      frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
    }
}

/*
 * Find a observerData entry which is not in use.
 *
 * \retval array index of the next unused entry in observerData[]
 */
int8_t CanHandler::findFreeObserverData()
{
    for (int i = 0; i < CFG_CAN_NUM_OBSERVERS; i++)
    {
        if (observerData[i].observer == NULL)
        {
            return i;
        }
    }

    return -1;
}

/*
 * If a message is available, read it and forward it to registered observers.
 */
void CanHandler::process()
{
    static CAN_FRAME frame;
    static SDO_FRAME sFrame;

    CanObserver *observer;

    unsigned char len = 0;
    unsigned char buf[8];

    if (CAN_MSGAVAIL == CAN.checkReceive())
    {

        CAN.readMsgBuf(&len, buf); // read data,  len: data length, buf: data buf

        frame.length = (uint8_t)len;
        for (int i = 0; i < len; i++)
        {
            frame.data[i] = uint8_t(buf[i]);
        }
        frame.id = CAN.getCanId();
        frame.extended = (bool)CAN.isExtendedFrame();
        frame.rtr = CAN.isRemoteRequest();

        Logger::debug("CAN:%d dlc=%X fid=%X id=%X ide=%X rtr=%X data=%X,%X,%X,%X,%X,%X,%X,%X", 0,
                      frame.length, frame.fid, frame.id, frame.extended, frame.rtr,
                      frame.data[0], frame.data[1], frame.data[2], frame.data[3],
                      frame.data[4], frame.data[5], frame.data[6], frame.data[7]);

        if (frame.id == CAN_SWITCH)
            CANIO(frame);

        for (int i = 0; i < CFG_CAN_NUM_OBSERVERS; i++)
        {
            observer = observerData[i].observer;
            if (observer != NULL)
            {
                // Apply mask to frame.id and observer.id. If they match, forward the frame to the observer
                if (observer->isCANOpen())
                {
                    if (frame.id > 0x17F && frame.id < 0x580)
                    {
                        observer->handlePDOFrame(&frame);
                    }

                    if (frame.id == 0x600 + observer->getNodeID()) // SDO request targetted to our ID
                    {
                        sFrame.nodeID = observer->getNodeID();
                        sFrame.index = frame.data[1] + (frame.data[2] * 256);
                        sFrame.subIndex = frame.data[3];
                        sFrame.cmd = (SDO_COMMAND)(frame.data[0] & 0xF0);

                        if ((frame.data[0] != 0x40) && (frame.data[0] != 0x60))
                        {
                            sFrame.dataLength = (3 - ((frame.data[0] & 0xC) >> 2)) + 1;
                        }
                        else
                            sFrame.dataLength = 0;

                        for (int x = 0; x < sFrame.dataLength; x++)
                            sFrame.data[x] = frame.data[4 + x];
                        observer->handleSDORequest(&sFrame);
                    }

                    if (frame.id == 0x580 + observer->getNodeID()) // SDO reply to our ID
                    {
                        sFrame.nodeID = observer->getNodeID();
                        sFrame.index = frame.data[1] + (frame.data[2] * 256);
                        sFrame.subIndex = frame.data[3];
                        sFrame.cmd = (SDO_COMMAND)(frame.data[0] & 0xF0);

                        if ((frame.data[0] != 0x40) && (frame.data[0] != 0x60))
                        {
                            sFrame.dataLength = (3 - ((frame.data[0] & 0xC) >> 2)) + 1;
                        }
                        else
                            sFrame.dataLength = 0;

                        for (int x = 0; x < sFrame.dataLength; x++)
                            sFrame.data[x] = frame.data[4 + x];

                        observer->handleSDOResponse(&sFrame);
                    }
                }
                else // raw canbus
                {
                    if ((frame.id & observerData[i].mask) == (observerData[i].id & observerData[i].mask))
                    {
                        observer->handleCanFrame(&frame);
                    }
                }
            }
        }
    }
}

/*
 * Prepare the CAN transmit frame.
 * Re-sets all parameters in the re-used frame.
 */
void CanHandler::prepareOutputFrame(CAN_FRAME *frame, uint32_t id)
{
    frame->length = 8;
    frame->id = id;
    frame->extended = 0;
    frame->rtr = 0;

    frame->data[0] = 0;
    frame->data[1] = 0;
    frame->data[2] = 0;
    frame->data[3] = 0;
    frame->data[4] = 0;
    frame->data[5] = 0;
    frame->data[6] = 0;
    frame->data[7] = 0;
}

void CanHandler::CANIO(CAN_FRAME &frame)
{
    static CAN_FRAME CANioFrame;
    int i;

    Logger::warn("CANIO %d msg: %X   %X   %X   %X   %X   %X   %X   %X  %X", 0, frame.id, frame.data[0],
                 frame.data[1], frame.data[2], frame.data[3], frame.data[4],
                 frame.data[5], frame.data[6], frame.data[7]);

    CANioFrame.id = CAN_OUTPUTS;
    CANioFrame.length = 8;
    CANioFrame.extended = 0; // standard frame
    CANioFrame.rtr = 0;

    // handle the incoming frame to set/unset/leave alone each digital output
    for (i = 0; i < 8; i++)
    {
        if (frame.data[i] == 0x88)
            systemIO.setDigitalOutput(i, true);
        if (frame.data[i] == 0xFF)
            systemIO.setDigitalOutput(i, false);
    }

    for (i = 0; i < 8; i++)
    {
        if (systemIO.getDigitalOutput(i))
            CANioFrame.data[i] = 0x88;
        else
            CANioFrame.data[i] = 0xFF;
    }

    this->sendFrame(CANioFrame);

    CANioFrame.id = CAN_ANALOG_INPUTS;
    i = 0;
    int16_t anaVal;

    for (int j = 0; j < 8; j += 2)
    {
        anaVal = systemIO.getAnalogIn(i++);
        CANioFrame.data[j] = highByte(anaVal);
        CANioFrame.data[j + 1] = lowByte(anaVal);
    }

    this->sendFrame(CANioFrame);

    CANioFrame.id = CAN_DIGITAL_INPUTS;
    CANioFrame.length = 4;

    for (i = 0; i < 4; i++)
    {
        if (systemIO.getDigitalIn(i))
            CANioFrame.data[i] = 0x88;
        else
            CANioFrame.data[i] = 0xff;
    }

    this->sendFrame(CANioFrame);
}

//(whatever happens to be open) or queue it to send (if nothing is open)
void CanHandler::sendFrame(CAN_FRAME &frame)
{

    Logger::warn("CANIO %d msg: %X   %X   %X   %X   %X   %X   %X   %X  %X", 0, frame.id, frame.data[0],
                 frame.data[1], frame.data[2], frame.data[3], frame.data[4],
                 frame.data[5], frame.data[6], frame.data[7]);

    CAN.MCP_CAN::sendMsgBuf(frame.id, frame.extended, frame.length, frame.data);
}

CanObserver::CanObserver()
{
    canOpenMode = false;
    nodeID = 0x7F;
}

// setting can open mode causes the can handler to run its own handleCanFrame system where things are sorted out
void CanObserver::setCANOpenMode(bool en)
{
    canOpenMode = en;
}

void CanObserver::setNodeID(int id)
{
    nodeID = id & 0x7F;
}

int CanObserver::getNodeID()
{
    return nodeID;
}

bool CanObserver::isCANOpen()
{
    return canOpenMode;
}

/*
 * Default implementation of the CanObserver method. Must be overwritten
 * by every sub-class. However, canopen devices should still call this version to save themselves some effort
 */
void CanObserver::handleCanFrame(CAN_FRAME *frame)
{
    Logger::error("CanObserver does not implement handleCanFrame(), frame.id=%d", frame->id);
}

void CanObserver::handlePDOFrame(CAN_FRAME *frame)
{
    Logger::error("CanObserver does not implement handlePDOFrame(), frame.id=%d", frame->id);
}

void CanObserver::handleSDORequest(SDO_FRAME *frame)
{
    Logger::error("CanObserver does not implement handleSDORequest(), frame.id=%d", frame->nodeID);
}

void CanObserver::handleSDOResponse(SDO_FRAME *frame)
{
    Logger::error("CanObserver does not implement handleSDOResponse(), frame.id=%d", frame->nodeID);
}

CAN_FRAME::CAN_FRAME()
{
	id = 0;
	fid = 0;
	rtr = 0;
	priority = 15;
	extended = false;
	timestamp = 0;
	length = 0;
	data[8] = {};
}