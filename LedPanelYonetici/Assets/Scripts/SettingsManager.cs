using System;
using System.Collections;
using System.Collections.Generic;
using TMPro;
using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.UI;

public class SettingsManager : MonoBehaviour
{

    public TextMeshProUGUI text1Speed;
    public TextMeshProUGUI text1RepetitionCount;

    public TextMeshProUGUI text2ScreenTime;
    public TextMeshProUGUI clockScreenTime;

    [SerializeField] Toggle text2Toggle;
    [SerializeField] Toggle clockToggle;

    [SerializeField] Button clockIncreaseBtn;
    [SerializeField] Button clockDecreaseBtn;

    [SerializeField] Canvas settingsCanvas;

    [SerializeField] Image PanelImage;

    public bool colorState = false;

    public string values()
    {
        string clock = "null";
        if (clockToggle.isOn)
            clock = DateTime.Now.ToString("HH:mm");

        string text1Speed;

        int speed = int.Parse(this.text1Speed.text);

        if (speed < 6)
        {
            text1Speed = ((speed * -5) + 55).ToString();                                                    // y = -5x + 55 Yani: speed 1 icin  y = 50 ... speed 5 icin y = 30.
        }
        else
        {
            text1Speed = ((speed * -4) + 50).ToString();                                                    // y = -4x + 50 Yani: speed 6 icin y = 26 .... speed 10 icin y = 10.
        }

        //                                                                                                 // 10:54@30@3@true@5@5  gibi bir dondurucez ve bu metni kayan ve sabit metin ile birlestirip esp'ye gonderecegiz.
        string totalText = clock + "@" + text1Speed + "@" + text1RepetitionCount.text + "@" + text2Toggle.isOn.ToString() + "@" + text2ScreenTime.text + "@" + clockScreenTime.text;
        return totalText;
    }


    void Start()
    {
        SetVariable();

        PanelImage = GetComponent<Image>();
    }


    void SetVariable()
    {
        if (PlayerPrefs.HasKey("Text1Speed"))
            text1Speed.text = PlayerPrefs.GetInt("Text1Speed").ToString();
        else
            PlayerPrefs.SetInt("Text1Speed", int.Parse(text1Speed.text));


        if (PlayerPrefs.HasKey("Text1RepetitionCount"))
            text1RepetitionCount.text = PlayerPrefs.GetInt("Text1RepetitionCount").ToString();
        else
            PlayerPrefs.SetInt("Text1RepetitionCount", int.Parse(text1RepetitionCount.text));


        if (PlayerPrefs.HasKey("Text2ScreenTime"))
            text2ScreenTime.text = PlayerPrefs.GetInt("Text2ScreenTime").ToString();
        else
            PlayerPrefs.SetInt("Text2ScreenTime", int.Parse(text2ScreenTime.text));


        if (PlayerPrefs.HasKey("ClockScreenTime"))
            clockScreenTime.text = PlayerPrefs.GetInt("ClockScreenTime").ToString();
        else
            PlayerPrefs.SetInt("ClockScreenTime", int.Parse(clockScreenTime.text));

        if (PlayerPrefs.HasKey("Text2Toggle"))
        {
            if (PlayerPrefs.GetString("Text2Toggle") == "T")
                text2Toggle.isOn = true;
            else
                text2Toggle.isOn = false;
        }
        else
            PlayerPrefs.SetString("Text2Toggle", text2Toggle.isOn ? "T" : "F");

        if (PlayerPrefs.HasKey("ClockToggle"))
        {
            if (PlayerPrefs.GetString("ClockToggle") == "T")
                clockToggle.isOn = true;
            else
                clockToggle.isOn = false;
        }
        else
            PlayerPrefs.SetString("ClockToggle", clockToggle.isOn ? "T" : "F");

    }

    public void CloseSettingPanel()
    {
        settingsCanvas.sortingOrder = 0;
        colorState = false;

        PlayerPrefs.SetInt("Text1Speed", int.Parse(text1Speed.text));                                                       // Degisiklikleri kaydet
        PlayerPrefs.SetInt("Text1RepetitionCount", int.Parse(text1RepetitionCount.text));
        PlayerPrefs.SetInt("Text2ScreenTime", int.Parse(text2ScreenTime.text));
        PlayerPrefs.SetInt("ClockScreenTime", int.Parse(clockScreenTime.text));
        PlayerPrefs.SetString("Text2Toggle", text2Toggle.isOn ? "T" : "F");
        PlayerPrefs.SetString("ClockToggle", clockToggle.isOn ? "T" : "F");

        PlayerPrefs.Save();
    }

    public void IncreaseValue(TextMeshProUGUI textObject)
    {
        int value = int.Parse(textObject.text);
        if (value < 10)
            textObject.text = (value + 1).ToString();
    }

    public void DecreaseValue(TextMeshProUGUI textObject)
    {
        int value = int.Parse(textObject.text);
        if (value > 1)
            textObject.text = (value - 1).ToString();
    }


    public void ClockIsOn(bool value)
    {
        if (value)
        {
            clockIncreaseBtn.interactable = true;
            clockDecreaseBtn.interactable = true;
        }
        else
        {
            clockIncreaseBtn.interactable = false;
            clockDecreaseBtn.interactable = false;
        }
    }


    public IEnumerator ChangePanelColor()
    {
        float R = 220f, G = 220f, B = 79f;
        float speed = 50f;

        var steps = new List<(char channel, int direction)>                                                 // Her adimda degisecek kanal ve yon
    {
        ('R', -1), ('B', 1), ('G', -1),
        ('R', 1),  ('B', -1), ('G', 1)
    };

        int index = 0;

        while (colorState)
        {
            var (channel, direction) = steps[index];

            switch (channel)
            {
                case 'R':
                    R += direction * Time.deltaTime * speed;
                    R = Mathf.Clamp(R, 79f, 220f);
                    if ((direction < 0 && R <= 79f) || (direction > 0 && R >= 220f)) index++;
                    break;

                case 'G':
                    G += direction * Time.deltaTime * speed;
                    G = Mathf.Clamp(G, 79f, 220f);
                    if ((direction < 0 && G <= 79f) || (direction > 0 && G >= 220f)) index++;
                    break;

                case 'B':
                    B += direction * Time.deltaTime * speed;
                    B = Mathf.Clamp(B, 79f, 220f);
                    if ((direction < 0 && B <= 79f) || (direction > 0 && B >= 220f)) index++;
                    break;
            }


            if (index >= steps.Count) index = 0;                                                          // Don dolas

            PanelImage.color = new Color(R / 255f, G / 255f, B / 255f, 1f);

            yield return null;
        }

    }

}