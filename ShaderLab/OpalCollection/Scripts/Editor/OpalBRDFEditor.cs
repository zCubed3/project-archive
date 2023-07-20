﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

namespace Opal
{
    // Yes this is inspired by the Poiyomi editor
    public class OpalBRDFEditor : ShaderGUI
    {
        public static GUIStyle opalLogoStyle, centerBoldStyle, centerBoldBiggerStyle;
        public static Texture2D sssSkinTex, sssPlantTex, sssGrayTex;
        public Texture2D newRamp;

        bool rampExplainOut = false;

        public enum EditorPage
        {
            BRDF, Surface, Toggles
        }

        public EditorPage activePage = EditorPage.BRDF;

        public enum SurfaceModel
        {
            Standard, Retroreflective
        }

        public SurfaceModel pbrModel;

        public static Dictionary<SurfaceModel, string> pbrModelWords = new Dictionary<SurfaceModel, string>()
        {
            { SurfaceModel.Standard, "" },
            { SurfaceModel.Retroreflective, "RETROREFLECTIVE" },
        };


        public override void OnGUI(MaterialEditor materialEditor, MaterialProperty[] properties)
        {
            if (opalLogoStyle == null)
            {
                opalLogoStyle = new GUIStyle(EditorStyles.boldLabel);
                opalLogoStyle.alignment = TextAnchor.MiddleCenter;
                opalLogoStyle.fontStyle = FontStyle.Bold;
                opalLogoStyle.fontSize = 24;
            }

            if (centerBoldStyle == null)
            {
                centerBoldStyle = new GUIStyle(EditorStyles.boldLabel);
                centerBoldStyle.alignment = TextAnchor.MiddleCenter;
            }

            if (centerBoldBiggerStyle == null)
            {
                centerBoldBiggerStyle = new GUIStyle(EditorStyles.boldLabel);
                centerBoldBiggerStyle.alignment = TextAnchor.MiddleCenter;
                centerBoldBiggerStyle.fontSize += 4;
            }

            if (sssSkinTex == null)
                sssSkinTex = Resources.Load<Texture2D>("BRDF/SSS");

            if (sssPlantTex == null)
                sssPlantTex = Resources.Load<Texture2D>("BRDF/SSS_plant");

            if (sssGrayTex == null)
                sssGrayTex = Resources.Load<Texture2D>("BRDF/SSS_gray");

            //base.OnGUI(materialEditor, properties);
            //return;

            GUILayout.Label("Opal - BRDF", opalLogoStyle);
            GUILayout.Label("IGNORE VRCSDK WARNINGS! DO NOT DISABLE KEYWORDS!", centerBoldStyle);

            GUILayout.BeginHorizontal();

            if (GUILayout.Button("BRDF", EditorStyles.miniButtonLeft))
                activePage = EditorPage.BRDF;

            if (GUILayout.Button("Surface", EditorStyles.miniButtonMid))
                activePage = EditorPage.Surface;

            if (GUILayout.Button("Toggles", EditorStyles.miniButtonRight))
                activePage = EditorPage.Toggles;

            GUILayout.EndHorizontal();

            Material material = materialEditor.target as Material;
            Undo.RecordObject(material, "Changed parameters");

            GUILayout.Space(10);
            if (activePage == EditorPage.BRDF)
            {
                GUILayout.Label("BRDF Editor", centerBoldBiggerStyle);
                GUILayout.Space(10);

                GUILayout.Label("NOTE: Disable shadows for BRDF ramps that light the back! (Ex: Crystal shading)", centerBoldStyle);

                GUILayout.Space(10);

                if (newRamp)
                    material.SetTexture("_BRDFTex", newRamp);

                if (OpalEditorHelpers.AskTexture(ref material, "_BRDFTex", "BRDF Ramp"))
                {
                    material.EnableKeyword("HAS_BRDF_MAP");

                    Texture2D texture = material.GetTexture("_BRDFTex") as Texture2D;
                    if (texture.wrapMode != TextureWrapMode.Clamp)
                    {
                        GUILayout.Space(10);

                        GUILayout.BeginHorizontal();
                        GUILayout.Label("WARNING: BRDF Ramp's 'wrapMode' is not Clamp!", EditorStyles.boldLabel);
                        if (GUILayout.Button("Fix"))
                        {
                            TextureImporter importer = TextureImporter.GetAtPath(AssetDatabase.GetAssetPath(texture)) as TextureImporter;

                            if (importer == null)
                                return;

                            importer.wrapMode = TextureWrapMode.Clamp;
                            importer.isReadable = true;

                            TextureImporterSettings importerSettings = new TextureImporterSettings();
                            importer.ReadTextureSettings(importerSettings);

                            EditorUtility.SetDirty(importer);
                            importer.SaveAndReimport();
                        }
                        GUILayout.EndHorizontal();
                    }
                }
                else
                    material.DisableKeyword("HAS_BRDF_MAP");

                GUILayout.Space(10);
                GUILayout.Label("Presets", centerBoldStyle);
                GUILayout.Space(10);

                GUILayout.BeginHorizontal();

                if (GUILayout.Button("None", EditorStyles.miniButtonLeft))
                    material.SetTexture("_BRDFTex", null);

                if (GUILayout.Button("Skin", EditorStyles.miniButtonMid))
                    material.SetTexture("_BRDFTex", sssSkinTex);

                if (GUILayout.Button("Plant", EditorStyles.miniButtonMid))
                    material.SetTexture("_BRDFTex", sssPlantTex);

                if (GUILayout.Button("Gray", EditorStyles.miniButtonRight))
                    material.SetTexture("_BRDFTex", sssGrayTex);

                GUILayout.EndHorizontal();

                if (GUILayout.Button("Open Ramp Maker"))
                {
                    RampTexMaker window = EditorWindow.GetWindow<RampTexMaker>();
                    window.connection = this;
                    window.Show();
                }

                if (GUILayout.Button("Can't pick?", EditorStyles.miniButton))
                    rampExplainOut = !rampExplainOut;
            
                if (rampExplainOut)
                {
                    GUILayout.Label("Preset Help", centerBoldStyle);
                    GUILayout.Space(10);
                    GUILayout.Label("None: Softer lighting than Unity Standard, use for softer looking materials");
                    GUILayout.Label("Skin: Use for skin or anything flesh-like");
                    GUILayout.Label("Plant: Use for plant like coloring (has green midtones)");
                    GUILayout.Label("Gray: Use for even softer lighting, for fur and such");
                }
            }

            if (activePage == EditorPage.Surface)
            {
                GUILayout.Label("Surface", centerBoldBiggerStyle);

                GUILayout.Space(10);
                GUILayout.Label("Base Map", centerBoldStyle);

                OpalEditorHelpers.AskTexture(ref material, "_MainTex", "Main Texture");
                OpalEditorHelpers.AskColor(ref material, "_Color", "Color");

                OpalEditorHelpers.AskScaleOffsetInfo(ref material, "_MainTex", "Scale and Offset");

                GUILayout.Space(10);
                GUILayout.Label("Bump Mapping", centerBoldStyle);

                if (OpalEditorHelpers.AskTexture(ref material, "_BumpMap", "Normal Map"))
                    material.EnableKeyword("HAS_BUMP_MAP");
                else
                    material.DisableKeyword("HAS_BUMP_MAP");

                OpalEditorHelpers.AskFloat(ref material, "_NormalDepth", "Depth");

                GUILayout.Space(10);
                GUILayout.Label("Emission", centerBoldStyle);

                OpalEditorHelpers.AskTexture(ref material, "_EmissionMap", "Emission Map");
                OpalEditorHelpers.AskColorHDR(ref material, "_EmissionColor", "Color");

                GUILayout.Space(10);
                GUILayout.Label("Ambient Occlusion", centerBoldStyle);

                if (OpalEditorHelpers.AskTexture(ref material, "_OcclusionMap", "Occlusion Map"))
                    material.EnableKeyword("HAS_AO_MAP");
                else
                    material.DisableKeyword("HAS_AO_MAP");

                GUILayout.Space(10);

                GUILayout.Label("Detail Maps", centerBoldStyle);
                //GUIHelpers.AskTexture(ref material, "_DetailAlbedo", "Detail Albedo");
                OpalEditorHelpers.AskFloat(ref material, "_DetailBumpScale", "Detail Normal Scale");
                OpalEditorHelpers.AskTexture(ref material, "_DetailBumpMap", "Detail Normal Map");
                OpalEditorHelpers.AskScaleOffsetInfo(ref material, "_DetailAlbedo", "Scale and Offset");

                GUILayout.Space(10);

                GUILayout.Label("Material", centerBoldStyle);
                OpalEditorHelpers.AskFloatRange(ref material, "_Roughness", "Roughness", 0, 1);
                OpalEditorHelpers.AskFloatRange(ref material, "_Metallic", "Metallic", 0, 1);
                OpalEditorHelpers.AskFloatRange(ref material, "_Hardness", "Hardness", 0, 1);

                GUILayout.BeginHorizontal();

                GUILayout.Label("Surface Model", EditorStyles.boldLabel);
                pbrModel = (SurfaceModel)EditorGUILayout.EnumPopup("", (SurfaceModel)material.GetInt("_PBRModel"));
                material.SetInt("_PBRModel", (int)pbrModel);

                GUILayout.EndHorizontal();

                foreach (var pair in pbrModelWords)
                {
                    if (string.IsNullOrEmpty(pair.Value))
                        continue;

                    if (pair.Key == pbrModel)
                        material.EnableKeyword(pair.Value);
                    else
                        material.DisableKeyword(pair.Value);
                }

                GUILayout.Space(10);
            }

            if (activePage == EditorPage.Toggles)
            {
                GUILayout.Label("Toggles", centerBoldBiggerStyle);
                GUILayout.Space(10);

                if (OpalEditorHelpers.AskBool(ref material, "_RecieveShadowsToggle", "Recieve Shadows"))
                    material.EnableKeyword("RECIEVE_SHADOWS");
                else
                    material.DisableKeyword("RECIEVE_SHADOWS");
            }
        }

        
    }
}