//
// RampTexMaker.cs - This script provides a tool that lets you bake Unity gradients into textures!
//

using System.Collections;
using System.Collections.Generic;
using System.IO;

using UnityEngine;
using UnityEditor;

namespace Opal
{
    public class RampTexMaker : EditorWindow
    {
        [MenuItem("Opal/Ramp Baker")]
        static void Init()
        {
            RampTexMaker window = EditorWindow.GetWindow<RampTexMaker>();
            window.connection = null;
            window.Show();
        }

        Gradient gradient = null;
        Gradient skinGradient = null;
        Gradient xGradient, yGradient = null;
        AnimationCurve curve = null;
        int width = 256;
        int height = 8;
        public OpalBRDFEditor connection = null;
        string lastSave = "";
        GUIStyle centerBoldStyle = null;

        public enum RampBakeMode
        {
            FalloffFunction,
            XYGradient,
            Gradient
        };

        RampBakeMode mode = RampBakeMode.Gradient;

        private void OnGUI()
        {
            if (centerBoldStyle == null)
            {
                centerBoldStyle = new GUIStyle(EditorStyles.boldLabel);
                centerBoldStyle.alignment = TextAnchor.MiddleCenter;
            }

            GUILayout.Label("Modes", centerBoldStyle);

            GUILayout.BeginHorizontal();

            if (GUILayout.Button("Gradient Baker", EditorStyles.miniButtonLeft))
                mode = RampBakeMode.Gradient;

            if (GUILayout.Button("XY Gradient Baker", EditorStyles.miniButtonMid))
                mode = RampBakeMode.XYGradient;

            if (GUILayout.Button("Curve Baker", EditorStyles.miniButtonRight))
                mode = RampBakeMode.FalloffFunction;

            GUILayout.EndHorizontal();

            GUILayout.Space(5);

            if (mode == RampBakeMode.Gradient)
            {
                GUILayout.Label("Gradient Mode", centerBoldStyle);

                if (gradient == null)
                {
                    gradient = new Gradient();
                    gradient.SetKeys(
                        new GradientColorKey[] {
                            new GradientColorKey(Color.black, 0), new GradientColorKey(Color.black, 0.45F), new GradientColorKey(Color.white, 1)
                        },
                        new GradientAlphaKey[] {
                            new GradientAlphaKey(1, 0), new GradientAlphaKey(1, 1)
                        }
                    );
                }

                gradient = EditorGUILayout.GradientField("Gradient", gradient);

                if (GUILayout.Button("Reset Gradient"))
                    gradient = null;
            }

            if (mode == RampBakeMode.XYGradient)
            {
                GUILayout.Label("XY Gradient Mode", centerBoldStyle);

                if (xGradient == null)
                {
                    xGradient = new Gradient();
                    xGradient.SetKeys(
                        new GradientColorKey[] {
                            new GradientColorKey(Color.black, 0), new GradientColorKey(Color.black, 0.45F), new GradientColorKey(Color.white, 1)
                        },
                        new GradientAlphaKey[] {
                            new GradientAlphaKey(1, 0), new GradientAlphaKey(1, 1)
                        }
                    );
                }

                if (yGradient == null)
                {
                    yGradient = new Gradient();
                    yGradient.SetKeys(
                        new GradientColorKey[] {
                            new GradientColorKey(Color.black, 0), new GradientColorKey(Color.black, 0.45F), new GradientColorKey(Color.white, 1)
                        },
                        new GradientAlphaKey[] {
                            new GradientAlphaKey(1, 0), new GradientAlphaKey(1, 1)
                        }
                    );
                }

                xGradient = EditorGUILayout.GradientField("X Gradient", xGradient);
                yGradient = EditorGUILayout.GradientField("Y Gradient", yGradient);

                GUILayout.BeginHorizontal();

                if (GUILayout.Button("Reset X Gradient", EditorStyles.miniButtonLeft))
                    xGradient = null;

                if (GUILayout.Button("Reset Y Gradient", EditorStyles.miniButtonRight))
                    yGradient = null;

                GUILayout.EndHorizontal();
            }

            if (mode == RampBakeMode.FalloffFunction)
            {
                GUILayout.Label("Falloff Mode", centerBoldStyle);

                if (curve == null)
                {
                    // Simulate a fake inverse sqaure falloff
                    const int samples = 128;

                    Keyframe[] frames = new Keyframe[samples];
                    for (int i = 0; i < samples; i++)
                    {
                        float t = i / (float)samples;
                        float v = 1F - Mathf.Sqrt(t);
                        frames[i] = new Keyframe(1F - t, v);
                    }

                    curve = new AnimationCurve(frames);

                    for (int i = 0; i < curve.length; i++)
                        curve.SmoothTangents(i, 0F);
                }

                if (skinGradient == null)
                    skinGradient = new Gradient();

                curve = EditorGUILayout.CurveField("Falloff Curve", curve);
                skinGradient = EditorGUILayout.GradientField("Skin Gradient", skinGradient);

                if (GUILayout.Button("Reset Curve"))
                    curve = null;
            }

            GUILayout.Space(5);
            GUILayout.Label("Output", centerBoldStyle);

            width = EditorGUILayout.IntField("Output Width", width);
            height = EditorGUILayout.IntField("Output Height", height);

            bool shouldSave = false;
            bool useLast = false;

            if (!string.IsNullOrEmpty(lastSave))
            {
                GUILayout.BeginHorizontal();

                if (GUILayout.Button("Overwrite", EditorStyles.miniButtonLeft))
                    useLast = shouldSave = true;

                if (GUILayout.Button("Save As New", EditorStyles.miniButtonRight))
                    shouldSave = true;

                GUILayout.EndHorizontal();
            }
            else
                shouldSave = GUILayout.Button("Save Texture");

            if (shouldSave)
            {
                Texture2D texture = new Texture2D(width, height, TextureFormat.ARGB32, true);

                for (int x = 0; x < width; x++)
                {
                    Color color = Color.red;
                    float t = x / (float)width;
                    
                    if (mode == RampBakeMode.XYGradient)
                    {
                        for (int y = 0; y < height; y++)
                        {
                            float t2 = y / (float)width;

                            Vector4 mix = xGradient.Evaluate(t) + yGradient.Evaluate(t2);
                            mix = Vector4.Min(mix, Vector4.one);
                            mix = Vector4.Max(mix, Vector4.zero);

                            texture.SetPixel(x, y, mix);
                        }

                        continue;
                    }

                    if (mode == RampBakeMode.Gradient)
                    {
                        color = gradient.Evaluate(t);
                    }
                    
                    if (mode == RampBakeMode.FalloffFunction)
                    {
                        float eval = curve.Evaluate(t);
                        color = skinGradient.Evaluate(eval);
                        color.a = 1;
                    }

                    for (int y = 0; y < height; y++)
                    {
                        texture.SetPixel(x, y, color);
                    }
                }

                texture.Apply();
                texture.wrapModeU = texture.wrapModeV = TextureWrapMode.Clamp;

                byte[] pixels = texture.EncodeToPNG();

                string path = "";
                if (useLast && !string.IsNullOrEmpty(lastSave))
                    path = lastSave;
                else
                    path = EditorUtility.SaveFilePanel("Save Image", "", "", "png");

                File.WriteAllBytes(path, pixels);

                string relative = "Assets" + path.Replace(Application.dataPath, "");
                AssetDatabase.ImportAsset(relative);

                if (connection != null) {
                    var cpy = AssetDatabase.LoadAssetAtPath<Texture2D>(relative);
                    connection.newRamp = cpy;
                }

                lastSave = path;
            }

            if (connection != null)
                GUILayout.Label("Connected to BRDF editor, will update ramp when saved!");
        }
    }
}