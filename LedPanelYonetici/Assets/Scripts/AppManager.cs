using DG.Tweening;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class AppManager : MonoBehaviour
{
    BluetoothManager bluetoothManager;
    SettingsManager settingsManager;

    [SerializeField] Canvas settingsCanvas;

    [SerializeField] CanvasGroup fadeScreen;

    [SerializeField] InputField dataToSend;

    [SerializeField] Image bluetoothStatusImg;
    void Start()
    {
        bluetoothManager = GetComponent<BluetoothManager>();

        settingsManager = FindObjectOfType<SettingsManager>();

        StartCoroutine(StartTheSceneCoroutine());

    }


    public void SendBtn()
    {
        string text1 = dataToSend.text;

        if (text1.Split("@").Length < 2)
        {
            text1 = text1 + "@null";
        }

        string fullText = text1 + "@" + settingsManager.values();

        bluetoothManager.WriteData(fullText);
    }


    public void OpenSettingPanel()
    {
        settingsCanvas.sortingOrder = 2;

        settingsManager.colorState = true;
        StartCoroutine(settingsManager.ChangePanelColor());
    }


    public void SahneDegis()
    {
        SceneManager.LoadScene("SampleScene");
    }
    public void SahneDegis_2()
    {
        SceneManager.LoadScene("MainScene1");
    }

    public void TurnOnThePanel()
    {
        string fullText = "Ac@null@" + settingsManager.values();
        bluetoothManager.WriteData(fullText);
    }
    public void TurnOffThePanel()
    {
        string fullText = "Kapat@null@" + settingsManager.values();
        bluetoothManager.WriteData(fullText);
    }


    public IEnumerator BluetoothStatus(string status)               // Harici sekilde BT baglantisi koparsa bunu anlayamayiz(BT baglanti paketini ben yazmadım). Yani tam islevli degil.
    {
        Color startColor = bluetoothStatusImg.color;
        Color targetColor;

        if (status == "connected")
        {
            targetColor = new Color32(0, 130, 1, 255);              // BT bagli ise resmi yesil yapacagiz
        }
        else
        {
            targetColor = new Color32(80, 80, 80, 255);             // Bt bagli degil ise resmi gri yapacagiz
        }

        float duration = 1f;
        float elapsed = 0f;

        while (elapsed < duration)
        {
            elapsed += Time.deltaTime;
            float t = Mathf.Clamp01(elapsed / duration);
            bluetoothStatusImg.color = Color.Lerp(startColor, targetColor, t);
            yield return null;
        }

        bluetoothStatusImg.color = targetColor;                     // Son rengi kesin olarak ayarla (floating point hataları icin)

    }

    IEnumerator StartTheSceneCoroutine()
    {

        fadeScreen.GetComponent<CanvasGroup>().alpha = 1;

        yield return new WaitForSeconds(.3f);

        fadeScreen.DOFade(0, 1f).SetEase(Ease.InFlash);


    }


}
