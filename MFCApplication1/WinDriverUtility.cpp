#include "stdafx.h"
#include "WinDriverUtility.h"

#include "Utility\Utility.h"
#include "Utility\Observer.h"
#include "Utility\EricException.h"


//#include <bcrypt.h>     //NTSTATUS


 
#include <algorithm>     //copy_if
#include <iterator> 


WinDriverUtility::WinDriverUtility() {
}

WinDriverUtility::~WinDriverUtility() {
    release();
}

estring WinDriverUtility::find_device_description(estring driveKeyName) {
    for (eu32 i = 0; i < m_deviceNode.size(); i++) {
        estring drvStr = m_deviceNode[i].deviceInstanceId;
        if (drvStr == driveKeyName) {
            return m_deviceNode[i].deviceDesc;
        }
    }
    return _ET("");
}

void WinDriverUtility::enumerate_hub_ports(HANDLE hHubDevice, UCHAR NumPorts, ULONG level, USHORT hubAddress, void* port_ctx) {
    SEND_MSG(_ET("roothub, hHubDevice=%X, total port num=%d"), hHubDevice, NumPorts);
    for (eu32 index = 1; index <= NumPorts; index++) {

        USB_NODE_CONNECTION_INFORMATION connectionInfo = m_ddku.get_usb_connection_info(hHubDevice, index);

        // If there is a device connected, get the Device Description
        // hbu 本身有沒有接上裝置?
        if (connectionInfo.ConnectionStatus != NoDeviceConnected) {
            
            estring driveKeyName = m_ddku.get_usb_node_driverkey_name(hHubDevice, index);
            estring deviceDesc = find_device_description(driveKeyName);

            SEND_MSG(_ET("DeviceConnected, driverKeyName, port=%d, name = %s, deviceDesc=%s"), index, driveKeyName.c_str(), deviceDesc.c_str());

            if (connectionInfo.ConnectionStatus == DeviceConnected) {
                SEND_MSG(_ET("Device DeviceConnected"));
            }
            if (connectionInfo.DeviceIsHub) {
                SEND_MSG(_ET("DeviceIsHub"));
            }
        }
    }
}

void WinDriverUtility::release() {
}

void WinDriverUtility::run() {
    if (m_ddku.load() == false) {
        SEND_MSG(_ET("load dll fail"));
        return;
    }

    m_ddku.get_object(m_ObjDirInfos);
    for (auto i : m_ObjDirInfos) {
        SEND_MSG(i.objName.c_str());
    }
    
    estring findStr = _ET("USBPcap");
    auto filterLambda = [findStr](ObjDirInfo& obj) {
        return obj.objName.find(findStr) == std::string::npos;
    };

    m_ObjDirInfos.erase(std::remove_if(m_ObjDirInfos.begin(), m_ObjDirInfos.end(), filterLambda), m_ObjDirInfos.end());

    m_deviceNode = m_ddku.get_all_device_node_info();
    estring msg = m_ddku.show_device_node_info(m_deviceNode);
    SEND_MSG(msg.c_str());

    for (auto item : m_ObjDirInfos) {
   
        //estring objName = ;
        estring symbol = m_ddku.get_hub_symlink(_ET("\\\\.\\") + item.objName);

        SEND_MSG(symbol.c_str());
        m_linkSymbol.push_back(symbol);

        USB_NODE_INFORMATION nodeInfo;
        HANDLE handle = m_ddku.get_usb_node_info(symbol, nodeInfo);

        enumerate_hub_ports(handle, nodeInfo.u.HubInformation.HubDescriptor.bNumberOfPorts, 0, 0, NULL);
    }

    release();
}

