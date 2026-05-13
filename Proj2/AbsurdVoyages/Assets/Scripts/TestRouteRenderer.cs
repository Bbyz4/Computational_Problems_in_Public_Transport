using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using TMPro;
using UnityEngine;

public class TestRouteRenderer : MonoBehaviour
{
    [Header("References")]
    [SerializeField] private GameObject busStopPrefab;

    [Header("Map Settings")]
    [SerializeField] private float coordinateScale = 1000f;

    // Optional manual origin
    [SerializeField] private float originLatitude = 50.0647f;
    [SerializeField] private float originLongitude = 19.9450f;

    private readonly List<GameObject> spawnedStops = new();

    private void Start()
    {
        LoadStops();
    }

    private void LoadStops()
    {
        // Put stops.txt inside:
        // Assets/Resources/stops.txt
        TextAsset stopsFile = Resources.Load<TextAsset>("stops");

        if (stopsFile == null)
        {
            Debug.LogError("Could not find stops.txt inside Resources folder.");
            return;
        }

        string[] lines = stopsFile.text.Split('\n');

        if (lines.Length <= 1)
        {
            Debug.LogError("stops.txt is empty.");
            return;
        }

        // Read header indexes
        string[] headers = SplitCsvLine(lines[0]);

        int stopNameIndex = Array.IndexOf(headers, "stop_name");
        int stopLatIndex = Array.IndexOf(headers, "stop_lat");
        int stopLonIndex = Array.IndexOf(headers, "stop_lon");

        if (stopNameIndex == -1 ||
            stopLatIndex == -1 ||
            stopLonIndex == -1)
        {
            Debug.LogError("Required GTFS columns not found.");
            return;
        }

        // Parse all stops
        for (int i = 1; i < lines.Length; i++)
        {
            if (string.IsNullOrWhiteSpace(lines[i]))
                continue;

            string[] columns = SplitCsvLine(lines[i]);

            if (columns.Length <= Mathf.Max(stopNameIndex, stopLatIndex, stopLonIndex))
                continue;

            string stopName = columns[stopNameIndex];

            if (!float.TryParse(columns[stopLatIndex], NumberStyles.Float, CultureInfo.InvariantCulture, out float lat))
                continue;

            if (!float.TryParse(columns[stopLonIndex], NumberStyles.Float, CultureInfo.InvariantCulture, out float lon))
                continue;

            SpawnStop(stopName, lat, lon);
        }

        Debug.Log($"Spawned {spawnedStops.Count} stops.");
    }

    private void SpawnStop(string stopName, float latitude, float longitude)
    {
        Vector3 worldPosition = GeoToWorldPosition(latitude, longitude);

        GameObject stopObject = Instantiate(
            busStopPrefab,
            worldPosition,
            Quaternion.identity,
            transform
        );

        stopObject.name = stopName;

        // Find child label
        Transform labelTransform = stopObject.transform.Find("Label");

        if (labelTransform != null)
        {
            TMP_Text text = labelTransform.GetComponent<TMP_Text>();

            if (text != null)
            {
                text.text = stopName;
            }
        }

        spawnedStops.Add(stopObject);
    }

    private Vector3 GeoToWorldPosition(float latitude, float longitude)
    {
        // Very simple projection suitable for city-scale maps

        float x = (longitude - originLongitude) * coordinateScale;
        float y = (latitude - originLatitude) * coordinateScale;

        return new Vector3(x, y, 0f);
    }

    private string[] SplitCsvLine(string line)
    {
        // Basic GTFS-safe CSV parser
        List<string> values = new();

        bool insideQuotes = false;
        string currentValue = "";

        foreach (char c in line)
        {
            if (c == '"')
            {
                insideQuotes = !insideQuotes;
            }
            else if (c == ',' && !insideQuotes)
            {
                values.Add(currentValue);
                currentValue = "";
            }
            else
            {
                currentValue += c;
            }
        }

        values.Add(currentValue);

        return values.ToArray();
    }
}