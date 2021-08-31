#pragma once

#include "Utility\Utility.h"
#include <vector>
#include "Ddk/DdkUtility.h"
#include <Cfgmgr32.h>   //DEVINST, CM_Locate_DevNode()�A���[�� Cfgmgr32.lib�A�_�h linker �|����

using namespace std;



class WinDriverUtility
{
public:
    WinDriverUtility();
    ~WinDriverUtility();

    void WinDriverUtility::run();
    void WinDriverUtility::release();
    void WinDriverUtility::enumerate_hub_ports(HANDLE hHubDevice, UCHAR NumPorts, ULONG level, USHORT hubAddress, void* port_ctx);

private:
    estring WinDriverUtility::find_device_description(estring driveKeyName);

    Utility m_u;

    vector<ObjDirInfo> m_ObjDirInfos;

    vector<estring> m_linkSymbol;

    vector<CmDevNode> m_deviceNode;

    DdkUtility m_ddku;

};

