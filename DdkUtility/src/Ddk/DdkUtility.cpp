#include "../../framework.h"
#include "DdkUtility.h"

#include <WinIoCtl.h>                      //CTL_CODE, FILE_DEVICE_UNKNOWN


#include "DefineFiles/DefineFile.h"
#include "Utility/Observer.h"
#include "Utility/EricException.h"


#define BUFFER_SIZE                        (0x1000)
#define DIRECTORY_QUERY                    (0x0001)

#define IOCTL_OUTPUT_BUFFER_SIZE           (1024)


#define IOCTL_USBPCAP_GET_HUB_SYMLINK \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)


using namespace std;

DdkUtility::DdkUtility() 
{
    m_ntdll_handle = NULL;
    NtQueryDirectoryObject = NULL;
    NtOpenDirectoryObject = NULL;
    NtClose = NULL;
}

DdkUtility::~DdkUtility() {
    if (m_ntdll_handle != NULL) {
        FreeLibrary(m_ntdll_handle);
        m_ntdll_handle = NULL;
    }
}

bool DdkUtility::load() {
    m_ntdll_handle = LoadLibrary(_ET("ntdll.dll"));
    if (m_ntdll_handle == NULL) {
        return FALSE;
    }

    NtQueryDirectoryObject =
        (NTQUERYDIRECTORYOBJECT)GetProcAddress(m_ntdll_handle,
            "NtQueryDirectoryObject");

    if (NtQueryDirectoryObject == NULL) {
        return FALSE;
    }

    NtOpenDirectoryObject =
        (NTOPENDIRECTORYOBJECT)GetProcAddress(m_ntdll_handle,
            "NtOpenDirectoryObject");
    if (NtOpenDirectoryObject == NULL) {
        return FALSE;
    }

    NtClose = (NTCLOSE)GetProcAddress(m_ntdll_handle, "NtClose");
    if (NtClose == NULL) {
        return FALSE;
    }
    return true;
}


CmDevNode DdkUtility::get_one_node_info(DEVINST devInst) {
    //show myself
    //�� MAX_DEVICE_ID_LEN, �o�˴N����Ū�⦸�F
    TCHAR      buf[MAX_DEVICE_ID_LEN] = { 0 };
    TCHAR      drivebuf[MAX_DEVICE_ID_LEN] = { 0 };
    ULONG      len;
    // Get the DriverName value
    len = sizeof(buf) / sizeof(buf[0]);
    CONFIGRET cr = CM_Get_DevNode_Registry_Property(devInst,
        CM_DRP_DRIVER,
        NULL,
        drivebuf,
        &len,
        0);

    estring driveName(drivebuf);

    CmDevNode cmDevNode;
    cmDevNode.nodeNum = devInst;
    cmDevNode.deviceInstanceId = driveName;

    if (cr == CR_SUCCESS) {
        cr = CM_Get_DevNode_Registry_Property(devInst,
            CM_DRP_DEVICEDESC,
            NULL,
            buf,
            &len,
            0);
        if (cr == CR_SUCCESS) {
            estring msg(buf);
            cmDevNode.deviceDesc = msg;
        }

    } else if (cr == CR_NO_SUCH_VALUE) {
        SEND_MSG(_ET("CM_Get_DevNode_Registry_Property, CR_NO_SUCH_VALUE fail, devInst=%X, cr=%X"), devInst, cr);
    } else {
        SEND_MSG(_ET("CM_Get_DevNode_Registry_Property fail, devInst=%X, cr=%X"), devInst, cr);
    }

    return cmDevNode;
}

void DdkUtility::get_device_node_info_recursive(DEVINST devInst, vector<CmDevNode>& deviceNode) {
    DEVINST devInstNext;
    CONFIGRET cr = CM_Get_Child(&devInstNext, devInst, 0);

    CmDevNode node = get_one_node_info(devInst);
    deviceNode.push_back(node);

    if (cr == CR_SUCCESS) {
        get_device_node_info_recursive(devInstNext, deviceNode);
    } else if (cr == CR_NO_SUCH_DEVNODE) {

    } else {
        SEND_MSG(_ET("get_node_info_recursive fail, node=%X, cr=%X"), devInst, cr);
    }

    //have brother
    cr = CM_Get_Sibling(&devInstNext, devInst, 0);
    if (cr == CR_SUCCESS) {
        get_device_node_info_recursive(devInstNext, deviceNode);
    }
}

vector<CmDevNode> DdkUtility::get_all_device_node_info() {
    DEVINST    devInst;
    // Get Root DevNode number
    CONFIGRET cr = CM_Locate_DevNode(&devInst, NULL, 0);
    if (cr != CR_SUCCESS) {
        THROW_MYEXCEPTION(0, _ET("Can get root devNode"));
    }

    vector<CmDevNode> deviceNode;
    CmDevNode root;
    root.nodeNum = devInst;
    deviceNode.push_back(root);

    DEVINST devInstNext;
    cr = CM_Get_Child(&devInstNext, devInst, 0);
    //move to next
    get_device_node_info_recursive(devInstNext, deviceNode);

    return deviceNode;
}

estring DdkUtility::show_device_node_info(vector<CmDevNode>& deviceNode) {
    estring msg;
    for (auto item : deviceNode) {
        msg += m_u.strFormat(_ET("Node(%d), desc=%s, DevInsId=%s"), item.nodeNum, item.deviceDesc.c_str(), item.deviceInstanceId.c_str());
        msg += m_u.crLf();
    }
    return msg;
}

bool DdkUtility::get_object(vector<ObjDirInfo>& devices) {
    UNICODE_STRING str;
    PWSTR path = (PWSTR)(_ET("\\Device"));
    str.Length = (USHORT)(wcslen(path) * 2);
    str.MaximumLength = (USHORT)(wcslen(path) * 2 + 2);
    str.Buffer = path;

    OBJECT_ATTRIBUTES attr;
    attr.Length = sizeof(OBJECT_ATTRIBUTES);
    attr.ObjectName = &str;
    attr.Attributes = 0;
    attr.RootDirectory = NULL;
    attr.SecurityDescriptor = NULL;
    attr.SecurityQualityOfService = NULL;

    POBJDIR_INFORMATION info = (POBJDIR_INFORMATION)HeapAlloc(GetProcessHeap(), 0, BUFFER_SIZE);

    if (info == NULL) {
        SEND_MSG(_ET("HeapAlloc() failed"));
        return FALSE;
    }

    //get directory Handle
    HANDLE handle;
    NTSTATUS status = NtOpenDirectoryObject(&handle, DIRECTORY_QUERY, &attr);
    if (status != 0) {
        SEND_MSG(_ET("NtOpenDirectoryObject() failed"));

        HeapFree(GetProcessHeap(), 0, info);
        return FALSE;
    }

    DWORD index = 0;
    DWORD written = 0;
    while (NtQueryDirectoryObject(handle, info, BUFFER_SIZE, TRUE, FALSE, &index, &written) == 0) {
        ObjDirInfo obj;
        obj.objName = estring(info->ObjectName.Buffer);
        obj.objTypeName = estring(info->ObjectTypeName.Buffer);
        devices.push_back(obj);
    }

    NtClose(handle);
    HeapFree(GetProcessHeap(), 0, info);
    return TRUE;
}

estring DdkUtility::get_hub_symlink(estring deviceName) {
    WCHAR buffer[IOCTL_OUTPUT_BUFFER_SIZE];
    HANDLE filter_handle;
    DWORD  bytes_ret = 0;

    filter_handle = CreateFile(deviceName.c_str(),
        0,
        0,
        0,
        OPEN_EXISTING,
        0,
        0);

    if (filter_handle == INVALID_HANDLE_VALUE) {
        THROW_MYEXCEPTION(0, _ET("CCouldn't open device err=%d"), GetLastError());
        return _ET("");
    }

    if (!DeviceIoControl(filter_handle,
        IOCTL_USBPCAP_GET_HUB_SYMLINK,
        NULL,
        0,
        buffer,
        IOCTL_OUTPUT_BUFFER_SIZE,
        &bytes_ret,
        0)) {
        bytes_ret = 0;
    }

    CloseHandle(filter_handle);

    estring res(buffer);
    return res;
}

HANDLE DdkUtility::get_usb_node_info(estring symbolLink, USB_NODE_INFORMATION& nodeInfo) {
    PTSTR                   deviceName;
    size_t                  deviceNameSize;
    deviceNameSize = symbolLink.size() + _tcslen(_T("\\\\.\\")) + 1;
    deviceName = (PTSTR)GlobalAlloc(GPTR, deviceNameSize * sizeof(TCHAR));


    // Try to hub the open device
    // �ǥѸ˸m�� descriptor ���X�Ӹ˸m�� handle
    HANDLE hHubDevice = CreateFile(symbolLink.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (hHubDevice == INVALID_HANDLE_VALUE) {
        SEND_MSG(_ET("open hHubDevice fail"));
        return NULL;
    }

    ULONG nBytes;

    //���X�� handle �ҹ����� hub info �X��
    BOOL success = DeviceIoControl(hHubDevice,
        IOCTL_USB_GET_NODE_INFORMATION,
        &nodeInfo,
        sizeof(USB_NODE_INFORMATION),
        &nodeInfo,
        sizeof(USB_NODE_INFORMATION),
        &nBytes,
        NULL
    );
    return hHubDevice;
}

estring DdkUtility::get_usb_node_driverkey_name(HANDLE handle, eu32 ConnectionIndex) {
    USB_NODE_CONNECTION_DRIVERKEY_NAME  driverKeyName = { 0 };
    ULONG                               nBytes;

    //�p�G��e��s�u���˸m���t�~�@��hub�A����i�H�q�L��devioceiocontrol�ǻ��޼�ioctl_usb_get_node_connection_name �ӱo�즹hub���W�r
    //driverKeyName ���u�O������ơA�L�]�t�d�ǻ���ơA�Ҧp�o��@�w�n���w index �~�i�H���QŪ���ơA���ޭȭn�q1�}�l
    driverKeyName.ConnectionIndex = ConnectionIndex;

    // �]�������D DriverKeyName ���r����סA�ҥH�Ĥ@�� deviceioctrl ���ت��ܦ������o��� struct+name string �����צ^�� (Get the length of the name of the driver key of the device attached)
    //�A�i�H�[�� USB_NODE_CONNECTION_DRIVERKEY_NAME �����c�A�̧��ݪ��O�@�� WCHAR DriverKeyName[1], ���׬�1
    //�άO�i�H�����ŧi�@�Ӥj�� wcahr buffer��A�A�ھڪ��װ��S�w�^��
    BOOL success = DeviceIoControl(handle,
        IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
        &driverKeyName,
        sizeof(driverKeyName),
        &driverKeyName,
        sizeof(driverKeyName),
        &nBytes,
        NULL);

    if (!success) {
        SEND_MSG(_ET("GetDriverKeyName fail, idx=%X"), ConnectionIndex);
        return _ET("");
    }

    nBytes = driverKeyName.ActualLength;

    //�o���ĥ� malloc����]�O�]���A���c�̫�@�� name �����פ��T�w�A�ҥH�ݭn�ϥ� malloc ���覡
    PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyNameW;
    driverKeyNameW = (PUSB_NODE_CONNECTION_DRIVERKEY_NAME)GlobalAlloc(GPTR, nBytes);

    driverKeyNameW->ConnectionIndex = ConnectionIndex;

    //�������w������
    success = DeviceIoControl(handle,
        IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
        driverKeyNameW,
        nBytes,
        driverKeyNameW,
        nBytes,
        &nBytes,
        NULL);

    if (!success) {
        SEND_MSG(_ET("GetDriverKeyName-2 fail"));
    }

    estring kn(driverKeyNameW->DriverKeyName);

    // All done, free the uncoverted driver key name and return the
    // converted driver key name
    GlobalFree(driverKeyNameW);

    return kn;
}



USB_NODE_CONNECTION_INFORMATION DdkUtility::get_usb_connection_info(HANDLE hHubDevice, eu32 index) {

    USB_NODE_CONNECTION_INFORMATION connectionInfo = { 0 };

    //nBytes �o�ӭȦb�I�s DeviceIoControl ��|���ܡA�ܦ� 35
    ULONG nBytes = 100;

    //�����index�A�o���ӴN�O�N�� hub �� index
    connectionInfo.ConnectionIndex = index;

    //�s���W���F�誺�ܡAinfo �~�|���ȡA�p�G�O�Ū�hub ���|����
    //�� info �̪�  USB_DEVICE_DESCRIPTOR DeviceDescriptor �¦������� ch9 �� Standard Device Descriptor�A�]�N�O�̤@�}�l������ get descriptor
    BOOL success = DeviceIoControl(hHubDevice,
        IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
        &connectionInfo,
        sizeof(USB_NODE_CONNECTION_INFORMATION),
        &connectionInfo,
        sizeof(USB_NODE_CONNECTION_INFORMATION),
        &nBytes,
        NULL);

    if (!success) {
        SEND_MSG(_ET("enumerate_hub_ports fail"));
    }
    return connectionInfo;
}
