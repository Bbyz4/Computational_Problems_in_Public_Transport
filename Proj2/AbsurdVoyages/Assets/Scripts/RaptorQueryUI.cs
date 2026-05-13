using System;
using UnityEngine;
using UnityEngine.UI;
using TMPro;

public class RouteQueryUI : MonoBehaviour
{
    [SerializeField] private TMP_InputField startStopInput;
    [SerializeField] private TMP_InputField endStopInput;
    [SerializeField] private TMP_InputField timeInput;
    [SerializeField] private TMP_InputField maxRoundsInput;
    [SerializeField] private Button runButton;

    private RaptorRouting router;

    void Awake()
    {
        router = FindFirstObjectByType<RaptorRouting>();

        runButton.onClick.AddListener(OnRunClicked);
    }

    private void OnRunClicked()
    {
        string start = startStopInput.text;
        string end = endStopInput.text;
        string time = timeInput.text;
        int maxRounds = Int32.Parse(maxRoundsInput.text);

        if(string.IsNullOrWhiteSpace(start) || string.IsNullOrWhiteSpace(end) || string.IsNullOrWhiteSpace(time))
        {
            Debug.LogError("Not all fields are filled");
            return;
        }

        JourneyResult result = router.FindRoute(start, end, time, maxRounds);

        if(result == null)
        {
            Debug.Log("No route found.");
            return;
        }

        Debug.Log("===== ROUTE FOUND =====");

        foreach(JourneyStep step in result.Steps)
        {
            Debug.Log(
                $"{step.FromStopName} -> {step.ToStopName}" + 
                $"via {step.RouteName}" + 
                $"arrive {step.ArrivalTime}"
            );

            Debug.Log($"Final arrival: {result.FinalArrivalTime}");
        }
    }
}