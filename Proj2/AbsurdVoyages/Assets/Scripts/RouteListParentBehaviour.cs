using System.Collections.Generic;
using UnityEngine;

public class RouteListParentBehaviour : MonoBehaviour
{
    [SerializeField] private GameObject contentParent;
    [SerializeField] private GameObject routeShortDescPrefab;

    void Awake()
    {
        
    }

    public void PlotResults(List<JourneyResult> results)
    {
        gameObject.SetActive(true);
        
        foreach(JourneyResult result in results)
        {
            GameObject newPref = Instantiate(routeShortDescPrefab, contentParent.transform);
        
            newPref.GetComponent<RouteShortDescPrefabBehaviour>().UpdateDisplay(result);
        }
    }
}
