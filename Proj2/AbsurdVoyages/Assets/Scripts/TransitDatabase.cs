using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using UnityEngine;

public class TransitDatabase : MonoBehaviour
{
    [Header("GTFS Files")]
    [SerializeField] private string stopsFileName = "stops.txt";
    [SerializeField] private string routesFileName = "routes.txt";
    [SerializeField] private string stopTimesFileName = "stop_times.txt";
    [SerializeField] private string tripsFileName = "trips.txt";

    // =========================
    // MAIN STORAGE
    // =========================

    public readonly Dictionary<string, StopData> StopsById = new();
    public readonly Dictionary<string, RouteData> RoutesById = new();
    public readonly Dictionary<string, TripData> TripsById = new();
    public readonly Dictionary<string, List<TripData>> TripsByRoute = new();

    public Dictionary<string, List<string>> SameNameStops {get; private set; }

    // IMPORTANT:
    // stop_times.txt is HUGE.
    // We therefore index it immediately.

    public readonly Dictionary<string, List<StopTimeData>> StopTimesByTrip = new();
    public readonly Dictionary<string, List<StopTimeData>> StopTimesByStop = new();

    // Optional raw storage
    public readonly List<StopTimeData> AllStopTimes = new();

    private void Awake()
    {
        LoadStops();
        LoadRoutes();
        LoadTrips();
        LoadStopTimes();

        BuildSameNameMap();

        Debug.Log($"Loaded:\n" +
                  $"Stops: {StopsById.Count}\n" +
                  $"Routes: {RoutesById.Count}\n" +
                  $"StopTimes: {AllStopTimes.Count}\n" +
                  $"Trips: {TripsById.Count}");
    }

    // =====================================================
    // STOPS
    // =====================================================

private void LoadTrips()
{
    string path = Path.Combine(Application.streamingAssetsPath, tripsFileName);

    if (!File.Exists(path))
    {
        Debug.LogError($"Trips file not found: {path}");
        return;
    }

    string[] lines = File.ReadAllLines(path);

    for (int i = 1; i < lines.Length; i++)
    {
        if (string.IsNullOrWhiteSpace(lines[i]))
            continue;

        string[] cols = SplitCsvLine(lines[i]);

        try
        {
            // Standard GTFS:
            // route_id,service_id,trip_id,...

            TripData trip = new()
            {
                RouteId = cols[0],
                ServiceId = cols[1],
                TripId = cols[2]
            };

            TripsById[trip.TripId] = trip;

            // =========================
            // INDEX BY ROUTE
            // =========================

            if (!TripsByRoute.ContainsKey(trip.RouteId))
            {
                TripsByRoute[trip.RouteId] = new List<TripData>();
            }

            TripsByRoute[trip.RouteId].Add(trip);
        }
        catch (Exception ex)
        {
            Debug.LogWarning($"Failed parsing trip line {i}: {ex.Message}");
        }
    }
}

    private void LoadStops()
    {
        string path = Path.Combine(Application.streamingAssetsPath, stopsFileName);

        if (!File.Exists(path))
        {
            Debug.LogError($"Stops file not found: {path}");
            return;
        }

        string[] lines = File.ReadAllLines(path);

        for (int i = 1; i < lines.Length; i++)
        {
            if (string.IsNullOrWhiteSpace(lines[i]))
                continue;

            string[] cols = SplitCsvLine(lines[i]);

            try
            {
                StopData stop = new()
                {
                    StopId = cols[0],
                    StopCode = cols[1],
                    StopName = cols[2],
                    StopDescription = cols[3],
                    Latitude = ParseFloat(cols[4]),
                    Longitude = ParseFloat(cols[5]),
                    ZoneId = cols[6]
                };

                StopsById[stop.StopId] = stop;
            }
            catch (Exception ex)
            {
                Debug.LogWarning($"Failed parsing stop line {i}: {ex.Message}");
            }
        }
    }

    // =====================================================
    // ROUTES
    // =====================================================

    private void LoadRoutes()
    {
        string path = Path.Combine(Application.streamingAssetsPath, routesFileName);

        if (!File.Exists(path))
        {
            Debug.LogError($"Routes file not found: {path}");
            return;
        }

        string[] lines = File.ReadAllLines(path);

        for (int i = 1; i < lines.Length; i++)
        {
            if (string.IsNullOrWhiteSpace(lines[i]))
                continue;

            string[] cols = SplitCsvLine(lines[i]);

            try
            {
                RouteData route = new()
                {
                    RouteId = cols[0],
                    AgencyId = cols[1],
                    ShortName = cols[2],
                    LongName = cols[3],
                    RouteType = ParseInt(cols[5]),
                    RouteColor = cols[7],
                    RouteTextColor = cols[8]
                };

                RoutesById[route.RouteId] = route;
            }
            catch (Exception ex)
            {
                Debug.LogWarning($"Failed parsing route line {i}: {ex.Message}");
            }
        }
    }

    // =====================================================
    // STOP TIMES
    // =====================================================

    private void LoadStopTimes()
    {
        string path = Path.Combine(Application.streamingAssetsPath, stopTimesFileName);

        if (!File.Exists(path))
        {
            Debug.LogError($"StopTimes file not found: {path}");
            return;
        }

        // IMPORTANT:
        // Read line-by-line instead of File.ReadAllLines
        // because stop_times.txt is massive.

        using StreamReader reader = new(path);

        // Skip header
        reader.ReadLine();

        int lineIndex = 1;

        while (!reader.EndOfStream)
        {
            string line = reader.ReadLine();
            lineIndex++;

            if (string.IsNullOrWhiteSpace(line))
                continue;

            string[] cols = SplitCsvLine(line);

            try
            {
                StopTimeData stopTime = new()
                {
                    TripId = cols[0],
                    ArrivalTime = cols[1],
                    DepartureTime = cols[2],
                    StopId = cols[3],
                    StopSequence = ParseInt(cols[4])
                };

                AllStopTimes.Add(stopTime);

                // =========================
                // INDEX BY TRIP
                // =========================

                if (!StopTimesByTrip.ContainsKey(stopTime.TripId))
                {
                    StopTimesByTrip[stopTime.TripId] = new List<StopTimeData>();
                }

                StopTimesByTrip[stopTime.TripId].Add(stopTime);

                // =========================
                // INDEX BY STOP
                // =========================

                if (!StopTimesByStop.ContainsKey(stopTime.StopId))
                {
                    StopTimesByStop[stopTime.StopId] = new List<StopTimeData>();
                }

                StopTimesByStop[stopTime.StopId].Add(stopTime);
            }
            catch (Exception ex)
            {
                Debug.LogWarning($"Failed parsing stop_time line {lineIndex}: {ex.Message}");
            }
        }

        // IMPORTANT:
        // Sort stop sequences once during import.

        foreach (List<StopTimeData> tripStopTimes in StopTimesByTrip.Values)
        {
            tripStopTimes.Sort((a, b) => a.StopSequence.CompareTo(b.StopSequence));
        }
    }
    public void BuildSameNameMap()
    {
        SameNameStops = new Dictionary<string, List<string>>();

        // Group by stop name, ignoring case and whitespace
        var groups = StopsById.Values
            .GroupBy(s => s.StopName.Trim(), StringComparer.OrdinalIgnoreCase);

        foreach (var group in groups)
        {
            var idList = group.Select(s => s.StopId).ToList();
            foreach (string id in idList)
            {
                // Siblings = all other IDs in the same group
                SameNameStops[id] = idList.Where(x => x != id).ToList();
            }
        }
    }

    // =====================================================
    // QUERY METHODS
    // =====================================================

    public StopData GetStop(string stopId)
    {
        StopsById.TryGetValue(stopId, out StopData stop);
        return stop;
    }

    public RouteData GetRoute(string routeId)
    {
        RoutesById.TryGetValue(routeId, out RouteData route);
        return route;
    }

    public List<StopTimeData> GetStopTimesForTrip(string tripId)
    {
        if (StopTimesByTrip.TryGetValue(tripId, out List<StopTimeData> stopTimes))
        {
            return stopTimes;
        }

        return null;
    }

    public List<StopTimeData> GetStopTimesForStop(string stopId)
    {
        if (StopTimesByStop.TryGetValue(stopId, out List<StopTimeData> stopTimes))
        {
            return stopTimes;
        }

        return null;
    }

    // =====================================================
    // HELPERS
    // =====================================================

    private float ParseFloat(string value)
    {
        return float.Parse(value, CultureInfo.InvariantCulture);
    }

    private int ParseInt(string value)
    {
        return int.Parse(value);
    }

    private string[] SplitCsvLine(string line)
    {
        List<string> values = new();

        bool insideQuotes = false;
        string current = "";

        foreach (char c in line)
        {
            if (c == '"')
            {
                insideQuotes = !insideQuotes;
            }
            else if (c == ',' && !insideQuotes)
            {
                values.Add(current);
                current = "";
            }
            else
            {
                current += c;
            }
        }

        values.Add(current);

        return values.ToArray();
    }

    public TripData GetTrip(string tripId)
    {
        TripsById.TryGetValue(tripId, out TripData trip);
        return trip;
    }

    public List<TripData> GetTripsForRoute(string routeId)
    {
        if (TripsByRoute.TryGetValue(routeId, out List<TripData> trips))
        {
            return trips;
        }

        return null;
    }
}

// =====================================================
// DATA CLASSES
// =====================================================

[Serializable]
public class StopData
{
    public string StopId;
    public string StopCode;
    public string StopName;
    public string StopDescription;

    public float Latitude;
    public float Longitude;

    public string ZoneId;
}

[Serializable]
public class RouteData
{
    public string RouteId;
    public string AgencyId;

    public string ShortName;
    public string LongName;

    public int RouteType;

    public string RouteColor;
    public string RouteTextColor;
}

[Serializable]
public class StopTimeData
{
    public string TripId;

    public string ArrivalTime;
    public string DepartureTime;

    public string StopId;

    public int StopSequence;
}

[Serializable]
public class TripData
{
    public string RouteId;
    public string ServiceId;

    public string TripId;
}