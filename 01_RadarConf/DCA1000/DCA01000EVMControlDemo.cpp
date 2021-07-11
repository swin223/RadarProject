#include <Windows.h>
#include "rf_api.h"

/* FrogLu Define                                                            */
#define UINT8 uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define uint8 uint8_t
#define DEFAULT_CONFIG_TIMER 30

// Status to check if DCA1000 is connected over Ethernet, will be set in the EventhandlerCallback
uint8 gCaptureCardConnect = 0;

//RF data card capture event handler
void EthernetEventHandlercallback(UINT16 u16CmdCode, UINT16 u16Status)
{
    if (u16CmdCode == 0x0A)
    {
        ////Syste Error cmd code
        //switch (u16Status)
        //{
        //    // handle Case STS_NO_LVDS_DATA to STS_PLAYBACK_OUT_OF_SEQUENCE
        //}
    }
    else
    {
        //Event register status
        switch (u16CmdCode)
        {
            // case RESET_FPGA_CMD_CODE to PLAYBACK_IS_IN_PROGRESS_CODE

        case SYSTEM_CONNECT_CMD_CODE:
            gCaptureCardConnect = 1;
            break;
        }
    }
}

int main()
{

    SINT32 retVal;
    UINT8  waitCount = 0;
    strEthConfigMode ethernetInitConfigurationParam;
    strConfigMode RFDCCardModeConfigurationParam;
    UINT16 PacketDelayData[3];

    printf("=========== DCA1000 Capture/Playback CLI Application =========== \n");

    // Register Ethernet Event Handler
    retVal = StatusDCCard_EventRegister(EthernetEventHandlercallback);
    // TODO: Handle return error value of above API call

    // Disconnet Ethernet connection if Any
    retVal = DisconnectRFDCCard();
    // TODO: Handle return error value of above API call

    // Set Ethernet configuration parameters
    ethernetInitConfigurationParam.au8MacId[0] = 12;
    ethernetInitConfigurationParam.au8MacId[1] = 34;
    ethernetInitConfigurationParam.au8MacId[2] = 56;
    ethernetInitConfigurationParam.au8MacId[3] = 78;
    ethernetInitConfigurationParam.au8MacId[4] = 90;
    ethernetInitConfigurationParam.au8MacId[5] = 12;
    // System IP address
    ethernetInitConfigurationParam.au8SourceIpAddr[0] = 192;
    ethernetInitConfigurationParam.au8SourceIpAddr[1] = 168;
    ethernetInitConfigurationParam.au8SourceIpAddr[2] = 33;
    ethernetInitConfigurationParam.au8SourceIpAddr[3] = 30;
    // FPGA IP Address
    ethernetInitConfigurationParam.au8DestiIpAddr[0] = 192;
    ethernetInitConfigurationParam.au8DestiIpAddr[1] = 168;
    ethernetInitConfigurationParam.au8DestiIpAddr[2] = 33;
    ethernetInitConfigurationParam.au8DestiIpAddr[3] = 180;

    ethernetInitConfigurationParam.u32RecordPortNo = 4098;
    ethernetInitConfigurationParam.u32ConfigPortNo = 4096;
    // Ethernet Init 
    retVal = ConfigureRFDCCard_RecordEthInit(ethernetInitConfigurationParam);
    // TODO: Handle return error value of above API call

    // Try to connect multiple times
    while (1)
    {
        // User needs to set gCaptureCardConnect to '1' in EthernetEventHandlercallback under SYSTEM_CONNECT_CMD_CODE case
        if (gCaptureCardConnect == 0)
        {
            // Connect DCA1000 card over Ethernet
            retVal = ConnectRFDCCard();
            Sleep(2000);// wait for 2 sec
            waitCount++;

            if (waitCount > 10)
            {
                printf("Ethernet Cable is disconnected Please check.....!!!");
                //break;
                return retVal;
            }
        }
        else
        {
            break;
        }
    }

    // Set the parameters, important param is eDataFormatMode and eLvdsMode
    RFDCCardModeConfigurationParam.eLogMode = RAW_MODE;
    // Change to AR1642 in case capturing data with AWR1642 device
    RFDCCardModeConfigurationParam.eLvdsMode = AR1642;
    RFDCCardModeConfigurationParam.eDataXferMode = CAPTURE;
    RFDCCardModeConfigurationParam.eDataCaptureMode = ETH_STREAM;
    // Data Format mode BIT12/BIT14/BIT16
    RFDCCardModeConfigurationParam.eDataFormatMode = BIT16;
    RFDCCardModeConfigurationParam.u8Timer = DEFAULT_CONFIG_TIMER;

    // Configure DCA1000
    retVal = ConfigureRFDCCard_Mode(RFDCCardModeConfigurationParam);
    // TODO: Handle return error value of above API call

    // invoke GetRFCaptureCardViaEthernetErrorType(retVal) API to get the error status/type.

    // Packet delay config
    PacketDelayData[0] = 1472; // User can use this as default value.
    PacketDelayData[1] = 25;  // User can use this as default value.
    PacketDelayData[2] = 0;

    retVal = ConfigureRFDCCard_DataPacketConfig((uint8_t *)&PacketDelayData[0], 6);
    // TODO: Handle return error value of above API call

    // Start the Recording , Provide the full capture file path. It'll trigger DCA1000 to start 
    // capturing raw data and store at given location.
    int8_t path[32] = "F:\\MatlabFiles\\adc_data_DCA.bin";
    retVal = StartRecordData(path, 1);
    // TODO: Handle return error value of above API call

    //Sleep(1650);
    // To stop the capture
    //retVal = StopRecordData();
    // TODO: Handle return error value of above API call


    // Note - EthernetEventHandlercallback function will get the error/status value, which user 
    // needs to take care of while init/configure/capture process.

    return retVal;
}