#pragma once

#include "DdkType.h"
#include <Cfgmgr32.h>                   //DEVINST, CM_Locate_DevNode() ...etc, need load Cfgmgr32.lib
#include <usbioctl.h>                   //USB_NODE_INFORMATION

#include <vector>
#include "Utility/Utility.h"



using namespace std;

typedef struct {
    eu32 nodeNum;
    estring deviceInstanceId;
    estring deviceDesc;
}CmDevNode;

typedef struct {
    estring objName;
    estring objTypeName;
}ObjDirInfo;


class DdkUtility
{
public:
    DdkUtility();
    ~DdkUtility();
    
    CmDevNode get_one_node_info(DEVINST devInst);

    void get_device_node_info_recursive(DEVINST devInst, vector<CmDevNode>& deviceNode);

    vector<CmDevNode> get_all_device_node_info();
    estring show_device_node_info(vector<CmDevNode>& deviceNode);

    bool load();
    bool get_object(vector<ObjDirInfo>& devices);

    estring get_hub_symlink(estring deviceName);
    HANDLE get_usb_node_info(estring symbolLink, USB_NODE_INFORMATION& nodeInfo);
    estring get_usb_node_driverkey_name(HANDLE handle, eu32 ConnectionIndex);

    USB_NODE_CONNECTION_INFORMATION get_usb_connection_info(HANDLE hHubDevice, eu32 index);

private:
    Utility m_u;

    HMODULE m_ntdll_handle;
    NTQUERYDIRECTORYOBJECT  NtQueryDirectoryObject;
    NTOPENDIRECTORYOBJECT   NtOpenDirectoryObject;
    NTCLOSE                 NtClose;

};

