using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class RouteDescParentBehaviour : MonoBehaviour
{
    [SerializeField] private GameObject stepPrefab;
    [SerializeField] private GameObject stepsContent;

    void Awake()
    {
        gameObject.SetActive(false);
    }

    private string MinutesToTime(int minutesFromMidnight)
    {
        int hours = minutesFromMidnight / 60;
        int minutes = minutesFromMidnight % 60;

        return $"{hours:D2}:{minutes:D2}";
    }

    public void UpdateDisplay(JourneyResult result)
    {
        foreach (Transform child in stepsContent.transform)
        {
            Destroy(child.gameObject);
        }

        gameObject.SetActive(true);

        foreach(JourneyStep step in result.Steps)
        {
            GameObject newlinepref = Instantiate(stepPrefab, stepsContent.transform);

            newlinepref.transform.Find("LineNumber").GetComponent<TMP_Text>().text = step.RouteName;
            newlinepref.transform.Find("DepartureStopName").GetComponent<TMP_Text>().text = step.FromStopName;
            newlinepref.transform.Find("ArrivalStopName").GetComponent<TMP_Text>().text = step.ToStopName;
            newlinepref.transform.Find("DepartureTime").GetComponent<TMP_Text>().text = MinutesToTime(step.ArrivalTime - 5);
            newlinepref.transform.Find("ArrivalTime").GetComponent<TMP_Text>().text = MinutesToTime(step.ArrivalTime);
        }
    }
}
