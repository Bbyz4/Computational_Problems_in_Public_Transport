using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class RaptorRouting : MonoBehaviour
{
    private TransitDatabase database;

    // Labels
    private readonly Dictionary<string, int> earliestArrival = new();
    private readonly Dictionary<string, string> previousStop = new();
    private readonly Dictionary<string, string> previousTrip = new();

    private void Awake()
    {
        database = GameObject.FindWithTag("TransitDatabase").GetComponent<TransitDatabase>();
    }

    public JourneyResult FindRoute(string startStopName, string endStopName,
                                   string departureTimeString, int maxRounds)
    {
        StopData startStop = database.StopsById.Values
            .FirstOrDefault(s => s.StopName.Equals(startStopName, StringComparison.OrdinalIgnoreCase));
        StopData endStop = database.StopsById.Values
            .FirstOrDefault(s => s.StopName.Equals(endStopName, StringComparison.OrdinalIgnoreCase));

        if (startStop == null || endStop == null)
        {
            Debug.LogError("Start or end stop not found.");
            return null;
        }

        int departureTime = ParseGtfsTimeToMinutes(departureTimeString);
        return RunRaptor(startStop.StopId, endStop.StopId, departureTime, maxRounds);
    }

    private JourneyResult RunRaptor(string startStopId, string targetStopId,
                                    int departureTime, int maxRounds)
    {
        // Initialisation
        earliestArrival.Clear();
        previousStop.Clear();
        previousTrip.Clear();

        foreach (string stopId in database.StopsById.Keys)
            earliestArrival[stopId] = int.MaxValue;
        earliestArrival[startStopId] = departureTime;

        HashSet<string> markedStops = new() { startStopId };

        // Pre‑compute an ordered list of trips for each route.
        // We sort by the departure time at the trip’s first stop.
        Dictionary<string, List<TripData>> tripsByRouteSorted = new();
        foreach (var kvp in database.TripsByRoute)
        {
            string routeId = kvp.Key;
            List<TripData> trips = kvp.Value;

            // Sort trips by the departure time of their first stop.
            List<TripData> sortedTrips = trips
                .Select(t => new { Trip = t, FirstDep = GetFirstDeparture(t.TripId) })
                .Where(x => x.FirstDep >= 0)
                .OrderBy(x => x.FirstDep)
                .Select(x => x.Trip)
                .ToList();

            tripsByRouteSorted[routeId] = sortedTrips;
        }

        // Main Raptor rounds
        for (int round = 0; round < maxRounds; round++)
        {
            HashSet<string> newMarkedStops = new();

            // Process every route exactly once per round.
            foreach (string routeId in tripsByRouteSorted.Keys)
            {
                List<TripData> trips = tripsByRouteSorted[routeId];

                foreach (TripData trip in trips)
                {
                    string tripId = trip.TripId;
                    if (!database.StopTimesByTrip.TryGetValue(tripId, out List<StopTimeData> stopSequence))
                        continue;  // should not happen

                    // Find the first stop in this trip’s sequence that is:
                    //   - in the currently marked set
                    //   - has earliestArrival[stop] <= the trip’s departure time at that stop
                    string boardingStopId = null;
                    int boardingDepartureTime = -1;
                    bool boarded = false;

                    foreach (StopTimeData st in stopSequence)
                    {
                        if (markedStops.Contains(st.StopId))
                        {
                            int dep = ParseGtfsTimeToMinutes(st.DepartureTime);
                            if (dep >= earliestArrival[st.StopId])
                            {
                                boardingStopId = st.StopId;
                                boardingDepartureTime = dep;
                                boarded = true;
                                break;   // board at the earliest possible stop
                            }
                        }
                    }

                    if (!boarded)
                        continue;   // cannot board this trip from any marked stop

                    // Now update all stops **after** the boarding point.
                    bool pastBoarding = false;
                    foreach (StopTimeData st in stopSequence)
                    {
                        if (!pastBoarding)
                        {
                            if (st.StopId == boardingStopId)
                                pastBoarding = true;
                            continue;
                        }

                        int arr = ParseGtfsTimeToMinutes(st.ArrivalTime);

                        foreach(string sameStopID in database.SameNameStops.GetValueOrDefault(st.StopId))
                        {
                            if (arr < earliestArrival[sameStopID])
                            {
                                StopData stoooop = database.GetStop(sameStopID);
                                Debug.Log($"Earliest arrival to {stoooop.StopName} will be at {arr}");

                                earliestArrival[sameStopID] = arr;
                                previousStop[sameStopID] = boardingStopId;
                                previousTrip[sameStopID] = tripId;
                                newMarkedStops.Add(sameStopID);
                            }   
                        }
                    }
                }
            }

            // If we reached the target during this round, we can stop.
            if (earliestArrival[targetStopId] < int.MaxValue)
                break;

            // Prepare for the next round
            markedStops = newMarkedStops;
            if (markedStops.Count == 0)
                break;   // no further improvements possible
        }

        return BuildJourney(startStopId, targetStopId);
    }

    /// <summary>
    /// Returns the departure time (minutes from midnight) of the first stop
    /// of the given trip, or -1 if not found.
    /// </summary>
    private int GetFirstDeparture(string tripId)
    {
        if (!database.StopTimesByTrip.TryGetValue(tripId, out List<StopTimeData> stops))
            return -1;
        if (stops.Count == 0)
            return -1;
        // stops are sorted by StopSequence during import
        return ParseGtfsTimeToMinutes(stops[0].DepartureTime);
    }

    private JourneyResult BuildJourney(string startStopId, string targetStopId)
    {
        if (!earliestArrival.TryGetValue(targetStopId, out int finalTime) || finalTime == int.MaxValue)
        {
            Debug.Log("No route found.");
            return null;
        }

        List<JourneyStep> steps = new();
        string currentStopId = targetStopId;

        while (currentStopId != startStopId)
        {
            if (!previousStop.ContainsKey(currentStopId))
                break;

            string prevStopId = previousStop[currentStopId];
            string tripId = previousTrip[currentStopId];

            StopData fromStop = database.GetStop(prevStopId);
            StopData toStop = database.GetStop(currentStopId);
            TripData trip = database.GetTrip(tripId);
            RouteData route = database.GetRoute(trip.RouteId);

            steps.Add(new JourneyStep
            {
                FromStopName = fromStop.StopName,
                ToStopName = toStop.StopName,
                RouteName = route.ShortName,
                TripId = tripId,
                ArrivalTime = earliestArrival[currentStopId]
            });

            currentStopId = prevStopId;
        }

        steps.Reverse();
        return new JourneyResult { Steps = steps, FinalArrivalTime = finalTime };
    }

    private int ParseGtfsTimeToMinutes(string time)
    {
        string[] parts = time.Split(':');
        int hours = int.Parse(parts[0]);
        int minutes = int.Parse(parts[1]);
        return hours * 60 + minutes;
    }
}
// --- Result classes (same signature) ---
[Serializable]
public class JourneyResult
{
    public List<JourneyStep> Steps;
    public int FinalArrivalTime; // in minutes from midnight
}

[Serializable]
public class JourneyStep
{
    public string FromStopName;
    public string ToStopName;
    public string RouteName;
    public string TripId;
    public int ArrivalTime; // arrival at the *to* stop, in minutes from midnight
}