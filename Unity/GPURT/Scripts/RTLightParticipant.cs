using System.Collections;
using System.Collections.Generic;
using Unity.Collections;
using UnityEngine;
using UnityEngine.Rendering;

namespace GPURaytracing
{
    [RequireComponent(typeof(Light))]
    public class RTLightParticipant : MonoBehaviour
    {
        protected new Light light;
        protected RTRenderer.RTLight rtLight;

        [Header("Shape")]
        public bool syncWithLight = true;
        public float range = 5F;
        public float radius = 0.01F;
        public float angle = 35.0F;
        public float innerAngle = 0.0F;

        [Header("Energy")]
        public Color color = Color.white;

        public float intensity = 1.0F;
        public RTRenderer.RTLightType type = RTRenderer.RTLightType.Point;

        public void OnEnable()
        {
            light = GetComponent<Light>();
            rtLight = new RTRenderer.RTLight();

            RTRenderer.Lights.Add(this);
        }

        public void OnDisable()
        {
            RTRenderer.Lights.Remove(this);
        }

        public virtual RTRenderer.RTLight GetLight()
        {
            RTRenderer.RTLightType type = this.type;

            // TODO: Encode sin/cos info for cone / spot lights
            float angle = this.angle;

            if (syncWithLight)
            {
                rtLight.color = (Vector4)light.color * light.intensity;
                rtLight.range = light.range;

                angle = light.spotAngle;

                switch (light.type)
                {
                    default:
                        type = RTRenderer.RTLightType.Point;
                        break;

                    case LightType.Directional:
                        type = RTRenderer.RTLightType.Sun;
                        break;

                    case LightType.Spot:
                        type = RTRenderer.RTLightType.Spot;
                        break;
                }
            }
            else
            {
                rtLight.color = (Vector4)color * intensity;
                rtLight.range = range;
            
                // TODO: Spotlights
            }


            if (type == RTRenderer.RTLightType.Sun)
                rtLight.position = transform.forward;
            else
                rtLight.position = transform.position;

            float innerPerc = this.innerAngle / angle;

            //float perc = Mathf.Clamp(innerPerc, 0.0f, 100.0F) / 100.0F;
            float perc = innerPerc;
            float phi = Mathf.Clamp(Mathf.Cos(angle * 0.5F * Mathf.Deg2Rad), 0, 1);
            float theta = Mathf.Clamp(Mathf.Cos(angle * perc * 0.5f * Mathf.Deg2Rad), 0, 1);
            float theta_phi = 1.0F / Mathf.Max(0.01F, theta - phi);

            rtLight.cosines = new Vector3(perc, phi, theta_phi);
            rtLight.radius = radius;
            rtLight.type = (uint)type;
            rtLight.forward = -transform.forward;

            return rtLight;
        }
    }
}