using TMPro;
using UnityEngine;
using UnityEngine.UI;

public class RouteShortDescPrefabBehaviour : MonoBehaviour
{
    [SerializeField] private GameObject linesContent;
    [SerializeField] private TMP_Text departureHour;
    [SerializeField] private TMP_Text arrivalHour;
    [SerializeField] private Button button;

    [SerializeField] private GameObject linePrefab;

    private string MinutesToTime(int minutesFromMidnight)
    {
        int hours = minutesFromMidnight / 60;
        int minutes = minutesFromMidnight % 60;

        return $"{hours:D2}:{minutes:D2}";
    }

    public void UpdateDisplay(JourneyResult result)
    {
        departureHour.text = MinutesToTime(result.FinalArrivalTime);
        arrivalHour.text = "9:00"; //xd


        foreach(JourneyStep step in result.Steps)
        {
            GameObject newlinepref = Instantiate(linePrefab, linesContent.transform);
            newlinepref.transform.Find("LineNumber").GetComponent<TMP_Text>().text = step.RouteName;
        }

        button.onClick.RemoveAllListeners();
        button.onClick.AddListener(() => {
            GameObject.FindFirstObjectByType<RouteDescParentBehaviour>(FindObjectsInactive.Include).UpdateDisplay(result);
        });
    }
}
