using GPURaytracing;
using System.Collections;
using System.Collections.Generic;
using Unity.Mathematics;
using UnityEngine;

public class RTGIDebug : MonoBehaviour
{
    public Color startColor = Color.white;
    public int samples = 4;

    private void OnDrawGizmosSelected()
    {
        Color color = startColor;
        Gizmos.color = color;

        Ray ray = new Ray(transform.position, transform.forward);

        for (int g = 0; g < samples; g++)
        {
            if (Physics.Raycast(ray, out RaycastHit info))
            {
                RTParticipant part = info.collider.GetComponent<RTParticipant>();

                if (part != null)
                {
                    color *= part.color;
                    color += part.glowColor;
                    color.a = 1;

                    print(part.gameObject);

                    Gizmos.DrawLine(ray.origin, info.point);
                    Gizmos.color = color;
                }

                float3 incidence = Vector3.Reflect(ray.direction, info.normal);
                ray = new Ray(info.point, incidence);
            }
            else
            {
                break;
            }
        }
    }
}
