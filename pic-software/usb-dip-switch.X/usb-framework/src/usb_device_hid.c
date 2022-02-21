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

/*******************************************************************************
  USB Device Human Interface Device (HID) Layer

  Company:
    Microchip Technology Inc.

  File Name:
    usb_device_hid.c

  Summary:
    USB Device Human Interface Device (HID) Layer interface API.

  Description:
    USB Device Human Interface Device (HID) Layer interface API.
*******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "usb_config.h"
#include "usb.h"
#include "usb_device_hid.h"

#if defined(__XC8)
    #define PACKED
    #define ALIGNED
#else
    #define PACKED __attribute__((packed))
    #define ALIGNED __attribute__((aligned))
#endif

// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Constants
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: File Scope Data Types
// *****************************************************************************
// *****************************************************************************
typedef struct PACKED
{
    unsigned :8;
    unsigned :8;
    uint8_t reportId;
    uint8_t duration;
} USB_SETUP_SET_IDLE_RATE;

typedef struct PACKED
{
    unsigned :8;
    unsigned :8;
    uint8_t protocol;
} USB_SETUP_SET_PROTOCOL;

// *****************************************************************************
// *****************************************************************************
// Section: Variables
// *****************************************************************************
// *****************************************************************************
static uint8_t idle_rate;
static uint8_t active_protocol;   // [0] Boot Protocol [1] Report Protocol

extern const struct{uint8_t report[HID_RPT01_SIZE];}hid_rpt01;

// *****************************************************************************
// *****************************************************************************
// Section: Prototypes
// *****************************************************************************
// *****************************************************************************
#if defined USER_GET_REPORT_HANDLER
    void USER_GET_REPORT_HANDLER(void);
#endif

#if defined USER_SET_REPORT_HANDLER
    extern void USER_SET_REPORT_HANDLER(void);
#endif

// *****************************************************************************
// *****************************************************************************
// Section: Macros or Functions
// *****************************************************************************
// *****************************************************************************

//To implement a set idle rate callback function in the application,
//Make sure "#define USB_DEVICE_HID_IDLE_RATE_CALLBACK(reportID, newIdleRate)    USBHIDCBSetIdleRateHandler(reportID, newIdleRate)"
//is placed in your usb_config.h file, and then in your application .c file,
//add the void USBHIDCBSetIdleRateHandler(reportID, newIdleRate) function
//implementation that saves the new idle rate and report ID info, so that it
//gets used later when sending subsequent HID input report packets to the host.
#ifndef USB_DEVICE_HID_IDLE_RATE_CALLBACK
    #define USB_DEVICE_HID_IDLE_RATE_CALLBACK(reportId, idleRate)
#else
    extern void USB_DEVICE_HID_IDLE_RATE_CALLBACK(uint8_t reportId, uint8_t idleRate);
#endif

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
void USBCheckHIDRequest(void)
{
    if(SetupPkt.Recipient != USB_SETUP_RECIPIENT_INTERFACE_BITFIELD) return;
    if(SetupPkt.bIntfID != HID_INTF_ID) return;

    /*
     * There are two standard requests that hid.c may support.
     * 1. GET_DSC(DSC_HID,DSC_RPT,DSC_PHY);
     * 2. SET_DSC(DSC_HID,DSC_RPT,DSC_PHY);
     */
    if(SetupPkt.bRequest == USB_REQUEST_GET_DESCRIPTOR)
    {
        switch(SetupPkt.bDescriptorType)
        {
            case DSC_HID: //HID Descriptor
                if(USBActiveConfiguration == 1)
                {
                    USBEP0SendROMPtr(
                        (const uint8_t*)&configDescriptor1 + 18,		//18 is a magic number.  It is the offset from start of the configuration descriptor to the start of the HID descriptor.
                        sizeof(USB_HID_DSC)+3,
                        USB_EP0_INCLUDE_ZERO);
                }
                break;
            case DSC_RPT:  //Report Descriptor
                //if(USBActiveConfiguration == 1)
                {
                    USBEP0SendROMPtr(
                        (const uint8_t*)&hid_rpt01,
                        HID_RPT01_SIZE,     //See usbcfg.h
                        USB_EP0_INCLUDE_ZERO);
                }
                break;
            case DSC_PHY:  //Physical Descriptor
				//Note: The below placeholder code is commented out.  HID Physical Descriptors are optional and are not used
				//in many types of HID applications.  If an application does not have a physical descriptor,
				//then the device should return STALL in response to this request (stack will do this automatically
				//if no-one claims ownership of the control transfer).
				//If an application does implement a physical descriptor, then make sure to declare
				//hid_phy01 (rom structure containing the descriptor data), and hid_phy01 (the size of the descriptors in uint8_ts),
				//and then uncomment the below code.
                //if(USBActiveConfiguration == 1)
                //{
                //    USBEP0SendROMPtr((const uint8_t*)&hid_phy01, sizeof(hid_phy01), USB_EP0_INCLUDE_ZERO);
                //}
                break;
        }//end switch(SetupPkt.bDescriptorType)
    }//end if(SetupPkt.bRequest == GET_DSC)

    if(SetupPkt.RequestType != USB_SETUP_TYPE_CLASS_BITFIELD)
    {
        return;
    }

    switch(SetupPkt.bRequest)
    {
        case GET_REPORT:
            #if defined USER_GET_REPORT_HANDLER
                USER_GET_REPORT_HANDLER();
            #endif
            break;
        case SET_REPORT:
            #if defined USER_SET_REPORT_HANDLER
                USER_SET_REPORT_HANDLER();
            #endif
            break;
        case GET_IDLE:
            USBEP0SendRAMPtr(
                (uint8_t*)&idle_rate,
                1,
                USB_EP0_INCLUDE_ZERO);
            break;
        case SET_IDLE:
            USBEP0Transmit(USB_EP0_NO_DATA);
            idle_rate = SetupPkt.W_Value.byte.HB;
            USB_DEVICE_HID_IDLE_RATE_CALLBACK(SetupPkt.W_Value.byte.LB, idle_rate);
            break;
        case GET_PROTOCOL:
            USBEP0SendRAMPtr(
                (uint8_t*)&active_protocol,
                1,
                USB_EP0_NO_OPTIONS);
            break;
        case SET_PROTOCOL:
            USBEP0Transmit(USB_EP0_NO_DATA);
            active_protocol = SetupPkt.W_Value.byte.LB;
            break;
    }//end switch(SetupPkt.bRequest)

}//end USBCheckHIDRequest

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
 // Implemented as a macro. See usb_function_hid.h

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
  // Implemented as a macro. See usb_function_hid.h

/*******************************************************************************
 End of File
*/
