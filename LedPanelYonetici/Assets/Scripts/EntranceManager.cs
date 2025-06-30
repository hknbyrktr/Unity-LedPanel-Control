using UnityEngine.SceneManagement;
using System.Collections;
using UnityEngine;
using DG.Tweening;

public class EntranceManager : MonoBehaviour
{

    [SerializeField]
    CanvasGroup fadeScreen;

    [SerializeField]
    Animator logoAnimation;

    bool animFinished;

    private void Start()
    {
        StartCoroutine(StartTheSceneCoroutine());

    }

    void Update()
    {


        if (logoAnimation.GetCurrentAnimatorStateInfo(0).IsName("Logo") &&
            logoAnimation.GetCurrentAnimatorStateInfo(0).normalizedTime >= 1f && !animFinished)
        {
            animFinished = true;
            LoadScene();
        }



    }

    void LoadScene()
    {
        fadeScreen.DOFade(1, 1f).SetEase(Ease.InFlash).OnComplete(() =>
            SceneManager.LoadScene("MainScene")
            );
    }



    IEnumerator StartTheSceneCoroutine()
    {

        fadeScreen.GetComponent<CanvasGroup>().alpha = 1;

        yield return new WaitForSeconds(.5f);

        fadeScreen.DOFade(0, 1f).SetEase(Ease.InFlash).OnComplete(() =>
        logoAnimation.Play("Logo")

        );


    }


}
