# USB-Notifier
Monitor USB connectivity when new device attached or removed from the system.

![alt text](https://raw.githubusercontent.com/proxytype/usbnotifier/main/usbnotifier.gif)

## Win32 Objects And Structures
[- SP_DEVINFO_DATA](https://docs.microsoft.com/en-us/windows/win32/api/setupapi/ns-setupapi-sp_devinfo_data)<br>
[- HDEVINFO](https://docs.microsoft.com/en-us/windows-hardware/drivers/install/device-information-sets)<br>
[- DEVPROPTYPE](https://docs.microsoft.com/en-us/previous-versions/ff543546(v=vs.85))<br>
[- DEV_BROADCAST_DEVICEINTERFACE](https://docs.microsoft.com/en-us/windows/win32/api/dbt/ns-dbt-dev_broadcast_deviceinterface_a)<br>
[- WNDCLASSEX](https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-wndclassexa)<br>

## Win32 Functions
 - SetupDiGetClassDevs
 - SetupDiEnumDeviceInfo
 - SetupDiGetDeviceRegistryPropertyA
 - RegisterDeviceNotification
 - RegisterClassEx
 - RegisterDeviceNotification
 - CreateWindowEx

TODO:
- add support to other classes guid.

