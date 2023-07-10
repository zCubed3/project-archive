using System.Collections;
using System.Collections.Generic;
using Unity.Collections;

using UnityEngine;
using UnityEngine.Rendering;

using Unity.Burst;
using System;
using UnityEditor.Rendering;
using Unity.Mathematics;

namespace GPURaytracing
{
    public class RTRenderer : MonoBehaviour
    {
        protected struct RTCamera
        {
            public static int Size => sizeof(float) * 16 * 4;

            public Matrix4x4 cameraToWorldMatrix;
            public Matrix4x4 worldToCameraMatrix;
            public Matrix4x4 inverseProjectionMatrix;
            public Matrix4x4 projectionMatrix;

            public RTCamera(Camera camera)
            {
                this.cameraToWorldMatrix = camera.cameraToWorldMatrix;
                this.worldToCameraMatrix = camera.worldToCameraMatrix;
                this.inverseProjectionMatrix = camera.projectionMatrix.inverse;
                this.projectionMatrix = camera.projectionMatrix;
            }
        }

        protected struct RTScene
        {
            public static int Size => (sizeof(float) * 6) + (sizeof(uint) * 4);

            public float seed;
            public float stochastic;
            public uint objectCount;
            public uint lightCount;
            public Vector4 fogParams;
            public uint2 tileOffset;
        }

        public struct RTIndex
        {
            public static int Size => sizeof(uint) * 4;

            public uint v0;
            public uint v1;
            public uint v2;
            public uint unused;
        }

        public struct RTVertex
        {
            public static int Size => sizeof(float) * 8;

            public Vector3 position;
            public Vector3 normal;
            public Vector2 uv0;
        }

        public struct RTMesh
        {
            public static int Size => sizeof(uint) * 4;

            public uint indexOffset;
            public uint indexCount;
            uint unused;
            uint unused2;
        }

        public struct RTAABB
        {
            public static int Size => sizeof(float) * 8;

            public Vector4 min;
            public Vector4 max;
        }

        public struct RTMaterial
        {
            public static int Size => sizeof(float) * 8;

            public Vector3 color;
            public Vector3 glowColor;
            public float roughness;
            public float metallic;
        }

        public struct RTObject
        {
            public static int Size => RTAABB.Size + RTMesh.Size + RTMaterial.Size + (sizeof(float) * 36);

            public Matrix4x4 trs;
            public Matrix4x4 trsInverse;
            public Vector4 shapeData;
            public RTAABB aabb;
            public RTMesh mesh;
            public RTMaterial material;
        }

        public enum RTLightType : uint
        {
            Point,
            Spot,
            Sun
        }

        public enum RTParticipantType : uint
        {
            Mesh,
            Sphere
        }

        public struct RTLight
        {
            public static int Size => sizeof(float) * 15 + sizeof(uint) * 1;

            public Vector3 position;
            public Vector3 color;
            public float range;
            public float radius;
            public uint type;
            public Vector3 cosines;
            public Vector3 forward;
            public float unused;
        }

        //
        // Inputs
        //
        [Header("Compute Shaders")]
        public ComputeShader computeCS;

        public ComputeShader presentCS;

        public ComputeShader uberPostCS;

        [Header("Scene")]
        public Color fogColor = Color.white;

        [Range(0.0F, 1.0F)]
        public float fogScatter = 0.7F;

        [Header("Sampling")]
        public Texture2D noisePattern = null;

        [Range(0F, 1F)]
        public float stochasticRatio = 0.9F;

        [Range(0F, 1F)]
        public float stochasticDetailRatio = 0.9F;

        [Header("Sampling Tweakables")]
        public bool temporalSampling = false;

        public int temporalSamples = 64;

        public bool tiledSampling = false;

        protected bool lastTemporalSampling = false;
        protected bool lastTiledSampling = false;
        protected int lastTemporalSamples = -1;
        protected int totalSamples = 0;

        [Header("BRDF")]
        public Texture2D iblBRDFLut = null;

        [Header("Post Processing")]
        public bool postProcessEnabled = true;
        
        [Range(0F, 1F)]
        public float bloomScatter = 0.2F;

        [Range(0F, 1F)]
        public float bloomMix = 0.1F;

        //
        // Data
        //
        protected new Camera camera;
        protected int computeKernel;

        protected ComputeBuffer cameraDataBuffer = null;
        protected ComputeBuffer lastCameraDataBuffer = null;
        protected ComputeBuffer sceneDataBuffer = null;

        protected RTCamera[] lastCameras;
        protected RTCamera[] cameras;

        protected CommandBuffer postCmd = null;

        protected RenderTexture dataBuffer = null;
        protected RenderTexture colorBuffer = null;

        public static ComputeBuffer IndexBuffer = null;
        public static ComputeBuffer VertexBuffer = null;
        public static ComputeBuffer ObjectBuffer = null;
        public static ComputeBuffer LightBuffer = null;

        public static List<RTParticipant> Participants = new List<RTParticipant>();
        public static List<RTLightParticipant> Lights = new List<RTLightParticipant>();
        public static Dictionary<Mesh, RTMesh> MeshCache = new Dictionary<Mesh, RTMesh>();
        protected static int indexBlobOffset = 0;
        protected static int vertexBlobOffset = 0;

        protected uint2 rtTileOffset;

        public static void VerifyBuffers()
        {
            if (IndexBuffer == null)
                IndexBuffer = new ComputeBuffer(65536, RTIndex.Size);

            if (VertexBuffer == null)
                VertexBuffer = new ComputeBuffer(65536, RTVertex.Size);

            if (ObjectBuffer == null)
                ObjectBuffer = new ComputeBuffer(512, RTObject.Size);

            if (LightBuffer == null)
                LightBuffer = new ComputeBuffer(512, RTLight.Size);
        }

        [BurstCompile]
        public static RTMesh CacheMesh(Mesh mesh)
        {
            if (MeshCache.ContainsKey(mesh))
                return MeshCache[mesh];

            VerifyBuffers();

            int roundedTris = (mesh.triangles.Length + 3 - 1) / 3;

            RTIndex[] indexes = new RTIndex[roundedTris];
            RTVertex[] vertices = new RTVertex[mesh.vertexCount];

            for (int i = 0; i < indexes.Length; i++)
            {
                int offset = i * 3;

                indexes[i] = new RTIndex()
                {
                    v0 = (uint)(vertexBlobOffset + mesh.triangles[offset]),
                    v1 = (uint)(vertexBlobOffset + mesh.triangles[offset + 1]),
                    v2 = (uint)(vertexBlobOffset + mesh.triangles[offset + 2]),
                    unused = 0
                };
            }

            for (int v = 0; v < vertices.Length; v++)
            {
                vertices[v] = new RTVertex()
                {
                    position = mesh.vertices[v],
                    normal = mesh.normals[v],
                    uv0 = mesh.uv[v]
                };
            }

            IndexBuffer.SetData(indexes, 0, indexBlobOffset, indexes.Length);
            VertexBuffer.SetData(vertices, 0, vertexBlobOffset, vertices.Length);

            RTMesh cachedMesh = new RTMesh()
            {
                indexCount = (uint)indexes.Length,
                indexOffset = (uint)indexBlobOffset
            };

            indexBlobOffset += indexes.Length;
            vertexBlobOffset += vertices.Length;

            MeshCache.Add(mesh, cachedMesh);
            return cachedMesh;
        }

        void OnEnable()
        {
            camera = GetComponent<Camera>();

            cameraDataBuffer = new ComputeBuffer(1, RTCamera.Size);
            lastCameraDataBuffer = new ComputeBuffer(1, RTCamera.Size);
            sceneDataBuffer = new ComputeBuffer(1, RTScene.Size, ComputeBufferType.Constant);

            VerifyBuffers();

            postCmd = new CommandBuffer();
        }

        void OnRenderImage(RenderTexture src, RenderTexture dest)
        {
            bool newBuffer = colorBuffer == null || dataBuffer == null;

            if (!newBuffer)
                newBuffer = colorBuffer.width != src.width || colorBuffer.height != src.height;

            if (!newBuffer)
                newBuffer = temporalSamples != lastTemporalSamples || temporalSampling != lastTemporalSampling;

            if (!newBuffer)
                newBuffer = tiledSampling != lastTiledSampling;

            if (newBuffer)
            {
                if (colorBuffer != null)
                    colorBuffer.Release();

                if (dataBuffer != null)
                    dataBuffer.Release();

                colorBuffer = new RenderTexture(src.width, src.height, 0, RenderTextureFormat.ARGBFloat);
                colorBuffer.enableRandomWrite = true;
                colorBuffer.Create();

                dataBuffer = new RenderTexture(src.width, src.height, 0, RenderTextureFormat.ARGBFloat);
                dataBuffer.enableRandomWrite = true;
                dataBuffer.Create();

                lastTemporalSamples = temporalSamples;
                lastTemporalSampling = temporalSampling;
                lastTiledSampling = tiledSampling;

                totalSamples = 0;
                rtTileOffset = new uint2();
            }

            //
            // Priming
            //
            computeKernel = computeCS.FindKernel("HelloTriangleKernel");
            computeCS.GetKernelThreadGroupSizes(computeKernel, out uint width, out uint height, out uint _);

            // Temporary texture
            RenderTextureDescriptor desc = src.descriptor;
            desc.msaaSamples = 1;
            desc.enableRandomWrite = true;
            desc.colorFormat = RenderTextureFormat.ARGBFloat;

            // Update the objects and lights
            RTObject[] objects = new RTObject[512];
            RTLight[] lights = new RTLight[512];

            int o = 0;

            foreach (RTParticipant participant in Participants)
                objects[o++] = participant.GetObject();

            o = 0;

            foreach (RTLightParticipant light in Lights)
                lights[o++] = light.GetLight();

            ObjectBuffer.SetData(objects);
            LightBuffer.SetData(lights);

            //
            // Sampling data
            //
            Vector4 sampleData = Vector4.zero;
            sampleData.x = Time.frameCount / 1000.0F;
            sampleData.y = stochasticRatio;
            sampleData.z = stochasticDetailRatio;

            computeCS.SetVector("_RTSampleData", sampleData);

            //
            // Scene
            //
            RTScene sceneData = new RTScene();
            sceneData.seed = Time.frameCount;
            sceneData.stochastic = stochasticRatio;
            sceneData.objectCount = (uint)Participants.Count;
            sceneData.lightCount = (uint)Lights.Count;
            sceneData.tileOffset = rtTileOffset;

            if (tiledSampling && totalSamples >= temporalSamples)
            {
                totalSamples = 0;
                rtTileOffset.x += width;

                if (rtTileOffset.x > src.width)
                {
                    rtTileOffset.y += height;
                    rtTileOffset.x = 0;
                }
            }

            // Thread counts
            int tileWidth = (int)width;
            int tileHeight = (int)height;

            int threadsX = 1, threadsY = 1;

            int fullThreadsX = (camera.pixelWidth + tileWidth - 1) / tileWidth;
            int fullThreadsY = (camera.pixelHeight + tileHeight - 1) / tileHeight;

            if (!tiledSampling)
            {
                threadsX = fullThreadsX;
                threadsY = fullThreadsY;
            }

            Vector4 fogEncode = (Vector4)fogColor;
            fogEncode.w = fogScatter;

            sceneData.fogParams = fogEncode;

            sceneDataBuffer.SetData(new RTScene[] { sceneData });

            // TEMP!
            //if (!temporalSampling)
            //{
                lastCameras = cameras;

                if (lastCameras != null)
                    lastCameraDataBuffer.SetData(lastCameras);

                //print("DODODO");
            //}

            cameras = new RTCamera[] { new RTCamera(camera) };
            cameraDataBuffer.SetData(cameras);

            //
            // Buffers
            //
            computeCS.SetConstantBuffer("_RTSceneData", sceneDataBuffer, 0, RTScene.Size);
            computeCS.SetBuffer(computeKernel, "_RTCameraData", cameraDataBuffer);
            computeCS.SetBuffer(computeKernel, "_RTLastCameraData", lastCameraDataBuffer);
            computeCS.SetBuffer(computeKernel, "_RTVertexBlob", VertexBuffer);
            computeCS.SetBuffer(computeKernel, "_RTIndexBlob", IndexBuffer);
            computeCS.SetBuffer(computeKernel, "_RTObjectBlob", ObjectBuffer);
            computeCS.SetBuffer(computeKernel, "_RTLightBlob", LightBuffer);

            computeCS.SetTexture(computeKernel, "_RTDataOutput", dataBuffer);
            computeCS.SetTexture(computeKernel, "_RTColorOutput", colorBuffer);

            //
            // Noise
            //
            if (noisePattern != null)
            {
                computeCS.EnableKeyword("_USE_RT_NOISE_TEX");
                computeCS.SetTexture(computeKernel, "_RTNoiseTex", noisePattern);
            }
            else
                computeCS.DisableKeyword("_USE_RT_NOISE_TEX");

            //
            // BRDF
            //
            if (iblBRDFLut != null)
            {
                computeCS.EnableKeyword("_USE_IBL_BRDF");
                computeCS.SetTexture(computeKernel, "_IBL_BRDF", iblBRDFLut);
            }
            else
                computeCS.DisableKeyword("_USE_IBL_BRDF");

            //
            // Temporal sampling
            //
            if (temporalSampling)
                computeCS.EnableKeyword("_TEMPORAL_SAMPLING");
            else
                computeCS.DisableKeyword("_TEMPORAL_SAMPLING");

            computeCS.SetInt("_TemporalSamples", temporalSamples);

            // Dispatch it
            computeCS.Dispatch(computeKernel, threadsX, threadsY, 1);
            totalSamples++;

            // Post processing
            postCmd.Clear();

            // Create a presentable image
            int presentID = Shader.PropertyToID("_PresentOut");
            postCmd.GetTemporaryRT(presentID, desc);

            tileWidth = (int)width;
            tileHeight = (int)height;

            threadsX = (camera.pixelWidth + tileWidth - 1) / tileWidth;
            threadsY = (camera.pixelHeight + tileHeight - 1) / tileHeight;

            int presentKernel = presentCS.FindKernel("Present");

            postCmd.SetComputeTextureParam(presentCS, presentKernel, "_SampleInput", colorBuffer);
            postCmd.SetComputeTextureParam(presentCS, presentKernel, "_SampleOutput", presentID);
            postCmd.DispatchCompute(presentCS, presentKernel, threadsX, threadsY, 1);

            if (postProcessEnabled)
            {
                int preprocessKernel = uberPostCS.FindKernel("BloomPreprocess");
                int downsampleKernel = uberPostCS.FindKernel("BloomDownsample");
                int upsampleKernel = uberPostCS.FindKernel("BloomUpsample");
                int postprocessKernel = uberPostCS.FindKernel("PostProcess");

                const int MAX_PYRAMID_SIZE = 16;
                int[] bloomMipUp = new int[MAX_PYRAMID_SIZE];
                int[] bloomMipDown = new int[MAX_PYRAMID_SIZE];

                for (int i = 0; i < MAX_PYRAMID_SIZE; i++)
                {
                    bloomMipUp[i] = Shader.PropertyToID("_BloomMipUp" + i);
                    bloomMipDown[i] = Shader.PropertyToID("_BloomMipDown" + i);
                }

                postCmd.GetTemporaryRT(bloomMipDown[0], desc, FilterMode.Bilinear);
                postCmd.GetTemporaryRT(bloomMipUp[0], desc, FilterMode.Bilinear);

                postCmd.SetComputeTextureParam(uberPostCS, preprocessKernel, "_SampleInput", presentID);
                postCmd.SetComputeTextureParam(uberPostCS, preprocessKernel, "_Output", bloomMipDown[0]);
                postCmd.DispatchCompute(uberPostCS, preprocessKernel, threadsX, threadsY, 1);

                int tw = camera.pixelWidth;
                int th = camera.pixelHeight;
                int maxSize = Mathf.Max(tw, th);
                int iterations = Mathf.FloorToInt(Mathf.Log(maxSize, 2f) - 1);
                int mipCount = Mathf.Clamp(iterations, 1, MAX_PYRAMID_SIZE);

                int lastDown = bloomMipDown[0];
                for (int i = 1; i < mipCount; i++)
                {
                    tw = Mathf.Max(1, tw >> 1);
                    th = Mathf.Max(1, th >> 1);
                    int mipDown = bloomMipDown[i];
                    int mipUp = bloomMipUp[i];

                    desc.width = tw;
                    desc.height = th;

                    postCmd.GetTemporaryRT(mipDown, desc, FilterMode.Bilinear);
                    postCmd.GetTemporaryRT(mipUp, desc, FilterMode.Bilinear);

                    postCmd.SetComputeTextureParam(uberPostCS, downsampleKernel, "_SampleInput", lastDown);
                    postCmd.SetComputeTextureParam(uberPostCS, downsampleKernel, "_Output", mipDown);
                    postCmd.DispatchCompute(uberPostCS, downsampleKernel, threadsX, threadsY, 1);

                    lastDown = mipDown;
                }

                for (int i = mipCount - 2; i >= 0; i--)
                {
                    int lowMip = (i == mipCount - 2) ? bloomMipDown[i + 1] : bloomMipUp[i + 1];
                    int highMip = bloomMipDown[i];
                    int dst = bloomMipUp[i];

                    postCmd.SetComputeTextureParam(uberPostCS, upsampleKernel, "_SampleInputLowMip", lowMip);
                    postCmd.SetComputeTextureParam(uberPostCS, upsampleKernel, "_SampleInput", highMip);
                    postCmd.SetComputeTextureParam(uberPostCS, upsampleKernel, "_Output", dst);
                    postCmd.DispatchCompute(uberPostCS, upsampleKernel, threadsX, threadsY, 1);
                }

                for (int i = 0; i < mipCount; i++)
                {
                    postCmd.ReleaseTemporaryRT(bloomMipDown[i]);
                    if (i > 0)
                        postCmd.ReleaseTemporaryRT(bloomMipUp[i]);
                }

                desc.width = camera.pixelWidth;
                desc.height = camera.pixelHeight;

                int tempId = Shader.PropertyToID("PostProcessTemp");
                postCmd.GetTemporaryRT(tempId, desc);
                postCmd.Blit(presentID, tempId);

                postCmd.SetComputeVectorParam(uberPostCS, "_BloomParams", new Vector4(bloomScatter, bloomMix, 0, 0));
                postCmd.SetComputeTextureParam(uberPostCS, postprocessKernel, "_SampleInput", presentID);
                postCmd.SetComputeTextureParam(uberPostCS, postprocessKernel, "_BloomTexture", bloomMipUp[0]);
                postCmd.SetComputeTextureParam(uberPostCS, postprocessKernel, "_Output", tempId);
                postCmd.DispatchCompute(uberPostCS, postprocessKernel, threadsX, threadsY, 1);

                postCmd.Blit(tempId, dest);
            }
            else
            {
                postCmd.Blit(presentID, dest);
            }

            
            Graphics.ExecuteCommandBuffer(postCmd);
        }

        protected Rect windowRect;
        private void OnGUI()
        {
            GUILayout.Window(4786, windowRect, OnWindow, "GPURT");
        }

        private void OnWindow(int id)
        {
            temporalSampling = GUILayout.Toggle(temporalSampling, "Temporal Sampling");
            tiledSampling = GUILayout.Toggle(tiledSampling, "Tiled Sampling");

            Time.timeScale = temporalSampling ? 0 : 1;

            temporalSamples = (int)GUILayout.HorizontalSlider(temporalSamples, 32, 1024);
            GUILayout.Label($"Total Samples = {totalSamples}/{temporalSamples}");

            GUI.DragWindow();
        }
    }
}