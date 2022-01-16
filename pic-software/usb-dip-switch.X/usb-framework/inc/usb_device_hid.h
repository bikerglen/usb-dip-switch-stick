// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright 2015 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license),
please contact mla_licensing@microchip.com
*******************************************************************************/
//DOM-IGNORE-END

#ifndef HID_H
#define HID_H


/** INCLUDES *******************************************************/

#include "usb_ch9.h"

/** DEFINITIONS ****************************************************/

/* Class-Specific Requests */
#define GET_REPORT      0x01
#define GET_IDLE        0x02
#define GET_PROTOCOL    0x03
#define SET_REPORT      0x09
#define SET_IDLE        0x0A
#define SET_PROTOCOL    0x0B

/* Class Descriptor Types */
#define DSC_HID         0x21
#define DSC_RPT         0x22
#define DSC_PHY         0x23

/* Protocol Selection */
#define BOOT_PROTOCOL   0x00
#define RPT_PROTOCOL    0x01

/* HID Interface Class Code */
#define HID_INTF                    0x03

/* HID Interface Class SubClass Codes */
#define BOOT_INTF_SUBCLASS          0x01

/* HID Interface Class Protocol Codes */
#define HID_PROTOCOL_NONE           0x00
#define HID_PROTOCOL_KEYBOARD       0x01
#define HID_PROTOCOL_MOUSE          0x02

/********************************************************************
	Function:
		void USBCheckHIDRequest(void)

 	Summary:
 		This routine handles HID specific request that happen on EP0.
        This function should be called from the USBCBCheckOtherReq() call back
        function whenever implementing a HID device.

 	Description:
 		This routine handles HID specific request that happen on EP0.  These
        include, but are not limited to, requests for the HID report
        descriptors.  This function should be called from the
        USBCBCheckOtherReq() call back function whenever using an HID device.

        Typical Usage:
        <code>
        void USBCBCheckOtherReq(void)
        {
            //Since the stack didn't handle the request I need to check
            //  my class drivers to see if it is for them
            USBCheckHIDRequest();
        }
        </code>

	PreCondition:
		None

	Parameters:
		None

	Return Values:
		None

	Remarks:
		None

 *******************************************************************/
void USBCheckHIDRequest(void);

/********************************************************************
    Function:
        bool HIDTxHandleBusy(USB_HANDLE handle)

    Summary:
        Retrieves the status of the buffer ownership

    Description:
        Retrieves the status of the buffer ownership.  This function will
        indicate if the previous transfer is complete or not.

        This function will take the input handle (pointer to a BDT entry) and
        will check the UOWN bit.  If the UOWN bit is set then that indicates
        that the transfer is not complete and the USB module still owns the data
        memory.  If the UOWN bit is clear that means that the transfer is
        complete and that the CPU now owns the data memory.

        For more information about the BDT, please refer to the appropriate
        datasheet for the device in use.

        Typical Usage:
        <code>
        //make sure that the last transfer isn't busy by checking the handle
        if(!HIDTxHandleBusy(USBInHandle))
        {
            //Send the data contained in the ToSendDataBuffer[] array out on
            //  endpoint HID_EP
            USBInHandle = HIDTxPacket(HID_EP,(uint8_t*)&ToSendDataBuffer[0],sizeof(ToSendDataBuffer));
        }
        </code>

    PreCondition:
        None.

    Parameters:
        USB_HANDLE handle - the handle for the transfer in question.
        The handle is returned by the HIDTxPacket() and HIDRxPacket()
        functions.  Please insure that USB_HANDLE objects are initialized
        to NULL.

    Return Values:
        TRUE - the HID handle is still busy
        FALSE - the HID handle is not busy and is ready to send
                additional data.

   Remarks:
        None

 *******************************************************************/
#define HIDTxHandleBusy(handle) USBHandleBusy(handle)

/********************************************************************
    Function:
        bool HIDRxHandleBusy(USB_HANDLE handle)

    Summary:
        Retrieves the status of the buffer ownership

    Description:
        Retrieves the status of the buffer ownership.  This function will
        indicate if the previous transfer is complete or not.

        This function will take the input handle (pointer to a BDT entry) and
        will check the UOWN bit.  If the UOWN bit is set then that indicates
        that the transfer is not complete and the USB module still owns the data
        memory.  If the UOWN bit is clear that means that the transfer is
        complete and that the CPU now owns the data memory.

        For more information about the BDT, please refer to the appropriate
        datasheet for the device in use.

        Typical Usage:
        <code>
        if(!HIDRxHandleBusy(USBOutHandle))
        {
            //The data is available in the buffer that was specified when the
            //  HIDRxPacket() was called.
        }
        </code>

    PreCondition:
        None

    Parameters:
        USB_HANDLE handle - the handle for the transfer in question.
        The handle is returned by the HIDTxPacket() and HIDRxPacket()
        functions.  Please insure that USB_HANDLE objects are initialized
        to NULL.

    Return Values:
        TRUE - the HID handle is still busy
        FALSE - the HID handle is not busy and is ready to receive
                additional data.

   Remarks:
        None

 *******************************************************************/
#define HIDRxHandleBusy(handle) USBHandleBusy(handle)

/********************************************************************
    Function:
        USB_HANDLE HIDTxPacket(uint8_t ep, uint8_t* data, uint16_t len)

    Summary:
        Sends the specified data out the specified endpoint

    Description:
        This function sends the specified data out the specified
        endpoint and returns a handle to the transfer information.

        Typical Usage:
        <code>
        //make sure that the last transfer isn't busy by checking the handle
        if(!HIDTxHandleBusy(USBInHandle))
        {
            //Send the data contained in the ToSendDataBuffer[] array out on
            //  endpoint HID_EP
            USBInHandle = HIDTxPacket(HID_EP,(uint8_t*)&ToSendDataBuffer[0],sizeof(ToSendDataBuffer));
        }
        </code>

    PreCondition:
        None

    Parameters:
        uint8_t ep    - the endpoint you want to send the data out of
        uint8_t* data - pointer to the data that you wish to send
        uint16_t len   - the length of the data that you wish to send

    Return Values:
        USB_HANDLE - a handle for the transfer.  This information
        should be kept to track the status of the transfer

    Remarks:
        None

 *******************************************************************/
#define HIDTxPacket USBTxOnePacket

/********************************************************************
    Function:
        USB_HANDLE HIDRxPacket(uint8_t ep, uint8_t* data, uint16_t len)

    Summary:
        Receives the specified data out the specified endpoint

    Description:
        Receives the specified data out the specified endpoint.

        Typical Usage:
        <code>
        //Read 64-uint8_ts from endpoint HID_EP, into the ReceivedDataBuffer array.
        //  Make sure to save the return handle so that we can check it later
        //  to determine when the transfer is complete.
        USBOutHandle = HIDRxPacket(HID_EP,(uint8_t*)&ReceivedDataBuffer,64);
        </code>

    PreCondition:
        None

    Parameters:
        uint8_t ep    - the endpoint you want to receive the data into
        uint8_t* data - pointer to where the data will go when it arrives
        uint16_t len   - the length of the data that you wish to receive

    Return Values:
        USB_HANDLE - a handle for the transfer.  This information
        should be kept to track the status of the transfer

    Remarks:
        None

 *******************************************************************/
#define HIDRxPacket USBRxOnePacket

// Section: STRUCTURES *********************************************/

//USB HID Descriptor header as detailed in section
//"6.2.1 HID Descriptor" of the HID class definition specification
typedef struct _USB_HID_DSC_HEADER
{
    uint8_t bDescriptorType;	//offset 9
    uint16_t wDscLength;		//offset 10
} USB_HID_DSC_HEADER;

//USB HID Descriptor header as detailed in section
//"6.2.1 HID Descriptor" of the HID class definition specification
typedef struct _USB_HID_DSC
{
    uint8_t bLength;			//offset 0
	uint8_t bDescriptorType;	//offset 1
	uint16_t bcdHID;			//offset 2
    uint8_t bCountryCode;		//offset 4
	uint8_t bNumDsc;			//offset 5


    //USB_HID_DSC_HEADER hid_dsc_header[HID_NUM_OF_DSC];
    /* HID_NUM_OF_DSC is defined in usbcfg.h */

} USB_HID_DSC;

/** Section: EXTERNS ********************************************************/
extern volatile CTRL_TRF_SETUP SetupPkt;
extern const uint8_t configDescriptor1[];
extern volatile uint8_t CtrlTrfData[USB_EP0_BUFF_SIZE];

#endif //HID_H
