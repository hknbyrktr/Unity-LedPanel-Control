using UnityEngine.Android;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class BluetoothManager : MonoBehaviour
{
    AppManager appManager;

    public GameObject devicesListContainer;
    public Transform device;
    private bool isConnected;

    private static AndroidJavaClass unity3dbluetoothplugin;
    private static AndroidJavaObject BluetoothConnector;

    Text receivedData;                                                                                         // Bizim projemizde veri alinmayacak. Hata vermemesi icin silmedim.    

    //public TextMeshProUGUI dataToSend;                                                                       // SAKIIIN BUNU YAPMA !!! - Input' u TMP ile aldigimda arka planda kendisi sacma bir karakter ekliyor -
    //public InputField dataToSend;                                                                              // ve bu karakteri "â▯▯" seklinde senin yazinin devamina ekliyor. Yarim gunumu yedi!!

    void Start()
    {
        appManager = GetComponent<AppManager>();

        InitBluetooth();
        isConnected = false;
    }

    // creating an instance of the bluetooth class from the plugin 
    public void InitBluetooth()
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        // Check BT and location permissions
        if (!Permission.HasUserAuthorizedPermission(Permission.CoarseLocation)
            || !Permission.HasUserAuthorizedPermission(Permission.FineLocation)
            || !Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_ADMIN")
            || !Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH")
            || !Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_SCAN")
            || !Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_ADVERTISE")
            || !Permission.HasUserAuthorizedPermission("android.permission.BLUETOOTH_CONNECT"))
        {

            Permission.RequestUserPermissions(new string[] {
                        Permission.CoarseLocation,
                            Permission.FineLocation,
                            "android.permission.BLUETOOTH_ADMIN",
                            "android.permission.BLUETOOTH",
                            "android.permission.BLUETOOTH_SCAN",
                            "android.permission.BLUETOOTH_ADVERTISE",
                             "android.permission.BLUETOOTH_CONNECT"
                    });

        }

        unity3dbluetoothplugin = new AndroidJavaClass("com.example.unity3dbluetoothplugin.BluetoothConnector");
        BluetoothConnector = unity3dbluetoothplugin.CallStatic<AndroidJavaObject>("getInstance");
    }

    // Start device scan
    public void StartScanDevices()
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        // Destroy devicesListContainer child objects for new scan display
        foreach (Transform child in devicesListContainer.transform)
        {
            Destroy(child.gameObject);
        }

        BluetoothConnector.CallStatic("StartScanDevices");
    }

    // Stop device scan
    public void StopScanDevices()
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        BluetoothConnector.CallStatic("StopScanDevices");
    }


    // This function will be called by Java class to update the scan status,
    // DO NOT CHANGE ITS NAME OR IT WILL NOT BE FOUND BY THE JAVA CLASS
    public void ScanStatus(string status)
    {
        Toast("Scan Status: " + status);
    }



    // This function will be called by Java class whenever a new device is found,
    // and delivers the new devices as a string data="MAC+NAME"
    // DO NOT CHANGE ITS NAME OR IT WILL NOT BE FOUND BY THE JAVA CLASS
    public void NewDeviceFound(string data)
    {
        string[] dt = data.Split("+");

        Transform newDevice = device;

        if (dt[1] != "null")
            newDevice.GetChild(0).GetComponent<TextMeshProUGUI>().text = dt[1];                                             // Cihazin adi.
        else
            newDevice.GetChild(0).GetComponent<TextMeshProUGUI>().text = "Bilinmeyen Cihaz";

        newDevice.GetChild(1).GetComponent<TextMeshProUGUI>().text = dt[0];                                                 // Cihazin MAC'i.

        Instantiate(newDevice, devicesListContainer.transform);
    }


    // Get paired devices from BT settings
    public void GetPairedDevices()
    {

        if (Application.platform != RuntimePlatform.Android)
            return;

        // This function when called returns an array of PairedDevices as "MAC+Name" for each device found
        string[] data = BluetoothConnector.CallStatic<string[]>("GetPairedDevices"); ;

        // Destroy devicesListContainer child objects for new Paired Devices display
        foreach (Transform child in devicesListContainer.transform)
        {
            Destroy(child.gameObject);
        }

        // Display the paired devices
        foreach (var d in data)
        {
            string[] dt = d.Split("+");

            Transform newDevice = device;

            if (dt[1] != "null")
                newDevice.GetChild(0).GetComponent<TextMeshProUGUI>().text = dt[1];                                             // Cihazin adi.
            else
                newDevice.GetChild(0).GetComponent<TextMeshProUGUI>().text = "Bilinmeyen Cihaz";

            newDevice.GetChild(1).GetComponent<TextMeshProUGUI>().text = dt[0];                                                 // Cihazin MAC'i.

            Instantiate(newDevice, devicesListContainer.transform);
        }
    }

    // Start BT connect using device MAC address "deviceAdd"
    public void StartConnection(Transform selectedMAC)
    {

        if (Application.platform != RuntimePlatform.Android)
            return;

        BluetoothConnector.CallStatic("StartConnection", selectedMAC.GetComponent<TextMeshProUGUI>().text.ToString().ToUpper());

    }


    // Stop BT connetion
    public void StopConnection()
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        if (isConnected)
            BluetoothConnector.CallStatic("StopConnection");
    }

    // This function will be called by Java class to update BT connection status,
    // DO NOT CHANGE ITS NAME OR IT WILL NOT BE FOUND BY THE JAVA CLASS
    public void ConnectionStatus(string status)
    {
        Toast("Connection Status: " + status);
        isConnected = status == "connected";


        StartCoroutine(appManager.BluetoothStatus(status));
    }

    // This function will be called by Java class whenever BT data is received,
    // DO NOT CHANGE ITS NAME OR IT WILL NOT BE FOUND BY THE JAVA CLASS
    public void ReadData(string data)
    {
        Debug.Log("BT Stream: " + data);
        receivedData.text = data;
    }

    // Write data to the connected BT device
    public void WriteData(string text)
    {
        Debug.Log($"Bluetooth'dan gonderdigin veri: {text}");

        if (Application.platform != RuntimePlatform.Android)
            return;

        if (isConnected)
            BluetoothConnector.CallStatic("WriteData", text);

    }

    // This function will be called by Java class to send Log messages,
    // DO NOT CHANGE ITS NAME OR IT WILL NOT BE FOUND BY THE JAVA CLASS
    public void ReadLog(string data)
    {
        Debug.Log(data);
    }


    // Function to display an Android Toast message
    public void Toast(string data)
    {
        if (Application.platform != RuntimePlatform.Android)
            return;

        BluetoothConnector.CallStatic("Toast", data);
    }
}
