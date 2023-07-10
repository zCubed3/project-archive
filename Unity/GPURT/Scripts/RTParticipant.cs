using System.Collections;
using System.Collections.Generic;
using Unity.Collections;
using UnityEngine;
using UnityEngine.Rendering;

namespace GPURaytracing
{
    //[RequireComponent(typeof(MeshFilter))]
    public class RTParticipant : MonoBehaviour
    {
        protected MeshFilter meshFilter;

        protected RTRenderer.RTObject rtObject;
        protected RTRenderer.RTMaterial rtMaterial;

        [Header("Shape")]
        public RTRenderer.RTParticipantType participantType;
        public Vector3 shapeData = Vector3.one;

        [Header("Material")]
        public Color color = Color.white;

        [ColorUsage(false, true)]
        public Color glowColor = Color.black;

        [Range(0F, 1F)]
        public float roughness = 0.5F;

        [Range(0F, 1F)]
        public float metallic = 0.5F;

        public void OnEnable()
        {
            rtObject = new RTRenderer.RTObject();

            meshFilter = GetComponent<MeshFilter>();
            if (meshFilter != null)
            {
                // Register it to the giant list of meshes
                rtObject.mesh = RTRenderer.CacheMesh(meshFilter.sharedMesh);
            }
            else
            {
                participantType = RTRenderer.RTParticipantType.Sphere;
                Debug.LogWarning($"No MeshFilter found on RTParticipant '{gameObject}', falling back to sphere mode!");
            }

            RTRenderer.Participants.Add(this);
        }

        public void OnDisable()
        {
            RTRenderer.Participants.Remove(this);
        }

        public virtual RTRenderer.RTObject GetObject()
        {
            Vector4 encodedShapeData = shapeData;
            encodedShapeData.w = (float)participantType;

            rtMaterial.color = (Vector4)color;
            rtMaterial.glowColor = (Vector4)glowColor;
            rtMaterial.roughness = roughness;
            rtMaterial.metallic = metallic;

            RTRenderer.RTAABB aabb = new RTRenderer.RTAABB();

            if (meshFilter != null)
            {
                aabb = new RTRenderer.RTAABB()
                {
                    min = meshFilter.sharedMesh.bounds.min,
                    max = meshFilter.sharedMesh.bounds.max
                };
            }

            //Matrix4x4 transform = Matrix4x4.TRS(Vector3.up * Mathf.Sin(Time.time), Quaternion.Euler(Time.time * 90, 0, 0), Vector3.one);
            Matrix4x4 trs = transform.localToWorldMatrix;

            rtObject.trs = trs;
            rtObject.trsInverse = trs.inverse;
            rtObject.shapeData = encodedShapeData;
            rtObject.aabb = aabb;
            rtObject.material = rtMaterial;

            return rtObject;
        }
    }
}