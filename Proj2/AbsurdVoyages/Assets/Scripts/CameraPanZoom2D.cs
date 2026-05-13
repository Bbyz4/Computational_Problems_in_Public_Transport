using System;
using System.Collections.Generic;
using UnityEngine;

[RequireComponent(typeof(Camera))]
public class CameraPanZoom2D : MonoBehaviour
{
    public float panSpeed = 1.0f;
    public float zoomSpeed = 10f;
    public float minZoom = 2f;
    public float maxZoom = 100f;

    private List<float> keyZoomValues = new List<float>{2f, 5f, 10f, 20f, 50f, 100f};

    private int currentClosestKeyZoomIndex = 5;

    public static event Action<float> OnZoomChanged;

    private Camera cam;
    private Vector3 lastMouseWorldPos;

    void Awake()
    {
        cam = GetComponent<Camera>();
        cam.orthographic = true;

        cam.orthographicSize = maxZoom;
    }

    void Update()
    {
        HandlePan();
        HandleZoom();
    }

    void HandlePan()
    {
        if (Input.GetMouseButtonDown(1))
        {
            lastMouseWorldPos = cam.ScreenToWorldPoint(Input.mousePosition);
        }

        if (Input.GetMouseButton(1))
        {
            Vector3 currentMouseWorldPos = cam.ScreenToWorldPoint(Input.mousePosition);
            Vector3 delta = lastMouseWorldPos - currentMouseWorldPos;

            transform.position += delta;
        }
    }

    void HandleZoom()
    {
        float scroll = Input.GetAxis("Mouse ScrollWheel");

        if (Mathf.Abs(scroll) > 0.0001f)
        {
            cam.orthographicSize -= scroll * zoomSpeed;
            cam.orthographicSize = Mathf.Clamp(cam.orthographicSize, minZoom, maxZoom);

            int newIndex = 0;
            float closestDiff = Mathf.Abs(cam.orthographicSize - keyZoomValues[0]);

            for (int i = 1; i < keyZoomValues.Count; i++)
            {
                float diff = Mathf.Abs(cam.orthographicSize - keyZoomValues[i]);
                if (diff < closestDiff)
                {
                    closestDiff = diff;
                    newIndex = i;
                }
            }

            if (newIndex != currentClosestKeyZoomIndex)
            {
                currentClosestKeyZoomIndex = newIndex;
                OnZoomChanged?.Invoke(keyZoomValues[currentClosestKeyZoomIndex]);
            }
        }
    }

    public float GetCurrentKeyZoom()
    {
        return keyZoomValues[currentClosestKeyZoomIndex];
    }
}
