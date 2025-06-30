using UnityEngine;

public class StartConnection : MonoBehaviour
{

    BluetoothManager bluetoothManager;


    private void Start()
    {
        bluetoothManager = FindObjectOfType<BluetoothManager>();
    }

    public void DeviceBtn()
    {
        bluetoothManager.StartConnection(this.transform.GetChild(1));

    }


}
