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

    public void UpdateDisplay(JourneyResult result)
    {
        departureHour.text = result.FinalArrivalTime.ToString();
        arrivalHour.text = "9:00"; //xd


        foreach(JourneyStep step in result.Steps)
        {
            GameObject newlinepref = Instantiate(linePrefab, linesContent.transform);
            newlinepref.transform.Find("LineNumber").GetComponent<TMP_Text>().text = step.RouteName;
        }
    }
}
